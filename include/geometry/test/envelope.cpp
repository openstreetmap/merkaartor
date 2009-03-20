// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <geometry/algorithms/envelope.hpp>
#include <geometry/algorithms/assign.hpp>
#include <geometry/algorithms/make.hpp>

#include <geometry/geometries/geometries.hpp>

#include "common.hpp"




template <typename T, typename B>
void check_result(const B& b, const T& x1, const T& y1, const T& x2, const T& y2)
{
	BOOST_CHECK_CLOSE(((double)geometry::get<geometry::min_corner, 0>(b)), (double)x1, 0.001);
	BOOST_CHECK_CLOSE(((double)geometry::get<geometry::min_corner, 1>(b)), (double)y1, 0.001);

	BOOST_CHECK_CLOSE(((double)geometry::get<geometry::max_corner, 0>(b)), (double)x2, 0.001);
	BOOST_CHECK_CLOSE(((double)geometry::get<geometry::max_corner, 1>(b)), (double)y2, 0.001);
}


template <typename P>
void test_envelope_point()
{
	typedef typename geometry::coordinate_type<P>::type T;

 	P p;
	geometry::assign(p, 1, 1);

	geometry::box<P> b;

	geometry::envelope(p, b);
	check_result<T>(b, 1, 1, 1, 1);
}


template <typename P>
void test_envelope_linestring()
{
	typedef typename geometry::coordinate_type<P>::type T;
	typedef geometry::box<P> B;

 	geometry::linestring<P> line;
	line.push_back(geometry::make<P>(1,1));
	line.push_back(geometry::make<P>(2,2));

	// Check envelope detection by return value and by reference
	check_result<T>(geometry::make_envelope<B>(line), 1, 1, 2, 2);

	B b;
	geometry::envelope(line, b);
	check_result<T>(b, 1, 1, 2, 2);
}


template <typename P>
void test_envelope_polygon()
{
	typedef typename geometry::coordinate_type<P>::type T;
	typedef geometry::box<P> B;

 	geometry::polygon<P> poly;

	poly.outer().push_back(geometry::make<P>(1,1));
	poly.outer().push_back(geometry::make<P>(1,3));
	poly.outer().push_back(geometry::make<P>(3,3));
	poly.outer().push_back(geometry::make<P>(3,3));
	poly.outer().push_back(geometry::make<P>(1,3));

	check_result<T>(geometry::make_envelope<B>(poly.outer()), 1, 1, 3, 3);
	check_result<T>(geometry::make_envelope<B>(poly), 1, 1, 3, 3);

	B b;

	// Check called by polygon
	geometry::envelope(poly, b);
	check_result<T>(b, 1, 1, 3, 3);

	// Check called by ring
	geometry::envelope(poly.outer(), b);
	check_result<T>(b, 1, 1, 3, 3);
}



template <typename P>
void test_all()
{
	test_envelope_point<P>();
	test_envelope_linestring<P>();
	test_envelope_polygon<P>();
}

template <typename P>
void test_3d()
{
	typedef typename geometry::coordinate_type<P>::type T;

 	P p;
	geometry::assign(p, 1, 1, 1);

	geometry::box<P> b;
	geometry::assign_zero(b);

	geometry::envelope(p, b);

	BOOST_CHECK_CLOSE(((double)geometry::get<geometry::min_corner, 0>(b)), 1.0, 0.001);
	BOOST_CHECK_CLOSE(((double)geometry::get<geometry::min_corner, 1>(b)), 1.0, 0.001);
	BOOST_CHECK_CLOSE(((double)geometry::get<geometry::min_corner, 2>(b)), 1.0, 0.001);

	BOOST_CHECK_CLOSE(((double)geometry::get<geometry::max_corner, 0>(b)), 1.0, 0.001);
	BOOST_CHECK_CLOSE(((double)geometry::get<geometry::max_corner, 1>(b)), 1.0, 0.001);
	BOOST_CHECK_CLOSE(((double)geometry::get<geometry::max_corner, 2>(b)), 1.0, 0.001);
}

int test_main(int, char* [])
{
	//test_all<int[2]>();
	//test_all<float[2]>();
	//test_all<double[2]>();
	test_all<boost::tuple<float, float> >();
	test_all<geometry::point_xy<int> >();
	test_all<geometry::point_xy<float> >();
	test_all<geometry::point_xy<double> >();

	test_3d<test_point>();

	return 0;
}


