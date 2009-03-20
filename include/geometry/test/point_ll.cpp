// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <geometry/geometries/point_ll.hpp>
#include <geometry/algorithms/transform.hpp>
#include <geometry/io/wkt/streamwkt.hpp>

#include "common.hpp"



template <typename P>
void test_all()
{
	typedef typename geometry::coordinate_type<P>::type T;

	P p1(
		geometry::latitude<T>(geometry::dms<geometry::south, T>(12, 2, 36)),
		geometry::longitude<T>(geometry::dms<geometry::west, T>(77, 1, 42)));

	// Check decimal/degree conversion
	BOOST_CHECK_CLOSE(geometry::get<0>(p1), T(-77.0283), 0.001);
	BOOST_CHECK_CLOSE(geometry::get<1>(p1), T(-12.0433), 0.001);

	// Check degree/radian conversion
	geometry::point_ll<T, geometry::cs::geographic<geometry::radian> > p2;
	geometry::transform(p1, p2);

	BOOST_CHECK_CLOSE(geometry::get<0>(p2), T(-1.3444), 0.001);
	BOOST_CHECK_CLOSE(geometry::get<1>(p2), T(-0.210196), 0.001);

	// Check degree/radian conversion back
	P p3;
	geometry::transform(p2, p3);
	BOOST_CHECK_CLOSE(geometry::get<0>(p3), T(-77.0283), 0.001);
	BOOST_CHECK_CLOSE(geometry::get<1>(p3), T(-12.0433), 0.001);


	// Check decimal/degree conversion back
	int d;
	int m;
	double s;
	bool positive;
	char cardinal;

	geometry::dms<geometry::cd_lat, T> d1(geometry::get<0>(p3));
	d1.get_dms(d, m, s, positive, cardinal);

	BOOST_CHECK(d == 77);
	BOOST_CHECK(m == 1);
	BOOST_CHECK_CLOSE(s, double(42), 0.1);
	BOOST_CHECK(positive == false);
	BOOST_CHECK(cardinal == 'S');

	// Check dd conversion as string, back. We cannot do that always because of the precision.
	// Only double gives correct results
	//std::string st = d1.get_dms();
	//std::cout << st << std::endl;
	//BOOST_CHECK(st == "77 1'42\" S");
}


int test_main(int, char* [])
{
	test_all<geometry::point_ll<float> >();
	test_all<geometry::point_ll<double> >();

	return 0;
}
