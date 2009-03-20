// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_BOX_HPP
#define _GEOMETRY_BOX_HPP

#include <geometry/core/concepts/point_concept.hpp>


#include <geometry/algorithms/assign.hpp>
#include <geometry/util/copy.hpp>

namespace geometry
{

	/*!
		\brief Class box: defines a box made of two describing points
		\ingroup Geometry
		\details Box is always described by a min_corner() and a max_corner() point. If another
		rectangle is used, use linear_ring or polygon.
		\note Boxes are for selections and for calculating the envelope of geometries. Not all algorithms
		are implemented for box. Boxes are also used in Spatial Indexes.
		\tparam P point type. The box takes a point type as template parameter.
		The point type can be any point type.
		It can be 2D but can also be 3D or more dimensional.
		The box can also take a latlong point type as template parameter.
	 */

	template<typename P>
	class box
	{
		BOOST_CONCEPT_ASSERT((Point<P>));

		public :
			inline box()
			{}

			/*!
				\brief Constructor taking the minimum corner point and the maximum corner point
			*/
			inline box(const P& min_corner, const P& max_corner)
			{
				copy_coordinates(min_corner, m_min_corner);
				copy_coordinates(max_corner, m_max_corner);
			}


			inline const P& min_corner() const { return m_min_corner; }
			inline const P& max_corner() const { return m_max_corner; }

			inline P& min_corner() { return m_min_corner; }
			inline P& max_corner() { return m_max_corner; }

		private :
			P m_min_corner, m_max_corner;
	};



	// Traits specializations for box above
	#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
	namespace traits
	{

		template <typename P>
		struct tag< box<P> > { typedef box_tag type; };

		template <typename P>
		struct point_type<box<P> > { typedef P type; };


		template <typename P, size_t C, size_t D>
		struct indexed_access<box<P>, C, D>
		{
			typedef box<P> B;

			static inline typename geometry::coordinate_type<B>::type get(const B& b)
			{
				return C == min_corner ? geometry::get<D>(b.min_corner()) : geometry::get<D>(b.max_corner());
			}

			static inline void set(B& b, const typename geometry::coordinate_type<B>::type& value)
			{
				if (C == min_corner)
				{
					geometry::set<D>(b.min_corner(), value);
				}
				else
				{
					geometry::set<D>(b.max_corner(), value);
				}
			}
		};

	}
	#endif


};

#endif // _GEOMETRY_BOX_HPP
