// Generic Geometry Library
//
// Copyright Barend Gehrels 2008-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_EXTENSIONS_GIS_IO_WKT_STREAM_WKT_HPP
#define GGL_EXTENSIONS_GIS_IO_WKT_STREAM_WKT_HPP

#include <ggl/extensions/gis/io/wkt/write_wkt.hpp>

// This short file contains only one manipulator, streaming as WKT
// Don't move contents to as_wkt, developers must be able to choose how to stream

// Don't use namespace ggl, to enable the library to stream custom geometries which
// are living outside the namespace ggl

//namespace ggl
//{


/*!
\brief Streams a geometry as Well-Known Text
\ingroup wkt
*/
template<typename CH, typename TR, typename G>
inline std::basic_ostream<CH,TR>& operator<<(std::basic_ostream<CH,TR> &os, const G& geometry)
{
    os << ggl::wkt(geometry);
    return os;
}

//} // namespace ggl

#endif // GGL_EXTENSIONS_GIS_IO_WKT_STREAM_WKT_HPP
