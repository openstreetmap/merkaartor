// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_ENRICH_INTERSECTION_POINTS_HPP
#define GGL_ALGORITHMS_ENRICH_INTERSECTION_POINTS_HPP

#include <algorithm>
#include <map>


#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>



namespace ggl
{


#ifndef DOXYGEN_NO_IMPL
namespace impl { namespace intersection {


#ifdef GGL_DEBUG_INTERSECTION

template <typename M>
inline void report_map(M const& map)
{
    std::cout << "Map [source,segment(,ring,multi)] -> [count]:" << std::endl;
    for (typename M::const_iterator it = map.begin(); it != map.end(); it++)
    {
        std::cout << "(" << it->first << ")" << " -> " << "(" << it->second << ")"
            << std::endl;
    }
}


template <typename V>
inline void report_ip(V const& intersection_points)
{
    typedef typename boost::range_const_iterator<V>::type iterator;
    for (iterator it = boost::begin(intersection_points);
            it != boost::end(intersection_points);
            ++it)
    {
        std::cout << *it;
    }
}

template <typename V>
inline void report_indexed(V const& index)
{
    typedef typename boost::range_const_iterator<V>::type iterator;
    for (iterator it = boost::begin(index); it != boost::end(index); ++it)
    {
        std::cout << it->index << ": " << it->subject;
    }
}

#endif






template <typename T>
struct indexed_source_segment
{
    std::size_t index;
    T subject;
    inline indexed_source_segment() {}

    inline indexed_source_segment(std::size_t i, T const& s,
            segment_identifier const& seg_id)
        : index(i)
    {
        s.clone_except_info(subject);
        typedef typename T::traversal_vector tv;
        for (typename boost::range_const_iterator<tv>::type it = s.info.begin();
            it != s.info.end(); ++it)
        {
            // Copy info-record if belonging to source and to segment
            if (it->seg_id == seg_id)
            {
                subject.info.push_back(*it);
            }
        }
    }
};


template <typename T>
struct indexed_source
{
    std::size_t index;
    T subject;
    inline indexed_source() {}

    inline indexed_source(std::size_t i, T const& s, 
                    int source_index, int multi_index, int ring_index)
        : index(i)
    {
        s.clone_except_info(subject);
        typedef typename T::traversal_vector tv;
        for (typename boost::range_const_iterator<tv>::type it = s.info.begin();
            it != s.info.end(); ++it)
        {
            // Copy info-record if belonging to source
            if (it->seg_id.source_index == source_index
                && it->seg_id.multi_index == multi_index
                && it->seg_id.ring_index == ring_index
                )
            {
                subject.info.push_back(*it);
            }
        }
    }
};


template <typename Indexed>
struct sort_on_distance
{
    inline bool operator()(Indexed const& left, Indexed const& right) const
    {
        // Sanity check, there should be info-records because only those are copied
        BOOST_ASSERT (left.subject.info.size() > 0 && right.subject.info.size() > 0);

        return left.subject.info.front().distance
            < right.subject.info.front().distance;
    }
};

// Sorts on segment + ring_index + multi_index
template <typename Indexed>
struct sort_on_segment_identifier
{
    inline bool operator()(Indexed const& left, Indexed const& right) const
    {
        // Sanity check
        BOOST_ASSERT (left.subject.info.size() > 0 && right.subject.info.size() > 0);

        segment_identifier const& sl = left.subject.info.front().seg_id;
        segment_identifier const& sr = right.subject.info.front().seg_id;

        return sl == sr
            ? left.subject.info.front().distance
              < right.subject.info.front().distance
            : sl < sr;
    }
};






// Assigns IP[index] . info[source/multi/ring/segment] . next_ip_index
template <typename V>
static inline void assign_next_ip_index(V& intersection_points, int index,
            segment_identifier const& seg_id,
            int next_ip_index)
{
    typedef typename boost::range_value<V>::type ip_type;
    typedef typename ip_type::traversal_vector tv;
    typedef typename boost::range_iterator<tv>::type iterator_type;

    ip_type& ip = intersection_points[index];

    for (iterator_type it = boost::begin(ip.info);
            it != boost::end(ip.info);
            ++it)
    {
        if (it->seg_id == seg_id)
        {
            it->next_ip_index = next_ip_index;
            return;
        }
    }
}


// Assigns IP[index] . info[source_index] . travels_to_[vertex,ip]_index
template <typename V>
static inline void assign_last_vertex(V& intersection_points, int index,
            int source_index,
            int travels_to_vertex_index, int travels_to_ip_index)
{
    typedef typename boost::range_value<V>::type ip_type;
    typedef typename ip_type::traversal_vector tv;
    typedef typename boost::range_iterator<tv>::type iterator_type;

    ip_type& ip = intersection_points[index];

    for (iterator_type it = boost::begin(ip.info);
            it != boost::end(ip.info);
            ++it)
    {
        if (it->seg_id.source_index == source_index)
        {
            it->travels_to_vertex_index = travels_to_vertex_index;
            it->travels_to_ip_index = travels_to_ip_index;
            // do not return here, there can be more than one
        }
    }
}


// Creates selection of IP-s of only this unique source/segment,
// then sorts on distance,
// then assigns for each IP which is the next IP on this segment.
// This is only applicable (but essential) for segments having
// more than one IP on it. It is not the usual situation, so not
// computational intensive.
template <typename V>
static inline bool assign_next_points(V& intersection_points,
            segment_identifier const& seg_id)
{
    typedef typename boost::range_value<V>::type ip_type;
    typedef typename boost::range_const_iterator<V>::type iterator_type;
    typedef indexed_source_segment<ip_type> indexed_type;
    typedef typename ip_type::traversal_vector tv;
    typedef typename boost::range_const_iterator<tv>::type tvit_type;

    // Create a copy of all IP's on this segment from this source

    std::vector<indexed_type> copy;
    copy.reserve(intersection_points.size());
    std::size_t index = 0;
    for (iterator_type it = boost::begin(intersection_points);
            it != boost::end(intersection_points);
            ++it, ++index)
    {
        bool to_be_copied = false;
        for (tvit_type tvit = boost::begin(it->info);
            ! to_be_copied && tvit != boost::end(it->info);
                ++tvit)
        {
            if (tvit->seg_id == seg_id)
            {
                to_be_copied = true;
            }
        }

        if (to_be_copied)
        {
            // Copy this row, plus ONLY the related information
            copy.push_back(indexed_type(index, *it, seg_id));
        }
    }

    // Normally there are more elements in "copy".
    // But in case of merges there could be only one.
    if (boost::size(copy) <= 1)
    {
        return false;
    }

    std::sort(copy.begin(), copy.end(), sort_on_distance<indexed_type>());


    // Now that it is sorted, do the main purpose: assign the next points
    typedef typename boost::range_const_iterator
        <
            std::vector<indexed_type>
        >::type indexed_iterator_type;
    indexed_iterator_type it = boost::begin(copy);
    for (indexed_iterator_type prev = it++;
        it != boost::end(copy); prev = it++)
    {
        for (tvit_type tvit = boost::begin(it->subject.info);
                tvit != boost::end(it->subject.info);
                ++tvit)
        {
            if (tvit->seg_id == seg_id)
            {
                assign_next_ip_index(intersection_points, prev->index, 
                            seg_id, it->index);
            }
        }
    }

#ifdef GGL_DEBUG_INTERSECTION
    std::cout << "Sorted (distance " << seg_id << "): " << std::endl;
    report_indexed(copy);
#endif

    return true;
}


// If a segment has more than one IP, we determine what is the next IP
// on that segment
template <typename M, typename V>
static inline bool assign_next_points(M& map, V& intersection_points)
{
    bool assigned = false;
    for (typename M::iterator mit = map.begin(); mit != map.end(); ++mit)
    {
        // IF there are more IP's on this segment
        if (mit->second > 1)
        {
            if (assign_next_points(intersection_points, mit->first))
            {
                assigned = true;
            }
        }
    }
    return assigned;
}



template <typename V>
static inline bool assign_order(V& intersection_points, 
            int source_index, int multi_index, int ring_index)
{
    typedef typename boost::range_value<V>::type ip_type;
    typedef typename boost::range_const_iterator<V>::type iterator_type;
    typedef indexed_source<ip_type> indexed_type;
    typedef typename ip_type::traversal_vector tv;
    typedef typename boost::range_const_iterator<tv>::type tvit_type;

    // Create a copy of all IP's from this source
    std::vector<indexed_type> copy;
    copy.reserve(intersection_points.size());
    std::size_t index = 0;
    for (iterator_type it = boost::begin(intersection_points);
            it != boost::end(intersection_points);
            ++it, ++index)
    {
        bool to_be_copied = false;
        for (tvit_type tvit = boost::begin(it->info);
            ! to_be_copied && tvit != boost::end(it->info);
                ++tvit)
        {
            if (tvit->seg_id.source_index == source_index
                && tvit->seg_id.multi_index == multi_index
                && tvit->seg_id.ring_index == ring_index
                )
            {
                to_be_copied = true;
            }
        }

        if (to_be_copied)
        {
            // Copy this row, plus ONLY the related information
            copy.push_back(indexed_type(index, *it, source_index, multi_index, ring_index));
        }
    }

#ifdef GGL_DEBUG_INTERSECTION
    std::cout << "Ordered/copy (segment "
        << " src: " << source_index
        << "): " << std::endl;
    report_indexed(copy);
#endif

    std::sort(copy.begin(), copy.end(), 
                sort_on_segment_identifier<indexed_type>());

    typedef typename boost::range_const_iterator<std::vector<indexed_type> >::type iit_type;


    // Now that it is sorted, do the main purpose:
    // assign travel-to-vertex/ip index for each IP
    // Because IP's are circular, PREV starts at the very last one,
    // being assigned from the first one.
    iit_type it = boost::begin(copy);
    for (iit_type prev = it + (boost::size(copy) - 1);
            it != boost::end(copy);
            prev = it++)
    {
        for (tvit_type tvit = boost::begin(it->subject.info);
                tvit != boost::end(it->subject.info);
                ++tvit)
        {
            if (tvit->seg_id.source_index == source_index
                    && tvit->seg_id.multi_index == multi_index
                    && tvit->seg_id.ring_index == ring_index)
            {
                assign_last_vertex(intersection_points, prev->index,
                    source_index,
                    tvit->seg_id.segment_index, it->index);
            }
        }
    }


#ifdef GGL_DEBUG_INTERSECTION
    std::cout << "Ordered (segment " << " src: "
                << source_index << "): " << std::endl;
    report_ip(intersection_points);
#endif


    return true;
}

template <typename M, typename V>
static inline void assign_order(M const& map, V& intersection_points)
{
    typename M::const_iterator prev;
    bool first = true;
    for (typename M::const_iterator mit = map.begin(); mit != map.end(); ++mit)
    {
        if (first 
            || prev->first.source_index != mit->first.source_index
            || prev->first.ring_index != mit->first.ring_index
            || prev->first.multi_index != mit->first.multi_index
                )
        {
            assign_order(intersection_points, 
                    mit->first.source_index, 
                    mit->first.multi_index, 
                    mit->first.ring_index);
            first = false;
        }
        prev = mit;
    }
}



}} // namespace impl::intersection
#endif //DOXYGEN_NO_IMPL




/*!
    \brief All intersection points are enriched by their successor
    \ingroup intersection
    \param has_collinear Boolean flag to indicate that there are collinear IP's. If false,
       it knows that it can skip some tests.
 */
template <typename V>
inline void enrich_intersection_points(V& intersection_points, bool has_collinear)
{
    // Create a map of segment<source_index,segment_index,ring_index,multi_index>
    // to <number of IP's on this segment, index of first IP>
    // Purpose: count IP's per source/segment
    std::map<impl::intersection::segment_identifier, int> map; //std::pair<int, int> > map;


    typedef typename boost::range_const_iterator<V>::type iterator_type;
    int ip_index = 0;
    for (iterator_type it = boost::begin(intersection_points);
            it != boost::end(intersection_points);
            ++it, ++ip_index)
    {
        typedef typename boost::range_value<V>::type::traversal_vector tv;
        typedef typename boost::range_const_iterator<tv>::type tvit_type;
        for (tvit_type tvit = boost::begin(it->info);
                tvit != boost::end(it->info);
                ++tvit)
        {
            map[tvit->seg_id]++;
        }
    }

#ifdef GGL_DEBUG_INTERSECTION
    impl::intersection::report_map(map);
#endif

    if (impl::intersection::assign_next_points(map, intersection_points))
    {
#ifdef GGL_DEBUG_INTERSECTION
        std::cout << "Enriched: " << std::endl;
        impl::intersection::report_ip(intersection_points);
#endif
    }

    impl::intersection::assign_order(map, intersection_points);
}




} // namespace ggl

#endif // GGL_ALGORITHMS_ENRICH_INTERSECTION_POINTS_HPP
