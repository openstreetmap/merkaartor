// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_LATLONG_HPP
#define GGL_LATLONG_HPP

// Predeclare common Cartesian 3D points for convenience

#include <ggl/geometries/geometries.hpp>

namespace ggl
{
typedef point_ll<double, cs::geographic<degree> > point_ll_deg;
typedef linestring<point_ll_deg> linestring_ll_deg;
typedef linear_ring<point_ll_deg> ring_ll_deg;
typedef polygon<point_ll_deg> polygon_ll_deg;
typedef box<point_ll_deg> box_ll_deg;
typedef segment<point_ll_deg> segment_ll_deg;

typedef point_ll<double, cs::geographic<radian> > point_ll_rad;
typedef linestring<point_ll_rad> linestring_ll_rad;
typedef linear_ring<point_ll_rad> ring_ll_rad;
typedef polygon<point_ll_rad> polygon_ll_rad;
typedef box<point_ll_rad> box_ll_rad;
typedef segment<point_ll_rad> segment_ll_rad;
}


#endif // GGL_LATLONG_HPP
