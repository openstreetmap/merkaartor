// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <sstream>
#include <boost/test/included/test_exec_monitor.hpp>
#include <boost/type_traits/is_integral.hpp>

#include <geometry/io/wkt/streamwkt.hpp>

#include <geometry/geometries/box.hpp>
#include <geometry/geometries/linestring.hpp>
#include <geometry/geometries/polygon.hpp>


#include "common.hpp"


using namespace geometry;


template <typename P>
void test_point_wkt()
{
	P p;
	geometry::assign(p, 1, 23, 456);
	std::ostringstream oss;
	oss << p;
	BOOST_CHECK_EQUAL(oss.str(), "POINT(1 23 456)");

	if (!boost::is_integral<typename coordinate_type<P>::type>::type::value)
	{
		geometry::assign(p, 0.1f, 2.34f, 5.6789f);
		std::ostringstream oss;
		oss << p;
		BOOST_CHECK_EQUAL(oss.str(), "POINT(0.1 2.34 5.6789)");
	}
}

template <typename P>
void test_linestring()
{
	linestring<P> ls;
	P p;

	geometry::assign(p, 1, 2, 3);
	ls.push_back(p);
	geometry::assign(p, 40, 50, 60);
	ls.push_back(p);
	geometry::assign(p, 700, 800, 900);
	ls.push_back(p);

	std::ostringstream oss;
	oss << ls;
	BOOST_CHECK_EQUAL(oss.str(), "LINESTRING(1 2 3,40 50 60,700 800 900)");
}

template <typename P>
void test_polygon_wkt()
{
	polygon<P> p;
	P pt;

	geometry::assign(pt, 100, 200, 300);
	p.outer().push_back(pt);
	geometry::assign(pt, 400, 500, 600);
	p.outer().push_back(pt);
	geometry::assign(pt, 700, 800, 900);
	p.outer().push_back(pt);

	p.inners().resize(2);
	geometry::assign(pt, 10, 20, 30);
	p.inners()[0].push_back(pt);
	geometry::assign(pt, 40, 50, 60);
	p.inners()[0].push_back(pt);
	geometry::assign(pt, 70, 80, 90);
	p.inners()[0].push_back(pt);
	geometry::assign(pt, 1, 2, 3);
	p.inners()[1].push_back(pt);
	geometry::assign(pt, 4, 5, 6);
	p.inners()[1].push_back(pt);
	geometry::assign(pt, 7, 8, 9);
	p.inners()[1].push_back(pt);

	std::ostringstream oss;
	oss << p;
	BOOST_CHECK_EQUAL(oss.str(), "POLYGON((100 200 300,400 500 600,700 800 900),(10 20 30,40 50 60,70 80 90),(1 2 3,4 5 6,7 8 9))");
}


template <typename P>
void test_all()
{
	test_point_wkt<P>();
	test_linestring<P>();
	test_polygon_wkt<P>();
}


int test_main(int, char* [])
{
	test_point_wkt<int[3]>();
	test_point_wkt<float[3]>();
	test_point_wkt<double[3]>();

	test_all<test_point>();
	test_all<point<int, 3, cs::cartesian> >();
	test_all<point<float, 3, cs::cartesian> >();
	test_all<point<double, 3, cs::cartesian> >();

	return 0;
}
