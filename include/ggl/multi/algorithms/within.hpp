// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_MULTI_ALGORITHMS_WITHIN_HPP
#define GGL_MULTI_ALGORITHMS_WITHIN_HPP

#include <vector>

#include <ggl/algorithms/within.hpp>
#include <ggl/multi/core/tags.hpp>

namespace ggl
{

#ifndef DOXYGEN_NO_IMPL
namespace impl { namespace within {


template<typename P, typename I, typename S>
inline bool point_in_multi_polygon(const P& p, const I& m, const S& strategy)
{
    for (typename I::const_iterator i = m.begin(); i != m.end(); i++)
    {
        // Point within a multi-polygon: true if within one of the polygons
        if (point_in_polygon(p, *i, strategy))
        {
            return true;
        }
    }
    return false;
}


template<typename I, typename C>
inline bool multi_polygon_in_circle(const I& m, const C& c)
{
    for (typename I::const_iterator i = m.begin(); i != m.end(); i++)
    {
        if (! polygon_in_circle(*i, c))
        {
            return false;
        }
    }
    return true;
}

}} // namespace impl::within
#endif // DOXYGEN_NO_IMPL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

/*
template <typename M, typename B>
struct within<multi_polygon_tag, box_tag, M, B>
{
    inline static bool calculate(const M& m, const B& b)
    {
        return impl::within::multi_polygon_in_box(m, b);
    }
};
*/

template <typename M, typename C>
struct within<multi_polygon_tag, nsphere_tag, M, C>
{
    inline static bool calculate(const M& m, const C& c)
    {
        return impl::within::multi_polygon_in_circle(m, c);
    }
};

template <typename P, typename M>
struct within<point_tag, multi_polygon_tag, P, M>
{
    template <typename S>
    inline static bool calculate(const P& p, const M& m, const S& strategy)
    {
        return impl::within::point_in_multi_polygon(p, m, strategy);
    }

    inline static bool calculate(const P& p, const M& m)
    {
        typedef typename point_type<M>::type PM;
        typename strategy_within<
                    typename cs_tag<P>::type, typename cs_tag<PM>::type, P, PM
                        >::type strategy;
        return calculate(p, m, strategy);
    }

};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

} // namespace ggl

#endif // GGL_MULTI_ALGORITHMS_WITHIN_HPP
