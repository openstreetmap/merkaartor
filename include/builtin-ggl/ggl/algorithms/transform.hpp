// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_TRANSFORM_HPP
#define GGL_ALGORITHMS_TRANSFORM_HPP

#include <cmath>
#include <iterator>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/algorithms/clear.hpp>
#include <ggl/core/cs.hpp>
#include <ggl/core/exterior_ring.hpp>
#include <ggl/core/interior_rings.hpp>
#include <ggl/core/ring_type.hpp>
#include <ggl/geometries/segment.hpp>
#include <ggl/strategies/strategies.hpp>


/*!
\defgroup transform transformations
\brief Transforms from one geometry to another geometry, optionally using a strategy
\details The transform algorithm automatically transforms from one coordinate system to another coordinate system.
If the coordinate system of both geometries are the same, the geometry is copied. All point(s of the geometry)
are transformed.

There is a version without a strategy, transforming automatically, and there is a version with a strategy.

This function has a lot of appliances, for example
- transform from spherical coordinates to cartesian coordinates, and back
- transform from geographic coordinates to cartesian coordinates (projections) and back
- transform from degree to radian, and back
- transform from and to cartesian coordinates (mapping, translations, etc)

The automatic transformations look to the coordinate system family, and dimensions, of the point type and by this
apply the strategy (internally bounded by traits classes).

\par Examples:
The example below shows automatic transformations to go from one coordinate system to another one:
\dontinclude doxygen_2.cpp
\skip example_for_transform()
\skipline XYZ
\until endl;

The next example takes another approach and transforms from Cartesian to Cartesian:
\skipline XY
\until endl;

\note Not every possibility is yet worked out, e.g. polar coordinate system is ignored until now
\note This "transform" is broader then geodetic datum transformations, those are currently not worked out

*/

namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace transform {

template <typename P1, typename P2>
struct transform_point
{
    template <typename S>
    static inline bool apply(P1 const& p1, P2& p2, S const& strategy)
    {
        return strategy(p1, p2);
    }
};

template <typename B1, typename B2>
struct transform_box
{
    template <typename S>
    static inline bool apply(B1 const& b1, B2& b2, S const& strategy)
    {
        typedef typename point_type<B1>::type point_type1;
        typedef typename point_type<B2>::type point_type2;

        point_type1 lower_left, upper_right;
        assign_box_corner<min_corner, min_corner>(b1, lower_left);
        assign_box_corner<max_corner, max_corner>(b1, upper_right);

        point_type2 p1, p2;
        if (strategy(lower_left, p1) && strategy(upper_right, p2))
        {
            // Create a valid box and therefore swap if necessary
            typedef typename coordinate_type<point_type2>::type coordinate_type;
            coordinate_type x1 = ggl::get<0>(p1)
                    , y1  = ggl::get<1>(p1)
                    , x2  = ggl::get<0>(p2)
                    , y2  = ggl::get<1>(p2);

            if (x1 > x2) { std::swap(x1, x2); }
            if (y1 > y2) { std::swap(y1, y2); }

            set<min_corner, 0>(b2, x1);
            set<min_corner, 1>(b2, y1);
            set<max_corner, 0>(b2, x2);
            set<max_corner, 1>(b2, y2);

            return true;
        }
        return false;
    }
};

template <typename P, typename OutputIterator, typename R, typename S>
inline bool transform_range_out(R const& range, OutputIterator out, S const& strategy)
{
    P point_out;
    for(typename boost::range_const_iterator<R>::type it = boost::begin(range);
        it != boost::end(range); ++it)
    {
        if (! transform_point<typename point_type<R>::type, P>::apply(*it, point_out, strategy))
        {
            return false;
        }
        *out = point_out;
        ++out;
    }
    return true;
}

template <typename P1, typename P2>
struct transform_polygon
{
    template <typename S>
    static inline bool apply(const P1& poly1, P2& poly2, S const& strategy)
    {
        typedef typename interior_type<P1>::type interior1_type;
        typedef typename interior_type<P2>::type interior2_type;
        typedef typename ring_type<P1>::type ring1_type;
        typedef typename ring_type<P2>::type ring2_type;
        typedef typename point_type<P2>::type point2_type;

        ggl::clear(poly2);

        if (!transform_range_out<point2_type>(exterior_ring(poly1),
                    std::back_inserter(exterior_ring(poly2)), strategy))
        {
            return false;
        }

        interior_rings(poly2).resize(boost::size(interior_rings(poly1)));

        typedef typename boost::range_const_iterator<interior1_type>::type iterator1_type;
        typedef typename boost::range_iterator<interior2_type>::type iterator2_type;

        iterator1_type it1 = boost::begin(interior_rings(poly1));
        iterator2_type it2 = boost::begin(interior_rings(poly2));
        for ( ; it1 != boost::end(interior_rings(poly1)); ++it1, ++it2)
        {
            if (!transform_range_out<point2_type>(*it1, std::back_inserter(*it2), strategy))
            {
                return false;
            }
        }

        return true;
    }
};


template <typename P1, typename P2>
struct select_strategy
{
    typedef typename strategy_transform
        <
        typename cs_tag<P1>::type,
        typename cs_tag<P2>::type,
        typename coordinate_system<P1>::type,
        typename coordinate_system<P2>::type,
        dimension<P1>::type::value,
        dimension<P2>::type::value,
        typename point_type<P1>::type,
        typename point_type<P2>::type
        >::type type;
};

template <typename R1, typename R2>
struct transform_range
{
    template <typename S>
    static inline bool apply(R1 const& range1, R2& range2, S const& strategy)
    {
        typedef typename point_type<R2>::type point_type;

        ggl::clear(range2);
        return transform_range_out<point_type>(range1, std::back_inserter(range2), strategy);
    }
};

}} // namespace detail::transform
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Tag1, typename Tag2, typename G1, typename G2>
struct transform {};

template <typename P1, typename P2>
struct transform<point_tag, point_tag, P1, P2>
    : detail::transform::transform_point<P1, P2>
{
};


template <typename L1, typename L2>
struct transform<linestring_tag, linestring_tag, L1, L2>
    : detail::transform::transform_range<L1, L2>
{
};

template <typename R1, typename R2>
struct transform<ring_tag, ring_tag, R1, R2>
    : detail::transform::transform_range<R1, R2>
{
};

template <typename P1, typename P2>
struct transform<polygon_tag, polygon_tag, P1, P2>
    : detail::transform::transform_polygon<P1, P2>
{
};

template <typename B1, typename B2>
struct transform<box_tag, box_tag, B1, B2>
    : detail::transform::transform_box<B1, B2>
{
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Transforms from one geometry to another geometry using a strategy
    \ingroup transform
    \tparam G1 first geometry type
    \tparam G2 second geometry type
    \tparam S strategy
    \param geometry1 first geometry
    \param geometry2 second geometry
    \param strategy the strategy to be used for transformation
 */
template <typename G1, typename G2, typename S>
inline bool transform(G1 const& geometry1, G2& geometry2, S const& strategy)
{
    typedef dispatch::transform
        <
        typename tag<G1>::type,
        typename tag<G2>::type,
        G1,
        G2
        > transform_type;

    return transform_type::apply(geometry1, geometry2, strategy);
}

/*!
    \brief Transforms from one geometry to another geometry using a strategy
    \ingroup transform
    \tparam G1 first geometry type
    \tparam G2 second geometry type
    \param geometry1 first geometry
    \param geometry2 second geometry
    \return true if the transformation could be done
 */
template <typename G1, typename G2>
inline bool transform(G1 const& geometry1, G2& geometry2)
{
    typename detail::transform::select_strategy<G1, G2>::type strategy;
    return transform(geometry1, geometry2, strategy);
}

} // namespace ggl

#endif // GGL_ALGORITHMS_TRANSFORM_HPP
