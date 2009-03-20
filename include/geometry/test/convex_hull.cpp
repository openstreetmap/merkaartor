// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <geometry/io/wkt/fromwkt.hpp>
#include <geometry/io/wkt/streamwkt.hpp>

#include <geometry/algorithms/area.hpp>
#include <geometry/algorithms/correct.hpp>
#include <geometry/algorithms/convex_hull.hpp>

#include <geometry/geometries/geometries.hpp>

#include "common.hpp"



template <typename P>
void test_check_polygon(const std::string& wkt, int size_original, int size_hull,
							double area_original, double area_hull)
{
	geometry::polygon<P> poly;
	geometry::from_wkt(wkt, poly);

	geometry::polygon<P> hull;
	convex_hull(poly, std::back_inserter(hull.outer()));

	BOOST_CHECK(poly.outer().size() == size_original);
	BOOST_CHECK(hull.outer().size() == size_hull);

	double a = area(poly);
	double ah = area(hull);

	BOOST_CHECK_CLOSE(a, area_original, 0.001);
	BOOST_CHECK_CLOSE(ah, area_hull, 0.001);
}

template <typename P>
void test_check_linestring(const std::string& wkt, int size_original, int size_hull,
							double area_hull)
{
	geometry::linestring<P> ls;
	geometry::from_wkt(wkt, ls);

	geometry::polygon<P> hull;
	convex_hull(ls, std::back_inserter(hull.outer()));

	BOOST_CHECK(ls.size() == size_original);
	BOOST_CHECK(hull.outer().size() == size_hull);

	double ah = area(hull);

	BOOST_CHECK_CLOSE(ah, area_hull, 0.001);
}




template <typename P>
void test_convex_hull_polygon()
{
	// rectangular, with concavity
	test_check_polygon<P>("polygon((1 1, 5 1, 5 4, 4 4, 4 3, 3 3, 3 4, 1 4, 1 1))", 
				9, 5, 11.0, 12.0);
	// from sample polygon, with concavity
	test_check_polygon<P>("polygon((2.0 1.3, 2.4 1.7, 2.8 1.8, 3.4 1.2, 3.7 1.6,3.4 2.0, 4.1 3.0, 5.3 2.6, 5.4 1.2, 4.9 0.8, 2.9 0.7,2.0 1.3))", 
				12, 8, 4.48, 5.245);
}


template <typename P>
void test_convex_hull_linestring()
{
	// from sample linestring, with concavity
	test_check_linestring<P>("linestring(1.1 1.1, 2.5 2.1, 3.1 3.1, 4.9 1.1, 3.1 1.9)", 5, 4, 3.8);
}


template <typename P>
void test_all()
{
	test_convex_hull_linestring<P>();
	test_convex_hull_polygon<P>();
}


int test_main(int, char* [])
{
	//test_all<geometry::point_xy<int> >();
	test_all<geometry::point_xy<float> >();
	test_all<geometry::point_xy<double> >();

	return 0;
}


