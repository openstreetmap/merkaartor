// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_SELECTED_HPP
#define _GEOMETRY_SELECTED_HPP

#include <cmath> // fabs

#include <boost/concept/requires.hpp>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <geometry/core/access.hpp>
#include <geometry/core/topological_dimension.hpp>

#include <geometry/core/concepts/point_concept.hpp>
#include <geometry/core/concepts/nsphere_concept.hpp>

#include <geometry/algorithms/within.hpp>


/*!
\defgroup selected selection: check if a geometry is "selected" by a point

Checks if one geometry is selected by a point lying within or in the neighborhood of that geometry

\par Geometries:
- POINT: checks if points are CLOSE TO each other (< search_radius)
- LINESTRING: checks if selection point is CLOSE TO linestring (< search_radius)
- RING: checks if selection point is INSIDE the ring, search radius is ignored
- POLYGON: checks if selection point is INSIDE the polygon, but not inside any of its holes

*/


namespace geometry
{
	/*!
		\ingroup impl
	 */
	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		namespace selected
		{

			/*!
			\details Checks, per dimension, if d[I] not larger than search distance. If true for all
			dimensions then returns true. If larger stops immediately and returns false.
			Calculate during this process the sum, which is only valid if returning true
			*/
			template <typename P1, typename P2, typename T, size_t D, size_t N>
			struct differences_loop
			{
				static inline bool close(const P1& p1, const P2& p2, const T& distance, T& sum)
				{
					typedef typename select_coordinate_type<P1, P2>::type PT;
					typedef typename select_type_traits<PT, T>::type T2;
					T d = std::abs(boost::numeric_cast<PT>(get<D>(p1)) - boost::numeric_cast<PT>(get<D>(p2)));
					if (d > distance)
					{
						return false;
					}
					sum += d * d;
					return differences_loop<P1, P2, T, D + 1, N>::close(p1, p2, distance, sum);
				}
			};

			template <typename P1, typename P2, typename T, size_t N>
			struct differences_loop<P1, P2, T, N, N>
			{
				static inline bool close(const P1&, const P2&, const T&, T&)
				{
					return true;
				}
			};


			template <typename S, typename P, typename T, size_t D, size_t N>
			struct outside_loop
			{
				static inline bool outside(const S& seg, const P& point, const T& distance)
				{
					typedef typename select_coordinate_type<S, P>::type PT;
					PT v = boost::numeric_cast<PT>(get<D>(point));

					PT s1 = get<0, D>(seg);
					PT s2 = get<1, D>(seg);
					// Out of reach if left/bottom or right/top of both points making up the segment
					// I know and currently accept that these comparisons/calculations are done twice per point
					if ((v < s1 - distance && v < s2 - distance) || (v > s1 + distance && v > s2 + distance))
					{
						return true;
					}
					return outside_loop<S, P, T, D + 1, N>::outside(seg, point, distance);
				}
			};

			template <typename S, typename P, typename T, size_t N>
			struct outside_loop<S, P, T, N, N>
			{
				static inline bool outside(const S&, const P&, const T&)
				{
					return false;
				}
			};


			template <typename P, typename SP, typename T>
			struct close_to_point
			{
				inline static bool calculate(const P& point, const P& selection_point, const T& search_radius)
				{
					assert_dimension_equal<P, SP>();
					static const size_t N = dimension<P>::value;
					T sum = 0;
					if (differences_loop<P, SP, T, 0, N>::close(point, selection_point, search_radius, sum))
					{
						return sum <= search_radius * search_radius;
					}
					return false;
				}
			};

			template <typename S, typename P, typename T>
			struct close_to_segment
			{
				inline static bool calculate(const S& seg, const P& selection_point, const T& search_radius)
				{
					assert_dimension_equal<S, P>();
					static const size_t N = dimension<P>::value;
					if (! outside_loop<S, P, T, 0, N>::outside(seg, selection_point, search_radius))
					{
						// Not outside, calculate dot product/square distance to segment.
						// Call corresponding strategy
						typedef typename strategy_distance_segment<
							typename cs_tag<P>::type,
							typename cs_tag<S>::type, P, S>::type STRATEGY;

						typedef typename STRATEGY::return_type R;
						STRATEGY strategy;
						R result = strategy(selection_point, seg);
						return result < search_radius;
					}

					return false;
				}
			};


			template <typename R, typename P, typename T>
			struct close_to_range
			{
				inline static bool calculate(const R& range, const P& selection_point, const T& search_radius)
				{
					assert_dimension_equal<R, P>();

					size_t n = boost::size(range);
					if (n == 0)
					{
						// Line with zero points, never close
						return false;
					}

					typedef typename point_type<R>::type PR;
					typedef typename boost::range_const_iterator<R>::type IT;
					IT it = boost::begin(range);

					if (n == 1)
					{
						// Line with one point ==> close to point
						return close_to_point<P, PR, T>::calculate(*it, selection_point, search_radius);
					}

					IT previous = it++;
					while(it != boost::end(range))
					{
						typedef segment<const PR> S;
						S s(*previous, *it);
						if (close_to_segment<S, P, T>::calculate(s, selection_point, search_radius))
						{
							return true;
						}
						previous = it++;
					}

					return false;
				}
			};


			template <typename TAG, typename G, typename P, typename T>
			struct use_within
			{
				inline static bool calculate(const G& geometry, const P& selection_point, const T& search_radius)
				{
					// Note the reversion, point-in-poly -> first point, then poly
					// Selected-at-point -> first geometry, then point
					return dispatch::within<point_tag, TAG, P, G>::calculate(selection_point, geometry);
				}
			};

		} // namespace selected
	} // namespace impl
	#endif


	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{

		/*!
			\tparam TD topological dimension
		 */
		template <typename TAG, typename G, size_t TD, typename P, typename T>
		struct selected
		{
		};

		template <typename P, typename SP, typename T>
		struct selected<point_tag, P, 0, SP, T> : impl::selected::close_to_point<P, SP, T> { };

		// SEGMENT, TODO HERE (close_to_segment)

		template <typename L, typename P, typename T>
		struct selected<linestring_tag, L, 1, P, T> : impl::selected::close_to_range<L, P, T> { };

		template <typename TAG, typename G, typename P, typename T>
		struct selected<TAG, G, 2, P, T> : impl::selected::use_within<TAG, G, P, T> { };

	} // namespace dispatch
	#endif


	/*!
		\brief Checks if one geometry is selected by a point lying within or in the neighborhood of that geometry
		\ingroup selected
		\tparam G type of geometry to check
		\tparam P type of point to check
		\tparam T type of search radius
		\param geometry geometry which might be located in the neighborhood
		\param selection_point point to select the geometry
		\param search_radius for points/linestrings: defines radius of "neighborhood" to find things in
		\return true if point is within or close to the other geometry

	 */
	template<typename G, typename P, typename T>
	inline bool selected(const G& geometry, const P& selection_point, const T& search_radius)
	{
		return dispatch::selected<typename tag<G>::type, G,
			topological_dimension<G>::value,
			P, T>::calculate(geometry, selection_point, search_radius);
	}


} // namespace geometry


#endif // _GEOMETRY_SELECTED_HPP
