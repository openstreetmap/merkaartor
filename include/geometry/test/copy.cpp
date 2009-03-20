// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>

#include <geometry/util/copy.hpp>
#include <geometry/algorithms/assign.hpp>

#include "common.hpp"


using namespace geometry;


template <typename P>
void test_all()
{
	P p1;
	geometry::assign(p1, 1, 22, 333);
	P p2;
	copy_coordinates(p1, p2);
	BOOST_CHECK(get<0>(p2) == 1);
	BOOST_CHECK(get<1>(p2) == 22);
	BOOST_CHECK(get<2>(p2) == 333);
}


int test_main(int, char* [])
{
	test_all<int[3]>();
	test_all<float[3]>();
	test_all<double[3]>();
	test_all<test_point>();
	test_all<point<int, 3, cs::cartesian> >();
	test_all<point<float, 3, cs::cartesian> >();
	test_all<point<double, 3, cs::cartesian> >();

	return 0;
}
