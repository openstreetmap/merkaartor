// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_MATH_HPP
#define _GEOMETRY_MATH_HPP

#include <limits>
#include <cmath>
#include <geometry/util/promotion_traits.hpp>

namespace geometry
{
	// Maybe replace this by boost equals or boost ublas numeric equals or so

	/*!
		\brief returns true if both arguments are equal.

		equals returns true if both arguments are equal.
		\param a first argument
		\param b second argument
		\return true if a == b
		\note If both a and b are of an integral type, comparison is done by ==. If one of the types
		is floating point, comparison is done by abs and comparing with epsilon.
	*/
	template <typename T1, typename T2>
	inline bool equals(const T1& a, const T2& b)
	{
		typedef typename select_type_traits<T1, T2>::type T;
		if (std::numeric_limits<T>::is_exact)
		{
			return a == b;
		}
		else
		{
			return std::abs(a - b) < std::numeric_limits<T>::epsilon();
		}
	}


	namespace math
	{
		// From boost/.../normal_distribution.hpp : "Can we have a boost::mathconst please?"
		static double const pi = 2.0 * acos(0.0);
		static const double two_pi = 2.0 * pi;
		static const double d2r = pi / 180.0;
		static const double r2d = 1.0 / d2r;

		/*!
			\brief Calculates the haversine of an angle
			\note See http://en.wikipedia.org/wiki/Haversine_formula
				haversin(alpha) = sin2(alpha/2)
		*/
		inline double hav(double theta)
		{
			double sn = sin(0.5 * theta);
			return sn * sn;
		}

		/*!
			\brief Short utility to return the square

			\param value Value to calculate the square from
			\return The squared value
		*/
		inline double sqr(double value) { return value * value; }

	}


	// To be moved somewhere.
	namespace constants
	{
		static const double average_earth_radius = 6372795.0;
	}


} // namespace geometry

#endif // _GEOMETRY_MATH_HPP
