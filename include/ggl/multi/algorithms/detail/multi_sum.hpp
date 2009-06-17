// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_MULTI_SUM_HPP
#define GGL_MULTI_SUM_HPP

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

namespace ggl
{
#ifndef DOXYGEN_NO_IMPL
namespace impl
{

    template <typename T, typename MultiGeometry, typename S, typename Policy>
    struct multi_sum
    {
        static inline T calculate(const MultiGeometry& geometry, const S& strategy)
        {
            typedef typename boost::range_const_iterator<MultiGeometry>::type iterator;
            T sum = T();
            for (iterator it = boost::begin(geometry); it != boost::end(geometry); ++it)
            {
                sum += Policy::calculate(*it, strategy);
            }
            return sum;
        }
    };


} // namespace impl
#endif

} // namespace ggl


#endif // GGL_MULTI_SUM_HPP
