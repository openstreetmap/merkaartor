// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>

#include <geometry/arithmetic/arithmetic.hpp>

#include <geometry/algorithms/assign.hpp>

#include "common.hpp"


using namespace geometry;


template <typename P>
void test_addition()
{
	P p1;
	geometry::assign(p1, 1, 2, 3);
	geometry::add_value(p1, 10);
	BOOST_CHECK(get<0>(p1) == 11);
	BOOST_CHECK(get<1>(p1) == 12);
	BOOST_CHECK(get<2>(p1) == 13);

	P p2;
	geometry::assign(p2, 4, 5, 6);
	geometry::add_point(p1, p2);
	BOOST_CHECK(get<0>(p1) == 15);
	BOOST_CHECK(get<1>(p1) == 17);
	BOOST_CHECK(get<2>(p1) == 19);
}

template <typename P>
void test_subtraction()
{
	P p1;
	geometry::assign(p1, 1, 2, 3);
	geometry::subtract_value(p1, 10);
	BOOST_CHECK(get<0>(p1) == -9);
	BOOST_CHECK(get<1>(p1) == -8);
	BOOST_CHECK(get<2>(p1) == -7);

	P p2;
	geometry::assign(p2, 4, 6, 8);
	geometry::subtract_point(p1, p2);
	BOOST_CHECK(get<0>(p1) == -13);
	BOOST_CHECK(get<1>(p1) == -14);
	BOOST_CHECK(get<2>(p1) == -15);
}

template <typename P>
void test_multiplication()
{
	P p1;
	geometry::assign(p1, 1, 2, 3);
	geometry::multiply_value(p1, 5);
	BOOST_CHECK(get<0>(p1) == 5);
	BOOST_CHECK(get<1>(p1) == 10);
	BOOST_CHECK(get<2>(p1) == 15);

	P p2;
	geometry::assign(p2, 4, 5, 6);
	geometry::multiply_point(p1, p2);
	BOOST_CHECK(get<0>(p1) == 20);
	BOOST_CHECK(get<1>(p1) == 50);
	BOOST_CHECK(get<2>(p1) == 90);
}

template <typename P>
void test_division()
{
	P p1;
	geometry::assign(p1, 50, 100, 150);
	geometry::divide_value(p1, 5);
	BOOST_CHECK(get<0>(p1) == 10);
	BOOST_CHECK(get<1>(p1) == 20);
	BOOST_CHECK(get<2>(p1) == 30);

	P p2;
	geometry::assign(p2, 2, 4, 6);
	geometry::divide_point(p1, p2);
	BOOST_CHECK(get<0>(p1) == 5);
	BOOST_CHECK(get<1>(p1) == 5);
	BOOST_CHECK(get<2>(p1) == 5);
}


template <typename P>
void test_all()
{
	test_addition<P>();
	test_subtraction<P>();
	test_multiplication<P>();
	test_division<P>();
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
