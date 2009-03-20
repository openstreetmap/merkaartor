// Generic Geometry Library Point concept unit tests
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_FUNCTION_ASSERTING_A_POINT_HPP
#define _GEOMETRY_FUNCTION_ASSERTING_A_POINT_HPP


#include <boost/concept/requires.hpp>
#include <geometry/core/concepts/point_concept.hpp>


namespace geometry
{
	template <typename P, typename CP>
	void function_asserting_a_point(P& p1, const CP& p2)
	{
		BOOST_CONCEPT_ASSERT((Point<P>));
		BOOST_CONCEPT_ASSERT((ConstPoint<P>));

		get<0>(p1) = get<0>(p2);
	}

}


#endif
