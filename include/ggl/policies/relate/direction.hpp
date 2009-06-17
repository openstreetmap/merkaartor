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
#include <ggl/util/promotion_traits.hpp>


namespace ggl
{


namespace policies { namespace relate {

struct direction_type
{
    inline direction_type(char h,
                double a = 0, double b = 0,
                int ha = 0, int hb = 0,
                int d = 0)
        : how(h)
        , ra(a)
        , rb(b)
        , how_a(ha)
        , how_b(hb)
        , direction(d)
    {}

    // "How" is the intersection formed?
    char how;


    // "Distance information", information on how far lies IP from a/b in ratio [0..1]
    double ra, rb;

    // Information on how A arrives at intersection, how B arrives at intersection
    // 1: arrives at intersection
    // -1: starts from intersection
    // Contrary to direction, this should be specified for both A and B
    int how_a;
    int how_b;

    // Direction: how is A positioned from B
    // 1: A is left from B
    // -1: A is right from B
    // In case of intersection: B's TO direction
    // In case that B's TO direction is at A: B's from direction
    // In collinear cases: it is 0
    int direction;
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
                ra0 && rb0 ? calculate_side<1, 'f', -1, -1>(ra, rb, dx1, dy1, s1, s2)

                // opposite and point to each other (TO)
                : ra1 && rb1 ? calculate_side<0, 't', 1, 1>(ra, rb, dx1, dy1, s1, s2)

                // not opposite, forming an angle, first a then b, directed either left, or right
                // Check side of B2 from A. This is not calculated before
                : ra1 && rb0 ? calculate_side<1, 'a', 1, -1>(ra, rb, dx1, dy1, s1, s2)

                // not opposite, forming a angle, first b then a, directed either left, or right
                : ra0 && rb1 ? calculate_side<0, 'a', -1, 1>(ra, rb, dx1, dy1, s1, s2)

                // b starts from interior of a
                : rb0 ? calculate_side<1, 's', 0, -1>(ra, rb, dx1, dy1, s1, s2)

                // a starts from interior of b (#39)
                : ra0 ? calculate_side<1, 's', -1, 0>(ra, rb, dx1, dy1, s1, s2)

                // b ends at interior of a
                : rb1 ? calculate_side<0, 'm', 0, 1>(ra, rb, dx1, dy1, s1, s2)

                // a ends at interior of b
                : ra1 ? calculate_side<1, 'm', 1, 0>(ra, rb, dx1, dy1, s1, s2)

                // normal intersection
                : calculate_side<1, 'i', 0, 0>(ra, rb, dx1, dy1, s1, s2)
                ;
        }

        // Not on segment, disjoint
        return return_type('d');
    }

    /// This is a "side calculation" as in the strategies, but here two terms are precalculated
    /// We might merge this with side, offering a pre-calculated term
    /// Waiting for implementing spherical...
    template <size_t I, char C, size_t A, size_t B>
    static inline return_type calculate_side(double ra, double rb,
                coordinate_type const& dx1, coordinate_type const& dy1,
                S1 const& s1, S2 const& s2)
    {
        coordinate_type dpx = get<I, 0>(s2) - get<0, 0>(s1);
        coordinate_type dpy = get<I, 1>(s2) - get<0, 1>(s1);
        return dx1 * dpy - dy1 * dpx > 0
            ? return_type(C, ra, rb, A, B, -1)
            : return_type(C, ra, rb, A, B, 1);
    }

    static inline return_type collinear_touch(coordinate_type const& , coordinate_type const& , bool)
    {
        return return_type('c');
    }

    template <typename S>
    static inline return_type collinear_interior_boundary_intersect(S const& , bool, bool)
    {
        return return_type('c');
    }

    static inline return_type collinear_a_in_b(S1 const& , bool)
    {
        return return_type('c');
    }
    static inline return_type collinear_b_in_a(S2 const& , bool)
    {
        return return_type('c');
    }

    static inline return_type collinear_overlaps(
                    coordinate_type const& , coordinate_type const& ,
                    coordinate_type const& , coordinate_type const& , bool)
    {
        return return_type('c');
    }

    static inline return_type segment_equal(S1 const& , bool)
    {
        return return_type('e');
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
};

}} // namespace policies::relate

} // namespace ggl

#endif // GGL_GEOMETRY_POLICIES_RELATE_DIRECTION_HPP
