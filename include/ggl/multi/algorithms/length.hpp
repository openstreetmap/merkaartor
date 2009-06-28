// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_MULTI_LENGTH_HPP
#define GGL_MULTI_LENGTH_HPP

#include <ggl/algorithms/length.hpp>
#include <ggl/multi/algorithms/detail/multi_sum.hpp>

namespace ggl
{
#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{
    template <typename MG, typename S>
    struct length<multi_linestring_tag, MG, S>
            : detail::multi_sum<double, MG, S,
                    detail::length::range_length<typename boost::range_value<MG>::type, S> > {};

} // namespace dispatch
#endif


} // namespace ggl


#endif // GGL_MULTI_LENGTH_HPP
