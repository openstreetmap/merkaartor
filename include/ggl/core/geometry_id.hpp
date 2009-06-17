// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_CORE_GEOMETRY_ID_HPP
#define GGL_CORE_GEOMETRY_ID_HPP


#include <boost/mpl/int.hpp>
#include <boost/type_traits.hpp>


#include <ggl/core/tag.hpp>
#include <ggl/core/tags.hpp>


namespace ggl {


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template <typename GeometryTag>
struct geometry_id {};


template <>
struct geometry_id<point_tag>      : boost::mpl::int_<1> {};


template <>
struct geometry_id<linestring_tag> : boost::mpl::int_<2> {};


template <>
struct geometry_id<polygon_tag>    : boost::mpl::int_<3> {};


template <>
struct geometry_id<nsphere_tag>    : boost::mpl::int_<91> {};


template <>
struct geometry_id<segment_tag>    : boost::mpl::int_<92> {};


template <>
struct geometry_id<ring_tag>       : boost::mpl::int_<93> {};


template <>
struct geometry_id<box_tag>        : boost::mpl::int_<94> {};



} // namespace core_dispatch
#endif



/*!
    \brief Meta-function the id for a geometry type
    \note Used for e.g. reverse meta-function
    \ingroup core
*/
template <typename Geometry>
struct geometry_id : core_dispatch::geometry_id<typename tag<Geometry>::type>
{};


} // namespace ggl


#endif // GGL_CORE_GEOMETRY_ID_HPP
