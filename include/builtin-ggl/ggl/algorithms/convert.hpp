// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_CONVERT_HPP
#define GGL_ALGORITHMS_CONVERT_HPP

#include <cstddef>

#include <boost/numeric/conversion/cast.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/algorithms/append.hpp>
#include <ggl/algorithms/for_each.hpp>
#include <ggl/core/cs.hpp>
#include <ggl/geometries/segment.hpp>
#include <ggl/strategies/strategies.hpp>

/*!
\defgroup convert convert geometries from one type to another
\details Convert from one geometry type to another type, for example from BOX to POLYGON
*/

namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace convert {

template <typename P, typename B, std::size_t C, std::size_t D, std::size_t N>
struct point_to_box
{
    static inline void apply(P const& point, B& box)
    {
        typedef typename coordinate_type<B>::type coordinate_type;

        set<C, D>(box, boost::numeric_cast<coordinate_type>(get<D>(point)));
        point_to_box<P, B, C, D + 1, N>::apply(point, box);
    }
};

template <typename P, typename B, std::size_t C, std::size_t N>
struct point_to_box<P, B, C, N, N>
{
    static inline void apply(P const& point, B& box)
    {}
};

}} // namespace detail::convert
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename T1, typename T2, typename G1, typename G2>
struct convert
{
};

template <typename T, typename G1, typename G2>
struct convert<T, T, G1, G2>
{
    // Same geometry type -> copy coordinates from G1 to G2
};

template <typename T, typename G>
struct convert<T, T, G, G>
{
    // Same geometry -> can be copied
};


// Partial specializations
template <typename B, typename R>
struct convert<box_tag, ring_tag, B, R>
{
    static inline void apply(B const& box, R& ring)
    {
        // go from box to ring -> add coordinates in correct order
        // only valid for 2D
        assert_dimension<B, 2>();

        ring.clear();
        typename point_type<B>::type point;

        ggl::assign(point, get<min_corner, 0>(box), get<min_corner, 1>(box));
        ggl::append(ring, point);

        ggl::assign(point, get<min_corner, 0>(box), get<max_corner, 1>(box));
        ggl::append(ring, point);

        ggl::assign(point, get<max_corner, 0>(box), get<max_corner, 1>(box));
        ggl::append(ring, point);

        ggl::assign(point, get<max_corner, 0>(box), get<min_corner, 1>(box));
        ggl::append(ring, point);

        ggl::assign(point, get<min_corner, 0>(box), get<min_corner, 1>(box));
        ggl::append(ring, point);
    }
};

template <typename B, typename P>
struct convert<box_tag, polygon_tag, B, P>
{
    static inline void apply(B const& box, P& polygon)
    {
        typedef typename ring_type<P>::type ring_type;

        convert<box_tag, ring_tag, B, ring_type>::apply(box, exterior_ring(polygon));
    }
};

template <typename P, typename B>
struct convert<point_tag, box_tag, P, B>
{
    static inline void apply(P const& point, B& box)
    {
        // go from point to box -> box with volume of zero, 2D or 3D
        static const std::size_t N = dimension<P>::value;

        detail::convert::point_to_box<P, B, min_corner, 0, N>::apply(point, box);
        detail::convert::point_to_box<P, B, max_corner, 0, N>::apply(point, box);
    }
};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

/*!
    \brief Converts one geometry to another geometry
    \details The convert algorithm converts one geometry, e.g. a BOX, to another geometry, e.g. a RING. This only
    if it is possible and applicable.
    \ingroup convert
    \tparam G1 first geometry type
    \tparam G2 second geometry type
    \param geometry1 first geometry
    \param geometry2 second geometry
 */
template <typename G1, typename G2>
inline void convert(G1 const& geometry1, G2& geometry2)
{
    dispatch::convert
        <
            typename tag<G1>::type,
            typename tag<G2>::type,
            G1,
            G2
        >::apply(geometry1, geometry2);
}

} // namespace ggl

#endif // GGL_ALGORITHMS_CONVERT_HPP
