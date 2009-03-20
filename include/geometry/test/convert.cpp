// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>

#include <geometry/geometries/geometries.hpp>

#include <geometry/algorithms/convert.hpp>
#include <geometry/algorithms/assign.hpp>
#include <geometry/algorithms/make.hpp>

#include <geometry/io/wkt/streamwkt.hpp>

#include "common.hpp"




template <typename P>
void test_all()
{
	typedef geometry::box<P> B;

	P p;
	geometry::assign(p, 1, 2);

	B b;
	geometry::convert(p, b);

	BOOST_CHECK_CLOSE(double(geometry::get<0, 0>(b)), 1.0, 0.001);
	BOOST_CHECK_CLOSE(double(geometry::get<0, 1>(b)), 2.0, 0.001);
	BOOST_CHECK_CLOSE(double(geometry::get<1, 0>(b)), 1.0, 0.001);
	BOOST_CHECK_CLOSE(double(geometry::get<1, 1>(b)), 2.0, 0.001);
}



template <typename P>
void test_std()
{
	test_all<P>();

	typedef geometry::box<P> B;
	typedef geometry::linear_ring<P> R;
	B b;
	geometry::set<geometry::min_corner, 0>(b, 1);
	geometry::set<geometry::min_corner, 1>(b, 2);
	geometry::set<geometry::max_corner, 0>(b, 3);
	geometry::set<geometry::max_corner, 1>(b, 4);

	R ring;
	geometry::convert(b, ring);

	std::cout << b << std::endl;
	std::cout << ring << std::endl;

	typename boost::range_const_iterator<R>::type it = ring.begin();
	BOOST_CHECK_CLOSE(double(geometry::get<0>(*it)), 1.0, 0.001);
	BOOST_CHECK_CLOSE(double(geometry::get<1>(*it)), 2.0, 0.001);
	it++;
	BOOST_CHECK_CLOSE(double(geometry::get<0>(*it)), 1.0, 0.001);
	BOOST_CHECK_CLOSE(double(geometry::get<1>(*it)), 4.0, 0.001);
	it++;
	BOOST_CHECK_CLOSE(double(geometry::get<0>(*it)), 3.0, 0.001);
	BOOST_CHECK_CLOSE(double(geometry::get<1>(*it)), 4.0, 0.001);
	it++;
	BOOST_CHECK_CLOSE(double(geometry::get<0>(*it)), 3.0, 0.001);
	BOOST_CHECK_CLOSE(double(geometry::get<1>(*it)), 2.0, 0.001);
	it++;
	BOOST_CHECK_CLOSE(double(geometry::get<0>(*it)), 1.0, 0.001);
	BOOST_CHECK_CLOSE(double(geometry::get<1>(*it)), 2.0, 0.001);

	BOOST_CHECK_EQUAL(ring.size(), 5);
}


int test_main(int, char* [])
{
	test_all<int[2]>();
	test_all<float[2]>();
	test_all<double[2]>();
	//test_all<test_point>();
	test_std<geometry::point<int, 2, geometry::cs::cartesian> >();
	test_std<geometry::point<float, 2, geometry::cs::cartesian> >();
	test_std<geometry::point<double, 2, geometry::cs::cartesian> >();

	return 0;
}
