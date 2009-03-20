// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_STRATEGY_AGNOSTIC_CONVEX_HULL_HPP
#define _GEOMETRY_STRATEGY_AGNOSTIC_CONVEX_HULL_HPP

#include <vector>
#include <boost/range/functions.hpp>

#include <geometry/core/cs.hpp>

#include <geometry/strategies/strategy_traits.hpp>



// Temporary, comparing tests
#if defined(GGL_USE_SMOOTH_SORT)
#  include "SmoothSort.hpp"
#elif defined(GGL_USE_MERGE_SORT)
#  include "MergeSort.hpp"
#else

#include <algorithm>

#endif


#include <boost/range/functions.hpp>



namespace geometry
{
	namespace strategy
	{
		namespace convex_hull
		{

			#ifndef DOXYGEN_NO_IMPL
			namespace impl
			{
				template <typename R, typename RIT, typename SS>
				static inline void get_extremes(const R& range, RIT& min_it, RIT& max_it,
					const SS& strategy)
				{
					min_it = boost::begin(range);
					max_it = boost::begin(range);
					for (RIT it = boost::begin(range) + 1; it != boost::end(range); it++)
					{
						if (strategy.smaller(*it, *min_it))
						{
							min_it = it;
						}
						if (strategy.larger(*it, *max_it))
						{
							max_it = it;
						}
					}
				}


				template <typename R>
				static inline void sort(R& range)
				{
					#if defined(USE_SMOOTH_SORT)
					smoothsort::sort(boost::begin(range), boost::end(range));
					#elif defined(USE_MERGE_SORT)
					comparing::merge_sort<thread_count>(boost::begin(range), boost::end(range), std::less<P>());
					#else

					std::sort(boost::begin(range), boost::end(range));

					#endif
				}

			}
			#endif


			// Completely reworked version from this source: http://marknelson.us/2007/08/22/convex
			template <typename P>
			class graham
			{
				private :

					typedef typename cs_tag<P>::type PTAG;
					typedef typename std::vector<P> CONTAINER;
					typedef typename std::vector<P>::const_iterator CIT;
					typedef typename std::vector<P>::const_reverse_iterator CRIT;

					CONTAINER m_lower_hull;
					CONTAINER m_upper_hull;

				public :
					// Constructor with a range
					template <typename R>
					inline graham(const R& range)
					{
						typedef typename boost::range_const_iterator<R>::type RIT;

						// Polygons with three corners, or closed with 4 points, are always convex
						if (boost::size(range) <= 3)
						{
							for (RIT it = boost::begin(range); it != boost::end(range); it++)
							{
								m_upper_hull.push_back(*it);
							}
							return;
						}

						// Get min/max (in most cases left / right) points
						RIT left_it, right_it;
						typename strategy_compare<PTAG, P, 0>::type comparing;
						impl::get_extremes(range, left_it, right_it, comparing);

						// Bounding left/right points
						CONTAINER upper_points;
						CONTAINER lower_points;

						typename strategy_side<PTAG, P>::type side;

						// Now put the remaining points in one of the two output sequences
						for (RIT it = boost::begin(range); it != boost::end(range); it++)
						{
							if (it != left_it && it != right_it)
							{
								int dir = side.side(*left_it, *right_it, *it);
								if ( dir < 0 )
								{
									upper_points.push_back(*it);
								}
								else
								{
									lower_points.push_back(*it);
								}
							}
						}

						impl::sort(lower_points);
						impl::sort(upper_points);

						build_half_hull<1>(lower_points, m_lower_hull, *left_it, *right_it);
						build_half_hull<-1>(upper_points, m_upper_hull, *left_it, *right_it);
					}


					template <int F>
					inline void build_half_hull(const CONTAINER& input, CONTAINER& output,
							const P& left, const P& right)
					{
						output.push_back(left);
						for(CIT it = input.begin(); it != input.end(); it++)
						{
							add_to_hull<F>(*it, output);
						}
						add_to_hull<F>(right, output);
					}

					template <int F>
					inline void add_to_hull(const P& p, CONTAINER& output)
					{
						typename strategy_side<PTAG, P>::type side;

						output.push_back(p);
						register size_t output_size = output.size();
						while (output_size >= 3)
						{
							CRIT rit = output.rbegin();
							const P& last = *rit++;
							const P& last2 = *rit++;

							if (F * side.side(*rit, last, last2) <= 0)
							{
								// Remove last two points from stack, and add last again
								// This is much faster then erasing the one but last.
								output.pop_back();
								output.pop_back();
								output.push_back(last);
								output_size--;
							}
							else
							{
								return;
							}
						}
					}


					template <typename O_IT>
					inline void get(O_IT out)
					{
						for (CIT it = m_upper_hull.begin(); it != m_upper_hull.end(); it++, out++)
						{
							*out = *it;
						}

						// STL Port does not accept iterating from rbegin+1 to rend
						size_t size = m_lower_hull.size();
						if (size > 0)
						{
							CRIT it = m_lower_hull.rbegin() + 1;
							for (unsigned int i = 1; i < size; i++, it++, out++)
							{
								*out = *it;
							}
						}
					}
			}; // graham

		} // namespace convex_hull
	} // namespace strategy



	#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
	template <typename P>
	struct strategy_convex_hull<cartesian_tag, P>
	{
		typedef strategy::convex_hull::graham<P> type;
	};
	#endif


} // namespace geometry


#endif // _GEOMETRY_STRATEGY_AGNOSTIC_CONVEX_HULL_HPP
