// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_MULTI_ALGORITHMS_REMOVE_HOLES_IF_HPP
#define GGL_MULTI_ALGORITHMS_REMOVE_HOLES_IF_HPP

#include <ggl/algorithms/remove_holes_if.hpp>
#include <ggl/multi/algorithms/detail/modify_with_predicate.hpp>

namespace ggl {



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{
    template <typename MultiPolygon, typename Predicate>
    struct remove_holes_if<multi_polygon_tag, MultiPolygon, Predicate>
        : impl::multi_modify_with_predicate
            <
                MultiPolygon,
                Predicate,
                impl::remove_holes_if::polygon_remove_holes_if
                    <
                        typename boost::range_value<MultiPolygon>::type, Predicate
                    >
            >
    {};


} // namespace dispatch
#endif


} // namespace ggl


#endif // GGL_MULTI_ALGORITHMS_REMOVE_HOLES_IF_HPP
