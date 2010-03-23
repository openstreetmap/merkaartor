// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_GET_SECTION_HPP
#define GGL_ALGORITHMS_GET_SECTION_HPP


#include <boost/concept_check.hpp>
#include <boost/concept/requires.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>


#include <ggl/core/access.hpp>
#include <ggl/core/exterior_ring.hpp>
#include <ggl/core/interior_rings.hpp>

#include <ggl/iterators/point_const_iterator.hpp>

#include <ggl/geometries/segment.hpp>



namespace ggl
{


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Tag, typename Geometry, typename Section>
struct get_section
{
    typedef typename ggl::point_const_iterator<Geometry>::type iterator_type;
    static inline void apply(Geometry const& geometry, Section const& section,
                iterator_type& begin, iterator_type& end)
    {
        begin = boost::begin(geometry) + section.begin_index;
        end = boost::begin(geometry) + section.end_index + 1;
    }
};

template <typename Polygon, typename Section>
struct get_section<polygon_tag, Polygon, Section>
{
    typedef typename ggl::point_const_iterator<Polygon>::type iterator_type;
    static inline void apply(Polygon const& polygon, Section const& section,
                iterator_type& begin, iterator_type& end)
    {
        typedef typename ggl::ring_type<Polygon>::type ring_type;
        ring_type const& ring = section.ring_index < 0
            ? ggl::exterior_ring(polygon)
            : ggl::interior_rings(polygon)[section.ring_index];

        begin = boost::begin(ring) + section.begin_index;
        end = boost::begin(ring) + section.end_index + 1;
    }
};

} // namespace dispatch
#endif




/*!
    \brief Get iterators for a specified section
    \ingroup sectionalize
    \tparam Geometry type
    \tparam Section type of section to get from
    \param geometry geometry which might be located in the neighborhood
    \param section structure with section
    \param begin begin-iterator (const iterator over points of section)
    \param end end-iterator (const iterator over points of section)
    \todo Create non-const version as well

 */
template <typename Geometry, typename Section>
inline void get_section(Geometry const& geometry, Section const& section,
    typename point_const_iterator<Geometry>::type& begin,
    typename point_const_iterator<Geometry>::type& end)
{
    dispatch::get_section
        <
            typename tag<Geometry>::type,
            Geometry,
            Section
        >::apply(geometry, section, begin, end);
}




} // namespace ggl

#endif // GGL_ALGORITHMS_GET_SECTION_HPP
