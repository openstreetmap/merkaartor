// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_AREA_HPP
#define GGL_ALGORITHMS_AREA_HPP

#include <boost/concept/requires.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/algorithms/detail/calculate_null.hpp>
#include <ggl/core/exterior_ring.hpp>
#include <ggl/core/interior_rings.hpp>
#include <ggl/core/ring_type.hpp>
#include <ggl/core/concepts/point_concept.hpp>
#include <ggl/core/concepts/nsphere_concept.hpp>
#include <ggl/strategies/strategies.hpp>
#include <ggl/util/loop.hpp>
#include <ggl/util/math.hpp>

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
namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace area {

template<typename B>
inline typename coordinate_type<B>::type area_box(B const& b)
{
    // Currently only works for Cartesian boxes
    // Todo: make strategy
    // Todo: use concept
    assert_dimension<B, 2>();

    typedef typename coordinate_type<B>::type coordinate_type;
    coordinate_type const dx = get<max_corner, 0>(b) - get<min_corner, 0>(b);
    coordinate_type const dy = get<max_corner, 1>(b) - get<min_corner, 1>(b);

    return dx * dy;
}

template<typename C>
inline double area_circle(C const& c)
{
    // Currently only works for Cartesian circles
    // Todo: make strategy
    // Todo: use concept
    assert_dimension<C, 2>();

    typename radius_type<C>::type const r = get_radius<0>(c);
    return ggl::math::pi * r * r;
}

// Area of a linear linear_ring, assuming a closed linear_ring
template<typename R, typename S>
struct ring_area
{
    static inline double apply(R const& ring, S const& strategy)
    {
        assert_dimension<R, 2>();

        // A closed linear_ring has at least four points, if not there is no area
        if (boost::size(ring) >= 4)
        {
            typename S::state_type state_type;
            if (loop(ring, strategy, state_type))
            {
                return state_type.area();
            }
        }

        return 0;
    }
};

// Area of a polygon, either clockwise or anticlockwise
template<typename Polygon, typename Strategy>
struct polygon_area
{
    static inline double apply(const Polygon& poly, const Strategy& strategy)
    {
        assert_dimension<Polygon, 2>();

        typedef typename ring_type<Polygon>::type ring_type;
        typedef typename boost::range_const_iterator
            <
                typename interior_type<Polygon>::type
            >::type iterator_type;

        double a = std::abs(
            ring_area<ring_type, Strategy>::apply(exterior_ring(poly), strategy));

        for (iterator_type it = boost::begin(interior_rings(poly));
             it != boost::end(interior_rings(poly)); ++it)
        {
            a -= std::abs(ring_area<ring_type, Strategy>::apply(*it, strategy));
        }
        return a;
    }
};

}} // namespace detail::area

#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch {

template <typename Tag, typename G, typename S>
struct area : detail::calculate_null<double, G, S> {};


template <typename G, typename S>
struct area<box_tag, G, S>
{
    static inline double apply(const G& b, S const&)
    {
        return detail::area::area_box(b);
    }
};

template <typename G, typename S>
struct area<nsphere_tag, G, S>
{
    static inline double apply(const G& c, S const&)
    {
        return detail::area::area_circle(c);
    }
};

// Area of ring currently returns area of closed rings but it might be argued
// that it is 0.0, because a ring is just a line.
template <typename G, typename S>
struct area<ring_tag, G, S> : detail::area::ring_area<G, S> {};

template <typename G, typename S>
struct area<polygon_tag, G, S> : detail::area::polygon_area<G, S> {};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

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
template <typename Geometry>
inline double area(const Geometry& geometry)
{
    typedef typename point_type<Geometry>::type point_type;
    typedef typename strategy_area
        <
            typename cs_tag<point_type>::type,
            point_type
        >::type strategy_type;

    return dispatch::area
        <
            typename tag<Geometry>::type,
            Geometry,
            strategy_type
        >::apply(geometry, strategy_type());
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
template <typename Geometry, typename Strategy>
inline double area(const Geometry& geometry, const Strategy& strategy)
{
    return dispatch::area
        <
            typename tag<Geometry>::type,
            Geometry,
            Strategy
        >::apply(geometry, strategy);
}

} // namespace ggl

#endif // GGL_ALGORITHMS_AREA_HPP
