// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_MULTI_TOPOLOGICAL_DIMENSION_HPP
#define GGL_MULTI_TOPOLOGICAL_DIMENSION_HPP


#include <boost/mpl/int.hpp>


#include <ggl/core/topological_dimension.hpp>
#include <ggl/multi/core/tags.hpp>


namespace ggl {

#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template <>
struct top_dim<multi_point_tag> : boost::mpl::int_<0> {};


template <>
struct top_dim<multi_linestring_tag> : boost::mpl::int_<1> {};


template <>
struct top_dim<multi_polygon_tag> : boost::mpl::int_<2> {};


} // namespace core_dispatch
#endif

} // namespace ggl


#endif
