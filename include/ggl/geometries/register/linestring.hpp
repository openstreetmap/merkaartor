// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_GEOMETRIES_REGISTER_LINESTRING_HPP
#define GGL_GEOMETRIES_REGISTER_LINESTRING_HPP


#include <ggl/core/tag.hpp>
#include <ggl/core/tags.hpp>


#define GEOMETRY_REGISTER_LINESTRING(Linestring) \
namespace ggl { namespace traits {  \
    template<> struct tag<Linestring> { typedef linestring_tag type; }; \
}}



#endif // GGL_GEOMETRIES_REGISTER_LINESTRING_HPP
