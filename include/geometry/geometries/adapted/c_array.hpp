// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_ADAPTED_C_ARRAY_HPP
#define _GEOMETRY_ADAPTED_C_ARRAY_HPP

#include <boost/type_traits/is_arithmetic.hpp>

#include <geometry/core/cs.hpp>
#include <geometry/core/tags.hpp>
#include <geometry/core/coordinate_type.hpp>
#include <geometry/core/coordinate_dimension.hpp>
#include <geometry/core/access.hpp>




namespace geometry
{

	#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
	namespace traits
	{

		#ifndef DOXYGEN_NO_IMPL
		namespace impl
		{
			// Create class and specialization to indicate the tag
			// for normal cases and the case that the type of the c-array is arithmetic
			template <bool> struct c_array_tag { typedef geometry_not_recognized_tag type; };
			template <> struct c_array_tag<true> { typedef point_tag type; };
		}
		#endif

		// Assign the point-tag, preventing arrays of points getting a point-tag
		template <typename T, size_t N>
		struct tag<T[N]> : impl::c_array_tag<boost::is_arithmetic<T>::value> {};

		template <typename T, size_t N>
		struct coordinate_type<T[N]> { typedef T type; };

		template <typename T, size_t N>
		struct dimension<T[N]>: boost::mpl::int_<N> {};

		template <typename T, size_t N>
		struct access<T[N]>
		{
			template <size_t I>
			static inline T get(const T p[N]) { return p[I]; }

			template <size_t I>
			static inline void set(T p[N], const T& value) { p[I] = value; }
		};


		// The library user has
		// 1) either to specify the coordinate system
		// 2) or include <geometry/geometries/adapted/c_array_@.hpp> where @=cartesian,geographic,...
	}
	#endif
}


#endif
