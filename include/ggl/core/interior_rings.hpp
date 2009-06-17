// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_CORE_INTERIOR_RINGS_HPP
#define GGL_CORE_INTERIOR_RINGS_HPP


#include <boost/type_traits/remove_const.hpp>

#include <ggl/core/tag.hpp>
#include <ggl/core/tags.hpp>

namespace ggl
{

namespace traits
{

    /*!
        \brief Traits class indicating interior container type of a polygon
        \details defines inner container type, so the container containing the interior rings
        \ingroup traits
        \par Geometries:
            - polygon
        \par Specializations should provide:
            - typedef CONTAINER<RING<P> > type (e.g. std::vector<linear_ring<P> >)
        \tparam G geometry
    */
    template <typename G>
    struct interior_type { };


    /*!
        \brief Traits class defining access to interior_rings of a polygon
        \details defines access (const and non const) to interior ring
        \ingroup traits
        \par Geometries:
            - polygon
        \par Specializations should provide:
            - static inline INTERIOR& get(POLY&)
            - static inline const INTERIOR& get(const POLY&)
        \tparam G geometry
    */
    template <typename G>
    struct interior_rings
    {
    };


} // namespace traits




#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{
    template <typename GeometryTag, typename G> struct interior_type {};

    template <typename P>
    struct interior_type<polygon_tag, P>
    {
        typedef typename traits::interior_type<P>::type type;
    };



    template <typename GeometryTag, typename G>
    struct interior_rings {};


    template <typename P>
    struct interior_rings<polygon_tag, P>
    {
        static inline typename interior_type<polygon_tag, P>::type& get(P& polygon)
        {
            return traits::interior_rings<P>::get(polygon);
        }

        static inline const typename interior_type<polygon_tag, P>::type& get(const P& polygon)
        {
            return traits::interior_rings<P>::get(polygon);
        }
    };



} // namespace core_dispatch
#endif




/*!
    \brief Meta-function defining container type of inner rings of (multi)polygon geometriy
    \details the interior rings should be organized as a container (std::vector, std::deque, boost::array) with
        boost range support. This meta function defines the type of that container.
    \ingroup core
*/
template <typename G>
struct interior_type
{
    typedef typename boost::remove_const<G>::type ncg;
    typedef typename core_dispatch::interior_type<
        typename tag<G>::type, ncg>::type type;
};



/*!
    \brief Function to get the interior rings of a polygon (non const version)
    \ingroup access
    \note OGC compliance: instead of InteriorRingN
    \tparam P polygon type
    \param polygon the polygon to get the interior rings from
    \return a reference to the interior rings
*/
template <typename P>
inline typename interior_type<P>::type& interior_rings(P& polygon)
{
    return core_dispatch::interior_rings<typename tag<P>::type, P>::get(polygon);
}


/*!
    \brief Function to get the interior rings of a polygon (const version)
    \ingroup access
    \note OGC compliance: instead of InteriorRingN
    \tparam P polygon type
    \param polygon the polygon to get the interior rings from
    \return a const reference to the interior rings
*/
template <typename P>
inline const typename interior_type<P>::type& interior_rings(const P& polygon)
{
    return core_dispatch::interior_rings<typename tag<P>::type, P>::get(polygon);
}



/*!
    \brief Function to get the number of interior rings of a polygon
    \ingroup access
    \note Defined by OGC as "numInteriorRing". To be consistent with "numPoints"
        letter "s" is appended
    \tparam P polygon type
    \param polygon the polygon
    \return the nubmer of interior rings
*/
template <typename P>
inline size_t num_interior_rings(const P& polygon)
{
    return boost::size(interior_rings(polygon));
}

}


#endif // GGL_CORE_INTERIOR_RINGS_HPP
