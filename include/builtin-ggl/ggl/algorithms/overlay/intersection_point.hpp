// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_INTERSECTION_POINT_HPP
#define GGL_ALGORITHMS_INTERSECTION_POINT_HPP

#include <vector>


#include <ggl/core/access.hpp>
#include <ggl/core/coordinate_dimension.hpp>

#include <ggl/strategies/distance_result.hpp>
#include <ggl/strategies/strategy_traits.hpp>

#include <ggl/algorithms/overlay/segment_identifier.hpp>


namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace intersection {


template<typename P>
struct intersection_info
{
    typedef P point_type;
    typedef typename distance_result<P, P>::type distance_type;

    inline intersection_info()
        : travels_to_vertex_index(-1)
        , travels_to_ip_index(-1)
        , next_ip_index(-1)
        , distance(ggl::make_distance_result<distance_type>(0))
        , direction(0)
        , how('?')
        , arrival(0)
        , opposite(false)
        , visit_code(0)
        , flagged(false)
    {}

    // Point to which the segment from IP is directing (TO-point)
    // If they intersect on their "arrival" points, it is the FROM-point.
    P other_point;

    // Identifier of this segment (source,segment,ring,multi)
    segment_identifier seg_id;

    // Identify the segment where it was intersected with to form this IP
    segment_identifier other_id;


    // vertex to which is free travel after this IP,
    // so from "segment_index+1" to "travels_to_vertex_index", without IP-s,
    // can be -1
    int travels_to_vertex_index;

    // same but now IP index, so "next IP index" but not on THIS segment
    int travels_to_ip_index;

    // index of next IP on this segment, -1 if there is no one
    int next_ip_index;

    distance_type distance; // distance-measurement from segment.first to IP

    // 1: left, -1: right, 0: collinear
    int direction;

    // Information about how intersection is done
    char how;

    // 1: arrived at IP, -1: departs from IP, 0: crosses IP
    int arrival;

    bool opposite;

    int visit_code;

    bool flagged; // flagged for deletion

#ifdef GGL_DEBUG_INTERSECTION
        static inline std::string dir(int d)
        {
            return d == 0 ? "-" : (d == 1 ? "L" : d == -1 ? "R" : "#");
        }
        static inline std::string how_str(int h)
        {
            return h == 0 ? "-" : (h == 1 ? "A" : "D");
        }

        friend std::ostream& operator<<(std::ostream &os, intersection_info<P> const& info)
        {
            os  << "\t"
                << " src " << info.seg_id.source_index
                << " seg " << info.seg_id.segment_index
                << " (// " << info.other_id.source_index
                    << "." << info.other_id.segment_index << ")"
                << " how " << info.how
                    << "[" << how_str(info.arrival)
                    << " " << dir(info.direction)
                    << (info.opposite ? " o" : "")
                    << "]"
                << " nxt seg " << info.travels_to_vertex_index
                << " , ip " << info.travels_to_ip_index
                << " , or " << info.next_ip_index
                << " dst " << std::setprecision(12) << double(info.distance);
            if (info.visit_code != 0)
            {
                os << " VIS: " << int(info.visit_code);
            }
            return os;
        }
#endif
};


template<typename P>
struct intersection_point
{
    public :
        inline intersection_point()
            : visit_code(0) // VISIT_NONE
            , trivial(true)
            , shared(false)
            , flagged(false)
        {
        }


#ifdef GGL_DEBUG_INTERSECTION
        friend std::ostream& operator<<(std::ostream &os, intersection_point<P> const& p)
        {
            os << "IP (" << ggl::get<0>(p.point) << "," << ggl::get<1>(p.point) << ")"
                << " visited: " << int(p.visit_code)
                << (p.shared ? " SHARED" : "")
                << (p.flagged ? " FLAGGED" : "")
                << std::endl;

            for (unsigned int i = 0; i < p.info.size(); i++)
            {
                os << p.info[i] << std::endl;
            }
            return os;
        }
#endif
        typedef intersection_info<P> traversal_type;
        typedef std::vector<traversal_type> traversal_vector;

        P point;

        int visit_code;
        bool trivial; // FALSE if there is an collinearity, touch or so.
        bool shared; // shared with more IP's
        bool flagged; // flagged for deletion afterwards

        // info about the two intersecting segments
        // usually two, but often more if IP's are merged
        traversal_vector info;

        inline void clone_except_info(intersection_point& other) const
        {
            other.point = point;
            other.visit_code = visit_code;
            other.trivial = trivial;
            other.shared = shared;
            other.flagged = flagged;
        }
};




}} // namespace detail::intersection
#endif //DOXYGEN_NO_DETAIL


// Register the intersection point as being a point fulfilling the ggl Point Concept
namespace traits
{

    template <typename P>
    struct coordinate_type<ggl::detail::intersection::intersection_point<P> >
    {
        typedef typename ggl::coordinate_type<P>::type type;
    };

    template <typename P>
    struct coordinate_system<ggl::detail::intersection::intersection_point<P> >
    {
        typedef typename ggl::coordinate_system<P>::type type;
    };

    template <typename P>
    struct dimension<ggl::detail::intersection::intersection_point<P> >
        : ggl::dimension<P>
    {};

    template <typename P>
    struct tag<ggl::detail::intersection::intersection_point<P> >
    {
        typedef point_tag type;
    };

    template <typename P>
    struct access<ggl::detail::intersection::intersection_point<P> >
    {
        template <int Index>
        static inline typename coordinate_type<P>::type get(
                ggl::detail::intersection::intersection_point<P> const& p)
        {
            return ggl::get<Index>(p.point);
        }

        template <int Index>
        static inline void set(ggl::detail::intersection::intersection_point<P>& p,
                typename coordinate_type<P>::type const& value)
        {
            ggl::set<Index>(p.point, value);
        }
    };

}


#ifdef GGL_DEBUG_INTERSECTION

template <typename V>
inline void report_ip(V const& intersection_points)
{
    typedef typename V::const_iterator iterator_type;

    for (iterator_type it = intersection_points.begin();
         it != intersection_points.end();
         ++it)
    {
        if (! it->flagged)
        {
            std::cout << *it;
        }
    }
}
#endif // GGL_DEBUG_INTERSECTION


} // namespace ggl

#endif // GGL_ALGORITHMS_INTERSECTION_POINT_HPP
