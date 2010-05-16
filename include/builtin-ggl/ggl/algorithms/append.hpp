// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_APPEND_HPP
#define GGL_ALGORITHMS_APPEND_HPP

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/concept_check.hpp>

#include <ggl/core/access.hpp>
#include <ggl/core/point_type.hpp>
#include <ggl/core/tags.hpp>
#include <ggl/util/copy.hpp>

namespace ggl
{

namespace traits
{

/*!
    \brief Traits class, optional, might be implemented to append a point
    \details If a geometry type should not use the std "push_back" then it can specialize
    the "use_std" traits class to false, it should then implement (a.o.) append_point
    \ingroup traits
    \par Geometries:
        - linestring
        - linear_ring
    \par Specializations should provide:
        - run
 */
template <typename G, typename P>
struct append_point
{
};

} // namespace traits


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace append {

template <typename G, typename P, bool Std>
struct append_point {};

template <typename G, typename P>
struct append_point<G, P, true>
{
    static inline void apply(G& geometry, P const& point, int , int )
    {
        typename point_type<G>::type point_type;

        copy_coordinates(point, point_type);
        geometry.push_back(point_type);
    }
};

template <typename G, typename P>
struct append_point<G, P, false>
{
    static inline void apply(G& geometry, P const& point, int ring_index, int multi_index)
    {
        traits::append_point<G, P>::apply(geometry, point, ring_index, multi_index);
    }
};

template <typename G, typename R, bool Std>
struct append_range
{
    typedef typename boost::range_value<R>::type point_type;

    static inline void apply(G& geometry, R const& range, int ring_index, int multi_index)
    {
        for (typename boost::range_const_iterator<R>::type it = boost::begin(range);
             it != boost::end(range); ++it)
        {
            append_point<G, point_type, Std>::apply(geometry, *it, ring_index, multi_index);
        }
    }
};

template <typename P, typename T, bool Std>
struct point_to_poly
{
    typedef typename ring_type<P>::type range_type;

    static inline void apply(P& polygon, T const& point, int ring_index, int multi_index)
    {
        boost::ignore_unused_variable_warning(multi_index);

        if (ring_index == -1)
        {
            append_point<range_type, T, Std>::apply(exterior_ring(polygon), point, -1, -1);
        }
        else if (ring_index < boost::size(interior_rings(polygon)))
        {
            append_point<range_type, T, Std>::apply(interior_rings(polygon)[ring_index], point, -1, -1);
        }
    }
};

template <typename P, typename R, bool Std>
struct range_to_poly
{
    typedef typename ring_type<P>::type ring_type;

    static inline void apply(P& polygon, R const& range, int ring_index, int multi_index)
    {
        if (ring_index == -1)
        {
            append_range<ring_type, R, Std>::apply(exterior_ring(polygon), range, -1, -1);
        }
        else if (ring_index < boost::size(interior_rings(polygon)))
        {
            append_range<ring_type, R, Std>::apply(interior_rings(polygon)[ring_index], range, -1, -1);
        }
    }
};

}} // namespace detail::append
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

// (RoP = range or point, Std = use std library)

// Default case (where RoP will be range/array/etc)
template <typename Tag, typename TagRoP, typename G, typename RoP, bool Std>
struct append : detail::append::append_range<G, RoP, Std> {};

// Append a point to any geometry
template <typename Tag, typename G, typename P, bool Std>
struct append<Tag, point_tag, G, P, Std>
    : detail::append::append_point<G, P, Std> {};

// Never possible to append anything to a point/box/n-sphere
template <typename TagRoP, typename P, typename RoP, bool Std>
struct append<point_tag, TagRoP, P, RoP, Std> {};

template <typename TagRoP, typename B, typename RoP, bool Std>
struct append<box_tag, TagRoP, B, RoP, Std> {};

template <typename TagRoP, typename N, typename RoP, bool Std>
struct append<nsphere_tag, TagRoP, N, RoP, Std> {};

template <typename P, typename TAG_R, typename R, bool Std>
struct append<polygon_tag, TAG_R, P, R, Std>
        : detail::append::range_to_poly<P, R, Std> {};

template <typename P, typename T, bool Std>
struct append<polygon_tag, point_tag, P, T, Std>
        : detail::append::point_to_poly<P, T, Std> {};

// Multi-linestring and multi-polygon might either implement traits or use standard...

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Appends one or more points to a linestring, linear-ring, polygon, multi
    \ingroup access
    \param geometry a geometry
    \param range_or_point the point or range to add
    \param ring_index the index of the ring in case of a polygon: exterior ring (-1, the default) or
        interior ring index
    \param multi_index reserved for multi polygons
 */
template <typename G, typename RoP>
inline void append(G& geometry, const RoP& range_or_point,
            int ring_index = -1, int multi_index = 0)
{
    typedef typename boost::remove_const<G>::type ncg_type;

    dispatch::append
        <
            typename tag<G>::type,
            typename tag<RoP>::type,
            ncg_type,
            RoP,
            traits::use_std<ncg_type>::value
        >::apply(geometry, range_or_point, ring_index, multi_index);
}

} // namespace ggl

#endif // GGL_ALGORITHMS_APPEND_HPP
