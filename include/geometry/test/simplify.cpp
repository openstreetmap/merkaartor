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
#include <geometry/algorithms/simplify.hpp>

#include <geometry/geometries/geometries.hpp>

#include "common.hpp"




template <typename P>
void test_simplify_linestring()
{
	typedef typename geometry::coordinate_type<P>::type T;

	geometry::linestring<P> line;

	// Generate linestring using only integer coordinates and obvious results (point 5,5 will be removed)
	geometry::from_wkt("linestring(0 0, 5 5, 10 10)", line);

	// Check using linestring, TO BE UPDATED
	/*
	{
		geometry::linestring<P> simplified;
		geometry::simplify(line, simplified, 3.0);
		BOOST_CHECK(simplified.size() == 2);
	}
	*/

	// Check using output iterator
	{
		geometry::linestring<P> simplified;
		geometry::simplify(line, std::back_inserter(simplified), 3.0);
		BOOST_CHECK(simplified.size() == 2);
	}

	// define strategy
	typedef typename geometry::cs_tag<P>::type TAG;
	typedef geometry::strategy::distance::xy_point_segment<P, geometry::segment<const P> > S;
	typedef geometry::linestring<P> L;
	typedef geometry::strategy::simplify::douglas_peucker<L, std::back_insert_iterator<L>, S> DOUGLAS;


	// Check using iterators and simplify strategy
	{
		geometry::linestring<P> simplified;
		geometry::simplify(line, std::back_inserter(simplified), 3.0, DOUGLAS());
		BOOST_CHECK(simplified.size() == 2);
	}
}



template <typename P>
void test_simplify_polygon()
{
	{
		geometry::polygon<P> poly;

		// Generate polygon using only integer coordinates and obvious results
		// Polygon is a hexagon, having one extra point (2,1) on a line which should be filtered out.
		geometry::from_wkt("polygon((4 0, 8 2, 8 7, 4 9, 0 7, 0 2, 2 1, 4 0))", poly);
		BOOST_CHECK(poly.outer().size() == 8);

		geometry::polygon<P> simplified;
		geometry::simplify(poly, simplified, 1.0);

		//std::cout << poly << std::endl;
		//std::cout << simplified << std::endl;

		BOOST_CHECK(simplified.outer().size() == 7);
	}

	{
		geometry::polygon<P> poly;

		// Same polygon, with a hole (inner ring)
		geometry::from_wkt("polygon((4 0, 8 2, 8 7, 4 9, 0 7, 0 2, 2 1, 4 0),(7 3, 7 6, 1 6, 1 3, 4 3, 7 3))", poly);
		BOOST_CHECK(poly.outer().size() == 8);
		BOOST_CHECK(poly.inners().size() == 1);
		BOOST_CHECK(poly.inners().front().size() == 6);

		geometry::polygon<P> simplified;
		geometry::simplify(poly, simplified, 1.0);

		//std::cout << poly << std::endl;
		//std::cout << simplified << std::endl;

		BOOST_CHECK(simplified.outer().size() == 7);
		BOOST_CHECK(simplified.inners().size() == 1);
		BOOST_CHECK(simplified.inners().front().size() == 5);
	}
}



template <typename P>
void test_all()
{
	test_simplify_linestring<P>();
	test_simplify_polygon<P>();
}


int test_main(int, char* [])
{
	// Integer still fails
	//test_all<geometry::point_xy<int> >();
	test_all<geometry::point_xy<float> >();
	test_all<geometry::point_xy<double> >();

	return 0;
}


