// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_OVERLAY_TRAVERSE_HPP
#define GGL_ALGORITHMS_OVERLAY_TRAVERSE_HPP

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/core/access.hpp>
#include <ggl/core/coordinate_dimension.hpp>

#include <ggl/algorithms/overlay/copy_segments.hpp>

#ifdef GGL_DEBUG_INTERSECTION
#include <ggl/io/wkt/write_wkt.hpp>
#endif


namespace ggl
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace intersection {


const int VISIT_NONE = 0;
const int VISIT_START = 1;
const int VISIT_VISITED = 2;
const int VISIT_FINISH = 3;
const int VISIT_WITHIN = 4;





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
                    ggl::copy_segments(g1, it->seg_id,
                            it->travels_to_vertex_index,
                            current_output);
                }
                else
                {
                    ggl::copy_segments(g2, it->seg_id,
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




}} // namespace detail::intersection
#endif // DOXYGEN_NO_DETAIL





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
        if (it->visit_code == detail::intersection::VISIT_NONE)
        {
            current_output.clear();

            current_output.push_back(it->point);
            it->visit_code = detail::intersection::VISIT_START;
            ip_iterator current = it;
            unsigned int i = 0;
            do
            {
#ifdef GGL_DEBUG_INTERSECTION
                std::cout << "traverse: " << *current;
#endif

                detail::intersection::select_next_ip(geometry1, geometry2, direction,
                            intersection_points, current, current_output);
                current_output.push_back(current->point);
                current->visit_code = detail::intersection::VISIT_VISITED;
                BOOST_ASSERT(i++ <= intersection_points.size());
            } while (current != it);// && i++ < intersection_points.size());
            it->visit_code = detail::intersection::VISIT_FINISH;
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
