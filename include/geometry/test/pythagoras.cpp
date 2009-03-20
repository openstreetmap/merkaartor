// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <geometry/strategies/cartesian/cart_distance.hpp>
#include <geometry/algorithms/assign.hpp>

#include "common.hpp"


using namespace geometry;


template <typename P1, typename P2>
void test_null_distance()
{
	typename geometry::strategy::distance::pythagoras<P1, P2> pythagoras;

	P1 p1;
	geometry::assign(p1, 1, 2, 3);
	P2 p2;
	geometry::assign(p2, 1, 2, 3);
	BOOST_CHECK_EQUAL(double(pythagoras(p1, p2)), 0);
}

template <typename P1, typename P2>
void test_axis()
{
	geometry::strategy::distance::pythagoras<P1, P2> pythagoras;

	P1 p1;
	geometry::assign(p1, 0, 0, 0);

	P2 p2;
	geometry::assign(p2, 1, 0, 0);
	BOOST_CHECK_EQUAL(double(pythagoras(p1, p2)), 1);
	geometry::assign(p2, 0, 1, 0);
	BOOST_CHECK_EQUAL(double(pythagoras(p1, p2)), 1);
	geometry::assign(p2, 0, 0, 1);
	BOOST_CHECK_EQUAL(double(pythagoras(p1, p2)), 1);
}

template <typename P1, typename P2>
void test_arbitrary()
{
	geometry::strategy::distance::pythagoras<P1, P2> pythagoras;

	P1 p1;
	geometry::assign(p1, 1, 2, 3);
	P2 p2;
	geometry::assign(p2, 9, 8, 7);
	BOOST_CHECK_CLOSE((double)pythagoras(p1, p2), sqrt((double)116), 0.001);
}


template <typename P1, typename P2>
void test_all()
{
	test_null_distance<P1, P2>();
	test_axis<P1, P2>();
	test_arbitrary<P1, P2>();
}

template <typename P>
void test_all()
{
	test_all<P, int[3]>();
	test_all<P, float[3]>();
	test_all<P, double[3]>();
	test_all<P, test_point>();
	test_all<P, point<int, 3, cs::cartesian> >();
	test_all<P, point<float, 3, cs::cartesian> >();
	test_all<P, point<double, 3, cs::cartesian> >();
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
