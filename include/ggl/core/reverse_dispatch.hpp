// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_CORE_REVERSE_DISPATCH_HPP
#define GGL_CORE_REVERSE_DISPATCH_HPP


#include <boost/type_traits.hpp>

#include <boost/mpl/greater.hpp>

#include <ggl/core/geometry_id.hpp>


namespace ggl {


#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

// Different geometries: reverse_dispatch if second ID < first ID
// Note: if "boost::mpl::greater is used, with geometry_id instead of geometry_id::type::value,
// it is not working correctly!!!! TODO: find out why not.
template <std::size_t GeometryId1, std::size_t GeometryId2>
struct reverse_dispatch : boost::mpl::if_c
<
    (GeometryId1 > GeometryId2),
    boost::true_type,
    boost::false_type
>
{};


// Same geometry: never reverse_dispatch
template <std::size_t GeometryId>
struct reverse_dispatch<GeometryId, GeometryId> : boost::false_type {};


} // namespace detail
#endif // DOXYGEN_NO_DETAIL


template <typename Geometry1, typename Geometry2>
struct reverse_dispatch : detail::reverse_dispatch
    <
    geometry_id<Geometry1>::type::value,
    geometry_id<Geometry2>::type::value
    >
{};


} // namespace ggl

#endif // GGL_CORE_REVERSE_DISPATCH_HPP
