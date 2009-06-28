// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_GET_INTERSECTION_POINTS_HPP
#define GGL_ALGORITHMS_GET_INTERSECTION_POINTS_HPP

#include <vector>

#include <boost/mpl/if.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/core/access.hpp>
#include <ggl/core/coordinate_dimension.hpp>
#include <ggl/core/reverse_dispatch.hpp>

#include <ggl/core/exterior_ring.hpp>
#include <ggl/core/interior_rings.hpp>
#include <ggl/core/ring_type.hpp>

#include <ggl/util/math.hpp>

#include <ggl/geometries/box.hpp>

#include <ggl/strategies/cartesian/cart_intersect.hpp>
#include <ggl/strategies/intersection_result.hpp>

#include <ggl/policies/relate/intersection_points.hpp>
#include <ggl/policies/relate/direction.hpp>
#include <ggl/policies/relate/tupled.hpp>

#include <ggl/algorithms/overlay/intersection_point.hpp>

#include <ggl/algorithms/distance.hpp>
#include <ggl/algorithms/disjoint.hpp>
#include <ggl/algorithms/sectionalize.hpp>
#include <ggl/algorithms/within.hpp>




namespace ggl
{




#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace get_intersection_points {


template <typename Segment1, typename Segment2, typename IntersectionPoints>
struct relate
{
    static inline void apply(Segment1 const& s1, Segment2 const& s2,
                segment_identifier const& seg_id1,
                segment_identifier const& seg_id2,
                IntersectionPoints& out, bool& non_trivial)
    {
        typedef typename boost::range_value<IntersectionPoints>::type intersection_point;
        typedef segment_intersection_points<intersection_point> ip_type;

        typedef boost::tuple
            <
                ip_type,
                policies::relate::direction_type
            > result_type;

        result_type result = strategy::intersection::relate_cartesian_segments
            <
                policies::relate::segments_tupled
                    <
                        policies::relate::segments_intersection_points
                            <
                                Segment1,
                                Segment2,
                                ip_type
                            > ,
                        policies::relate::segments_direction
                            <
                                Segment1,
                                Segment2
                            >
                    >
            >::relate(s1, s2);

        ip_type& is = result.get<0>();
        policies::relate::direction_type const& dir = result.get<1>();
        for (int i = 0; i < is.count; i++)
        {
            typedef typename point_type<Segment1>::type point1_type;
            typedef typename cs_tag<point1_type>::type tag;

            // Slight enhancement for distance-calculations below:
            // make this a separate policy to have less computations
            // NOTE: tried using ra/rb from segment-intersection (only valid for non-collinear).
            // There was NO measurable performance increment.

            typename intersection_point::traversal_type info;
            info.seg_id = seg_id1;
            info.distance = ggl::distance(is.intersections[i], s1.first);
            //info.distance = dir.ra; // NOTE: not possible for collinear intersections!
            info.how = dir.how;
            info.arrival = dir.how_a;
            info.direction = dir.direction; // Direction from A with respect to B
            is.intersections[i].info.push_back(info);

            info.seg_id = seg_id2;
            info.distance = ggl::distance(is.intersections[i], s2.first);
            //info.distance = dir.rb; // NOTE: not possible for collinear intersections!
            info.arrival = dir.how_b;
            info.direction = -dir.direction; // Direction from B with respect to A
            is.intersections[i].info.push_back(info);

            out.push_back(is.intersections[i]);

            if (dir.how != 'i')
            {
                non_trivial = true;
            }
        }
    }
};

template
<
    typename Geometry1, typename Geometry2,
    typename Section1, typename Section2,
    typename IntersectionPoints
>
class get_ips_in_sections
{
public :
    static inline void apply(Geometry1 const& geometry1, Geometry2 const& geometry2,
            Section1 const& sec1, Section2 const& sec2,
            IntersectionPoints& intersection_points, bool& non_trivial)
    {

        typedef typename ggl::point_const_iterator<Geometry1>::type range1_iterator;
        typedef typename ggl::point_const_iterator<Geometry2>::type range2_iterator;

        int const dir1 = sec1.directions[0];
        int const dir2 = sec2.directions[0];
        register int index1 = sec1.begin_index;

        // Note that it is NOT possible to have section-iterators here
        // because of the logistics of "index" (the section-iterator automatically
        // skips to the begin-point, we loose the index or have to recalculate it)
        // So we mimic it here
        range1_iterator it1, end1;
        get_section
            <
                typename ggl::tag<Geometry1>::type,
                Geometry1,
                Section1
            >::apply(geometry1, sec1, it1, end1);

        // Mimic 1: Skip to the point that the section interects the other box
        range1_iterator prev1 = it1++;
        for(; it1 != end1 && preceding<0>(dir1, *it1, sec2.bounding_box);
            prev1 = it1++, index1++)
        {
        }
        // Go back one step because we want to start completely preceding
        it1 = prev1;

        // Walk through section and stop if we exceed the other box
        for (prev1 = it1++;
            it1 != end1 && ! exceeding<0>(dir1, *prev1, sec2.bounding_box);
            prev1 = it1++, index1++)
        {
            segment1_type s1(*prev1, *it1);

            register int index2 = sec2.begin_index;

            range2_iterator it2, end2;
            get_section
                <
                    typename ggl::tag<Geometry2>::type,
                    Geometry2,
                    Section2
                >::apply(geometry2, sec2, it2, end2);

            range2_iterator prev2 = it2++;
            // Mimic 2:
            for(; it2 != end2 && preceding<0>(dir2, *it2, sec1.bounding_box);
                prev2 = it2++, index2++)
            {
            }
            it2 = prev2;

            for (prev2 = it2++;
                it2 != end2 && ! exceeding<0>(dir2, *prev2, sec1.bounding_box);
                prev2 = it2++, index2++)
            {
                relate<segment1_type, segment2_type, IntersectionPoints>
                    ::apply(s1, segment2_type(*prev2, *it2),
                    segment_identifier(0,
                                sec1.multi_index, sec1.ring_index, index1),
                    segment_identifier(1,
                                sec2.multi_index, sec2.ring_index, index2),
                    intersection_points, non_trivial);
            }
        }
    }


private :
    typedef typename ggl::point_type<Geometry1>::type point1_type;
    typedef typename ggl::point_type<Geometry1>::type point2_type; // TODO: change this with 2 as soon as boxes are solved / not converted!
    typedef typename ggl::segment<const point1_type> segment1_type;
    typedef typename ggl::segment<const point2_type> segment2_type;



    template <size_t Dim, typename Point, typename Box>
    static inline bool preceding(int dir, Point const& point, Box const& box)
    {
        return (dir == 1  && get<Dim>(point) < get<min_corner, Dim>(box))
            || (dir == -1 && get<Dim>(point) > get<max_corner, Dim>(box));
    }

    template <size_t Dim, typename Point, typename Box>
    static inline bool exceeding(int dir, Point const& point, Box const& box)
    {
        return (dir == 1  && get<Dim>(point) > get<max_corner, Dim>(box))
            || (dir == -1 && get<Dim>(point) < get<min_corner, Dim>(box));
    }


};


template
<
    typename Ring, typename Box,
    typename Section1, typename Section2,
    typename IntersectionPoints
>
class get_ips_range_box
{
public :
    static inline void apply(Ring const& ring, Box const& box,
            Section1 const& sec1, Section2 const& sec2,
            IntersectionPoints& intersection_points, bool& non_trivial)
    {
        get_ips_in_sections<Ring, Box, Section1, Section2, IntersectionPoints>
            ::apply(ring, box, sec1, sec2, intersection_points, non_trivial);
    }
};




template<typename Geometry1, typename Geometry2, typename IntersectionPoints>
struct get_ips_generic
{
    static inline bool apply(Geometry1 const& geometry1, Geometry2 const& geometry2,
            IntersectionPoints& intersection_points)
    {
        // Create monotonic sections in ONE direction
        // (this is ~1% faster than in TWO directions, at least for the NLP4 testset)
        typedef typename ggl::sections
            <
                ggl::box < typename ggl::point_type<Geometry1>::type >, 1
            > sections1_type;
        typedef typename ggl::sections
            <
                ggl::box < typename ggl::point_type<Geometry2>::type >, 1
            > sections2_type;

        sections1_type sec1;
        sections2_type sec2;

        ggl::sectionalize(geometry1, sec1);
        ggl::sectionalize(geometry2, sec2);

        bool non_trivial = false;
        for (typename boost::range_const_iterator<sections1_type>::type it1 = sec1.begin();
            it1 != sec1.end();
            ++it1)
        {
            for (typename boost::range_const_iterator<sections2_type>::type it2 = sec2.begin();
                it2 != sec2.end();
                ++it2)
            {
                if (! ggl::disjoint(it1->bounding_box, it2->bounding_box))
                {
                    get_ips_in_sections
                    <
                        Geometry1,
                        Geometry2,
                        typename boost::range_value<sections1_type>::type,
                        typename boost::range_value<sections2_type>::type,
                        IntersectionPoints
                    >::apply(geometry1, geometry2, *it1, *it2, intersection_points, non_trivial);
                }
            }
        }
        return non_trivial;
    }
};


static const char cohen_sutherland_top    = 1; // 0001
static const char cohen_sutherland_bottom = 2; // 0010
static const char cohen_sutherland_right  = 4; // 0100
static const char cohen_sutherland_left   = 8; // 1000


template<typename Range, typename Box, typename IntersectionPoints>
struct get_ips_cs
{
    static inline void apply(Range const& range,
            int multi_index, int ring_index,
            Box const& box,
            IntersectionPoints& intersection_points,
            bool non_trivial)
    {
        if (boost::size(range) <= 1)
        {
            return;
        }


        typedef typename ggl::point_type<Box>::type box_point_type;
        typedef typename ggl::point_type<Range>::type point_type;

        typedef segment<const box_point_type> box_segment_type;
        typedef segment<const point_type> segment_type;

        point_type lower_left, upper_left, lower_right, upper_right;
        assign_box_corners(box, lower_left, lower_right, upper_left, upper_right);

        box_segment_type left(lower_left, upper_left);
        box_segment_type top(upper_left, upper_right);
        box_segment_type right(upper_right, lower_right);
        box_segment_type bottom(lower_right, lower_left);


        typedef typename boost::range_const_iterator<Range>::type iterator_type;
        iterator_type it = boost::begin(range);

        bool first = true;

        char previous_side[2] = {0, 0};

        int index = 0;

        for (iterator_type prev = it++;
            it != boost::end(range);
            prev = it++, index++)
        {
            segment_type segment(*prev, *it);

            if (first)
            {
                previous_side[0] = get_side<0>(box, *prev);
                previous_side[1] = get_side<1>(box, *prev);
            }

            char current_side[2];
            current_side[0] = get_side<0>(box, *it);
            current_side[1] = get_side<1>(box, *it);

            // There can NOT be intersections if
            // 1) EITHER the two points are lying on one side of the box (! 0 && the same)
            // 2) OR same in Y-direction
            // 3) OR all points are inside the box (0)
            if (! (
                (current_side[0] != 0 && current_side[0] == previous_side[0])
                || (current_side[1] != 0 && current_side[1] == previous_side[1])
                || (current_side[0] == 0
                        && current_side[1] == 0
                        && previous_side[0] == 0
                        && previous_side[1] == 0)
                  )
                )
            {
                segment_identifier seg_id(0,
                            multi_index, ring_index, index);

                typedef
                relate
                    <
                        segment_type, box_segment_type, IntersectionPoints
                    > relater;

                // Todo: depending on code some relations can be left out
                relater::apply(segment, left, seg_id,
                        segment_identifier(1, -1, -1, 0),
                        intersection_points, non_trivial);
                relater::apply(segment, top, seg_id,
                        segment_identifier(1, -1, -1, 1),
                        intersection_points, non_trivial);
                relater::apply(segment, right, seg_id,
                        segment_identifier(1, -1, -1, 2),
                        intersection_points, non_trivial);
                relater::apply(segment, bottom, seg_id,
                        segment_identifier(1, -1, -1, 3),
                        intersection_points, non_trivial);

            }
        }
    }


    template<std::size_t Index, typename Point>
    static inline int get_side(Box const& box, Point const& point)
    {
        // Note: border has to be included because of boundary cases

        if (get<Index>(point) <= get<min_corner, Index>(box)) return -1;
        else if (get<Index>(point) >= get<max_corner, Index>(box)) return 1;
        else return 0;
    }


};


}} // namespace detail::get_intersection_points
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename GeometryTag1, typename GeometryTag2,
    typename Geometry1, typename Geometry2,
    typename IntersectionPoints

>
struct get_intersection_points
{
};


template<typename Polygon, typename Box, typename IntersectionPoints>
struct get_intersection_points
    <
        polygon_tag, box_tag,
        Polygon, Box,
        IntersectionPoints
    >
{

    static inline bool apply(Polygon const& polygon, Box const& box,
            IntersectionPoints& intersection_points)
    {
        typedef typename ggl::ring_type<Polygon>::type ring_type;

        typedef typename boost::range_const_iterator
            <
                typename interior_type<Polygon>::type
            >::type iterator_type;


        typedef detail::get_intersection_points::get_ips_cs
            <ring_type, Box, IntersectionPoints> intersector_type;

        bool non_trivial = false;
        intersector_type::apply(ggl::exterior_ring(polygon),
            -1, -1, box, intersection_points, non_trivial);

        int i = 0;
        for (iterator_type it = boost::begin(interior_rings(polygon));
             it != boost::end(interior_rings(polygon));
             ++it, ++i)
        {
            intersector_type::apply(*it, -1, i, box, intersection_points, non_trivial);
        }

        return non_trivial;
    }
};

template<typename Ring1, typename Ring2, typename IntersectionPoints>
struct get_intersection_points
    <
        ring_tag, ring_tag,
        Ring1, Ring2,
        IntersectionPoints
    >
    : detail::get_intersection_points::get_ips_generic<Ring1, Ring2, IntersectionPoints>
{};


template<typename Polygon1, typename Polygon2, typename IntersectionPoints>
struct get_intersection_points
    <
        polygon_tag, polygon_tag,
        Polygon1, Polygon2,
        IntersectionPoints
    >
    : detail::get_intersection_points::get_ips_generic<Polygon1, Polygon2, IntersectionPoints>
{};

template<typename LineString1, typename LineString2, typename IntersectionPoints>
struct get_intersection_points
    <
        linestring_tag, linestring_tag,
        LineString1, LineString2,
        IntersectionPoints
    >
    : detail::get_intersection_points::get_ips_generic<LineString1, LineString2, IntersectionPoints>
{};

template
<
    typename GeometryTag1, typename GeometryTag2,
    typename Geometry1, typename Geometry2,
    typename IntersectionPoints
>
struct get_intersection_points_reversed
{
    static inline bool apply(Geometry1 const& g1, Geometry2 const& g2,
            IntersectionPoints& intersection_points)
    {
        return get_intersection_points
            <
                GeometryTag2, GeometryTag1,
                Geometry2, Geometry1,
                IntersectionPoints
            >::apply(g2, g1, intersection_points);
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



/*!
    \brief Calculate if two geometries are get_intersection_points
    \ingroup get_intersection_points
    \tparam Geometry1 first geometry type
    \tparam Geometry2 second geometry type
    \tparam V type of intersection container (e.g. vector of "intersection_point"'s
    \param geometry1 first geometry
    \param geometry2 second geometry
 */
template <typename Geometry1, typename Geometry2, typename IntersectionPoints>
inline bool get_intersection_points(const Geometry1& geometry1,
            const Geometry2& geometry2, IntersectionPoints& intersection_points)
{
    assert_dimension_equal<Geometry1, Geometry2>();

    typedef typename boost::remove_const<Geometry1>::type ncg1_type;
    typedef typename boost::remove_const<Geometry2>::type ncg2_type;

    return boost::mpl::if_c
        <
            reverse_dispatch<Geometry1, Geometry2>::type::value,
            dispatch::get_intersection_points_reversed
            <
                typename tag<ncg1_type>::type,
                typename tag<ncg2_type>::type,
                ncg1_type,
                ncg2_type,
                IntersectionPoints
            >,
            dispatch::get_intersection_points
            <
                typename tag<ncg1_type>::type,
                typename tag<ncg2_type>::type,
                ncg1_type,
                ncg2_type,
               IntersectionPoints
            >
        >::type::apply(geometry1, geometry2, intersection_points);
}


} // namespace ggl

#endif // GGL_ALGORITHMS_GET_INTERSECTION_POINTS_HPP
