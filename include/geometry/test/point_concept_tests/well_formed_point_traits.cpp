// Generic Geometry Library Point concept test file
//
// Copyright Bruno Lalande 2008
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include "function_requiring_a_point.hpp"
#include <geometry/core/cs.hpp>

struct point
{
	point(): x(), y() {}
	float x, y;
};


template <int I> struct accessor;

template <>
struct accessor<0>
{
	static float get(const point& p) { return p.x; }
	static void set(point& p, float value) { p.x = value; }
};

template <>
struct accessor<1>
{
	static float get(const point& p) { return p.y; }
	static void set(point& p, float value) { p.y = value; }
};


namespace geometry
{
	namespace traits
	{
		template <>
		struct tag<point>
		{ typedef point_tag type; };

		template <>
		struct coordinate_type<point>
		{ typedef float type; };

		template <>
		struct coordinate_system<point>
		{ typedef geometry::cs::cartesian type; };

		template <>
		struct dimension<point>
		{ enum { value = 2 }; };

		template <>
		struct access<point>
		{
			template <int I>
			static float get(const point& p)
			{ return accessor<I>::get(p); }

			template <int I>
			static void set(point& p, float value)
			{ accessor<I>::set(p, value); }
		};
	}
}


int main()
{
	point p1;
	const point p2;
	geometry::function_requiring_a_point(p1, p2);
}
