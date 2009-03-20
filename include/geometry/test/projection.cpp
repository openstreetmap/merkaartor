// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>
#include <boost/test/floating_point_comparison.hpp>


#include "common.hpp"

#include <geometry/geometries/adapted/c_array.hpp>
#include <geometry/algorithms/transform.hpp>

#include <geometry/projections/parameters.hpp>
#include <geometry/projections/proj/aea.hpp>

#include <geometry/geometries/geometries.hpp>


template <typename PRJ, typename LL, typename P>
void test_one(double lon, double lat,
			  typename geometry::coordinate_type<P>::type x,
			  typename geometry::coordinate_type<P>::type y,
			  const std::string& parameters)
{
	typedef typename geometry::coordinate_type<P>::type T;

	LL ll;
	ll.lon(lon);
	ll.lat(lat);

	projection::parameters par = projection::impl::pj_init_plus(parameters);
	PRJ prj(par);

	P xy;
	prj.forward(ll, xy);

	BOOST_CHECK_CLOSE(geometry::get<0>(xy), x, 0.001);
	BOOST_CHECK_CLOSE(geometry::get<1>(xy), y, 0.001);
}



template <typename P>
void test_all()
{
	typedef typename geometry::coordinate_type<P>::type T;
	typedef geometry::point_ll<T, geometry::cs::geographic<geometry::degree> > LL;

	/* aea */ test_one<projection::aea_ellipsoid<LL, P>, LL, P>(
					4.897000, 52.371000, 334609.583974, 5218502.503686,
					"+proj=aea +ellps=WGS84 +units=m +lat_1=55 +lat_2=65");
}


int test_main(int, char* [])
{
	//test_all<int[2]>();
	test_all<float[2]>();
	test_all<double[2]>();
	test_all<test_point>();
	//test_all<geometry::point_xy<int> >();
	test_all<geometry::point_xy<float> >();
	test_all<geometry::point_xy<double> >();
	test_all<geometry::point_xy<long double> >();

	return 0;
}


