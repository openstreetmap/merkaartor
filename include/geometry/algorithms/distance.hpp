// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_DISTANCE_HPP
#define _GEOMETRY_DISTANCE_HPP

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <geometry/core/cs.hpp>

#include <geometry/geometries/segment.hpp>

#include <geometry/strategies/strategies.hpp>

#include <geometry/strategies/distance_result.hpp>
#include <geometry/util/promotion_traits.hpp>


/*!
\defgroup distance distance calculation
The distance algorithm returns the distance between two geometries.
\par Coordinate systems and strategies:
With help of strategies the distance function returns the appropriate distance.
If the input is in cartesian coordinates, the Euclidian distance (Pythagoras) is calculated.
If the input is in spherical coordinates (either degree or radian), the distance over the sphere is returned.
If the input is in geographic coordinates, distance is calculated over the globe and returned in meters.

\par Distance result:
Depending on calculation type the distance result is either a structure, convertable
to a double, or a double value. In case of Pythagoras it makes sense to not draw the square root in the
strategy itself. Taking a square root is relative expensive and is not necessary when comparing distances.

\par Geometries:
Currently implemented, for both cartesian and spherical/geographic:
- POINT - POINT
- POINT - SEGMENT and v.v.
- POINT - LINESTRING and v.v.

Not yet implemented:
- POINT - RING etc, note that it will return a zero if the point is anywhere within the ring

\par Example:
Example showing distance calculation of two points, in xy and in latlong coordinates
\dontinclude doxygen_examples.cpp
\skip example_distance_point_point
\line {
\until }
*/

namespace geometry
{

	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		namespace distance
		{
			template <typename P1, typename P2, typename S>
			inline typename S::return_type point_to_point(const P1& p1, const P2& p2, const S& strategy)
			{
				return strategy(p1, p2);
			}

			template<typename P, typename SEG, typename STR>
			inline typename STR::return_type point_to_segment(const P& point, const SEG& segment, const STR& strategy)
			{
				return strategy(point, segment);
			}

			template<typename P, typename SEG>
			inline typename distance_result<P, SEG>::type point_to_segment(const P& point, const SEG& segment)
			{
				typename strategy_distance_segment<
									typename cs_tag<P>::type,
									typename cs_tag<SEG>::type,
									P, SEG>::type strategy;
				return strategy(point, segment);
			}

			template<typename P, typename L, typename S>
			inline typename S::return_type point_to_linestring(const P& point, const L& linestring, const S& strategy)
			{
				typedef typename select_coordinate_type<P, typename boost::range_value<L>::type>::type T;
				typedef typename S::return_type RET;

				if (boost::begin(linestring) == boost::end(linestring))
				{
					return RET(0);
				}

				// line of one point: return point square_distance
				typedef typename boost::range_const_iterator<L>::type IT;
				IT it = boost::begin(linestring);
				IT prev = it++;
				if (it == boost::end(linestring))
				{
					typename S::distance_strategy_type pp;
					return pp(point, *boost::begin(linestring));
				}

				typedef segment<const typename point_type<L>::type> CS;

				// start with first segment distance
				S f2;
				RET d = f2(point, CS(*prev, *it));

				// check if other segments are closer
				prev = it++;
				while(it != boost::end(linestring))
				{
					RET ds = f2(point, CS(*prev, *it));
					if (close_to_zero(ds))
					{
						return RET(0);
					}
					else if (ds < d)
					{
						d = ds;
					}
					prev = it++;
				}
				return d;
			}

			template<typename P, typename L>
			inline typename distance_result<P, L>::type point_to_linestring(const P& point, const L& linestring)
			{
				typedef typename point_type<L>::type LP;
				typedef segment<const LP> SEG; // helper geometry
				typedef typename strategy_distance_segment<
									typename cs_tag<P>::type,
									typename cs_tag<LP>::type,
									P, SEG>::type STRATEGY;

				STRATEGY strategy;
				return point_to_linestring(point, linestring, strategy);
			}



		} // namespace distance

	} // namespace impl
	#endif


	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{

		template <typename TAG1, typename TAG2, typename G1, typename G2>
		struct distance
		{
		};

		template <typename P1, typename P2>
		struct distance<point_tag, point_tag, P1, P2>
		{
			template <typename S>
			static inline typename S::return_type calculate(const P1& p1, const P2& p2, const S& strategy)
			{
				return impl::distance::point_to_point(p1, p2, strategy);
			}

			static inline typename distance_result<P1, P2>::type
						calculate(const P1& p1, const P2& p2)
			{
				return impl::distance::point_to_point(p1, p2,
						typename strategy_distance<
							typename cs_tag<P1>::type,
							typename cs_tag<P2>::type,
							P1, P2>::type());
			}
		};

		template <typename P, typename L>
		struct distance<point_tag, linestring_tag, P, L>
		{
			template<typename S>
			static inline typename S::return_type calculate(const P& point, const L& linestring, const S& strategy)
			{
				return impl::distance::point_to_linestring(point, linestring, strategy);
			}
			static inline typename distance_result<P, L>::type calculate(const P& point, const L& linestring)
			{
				return impl::distance::point_to_linestring(point, linestring);
			}
		};

		template <typename L, typename P>
		struct distance<linestring_tag, point_tag, L, P>
		{
			template<typename S>
			static inline typename S::return_type calculate(const L& linestring, const P& point, const S& strategy)
			{
				return impl::distance::point_to_linestring(point, linestring, strategy);
			}
			static inline typename distance_result<P, L>::type calculate(const L& linestring, const P& point)
			{
				return impl::distance::point_to_linestring(point, linestring);
			}
		};

		template <typename P, typename SEG>
		struct distance<point_tag, segment_tag, P, SEG>
		{
			template<typename STR>
			static inline typename STR::return_type calculate(const P& point, const SEG& segment, const STR& strategy)
			{
				return impl::distance::point_to_segment(point, segment, strategy);
			}
			static inline typename distance_result<P, SEG>::type calculate(const P& point, const SEG& segment)
			{
				return impl::distance::point_to_segment(point, segment);
			}
		};

		template <typename SEG, typename P>
		struct distance<segment_tag, point_tag, SEG, P>
		{
			template<typename STR>
			static inline typename STR::return_type calculate(const SEG& segment, const P& point, const STR& strategy)
			{
				return impl::distance::point_to_segment(point, segment, strategy);
			}
			static inline typename distance_result<P, SEG>::type calculate(const SEG& segment, const P& point)
			{
				return impl::distance::point_to_segment(point, segment);
			}
		};


	} // namespace dispatch
	#endif


	/*!
		\brief Calculate distance between two geometries
		\ingroup distance
		\details The default strategy is used, belonging to the corresponding coordinate system of the geometries
		\tparam G1 first geometry type
		\tparam G2 second geometry type
		\param geometry1 first geometry
		\param geometry2 second geometry
		\return the distance (either a double or a distance result, convertable to double)
	 */
	template <typename G1, typename G2>
	inline typename distance_result<G1, G2>::type
		distance(const G1& geometry1, const G2& geometry2)
	{
		return dispatch::distance<typename tag<G1>::type,
				typename tag<G2>::type, G1, G2>::calculate(geometry1, geometry2);
	}

	/*!
		\brief Calculate distance between two geometries with a specified strategy
		\ingroup distance
		\tparam G1 first geometry type
		\tparam G2 second geometry type
		\tparam S point-point-distance strategy type
		\param geometry1 first geometry
		\param geometry2 second geometry
		\param strategy strategy to calculate distance between two points
		\return the distance (either a double or a distance result, convertable to double)
		\par Example:
		Example showing distance calculation of two lat long points, using the accurate Vincenty approximation
		\dontinclude doxygen_examples.cpp
		\skip example_distance_point_point_strategy
		\line {
		\until }
	 */
	template <typename G1, typename G2, typename S>
	inline typename S::return_type distance(const G1& geometry1, const G2& geometry2, const S& strategy)
	{
		return dispatch::distance<typename tag<G1>::type,
				typename tag<G2>::type, G1, G2>::calculate(geometry1, geometry2, strategy);
	}


} // namespace geometry


#endif // _GEOMETRY_DISTANCE_HPP
