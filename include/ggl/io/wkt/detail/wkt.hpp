// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_IO_WKT_DETAIL_WKT_HPP
#define GGL_IO_WKT_DETAIL_WKT_HPP




namespace ggl
{

#ifndef DOXYGEN_NO_IMPL
namespace impl { namespace wkt {


struct prefix_point
{
    static inline const char* apply() { return "POINT"; }
};

struct prefix_polygon
{
    static inline const char* apply() { return "POLYGON"; }
};

struct prefix_linestring
{
    static inline const char* apply() { return "LINESTRING"; }
};



}} // namespace wkt::impl
#endif



} // namescpae ggl

#endif // GGL_IO_WKT_DETAIL_WKT_HPP
