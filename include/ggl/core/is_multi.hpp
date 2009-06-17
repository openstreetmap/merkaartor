// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_CORE_IS_MULTI_HPP
#define GGL_CORE_IS_MULTI_HPP


#include <boost/type_traits.hpp>


#include <ggl/core/tag.hpp>
#include <ggl/core/tags.hpp>


namespace ggl {


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template <typename GeometryTag>
struct is_multi : boost::false_type {};


} // namespace core_dispatch
#endif




/*!
    \brief Meta-function defining "true" for multi geometries (multi_point, etc)
    \ingroup core
*/
template <typename Geometry>
struct is_multi : core_dispatch::is_multi<typename tag<Geometry>::type>
{};


} // namespace ggl

#endif // GGL_CORE_IS_MULTI_HPP
