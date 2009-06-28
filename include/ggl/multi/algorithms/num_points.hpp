// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_MULTI_ALGORITHMS_NUM_POINTS_HPP
#define GGL_MULTI_ALGORITHMS_NUM_POINTS_HPP


#include <ggl/multi/core/tags.hpp>
#include <ggl/algorithms/num_points.hpp>


namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace num_points {


template <typename MultiGeometry>
struct multi_count
{
    static inline size_t apply(MultiGeometry const& geometry)
    {
        typedef typename boost::range_value<MultiGeometry>::type geometry_type;
        typedef typename boost::remove_const<geometry_type>::type ncg;
        typedef typename boost::range_const_iterator
            <
                MultiGeometry
            >::type iterator_type;

        size_t n = 0;
        for (iterator_type it = boost::begin(geometry);
            it != boost::end(geometry);
            ++it)
        {
            n += dispatch::num_points<typename tag<ncg>::type,
                ggl::is_linear<ncg>::value, ncg>::apply(*it);
        }
        return n;
    }
};


}} // namespace detail::num_points
#endif


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch {


template <typename Geometry>
struct num_points<multi_point_tag, false, Geometry>
    : detail::num_points::multi_count<Geometry> {};

template <typename Geometry>
struct num_points<multi_linestring_tag, false, Geometry>
    : detail::num_points::multi_count<Geometry> {};

template <typename Geometry>
struct num_points<multi_polygon_tag, false, Geometry>
    : detail::num_points::multi_count<Geometry> {};


} // namespace dispatch
#endif


}


#endif // GGL_MULTI_ALGORITHMS_NUM_POINTS_HPP
