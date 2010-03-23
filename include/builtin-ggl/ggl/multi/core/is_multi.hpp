// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_MULTI_CORE_IS_MULTI_HPP
#define GGL_MULTI_CORE_IS_MULTI_HPP


#include <boost/type_traits.hpp>


#include <ggl/core/is_multi.hpp>
#include <ggl/multi/core/tags.hpp>


namespace ggl {


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template <>
struct is_multi<multi_point_tag> : boost::true_type {};


template <>
struct is_multi<multi_linestring_tag> : boost::true_type {};


template <>
struct is_multi<multi_polygon_tag> : boost::true_type {};


} // namespace core_dispatch
#endif




} // namespace ggl


#endif // GGL_MULTI_CORE_IS_MULTI_HPP
