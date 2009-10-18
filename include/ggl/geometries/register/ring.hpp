// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_GEOMETRIES_REGISTER_RING_HPP
#define GGL_GEOMETRIES_REGISTER_RING_HPP

#include <ggl/core/tag.hpp>
#include <ggl/core/tags.hpp>

#define GEOMETRY_REGISTER_RING(Ring) \
namespace ggl { namespace traits {  \
    template<> struct tag<Ring> { typedef ring_tag type; }; \
}}


#define GEOMETRY_REGISTER_RING_TEMPLATIZED(Ring) \
namespace ggl { namespace traits {  \
    template<typename P> struct tag< Ring<P> > { typedef ring_tag type; }; \
}}


#endif // GGL_GEOMETRIES_REGISTER_RING_HPP
