// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_MULTI_CORE_TAGS_HPP
#define GGL_MULTI_CORE_TAGS_HPP

namespace ggl
{

/// OGC Multi point identifying tag
struct multi_point_tag {};

/// OGC Multi linestring identifying tag
struct multi_linestring_tag {};

/// OGC Multi polygon identifying tag
struct multi_polygon_tag {};

/// OGC Geometry Collection identifying tag
struct geometry_collection_tag {};

} // namespace ggl

#endif // GGL_MULTI_CORE_TAGS_HPP
