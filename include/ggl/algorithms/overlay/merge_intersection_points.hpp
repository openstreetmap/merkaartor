// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_MERGE_INTERSECTION_POINTS_HPP
#define GGL_ALGORITHMS_MERGE_INTERSECTION_POINTS_HPP

#include <algorithm>


#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>


#include <ggl/core/coordinate_type.hpp>

#include <ggl/algorithms/equals.hpp>


namespace ggl
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace intersection {

template <typename PointType>
struct on_increasing_dimension
{
    typedef typename ggl::coordinate_type<PointType>::type coordinate_type;

    inline bool operator()(PointType const& lhs, PointType const& rhs) const
    {
        coordinate_type const& left0 = ggl::get<0>(lhs);
        coordinate_type const& right0 = ggl::get<0>(rhs);

        return math::equals(left0, right0)
            ? ggl::get<1>(lhs) < ggl::get<1>(rhs)
            : left0 < right0;
    }
};



// T can be an intersection_point or intersection_info record
template <typename T>
struct is_flagged
{
    inline bool operator()(T const& object) const
    {
        return object.flagged;
    }
};



template <typename V>
inline void remove_collinearities(V& intersection_points)
{
    typedef typename boost::range_iterator<V>::type iterator_type;
    typedef typename boost::range_value<V>::type ip_type;
    typedef typename ip_type::traversal_type info_type;


    for (iterator_type it = boost::begin(intersection_points);
         it != boost::end(intersection_points);
         ++it)
    {
        if (! it->trivial && ! it->flagged)
        {
            // Remove anything having to do with collinearity
            typedef typename boost::range_value<V>::type::traversal_vector vector_type;
            typedef typename boost::range_iterator<vector_type>::type tvit_type;

            bool has_flag = false;

            // Note, this is done n*m, in case of collinearity, but it is only done if not trivial
            // or if there
            bool middle = false;
            for (tvit_type tvit = boost::begin(it->info);
                ! middle && tvit != boost::end(it->info);
                ++tvit)
            {
                if (tvit->how == 'e' || tvit->how == 'c')
                {
                    tvit->flagged = true;
                    has_flag = true;

                    for (tvit_type tvit2 = boost::begin(it->info);
                        tvit2 != boost::end(it->info); ++tvit2)
                    {
                        // Do NOT remove anything starting from collinear, or ending on, in the middle.
                        if (tvit2->how != 'm' && tvit2->how != 's')
                        {
                            if (tvit->seg_id == tvit2->seg_id
                                || tvit->seg_id == tvit2->other_id
                                || tvit->other_id == tvit2->seg_id
                                || tvit->other_id == tvit2->other_id
                                )
                            {
                                tvit2->flagged = true;
                            }
                        }
                        else
                        {
                            tvit->flagged = false;
                            has_flag = false;
                            middle = true;
                        }
                    }
                }
            }

            if (has_flag)
            {
                it->info.erase(
                    std::remove_if(
                            boost::begin(it->info),
                            boost::end(it->info),
                            is_flagged<info_type>()),
                    boost::end(it->info));

                // Mark for deletion afterwards if there are no info-records left
                if (boost::size(it->info) == 0)
                {
                    it->flagged = true;
                }

                // Cases, previously forming an 'angle' (test #19)
                // will be normal (neutral) case now,
                // so to continue traversal:
                if (it->info.size() == 2
                    && it->info.front().how == 'a'
                    && it->info.back().how == 'a')
                {
                    it->info.front().direction = 1;
                    it->info.back().direction = 1;
                }
            }
        }
    }

#ifdef GGL_DEBUG_INTERSECTION
    std::cout << "Removed collinearities: " << std::endl;
    report_ip(intersection_points);
#endif
}





}} // namespace detail::intersection
#endif //DOXYGEN_NO_DETAIL



/*!
    \brief Merges intersection points such that points at the same location will be merged, having one point
        and their info-records appended
    \ingroup overlay
    \tparam IntersectionPoints type of intersection container (e.g. vector of "intersection_point"'s)
    \param intersection_points container containing intersectionpoints
 */
template <typename IntersectionPoints>
inline void merge_intersection_points(IntersectionPoints& intersection_points)
{
    typedef typename boost::range_value<IntersectionPoints>::type trav_type;

    if (boost::size(intersection_points) <= 1)
    {
        return;
    }


    // Sort all IP's from left->right, ymin->ymax such that
    // all same IP's are consecutive
    // (and we need this order lateron again)
    // This order is NOT changed here and should not be after
    // (otherwise indexes are wrong)
    std::sort(boost::begin(intersection_points),
        boost::end(intersection_points),
        detail::intersection::on_increasing_dimension<trav_type>());

    typedef typename boost::range_iterator<IntersectionPoints>::type iterator;

#ifdef GGL_DEBUG_INTERSECTION
    std::cout << "Sorted (x then y): " << std::endl;
    for (iterator it = boost::begin(intersection_points);
        it != boost::end(intersection_points); ++it)
    {
        std::cout << *it;
    }
#endif
    bool has_merge = false;

    // Merge all same IP's, combining there IP/segment-info entries
    iterator it = boost::begin(intersection_points);
    for (iterator prev = it++; it != boost::end(intersection_points); ++it)
    {
        // IP can be merged if the point is equal
        if (ggl::equals(prev->point, it->point))
        {
            has_merge = true;
            prev->shared = true;
            prev->trivial = false;
            it->flagged = true;
            std::copy(it->info.begin(), it->info.end(),
                        std::back_inserter(prev->info));
        }
        else
        {
            prev = it;
        }
    }


    if (has_merge)
    {
#ifdef GGL_DEBUG_INTERSECTION
        std::cout << "Merged (1): " << std::endl;
        report_ip(intersection_points);
#endif

        // If there merges, there might be  collinearities
        detail::intersection::remove_collinearities(intersection_points);

        // Remove all IP's which are flagged for deletion
        intersection_points.erase(
            std::remove_if(
                    boost::begin(intersection_points),
                    boost::end(intersection_points),
                    detail::intersection::is_flagged<trav_type>()),
            boost::end(intersection_points));


#ifdef GGL_DEBUG_INTERSECTION
        std::cout << "Merged (2): " << std::endl;
        report_ip(intersection_points);
#endif


    }

}


} // namespace ggl

#endif // GGL_ALGORITHMS_MERGE_INTERSECTION_POINTS_HPP
