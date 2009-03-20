// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_ADAPTED_C_ARRAY_GEOGRAPHIC_HPP
#define _GEOMETRY_ADAPTED_C_ARRAY_GEOGRAPHIC_HPP

#ifdef _GEOMETRY_ADAPTED_C_ARRAY_COORDINATE_SYSTEM_DEFINED
#error Include only one headerfile to register coordinate coordinate_system for adapted c array
#endif

#define _GEOMETRY_ADAPTED_C_ARRAY_COORDINATE_SYSTEM_DEFINED

#include <geometry/geometries/adapted/c_array.hpp>

namespace geometry
{
	#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
	namespace traits
	{
		template <typename T, int N>
		struct coordinate_system<T[N]>
		{ typedef cs::geographic type; };

	}
	#endif
}


#endif
