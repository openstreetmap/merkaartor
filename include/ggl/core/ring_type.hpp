// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_CORE_RING_TYPE_HPP
#define GGL_CORE_RING_TYPE_HPP


#include <boost/type_traits/remove_const.hpp>


#include <ggl/core/tag.hpp>
#include <ggl/core/tags.hpp>


namespace ggl
{

namespace traits
{


/*!
    \brief Traits class to indicate ring-type  of a polygon's exterior ring/interior rings
    \ingroup traits
    \par Geometries:
        - polygon
    \par Specializations should provide:
        - typedef XXX type (e.g. linear_ring<P>)
    \tparam G geometry
*/
template <typename G>
struct ring_type
{
    // should define type
};




} // namespace traits




#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{


template <typename GeometryTag, typename Geometry> struct ring_type
{};



template <typename Polygon>
struct ring_type<polygon_tag, Polygon>
{
    typedef typename traits::ring_type<Polygon>::type type;
};




} // namespace core_dispatch
#endif


/*!
    \brief Meta-function which defines ring type of (multi)polygon geometry
    \details a polygon contains one exterior ring and zero or more interior rings (holes).
        The type of those rings is assumed to be equal. This meta function retrieves the type
        of such rings.
    \ingroup core
*/
template <typename Geometry>
struct ring_type
{
    typedef typename boost::remove_const<Geometry>::type ncg;
    typedef typename core_dispatch::ring_type
        <
            typename tag<Geometry>::type, ncg
        >::type type;
};




}


#endif // GGL_CORE_RING_TYPE_HPP
