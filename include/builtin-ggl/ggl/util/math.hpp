// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_UTIL_MATH_HPP
#define GGL_UTIL_MATH_HPP

#include <cmath>
#include <limits>

#include <boost/math/constants/constants.hpp>

#include <ggl/util/select_most_precise.hpp>

namespace ggl
{

namespace math
{

// Maybe replace this by boost equals or boost ublas numeric equals or so

/*!
    \brief returns true if both arguments are equal.

    equals returns true if both arguments are equal.
    \param a first argument
    \param b second argument
    \return true if a == b
    \note If both a and b are of an integral type, comparison is done by ==. If one of the types
    is floating point, comparison is done by abs and comparing with epsilon.
*/

template <typename T1, typename T2>
inline bool equals(T1 const& a, T2 const& b)
{
    typedef typename select_most_precise<T1, T2>::type select_type;

    // TODO: select on is_fundemental. Otherwise (non-fundamental), take == operator
    if (std::numeric_limits<select_type>::is_exact)
    {
        return a == b;
    }
    else
    {
        return std::abs(a - b) < std::numeric_limits<select_type>::epsilon();
    }
}



double const pi = boost::math::constants::pi<double>();
double const two_pi = 2.0 * pi;
double const d2r = pi / 180.0;
double const r2d = 1.0 / d2r;

/*!
    \brief Calculates the haversine of an angle
    \note See http://en.wikipedia.org/wiki/Haversine_formula
    haversin(alpha) = sin2(alpha/2)
*/
template <typename T>
inline T hav(T const& theta)
{
    using boost::math::constants::half;
    T const sn = std::sin(half<T>() * theta);
    return sn * sn;
}

/*!
\brief Short utility to return the square

\param value Value to calculate the square from
\return The squared value
*/
template <typename T>
inline T sqr(T const& value)
{
    return value * value;
}

} // namespace math


} // namespace ggl

#endif // GGL_UTIL_MATH_HPP
