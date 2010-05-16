// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_STRATEGY_CARTESIAN_COMPARE_HPP
#define GGL_STRATEGY_CARTESIAN_COMPARE_HPP


#include <ggl/core/cs.hpp>
#include <ggl/core/access.hpp>


#include <ggl/strategies/strategy_traits.hpp>


namespace ggl
{
namespace strategy
{
    namespace compare
    {


        /*!
            \brief Compare (in one direction) strategy for cartesian coordinates
            \ingroup util
            \tparam P point-type
            \tparam D dimension
        */
        template <typename P, size_t D>
        struct euclidian
        {
            static inline bool smaller(const P& p1, const P& p2)
            {
                return get<D>(p1) < get<D>(p2);
            }

            static inline bool larger(const P& p1, const P& p2)
            {
                return get<D>(p1) > get<D>(p2);
            }

        };
    } // namespace compare
} // namespace strategy


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

template <typename P, size_t D>
struct strategy_compare<cartesian_tag, P, D>
{
    typedef strategy::compare::euclidian<P, D> type;
};

#endif


} // namespace ggl


#endif // GGL_STRATEGY_CARTESIAN_COMPARE_HPP
