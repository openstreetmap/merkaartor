// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_GEOMETRY_POLICIES_RELATE_TUPLED_HPP
#define GGL_GEOMETRY_POLICIES_RELATE_TUPLED_HPP

#include <boost/tuple/tuple.hpp>
#include <ggl/util/select_coordinate_type.hpp>

namespace ggl
{

namespace policies { namespace relate {


// "tupled" to return intersection results together.
// Now with two, with some meta-programming and derivations it can also be three (or more)
template <typename Policy1, typename Policy2>
struct segments_tupled
{
    typedef boost::tuple
        <
            typename Policy1::return_type,
            typename Policy2::return_type
        > return_type;

    // Take segments of first policy, they should be equal
    typedef typename Policy1::segment_type1 segment_type1;
    typedef typename Policy1::segment_type2 segment_type2;

    typedef typename select_coordinate_type
        <
            segment_type1,
            segment_type2
        >::type coordinate_type;


    static inline return_type rays_intersect(bool on_segment,
                    double ra, double rb,
                    coordinate_type const& dx1, coordinate_type const& dy1,
                    coordinate_type const& dx2, coordinate_type const& dy2,
                    coordinate_type const& wx, coordinate_type const& wy,
                    segment_type1 const& s1, segment_type2 const& s2)
    {
        return boost::make_tuple
            (
                Policy1::rays_intersect(on_segment, ra, rb, dx1, dy1, dx2, dy2, wx, wy, s1, s2),
                Policy2::rays_intersect(on_segment, ra, rb, dx1, dy1, dx2, dy2, wx, wy, s1, s2)
            );
    }

    static inline return_type collinear_touch(coordinate_type const& x,
                coordinate_type const& y, bool opposite, char how)
    {
        return boost::make_tuple
            (
                Policy1::collinear_touch(x, y, opposite, how),
                Policy2::collinear_touch(x, y, opposite, how)
            );
    }

    template <typename S>
    static inline return_type collinear_interior_boundary_intersect(S const& segment,
                bool a_within_b, bool opposite)
    {
        return boost::make_tuple
            (
                Policy1::collinear_interior_boundary_intersect(segment, a_within_b, opposite),
                Policy2::collinear_interior_boundary_intersect(segment, a_within_b, opposite)
            );
    }

    static inline return_type collinear_a_in_b(segment_type1 const& segment,
                bool opposite)
    {
        return boost::make_tuple
            (
                Policy1::collinear_a_in_b(segment, opposite),
                Policy2::collinear_a_in_b(segment, opposite)
            );
    }
    static inline return_type collinear_b_in_a(segment_type2 const& segment,
                    bool opposite)
    {
        return boost::make_tuple
            (
                Policy1::collinear_b_in_a(segment, opposite),
                Policy2::collinear_b_in_a(segment, opposite)
            );
    }


    static inline return_type collinear_overlaps(
                    coordinate_type const& x1, coordinate_type const& y1,
                    coordinate_type const& x2, coordinate_type const& y2,
                    bool opposite)
    {
        return boost::make_tuple
            (
                Policy1::collinear_overlaps(x1, y1, x2, y2, opposite),
                Policy2::collinear_overlaps(x1, y1, x2, y2, opposite)
            );
    }

    static inline return_type segment_equal(segment_type1 const& s,
                bool opposite)
    {
        return boost::make_tuple
            (
                Policy1::segment_equal(s, opposite),
                Policy2::segment_equal(s, opposite)
            );
    }

    static inline return_type degenerate(segment_type1 const& segment,
                bool a_degenerate)
    {
        return boost::make_tuple
            (
                Policy1::degenerate(segment, a_degenerate),
                Policy2::degenerate(segment, a_degenerate)
            );
    }

    static inline return_type collinear_disjoint()
    {
        return boost::make_tuple
            (
                Policy1::collinear_disjoint(),
                Policy2::collinear_disjoint()
            );
    }


    static inline return_type parallel()
    {
        return boost::make_tuple
            (
                Policy1::parallel(),
                Policy2::parallel()
            );
    }
};

}} // namespace policies::relate

} // namespace ggl

#endif // GGL_GEOMETRY_POLICIES_RELATE_TUPLED_HPP
