// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_EQUALS_HPP
#define GGL_ALGORITHMS_EQUALS_HPP

#include <cstddef>

#include <boost/static_assert.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/core/access.hpp>
#include <ggl/core/coordinate_dimension.hpp>
#include <ggl/core/is_multi.hpp>
#include <ggl/algorithms/disjoint.hpp>
#include <ggl/algorithms/detail/not.hpp>
#include <ggl/util/math.hpp>

/*!

\defgroup boolean_relations boolean relationships (equals, disjoint, overlaps, etc)

*/

namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace equals {

template <typename B1, typename B2, std::size_t D, std::size_t N>
struct box_box
{
    static inline bool apply(B1 const& box1, B2 const& box2)
    {
        if (!math::equals(get<min_corner, D>(box1), get<min_corner, D>(box2))
            || !math::equals(get<max_corner, D>(box1), get<max_corner, D>(box2)))
        {
            return false;
        }
        return box_box<B1, B2, D + 1, N>::apply(box1, box2);
    }
};

template <typename B1, typename B2, std::size_t N>
struct box_box<B1, B2, N, N>
{
    static inline bool apply(B1 const& , B2 const& )
    {
        return true;
    }
};

}} // namespace detail::equals
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename GeometryTag,
    bool IsMulti,
    typename G1,
    typename G2,
    std::size_t DimensionCount
>
struct equals
{
};

template <typename P1, typename P2, std::size_t DimensionCount>
struct equals<point_tag, false, P1, P2, DimensionCount>
    : ggl::detail::not_
        <
            P1,
            P2,
            detail::disjoint::point_point<P1, P2, 0, DimensionCount>
        >
{
};

template <typename B1, typename B2, std::size_t DimensionCount>
struct equals<box_tag, false, B1, B2, DimensionCount>
    : detail::equals::box_box<B1, B2, 0, DimensionCount>
{
};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Calculate if two geometries are equals
    \ingroup boolean_relations
    \tparam Geometry1 first geometry type
    \tparam Geometry2 second geometry type
    \param geometry1 first geometry
    \param geometry2 second geometry
    \return true if equals, else false
 */
template <typename Geometry1, typename Geometry2>
inline bool equals(Geometry1 const& geometry1, Geometry2 const& geometry2)
{
    assert_dimension_equal<Geometry1, Geometry2>();

// TODO: assert types equal:
// typename tag<ncg1_type>::type, typename tag<ncg2_type>::type,
// (LATER): NO! a linestring can be spatially equal to a multi_linestring

    typedef typename boost::remove_const<Geometry1>::type ncg1_type;
    typedef typename boost::remove_const<Geometry2>::type ncg2_type;

    return dispatch::equals
            <
                typename tag<ncg1_type>::type,
                is_multi<ncg1_type>::type::value,
                ncg1_type,
                ncg2_type,
                dimension<ncg1_type>::type::value
            >::apply(geometry1, geometry2);
}

} // namespace ggl

#endif // GGL_ALGORITHMS_EQUALS_HPP
