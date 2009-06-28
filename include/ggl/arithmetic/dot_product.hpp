// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ARITHMETIC_DOT_PRODUCT_HPP
#define GGL_ARITHMETIC_DOT_PRODUCT_HPP

#include <boost/concept/requires.hpp>

#include <ggl/core/concepts/point_concept.hpp>

namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template <typename P1, typename P2, std::size_t Dimension, std::size_t DimensionCount>
struct dot_product_maker
{
    static inline typename coordinate_type<P1>::type
        apply(P1 const& p1, P2 const& p2)
    {
        return get<Dimension>(p1) * get<Dimension>(p2)
            + dot_product_maker<P1, P2, Dimension+1, DimensionCount>::apply(p1, p2);
    }
};

template <typename P1, typename P2, std::size_t DimensionCount>
struct dot_product_maker<P1, P2, DimensionCount, DimensionCount>
{
    static inline typename coordinate_type<P1>::type
        apply(P1 const& p1, P2 const& p2)
    {
        return get<DimensionCount>(p1) * get<DimensionCount>(p2);
    }
};

} // namespace detail
#endif // DOXYGEN_NO_DETAIL


/*!
    \brief Computes the dot product of 2 points
    \ingroup arithmetic
    \param p1 first point
    \param p2 second point
    \return the dot product
 */
template <typename P1, typename P2>
inline typename coordinate_type<P1>::type dot_product(P1 const& p1, P2 const& p2)
{
    BOOST_CONCEPT_ASSERT( (concept::ConstPoint<P1>) );
    BOOST_CONCEPT_ASSERT( (concept::ConstPoint<P2>) );

    return detail::dot_product_maker
        <
            P1, P2,
            0, dimension<P1>::type::value - 1
        >::apply(p1, p2);
}

} // namespace ggl

#endif // GGL_ARITHMETIC_DOT_PRODUCT_HPP
