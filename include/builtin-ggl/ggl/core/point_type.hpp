// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_CORE_POINT_TYPE_HPP
#define GGL_CORE_POINT_TYPE_HPP

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <ggl/core/ring_type.hpp>
#include <ggl/core/tag.hpp>
#include <ggl/core/tags.hpp>

namespace ggl {

namespace traits {

/*!
    \brief Traits class indicating the type of contained points
    \ingroup traits
    \par Geometries:
        - all geometries except point
    \par Specializations should provide:
        - typedef P type (where P should fulfil the Point concept)
    \tparam G geometry
*/
template <typename G>
struct point_type
{};


} // namespace traits


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template <typename Tag, typename Geometry>
struct point_type
{
    // Default: call traits to get point type
    typedef typename boost::remove_const
        <
            typename traits::point_type<Geometry>::type
        >::type type;
};


// Specialization for point: the point itself
template <typename Point>
struct point_type<point_tag, Point>
{
    typedef Point type;
};

// Specializations for linestring/linear ring, via boost::range
template <typename Linestring>
struct point_type<linestring_tag, Linestring>
{
    typedef typename boost::range_value<Linestring>::type type;
};

template <typename Ring>
struct point_type<ring_tag, Ring>
{
    typedef typename boost::range_value<Ring>::type type;
};

// Specialization for polygon: the point-type is the point-type of its rinsg
template <typename Polygon>
struct point_type<polygon_tag, Polygon>
{
    typedef typename point_type
        <
            ring_tag,
            typename ring_type<polygon_tag, Polygon>::type
        >::type type;
};

} // namespace core_dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Meta-function which defines point type of any geometry
    \ingroup core
*/
template <typename Geometry>
struct point_type
{
    typedef typename boost::remove_const<Geometry>::type ncg;
    typedef typename core_dispatch::point_type<
        typename tag<Geometry>::type, ncg>::type type;




};

} // namespace ggl

#endif // GGL_CORE_POINT_TYPE_HPP
