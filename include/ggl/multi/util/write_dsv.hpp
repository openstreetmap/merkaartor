// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_MULTI_UTIL_WRITE_DSV_HPP
#define GGL_MULTI_UTIL_WRITE_DSV_HPP

#include <ggl/util/write_dsv.hpp>



namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace dsv {


template <typename Geometry>
struct dsv_multi
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os,
                Geometry const& geometry,
                dsv_settings const& settings)
    {
        os << settings.list_open;

        typedef typename range_const_iterator<MultiGeometry>::type iterator;
        for(iterator it = begin(multi); it != end(multi); ++it)
        {
            os << ggl::dsv(*it);
        }
        os << settings.list_close;
    }
};




}} // namespace detail::dsv
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch {


template <typename Tag, typename Geometry>
struct dsv<Tag, true, Geometry>
    : detail::dsv::dsv_multi<Geometry>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



} // namespace ggl

#endif // GGL_MULTI_UTIL_WRITE_DSV_HPP
