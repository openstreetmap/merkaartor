// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_CORE_EXTERIOR_RING_HPP
#define GGL_CORE_EXTERIOR_RING_HPP


#include <boost/type_traits/remove_const.hpp>


#include <ggl/core/ring_type.hpp>
#include <ggl/core/tag.hpp>
#include <ggl/core/tags.hpp>


namespace ggl {

namespace traits {

/*!
    \brief Traits class defining access to exterior_ring of a polygon
    \details Should define const and non const access
    \ingroup traits
    \tparam G geometry
    \par Geometries:
        - polygon
    \par Specializations should provide:
        - static inline RING& get(POLY& )
        - static inline const RING& get(const POLY& )
*/
template <typename P>
struct exterior_ring {};

} // namespace traits


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template <typename T, typename G>
struct exterior_ring {};

template <typename P>
struct exterior_ring<polygon_tag, P>
{
    static inline typename ring_type<polygon_tag, P>::type& get(P& polygon)
    {
        return traits::exterior_ring<P>::get(polygon);
    }

    static inline const typename ring_type<polygon_tag, P>::type& get(const P& polygon)
    {
        return traits::exterior_ring<P>::get(polygon);
    }
};

} // namespace core_dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Function to get the exterior_ring ring of a polygon
    \ingroup access
    \note OGC compliance: instead of ExteriorRing
    \tparam P polygon type
    \param polygon the polygon to get the exterior ring from
    \return a reference to the exterior ring
*/
template <typename P>
inline typename ring_type<P>::type& exterior_ring(P& polygon)
{
    return core_dispatch::exterior_ring<typename tag<P>::type, P>::get(polygon);
}

/*!
    \brief Function to get the exterior ring of a polygon (const version)
    \ingroup access
    \note OGC compliance: instead of ExteriorRing
    \tparam P polygon type
    \param polygon the polygon to get the exterior ring from
    \return a const reference to the exterior ring
*/
template <typename P>
inline const typename ring_type<P>::type& exterior_ring(const P& polygon)
{
    return core_dispatch::exterior_ring<typename tag<P>::type, P>::get(polygon);
}


} // namespace ggl


#endif // GGL_CORE_EXTERIOR_RING_HPP
