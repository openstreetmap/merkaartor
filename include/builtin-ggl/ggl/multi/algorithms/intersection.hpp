// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_MULTI_ALGORITHMS_INTERSECTION_HPP
#define GGL_MULTI_ALGORITHMS_INTERSECTION_HPP

#include <vector>

#include <ggl/algorithms/intersection.hpp>

namespace ggl
{

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template
<
    typename MultiPolygon1, typename MultiPolygon2,
    typename OutputIterator, typename GeometryOut
>
struct intersection
    <
        multi_polygon_tag, multi_polygon_tag, polygon_tag,
        MultiPolygon1, MultiPolygon2,
        OutputIterator, GeometryOut
    >
{
    // todo: implement this
};

} // namespace ggl


#endif // GGL_MULTI_ALGORITHMS_INTERSECTION_HPP

