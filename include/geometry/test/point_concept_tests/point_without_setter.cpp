// Generic Geometry Library Point concept test file
//
// Copyright Bruno Lalande 2008
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/tuple/tuple.hpp>
#include "function_requiring_a_point.hpp"


struct point: private boost::tuple<float, float>
{
	typedef float coordinate_type;
	enum { coordinate_count = 2 };

	/*
	template <int I>
	float& get()
	{ return boost::tuple<float, float>::get<I>(); }
	*/

	template <int I>
	const float& get() const
	{ return boost::tuple<float, float>::get<I>(); }
};


int main()
{
	point p1;
	const point p2;
	geometry::function_requiring_a_point(p1, p2);
}
