// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_GEOMETRIES_LINEAR_RING_HPP
#define GGL_GEOMETRIES_LINEAR_RING_HPP

#include <memory>
#include <vector>

#include <boost/concept/assert.hpp>

#include <ggl/core/tag.hpp>
#include <ggl/core/tags.hpp>

#include <ggl/core/concepts/point_concept.hpp>


namespace ggl
{

/*!
    \brief A linear_ring (linear linear_ring) is a closed line which should not be selfintersecting
    \ingroup Geometry
    \tparam P point type
    \tparam V optional container type, for example std::vector, std::list, std::deque
    \tparam A optional container-allocator-type
*/
template
<
    typename P,
    template<typename, typename> class V = std::vector,
    template<typename> class A = std::allocator
>
class linear_ring : public V<P, A<P> >
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
struct tag< linear_ring<P, V, A> >
{
    typedef ring_tag type;
};

} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

} // namespace ggl

#endif // GGL_GEOMETRIES_LINEAR_RING_HPP
