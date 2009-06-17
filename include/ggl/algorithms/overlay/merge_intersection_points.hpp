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


#ifndef DOXYGEN_NO_IMPL
namespace impl { namespace intersection {

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


template <typename T>
struct is_collinear
{
    inline bool operator()(T const& object) const
    {
        typedef typename T::traversal_vector tv;
        typedef typename boost::range_const_iterator<tv>::type tvit_type;
        for (tvit_type it = boost::begin(object.info);
                it != boost::end(object.info); ++it)
        {
            // If not collinear / not "TO", do NOT delete (false)
            if (it->how != 'c' && it->how != 't' && it->how != 'm')
            {
                return false;
            }

        }
        return true;
    }
};


template <typename T, int Index>
struct shared_code_is
{
    inline bool operator()(T const& object) const
    {
        return object.shared_code == Index;
    }
};


}} // namespace impl::intersection
#endif //DOXYGEN_NO_IMPL



template <typename V>
inline void merge_intersection_points(V& intersection_points)
{
    typedef typename boost::range_value<V>::type trav_type;

    // Remove all IP's which are collinear
    intersection_points.erase(
        std::remove_if(
                boost::begin(intersection_points),
                boost::end(intersection_points),
                impl::intersection::is_collinear<trav_type>()),
        boost::end(intersection_points));

    if (intersection_points.size() <= 1)
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
        impl::intersection::on_increasing_dimension<trav_type>());

    typedef typename boost::range_iterator<V>::type iterator;

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
            prev->shared_code = 1;
            it->shared_code = 2;
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

        // Remove all IP's which are merged (code=2)
        intersection_points.erase(
            std::remove_if(
                    boost::begin(intersection_points),
                    boost::end(intersection_points),
                    impl::intersection::shared_code_is<trav_type, 2>()),
            boost::end(intersection_points));


#ifdef GGL_DEBUG_INTERSECTION
        std::cout << "Merged: " << std::endl;
        for (iterator it = boost::begin(intersection_points);
            it != boost::end(intersection_points);
            ++it)
        {
            std::cout << *it;
        }
#endif
    }
}

} // namespace ggl

#endif // GGL_ALGORITHMS_MERGE_INTERSECTION_POINTS_HPP
