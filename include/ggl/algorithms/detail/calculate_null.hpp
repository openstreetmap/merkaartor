// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_DETAIL_CALCULATE_NULL_HPP
#define GGL_ALGORITHMS_DETAIL_CALCULATE_NULL_HPP

namespace ggl
{

#ifndef DOXYGEN_NO_IMPL
namespace impl
{

template<typename T, typename G, typename S>
struct calculate_null
{
    static inline T calculate(const G& , const S&)
    {
        return T();
    }
};

} // namespace impl
#endif // DOXYGEN_NO_IMPL

} // namespace ggl

#endif // GGL_ALGORITHMS_DETAIL_CALCULATE_NULL_HPP
