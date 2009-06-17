// Generic Geometry Library
//
// Boost.SpatialIndex - geometry helper functions
//
// Copyright 2008 Federico J. Fernandez.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_GGL_INDEX_RTREE_HELPERS_HPP
#define GGL_GGL_INDEX_RTREE_HELPERS_HPP

#include <ggl/algorithms/area.hpp>
#include <ggl/core/point_type.hpp>

namespace ggl { namespace index { namespace rtree {

/**
 * \brief Given two boxes, returns the minimal box that contains them
 */
// TODO: use ggl::combine
template <typename Box>
inline Box enlarge_box(Box const& b1, Box const& b2)
{
    // TODO: mloskot - Refactor to readable form. Fix VC++8.0 min/max warnings:
    //  warning C4002: too many actual parameters for macro 'min

    typedef typename ggl::point_type<Box>::type point_type;

    point_type pmin(
        ggl::get<min_corner, 0>(b1) < ggl::get<min_corner, 0>(b2)
            ? ggl::get<min_corner, 0>(b1) : ggl::get<min_corner, 0>(b2),
        ggl::get<min_corner, 1>(b1) < ggl::get<min_corner, 1>(b2)
            ? ggl::get<min_corner, 1>(b1) : ggl::get<min_corner, 1>(b2));

    point_type pmax(
        ggl::get<max_corner, 0>(b1) > ggl::get<max_corner, 0>(b2)
            ? ggl::get<max_corner, 0>(b1) : ggl::get<max_corner, 0>(b2),
        ggl::get<max_corner, 1>(b1) > ggl::get<max_corner, 1>(b2)
            ? ggl::get<max_corner, 1>(b1) : ggl::get<max_corner, 1>(b2));

    return Box(pmin, pmax);
}

/**
 * \brief Compute the area of the union of b1 and b2
 */
template <typename Box>
inline double compute_union_area(Box const& b1, Box const& b2)
{
    Box enlarged_box = enlarge_box(b1, b2);
    return ggl::area(enlarged_box);
}

/**
 * \brief Checks if boxes overlap
 */
// TODO: combine with ggl::overlaps but NOTE:
// this definition is slightly different from GGL-definition
template <typename Box>
inline bool is_overlapping(Box const& b1, Box const& b2)
{
    bool overlaps_x = false;
    bool overlaps_y = false;

    if (ggl::get<min_corner, 0>(b1) >= ggl::get<min_corner, 0>(b2)
        && ggl::get<min_corner, 0>(b1) <= ggl::get<max_corner, 0>(b2))
    {
        overlaps_x = true;
    }

    if (ggl::get<max_corner, 0>(b1) >= ggl::get<min_corner, 0>(b2)
        && ggl::get<min_corner, 0>(b1) <= ggl::get<max_corner, 0>(b2))
    {
        overlaps_x = true;
    }

    if (ggl::get<min_corner, 1>(b1) >= ggl::get<min_corner, 1>(b2)
        && ggl::get<min_corner, 1>(b1) <= ggl::get<max_corner, 1>(b2))
    {
        overlaps_y = true;
    }

    if (ggl::get<max_corner, 1>(b1) >= ggl::get<min_corner, 1>(b2)
        && ggl::get<min_corner, 1>(b1) <= ggl::get<max_corner, 1>(b2))
    {
        overlaps_y = true;
    }

    return overlaps_x && overlaps_y;
}

}}} // namespace ggl::index::rtree

#endif // GGL_GGL_INDEX_RTREE_HELPERS_HPP
