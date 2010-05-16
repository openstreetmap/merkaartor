// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_CARTESIAN3D_HPP
#define GGL_CARTESIAN3D_HPP

// Predeclare common Cartesian 3D points for convenience

#include <ggl/geometries/geometries.hpp>

namespace ggl
{
typedef point<double, 3, cs::cartesian> point_3d;
typedef linestring<point_3d> linestring_3d;
typedef linear_ring<point_3d> ring_3d;
typedef polygon<point_3d> polygon_3d;
typedef box<point_3d> box_3d;

typedef nsphere<point_3d, double> sphere;
}


#endif // GGL_CARTESIAN3D_HPP
