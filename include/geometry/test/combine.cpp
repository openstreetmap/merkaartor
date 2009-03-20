// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>

#include <geometry/algorithms/buffer.hpp>
#include <geometry/algorithms/combine.hpp>

#include <geometry/core/concepts/box_concept.hpp>

#include <geometry/geometries/box.hpp>

#include "common.hpp"

using namespace geometry;


template <typename P>
box<P> create_box()
{
	P p1;
	P p2;
	geometry::assign(p1, 1, 2, 5);
	geometry::assign(p2, 3, 4, 6);
	return box<P>(p1, p2);
}

template <typename B, typename T>
void check_box(B& to_check,
               T min_x, T min_y, T min_z,
               T max_x, T max_y, T max_z)
{
	BOOST_CHECK_EQUAL((geometry::get<geometry::min_corner, 0>(to_check)), min_x);
	BOOST_CHECK_EQUAL((geometry::get<geometry::min_corner, 1>(to_check)), min_y);
	BOOST_CHECK_EQUAL((geometry::get<geometry::min_corner, 2>(to_check)), min_z);
	BOOST_CHECK_EQUAL((geometry::get<geometry::max_corner, 0>(to_check)), max_x);
	BOOST_CHECK_EQUAL((geometry::get<geometry::max_corner, 1>(to_check)), max_y);
	BOOST_CHECK_EQUAL((geometry::get<geometry::max_corner, 2>(to_check)), max_z);
}

template <typename P, typename T>
void combine_point_and_check(box<P>& to_grow,
                      T grow_x, T grow_y, T grow_z,
                      T result_min_x, T result_min_y, T result_min_z,
                      T result_max_x, T result_max_y, T result_max_z)
{
	P p;
	geometry::assign(p, grow_x, grow_y, grow_z);
	combine(to_grow, p);

	check_box(to_grow,
	          result_min_x, result_min_y, result_min_z,
	          result_max_x, result_max_y, result_max_z
	);
}

template <typename P, typename T>
void combine_box_and_check(box<P>& to_grow,
                      T grow_min_x, T grow_min_y, T grow_min_z,
                      T grow_max_x, T grow_max_y, T grow_max_z,
                      T result_min_x, T result_min_y, T result_min_z,
                      T result_max_x, T result_max_y, T result_max_z)
{
	P p1, p2;
	geometry::assign(p1, grow_min_x, grow_min_y, grow_min_z);
	geometry::assign(p2, grow_max_x, grow_max_y, grow_max_z);
	box<P> grow_box(p1, p2);
	combine(to_grow, grow_box);

	check_box(to_grow,
	          result_min_x, result_min_y, result_min_z,
	          result_max_x, result_max_y, result_max_z);
}

template <typename P, typename T>
void buffer_box_and_check(box<P>& to_grow, T factor,
                      T result_min_x, T result_min_y, T result_min_z,
                      T result_max_x, T result_max_y, T result_max_z)
{
	box<P> out;
	buffer(to_grow, out, factor);
	to_grow = out;

	check_box(to_grow,
	          result_min_x, result_min_y, result_min_z,
	          result_max_x, result_max_y, result_max_z);

	// Compile test:
	box<P> out2 = make_buffer<box<P> >(out, factor);
}


template <typename P>
void test_combine_point()
{
	box<P> b(create_box<P>());
	combine_point_and_check(b, 4,4,5,   1,2,5,4,4,6);
	combine_point_and_check(b, 4,5,5,   1,2,5,4,5,6);
	combine_point_and_check(b, 10,10,4, 1,2,4,10,10,6);
	combine_point_and_check(b, 9,9,4,   1,2,4,10,10,6);

	combine_point_and_check(b, 0,2,7,   0,2,4,10,10,7);
	combine_point_and_check(b, 0,0,7,   0,0,4,10,10,7);
	combine_point_and_check(b, -1,-1,5, -1,-1,4,10,10,7);
	combine_point_and_check(b, 0,0,5,   -1,-1,4,10,10,7);

	combine_point_and_check(b, 15,-1,0, -1,-1,0,15,10,7);
	combine_point_and_check(b, -1,15,10, -1,-1,0,15,15,10);
}

template <typename P>
void test_combine_box()
{
	box<P> b(create_box<P>());

	combine_box_and_check(b, 0,2,5,4,4,6,     0,2,5,4,4,6);
	combine_box_and_check(b, 0,1,5,4,6,6,     0,1,5,4,6,6);
	combine_box_and_check(b, -1,-1,6,10,10,5, -1,-1,5,10,10,6);
	combine_box_and_check(b, 3,3,6,3,3,5,     -1,-1,5,10,10,6);

	combine_box_and_check(b, 3,15,7,-1,3,4,   -1,-1,4,10,15,7);
	combine_box_and_check(b, -15,3,7,3,20,4,   -15,-1,4,10,20,7);
	combine_box_and_check(b, 3,-20,8,3,20,3,   -15,-20,3,10,20,8);
	combine_box_and_check(b, -20,3,8,20,3,3,   -20,-20,3,20,20,8);
}

template <typename P>
void test_buffer_box()
{
	box<P> b(create_box<P>());

	buffer_box_and_check(b, 5, -4,-3,0,8,9,11);
	buffer_box_and_check(b, -2, -2,-1,2,6,7,9);
	buffer_box_and_check(b, -100, 98,99,102,-94,-93,-91);
}


template <typename P>
void test_all()
{
	test_combine_point<P>();
	test_combine_box<P>();
	test_buffer_box<P>();
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
