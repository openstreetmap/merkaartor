// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_STRATEGY_AGNOSTIC_WITHIN_HPP
#define _GEOMETRY_STRATEGY_AGNOSTIC_WITHIN_HPP



#include <geometry/geometries/segment.hpp>

#include <geometry/strategies/strategy_traits.hpp>



namespace geometry
{
	namespace strategy
	{
		namespace within
		{

			/*!
				\brief Within detection using winding rule
				\tparam P point type of point to examine
				\tparam PS point type of segments, defaults to P
				\author The implementation is inspired by terralib http://www.terralib.org (LGPL)
				and http://geometryalgorithms.com/Archive/algorithm_0103/algorithm_0103.htm
				\note Added the horizontal case, for "completely within"
				\note Is tested for points ON border, more tests have to be done.
				\note More efficient (less comparisons and no divison) than the cross count algorithm
				\note Only dependant on "side", -> agnostic, suitable for latlong
			 */
			template<typename P, typename PS = P>
			class winding
			{
				private :
					/*! subclass to keep state */
					struct windings
					{
						int count;
						bool touches;
						const P& p;
						explicit windings(const P& ap)
							: count(0)
							, touches(false)
							, p(ap)
						{}
						bool within() const
						{
							return ! touches && count != 0;
						}
					};

				public :

					typedef windings state_type;

					inline bool operator()(const segment<const PS>& s, state_type& state) const
					{
						bool up = false;
						bool down = false;

						if (equals(get<0, 1>(s), get<1>(state.p)) && equals(get<0, 1>(s), get<1, 1>(s)))
						{
							// Horizontal case
							if (get<0, 0>(s) <= get<0>(state.p) && get<1, 0>(s) > get<0>(state.p))
							{
								up = true; // "up" means from left to right here
							}
							else if (get<1, 0>(s) < get<0>(state.p) && get<0, 0>(s) >= get<0>(state.p))
							{
								down = true; // from right to left
							}
						}
						else if (get<0, 1>(s) <= get<1>(state.p) && get<1, 1>(s) > get<1>(state.p))
						{
							up = true;
						}
						else if (get<1, 1>(s) < get<1>(state.p) && get<0, 1>(s) >= get<1>(state.p))
						{
							down = true;
						}

						if (up || down)
						{
							typedef typename select_coordinate_type<P, PS>::type T;

							typedef typename strategy_side<typename cs_tag<P>::type, P, PS>::type SS;

							T side = SS::side(s, state.p);

							if (equals<T>(side, 0))
							{
								state.touches = true;
								return false;
							}
							else if (up && side > 0)
							{
								state.count++;
							}
							else if (down && side < 0)
							{
								state.count--;
							}
						}
						return true;
					}

			};

		} // namespace within

	} // namespace strategy


	#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
	template <typename P, typename PS>
	struct strategy_within<cartesian_tag, cartesian_tag, P, PS>
	{
		typedef strategy::within::winding<P, PS> type;
	};

	template <typename P, typename PS>
	struct strategy_within<geographic_tag, geographic_tag, P, PS>
	{
		typedef strategy::within::winding<P, PS> type;
	};
	#endif


} // namespace geometry


#endif // _GEOMETRY_STRATEGY_AGNOSTIC_WITHIN_HPP
