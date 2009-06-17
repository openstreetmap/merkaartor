// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_GEOMETRY_STRATEGIES_INTERSECTION_POLICIES_HPP
#define GGL_GEOMETRY_STRATEGIES_INTERSECTION_POLICIES_HPP

// TODO: File will be splitted and/or renamed

namespace ggl
{

template <typename S1, typename S2, typename RET>
struct relate_segments_intersection_points
{
    typedef RET return_type;
    typedef S1 segment_type1;
    typedef S2 segment_type2;
    typedef typename select_coordinate_type<S1, S2>::type T;

    static inline return_type rays_intersect(bool on_segment, double ra, double rb,
                    const T& dx1, const T& dy1, const T& dx2, const T& dy2, const T& wx, const T& wy,
                    const S1& s1, const S2& s2)
    {
        return_type result;

        if (on_segment)
        {
            result.count = 1;
            set<0>(result.intersections[0], get<0, 0>(s1) + ra * dx1);
            set<1>(result.intersections[0], get<0, 1>(s1) + ra * dy1);
        }
        return result;
    }

    static inline return_type collinear_touch(const T& x, const T& y, bool)
    {
        return_type result;
        result.count = 1;
        set<0>(result.intersections[0], x);
        set<1>(result.intersections[0], y);
        return result;
    }

    template <typename S>
    static inline return_type collinear_inside(const S& s)
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
    static inline return_type collinear_interior_boundary_intersect(const S& s, bool, bool)
    {
        return collinear_inside(s);
    }

    static inline return_type collinear_a_in_b(const S1& s, bool)
    {
        return collinear_inside(s);
    }
    static inline return_type collinear_b_in_a(const S2& s, bool)
    {
        return collinear_inside(s);
    }

    static inline return_type collinear_overlaps(const T& x1, const T& y1, const T& x2, const T& y2, bool opposite)
    {
        return_type result;
        result.count = 2;
        set<0>(result.intersections[0], x1);
        set<1>(result.intersections[0], y1);
        set<0>(result.intersections[1], x2);
        set<1>(result.intersections[1], y2);
        return result;
    }

    static inline return_type segment_equal(const S1& s, bool opposite)
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
    static inline return_type degenerate(const S1& s, bool)
    {
        return_type result;
        result.count = 1;
        set<0>(result.intersections[0], get<0, 0>(s));
        set<1>(result.intersections[0], get<0, 1>(s));
        return result;
    }
};


template <typename S1, typename S2>
struct relate_segments_de9im
{
    typedef de9im_segment return_type;
    typedef S1 segment_type1;
    typedef S2 segment_type2;
    typedef typename select_coordinate_type<S1, S2>::type T;

    static inline return_type rays_intersect(bool on_segment, double ra, double rb,
                    const T& dx1, const T& dy1, const T& dx2, const T& dy2, const T& wx, const T& wy,
                    const S1& s1, const S2& s2)
    {
        if(on_segment)
        {
            // 0 <= ra <= 1 and 0 <= rb <= 1
            // Now check if one of them is 0 or 1, these are "touch" cases
            bool a = math::equals(ra, 0.0) || math::equals(ra, 1.0);
            bool b = math::equals(rb, 0.0) || math::equals(rb, 1.0);
            if (a && b)
            {
                // Touch boundary/boundary: i-i == -1, i-b == -1, b-b == 0
                // Opposite: if both are equal they touch in opposite direction
                return de9im_segment(ra,rb,
                        -1, -1, 1,
                        -1,  0, 0,
                         1,  0, 2, false, math::equals(ra,rb));
            }
            else if (a || b)
            {
                // Touch boundary/interior: i-i == -1, i-b == -1 or 0, b-b == -1
                int A = a ? 0 : -1;
                int B = b ? 0 : -1;
                return de9im_segment(ra,rb,
                        -1,  B, 1,
                         A, -1, 0,
                         1,  0, 2);
            }

            // Intersects: i-i == 0, i-b == -1, i-e == 1
            return de9im_segment(ra,rb,
                     0, -1, 1,
                    -1, -1, 0,
                     1,  0, 2);
        }

        // Not on segment, disjoint
        return de9im_segment(ra,rb,
                -1, -1, 1,
                -1, -1, 0,
                 1,  0, 2);
    }

    static inline return_type collinear_touch(const T& x, const T& y, bool opposite)
    {
        return de9im_segment(0,0,
                -1, -1, 1,
                -1,  0, 0,
                 1,  0, 2,
                true, opposite);
    }

    template <typename S>
    static inline return_type collinear_interior_boundary_intersect(const S& s, bool a_within_b, bool opposite)
    {
        return a_within_b
            ? de9im_segment(0,0,
                 1, -1, -1,
                 0,  0, -1,
                 1,  0, 2,
                true, opposite)
            : de9im_segment(0,0,
                 1,  0, 1,
                -1,  0, 0,
                -1, -1, 2,
                true, opposite);
    }



    static inline return_type collinear_a_in_b(const S1& s, bool opposite)
    {
        return de9im_segment(0,0,
                1, -1, -1,
                0, -1, -1,
                1,  0,  2,
                true, opposite);
    }
    static inline return_type collinear_b_in_a(const S2& s, bool opposite)
    {
        return de9im_segment(0,0,
                 1,  0,  1,
                -1, -1,  0,
                -1, -1,  2,
                true, opposite);
    }

    static inline return_type collinear_overlaps(const T& x1, const T& y1, const T& x2, const T& y2, bool opposite)
    {
        return de9im_segment(0,0,
                1,  0, 1,
                0, -1, 0,
                1,  0, 2,
                true, opposite);
    }

    static inline return_type segment_equal(const S1& s, bool opposite)
    {
        return de9im_segment(0,0,
                 1, -1, -1,
                -1,  0, -1,
                -1, -1,  2,
                 true, opposite);
    }

    static inline return_type degenerate(const S1& segment, bool a_degenerate)
    {
            return a_degenerate
                ? de9im_segment(0,0,
                     0, -1, -1,
                    -1, -1, -1,
                     1,  0,  2,
                     false, false, false, true)
                : de9im_segment(0,0,
                     0, -1,  1,
                    -1, -1,  0,
                    -1, -1,  2,
                     false, false, false, true);
    }

    static inline return_type collinear_disjoint()
    {
        return de9im_segment(0,0,
                -1, -1, 1,
                -1, -1, 0,
                 1,  0, 2,
                true);
    }


    static inline return_type parallel()
    {
        return de9im_segment(0,0,
                -1, -1, 1,
                -1, -1, 0,
                 1,  0, 2, false, false, true);
    }
};

template <typename S1, typename S2>
struct relate_segments_direction
{
    typedef char return_type;
    typedef S1 segment_type1;
    typedef S2 segment_type2;
    typedef typename select_coordinate_type<S1, S2>::type T;

    static inline return_type rays_intersect(bool on_segment, double ra, double rb,
                    const T& dx1, const T& dy1, const T& dx2, const T& dy2, const T& wx, const T& wy,
                    const S1& s1, const S2& s2)
    {
        if(on_segment)
        {
            // 0 <= ra <= 1 and 0 <= rb <= 1
            // Check the configuration
            bool ra0 = math::equals(ra, 0.0);
            bool ra1 = math::equals(ra, 1.0);
            bool rb0 = math::equals(rb, 0.0);
            bool rb1 = math::equals(rb, 1.0);

            return
                ra0 && rb0 ? 'f'   // opposite and point to each other (TO)
                : ra1 && rb1 ? 't' // opposite and point from each other (FROM)
                    // not opposite, forming a corner, first a then b, directed either left, or right
                : ra1 && rb0 ? calculate_side<1>(dx1, dy1, s1, s2)  // Check side of B2 from A. This is not calculated before
                    // not opposite, forming a corner, first b then a, directed either left, or right
                : ra0 && rb1 ? side_a2_from_b(dx1, dy1, wx, wy)
                : rb0 ? calculate_side<1>(dx1, dy1, s1, s2) // b starts from interior of a
                : ra0 ? side_a2_from_b(dx1, dy1, wx, wy) // a starts from interior of b
                : rb1 ? 'm' // calculate_side<0>(dx1, dy1, s1, s2) // b ends at interior of a
                : ra1 ? 'm' // calculate_side<0>(dx2, dy2, s2, s1) // a ends at interior of b
                : 'i'
                ;
        }

        // Not on segment, disjoint
        return 'd';
    }

    /// This is a "side calculation" as in the strategies, but here two terms are precalculated
    /// We might merge this with side, offering a pre-calculated term
    /// Waiting for implementing spherical...
    template <size_t I>
    static inline return_type calculate_side(const T& dx1, const T& dy1, const S1& s1, const S2& s2)
    {
        T dpx = get<I, 0>(s2) - get<0, 0>(s1);
        T dpy = get<I, 1>(s2) - get<0, 1>(s1);
        return dx1 * dpy - dy1 * dpx > 0 ? 'L' : 'R';
    }

    static inline return_type side_a2_from_b(const T& dx1, const T& dy1, const T& wx, const T& wy)
    {
        // Check side of A2 from B. This was calculated before (wx,wy) but then REVERSE direction.
        // (if A2 is left from B, then B1 will be right from A)
        return dx1 * wy - dy1 * wx > 0 ? 'R' : 'L';
    }


    static inline return_type collinear_touch(const T& x, const T& y, bool opposite)
    {
        return 'c';
    }

    template <typename S>
    static inline return_type collinear_interior_boundary_intersect(const S& s, bool a_within_b, bool opposite)
    {
        return 'c';
    }

    static inline return_type collinear_a_in_b(const S1& s, bool opposite)
    {
        return 'c';
    }
    static inline return_type collinear_b_in_a(const S2& s, bool opposite)
    {
        return 'c';
    }

    static inline return_type collinear_overlaps(const T& x1, const T& y1, const T& x2, const T& y2, bool opposite)
    {
        return 'c';
    }

    static inline return_type segment_equal(const S1& s, bool opposite)
    {
        return 'e';
    }

    static inline return_type degenerate(const S1& segment, bool a_degenerate)
    {
        return '0';
    }

    static inline return_type collinear_disjoint()
    {
        return 'd';
    }


    static inline return_type parallel()
    {
        return 'p';
    }
};

} // namespace ggl

#endif // GGL_GEOMETRY_STRATEGIES_INTERSECTION_POLICIES_HPP
