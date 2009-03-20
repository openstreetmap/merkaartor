// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <vector>
#include <boost/array.hpp>

#include <boost/test/included/test_exec_monitor.hpp>

#include <geometry/core/coordinate_type.hpp>

#include <geometry/util/loop.hpp>
#include <geometry/algorithms/assign.hpp>



#include "common.hpp"



using namespace geometry;


struct test_operation_1
{
	template <typename Segment, typename State>
	bool operator()(Segment&, State& state) const
	{
		state +=1;
		return true;
	}
};

struct test_operation_2
{
	template <typename Segment, typename State>
	bool operator()(Segment& segment, State& state) const
	{
		state += get<0>(segment.first)
		       + get<1>(segment.first)
		       + get<2>(segment.first)
		       + get<0>(segment.second)
		       + get<1>(segment.second)
		       + get<2>(segment.second);

		static int count = 0;
		if (++count == 3)
		{
			count = 0;
			return false;
		}

		return true;
	}
};


template <typename P>
void test_all()
{
	boost::array<P, 5> a;
	geometry::assign(a[0], 1, 2, 3);
	geometry::assign(a[1], 4, 5, 6);
	geometry::assign(a[2], 7, 8, 9);
	geometry::assign(a[3], 10, 11, 12);
	geometry::assign(a[4], 13, 14, 15);
	typename coordinate_type<P>::type state;

	state = 0;
	BOOST_CHECK_EQUAL(loop(a, test_operation_1(), state), true);
	BOOST_CHECK_EQUAL(state, 4);

	state = 0;
	BOOST_CHECK_EQUAL(loop(a, test_operation_2(), state), false);
	BOOST_CHECK_EQUAL(state, 1+2+3+4+5+6 + 4+5+6+7+8+9 + 7+8+9+10+11+12);
}


int test_main(int, char* [])
{
	test_all<int[3]>();
	test_all<float[3]>();
	test_all<double[3]>();
	test_all<test_point>();
	test_all<point<int, 3, geometry::cs::cartesian> >();
	test_all<point<float, 3, geometry::cs::cartesian> >();
	test_all<point<double, 3, geometry::cs::cartesian> >();

	return 0;
}
