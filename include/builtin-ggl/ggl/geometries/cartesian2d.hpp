// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_GEOMETRIES_CARTESIAN2D_HPP
#define GGL_GEOMETRIES_CARTESIAN2D_HPP

// Predeclare common Cartesian 2D points for convenience

#include <ggl/geometries/geometries.hpp>

namespace ggl
{

typedef point_xy<double, cs::cartesian> point_2d;
typedef linestring<point_2d> linestring_2d;
typedef linear_ring<point_2d> ring_2d;
typedef polygon<point_2d> polygon_2d;
typedef box<point_2d> box_2d;
typedef segment<point_2d> segment_2d;
typedef nsphere<point_2d, double> circle;

} // namespace ggl

#endif // GGL_GEOMETRIES_CARTESIAN2D_HPP
