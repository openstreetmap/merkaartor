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
#include <ggl/extensions/gis/io/wkt/write_wkt.hpp>
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
    typename IntersectionPoint,
    typename IntersectionInfo
>
struct on_direction
{
    on_direction(IntersectionPoint const& ip, int direction)
        : m_ip(ip)
        , m_direction(direction)
    {}

    // TEMP: convenient COPY of side
    template <typename P1, typename P2, typename P>
    static inline int side(P1 const& p1, P2 const& p2, P const& p)
    {
        typedef typename select_coordinate_type<P, P1>::type T;

        T dx = get<0>(p2) - get<0>(p1);
        T dy = get<1>(p2) - get<1>(p1);
        T dpx = get<0>(p) - get<0>(p1);
        T dpy = get<1>(p) - get<1>(p1);
        T product =  dx * dpy - dy * dpx;
        return product > 0 ? 1 : product < 0 ? -1 : 0;
    }

    inline bool operator()(IntersectionInfo const& first, IntersectionInfo const& second) const
    {
        int dir = side(m_ip, first->other_point, second->other_point);
        return m_direction == dir;
    }

private :
    IntersectionPoint const& m_ip;
    int m_direction;
};


template
<
    typename GeometryOut,
    typename G1,
    typename G2,
    typename IntersectionPoints,
    typename IntersectionInfo
>
inline bool assign_next_ip(G1 const& g1, G2 const& g2, int direction,
            IntersectionPoints& intersection_points,
            typename boost::range_iterator<IntersectionPoints>::type & ip,
            GeometryOut& current_output,
            IntersectionInfo & info)
{
    info.visit_code = VISIT_VISITED;

#ifdef GGL_DEBUG_INTERSECTION
    std::cout << " take: " << info << std::endl;
#endif

    // If there is no next IP on this segment
    if (info.next_ip_index < 0)
    {
        if (info.seg_id.source_index == 0)
        {
            ggl::copy_segments(g1, info.seg_id,
                    info.travels_to_vertex_index,
                    current_output);
        }
        else
        {
            ggl::copy_segments(g2, info.seg_id,
                    info.travels_to_vertex_index,
                    current_output);
        }
        ip = boost::begin(intersection_points) + info.travels_to_ip_index;
    }
    else
    {
        ip = boost::begin(intersection_points) + info.next_ip_index;
    }
    current_output.push_back(ip->point);

    return true;
}

template <typename Info>
inline bool turning(Info const& info, int direction)
{
    // If it is turning in specified direction (RIGHT for intersection,
    // LEFT for union, and NOT arriving at that point
    return info.direction == direction
        && info.arrival != 1
        //&& (! (info.how == 'a' && info.direction != 0))
        ;
}

template
<
    typename GeometryOut,
    typename G1,
    typename G2,
    typename IntersectionPoints
>
inline bool select_next_ip_trivial(G1 const& g1, G2 const& g2, int direction,
            IntersectionPoints& intersection_points,
            typename boost::range_iterator<IntersectionPoints>::type & ip,
            GeometryOut& current_output)
{
    // Check all intersecting segments on this IP:
    typedef typename boost::range_value<IntersectionPoints>::type ip_type;
    typedef typename ip_type::traversal_vector tv;
    typedef typename boost::range_iterator<tv>::type tit_type;

    for (tit_type it = boost::begin(ip->info); it != boost::end(ip->info); ++it)
    {
        if (turning(*it, direction))
        {
            return assign_next_ip(g1, g2, direction,
                        intersection_points, ip, current_output, *it);
        }
    }

    return false;
}


template
<
    typename GeometryOut,
    typename G1,
    typename G2,
    typename IntersectionPoints
>
inline bool select_next_ip_with_sorting(G1 const& g1, G2 const& g2,
            int direction,
            IntersectionPoints& intersection_points,
            typename boost::range_iterator<IntersectionPoints>::type & ip,
            GeometryOut& current_output)
{

    typedef typename boost::range_value<IntersectionPoints>::type ip_type;
    typedef typename ip_type::traversal_vector tv;
    typedef typename boost::range_iterator<tv>::type tit_type;
    typedef typename ip_type::traversal_type info_type;

    std::vector<info_type*> info;
    for (tit_type it = boost::begin(ip->info); it != boost::end(ip->info); ++it)
    {
        if (turning(*it, direction))
        {
            info.push_back(&(*it));
        }
    }

    // If there are no intersection points, fall-back to collinear cases or
    // if already in that case, return false.
    if (boost::size(info) == 0)
    {
        return direction == 0
            ? false
            : select_next_ip_with_sorting(g1, g2, 0,
                            intersection_points, ip, current_output);
    }

    // For one IP, it is easy: take that one.
    if (boost::size(info) == 1)
    {
        return assign_next_ip(g1, g2, direction,
                    intersection_points, ip, current_output, *info.front());
    }

    // In case of direction 0, also take first one
    // TODO: sort this vector somehow, there are more rows, it is too
    // arbitrary to take first one (though working well)
    if (direction == 0)
    {
        return assign_next_ip(g1, g2, direction,
                    intersection_points, ip, current_output, *info.front());
    }


    // For more, sort the information on direction, take the most left / right one
    //std::cout << " " << boost::size(info);
    std::sort(info.begin(), info.end(), on_direction<ip_type, info_type*>(*ip, direction));
    return assign_next_ip(g1, g2, direction, intersection_points, ip, current_output, *info.back());
}

template
<
    typename GeometryOut,
    typename G1,
    typename G2,
    typename IntersectionPoints
>
inline bool select_next_ip(G1 const& g1, G2 const& g2, int direction,
            IntersectionPoints& intersection_points,
            typename boost::range_iterator<IntersectionPoints>::type & ip,
            GeometryOut& current_output)
{
    if (ip->trivial)
    {
        return select_next_ip_trivial(g1, g2, direction, intersection_points,
                ip, current_output);
    }
    else
    {
        return select_next_ip_with_sorting(g1, g2, direction, intersection_points,
                ip, current_output);
    }
}


template<typename IntersectionPoint>
inline bool is_starting_point(IntersectionPoint const& ip, int direction)
{
    for (typename IntersectionPoint::traversal_vector::const_iterator it
        = boost::begin(ip.info); it != boost::end(ip.info); ++it)
    {
        if (it->direction == direction
            && it->arrival != 1)
        {
            return true;
        }
    }
    return false;
}


template <typename Container>
inline void stop_gracefully(Container& container, bool& stop,
            std::string const& reason)
{
#ifdef GGL_DEBUG_INTERSECTION
    std::cout << "STOPPING: " << reason << std::endl;
#endif

    stop = true;
    if (container.size() > 0)
    {
        container.push_back(container.front());
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
            bool trivial,
            OutputIterator out)
{
    typedef typename boost::range_iterator
                <IntersectionPoints>::type ip_iterator;

    typedef typename boost::range_value<IntersectionPoints>::type ip_type;
    typedef typename ip_type::traversal_vector tv;
    typedef typename boost::range_iterator<tv>::type tit_type;
    typedef typename ip_type::traversal_type info_type;



    GeometryOut current_output;


    // Iterate through all unvisited points
    for (ip_iterator it = boost::begin(intersection_points);
        it != boost::end(intersection_points);
        ++it)
    {
#ifdef GGL_DEBUG_INTERSECTION
        std::cout << "TRY traversal: " << *it;
#endif

        if (it->visit_code == detail::intersection::VISIT_NONE
            // UNION may operate on non-starting points, but INTERSECTION may not.
            // TODO: re-evaluate that
            && (direction == 1
            || detail::intersection::is_starting_point(*it, direction)
                )
            )
        {
            for (tit_type iit = boost::begin(it->info);
                iit != boost::end(it->info);
                ++iit)
            {
                if (iit->arrival == -1
                    && iit->visit_code == detail::intersection::VISIT_NONE
                    && iit->direction == direction)
                {
                    it->visit_code = detail::intersection::VISIT_START;
                    iit->visit_code = detail::intersection::VISIT_START;

                    current_output.push_back(it->point);

                    ip_iterator current = it;

#ifdef GGL_DEBUG_INTERSECTION
                    std::cout << "START traversal: " << *current;
#endif

                    detail::intersection::assign_next_ip(geometry1, geometry2,
                                direction,
                                intersection_points,
                                current, current_output, *iit);

                    std::vector<segment_identifier> segments;
                    segments.push_back(iit->seg_id);

                    unsigned int i = 0;
                    bool stop = false;

                    while (current != it && ! stop)
                    {
#ifdef GGL_DEBUG_INTERSECTION
                        std::cout << "traverse: " << *current;
#endif

                        // We assume clockwise polygons only, non self-intersecting, closed.
                        // However, the input might be different, and checking validity
                        // is up to the library user.

                        // Therefore we make here some sanity checks. If the input
                        // violates the assumptions, the output polygon will not be correct
                        // but the routine will stop and output the current polygon, and
                        // will continue with the next one.

                        // Below three reasons to stop.
                        if (! detail::intersection::select_next_ip(geometry1,
                                    geometry2, direction,
                                    intersection_points,
                                    current, current_output))
                        {
                            // Should not occur in valid (non-self-intersecting) polygons
                            // Should not occur in self-intersecting polygons without spikes
                            // Might occur in polygons with spikes
                            detail::intersection::stop_gracefully(
                                current_output, stop, "Dead end");
                        }

                        if (current->visit_code == detail::intersection::VISIT_VISITED)
                        {
                            // It visits a visited node again, without passing the start node.
                            // This makes it suspicious for endless loops
                            // Check if it is really same node
                            detail::intersection::stop_gracefully(
                                current_output, stop, "Visit again");
                        }


                        if (i++ > intersection_points.size())
                        {
                            // Sanity check: there may be never more loops
                            // than intersection points.
                            detail::intersection::stop_gracefully(
                                current_output, stop, "Endless loop");
                        }

                        current->visit_code = detail::intersection::VISIT_VISITED;
                    }

                    iit->visit_code = detail::intersection::VISIT_FINISH;

#ifdef GGL_DEBUG_INTERSECTION
                    std::cout << "finish: " << *current;
                    std::cout << ggl::wkt(current_output) << std::endl;
#endif

                    *out = current_output;
                    ++out;
                    current_output.clear();
                }
            }
            it->visit_code = detail::intersection::VISIT_FINISH;
        }
    }
}


} // namespace ggl

#endif // GGL_ALGORITHMS_OVERLAY_TRAVERSE_HPP
