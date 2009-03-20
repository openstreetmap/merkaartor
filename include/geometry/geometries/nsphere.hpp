// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_NSPHERE_HPP
#define _GEOMETRY_NSPHERE_HPP

#include <geometry/core/concepts/point_concept.hpp>
#include <geometry/core/concepts/nsphere_concept.hpp>

#include <geometry/algorithms/assign.hpp>
#include <geometry/util/copy.hpp>

namespace geometry
{
	/*!
		\brief Class nsphere: defines a circle or a sphere: a point with radius
		\ingroup Geometry
		\details The name nsphere is quite funny but the best description of the class. It can be a circle (2D),
		a sphere (3D), or higher (hypersphere) or lower. According to Wikipedia this name is the most appropriate.
		It was mentioned on the Boost list.
		An alternative is the more fancy name "sphercle" but that might be a bit too much an invention.
		\note Circle is currently used for selections, for example polygon_in_circle. Currently not all
		algorithms are implemented for n-spheres.
		\tparam P point type of the center
		\tparam T number type of the radius
	 */
	template <typename P, typename T>
	class nsphere
	{
		BOOST_CONCEPT_ASSERT((Point<P>));

		public :
			typedef T radius_type;
			typedef typename coordinate_type<P>::type coordinate_type;

			nsphere()
				: m_radius(0)
			{
				impl::assign::assign_value(m_center, coordinate_type());
			}

			nsphere(const P& center, const T& radius)
				: m_radius(radius)
			{
				copy_coordinates(center, m_center);
			}

			inline const P& center() const { return m_center; }
			inline const T& radius() const { return m_radius; }

			inline void radius(const T& r) { m_radius = r; }
			inline P& center() { return m_center; }

		private :
			P m_center;
			T m_radius;
	};



	// Traits specializations for n-sphere above
	#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
	namespace traits
	{

		template <typename P, typename T>
		struct tag< nsphere<P, T> > { typedef nsphere_tag type; };

		template <typename P, typename T>
		struct point_type<nsphere<P, T> > { typedef P type; };

		template <typename P, typename T>
		struct radius_type<nsphere<P, T> > { typedef T type; };

		template <typename P, typename T>
		struct access<nsphere<P, T> >
		{
			typedef nsphere<P, T> S;

			template <size_t D>
			static inline typename geometry::coordinate_type<S>::type get(const S& s)
			{
				return geometry::get<D>(s.center());
			}

			template <size_t D>
			static inline void set(S& s, const typename geometry::coordinate_type<S>::type& value)
			{
				geometry::set<D>(s.center(), value);
			}
		};


		template <typename P, typename T>
		struct radius_access<nsphere<P, T>, T, 0>
		{
			typedef nsphere<P, T> S;

			static inline T get(const S& s)
			{
				return s.radius();
			}

			static inline void set(S& s, const T& value)
			{
				s.radius(value);
			}
		};


	}
	#endif


}

#endif // _GEOMETRY_NSPHERE_HPP
