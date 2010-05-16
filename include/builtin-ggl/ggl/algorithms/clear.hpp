// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_CLEAR_HPP
#define GGL_ALGORITHMS_CLEAR_HPP

#include <ggl/core/access.hpp>
#include <ggl/core/exterior_ring.hpp>
#include <ggl/core/interior_rings.hpp>

namespace ggl
{

// This traits is currently NOT defined in ../core/ but here, just because it default
// does not have to be implemented
namespace traits
{

/*!
    \brief Traits class, optional, might be implemented to clear a geometry
    \details If a geometry type should not use the std ".clear()" then it can specialize
    the "use_std" traits class to false, it should then implement (a.o.) clear
    \ingroup traits
    \par Geometries:
        - linestring
        - linear_ring
    \par Specializations should provide:
        - apply
 */
template <typename G>
struct clear
{
};

} // namespace traits


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace clear {

template <typename G>
struct use_std_clear
{
    static inline void apply(G& geometry)
    {
        geometry.clear();
    }
};

template <typename G>
struct use_traits_clear
{
    static inline void apply(G& geometry)
    {
        traits::clear<G>::apply(geometry);
    }
};

template <typename P>
struct polygon_clear
{
    static inline void apply(P& polygon)
    {
        interior_rings(polygon).clear();
        exterior_ring(polygon).clear();
    }
};

template <typename G>
struct no_action
{
    static inline void apply(G& geometry)
    {
    }
};

}} // namespace detail::clear
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Tag, bool Std, typename G>
struct clear
{};

// True (default for all geometry types, unless otherwise implemented in traits)
// uses std::clear
template <typename Tag, typename G>
struct clear<Tag, true, G>
    : detail::clear::use_std_clear<G>
{};

// If any geometry specializes use_std<G> to false, specialize to use the traits clear.
template <typename Tag, typename G>
struct clear<Tag, false, G>
    : detail::clear::use_traits_clear<G>
{};

// Point/box/nsphere/segment do not have clear. So specialize to do nothing.
template <typename G>
struct clear<point_tag, true, G>
    : detail::clear::no_action<G>
{};

template <typename G>
struct clear<box_tag, true, G>
    : detail::clear::no_action<G>
{};

template <typename G>
struct clear<segment_tag, true, G>
    : detail::clear::no_action<G>
{};


template <typename G>
struct clear<nsphere_tag, true, G>
    : detail::clear::no_action<G>
{};


// Polygon can (indirectly) use std for clear
template <typename P>
struct clear<polygon_tag, true, P>
    : detail::clear::polygon_clear<P>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Clears a linestring, linear ring or polygon (exterior+interiors) or multi*
    \details Generic function to clear a geometry
    \ingroup access
    \note points and boxes cannot be cleared, instead they can be set to zero by "assign_zero"
*/
template <typename G>
inline void clear(G& geometry)
{
    typedef typename boost::remove_const<G>::type ncg_type;

    dispatch::clear
        <
        typename tag<G>::type,
        traits::use_std<ncg_type>::value,
        ncg_type
        >::apply(geometry);
}

}  // namespace ggl

#endif // GGL_ALGORITHMS_CLEAR_HPP
