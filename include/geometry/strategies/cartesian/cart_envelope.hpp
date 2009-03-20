// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_STRATEGY_CARTESIAN_ENVELOPE_HPP
#define _GEOMETRY_STRATEGY_CARTESIAN_ENVELOPE_HPP



#include <geometry/geometries/point_xy.hpp>
#include <geometry/geometries/segment.hpp>

#include <geometry/algorithms/combine.hpp>


namespace geometry
{
	namespace strategy
	{
		namespace envelope
		{
			// envelope calculation strategy for xy-points
			template <typename P, typename B>
			struct combine_xy
			{
				struct state
				{
					B& m_box;
					state(B& box) : m_box(box)
					{
						assign_inverse(m_box);
					}
				};

				typedef state state_type;

				void operator()(const P& p, state_type& s) const
				{
					geometry::combine(s.m_box, p);
				}
			};
		} // namespace envelope

	} // namespace strategy



	#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
	template <typename P, typename B>
	struct strategy_envelope<cartesian_tag, cartesian_tag, P, B>
	{
		typedef strategy::envelope::combine_xy<P, B> type;
	};
	#endif


} // namespace geometry


#endif // _GEOMETRY_STRATEGY_CARTESIAN_ENVELOPE_HPP
