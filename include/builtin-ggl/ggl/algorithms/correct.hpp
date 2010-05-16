// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_CORRECT_HPP
#define GGL_ALGORITHMS_CORRECT_HPP

#include <algorithm>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/core/cs.hpp>
#include <ggl/core/ring_type.hpp>
#include <ggl/core/exterior_ring.hpp>
#include <ggl/core/interior_rings.hpp>

#include <ggl/core/concepts/point_concept.hpp>

#include <ggl/algorithms/area.hpp>
#include <ggl/algorithms/disjoint.hpp>


namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace correct {

// correct an box: make min/max are correct
template <typename B>
inline void correct_box(B& b)
{
    // Currently only for Cartesian coordinates
    // TODO: adapt using strategies
    // TODO: adapt using traits
    typedef typename coordinate_type<B>::type coordinate_type;

    if (get<min_corner, 0>(b) > get<max_corner, 0>(b))
    {
        coordinate_type max_value = get<min_corner, 0>(b);
        coordinate_type min_value = get<max_corner, 0>(b);
        set<min_corner, 0>(b, min_value);
        set<max_corner, 0>(b, max_value);
    }

    if (get<min_corner, 1>(b) > get<max_corner, 1>(b))
    {
        coordinate_type max_value = get<min_corner, 1>(b);
        coordinate_type min_value = get<max_corner, 1>(b);
        set<min_corner, 1>(b, min_value);
        set<max_corner, 1>(b, max_value);
    }
}

// close a linear_ring, if not closed
template <typename R>
inline void ensure_closed_ring(R& r)
{
    if (boost::size(r) > 2)
    {
        // check if closed, if not, close it
        if (ggl::disjoint(r.front(), r.back()))
        {
            r.push_back(r.front());
        }
    }
}

// correct a polygon: normalizes all rings, sets outer linear_ring clockwise, sets all
// inner rings counter clockwise
template <typename Y>
inline void correct_polygon(Y& poly)
{
    typename ring_type<Y>::type& outer = exterior_ring(poly);
    ensure_closed_ring(outer);

    typedef typename point_type<Y>::type point_type;
    typedef typename ring_type<Y>::type ring_type;
    typedef typename strategy_area
        <
            typename cs_tag<point_type>::type,
            point_type
        >::type strategy_type;

    strategy_type strategy;

    if (detail::area::ring_area<ring_type, strategy_type>::apply(outer, strategy) < 0)
    {
        std::reverse(boost::begin(outer), boost::end(outer));
    }

    typedef typename boost::range_iterator
        <
            typename interior_type<Y>::type
        >::type iterator_type;

    for (iterator_type it = boost::begin(interior_rings(poly));
         it != boost::end(interior_rings(poly)); ++it)
    {
        ensure_closed_ring(*it);
        if (detail::area::ring_area<ring_type, strategy_type>::apply(*it, strategy) > 0)
        {
            std::reverse(boost::begin(*it), boost::end(*it));
        }
    }
}

}} // namespace detail::correct
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Tag, typename G>
struct correct {};

template <typename B>
struct correct<box_tag, B>
{
    static inline void apply(B& box)
    {
        detail::correct::correct_box(box);
    }
};

template <typename R>
struct correct<ring_tag, R>
{
    static inline void apply(R& ring)
    {
        detail::correct::ensure_closed_ring(ring);
    }
};

template <typename P>
struct correct<polygon_tag, P>
{
    static inline void apply(P& poly)
    {
        detail::correct::correct_polygon(poly);
    }
};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

template <typename G>
inline void correct(G& geometry)
{
    dispatch::correct<typename tag<G>::type, G>::apply(geometry);
}

} // namespace ggl

#endif // GGL_ALGORITHMS_CORRECT_HPP
