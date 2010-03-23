// Generic Geometry Library
//
// Copyright Barend Gehrels 2008-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_MULTI_IO_WKT_DETAIL_WKT_HPP
#define GGL_MULTI_IO_WKT_DETAIL_WKT_HPP


#include <ggl/extensions/gis/io/wkt/write_wkt.hpp>
#include <ggl/multi/core/tags.hpp>


namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace wkt {

struct prefix_null
{
    static inline const char* apply() { return ""; }
};

struct prefix_multipoint
{
    static inline const char* apply() { return "MULTIPOINT"; }
};

struct prefix_multilinestring
{
    static inline const char* apply() { return "MULTILINESTRING"; }
};

struct prefix_multipolygon
{
    static inline const char* apply() { return "MULTIPOLYGON"; }
};



}} // namespace wkt::impl
#endif



} // namescpae ggl

#endif // GGL_MULTI_IO_WKT_DETAIL_WKT_HPP
