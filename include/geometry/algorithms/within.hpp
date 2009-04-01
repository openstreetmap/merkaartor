// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_WITHIN_HPP
#define _GEOMETRY_WITHIN_HPP

#include <boost/concept/requires.hpp>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <geometry/core/access.hpp>
#include <geometry/core/exterior_ring.hpp>
#include <geometry/core/interior_rings.hpp>

#include <geometry/core/concepts/point_concept.hpp>
#include <geometry/core/concepts/nsphere_concept.hpp>

#include <geometry/core/cs.hpp>

#include <geometry/algorithms/distance.hpp>
#include <geometry/algorithms/make.hpp>

#include <geometry/util/loop.hpp>


#include <geometry/strategies/strategies.hpp>


/*!
\defgroup within within: examine if one geometry is within another geometry (a.o. point in polygon)

\par Source descriptions:
- OGC: Returns 1 (TRUE) if this geometric object is "spatially within" another Geometry.

\par Performance
2776 within determinations using bounding box and polygon are done in 0.09 seconds (other libraries: 0.14 seconds, 3.0 seconds, 3.8)

\par Example:
The within algorithm is used as following:
\dontinclude doxygen_examples.cpp
\skip example_within
\line {
\until }
\par Geometries:
- POINT + POLYGON: The well-known point-in-polygon, returning true if a point falls within a polygon (and not
	within one of its holes) \image html within_polygon.png
- POINT + RING: returns true if point is completely within a ring \image html within_ring.png
*/


namespace geometry
{
	/*!
		\ingroup impl
	 */
	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		namespace within
		{

			// Within, should return true if one geometry falls completely within another geometry
			// Geometries can lie in something with an area, so in an box, circle, linear_ring,polygon

			//-------------------------------------------------------------------------------------------------------
			// Implementation for boxes. Supports boxes in 2 or 3 dimensions, in Euclidian system
			// Todo: implement as strategy
			//-------------------------------------------------------------------------------------------------------
			template <typename P, typename B, size_t D, size_t N>
			struct point_in_box
			{
				static bool inside(const P& p, const B& b)
				{
					if (get<D>(p) <= get<min_corner, D>(b) || get<D>(p) >= get<max_corner, D>(b))
					{
						return false;
					}

					return point_in_box<P, B, D + 1, N>::inside(p, b);
				}
			};

			template <typename P, typename B, size_t N>
			struct point_in_box<P, B, N, N>
			{
				static bool inside(const P& /* p */, const B& /* b */)
				{
					return true;
				}
			};


			//-------------------------------------------------------------------------------------------------------
			// Implementation for n-spheres. Supports circles or spheres, in 2 or 3 dimensions, in Euclidian system
			// Circle center might be of other point-type as geometry
			// Todo: implement as strategy
			//-------------------------------------------------------------------------------------------------------
			template<typename P, typename C>
			inline bool point_in_circle(const P& p, const C& c)
			{
				assert_dimension<C, 2>();
				typedef typename point_type<C>::type PC;

				typedef typename strategy_distance<typename cs_tag<P>::type,
							typename cs_tag<PC>::type,
							P, PC>::type S;
				typedef typename S::return_type RET;

				P center = geometry::make<P>(get<0>(c), get<1>(c));
				S distance;
				RET r = distance(p, center);
				RET rad = make_distance_result<RET>(get_radius<0>(c));
				return r < rad;
			}
			/// 2D version
			template<typename T, typename C>
			inline bool point_in_circle(const T& coor1, const T& coor2, const C& c)
			{
				typedef typename point_type<C>::type P;
				P p = geometry::make<P>(coor1, coor2);
				return point_in_circle(p, c);
			}


			template<typename B, typename C>
			inline bool box_in_circle(const B& b, const C& c)
			{
				typedef typename point_type<B>::type P;

				// Currently only implemented for 2d geometries
				assert_dimension<P, 2>();
				assert_dimension<C, 2>();

				// Box: all four points must lie within circle

				// Check points lower-left and upper-right, then lower-right and upper-left
				return point_in_circle(get<min_corner, 0>(b), get<min_corner, 1>(b), c)
					&& point_in_circle(get<max_corner, 0>(b), get<max_corner, 1>(b), c)
					&& point_in_circle(get<min_corner, 0>(b), get<max_corner, 1>(b), c)
					&& point_in_circle(get<max_corner, 0>(b), get<min_corner, 1>(b), c);
			}


			// Generic "range-in-circle", true if all points within circle
			template<typename R, typename C>
			inline bool range_in_circle(const R& range, const C& c)
			{
				assert_dimension<R, 2>();
				assert_dimension<C, 2>();
				for (typename boost::range_const_iterator<R>::type it = boost::begin(range);
					it != boost::end(range); it++)
				{
					if (! point_in_circle(*it, c))
					{
						return false;
					}
				}
				return true;
			}

			template<typename Y, typename C>
			inline bool polygon_in_circle(const Y& poly, const C& c)
			{
				return range_in_circle(exterior_ring(poly), c);
			}




			template<typename P, typename R, typename S>
			inline bool point_in_ring(const P& p, const R& r, const S& strategy)
			{
				if (boost::size(r) < 4)
				{
					return false;
				}

				typename S::state_type state(p);
				if (loop(r, strategy, state))
				{
					return state.within();
				}
				return false;
			}

			// Polygon: in exterior ring, and if so, not within interior ring(s)
			template<typename P, typename Y, typename S>
			inline bool point_in_polygon(const P& p, const Y& poly, const S& strategy)
			{
				if (point_in_ring(p, exterior_ring(poly), strategy))
				{
					typedef typename boost::range_const_iterator<typename interior_type<Y>::type>::type IT;
					for (IT i = boost::begin(interior_rings(poly)); i != boost::end(interior_rings(poly)); i++)
					{
						if (point_in_ring(p, *i, strategy))
						{
							return false;
						}
					}
					return true;
				}
				return false;
			}






		} // namespace within
	} // namespace impl
	#endif


	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{

		template <typename TAG1, typename TAG2, typename G1, typename G2>
		struct within {};


		template <typename P, typename B>
		struct within<point_tag, box_tag, P, B>
		{
			inline static bool calculate(const P& p, const B& b)
			{
				assert_dimension_equal<P, B>();
				return impl::within::point_in_box<P, B,
						0, dimension<P>::value>::inside(p, b);
			}
		};

		template <typename P, typename C>
		struct within<point_tag, nsphere_tag, P, C>
		{
			inline static bool calculate(const P& p, const C& c)
			{
				return impl::within::point_in_circle(p, c);
			}
		};

		template <typename B, typename C>
		struct within<box_tag, nsphere_tag, B, C>
		{
			inline static bool calculate(const B& b, const C& c)
			{
				return impl::within::box_in_circle(b, c);
			}
		};

		template <typename R, typename C>
		struct within<linestring_tag, nsphere_tag, R, C>
		{
			inline static bool calculate(const R& ln, const C& c)
			{
				return impl::within::range_in_circle(ln, c);
			}
		};

		template <typename R, typename C>
		struct within<ring_tag, nsphere_tag, R, C>
		{
			inline static bool calculate(const R& r, const C& c)
			{
				return impl::within::range_in_circle(r, c);
			}
		};

		template <typename Y, typename C>
		struct within<polygon_tag, nsphere_tag, Y, C>
		{
			inline static bool calculate(const Y& poly, const C& c)
			{
				return impl::within::polygon_in_circle(poly, c);
			}
		};

		template <typename P, typename R>
		struct within<point_tag, ring_tag, P, R>
		{
			inline static bool calculate(const P& p, const R& r)
			{
				typedef typename boost::range_value<R>::type PS;
				return impl::within::point_in_ring(p, r,
						typename strategy_within<
							typename cs_tag<P>::type, typename cs_tag<PS>::type,
									P, PS>::type());
			}
		};

		template <typename P, typename Y>
		struct within<point_tag, polygon_tag, P, Y>
		{
			inline static bool calculate(const P& point, const Y& poly)
			{
				typedef typename point_type<Y>::type PS;
				return impl::within::point_in_polygon(point, poly,
						typename strategy_within<
							typename cs_tag<P>::type, typename cs_tag<PS>::type,
									P, PS>::type());
			}

			template<typename S>
			inline static bool calculate(const P& point, const Y& poly, const S& strategy)
			{
				return impl::within::point_in_polygon(point, poly, strategy);
			}
		};


	} // namespace dispatch
	#endif


	/*!
		\brief Within check
		\ingroup within
		\param geometry1 geometry which might be within the second geometry
		\param geometry2 geometry which might contain the first geometry
		\return true if geometry1 is completely contained within geometry2, else false
		\note The default strategy is used for within detection
	 */
	template<typename G1, typename G2>
	inline bool within(const G1& geometry1, const G2& geometry2)
	{
		return dispatch::within<typename tag<G1>::type,
					typename tag<G2>::type, G1, G2>::calculate(geometry1, geometry2);
	}

	/*!
		\brief Within check using a strategy
		\ingroup within
		\param geometry1 geometry which might be within the second geometry
		\param geometry2 geometry which might contain the first geometry
		\param strategy strategy to be used
		\return true if geometry1 is completely contained within geometry2, else false
	 */
	template<typename G1, typename G2, typename S>
	inline bool within(const G1& geometry1, const G2& geometry2, const S& strategy)
	{
		return dispatch::within<typename tag<G1>::type,
					typename tag<G2>::type, G1, G2>::calculate(geometry1, geometry2, strategy);
	}

} // namespace geometry


#endif // _GEOMETRY_WITHIN_HPP
