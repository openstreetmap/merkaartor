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

#include <geometry/strategies/transform/matrix_transformers.hpp>
#include <geometry/strategies/transform/inverse_transformer.hpp>
#include <geometry/strategies/transform/map_transformer.hpp>


#include "common.hpp"


template <typename P, typename TRANS>
void check_inverse(const P& p, const TRANS& trans)
{
	geometry::strategy::transform::inverse_transformer<P, P> inverse(trans);

	P i;
	geometry::transform(p, i, inverse);

	BOOST_CHECK_CLOSE(double(geometry::get<0>(i)), 1.0, 0.001);
	BOOST_CHECK_CLOSE(double(geometry::get<1>(i)), 1.0, 0.001);
}


template <typename P>
void test_all()
{
	P p;
	geometry::assign(p, 1, 1);

	{
		geometry::strategy::transform::translate_transformer<P, P> trans(1, 1);
		P tp;
		geometry::transform(p, tp, trans);

		BOOST_CHECK_CLOSE(double(geometry::get<0>(tp)), 2.0, 0.001);
		BOOST_CHECK_CLOSE(double(geometry::get<1>(tp)), 2.0, 0.001);

		check_inverse(tp, trans);
	}

	{
		geometry::strategy::transform::scale_transformer<P, P> trans(10, 10);
		P tp;
		geometry::transform(p, tp, trans);

		BOOST_CHECK_CLOSE(double(geometry::get<0>(tp)), 10.0, 0.001);
		BOOST_CHECK_CLOSE(double(geometry::get<1>(tp)), 10.0, 0.001);

		check_inverse(tp, trans);
	}

	{
		geometry::strategy::transform::rotate_transformer<P, P, geometry::degree> trans(90.0);
		P tp;
		geometry::transform(p, tp, trans);

		BOOST_CHECK_CLOSE(double(geometry::get<0>(tp)), 1.0, 0.001);
		BOOST_CHECK_CLOSE(double(geometry::get<1>(tp)), -1.0, 0.001);
		check_inverse(tp, trans);
	}


	{
		// Map from 0,0,2,2 to 0,0,500,500
		geometry::strategy::transform::map_transformer<P, P, false> trans(0.0, 0.0, 2.0, 2.0, 500, 500);
		P tp;
		geometry::transform(p, tp, trans);

		BOOST_CHECK_CLOSE(double(geometry::get<0>(tp)), 250.0, 0.001);
		BOOST_CHECK_CLOSE(double(geometry::get<1>(tp)), 250.0, 0.001);

		check_inverse(tp, trans);
	}


}

int test_main(int, char* [])
{
	using namespace geometry;

	//test_all<int[2]>();
	test_all<float[2]>();
	test_all<double[2]>();

	test_all<boost::tuple<float, float> >();

	//test_all<point<int, 2, cs::cartesian> >();
	test_all<point<float, 2, cs::cartesian> >();
	test_all<point<double, 2, cs::cartesian> >();



	return 0;
}


