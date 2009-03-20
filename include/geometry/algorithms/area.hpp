// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_AREA_HPP
#define _GEOMETRY_AREA_HPP

#include <boost/concept/requires.hpp>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <geometry/core/ring_type.hpp>
#include <geometry/core/exterior_ring.hpp>
#include <geometry/core/interior_rings.hpp>

#include <geometry/core/concepts/point_concept.hpp>
#include <geometry/core/concepts/nsphere_concept.hpp>


#include <geometry/strategies/strategies.hpp>

#include <geometry/util/loop.hpp>
#include <geometry/util/math.hpp>

/*!
\defgroup area area calculation

\par Performance
2776 * 1000 area calculations are done in 0.11 seconds (other libraries: 0.125 seconds, 0.125 seconds, 0.5 seconds)

\par Coordinate systems and strategies
Area calculation can be done in Cartesian and in spherical/geographic coordinate systems.

\par Geometries
The area algorithm calculates the surface area of all geometries having a surface:
box, circle, polygon, multi_polygon. The units are the square of the units used for the points
defining the surface. If the polygon is defined in meters, the area is in square meters.

\par Example:
Example showing area calculation of polygons built of xy-points and of latlong-points
\dontinclude doxygen_examples.cpp
\skip example_area_polygon()
\line {
\until }

*/
namespace geometry
{
	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		namespace area
		{
			template<typename B>
			inline typename coordinate_type<B>::type area_box(const B& b)
			{
				// Currently only works for Cartesian boxes
				// Todo: make strategy
				// Todo: use concept
				assert_dimension<B, 2>();
				typedef typename coordinate_type<B>::type T;
				T dx = get<max_corner, 0>(b) - get<min_corner, 0>(b);
				T dy = get<max_corner, 1>(b) - get<min_corner, 1>(b);
				return dx * dy;
			}

			template<typename C>
			inline double area_circle(const C& c)
			{
				// Currently only works for Cartesian circles
				// Todo: make strategy
				// Todo: use concept
				assert_dimension<C, 2>();
				typename radius_type<C>::type r = get_radius<0>(c);
				return geometry::math::pi * r * r;
			}

			// Area of a linear linear_ring, assuming a closed linear_ring
			template<typename R, typename S>
			inline double area_ring(const R& r, const S& strategy)
			{
				assert_dimension<R, 2>();

				// A closed linear_ring has at least four points, if not there is no area
				if (boost::size(r) >= 4)
				{
					typename S::state_type state;
					if (loop(r, strategy, state))
					{
						return state.area();
					}
				}

				return 0;
			}

			// Area of a polygon, assuing a closed clockwise polygon (with holes counter clockwise)
			template<typename Y, typename S>
			inline double area_polygon(const Y& poly, const S& strategy)
			{
				assert_dimension<Y, 2>();

				typedef typename boost::range_const_iterator<typename interior_type<Y>::type>::type IT;

				double a = fabs(area_ring(exterior_ring(poly), strategy));

				for (IT i = boost::begin(interior_rings(poly)); i != boost::end(interior_rings(poly)); i++)
				{
					a -= fabs(area_ring(*i, strategy));
				}
				return a;
			}


		} // namespace area
	} // namespace impl
	#endif


	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{
		template <typename TAG, typename G>
		struct area {};


		template <typename G>
		struct area<box_tag, G>
		{
			inline static double calculate(const G& b)
			{
				return impl::area::area_box(b);
			}
		};


		template <typename G>
		struct area<nsphere_tag, G>
		{
			inline static double calculate(const G& c)
			{
				return impl::area::area_circle(c);
			}
		};

		template <typename G>
		struct area<ring_tag, G>
		{

			inline static double calculate(const G& ring)
			{
				typedef typename boost::range_value<G>::type P;

				return impl::area::area_ring(ring,
					typename strategy_area<typename cs_tag<P>::type, P>::type());
			}

			template <typename S>
			inline static double calculate(const G& ring, const S& strategy)
			{
				return impl::area::area_ring(ring, strategy);
			}
		};

		// Specialization for polygons:
		template <typename G>
		struct area<polygon_tag, G>
		{
			inline static double calculate(const G& poly)
			{
				typedef typename point_type<G>::type P;

				return impl::area::area_polygon(poly, typename strategy_area<
						typename cs_tag<P>::type, P>::type());
			}
			template <typename S>
			inline static double calculate(const G& poly, const S& strategy)
			{
				return impl::area::area_polygon(poly, strategy);
			}
		};


	} // namespace dispatch
	#endif


	/*!
		\brief Calculate area of a geometry
		\ingroup area
		\details The function area returns the area of a polygon, ring, box or circle,
		using the default area-calculation strategy. Strategies are
		provided for cartesian ans spherical points
		The geometries should correct, polygons should be closed and orientated clockwise, holes,
		if any, must be orientated counter clockwise
		\param geometry a geometry
		\return the area
	 */
	template <typename G>
	inline double area(const G& geometry)
	{
		return dispatch::area<typename tag<G>::type, G>::calculate(geometry);
	}

	/*!
		\brief Calculate area of a geometry using a strategy
		\ingroup area
		\details This version of area calculation takes a strategy
		\param geometry a geometry
		\param strategy the strategy to calculate area. Especially for spherical areas there are
			some approaches.
		\return the area
	 */
	template <typename G, typename S>
	inline double area(const G& geometry, const S& strategy)
	{
		return dispatch::area<typename tag<G>::type, G>::calculate(geometry, strategy);
	}


} // namespace geometry


#endif // _GEOMETRY_AREA_HPP
