// Generic Geometry Library Point concept test file
//
// Copyright Bruno Lalande 2008
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include "function_requiring_a_point.hpp"

#include "common.hpp"



int main()
{
	float p1[3] = { 0, 0, 0 };
	const float p2[3] = { 0, 0, 0 };
	geometry::function_requiring_a_point(p1, p2);
}
