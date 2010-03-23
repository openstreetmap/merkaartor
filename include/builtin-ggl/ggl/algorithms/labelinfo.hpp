// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_LABELINFO_HPP
#define GGL_ALGORITHMS_LABELINFO_HPP

// Algorithms to generate appropriate labelpoint(s) for all geometries

#include <boost/concept/requires.hpp>

#include <ggl/core/concepts/point_concept.hpp>
#include <ggl/algorithms/centroid.hpp>

namespace ggl
{

// For a polygon the broadest line is probably the best, so two points (left/right)
// For a line either the centerpoint, of the longest line, or the most horizontal line,
//   or more than two points to smooth the text around some points. So one/two or more points.
// For a point just return the point.

// The algorithms output to an output iterator
// They have a label_option to influence behaviour. Not yet implemented.
// Is there a better approach? Class?

enum label_option
{
    label_default,
    // line
    label_longest,
    label_horizontal,
    // polygon
    label_centroid,
    label_broadest
};

template<typename P, typename M>
inline
BOOST_CONCEPT_REQUIRES(((Point<P>))
(void)) label_info_point(const P& point, label_option option, M& mp)
{
    mp.resize(1);
    get<0>(mp.front()) = get<0>(point);
    get<1>(mp.front()) = get<1>(point);
}

template<typename Y, typename M>
inline void label_info_polygon(const Y& poly, label_option option, M& mp)
{
    mp.resize(0);
    mp.push_back(centroid_polygon<typename boost::range_value<M>::type>(poly));
}

template<typename B, typename M>
inline void label_info_box(const B& box, label_option option, M& mp)
{
    mp.resize(0);
    mp.push_back(centroid_box(box));
}

//-------------------------------------------------------------------------------------------------------
// General "label_info" versions
//-------------------------------------------------------------------------------------------------------
template<typename P, typename M>
inline
BOOST_CONCEPT_REQUIRES(((ConstPoint<P>))
(void)) label_info(const P& p, label_option option, M& mp)
{
    label_info_point(p, option, mp);
}

template<typename P, typename M>
inline
BOOST_CONCEPT_REQUIRES(((ConstPoint<P>))
(void)) label_info(const box<P>& b, label_option option, M& mp)
{
    label_info_box(b, option, mp);
}

template
<
    typename P,
    template<typename, typename> class PointList,
    template<typename, typename> class RingList,
    template<typename> class PointAlloc,
    template<typename> class RingAlloc,
    typename M
>
inline
BOOST_CONCEPT_REQUIRES(((ConstPoint<P>))
(void)) label_info(const polygon<P, PointList, RingList, PointAlloc, RingAlloc>& poly, label_option option, M& mp)
{
    label_info_polygon(poly, option, mp);
}

} // namespace ggl

#endif // GGL_ALGORITHMS_LABELINFO_HPP
