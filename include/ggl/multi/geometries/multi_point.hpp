// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_MULTI_GEOMETRIES_MULTI_POINT_HPP
#define GGL_MULTI_GEOMETRIES_MULTI_POINT_HPP

#include <memory>
#include <vector>

#include <ggl/core/concepts/point_concept.hpp>
#include <ggl/multi/core/tags.hpp>

namespace ggl
{

// Class: multi_point
// Purpose: groups points belonging to each other, e.g. a constellation
template
<
    typename P,
    template<typename, typename> class V = std::vector,
    template<typename> class A = std::allocator
>
struct multi_point : public V<P, A<P> >
{
    BOOST_CONCEPT_ASSERT( (concept::Point<P>) );
};

#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template
<
    typename P,
    template<typename, typename> class V,
    template<typename> class A
>
struct tag< multi_point<P, V, A> >
{
    typedef multi_point_tag type;
};

} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

} // namespace ggl

#endif // GGL_MULTI_GEOMETRIES_MULTI_POINT_HPP
