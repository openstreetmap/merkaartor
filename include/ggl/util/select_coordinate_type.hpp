// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_UTIL_SELECT_COORDINATE_TYPE_HPP
#define GGL_UTIL_SELECT_COORDINATE_TYPE_HPP


#include <ggl/core/coordinate_type.hpp>
#include <ggl/util/select_most_precise.hpp>

/*!
\defgroup utility utility: utilities
*/

namespace ggl
{


/*!
    \brief Utility selecting the most precise coordinate type of two geometries
    \ingroup utility
 */
template <typename T1, typename T2>
struct select_coordinate_type
{
    typedef typename select_most_precise
        <
            typename coordinate_type<T1>::type,
            typename coordinate_type<T2>::type
        >::type type;
};

} // namespace ggl

#endif // GGL_UTIL_SELECT_COORDINATE_TYPE_HPP
