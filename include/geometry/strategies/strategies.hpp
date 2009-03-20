// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_STRATEGIES_HPP
#define _GEOMETRY_STRATEGIES_HPP


#include <geometry/strategies/strategy_traits.hpp>

#include <geometry/strategies/cartesian/cart_area.hpp>
#include <geometry/strategies/cartesian/cart_centroid.hpp>
#include <geometry/strategies/cartesian/cart_compare.hpp>
#include <geometry/strategies/cartesian/cart_distance.hpp>
#include <geometry/strategies/cartesian/cart_envelope.hpp>
#include <geometry/strategies/cartesian/cart_side.hpp>
#include <geometry/strategies/cartesian/cart_within.hpp>

#include <geometry/strategies/spherical/sph_area.hpp>
#include <geometry/strategies/spherical/sph_distance.hpp>
#include <geometry/strategies/spherical/sph_envelope.hpp>

#include <geometry/strategies/geographic/geo_distance.hpp>
#include <geometry/strategies/geographic/geo_parse.hpp>

#include <geometry/strategies/agnostic/agn_convex_hull.hpp>
#include <geometry/strategies/agnostic/agn_simplify.hpp>
#include <geometry/strategies/agnostic/agn_within.hpp>

#include <geometry/strategies/strategy_transform.hpp>

#include <geometry/strategies/transform/matrix_transformers.hpp>
#include <geometry/strategies/transform/map_transformer.hpp>
#include <geometry/strategies/transform/inverse_transformer.hpp>


#endif // _GEOMETRY_STRATEGIES_HPP
