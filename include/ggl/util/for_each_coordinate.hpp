// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_UTIL_FOR_EACH_COORDINATE_HPP
#define GGL_UTIL_FOR_EACH_COORDINATE_HPP

#include <boost/concept/requires.hpp>
#include <ggl/core/concepts/point_concept.hpp>

namespace ggl
{

#ifndef DOXYGEN_NO_IMPL
namespace impl
{

template <typename P, int I, int N>
struct coordinates_scanner
{
    template <typename Op>
    static void apply(P& point, Op operation)
    {
        operation.template run<P, I>(point);
        coordinates_scanner<P, I+1, N>::apply(point, operation);
    }
};

template <typename P, int N>
struct coordinates_scanner<P, N, N>
{
    template <typename Op>
    static void apply(P&, Op)
    {}
};

} // namespace impl
#endif // DOXYGEN_NO_IMPL

template <typename P, typename Op>
BOOST_CONCEPT_REQUIRES(((concept::Point<P>)),
(void)) for_each_coordinate(P& point, Op operation)
{
    typedef typename impl::coordinates_scanner<P, 0, dimension<P>::value> scanner;

    scanner::apply(point, operation);
}

} // namespace ggl

#endif // GGL_UTIL_FOR_EACH_COORDINATE_HPP
