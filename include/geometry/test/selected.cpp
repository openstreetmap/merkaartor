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
#include <geometry/algorithms/selected.hpp>

#include <geometry/geometries/geometries.hpp>

#include "common.hpp"


template <typename G, typename P>
void test_selected(const G& g, const P& point, bool result, double dist)
{
	bool sel = geometry::selected(g, point, dist);
	BOOST_CHECK_EQUAL(sel, result);
}


template <typename G, typename P>
void test_selected(const std::string& wkt, const P& point, bool result, double dist)
{
	G g;
	geometry::from_wkt(wkt, g);
	test_selected(g, point, result, dist);
}




template <typename P>
void test_all()
{
	test_selected<P>("POINT(1 1)", P(1,1), true, 0.001);
	test_selected<P>("POINT(1 1)", P(3,3), false, 2);
	test_selected<P>("POINT(1 1)", P(1,2.00001), false, 1);
	test_selected<P>("POINT(1 1)", P(1,1.99999), true, 1);
	test_selected<P>("POINT(1 1)", P(1.99999,1.99999), false, 1);

	test_selected<geometry::linestring<P> >("LINESTRING(1 1,2 2)", P(1,1), true, 0.0001);
	test_selected<geometry::linestring<P> >("LINESTRING(1 1,2 2)", P(2,2), true, 0.0001);
	test_selected<geometry::linestring<P> >("LINESTRING(1 1,2 2)", P(2.01,2.01), false, 0.0001);
	test_selected<geometry::linestring<P> >("LINESTRING(1 1,2 2)", P(1,0.9), false, 0.0001);
	test_selected<geometry::linestring<P> >("LINESTRING(1 1,2 2)", P(1.5,1.5), true, 0.0001);
	test_selected<geometry::linestring<P> >("LINESTRING(1 1,2 2)", P(1.5,1.6), false, 0.0001);
	test_selected<geometry::linestring<P> >("LINESTRING(1 1,2 2,3 0,5 0,5 8)", P(5,5.000001), true, 0.0001);

	// Lines with zero,one points
	test_selected<geometry::linestring<P> >("LINESTRING( )", P(1,1), false, 0.0001);
	test_selected<geometry::linestring<P> >("LINESTRING(1 1)", P(1,1), true, 0.0001);
	test_selected<geometry::linestring<P> >("LINESTRING(1 2)", P(1,1), false, 0.0001);

	// nyi
	//test_selected<geometry::linear_ring<P> >();

	test_selected<geometry::polygon<P> >("POLYGON((0 0,0 7,4 2,2 0,0 0))", P(0.001, 0.001), true, 0.0001);
	test_selected<geometry::polygon<P> >("POLYGON((0 0,0 7,4 2,2 0,0 0))", P(1, 1), true, 0.0001);
	test_selected<geometry::polygon<P> >("POLYGON((0 0,0 7,4 2,2 0,0 0))", P(2, 5), false, 0.0001);

	typedef geometry::box<P> B;
	test_selected(geometry::make<B>(0,0,4,4), P(2,2), true, 0.001);
	test_selected(geometry::make<B>(0,0,4,4), P(5,5), false, 0.001);
	test_selected(geometry::make<B>(0,0,4,4), P(0,0), false, 0.001);
	test_selected(geometry::make<B>(0,0,4,4), P(4,4), false, 0.001);

	// nyi
	//test_selected<geometry::segment<P> >();
	//test_selected<geometry::segment<const P> >();
	//test_selected<geometry::nsphere<P, double> >();
}


int test_main(int, char* [])
{
	// Integer not applicable here, just because of the tests using floating point
	// test_all<geometry::point_xy<int> >();

	test_all<geometry::point_xy<float> >();
	test_all<geometry::point_xy<double> >();

	return 0;
}


