// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_CORE_COORDINATE_DIMENSION_HPP
#define GGL_CORE_COORDINATE_DIMENSION_HPP

#include <cstddef>

#include <boost/mpl/equal_to.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <ggl/core/coordinate_dimension.hpp>
#include <ggl/core/point_type.hpp>

namespace ggl
{

namespace traits
{

/*!
    \brief Traits class indicating the number of dimensions of a point
    \par Geometries:
        - point
    \par Specializations should provide:
        - value (should be derived from boost::mpl::int_<D>
    \ingroup traits
*/
template <typename P>
struct dimension {};

} // namespace traits

#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

// Base class derive from its own specialization of point-tag
template <typename T, typename G>
struct dimension : dimension<point_tag, typename point_type<T, G>::type> {};

template <typename P>
struct dimension<point_tag, P> : traits::dimension<P> {};

} // namespace core_dispatch
#endif

/*!
    \brief Meta-function which defines coordinate dimensions, i.e. the number of axes of any geometry
    \ingroup core
*/
template <typename G>
struct dimension
    : core_dispatch::dimension
        <
        typename tag<G>::type,
        typename boost::remove_const<G>::type
        >
{};

/*!
    \brief assert_dimension, enables compile-time checking if coordinate dimensions are as expected
    \ingroup utility
*/
template <typename G, int D>
inline void assert_dimension()
{
    BOOST_STATIC_ASSERT((
        boost::mpl::equal_to
        <
        ggl::dimension<G>,
        boost::mpl::int_<D>
        >::type::value
        ));
}

/*!
    \brief assert_dimension, enables compile-time checking if coordinate dimensions are as expected
    \ingroup utility
*/
template <typename G, int D>
inline void assert_dimension_less_equal()
{
    BOOST_STATIC_ASSERT(( dimension<G>::type::value <= D ));
}

template <typename G, int D>
inline void assert_dimension_greater_equal()
{
    BOOST_STATIC_ASSERT(( dimension<G>::type::value >= D ));
}

/*!
    \brief assert_dimension_equal, enables compile-time checking if coordinate dimensions of two geometries are equal
    \ingroup utility
*/
template <typename G1, typename G2>
inline void assert_dimension_equal()
{
    BOOST_STATIC_ASSERT(( dimension<G1>::type::value == dimension<G2>::type::value ));
}

} // namespace ggl

#endif // GGL_CORE_COORDINATE_DIMENSION_HPP
