// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_SEGMENT_HPP
#define _GEOMETRY_SEGMENT_HPP

#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/concept/assert.hpp>

#include <geometry/core/concepts/point_concept.hpp>


namespace geometry
{

	/*!
		\brief Class segment: small containing two (templatized) point references
		\ingroup Geometry
		\details From Wikipedia: In geometry, a line segment is a part of a line that is bounded
		by two distinct end points, and contains every point on the line between its end points
		\note The structure is like std::pair, and can often be used interchangeable.
		So points are public available. We cannot derive from std::pair<P&, P&> because of reference
		assignments. Points are not const and might be changed by the algorithm
		(used in intersection_linestring)
		\tparam P point type of the segment
	 */
	template<typename P>
	struct segment
	{
		private :
			BOOST_CONCEPT_ASSERT((typename boost::mpl::if_<
									  boost::is_const<P>,
									  ConstPoint<P>,
									  Point<P>
								  >
			));

		public :
			typedef P point_type;
			P& first;
			P& second;
			inline segment(P& p1, P& p2)
				: first(p1), second(p2)
			{}
	};


	// Traits specializations for segment above
	#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
	namespace traits
	{

		template <typename P>
		struct tag< segment<P> > { typedef segment_tag type; };

		template <typename P>
		struct point_type<segment<P> > { typedef P type; };

		template <typename P, size_t I, size_t D>
		struct indexed_access<segment<P>, I, D>
		{
			typedef segment<P> S;
			typedef typename geometry::coordinate_type<S>::type T;

			static inline T get(const S& s)
			{
				return I == 0 ? geometry::get<D>(s.first) : geometry::get<D>(s.second);
			}

			static inline void set(S& s, const T& value)
			{
				if (I == 0)
				{
					geometry::set<D>(s.first, value);
				}
				else
				{
					geometry::set<D>(s.second, value);
				}
			}
		};

	}
	#endif


} // namespace geometry


#endif //_GEOMETRY_SEGMENT_HPP
