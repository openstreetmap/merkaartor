// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_CORE_TOPOLOGICAL_DIMENSION_HPP
#define GGL_CORE_TOPOLOGICAL_DIMENSION_HPP


#include <boost/mpl/int.hpp>
#include <boost/type_traits/remove_const.hpp>


#include <ggl/core/tag.hpp>
#include <ggl/core/tags.hpp>


namespace ggl {


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch {


template <typename GeometryTag>
struct top_dim {};


template <>
struct top_dim<point_tag>      : boost::mpl::int_<0> {};


template <>
struct top_dim<linestring_tag> : boost::mpl::int_<1> {};


template <>
struct top_dim<segment_tag>    : boost::mpl::int_<1> {};


// ring: topological dimension of two, but some people say: 1 !!
template <>
struct top_dim<ring_tag>       : boost::mpl::int_<2> {};


template <>
struct top_dim<box_tag>        : boost::mpl::int_<2> {};


template <>
struct top_dim<polygon_tag>    : boost::mpl::int_<2> {};


// nsphere: 2, but there is discussion. Is it CLOSED? Then 2, but
// then it should be called "disk"...
template <>
struct top_dim<nsphere_tag>    : boost::mpl::int_<2> {};




} // namespace core_dispatch
#endif





/*!
    \brief Meta-function returning the topological dimension of a geometry
    \details The topological dimension defines a point as 0-dimensional,
        a linestring as 1-dimensional,
        and a ring or polygon as 2-dimensional.
    \see http://www.math.okstate.edu/mathdept/dynamics/lecnotes/node36.html
    \ingroup core
*/
template <typename Geometry>
struct topological_dimension
    : core_dispatch::top_dim<typename tag<Geometry>::type> {};


} // namespace ggl


#endif // GGL_CORE_TOPOLOGICAL_DIMENSION_HPP
