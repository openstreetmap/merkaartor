// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_UTIL_AS_RANGE_HPP
#define GGL_UTIL_AS_RANGE_HPP

#include <boost/type_traits.hpp>

#include <ggl/core/exterior_ring.hpp>
#include <ggl/core/ring_type.hpp>
#include <ggl/core/tag.hpp>
#include <ggl/core/tags.hpp>

namespace ggl {

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename GeometryTag, typename Geometry>
struct as_range_type
{
    typedef Geometry type;
};

template <typename Geometry>
struct as_range_type<polygon_tag, Geometry>
{
    typedef typename ring_type<Geometry>::type type;
};



template <typename GeometryTag, typename Geometry, typename Range>
struct as_range
{
    static inline Range const& get(Geometry const& input)
    {
        return input;
    }
};

template <typename Geometry, typename Range>
struct as_range<polygon_tag, Geometry, Range>
{
    static inline Range const& get(Geometry const& input)
    {
        return exterior_ring(input);
    }
};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
\brief Meta-function utility returning either type itself, or outer ring
    \details Utility to handle polygon's outer ring as a range
\ingroup utility
*/
template <typename Geometry>
struct as_range_type
{
    typedef typename dispatch::as_range_type
        <
            typename tag<Geometry>::type,
            Geometry
        >::type type;
};

/*!
\brief Function getting either the range (ring, linestring) itself
or the outer ring
    \details Utility to handle polygon's outer ring as a range
\ingroup utility
*/
template <typename Range, typename Geometry>
inline Range const& as_range(Geometry const& input)
{
    return dispatch::as_range
        <
            typename tag<Geometry>::type,
            Geometry,
            Range
        >::get(input);
}

} // namespace ggl

#endif // GGL_UTIL_AS_RANGE_HPP
