// Generic Geometry Library
//
// Copyright Barend Gehrels, Geodan B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry/algorithms/within.hpp>
#include <geometry/normalize.hpp>

#include <boost/config.hpp>
#include <boost/test/included/test_exec_monitor.hpp>


int test_main1( int , char* [] )
{
	typedef geometry::point<double> gl_point;
	typedef geometry::circle<gl_point, double> gl_circle;
	typedef geometry::box<gl_point> gl_box;
	typedef geometry::linestring<gl_point> gl_line;
	typedef geometry::linear_ring<gl_point> gl_ring;
	typedef geometry::polygon<gl_point> gl_polygon;
	typedef geometry::multi_polygon<gl_polygon> gl_multi_polygon;


	gl_box box(gl_point(0,0), gl_point(2,2));
	gl_circle circle(gl_point(1, 1), 2.5);
	gl_line line;
	line.push_back(gl_point(1,1));
	line.push_back(gl_point(2,1));
	line.push_back(gl_point(2,2));

	gl_ring ring;
	ring.push_back(gl_point(0,0));
	ring.push_back(gl_point(1,0));
	ring.push_back(gl_point(1,1));
	ring.push_back(gl_point(0,1));
	normalize(ring);

	gl_polygon pol;
	pol.outer() = ring;
	gl_multi_polygon multi_polygon;
	multi_polygon.push_back(pol);


	// Point in circle
	BOOST_CHECK_EQUAL(within(gl_point(2, 1), circle), true);
	BOOST_CHECK_EQUAL(within(gl_point(12, 1), circle), false);

	// Line in circle
	BOOST_CHECK_EQUAL(within(line, circle), true);

	line.push_back(gl_point(10,10));
	BOOST_CHECK_EQUAL(within(line, circle), false);

	// Box/ring/poly/multipoly in circle
	BOOST_CHECK_EQUAL(within(box, circle), true);
	BOOST_CHECK_EQUAL(within(ring, circle), true);

	BOOST_CHECK_EQUAL(within(multi_polygon, circle), true);

	// Point in box
	BOOST_CHECK_EQUAL(within(gl_point(1, 1), box), true);
	BOOST_CHECK_EQUAL(within(gl_point(12, 1), box), false);

	return 0;
}
