// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_MULTI_PERIMETER_HPP
#define GGL_MULTI_PERIMETER_HPP

#include <ggl/algorithms/perimeter.hpp>
#include <ggl/multi/algorithms/detail/multi_sum.hpp>

namespace ggl
{

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{
    template <typename MG, typename S>
    struct perimeter<multi_polygon_tag, MG, S>
            : detail::multi_sum<double, MG, S,
                    detail::perimeter::polygon_perimeter<typename boost::range_value<MG>::type, S> > {};

} // namespace dispatch
#endif


} // namespace ggl


#endif // GGL_MULTI_PERIMETER_HPP
