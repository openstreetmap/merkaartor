// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>

#include <geometry/algorithms/make.hpp>

#include <geometry/core/concepts/box_concept.hpp>

#include <geometry/geometries/geometries.hpp>

#include <geometry/io/wkt/aswkt.hpp>


#include "common.hpp"

using namespace geometry;

template <typename T, typename P>
void test_point_2d()
{
	P p = make<P>((T) 123, (T) 456);
	BOOST_CHECK_CLOSE( ((double) get<0>(p)), (double) 123, 1.0e-6);
	BOOST_CHECK_CLOSE( ((double) get<1>(p)), (double) 456, 1.0e-6);

}


template <typename T, typename P>
void test_point_3d()
{
	P p = make<P>((T) 123, (T) 456, (T) 789);
	BOOST_CHECK_CLOSE( ((double) get<0>(p)), (double) 123, 1.0e-6);
	BOOST_CHECK_CLOSE( ((double) get<1>(p)), (double) 456, 1.0e-6);
	BOOST_CHECK_CLOSE( ((double) get<2>(p)), (double) 789, 1.0e-6);
}


template <typename T, typename P>
void test_box_2d()
{
	typedef box<P> B;
	B b = make<B>((T) 123, (T) 456, (T) 789, (T) 1011);
	BOOST_CHECK_CLOSE( ((double) get<min_corner, 0>(b)), (double) 123, 1.0e-6);
	BOOST_CHECK_CLOSE( ((double) get<min_corner, 1>(b)), (double) 456, 1.0e-6);
	BOOST_CHECK_CLOSE( ((double) get<max_corner, 0>(b)), (double) 789, 1.0e-6);
	BOOST_CHECK_CLOSE( ((double) get<max_corner, 1>(b)), (double) 1011, 1.0e-6);

	b = make_inverse<B>();
}


template <typename T, typename P>
void test_linestring_2d()
{
	typedef linestring<P> L;

	T coors[][2] = {{1,2}, {3,4}};

	L line = make<L>(coors);

	BOOST_CHECK_EQUAL(line.size(), 2);
}

template <typename T, typename P>
void test_linestring_3d()
{
	typedef linestring<P> L;

	T coors[][3] = {{1,2,3}, {4,5,6}};

	L line = make<L>(coors);

	BOOST_CHECK_EQUAL(line.size(), 2);
	//std::cout << make_wkt(line) << std::endl;

}

template <typename T, typename P>
void test_2d_t()
{
	test_point_2d<T, P>();
	test_box_2d<T, P>();
	test_linestring_2d<T, P>();
}


template <typename P>
void test_2d()
{
	test_2d_t<int, P>();
	test_2d_t<float, P>();
	test_2d_t<double, P>();
}


template <typename T, typename P>
void test_3d_t()
{
	test_linestring_3d<T, P>();
//	test_point_3d<T, test_point>();
}

template <typename P>
void test_3d()
{
	test_3d_t<int, P>();
	test_3d_t<float, P>();
	test_3d_t<double, P>();
}


int test_main(int, char* [])
{
	//test_2d<int[2]>();
	//test_2d<float[2]>();
	//test_2d<double[2]>();
	test_2d<point<int, 2, cs::cartesian> >();
	test_2d<point<float, 2, cs::cartesian> >();
	test_2d<point<double, 2, cs::cartesian> >();


	test_3d<point<double, 3, cs::cartesian> >();

	return 0;
}
