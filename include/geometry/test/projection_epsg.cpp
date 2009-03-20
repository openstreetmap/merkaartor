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
#include <geometry/projections/proj/tmerc.hpp>
#include <geometry/projections/proj/sterea.hpp>

#include <geometry/geometries/geometries.hpp>


template <int EPSG, typename LL, typename P>
void test_one(double lon, double lat,
			  typename geometry::coordinate_type<P>::type x,
			  typename geometry::coordinate_type<P>::type y)
{
	typedef typename geometry::coordinate_type<P>::type T;

	LL ll;
	ll.lon(lon);
	ll.lat(lat);

	typedef projection::epsg_traits<EPSG, LL, P> E;
	projection::parameters par = projection::impl::pj_init_plus(E::par());
	typedef typename E::type PRJ;
	PRJ prj(par);

	P xy;
	prj.forward(ll, xy);

	BOOST_CHECK_CLOSE(geometry::get<0>(xy), x, 0.001);
	BOOST_CHECK_CLOSE(geometry::get<1>(xy), y, 0.001);
}


template <typename DR, typename P>
void test_deg_rad(double factor)
{
	typedef typename geometry::coordinate_type<P>::type T;
	typedef geometry::point_ll<T, geometry::cs::geographic<DR> > LL;

	test_one<28992, LL, P>(4.897000 * factor, 52.371000 * factor, 121590.388077, 487013.903377);
	test_one<29118, LL, P>(4.897000 * factor, 52.371000 * factor, 4852882, 9129373);
}

template <typename P>
void test_all()
{
	test_deg_rad<geometry::degree, P>(1.0);
	test_deg_rad<geometry::radian, P>(geometry::math::d2r);
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


