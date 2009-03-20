// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>

#include <geometry/core/concepts/segment_concept.hpp>
#include <geometry/geometries/segment.hpp>

#include "common.hpp"


using namespace geometry;




template <typename P>
void test_all()
{
	typedef segment<P> S;

	P p1;
	P p2;
	S s(p1, p2);
	BOOST_CHECK_EQUAL(&s.first, &p1);
	BOOST_CHECK_EQUAL(&s.second, &p2);

	// Compilation tests, all things should compile.
	BOOST_CONCEPT_ASSERT((ConstSegment<S>));
	BOOST_CONCEPT_ASSERT((Segment<S>));

	typedef typename coordinate_type<S>::type T;
	typedef typename point_type<S>::type SP;


std::cout << sizeof(typename coordinate_type<S>::type) << std::endl;

	typedef segment<const P> CS;
	//BOOST_CONCEPT_ASSERT((ConstSegment<CS>));

	CS cs(p1, p2);

	typedef typename coordinate_type<CS>::type CT;
	typedef typename point_type<CS>::type CSP;


}


int test_main(int, char* [])
{
	test_all<int[3]>();
	test_all<float[3]>();
	test_all<double[3]>();
	//test_all<test_point>();
	test_all<point<int, 3, cs::cartesian> >();
	test_all<point<float, 3, cs::cartesian> >();
	test_all<point<double, 3, cs::cartesian> >();

	return 0;
}
