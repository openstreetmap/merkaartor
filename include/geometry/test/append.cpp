// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>

#include <boost/concept/requires.hpp>

#include <geometry/core/access.hpp>
#include <geometry/algorithms/make.hpp>

#include <geometry/algorithms/clear.hpp>
#include <geometry/algorithms/append.hpp>
#include <geometry/algorithms/num_points.hpp>

#include <geometry/geometries/geometries.hpp>
#include <geometry/geometries/adapted/std_as_linestring.hpp>
#include <geometry/geometries/adapted/boost_array_as_linestring.hpp>

#include <geometry/core/concepts/linestring_concept.hpp>


#include "common.hpp"



template <typename G>
void test_geometry()
{
	//BOOST_CONCEPT_ASSERT((geometry::Linestring<G>));
	//BOOST_CONCEPT_ASSERT((geometry::ConstLinestring<G>));

	G geometry;
	typedef typename geometry::point_type<G>::type P;

	geometry::append(geometry, geometry::make_zero<P>());
	BOOST_CHECK_EQUAL(geometry::num_points(geometry), 1);

	geometry::clear(geometry);
	BOOST_CHECK_EQUAL(geometry::num_points(geometry), 0);
	//P p = boost::range::front(geometry);
}

template <typename P>
void test_all()
{
	test_geometry<geometry::linestring<P> >();
	test_geometry<geometry::linear_ring<P> >();
	test_geometry<geometry::polygon<P> >();

	test_geometry<std::vector<P> >();
	test_geometry<std::deque<P> >();
	//test_geometry<std::list<P> >();
}



int test_main(int, char* [])
{
	test_all<test_point>();
	test_all<geometry::point<int, 2, geometry::cs::cartesian> >();
	test_all<geometry::point<float, 2, geometry::cs::cartesian> >();
	test_all<geometry::point<double, 2, geometry::cs::cartesian> >();

 	return 0;
}
