// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>

#include <geometry/arithmetic/dot_product.hpp>

#include <geometry/algorithms/assign.hpp>

#include "common.hpp"


using namespace geometry;


template <typename P>
void test_all()
{
	P p1;
	geometry::assign(p1, 1, 2, 3);
	P p2;
	geometry::assign(p2, 4, 5, 6);
	BOOST_CHECK(geometry::dot_product(p1, p2) == 1*4 + 2*5 + 3*6);
}


int test_main(int, char* [])
{
	test_all<int[3]>();
	test_all<float[3]>();
	test_all<double[3]>();
	test_all<test_point>();
	test_all<point<int, 3, geometry::cs::cartesian> >();
	test_all<point<float, 3, geometry::cs::cartesian> >();
	test_all<point<double, 3, geometry::cs::cartesian> >();

	return 0;
}
