// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_MULTI_ALGORITHMS_AREA_HPP
#define GGL_MULTI_ALGORITHMS_AREA_HPP

#include <ggl/algorithms/area.hpp>
#include <ggl/multi/core/point_type.hpp>
#include <ggl/multi/algorithms/detail/multi_sum.hpp>

namespace ggl {


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{
    template <typename MultiGeometry, typename Strategy>
    struct area<multi_polygon_tag, MultiGeometry, Strategy>
        : detail::multi_sum<double, MultiGeometry, Strategy,
            detail::area::polygon_area<
                typename boost::range_value<MultiGeometry>::type, Strategy> >
    {};


} // namespace dispatch
#endif


} // namespace ggl


#endif // GGL_MULTI_ALGORITHMS_AREA_HPP
