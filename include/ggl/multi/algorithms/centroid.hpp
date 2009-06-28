// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_MULTI_ALGORITHMS_CENTROID_HPP
#define GGL_MULTI_ALGORITHMS_CENTROID_HPP

#include <ggl/algorithms/centroid.hpp>
#include <ggl/multi/core/point_type.hpp>
#include <ggl/multi/algorithms/detail/multi_sum.hpp>

namespace ggl {


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace centroid {

template<typename MultiPolygon, typename Point, typename Strategy>
struct centroid_multi_polygon
{
    static inline void apply(MultiPolygon const& multi, Point& c, Strategy const& strategy)
    {
        typedef typename boost::range_const_iterator<MultiPolygon>::type iterator;

        typename Strategy::state_type state;

        for (iterator it = boost::begin(multi); it != boost::end(multi); ++it)
        {
// TODO: make THIS the building block!
            typedef typename boost::range_value<MultiPolygon>::type polygon_type;
            polygon_type const& poly = *it;
            if (ring_ok(exterior_ring(poly), c))
            {

                loop(exterior_ring(poly), strategy, state);

                typedef typename boost::range_const_iterator
                    <
                        typename interior_type<polygon_type>::type
                    >::type iterator_type;

                for (iterator_type it = boost::begin(interior_rings(poly));
                     it != boost::end(interior_rings(poly));
                     ++it)
                {
                    loop(*it, strategy, state);
                }
            }
        }
        state.centroid(c);
    }
};



}} // namespace detail::centroid
#endif // DOXYGEN_NO_DETAIL



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{
    template <typename MultiPolygon, typename Point, typename Strategy>
    struct centroid<multi_polygon_tag, MultiPolygon, Point, Strategy>
        : detail::centroid::centroid_multi_polygon<MultiPolygon, Point, Strategy>
    {};


} // namespace dispatch
#endif


} // namespace ggl


#endif // GGL_MULTI_ALGORITHMS_CENTROID_HPP
