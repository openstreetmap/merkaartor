// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_STRATEGY_CARTESIAN_INTERSECTION_HPP
#define GGL_STRATEGY_CARTESIAN_INTERSECTION_HPP

#include <algorithm>

#include <ggl/core/exception.hpp>
#include <ggl/core/concepts/point_concept.hpp>
#include <ggl/core/concepts/segment_concept.hpp>

#include <ggl/util/select_coordinate_type.hpp>


namespace ggl
{


class relate_cartesian_segments_exception : public ggl::exception
{
public:

    relate_cartesian_segments_exception()  {}

    virtual char const* what() const throw()
    {
        return "GGL: Internal error, unexpected case in relate_segment";
    }
};


namespace strategy { namespace intersection {


#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template <typename S, size_t D>
struct segment_interval
{
    template <typename T>
    static inline void arrange(S const& s, T& s_1, T& s_2, bool& swapped)
    {
        s_1 = get<0, D>(s);
        s_2 = get<1, D>(s);
        if (s_1 > s_2)
        {
            std::swap(s_1, s_2);
            swapped = true;
        }
    }
};

}
#endif


/*!
    \see http://mathworld.wolfram.com/Line-LineIntersection.html
 */
template <typename F>
struct relate_cartesian_segments
{
    typedef typename F::return_type RETURN_TYPE;
    typedef typename F::segment_type1 S1;
    typedef typename F::segment_type2 S2;

    typedef typename point_type<S1>::type P;
    BOOST_CONCEPT_ASSERT( (concept::Point<P>) );

    BOOST_CONCEPT_ASSERT( (concept::ConstSegment<S1>) );
    BOOST_CONCEPT_ASSERT( (concept::ConstSegment<S2>) );
    typedef typename select_coordinate_type<S1, S2>::type T;

    /// Relate segments a and b
    static inline RETURN_TYPE relate(S1 const& a, S2 const& b)
    {
        T dx_a = get<1, 0>(a) - get<0, 0>(a); // distance in x-dir
        T dx_b = get<1, 0>(b) - get<0, 0>(b);
        T dy_a = get<1, 1>(a) - get<0, 1>(a); // distance in y-dir
        T dy_b = get<1, 1>(b) - get<0, 1>(b);
        return relate(a, b, dx_a, dy_a, dx_b, dy_b);
    }


    /// Relate segments a and b using precalculated differences. This can save two or four substractions in many cases
    static inline RETURN_TYPE relate(S1 const& a, S2 const& b,
            T const& dx_a, T const& dy_a, T const& dx_b, T const& dy_b)
    {
        T wx = get<0, 0>(a) - get<0, 0>(b);
        T wy = get<0, 1>(a) - get<0, 1>(b);

        // Calculate determinants - Cramers rule
        T d = (dy_b * dx_a) - (dx_b * dy_a);
        T da = (dx_b * wy) - (dy_b * wx);
        T db = (dx_a * wy) - (dy_a * wx);

        if(! math::equals(d, 0))
        {
            // Determinant d is nonzero. Rays do intersect. This is the normal case.
            // ra/rb: ratio 0-1 where intersection divides A/B

            // Maybe:
            // The ra/rb calculation can probably also be avoided, only necessary to calculate the points themselves
            // On segment: 0 <= r <= 1
            //      where: r==0 and r==1 are special cases
            // --> r=0 if and only if da==0 (d != 0) [this is also checked below]
            // --> r=1 da==d
            // --> da/d > 0 if da==positive and d==positive OR da==neg and d==neg
            // --> da/d < 1 if (both positive) da < d or (negative) da > d, e.g. -2 > -4
            // --> it would save a division but makes it complexer

            double ra = double(da) / double(d);
            double rb = double(db) / double(d);

            return F::rays_intersect(ra >= 0 && ra <= 1 && rb >= 0 && rb <= 1, ra, rb,
                dx_a, dy_a, dx_b, dy_b, wx, wy, a, b);
        }

        if(math::equals(da, 0) && math::equals(db, 0))
        {
            // Both da & db are zero. Segments are collinear. We'll find out how.
            return relate_collinear(a, b, dx_a, dy_a, dx_b, dy_b);
        }

        // Segments are parallel (might still be opposite but this is probably never interesting)
        return F::parallel();
    }

private :


    /// Relate segments known collinear
    static inline RETURN_TYPE relate_collinear(S1 const& a, S2 const& b,
            T const& dx_a, T const& dy_a,
            T const& dx_b, T const& dy_b)
    {
        // All ca. 200 lines are about collinear rays
        // The intersections, if any, are always boundary points of the segments. No need to calculate anything.
        // However we want to find out HOW they intersect, there are many cases.
        // Most sources only provide the intersection (above) or that there is a collinearity (but not the points)
        // or some spare sources give the intersection points (calculated) but not how they align.
        // This source tries to give everything and still be efficient.
        // It is therefore (and because of the extensive clarification comments) rather long...

        // \see http://mpa.itc.it/radim/g50history/CMP/4.2.1-CERL-beta-libes/file475.txt
        // \see http://docs.codehaus.org/display/GEOTDOC/Point+Set+Theory+and+the+DE-9IM+Matrix
        // \see http://mathworld.wolfram.com/Line-LineIntersection.html

        // Because of collinearity the case is now one-dimensional and can be checked using intervals
        // We arrange either horizontally or vertically
        // We get then two intervals:
        // a_1-------------a_2 where a_1 < a_2
        // b_1-------------b_2 where b_1 < b_2
        // In all figures below a_1/a_2 denotes arranged intervals, a1-a2 or a2-a1 are still unarranged
        T a_1, a_2, b_1, b_2;
        bool a_swapped = false, b_swapped = false;
        if (math::equals(dx_b, 0))
        {
            // Vertical -> Check y-direction
            detail::segment_interval<S1, 1>::arrange(a, a_1, a_2, a_swapped);
            detail::segment_interval<S1, 1>::arrange(b, b_1, b_2, b_swapped);
        }
        else
        {
            // Check x-direction
            detail::segment_interval<S1, 0>::arrange(a, a_1, a_2, a_swapped);
            detail::segment_interval<S1, 0>::arrange(b, b_1, b_2, b_swapped);
        }

        // First handle "disjoint", probably common case.
        // 2 cases: a_1----------a_2    b_1-------b_2 or B left of A
        if (a_2 < b_1 || a_1 > b_2)
        {
            return F::collinear_disjoint();
        }

        // Then handle "equal", in polygon neighbourhood comparisons also a common case

        // Check if segments are equal...
        bool a1_eq_b1 = math::equals(get<0, 0>(a), get<0, 0>(b))
                    && math::equals(get<0, 1>(a), get<0, 1>(b));
        bool a2_eq_b2 = math::equals(get<1, 0>(a), get<1, 0>(b))
                    && math::equals(get<1, 1>(a), get<1, 1>(b));
        if (a1_eq_b1 && a2_eq_b2)
        {
            return F::segment_equal(a, false);
        }

        // ... or opposite equal
        bool a1_eq_b2 = math::equals(get<0, 0>(a), get<1, 0>(b))
                    && math::equals(get<0, 1>(a), get<1, 1>(b));
        bool a2_eq_b1 = math::equals(get<1, 0>(a), get<0, 0>(b))
                    && math::equals(get<1, 1>(a), get<0, 1>(b));
        if (a1_eq_b2 && a2_eq_b1)
        {
            return F::segment_equal(a, true);
        }


        // Degenerate cases: segments of single point, lying on other segment, non disjoint
        if (math::equals(dx_a, 0) && math::equals(dy_a, 0))
        {
            return F::degenerate(a, true);
        }
        if (math::equals(dx_b, 0) && math::equals(dy_b, 0))
        {
            return F::degenerate(b, false);
        }


        // The rest below will return one or two intersections.
        // The delegated class can decide which is the intersection point, or two, build the Intersection Matrix (IM)
        // For IM it is important to know which relates to which. So this information is given,
        // without performance penalties to intersection calculation

        bool has_common_points = a1_eq_b1 || a1_eq_b2 || a2_eq_b1 || a2_eq_b2;


        // "Touch" -> one intersection point -> one but not two common points
        // -------->             A (or B)
        //         <----------   B (or A)
        //        a_2==b_1         (b_2==a_1 or a_2==b1)

        // The check a_2/b_1 is necessary because it excludes cases like
        // ------->
        //     --->
        // ... which are handled lateron

        // Corresponds to 4 cases, of which the equal points are determined above
        // 1: a1---->a2 b1--->b2   ("a" first)
        // 2: a2<----a1 b2<---b1   ("b" first)
        // 3: a1---->a2 b2<---b1   ("t": to)
        // 4: a2<----a1 b1--->b2   ("f": from)
        // Where the arranged forms have two forms:
        //    a_1-----a_2/b_1-------b_2 or reverse (B left of A)
        if (has_common_points && (math::equals(a_2, b_1) || math::equals(b_2, a_1)))
        {
            if (a2_eq_b1) return F::collinear_touch(get<1, 0>(a), get<1, 1>(a), false, 'A');
            if (a2_eq_b2) return F::collinear_touch(get<1, 0>(a), get<1, 1>(a), true, 'T');
            if (a1_eq_b2) return F::collinear_touch(get<0, 0>(a), get<0, 1>(a), false, 'B');
            if (a1_eq_b1) return F::collinear_touch(get<0, 0>(a), get<0, 1>(a), true, 'F');
        }


        // "Touch/within" -> there are common points and also an intersection of interiors:
        // Corresponds to many cases:
        // Case 1: a1------->a2  Case 5:      a1-->a2  Case 9:  a1--->a2
        //             b1--->b2          b1------->b2           b1---------b2
        // Case 2: a2<-------a1
        //             b1----b2  Et cetera
        // Case 3: a1------->a2
        //             b2<---b1
        // Case 4: a2<-------a1
        //             b2<---b1

        // For case 1-4: a_1 < (b_1 or b_2) < a_2, two intersections are equal to segment B
        // For case 5-8: b_1 < (a_1 or a_2) < b_2, two intersections are equal to segment A
        if (has_common_points)
        {
            bool a_in_b =  (b_1 < a_1 && a_1 < b_2) || (b_1 < a_2 && a_2 < b_2);
            if (a2_eq_b2) return F::collinear_interior_boundary_intersect(a_in_b ? a : b, a_in_b, false);
            if (a1_eq_b2) return F::collinear_interior_boundary_intersect(a_in_b ? a : b, a_in_b, true);
            if (a2_eq_b1) return F::collinear_interior_boundary_intersect(a_in_b ? a : b, a_in_b, true);
            if (a1_eq_b1) return F::collinear_interior_boundary_intersect(a_in_b ? a : b, a_in_b, false);
        }

        bool opposite = a_swapped ^ b_swapped;


        // "Inside", a completely within b or b completely within a
        // 2 cases:
        // case 1:
        //        a_1---a_2        -> take A's points as intersection points
        //   b_1------------b_2
        // case 2:
        //   a_1------------a_2
        //       b_1---b_2         -> take B's points
        if (a_1 > b_1 && a_2 < b_2)
        {
            // A within B
            return F::collinear_a_in_b(a, opposite);
        }
        if (b_1 > a_1 && b_2 < a_2)
        {
            // B within A
            return F::collinear_b_in_a(b, opposite);
        }


        // Now that all cases with equal,touch,inside,disjoint,degenerate are handled the only thing left is an overlap
        // Case 1: a1--------->a2         a_1---------a_2
        //                 b1----->b2             b_1------b_2
        // Case 2: a2<---------a1         a_1---------a_2          a_swapped
        //                 b1----->b2             b_1------b_2
        // Case 3: a1--------->a2         a_1---------a_2
        //                 b2<-----b1             b_1------b_2     b_swapped
        // Case 4: a2<-------->a1         a_1---------a_2          a_swapped
        //                 b2<-----b1             b_1------b_2     b_swapped

        // Case 5:     a1--------->a2          a_1---------a_2
        //         b1----->b2             b1--------b2
        // Case 6:     a2<---------a1
        //         b1----->b2
        // Case 7:     a1--------->a2
        //         b2<-----b1
        // Case 8:     a2<-------->a1
        //         b2<-----b1

        if (a_1 < b_1 && b_1 < a_2)
        {
            // Case 4,2,3,1
            return
                  a_swapped && b_swapped   ? F::collinear_overlaps(get<0, 0>(a), get<0, 1>(a), get<1, 0>(b), get<1, 1>(b), opposite)
                : a_swapped                ? F::collinear_overlaps(get<0, 0>(a), get<0, 1>(a), get<0, 0>(b), get<0, 1>(b), opposite)
                : b_swapped                ? F::collinear_overlaps(get<1, 0>(a), get<1, 1>(a), get<1, 0>(b), get<1, 1>(b), opposite)
                :                            F::collinear_overlaps(get<1, 0>(a), get<1, 1>(a), get<0, 0>(b), get<0, 1>(b), opposite)
                ;
        }
        if (b_1 < a_1 && a_1 < b_2)
        {
            // Case 8, 6, 7, 5
            return
                  a_swapped && b_swapped   ? F::collinear_overlaps(get<1, 0>(a), get<1, 1>(a), get<0, 0>(b), get<0, 1>(b), opposite)
                : a_swapped                ? F::collinear_overlaps(get<1, 0>(a), get<1, 1>(a), get<1, 0>(b), get<1, 1>(b), opposite)
                : b_swapped                ? F::collinear_overlaps(get<0, 0>(a), get<0, 1>(a), get<0, 0>(b), get<0, 1>(b), opposite)
                :                            F::collinear_overlaps(get<0, 0>(a), get<0, 1>(a), get<1, 0>(b), get<1, 1>(b), opposite)
                ;
        }

        // Nothing should goes through. If any we have made an error
        // TODO: proper exception
        throw relate_cartesian_segments_exception();
    }
};


}} // namespace strategy::intersection



} // namespace ggl


#endif // GGL_STRATEGY_CARTESIAN_INTERSECTION_HPP
