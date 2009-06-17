// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_PROJECTIONS_IMPL_FUNCTION_OVERLOADS_HPP
#define GGL_PROJECTIONS_IMPL_FUNCTION_OVERLOADS_HPP

#include <cmath>

namespace ggl { namespace projection {

// Functions to resolve ambiguity when compiling with coordinates of different types
/*inline double atan2(double a, double b)
{
    return std::atan2(a, b);
}
inline double pow(double a, double b)
{
    return std::pow(a, b);
}
*/

inline int int_floor(double f)
{
    return int(std::floor(f));
}

}} // namespace ggl::projection

#endif // GGL_PROJECTIONS_IMPL_FUNCTION_OVERLOADS_HPP
