// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>

#include <geometry/geometries/nsphere.hpp>
#include <geometry/core/concepts/nsphere_concept.hpp>

#include "common.hpp"

using namespace geometry;


// define a custom circle
struct custom_circle { float x,y; int r; };

// adapt custom circle using traits
namespace geometry
{
	namespace traits
	{
		template<> struct tag<custom_circle> { typedef nsphere_tag type; };
		template<> struct point_type<custom_circle> { typedef point<double, 2, cs::cartesian> type; };
		template<> struct radius_type<custom_circle> { typedef int type; };

		template<> struct access<custom_circle>
		{
			template <int I>
			static inline const float& get(const custom_circle& c) { return I == 0 ? c.x : c.y; }

			template <int I>
			static inline void set(custom_circle& c, const float& value)
			{ switch (I) { case 0 : c.x = value; break; case 1 : c.y = value; break; } }
		};

		template<> struct radius_access<custom_circle, int, 0>
		{
			static inline int get(const custom_circle& c) { return c.r; }
			static inline void set(custom_circle& c, const int& radius) { c.r = radius; }
		};
}
};



template <typename S, typename RT, typename CT>
void check_nsphere(S& to_check,
                  RT radius, CT center_x, CT center_y, CT center_z)
{
	BOOST_CONCEPT_ASSERT((ConstNsphere<S>));
	BOOST_CONCEPT_ASSERT((Nsphere<S>));


	BOOST_CHECK_EQUAL(get_radius<0>(to_check), radius);

	BOOST_CHECK_EQUAL(get<0>(to_check), center_x);
	BOOST_CHECK_EQUAL(get<1>(to_check), center_y);
	if (dimension<S>::value >= 3)
	{
		BOOST_CHECK_EQUAL(get<2>(to_check), center_z);
	}
}


template <typename P, typename T>
void test_construction()
{
	typedef typename coordinate_type<P>::type ctype;

	nsphere<P, T> c1;
	check_nsphere(c1, 0, 0,0,0);

	P center;
	geometry::assign(center, 1, 2, 3);
	nsphere<P, T> c2(center, 4);
	check_nsphere(c2, 4, 1,2,3);
}

template <typename C>
void test_assignment()
{
	C c;

	// by hand
	set<0>(c, 5);
	set<1>(c, 50);
	set<2>(c, 500);

	set_radius<0>(c, 5000);
	check_nsphere(c, 5000, 5,50,500);

	// using assign
	if (dimension<C>::value >= 3)
	{
		assign(c, 6, 60, 600);
		check_nsphere(c, 5000, 6,60,600);
	}
}


template <typename P, typename T>
void test_all()
{
	test_construction<P, T>();
	test_assignment<nsphere<P, T> >();
	test_assignment<custom_circle>();
}


template <typename P>
void test_all()
{
	test_all<P, int>();
	test_all<P, float>();
	test_all<P, double>();
}


int test_main(int, char* [])
{
	//test_all<int[3]>();
	//test_all<float[3]>();
	//test_all<double[3]>();
	//test_all<test_point>();
	test_all<point<int, 3, cs::cartesian> >();
	test_all<point<float, 3, cs::cartesian> >();
	test_all<point<double, 3, cs::cartesian> >();

	return 0;
}
