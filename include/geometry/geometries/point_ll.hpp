// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_POINT_LL_HPP
#define _GEOMETRY_POINT_LL_HPP

#include <string>
#include <sstream>

#include <boost/numeric/conversion/cast.hpp>

#include <geometry/core/cs.hpp>

#include <geometry/geometries/point.hpp>
#include <geometry/arithmetic/arithmetic.hpp>

#include <geometry/util/graticule.hpp>
#include <geometry/util/copy.hpp>

namespace geometry
{

	/*!
		\brief Point using spherical coordinates \a lat and \a lon, on Earth
		\ingroup Geometry
		\details The point_ll class implements a point with lat and lon functions. It can be constructed
		using latitude and longitude classes. The latlong class can be defined in degrees or in radians.
		There is a conversion method from degree to radian, and from radian to degree.
		\tparam D degree/radian enumeration
		\tparam T coordinate type, double (the default) or float (it might be int as well)
		\note There is NO constructor with two values to avoid exchanging lat and long
		\note Construction with latitude and longitude can be done in both orders, so lat/long and long/lat
		\par Example:
		Example showing how the point_ll class can be constructed. Note that it can also be constructed using
		decimal degrees (43.123).
		\dontinclude doxygen_examples.cpp
		\skip example_point_ll_construct
		\line {
		\until }
	*/
	template <typename T = double, typename C = cs::geographic<degree> >
	class point_ll : public point<T, 2, C>
	{
		public :

			/// Default constructor, does not initialize anything
			inline point_ll() : point<T, 2, C>() {}

			/// Constructor with longitude/latitude
			inline point_ll(const longitude<T>& lo, const latitude<T>& la) : point<T, 2, C>(lo, la) {}

			/// Constructor with latitude/longitude
			inline point_ll(const latitude<T>& la, const longitude<T>& lo) : point<T, 2, C>(lo, la) {}

			/// Get longitude
			inline const T& lon() const { return this->template get<0>(); }
			/// Get latitude
			inline const T& lat() const { return this->template get<1>(); }

			/// Set longitude
			inline void lon(const T& v) { this->template set<0>(v); }
			/// Set latitude
			inline void lat(const T& v) { this->template set<1>(v); }

			/// Get longitude
			/// Set
			inline void lon(const dms<east, T>& v) { this->template set<0>(v.as_value()); }
			inline void lon(const dms<west, T>& v) { this->template set<0>(v.as_value()); }

			inline void lat(const dms<north, T>& v) { this->template set<1>(v.as_value()); }
			inline void lat(const dms<south, T>& v) { this->template set<1>(v.as_value()); }
	};


	// Adapt the point_ll to the concept
	#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
	namespace traits
	{
		template <typename T, typename C>
		struct tag<point_ll<T, C> > { typedef point_tag type; };

		template<typename T, typename C>
		struct coordinate_type<point_ll<T, C> > { typedef T type; };

		template<typename T, typename C>
		struct coordinate_system<point_ll<T, C> > { typedef C type; };

		template<typename T, typename C>
		struct dimension<point_ll<T, C> >: boost::mpl::int_<2> {};

		template<typename T, typename C>
		struct access<point_ll<T, C> >
		{
			template <size_t I>
			static inline T get(const point_ll<T, C>& p)
			{ return p.template get<I>(); }

			template <size_t I>
			static inline void set(point_ll<T, C>& p, const T& value)
			{ p.template set<I>(value); }
		};

	}
	#endif

} // namespace geometry


#endif // _GEOMETRY_POINT_LL_HPP
