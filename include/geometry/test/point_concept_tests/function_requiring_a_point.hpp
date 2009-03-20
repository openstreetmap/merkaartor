// Generic Geometry Library Point concept unit tests
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_FUNCTION_REQUIRING_A_POINT_HPP
#define _GEOMETRY_FUNCTION_REQUIRING_A_POINT_HPP


#include <boost/concept/requires.hpp>
#include <geometry/core/concepts/point_concept.hpp>


namespace geometry
{
	template <typename P, typename CP>

	inline BOOST_CONCEPT_REQUIRES(
		((Point<P>)) ((ConstPoint<CP>)),
		(void))
	function_requiring_a_point(P& p1, const CP& p2)
	{
		set<0>(p1, get<0>(p2));
	}
}


#endif
