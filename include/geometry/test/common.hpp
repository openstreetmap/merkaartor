// Generic Geometry Library test file
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_TEST_COMMON_HPP
#define _GEOMETRY_TEST_COMMON_HPP


#include <geometry/geometries/point.hpp>
#include <geometry/geometries/adapted/c_array_cartesian.hpp>
#include <geometry/geometries/adapted/tuple_cartesian.hpp>


// Test point class

struct test_point
{
	float c1, c2, c3;
};


// Struct set of metafunctions to read/write coordinates above, specialized per dimension
template <int I> struct accessor {};

template<> struct accessor<0>
{
	static inline const float& get(const test_point& p) { return p.c1; }
	static inline void set(test_point& p, const float& value) { p.c1 = value; }
};

template<> struct accessor<1>
{
	static inline const float& get(const test_point& p) { return p.c2; }
	static inline void set(test_point& p, const float& value) { p.c2 = value; }
};

template <> struct accessor<2>
{
	static inline const float& get(const test_point& p) { return p.c3; }
	static inline void set(test_point& p, const float& value) { p.c3 = value; }
};


namespace geometry
{
	namespace traits
	{
		template<> struct tag<test_point> { typedef point_tag type; };
		template<> struct coordinate_type<test_point> { typedef float type; };
		template<> struct coordinate_system<test_point> { typedef cs::cartesian type; };
		template<> struct dimension<test_point>: boost::mpl::int_<3> {};

		template<> struct access<test_point>
		{
			template <int I>
			static inline const float& get(const test_point& p) { return accessor<I>::get(p); }

			template <int I>
			static inline void set(test_point& p, const float& value) { accessor<I>::set(p, value); }
		};
	}
};



#endif
