// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_POLYGON_HPP
#define _GEOMETRY_POLYGON_HPP

#include <vector>

#include <boost/concept/assert.hpp>

#include <geometry/core/point_type.hpp>
#include <geometry/core/ring_type.hpp>
#include <geometry/core/exterior_ring.hpp>
#include <geometry/core/interior_rings.hpp>


#include <geometry/core/concepts/point_concept.hpp>

#include <geometry/geometries/linear_ring.hpp>


namespace geometry
{

	/*!
		\brief The \b polygon contains an outer ring and zero or more inner rings.
		\ingroup Geometry
		\tparam P point type
		\tparam VR optional container type for inner rings, for example std::vector, std::list, std::deque
		\tparam VP optional container type for points, for example std::vector, std::list, std::deque
		\tparam AR container-allocator-type
		\tparam AP container-allocator-type
		\note The container collecting the points in the rings can be different from the
		container collecting the inner rings. They all default to vector.
	*/
	template<typename P,
			template<typename,typename> class VP = std::vector,
			template<typename,typename> class VR = std::vector,
			template<typename> class AP = std::allocator,
			template<typename> class AR = std::allocator>
	class polygon
	{
		BOOST_CONCEPT_ASSERT((Point<P>));

		public :
			// Member types
			typedef P point_type;
			typedef linear_ring<P, VP, AP> ring_type;
			typedef VR<ring_type , AR<ring_type > > inner_container_type;

			inline const ring_type& outer() const { return m_outer; }
			inline const inner_container_type & inners() const { return m_inners; }

			inline ring_type& outer() { return m_outer; }
			inline inner_container_type & inners() { return m_inners; }

			/// Utility method, clears outer and inner rings
			inline void clear()
			{
				m_outer.clear();
				m_inners.clear();
			}

		private :
			ring_type m_outer;
			inner_container_type m_inners;
	};


	#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
	namespace traits
	{

		template<typename P,
				template<typename,typename> class VP, template<typename,typename> class VR,
				template<typename> class AP, template<typename> class AR>
		struct tag< polygon<P, VP, VR, AP, AR> >
		{
			typedef polygon_tag type;
		};


		template<typename P,
				template<typename,typename> class VP, template<typename,typename> class VR,
				template<typename> class AP, template<typename> class AR>
		struct ring_type< polygon<P, VP, VR, AP, AR> >
		{
			typedef typename polygon<P, VP, VR, AP, AR>::ring_type type;
		};


		template<typename P,
				template<typename,typename> class VP, template<typename,typename> class VR,
				template<typename> class AP, template<typename> class AR>
		struct interior_type< polygon<P, VP, VR, AP, AR> >
		{
			typedef typename polygon<P, VP, VR, AP, AR>::inner_container_type type;
		};


		template<typename P,
				template<typename,typename> class VP, template<typename,typename> class VR,
				template<typename> class AP, template<typename> class AR>
		struct exterior_ring< polygon<P, VP, VR, AP, AR> >
		{
			typedef polygon<P, VP, VR, AP, AR> POLY;
			static inline typename POLY::ring_type& get(POLY& polygon)
			{
				return polygon.outer();
			}

			static inline const typename POLY::ring_type& get(const POLY& polygon)
			{
				return polygon.outer();
			}
		};


		template<typename P,
				template<typename,typename> class VP, template<typename,typename> class VR,
				template<typename> class AP, template<typename> class AR>
		struct interior_rings< polygon<P, VP, VR, AP, AR> >
		{
			typedef polygon<P, VP, VR, AP, AR> POLY;

			static inline typename POLY::inner_container_type& get(POLY& polygon)
			{
				return polygon.inners();
			}

			static inline const typename POLY::inner_container_type& get(const POLY& polygon)
			{
				return polygon.inners();
			}
		};


	}
	#endif


} // namespace geometry


#endif //_GEOMETRY_POLYGON_HPP
