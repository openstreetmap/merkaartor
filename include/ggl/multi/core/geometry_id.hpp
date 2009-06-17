// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_MULTI_CORE_GEOMETRY_ID_HPP
#define GGL_MULTI_CORE_GEOMETRY_ID_HPP


#include <boost/mpl/int.hpp>
#include <boost/type_traits.hpp>


#include <ggl/core/tag.hpp>
#include <ggl/core/tags.hpp>
#include <ggl/multi/core/tags.hpp>


namespace ggl {


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template <>
struct geometry_id<multi_point_tag>      : boost::mpl::int_<11> {};


template <>
struct geometry_id<multi_linestring_tag> : boost::mpl::int_<12> {};


template <>
struct geometry_id<multi_polygon_tag>    : boost::mpl::int_<13> {};



} // namespace core_dispatch
#endif


} // namespace ggl


#endif // GGL_MULTI_CORE_GEOMETRY_ID_HPP
