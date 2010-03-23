// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_INTERSECTS_HPP
#define GGL_ALGORITHMS_INTERSECTS_HPP

#include <ggl/algorithms/intersection_linestring.hpp>

#include <ggl/algorithms/overlay/get_intersection_points.hpp>
#include <ggl/algorithms/overlay/self_intersection_points.hpp>
#include <ggl/algorithms/overlay/adapt_turns.hpp>
#include <ggl/algorithms/overlay/enrich_intersection_points.hpp>
#include <ggl/algorithms/overlay/traverse.hpp>

#include <ggl/algorithms/assign.hpp>
#include <ggl/algorithms/convert.hpp>
#include <ggl/algorithms/within.hpp>



namespace ggl
{

/*!
    \brief Determine if there is at least one intersection
        (crossing or self-tangency)
    \note This function can be called for one geometry (self-intersection) and
        also for two geometries (intersection)
    \ingroup overlay
    \tparam Geometry geometry type
    \param geometry geometry
    \return TRUE if there are intersections, else FALSE
 */
template <typename Geometry>
inline bool intersects(Geometry const& geometry)
{
    typedef typename boost::remove_const<Geometry>::type ncg_type;

    typedef std::vector<ggl::detail::intersection::intersection_point
        <typename ggl::point_type<Geometry>::type> > ip_vector;

    ip_vector ips;

    dispatch::self_intersection_points
            <
                typename tag<ncg_type>::type,
                is_multi<ncg_type>::type::value,
                ncg_type,
                ip_vector
            >::apply(geometry, true, ips);
    return ips.size() > 0;
}

} // ggl

#endif //GGL_ALGORITHMS_INTERSECTS_HPP
