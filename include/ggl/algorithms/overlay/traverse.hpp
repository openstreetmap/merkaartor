// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_OVERLAY_TRAVERSE_HPP
#define GGL_ALGORITHMS_OVERLAY_TRAVERSE_HPP

#include <vector>

#include <boost/mpl/if.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/core/access.hpp>
#include <ggl/core/coordinate_dimension.hpp>
#include <ggl/core/reverse_dispatch.hpp>

#include <ggl/iterators/ever_circling_iterator.hpp>

#ifdef GGL_DEBUG_INTERSECTION
#include <ggl/io/wkt/write_wkt.hpp>
#endif


namespace ggl
{


#ifndef DOXYGEN_NO_IMPL
namespace impl { namespace intersection {


const int VISIT_NONE = 0;
const int VISIT_START = 1;
const int VISIT_VISITED = 2;
const int VISIT_FINISH = 3;
const int VISIT_WITHIN = 4;



template <typename Tag, typename GeometryIn, typename GeometryOut>
struct copy_segments
{
};


template <typename Ring, typename GeometryOut>
struct copy_segments<ring_tag, Ring, GeometryOut>
{
    static inline void apply(Ring const& ring,
            segment_identifier const& seg_id, int to_index,
            GeometryOut& current_output)
    {
        typedef typename ggl::point_const_iterator<Ring>::type iterator;
        typedef ggl::ever_circling_iterator<iterator> ec_iterator;

        // The problem: sometimes we want to from "3" to "2" -> end = "3" -> end == begin
        // This is not convenient with iterators.

        // So we use the ever-circling iterator and determine when to step out

        int from_index = seg_id.segment_index + 1;
        ec_iterator it(boost::begin(ring), boost::end(ring),
                    boost::begin(ring) + from_index);

        // [2..4] -> 4 - 2 + 1 = 3 -> {2,3,4} -> OK
        // [4..2],size=6 -> 6 - 4 + 2 + 1 = 5 -> {4,5,0,1,2} -> OK
        int count = from_index <= to_index
            ? to_index - from_index + 1
            : boost::size(ring) - from_index + to_index + 1;

        for (int i = 0; i < count; ++i, ++it)
        {
            current_output.push_back(*it);
#ifdef GGL_DEBUG_INTERSECTION
            std::cout << ggl::wkt(*it) << std::endl;
#endif
        }
    }
};

template <typename Polygon, typename GeometryOut>
struct copy_segments<polygon_tag, Polygon, GeometryOut>
{
    static inline void apply(Polygon const& polygon,
            segment_identifier const& seg_id, int to_index,
            GeometryOut& current_output)
    {
        // Just call the ring-version with the right ring
        copy_segments
            <
                ring_tag,
                typename ggl::ring_type<Polygon>::type,
                GeometryOut
            >::apply
                (
                    seg_id.ring_index < 0
                    ? ggl::exterior_ring(polygon)
                    : ggl::interior_rings(polygon)[seg_id.ring_index],
                    seg_id, to_index, current_output
                );
    }
};




template
<
    typename GeometryOut,
    typename G1,
    typename G2,
    typename IntersectionPoints
>
inline void select_next_ip(G1 const& g1, G2 const& g2, int direction,
            IntersectionPoints& intersection_points,
            typename boost::range_iterator<IntersectionPoints>::type & ip,
            GeometryOut& current_output)
{
    // Check all intersecting segments on this IP:
    typedef typename boost::range_value<IntersectionPoints>::type ip_type;
    typedef typename ip_type::traversal_vector tv;
    typedef typename boost::range_const_iterator<tv>::type tit_type;
    for (tit_type it = boost::begin(ip->info); it != boost::end(ip->info); ++it)
    {
        // If it is turning in specified direction (RIGHT for intersection,
        // LEFT for union)
        if (it->direction == direction
            && it->arrival != 1
            )
        {
            // If there is no next IP on this segment
            if (it->next_ip_index < 0)
            {
                if (it->seg_id.source_index == 0)
                {
                    copy_segments
                        <
                            typename ggl::tag<G1>::type,
                            G1,
                            GeometryOut
                        >::apply(g1, it->seg_id,
                            it->travels_to_vertex_index,
                            current_output);
                }
                else
                {
                    copy_segments
                        <
                            typename ggl::tag<G2>::type,
                            G2,
                            GeometryOut
                        >::apply(g2, it->seg_id,
                            it->travels_to_vertex_index,
                            current_output);
                }
                ip = boost::begin(intersection_points) + it->travels_to_ip_index;
            }
            else
            {
                ip = boost::begin(intersection_points) + it->next_ip_index;
            }
            return;
        }
    }

}




}} // namespace impl::intersection
#endif // DOXYGEN_NO_IMPL





/*!
    \brief Traverses through intersection points / geometries
    \ingroup overlay
 */
template
<
    typename GeometryOut,
    typename Geometry1,
    typename Geometry2,
    typename IntersectionPoints,
    typename OutputIterator
>
inline void traverse(Geometry1 const& geometry1,
            Geometry2 const& geometry2, int direction,
            IntersectionPoints& intersection_points,
            OutputIterator out)
{
    typedef typename boost::range_iterator
                <IntersectionPoints>::type ip_iterator;


    GeometryOut current_output;

    // Iterate through all unvisited points
    for (ip_iterator it = boost::begin(intersection_points);
        it != boost::end(intersection_points);
        ++it)
    {
        if (it->visit_code == impl::intersection::VISIT_NONE)
        {
            current_output.clear();

            current_output.push_back(it->point);
            it->visit_code = impl::intersection::VISIT_START;
            ip_iterator current = it;
            unsigned int i = 0;
            do
            {
#ifdef GGL_DEBUG_INTERSECTION
                std::cout << "traverse: " << *current;
#endif

                impl::intersection::select_next_ip(geometry1, geometry2, direction,
                            intersection_points, current, current_output);
                current_output.push_back(current->point);
                current->visit_code = impl::intersection::VISIT_VISITED;
                BOOST_ASSERT(i++ <= intersection_points.size());
            } while (current != it);// && i++ < intersection_points.size());
            it->visit_code = impl::intersection::VISIT_FINISH;
            *out = current_output;
            ++out;

#ifdef GGL_DEBUG_INTERSECTION
            std::cout << "finish: " << *current;
            std::cout << ggl::wkt(current_output) << std::endl;
#endif
        }
    }
}


} // namespace ggl

#endif // GGL_ALGORITHMS_OVERLAY_TRAVERSE_HPP
