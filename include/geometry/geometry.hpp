// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_GEOMETRY_HPP
#define _GEOMETRY_GEOMETRY_HPP

// Shortcut to include all header files



#include <geometry/core/concepts/point_concept.hpp>
#include <geometry/core/concepts/ring_concept.hpp>
#include <geometry/core/concepts/linestring_concept.hpp>
#include <geometry/core/concepts/polygon_concept.hpp>

#include <geometry/core/concepts/box_concept.hpp>
#include <geometry/core/concepts/nsphere_concept.hpp>
#include <geometry/core/concepts/segment_concept.hpp>

#include <geometry/core/cs.hpp>
#include <geometry/core/tag.hpp>
#include <geometry/core/tags.hpp>

#include <geometry/core/access.hpp>
#include <geometry/core/radian_access.hpp>
#include <geometry/core/topological_dimension.hpp>


#include <geometry/arithmetic/arithmetic.hpp>
#include <geometry/arithmetic/dot_product.hpp>

#include <geometry/strategies/strategies.hpp>

#include <geometry/algorithms/append.hpp>
#include <geometry/algorithms/area.hpp>
#include <geometry/algorithms/assign.hpp>
#include <geometry/algorithms/buffer.hpp>
#include <geometry/algorithms/centroid.hpp>
#include <geometry/algorithms/clear.hpp>
#include <geometry/algorithms/convert.hpp>
#include <geometry/algorithms/convex_hull.hpp>
#include <geometry/algorithms/correct.hpp>
#include <geometry/algorithms/distance.hpp>
#include <geometry/algorithms/envelope.hpp>
#include <geometry/algorithms/foreach.hpp>
#include <geometry/algorithms/intersection.hpp>
#include <geometry/algorithms/length.hpp>
#include <geometry/algorithms/make.hpp>
#include <geometry/algorithms/num_points.hpp>
#include <geometry/algorithms/parse.hpp>
#include <geometry/algorithms/selected.hpp>
#include <geometry/algorithms/simplify.hpp>
#include <geometry/algorithms/transform.hpp>
#include <geometry/algorithms/within.hpp>


#include <geometry/io/wkt/aswkt.hpp>
#include <geometry/io/wkt/fromwkt.hpp>

#include <geometry/util/for_each_coordinate.hpp>
#include <geometry/util/copy.hpp>
#include <geometry/util/loop.hpp>
#include <geometry/util/promotion_traits.hpp>
#include <geometry/util/math.hpp>


#endif // _GEOMETRY_GEOMETRY_HPP
