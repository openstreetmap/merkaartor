// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_GEOMETRY_POLICIES_RELATE_INTERSECTION_POINTS_HPP
#define GGL_GEOMETRY_POLICIES_RELATE_INTERSECTION_POINTS_HPP

#include <boost/concept_check.hpp>

#include <ggl/core/access.hpp>
#include <ggl/util/select_coordinate_type.hpp>


namespace ggl
{

namespace policies { namespace relate {


template <typename S1, typename S2, typename ReturnType>
struct segments_intersection_points
{
    typedef ReturnType return_type;
    typedef S1 segment_type1;
    typedef S2 segment_type2;
    typedef typename select_coordinate_type<S1, S2>::type coordinate_type;

    static inline return_type rays_intersect(bool on_segment,
                    double ra, double rb,
                    coordinate_type const& dx1, coordinate_type const& dy1,
                    coordinate_type const& dx2, coordinate_type const& dy2,
                    coordinate_type const& wx, coordinate_type const& wy,
                    S1 const& s1, S2 const& s2)
    {
        boost::ignore_unused_variable_warning(rb);
        boost::ignore_unused_variable_warning(dx2);
        boost::ignore_unused_variable_warning(dy2);
        boost::ignore_unused_variable_warning(wx);
        boost::ignore_unused_variable_warning(wy);
        boost::ignore_unused_variable_warning(s2);

        return_type result;

        if (on_segment)
        {
            result.count = 1;
            set<0>(result.intersections[0],
                boost::numeric_cast<coordinate_type>(get<0, 0>(s1) + ra * dx1));
            set<1>(result.intersections[0],
                boost::numeric_cast<coordinate_type>(get<0, 1>(s1) + ra * dy1));
        }
        return result;
    }

    static inline return_type collinear_touch(coordinate_type const& x,
                coordinate_type const& y, bool, char)
    {
        return_type result;
        result.count = 1;
        set<0>(result.intersections[0], x);
        set<1>(result.intersections[0], y);
        return result;
    }

    template <typename S>
    static inline return_type collinear_inside(S const& s)
    {
        return_type result;
        result.count = 2;
        set<0>(result.intersections[0], get<0, 0>(s));
        set<1>(result.intersections[0], get<0, 1>(s));
        set<0>(result.intersections[1], get<1, 0>(s));
        set<1>(result.intersections[1], get<1, 1>(s));
        return result;
    }

    template <typename S>
    static inline return_type collinear_interior_boundary_intersect(S const& s, bool, bool)
    {
        return collinear_inside(s);
    }

    static inline return_type collinear_a_in_b(S1 const& s, bool)
    {
        return collinear_inside(s);
    }
    static inline return_type collinear_b_in_a(S2 const& s, bool)
    {
        return collinear_inside(s);
    }

    static inline return_type collinear_overlaps(
                coordinate_type const& x1, coordinate_type const& y1,
                coordinate_type const& x2, coordinate_type const& y2, bool)
    {
        return_type result;
        result.count = 2;
        set<0>(result.intersections[0], x1);
        set<1>(result.intersections[0], y1);
        set<0>(result.intersections[1], x2);
        set<1>(result.intersections[1], y2);
        return result;
    }

    static inline return_type segment_equal(S1 const& s, bool)
    {
        return_type result;
        result.count = 2;
        set<0>(result.intersections[0], get<0, 0>(s));
        set<1>(result.intersections[0], get<0, 1>(s));
        set<0>(result.intersections[1], get<1, 0>(s));
        set<1>(result.intersections[1], get<1, 1>(s));
        return result;
    }

    static inline return_type collinear_disjoint()
    {
        return return_type();
    }
    static inline return_type parallel()
    {
        return return_type();
    }
    static inline return_type degenerate(S1 const& s, bool)
    {
        return_type result;
        result.count = 1;
        set<0>(result.intersections[0], get<0, 0>(s));
        set<1>(result.intersections[0], get<0, 1>(s));
        return result;
    }
};


}} // namespace policies::relate

} // namespace ggl

#endif // GGL_GEOMETRY_POLICIES_RELATE_INTERSECTION_POINTS_HPP
