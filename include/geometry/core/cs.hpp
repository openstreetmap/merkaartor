// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_CS_HPP
#define _GEOMETRY_CS_HPP

#include <boost/type_traits.hpp>

#include <geometry/core/tags.hpp>
#include <geometry/core/coordinate_system.hpp>

/*!
\defgroup cs coordinate systems
\brief Defines coordinate systems
\details Coordinate systems are essential for any point in the Generic Geometry Library. Many
	algorithms such as distance or transform use coordinate systems to select the strategy to use.
*/

namespace geometry
{

	/*!
		\brief Unit of plan angles: degrees
		\ingroup utility
	*/
	class degree {};


	/*!
		\brief Unit of plan angles: radians
		\ingroup utility
	*/
	class radian {};


	namespace cs
	{
		/*!
			\brief Cartesian coordinate system
			\details Defines the Cartesian or rectangular coordinate system where points are defined in 2 or 3 (or more)
			dimensions and usually (but not always) known as x,y,z
			\see http://en.wikipedia.org/wiki/Cartesian_coordinate_system
			\ingroup cs
		*/
		struct cartesian
		{};



		/*!
			\brief EPSG Cartesian coordinate system
			\details EPSG (European Petrol Survey Group) has a standard list of projections,
				each having a code
			\see
			\ingroup cs
			\tparam C the EPSG code
		*/
		template<size_t C>
		struct epsg
		{
			static const size_t epsg_code = C;
		};


		/*!
			\brief Geographic coordinate system, in degree or in radian
			\details Defines the geographic coordinate system where points are defined in two angles and usually
			known as lat,long or lo,la or phi,lambda
			\see http://en.wikipedia.org/wiki/Geographic_coordinate_system
			\ingroup cs
		*/
		template<typename D>
		struct geographic
		{
			typedef D units;
		};

		/*!
			\brief Earth Centered, Earth Fixed
			\details Defines a Cartesian coordinate system x,y,z with the center of the earth as its origin,
				going through the Greenwich
			\see http://en.wikipedia.org/wiki/ECEF
			\see http://en.wikipedia.org/wiki/Geodetic_system
			\note Also known as "Geocentric", but geocentric is also an astronomic coordinate system
			\ingroup cs
		*/
		struct ecef
		{
		};

		/*!
			\brief Spherical coordinate system, in degree or in radian
			\details Defines the spherical coordinate system where points are defined in two angles
				and an optional radius usually known as r, theta, phi
			\par Coordinates:
			- coordinate 0:
				0 <= phi < 2pi is the angle between the positive x-axis and the line from the origin to the P projected onto the xy-plane.
			- coordinate 1:
				0 <= theta <= pi is the angle between the positive z-axis and the line formed between the origin and P.
			- coordinate 2 (if specified):
				r >= 0 is the distance from the origin to a given point P.

			\see http://en.wikipedia.org/wiki/Spherical_coordinates
			\ingroup cs
		*/
		template<typename D>
		struct spherical
		{
			typedef D units;
		};

		/*!
			\brief Polar coordinate system
			\details Defines the polar coordinate system "in which each point on a plane is determined by an angle and a distance"
			\see http://en.wikipedia.org/wiki/Polar_coordinates
			\ingroup cs
		*/
		template<typename D>
		struct polar
		{
			typedef D units;
		};


		namespace celestial
		{
			/*!
				\brief Ecliptic (celestial) coordinate system
				\details Defines the astronomical ecliptic coordinate system "that uses the ecliptic for its fundamental plane"
				It uses Beta and Lambda as its latitude and longitude.
				\see http://en.wikipedia.org/wiki/Ecliptic_coordinate_system
				\ingroup cs
			*/
			template<typename D>
			struct ecliptic
			{};


			// More celestial coordinate systems could be defined
		}

	}

	namespace traits
	{
		/*!
			\brief Traits class defining coordinate system tag, bound to coordinate system
			\ingroup traits
			\tparam CS coordinate system
		*/
		template <typename CS>
		struct cs_tag
		{
		};


		#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS

		template<typename DR>
		struct cs_tag<cs::geographic<DR> >
		{
			typedef geographic_tag type;
		};

		template<typename DR>
		struct cs_tag<cs::spherical<DR> >
		{
			typedef spherical_tag type;
		};


		template<>
		struct cs_tag<cs::cartesian>
		{
			typedef cartesian_tag type;
		};

		template<>
		struct cs_tag<cs::ecef>
		{
			typedef cartesian_tag type;
		};

		template <size_t C>
		struct cs_tag<cs::epsg<C> >
		{
			typedef cartesian_tag type;
		};

		#endif

	}

	/*!
		\brief Meta-function returning coordinate system tag (cs family) of any geometry
		\ingroup core
	*/
	template <typename G>
	struct cs_tag
	{
		typedef typename traits::cs_tag<typename geometry::coordinate_system<G>::type>::type type;
	};


	/*!
		\brief Meta-function to verify if a coordinate system is radian
		\ingroup core
	*/
	template <typename CS>
	struct is_radian : boost::true_type {};


	#ifndef DOXYGEN_NO_SPECIALIZATIONS

	// Specialization for any degree coordinate systems
	template <template<typename> class CS>
	struct is_radian< CS<degree> > : boost::false_type {};

	#endif



} // namespace geometry


#endif //_GEOMETRY_CS_HPP
