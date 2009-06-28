// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_MULTI_CORE_RING_TYPE_HPP
#define GGL_MULTI_CORE_RING_TYPE_HPP


#include <boost/range/metafunctions.hpp>

#include <ggl/core/ring_type.hpp>
#include <ggl/multi/core/tags.hpp>



namespace ggl
{


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template <typename MultiPolygon>
struct ring_type<multi_polygon_tag, MultiPolygon>
{
    typedef typename ggl::ring_type
        <
            typename boost::range_value<MultiPolygon>::type
        >::type type;
};




} // namespace core_dispatch
#endif


} // namespace ggl


#endif // GGL_MULTI_CORE_RING_TYPE_HPP
