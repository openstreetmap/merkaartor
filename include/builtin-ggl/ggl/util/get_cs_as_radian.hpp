// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_UTIL_GET_CS_AS_RADIAN_HPP
#define GGL_UTIL_GET_CS_AS_RADIAN_HPP

#include <ggl/core/cs.hpp>

namespace ggl {

#ifndef DOXYGEN_NO_DETAIL
namespace detail {

    template <typename CoordinateSystem>
    struct get_cs_as_radian {};

    template <typename Units>
    struct get_cs_as_radian<cs::geographic<Units> >
    {
        typedef cs::geographic<radian> type;
    };

    template <typename Units>
    struct get_cs_as_radian<cs::spherical<Units> >
    {
        typedef cs::spherical<radian> type;
    };

} // namespace detail
#endif




} // namespace ggl

#endif // GGL_UTIL_GET_CS_AS_RADIAN_HPP
