// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_MULTI_ALGORITHMS_GET_INTERSECTION_POINTS_HPP
#define GGL_MULTI_ALGORITHMS_GET_INTERSECTION_POINTS_HPP


#include <ggl/multi/algorithms/distance.hpp>
#include <ggl/multi/algorithms/sectionalize.hpp>

#include <ggl/multi/iterators/point_const_iterator.hpp>

#include <ggl/algorithms/overlay/get_intersection_points.hpp>



namespace ggl
{




#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace get_intersection_points {



}} // namespace detail::get_intersection_points
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{




template<typename Polygon1, typename Polygon2, typename IntersectionPoints>
struct get_intersection_points
    <
        multi_polygon_tag, multi_polygon_tag,
        Polygon1, Polygon2,
        IntersectionPoints
    >
    : detail::get_intersection_points::get_ips_generic<Polygon1, Polygon2, IntersectionPoints>
{};




} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



} // namespace ggl

#endif // GGL_MULTI_ALGORITHMS_GET_INTERSECTION_POINTS_HPP
