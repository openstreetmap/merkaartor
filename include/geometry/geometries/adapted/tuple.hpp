// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_ADAPTED_TUPLE_HPP
#define _GEOMETRY_ADAPTED_TUPLE_HPP


#include <boost/tuple/tuple.hpp>

#include <geometry/core/tags.hpp>

#include <geometry/core/coordinate_type.hpp>
#include <geometry/core/coordinate_dimension.hpp>
#include <geometry/core/point_type.hpp>



namespace geometry
{
	#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
	namespace traits
	{
		template <typename T>
		struct coordinate_type<boost::tuple<T, T> >
		{ typedef T type; };

		template <typename T>
		struct dimension<boost::tuple<T, T> > : boost::mpl::int_<2> {};

		template <typename T>
		struct access<boost::tuple<T, T> >
		{
			template <int I>
			static inline T get(const boost::tuple<T, T>& p) { return p.get<I>(); }

			template <int I>
			static inline void set(boost::tuple<T, T>& p, const T& value) { p.get<I>() = value; }
		};


		template <typename T>
		struct tag<boost::tuple<T, T>  >
		{ typedef point_tag type; };


		// The library user has
		// 1) either to specify the coordinate system
		// 2) or include <geometry/geometries/adapted/tuple_@.hpp> where @=cartesian,geographic,...
	}
	#endif
}


#endif
