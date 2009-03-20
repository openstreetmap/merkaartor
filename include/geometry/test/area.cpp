// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <geometry/algorithms/area.hpp>
#include <geometry/algorithms/transform.hpp>
#include <geometry/io/wkt/fromwkt.hpp>

#include <geometry/geometries/geometries.hpp>

#include <geometry/projections/epsg.hpp>
#include <geometry/projections/parameters.hpp>
#include <geometry/projections/proj/sterea.hpp>

#include "common.hpp"

using namespace geometry;


template <typename P>
void test_area_box()
{

 	P min, max;
	set<0>(min, 0);
	set<1>(min, 0);
	set<0>(max, 2);
	set<1>(max, 2);

	box<P> b(min, max);

	double d = area(b);
	BOOST_CHECK_CLOSE(d, 4.0, 0.001);
}


template <typename P, typename T>
void test_area_circle()
{
	nsphere<P, T> c;

	set<0>(c.center(), 0);
	set<1>(c.center(), 0);
	c.radius(2);

	double d = area(c);
	BOOST_CHECK_CLOSE(d, 4 * 3.1415926535897932384626433832795, 0.001);
}


template <typename P>
void test_area_polygon()
{
	// without holes
	polygon<P> poly;
	from_wkt("POLYGON((0 0,0 7,4 2,2 0,0 0))", poly);
	double d = area(poly);
	BOOST_CHECK_CLOSE(d, 16.0, 0.001);

	// with holes
	from_wkt("POLYGON((0 0,0 7,4 2,2 0,0 0), (1 1,2 1,2 2,1 2,1 1))", poly);
	d = area(poly);
	BOOST_CHECK_CLOSE(d, 15.0, 0.001);
}

template <typename PRJ, typename XY, typename LL>
void add_to_ring(const PRJ& prj, const LL& ll,
				 geometry::linear_ring<LL>& ring_ll,
				 geometry::linear_ring<XY>& ring_xy)
{
	ring_ll.push_back(ll);

	XY xy;
	prj.forward(ll, xy);
	ring_xy.push_back(xy);
}

template <typename XY, typename LL>
void test_area_polygon_ll(bool concave, bool hole)
{
	BOOST_ASSERT(! (concave && hole) );

	typedef typename coordinate_type<LL>::type T;

	LL a, r, h, u; // Amsterdam, Rotterdam, The Hague, Utrecht, these cities together are the city group "Randstad"
	// Amsterdam 52°22'23"N 4°53'32"E
	a.lat(dms<north, T>(52, 22, 23));
	a.lon(dms<east, T>(4, 53, 32));

	// Rotterdam 51°55'51"N 4°28'45"E
	r.lat(dms<north, T>(51, 55, 51));
	r.lon(dms<east, T>(4, 28, 45));

	// The hague: 52° 4' 48" N, 4° 18' 0" E
	h.lat(dms<north, T>(52, 4, 48));
	h.lon(dms<east, T>(4, 18, 0));

	// Utrecht
	u.lat(dms<north, T>(52, 5, 36));
	u.lon(dms<east, T>(5, 7, 10));


	// Use the Dutch projection (RD), this is EPSG code 28992
	projection::sterea_ellipsoid<LL, XY> dutch_prj(projection::init(28992));

	// Concave case
	polygon<LL> randstad;
	polygon<XY> randstad_xy;
	add_to_ring(dutch_prj, a, randstad.outer(), randstad_xy.outer());
	add_to_ring(dutch_prj, u, randstad.outer(), randstad_xy.outer());
	if (concave)
	{
		// Add the city "Alphen" to create a convex case
		// Alphen 52° 7' 48" N, 4° 39' 0" E
		LL alphen(latitude<T>(dms<north, T>(52, 7, 48)), longitude<T>(dms<east, T>(4, 39)));
		add_to_ring(dutch_prj, alphen, randstad.outer(), randstad_xy.outer());
	}
	add_to_ring(dutch_prj, r, randstad.outer(), randstad_xy.outer());
	add_to_ring(dutch_prj, h, randstad.outer(), randstad_xy.outer());
	add_to_ring(dutch_prj, a, randstad.outer(), randstad_xy.outer());

	// Hole case
	if (hole)
	{
		// Gouda 52° 1' 12" N, 4° 42' 0" E
		LL gouda(latitude<T>(dms<north, T>(52, 1, 12)), longitude<T>(dms<east, T>(4, 42)));
		// Alphen 52° 7' 48" N, 4° 39' 0" E
		LL alphen(latitude<T>(dms<north, T>(52, 7, 48)), longitude<T>(dms<east, T>(4, 39)));
		// Uithoorn  52° 13' 48" N, 4° 49' 48" E
		LL uithoorn(latitude<T>(dms<north, T>(52, 13, 48)), longitude<T>(dms<east, T>(4, 49, 48)));
		// Woerden 52° 5' 9" N, 4° 53' 0" E
		LL woerden(latitude<T>(dms<north, T>(52, 5, 9)), longitude<T>(dms<east, T>(4, 53, 0)));

		randstad.inners().resize(1);
		randstad_xy.inners().resize(1);

		typename polygon<LL>::ring_type& ring = randstad.inners()[0];
		typename polygon<XY>::ring_type& ring_xy = randstad_xy.inners()[0];

		add_to_ring(dutch_prj, gouda, ring, ring_xy);
		add_to_ring(dutch_prj, alphen, ring, ring_xy);
		add_to_ring(dutch_prj, uithoorn, ring, ring_xy);
		add_to_ring(dutch_prj, woerden, ring, ring_xy);
		add_to_ring(dutch_prj, gouda, ring, ring_xy);
	}


	// Check the area in square KM
	static const double KM2 = 1.0e6;
	double d_ll = area(randstad) / KM2;
	double d_xy = area(randstad_xy) / KM2;

	BOOST_CHECK_CLOSE(d_ll, d_xy, 1.0);
	if (hole)
	{
		BOOST_CHECK_CLOSE(d_ll, 1148.210, 0.01);
		BOOST_CHECK_CLOSE(d_xy, 1151.573, 0.01);
	}
	else
	{
		BOOST_CHECK_CLOSE(d_ll, concave ? 977.786 : 1356.168, 0.01);
		BOOST_CHECK_CLOSE(d_xy, concave ? 980.658 : 1360.140, 0.01);

		// No hole: area of outer should be equal to area of ring
		double r_ll = area(randstad.outer()) / KM2;
		double r_xy = area(randstad.outer()) / KM2;

		BOOST_CHECK_CLOSE(d_ll, r_ll, 0.01);
		BOOST_CHECK_CLOSE(d_ll, r_xy, 0.01);
	}
}


template <typename P>
void test_all()
{
	test_area_box<P>();
	test_area_circle<P, double>();
	test_area_polygon<P>();
}

template <typename T>
void test_latlong()
{
	test_area_polygon_ll<point_xy<T>, point_ll<T, cs::geographic<degree> > >(false, false);
	test_area_polygon_ll<point_xy<T>, point_ll<T, cs::geographic<degree> > >(true, false);

	// with holes
	test_area_polygon_ll<point_xy<T>, point_ll<T, cs::geographic<degree> > >(false, true);

}


int test_main(int, char* [])
{
	//test_all<int[2]>();
	//test_all<float[2]>(); not yet because cannot be copied, for polygon
	//test_all<double[2]>();
	// test_all<test_point >(); does not apply because is defined 3D

	test_all<point_xy<int> >();
	test_all<point_xy<float> >();
	test_all<point_xy<double> >();

	test_latlong<double>();
	test_latlong<float>();

	return 0;
}


