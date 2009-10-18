// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_GEOMETRY_POLICIES_RELATE_DIRECTION_HPP
#define GGL_GEOMETRY_POLICIES_RELATE_DIRECTION_HPP

#include <boost/concept_check.hpp>

#include <ggl/util/math.hpp>
#include <ggl/util/select_coordinate_type.hpp>


namespace ggl
{


namespace policies { namespace relate {

struct direction_type
{
    inline direction_type(char h,
                double a = 0, double b = 0,
                int ha = 0, int hb = 0,
                int da = 0, int db = 0,
                bool op = false)
        : how(h)
        , opposite(op)
        , ra(a)
        , rb(b)
        , how_a(ha)
        , how_b(hb)
        , dir_a(da)
        , dir_b(db)
    {
    }

    inline direction_type(char h, bool op, int ha = 0, int hb = 0)
        : how(h)
        , opposite(op)
        , ra(0)
        , rb(0)
        , how_a(ha)
        , how_b(hb)
        , dir_a(0)
        , dir_b(0)
    {
    }


    // "How" is the intersection formed?
    char how;

    // Is it opposite (for collinear/equal cases)
    bool opposite;

    // "Distance information", information on how far lies IP from a/b in ratio [0..1]
    double ra, rb;

    // Information on how A arrives at intersection, how B arrives at intersection
    // 1: arrives at intersection
    // -1: starts from intersection
    int how_a;
    int how_b;

    // Direction: how is A positioned from B
    // 1: points left, seen from IP
    // -1: points right, seen from IP
    // In case of intersection: B's TO direction
    // In case that B's TO direction is at A: B's from direction
    // In collinear cases: it is 0
    int dir_a; // Direction of A-s TO from IP
    int dir_b; // Direction of B-s TO from IP
};


template <typename S1, typename S2>
struct segments_direction
{
    typedef direction_type return_type;
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
        boost::ignore_unused_variable_warning(dx2);
        boost::ignore_unused_variable_warning(dy2);
        boost::ignore_unused_variable_warning(wx);
        boost::ignore_unused_variable_warning(wy);

        if(on_segment)
        {
            // 0 <= ra <= 1 and 0 <= rb <= 1
            // Check the configuration
            bool ra0 = math::equals(ra, 0.0);
            bool ra1 = math::equals(ra, 1.0);
            bool rb0 = math::equals(rb, 0.0);
            bool rb1 = math::equals(rb, 1.0);

            return
                // opposite and same starting point (FROM)
                ra0 && rb0 ? calculate_side<1>(ra, rb, dx1, dy1, s1, s2, 'f', -1, -1)

                // opposite and point to each other (TO)
                : ra1 && rb1 ? calculate_side<0>(ra, rb, dx1, dy1, s1, s2, 't', 1, 1)

                // not opposite, forming an angle, first a then b,
                // directed either both left, or both right
                // Check side of B2 from A. This is not calculated before
                : ra1 && rb0 ? angle<1>(ra, rb, dx1, dy1, s1, s2, 'a', 1, -1)

                // not opposite, forming a angle, first b then a,
                // directed either both left, or both right
                : ra0 && rb1 ? angle<0>(ra, rb, dx1, dy1, s1, s2, 'a', -1, 1)

                // b starts from interior of a
                : rb0 ? starts_from_middle(ra, rb, dx1, dy1, s1, s2, 'B', 0, -1)

                // a starts from interior of b (#39)
                : ra0 ? starts_from_middle(ra, rb, dx1, dy1, s1, s2, 'A', -1, 0)

                // b ends at interior of a, calculate direction of A from IP
                : rb1 ? b_ends_at_middle(ra, rb, dx2, dy2, s1, s2)

                // a ends at interior of b
                : ra1 ? a_ends_at_middle(ra, rb, dx1, dy1, s1, s2)

                // normal intersection
                : calculate_side<1>(ra, rb, dx1, dy1, s1, s2, 'i', -1, -1)
                ;
        }

        // Not on segment, disjoint
        return return_type('d');
    }

    static inline return_type collinear_touch(coordinate_type const& , coordinate_type const& , bool opposite, char how)
    {
        // Though this is 'collinear', we handle it as To/From/Angle because it is the same.
        // It only does NOT have a direction.
        int const arrive = how == 'T' ? 1 : -1;
        return
            ! opposite
            ? return_type('a', 0, 0, how == 'A' ? 1 : -1, how == 'B' ? 1 : -1)
            : return_type(how == 'T' ? 't' : 'f', 0, 0, arrive, arrive, 0, 0, true);
    }

    template <typename S>
    static inline return_type collinear_interior_boundary_intersect(S const& , bool, bool opposite)
    {
        return return_type('c', opposite);
    }

    static inline return_type collinear_a_in_b(S1 const& , bool opposite)
    {
        return return_type('c', opposite);
    }
    static inline return_type collinear_b_in_a(S2 const& , bool opposite)
    {
        return return_type('c', opposite);
    }

    static inline return_type collinear_overlaps(
                    coordinate_type const& , coordinate_type const& ,
                    coordinate_type const& , coordinate_type const& , bool opposite)
    {
        return return_type('c', opposite);
    }

    static inline return_type segment_equal(S1 const& , bool opposite)
    {
        return return_type('e', opposite);
    }

    static inline return_type degenerate(S1 const& , bool)
    {
        return return_type('0');
    }

    static inline return_type collinear_disjoint()
    {
        return return_type('d');
    }


    static inline return_type parallel()
    {
        return return_type('p');
    }

private :


    template <std::size_t I>
    static inline return_type calculate_side(double ra, double rb,
                coordinate_type const& dx1, coordinate_type const& dy1,
                S1 const& s1, S2 const& s2,
                char how, int how_a, int how_b)
    {
        coordinate_type dpx = get<I, 0>(s2) - get<0, 0>(s1);
        coordinate_type dpy = get<I, 1>(s2) - get<0, 1>(s1);

        // This is a "side calculation" as in the strategies, but here two terms are precalculated
        // We might merge this with side, offering a pre-calculated term
        // Waiting for implementing spherical...

        return dx1 * dpy - dy1 * dpx > 0
            ? return_type(how, ra, rb, how_a, how_b, -1, 1)
            : return_type(how, ra, rb, how_a, how_b, 1, -1);
    }

    template <std::size_t I>
    static inline return_type angle(double ra, double rb,
                coordinate_type const& dx1, coordinate_type const& dy1,
                S1 const& s1, S2 const& s2,
                char how, int how_a, int how_b)
    {
        coordinate_type dpx = get<I, 0>(s2) - get<0, 0>(s1);
        coordinate_type dpy = get<I, 1>(s2) - get<0, 1>(s1);

         return dx1 * dpy - dy1 * dpx > 0
            ? return_type(how, ra, rb, how_a, how_b, 1, 1)
            : return_type(how, ra, rb, how_a, how_b, -1, -1);
    }


    static inline return_type starts_from_middle(double ra, double rb,
                coordinate_type const& dx1, coordinate_type const& dy1,
                S1 const& s1, S2 const& s2,
                char which,
                int how_a, int how_b)
    {
        // Calculate ARROW of b segment w.r.t. s1
        coordinate_type dpx = get<1, 0>(s2) - get<0, 0>(s1);
        coordinate_type dpy = get<1, 1>(s2) - get<0, 1>(s1);

        int dir = dx1 * dpy - dy1 * dpx > 0 ? 1 : -1;

        // From other perspective, then reverse
        bool const is_a = which == 'A';
        if (is_a)
        {
            dir = -dir;
        }

        return return_type('s', ra, rb,
            how_a,
            how_b,
            is_a ? dir : -dir,
            ! is_a ? dir : -dir);
    }



    // To be harmonized
    static inline return_type a_ends_at_middle(double ra, double rb,
                coordinate_type const& dx, coordinate_type const& dy,
                S1 const& s1, S2 const& s2)
    {
        coordinate_type dpx = get<1, 0>(s2) - get<0, 0>(s1);
        coordinate_type dpy = get<1, 1>(s2) - get<0, 1>(s1);

        // Ending at the middle, one ARRIVES, the other one is NEUTRAL
        // (because it both "arrives"  and "departs"  there
        return dx * dpy - dy * dpx > 0
            ? return_type('m', ra, rb, 1, 0, 1, 1)
            : return_type('m', ra, rb, 1, 0, -1, -1);
    }


    static inline return_type b_ends_at_middle(double ra, double rb,
                coordinate_type const& dx, coordinate_type const& dy,
                S1 const& s1, S2 const& s2)
    {
        coordinate_type dpx = get<1, 0>(s1) - get<0, 0>(s2);
        coordinate_type dpy = get<1, 1>(s1) - get<0, 1>(s2);

        return dx * dpy - dy * dpx > 0
            ? return_type('m', ra, rb, 0, 1, 1, 1)
            : return_type('m', ra, rb, 0, 1, -1, -1);
    }

};

}} // namespace policies::relate

} // namespace ggl

#endif // GGL_GEOMETRY_POLICIES_RELATE_DIRECTION_HPP
