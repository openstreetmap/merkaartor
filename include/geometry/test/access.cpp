// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>

#include <geometry/algorithms/make.hpp>
#include <geometry/geometries/geometries.hpp>

#include "common.hpp"

template <typename G>
void test_get_set()
{
	typedef typename geometry::coordinate_type<G>::type T;

	G g;
	geometry::set<0>(g, T(1));
	geometry::set<1>(g, T(2));

	T x = geometry::get<0>(g);
	T y = geometry::get<1>(g);

	BOOST_CHECK_CLOSE(double(x), 1.0, 0.0001);
	BOOST_CHECK_CLOSE(double(y), 2.0, 0.0001);
}


template <typename G>
void test_indexed_get_set(G& g)
{
	geometry::set<0, 0>(g, 1);
	geometry::set<0, 1>(g, 2);
	geometry::set<1, 0>(g, 3);
	geometry::set<1, 1>(g, 4);

	typedef typename geometry::coordinate_type<G>::type T;
	T x1 = geometry::get<0, 0>(g);
	T y1 = geometry::get<0, 1>(g);
	T x2 = geometry::get<1, 0>(g);
	T y2 = geometry::get<1, 1>(g);

	BOOST_CHECK_CLOSE(double(x1), 1.0, 0.0001);
	BOOST_CHECK_CLOSE(double(y1), 2.0, 0.0001);
	BOOST_CHECK_CLOSE(double(x2), 3.0, 0.0001);
	BOOST_CHECK_CLOSE(double(y2), 4.0, 0.0001);
}

template <typename G, typename T>
void test_indexed_get(const G& g, T a, T b, T c, T d)
{
	T x1 = geometry::get<0, 0>(g);
	T y1 = geometry::get<0, 1>(g);
	T x2 = geometry::get<1, 0>(g);
	T y2 = geometry::get<1, 1>(g);

	BOOST_CHECK_CLOSE(double(x1), double(a), 0.0001);
	BOOST_CHECK_CLOSE(double(y1), double(b), 0.0001);
	BOOST_CHECK_CLOSE(double(x2), double(c), 0.0001);
	BOOST_CHECK_CLOSE(double(y2), double(d), 0.0001);
}

template <typename P>
void test_all()
{
	typedef typename geometry::coordinate_type<P>::type T;

	// POINT, setting coordinate
	test_get_set<P>();

	// N-SPHERE, setting sphere center
	test_get_set<geometry::nsphere<P, double> >();

	// BOX, setting left/right/top/bottom
	geometry::box<P> b;
	test_indexed_get_set(b);

	// SEGMENT (in GGL not having default constructor; however that is not a requirement)
	P p1 = geometry::make_zero<P>(), p2 = geometry::make_zero<P>();
	geometry::segment<P> s(p1, p2);
	test_indexed_get_set(s);

	// CONST SEGMENT
	geometry::set<0>(p1, 1); // we don't use assign because dim in {2,3}
	geometry::set<1>(p1, 2);
	geometry::set<0>(p2, 3);
	geometry::set<1>(p2, 4);
	geometry::segment<const P> cs(p1, p2);
	test_indexed_get(cs, T(1), T(2), T(3), T(4));

	// TODO, linestring, linear_ring, polygon
}



int test_main(int, char* [])
{
	test_get_set<int[2]>();
	test_get_set<float[2]>();
	test_get_set<double[2]>();

	//test_all<test_point>();
	test_all<geometry::point<int, 2, geometry::cs::cartesian> >();
	test_all<geometry::point<float, 2, geometry::cs::cartesian> >();
	test_all<geometry::point<double, 2, geometry::cs::cartesian> >();

 	return 0;
}
