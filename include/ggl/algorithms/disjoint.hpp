// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_DISJOINT_HPP
#define GGL_ALGORITHMS_DISJOINT_HPP

#include <boost/mpl/if.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>
#include <boost/static_assert.hpp>

#include <ggl/core/access.hpp>
#include <ggl/core/coordinate_dimension.hpp>
#include <ggl/core/is_multi.hpp>
#include <ggl/core/reverse_dispatch.hpp>
#include <ggl/util/math.hpp>
#include <ggl/util/select_coordinate_type.hpp>



namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace disjoint {

template <typename P1, typename P2, std::size_t D, std::size_t N>
struct point_point
{
    typedef typename select_coordinate_type<P1, P2>::type coordinate_type;

    static inline bool apply(P1 const& p1, P2 const& p2)
    {
        if (! math::equals(get<D>(p1), get<D>(p2)))
        {
            return true;
        }
        return point_point<P1, P2, D + 1, N>::apply(p1, p2);
    }
};

template <typename P1, typename P2, std::size_t N>
struct point_point<P1, P2, N, N>
{
    static inline bool apply(P1 const& , P2 const& )
    {
        return false;
    }
};


template <typename P, typename B, std::size_t D, std::size_t N>
struct point_box
{
    static inline bool apply(P const& point, B const& box)
    {
        if (get<D>(point) < get<min_corner, D>(box)
            || get<D>(point) > get<max_corner, D>(box))
        {
            return true;
        }
        return point_box<P, B, D + 1, N>::apply(point, box);
    }
};

template <typename P, typename B, std::size_t N>
struct point_box<P, B, N, N>
{
    static inline bool apply(P const& , B const& )
    {
        return false;
    }
};


template <typename B1, typename B2, std::size_t D, std::size_t N>
struct box_box
{
    static inline bool apply(B1 const& box1, B2 const& box2)
    {
        if (get<max_corner, D>(box1) < get<min_corner, D>(box2))
        {
            return true;
        }
        if (get<min_corner, D>(box1) > get<max_corner, D>(box2))
        {
            return true;
        }
        return box_box<B1, B2, D + 1, N>::apply(box1, box2);
    }
};

template <typename B1, typename B2, std::size_t N>
struct box_box<B1, B2, N, N>
{
    static inline bool apply(B1 const& , B2 const& )
    {
        return false;
    }
};



}} // namespace detail::disjoint
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename GeometryTag1, typename GeometryTag2,
    typename G1, typename G2,
    bool IsMulti1, bool IsMulti2,
    std::size_t DimensionCount
>
struct disjoint
{
};

template <typename P1, typename P2, std::size_t DimensionCount>
struct disjoint<point_tag, point_tag, P1, P2, false, false, DimensionCount>
    : detail::disjoint::point_point<P1, P2, 0, DimensionCount>
{
};

template <typename B1, typename B2, std::size_t DimensionCount>
struct disjoint<box_tag, box_tag, B1, B2, false, false, DimensionCount>
    : detail::disjoint::box_box<B1, B2, 0, DimensionCount>
{
};

template <typename P, typename B, std::size_t DimensionCount>
struct disjoint<point_tag, box_tag, P, B, false, false, DimensionCount>
    : detail::disjoint::point_box<P, B, 0, DimensionCount>
{
};


template
<
    typename GeometryTag1, typename GeometryTag2,
    typename G1, typename G2,
    bool IsMulti1, bool IsMulti2,
    std::size_t DimensionCount
>
struct disjoint_reversed
{
    static inline bool apply(G1 const& g1, G2 const& g2)
    {
        return disjoint
            <
                GeometryTag2, GeometryTag1,
                G2, G1,
                IsMulti2, IsMulti1,
                DimensionCount
            >::apply(g2, g1);
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



/*!
    \brief Calculate if two geometries are disjoint
    \ingroup boolean_relations
    \tparam Geometry1 first geometry type
    \tparam Geometry2 second geometry type
    \param geometry1 first geometry
    \param geometry2 second geometry
    \return true if disjoint, else false
 */
template <typename Geometry1, typename Geometry2>
inline bool disjoint(const Geometry1& geometry1,
            const Geometry2& geometry2)
{
    assert_dimension_equal<Geometry1, Geometry2>();

    typedef typename boost::remove_const<Geometry1>::type ncg1_type;
    typedef typename boost::remove_const<Geometry2>::type ncg2_type;

    return boost::mpl::if_c
        <
            reverse_dispatch<Geometry1, Geometry2>::type::value,
            dispatch::disjoint_reversed
            <
                typename tag<ncg1_type>::type,
                typename tag<ncg2_type>::type,
                ncg1_type,
                ncg2_type,
                is_multi<ncg1_type>::type::value,
                is_multi<ncg2_type>::type::value,
                dimension<ncg1_type>::type::value
            >,
            dispatch::disjoint
            <
                typename tag<ncg1_type>::type,
                typename tag<ncg2_type>::type,
                ncg1_type,
                ncg2_type,
                is_multi<ncg1_type>::type::value,
                is_multi<ncg2_type>::type::value,
                dimension<ncg1_type>::type::value
            >
        >::type::apply(geometry1, geometry2);
}


} // namespace ggl

#endif // GGL_ALGORITHMS_DISJOINT_HPP
