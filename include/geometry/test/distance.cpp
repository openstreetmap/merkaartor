// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


//#define TEST_ARRAY


#include <boost/test/included/test_exec_monitor.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <geometry/algorithms/distance.hpp>

#ifndef TEST_ARRAY
#include <geometry/algorithms/make.hpp>
#endif

#include <geometry/geometries/geometries.hpp>
#include <geometry/geometries/adapted/boost_array_as_linestring.hpp>

#include "common.hpp"


template <typename P>
void test_distance_result()
{
	typedef typename geometry::distance_result<P, P>::type RET;

#ifndef TEST_ARRAY
	P p1 = geometry::make<P>(0, 0);
	P p2 = geometry::make<P>(3, 0);
	P p3 = geometry::make<P>(0, 4);
#else
	P p1, p2, p3;
	geometry::set<0>(p1, 0); geometry::set<1>(p1, 0);
	geometry::set<0>(p2, 3); geometry::set<1>(p2, 0);
	geometry::set<0>(p3, 0); geometry::set<1>(p3, 4);
#endif

	RET dr12 = geometry::distance(p1, p2);
	RET dr13 = geometry::distance(p1, p3);
	RET dr23 = geometry::distance(p2, p3);

	BOOST_CHECK_CLOSE(double(dr12), 3.000, 0.001);
	BOOST_CHECK_CLOSE(double(dr13), 4.000, 0.001);
	BOOST_CHECK_CLOSE(double(dr23), 5.000, 0.001);

	// COMPILATION TESTS
	RET comparable = geometry::make_distance_result<RET>(3);
	//BOOST_CHECK_CLOSE(comparable.value(), 9.000, 0.001);


	// Question: how to check if the implemented operator is used, and not the auto-conversion to double?
	if (comparable == dr12);
	if (comparable < dr12);
	if (comparable > dr12);

	// Check streamability
	std::ostringstream s;
	s << comparable;

	// Check comparisons with plain double
	double d = 3.0;
	if (dr12 == d);
	if (dr12 < d);
	if (dr12 > d);

}


template <typename P>
void test_distance_point()
{
 	P p1;
	geometry::set<0>(p1, 1);
	geometry::set<1>(p1, 1);

	P p2;
	geometry::set<0>(p2, 2);
	geometry::set<1>(p2, 2);

	double d = geometry::distance(p1, p2);
	BOOST_CHECK_CLOSE(d, 1.4142135, 0.001);
}


template <typename P>
void test_distance_segment()
{
 	P s1; geometry::set<0>(s1, 2); geometry::set<1>(s1, 2);
	P s2; geometry::set<0>(s2, 3); geometry::set<1>(s2, 3);

	// Check points left, right, projected-left, projected-right, on segment
	P p1; geometry::set<0>(p1, 0); geometry::set<1>(p1, 0);
	P p2; geometry::set<0>(p2, 4); geometry::set<1>(p2, 4);
	P p3; geometry::set<0>(p3, 2.4); geometry::set<1>(p3, 2.6);
	P p4; geometry::set<0>(p4, 2.6); geometry::set<1>(p4, 2.4);
	P p5; geometry::set<0>(p5, 2.5); geometry::set<1>(p5, 2.5);

	const geometry::segment<const P> seg(s1, s2);

	double d1 = geometry::distance(p1, seg); BOOST_CHECK_CLOSE(d1, 2.8284271, 0.001);
	double d2 = geometry::distance(p2, seg); BOOST_CHECK_CLOSE(d2, 1.4142135, 0.001);
	double d3 = geometry::distance(p3, seg); BOOST_CHECK_CLOSE(d3, 0.141421, 0.001);
	double d4 = geometry::distance(p4, seg); BOOST_CHECK_CLOSE(d4, 0.141421, 0.001);
	double d5 = geometry::distance(p5, seg); BOOST_CHECK_CLOSE(d5, 0.0, 0.001);

	// check other way round
	double dr1 = geometry::distance(seg, p1); BOOST_CHECK_CLOSE(dr1, d1, 0.001);
	double dr2 = geometry::distance(seg, p2); BOOST_CHECK_CLOSE(dr2, d2, 0.001);
}

template <typename P>
void test_distance_linestring()
{
	typedef typename geometry::coordinate_type<P>::type T;

	boost::array<P, 2> points;
	geometry::set<0>(points[0], 1);
	geometry::set<1>(points[0], 1);
	geometry::set<0>(points[1], 3);
	geometry::set<1>(points[1], 3);

#ifndef TEST_ARRAY
	P p = geometry::make<P>(2, 1);
#else
	P p;
	geometry::set<0>(p, 2); geometry::set<1>(p, 1);
#endif

	double d = geometry::distance(p, points);
	BOOST_CHECK_CLOSE(d, 0.70710678, 0.001);

#ifndef TEST_ARRAY
	p = geometry::make<P>(5, 5);
#else
	geometry::set<0>(p, 5); geometry::set<1>(p, 5);
#endif
	d = geometry::distance(p, points);
	BOOST_CHECK_CLOSE(d, 2.828427, 0.001);


	geometry::linestring<P> line;
#ifndef TEST_ARRAY
	line.push_back(geometry::make<P>(1,1));
	line.push_back(geometry::make<P>(2,2));
	line.push_back(geometry::make<P>(3,3));
#else
	{
		P lp;
		geometry::set<0>(lp, 1); geometry::set<1>(lp, 1); line.push_back(lp);
		geometry::set<0>(lp, 2); geometry::set<1>(lp, 2); line.push_back(lp);
		geometry::set<0>(lp, 3); geometry::set<1>(lp, 3); line.push_back(lp);
	}
#endif

#ifndef TEST_ARRAY
	p = geometry::make<P>(5, 5);
#else
	geometry::set<0>(p, 5); geometry::set<1>(p, 5);
#endif

	d = geometry::distance(p, line);
	BOOST_CHECK_CLOSE(d, 2.828427, 0.001);
}


template <typename P>
void test_all()
{
	test_distance_result<P>();
	test_distance_point<P>();
	test_distance_segment<P>();
#ifndef TEST_ARRAY
	test_distance_linestring<P>();
#endif
}



int test_main(int, char* [])
{
#ifdef TEST_ARRAY
	//test_all<int[2]>();
	test_all<float[2]>();
	test_all<double[2]>();
	test_all<test_point>(); // located here because of 3D
#endif

	//test_all<geometry::point_xy<int> >();
	test_all<boost::tuple<float, float> >();
	test_all<geometry::point_xy<float> >();
	test_all<geometry::point_xy<double> >();

	return 0;
}


