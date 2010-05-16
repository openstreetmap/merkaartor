// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_STRATEGIES_STRATEGY_TRAITS_HPP
#define GGL_STRATEGIES_STRATEGY_TRAITS_HPP

#include <ggl/core/cs.hpp>
#include <ggl/strategies/distance_result.hpp>

// File containing strategy traits classes, to be specialized in other files
// (This file might be splitted resulting into several small files)

namespace ggl
{

namespace strategy
{
    /*!
        \brief Indicate compiler/library user that strategy is not implemented.
        \details The strategy_traits class define strategies for point types or for point type
        combinations. If there is no implementation for that specific point type, or point type
        combination, the calculation cannot be done. To indicate this, this not_implemented
        class is used as a typedef stub.

    */
    struct not_implemented {};
}


/*!
    \brief Traits class binding an area strategy to a coordinate system
    \ingroup area
    \tparam T tag of coordinate system
    \tparam P point-type
*/
template <typename T, typename P>
struct strategy_area
{
    typedef strategy::not_implemented type;
};

/*!
    \brief Traits class binding a distance strategy to a (possibly two) coordinate system(s)
    \ingroup distance
    \tparam T1 tag of coordinate system of first point type
    \tparam T2 tag of coordinate system of second point type
    \tparam P1 first point-type
    \tparam P2 second point-type
*/
template <typename T1, typename T2, typename P1, typename P2>
struct strategy_distance
{
    typedef strategy::not_implemented type;
};

/*!
    \brief Traits class binding a distance-to-segment strategy to a (possibly two) coordinate system(s)
    \ingroup distance
    \tparam CsTag1 tag of coordinate system of point type
    \tparam CsTag2 tag of coordinate system of segment type, usually same as CsTag1
    \tparam Point point-type
    \tparam Segment segment-type
*/
template <typename CsTag1, typename CsTag2, typename Point, typename Segment>
struct strategy_distance_segment
{
    typedef strategy::not_implemented type;
};


/*!
    \brief Traits class binding a centroid calculation strategy to a coordinate system
    \ingroup centroid
    \tparam T tag of coordinate system
    \tparam P point-type
    \tparam PS segment point-type
*/
template <typename T, typename P, typename PS>
struct strategy_centroid
{
    typedef strategy::not_implemented type;
};


/*!
    \brief Traits class binding envelope strategy to a coordinate system
    \ingroup envelope
    \tparam TP tag of coordinate system of point
    \tparam TB tag of coordinate system of box, usually same as TP
    \tparam P point-type
    \tparam B box-type
*/
template <typename TP, typename TB, typename P, typename B>
struct strategy_envelope
{
    typedef strategy::not_implemented type;
};


/*!
    \brief Traits class binding a convex hull calculation strategy to a coordinate system
    \ingroup convex_hull
    \tparam T tag of coordinate system
    \tparam P point-type of input points
*/
template <typename T, typename P>
struct strategy_convex_hull
{
    typedef strategy::not_implemented type;
};


/*!
    \brief Traits class binding a within determination strategy to a coordinate system
    \ingroup within
    \tparam TP tag of coordinate system of point-type
    \tparam TS tag of coordinate system of segment-type
    \tparam P point-type of input points
    \tparam PS point-type of input segment-points
*/
template <typename TP, typename TS, typename P, typename PS>
struct strategy_within
{
    typedef strategy::not_implemented type;
};


/*!
    \brief Traits class binding a side determination strategy to a coordinate system
    \ingroup util
    \tparam T tag of coordinate system of point-type
    \tparam P point-type of input points
    \tparam PS point-type of input points
*/
template <typename T, typename P, typename PS = P>
struct strategy_side
{
    typedef strategy::not_implemented type;
};



/*!
    \brief Traits class binding a comparing strategy to a coordinate system
    \ingroup util
    \tparam T tag of coordinate system of point-type
    \tparam P point-type
    \tparam D dimension to compare
*/
template <typename T, typename P, size_t D>
struct strategy_compare
{
    typedef strategy::not_implemented type;
};


/*!
    \brief Traits class binding a transformation strategy to a coordinate system
    \ingroup transform
    \details Can be specialized
    - per coordinate system family (tag)
    - per coordinate system (or groups of them)
    - per dimension
    - per point type
    \tparam CS_TAG 1,2 coordinate system tags
    \tparam CS 1,2 coordinate system
    \tparam D 1, 2 dimension
    \tparam P 1, 2 point type
 */
template <typename CS_TAG1, typename CS_TAG2,
            typename CS1, typename CS2,
            size_t D1, size_t D2,
            typename P1, typename P2>
struct strategy_transform
{
    typedef strategy::not_implemented type;
};


/*!
    \brief Traits class binding a parsing strategy to a coordinate system
    \ingroup parse
    \tparam T tag of coordinate system of point-type
    \tparam CS coordinate system
*/
template <typename T, typename CS>
struct strategy_parse
{
    typedef strategy::not_implemented type;
};




/*!
    \brief Shortcut to define return type of distance strategy
    \ingroup distance
    \tparam G1 first geometry
    \tparam G2 second geometry
 */
template <typename G1, typename G2 = G1>
struct distance_result
{
    typedef typename point_type<G1>::type P1;
    typedef typename point_type<G2>::type P2;
    typedef typename strategy_distance<
                typename cs_tag<P1>::type,
                typename cs_tag<P2>::type, P1, P2>::type S;
    typedef typename S::return_type type;
};



struct strategy_tag_unknown {};
struct strategy_tag_distance_point_point {};
struct strategy_tag_distance_point_segment {};

template <typename T>
struct strategy_tag
{
    typedef strategy_tag_unknown type;
};

} // namespace ggl

#endif // GGL_STRATEGIES_STRATEGY_TRAITS_HPP
