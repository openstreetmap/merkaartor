// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_MULTI_NUM_POINTS_HPP
#define GGL_MULTI_NUM_POINTS_HPP


#include <ggl/multi/core/tags.hpp>
#include <ggl/algorithms/num_points.hpp>


namespace ggl
{

#ifndef DOXYGEN_NO_IMPL
namespace impl
{
    namespace num_points
    {
        template <typename G>
        struct multi_count
        {
            static inline size_t calculate(const G& geometry)
            {
                typedef typename boost::range_value<G>::type V;
                typedef typename boost::range_const_iterator<G>::type IT;

                size_t n = 0;
                for (IT it = boost::begin(geometry); it != boost::end(geometry); it++)
                {
                    typedef typename boost::remove_const<V>::type NCG;
                    n += dispatch::num_points<typename tag<NCG>::type,
                        ggl::is_linear<NCG>::value, NCG>::calculate(*it);
                }
                return n;
            }
        };
    }



} // namespace impl
#endif


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{
    template <typename G>
    struct num_points<multi_point_tag, false, G> : impl::num_points::multi_count<G> {};

    template <typename G>
    struct num_points<multi_linestring_tag, false, G> : impl::num_points::multi_count<G> {};

    template <typename G>
    struct num_points<multi_polygon_tag, false, G> : impl::num_points::multi_count<G> {};

} // namespace dispatch
#endif


}


#endif
