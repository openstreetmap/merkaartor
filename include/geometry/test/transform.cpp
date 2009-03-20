// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <geometry/algorithms/transform.hpp>
#include <geometry/algorithms/make.hpp>
#include <geometry/geometries/geometries.hpp>
#include <geometry/io/wkt/streamwkt.hpp>

#include "common.hpp"



template <typename P1, typename P2>
void test_transform_point(double value)
{
	P1 p1;
	geometry::set<0>(p1, 1);
	geometry::set<1>(p1, 2);
	P2 p2;
	geometry::transform(p1, p2);
	BOOST_CHECK_CLOSE((double) value * geometry::get<0>(p1), (double) geometry::get<0>(p2), 0.001);
	BOOST_CHECK_CLOSE((double) value * geometry::get<1>(p1), (double) geometry::get<1>(p2), 0.001);
}

template <typename P1, typename P2>
void test_transform_linestring()
{
	geometry::linestring<P1> line1;
	line1.push_back(geometry::make<P1>(1, 1));
	line1.push_back(geometry::make<P1>(2, 2));
	geometry::linestring<P2> line2;
	geometry::transform(line1, line2);
	BOOST_CHECK_EQUAL(line1.size(), line2.size());

	std::ostringstream out1, out2;
	out1 << line1;
	out2 << line2;
	BOOST_CHECK_EQUAL(out1.str(), out1.str());
}


template <typename P1, typename P2>
void test_all(double value = 1.0)
{
	test_transform_point<P1, P2>(value);
	test_transform_linestring<P1, P2>();
}

template <typename T, typename DR>
void test_transformations(double phi, double theta, double r)
{
	using namespace geometry;

	typedef point<T, 3, cs::cartesian> XYZ;
	XYZ p;

	// 1: using spherical coordinates
	{
		typedef point<T, 3,	cs::spherical<DR> >  SPH;
		SPH sph1, sph2;
		assign(sph1, phi, theta, r);
		transform(sph1, p);
		transform(p, sph2);

		BOOST_CHECK_CLOSE((double) geometry::get<0>(sph1), (double) geometry::get<0>(sph2), 0.001);
		BOOST_CHECK_CLOSE((double) geometry::get<1>(sph1), (double) geometry::get<1>(sph2), 0.001);

		//std::cout << make_wkt(p) << std::endl;
		//std::cout << make_wkt(sph2) << std::endl;
	}

	// 2: using spherical coordinates on unit sphere
	{
		typedef point<T, 2,	cs::spherical<DR> >  SPH;
		SPH sph1, sph2;
		assign(sph1, phi, theta);
		transform(sph1, p);
		transform(p, sph2);

		BOOST_CHECK_CLOSE((double) geometry::get<0>(sph1), (double) geometry::get<0>(sph2), 0.001);
		BOOST_CHECK_CLOSE((double) geometry::get<1>(sph1), (double) geometry::get<1>(sph2), 0.001);

		std::cout << make_wkt(sph1) << " " << make_wkt(p) << " " << make_wkt(sph2) << std::endl;
	}
}

int test_main(int, char* [])
{
	using namespace geometry;
	using namespace geometry::cs;
	using namespace geometry::math;
	//test_all<int[2]>();
	//test_all<float[2]>(); not yet because cannot be copied, for polygon
	//test_all<double[2]>();
	//test_all<test_point, test_point>();

	typedef geometry::point_xy<double > P;
	test_all<P, P>();
	test_all<geometry::point_xy<int>, geometry::point_xy<float> >();

	test_all<geometry::point_ll<double, geographic<degree> >,
		geometry::point_ll<double, geographic<radian> > >(d2r);
	test_all<geometry::point_ll<double, geographic<radian> >,
		geometry::point_ll<double, geographic<degree> > >(r2d);

	test_all<geometry::point_ll<int, geographic<degree> >,
		geometry::point_ll<float, geographic<radian> > >(d2r);

	test_transformations<float, degree>(4, 52, 1);
	test_transformations<double, degree>(4, 52, 1);

	test_transformations<float, radian>(3 * d2r, 51 * d2r, 1);
	test_transformations<double, radian>(3 * d2r, 51 * d2r, 1);


	return 0;
}


