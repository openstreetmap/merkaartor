// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>

#include <geometry/algorithms/parse.hpp>
#include <geometry/geometries/point.hpp>

#include <geometry/strategies/geographic/geo_parse.hpp>


#include "common.hpp"

using namespace geometry;



template <typename P>
void test_2d(double c, bool use_strategy)
{
	// normal order, east=x, north=y
	P p;
	parse(p, std::string("1dE"), std::string("2N"));
	BOOST_CHECK_CLOSE( ((double) get<0>(p)), (double) 1 * c, 1.0e-6);
	BOOST_CHECK_CLOSE( ((double) get<1>(p)), (double) 2 * c, 1.0e-6);

	// reversed order, y,x -> should be interpreted correctly
	parse(p, std::string("1dN"), std::string("2E"));
	BOOST_CHECK_CLOSE( ((double) get<0>(p)), (double) 2 * c, 1.0e-6);
	BOOST_CHECK_CLOSE( ((double) get<1>(p)), (double) 1 * c, 1.0e-6);

	if (use_strategy)
	{
		// DUTCH system NOZW, only for degrees
		geometry::strategy::dms_parser<false, 'N', 'O', 'Z', 'W'> strategy;
		parse(p, std::string("1dO"), std::string("2Z"), strategy);
		BOOST_CHECK_CLOSE( ((double) get<0>(p)), (double) 1, 1.0e-6);
		BOOST_CHECK_CLOSE( ((double) get<1>(p)), (double) -2, 1.0e-6);
	}

	// rest of DMS is checked in parse_dms
}


template <typename T, typename P>
void test_3d()
{
}



int test_main(int, char* [])
{
	//test_2d<point<int, 2, cs::geographic<radian> > >();
	//test_2d<point<float, 2, cs::geographic<radian> > >();

	test_2d<point<double, 2, cs::geographic<degree> > >(1.0, true);
	test_2d<point<double, 2, cs::geographic<radian> > >(geometry::math::d2r, false);

	return 0;
}
