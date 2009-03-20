// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_INTERSECTION_LINESTRING_HPP
#define _GEOMETRY_INTERSECTION_LINESTRING_HPP

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>


#include <geometry/algorithms/clear.hpp>
#include <geometry/algorithms/append.hpp>


#include <geometry/geometries/segment.hpp>


namespace geometry
{
	namespace strategy
	{
		namespace intersection
		{
			/*!
				\brief Strategy: line clipping algorithm after Liang Barsky
				\ingroup intersection
				\details The Liang-Barsky line clipping algorithm clips a line with a clipping box.
				It is slightly adapted in the sense that it returns which points are clipped
				\tparam B box type of clipping box
				\tparam S segment type of segments to be clipped
				\note The algorithm is currently only implemented for 2D Cartesian points
				\author Barend Gehrels, and the following recourses
				- A tutorial: http://www.skytopia.com/project/articles/compsci/clipping.html
				- a German applet (link broken): http://ls7-www.cs.uni-dortmund.de/students/projectgroups/acit/lineclip.shtml
			*/
			template<typename B, typename S>
			class liang_barsky
			{
				private :
					inline bool check_edge(const double& p, const double& q, double &t1, double &t2) const
					{
						bool visible = true;

						if(p < 0)
						{
							double r = q / p;
							if(r > t2) visible = false;
							else if(r > t1) t1 = r;
						}
						else if(p > 0)
						{
							double r = q / p;
							if(r < t1) visible = false;
							else if(r < t2) t2 = r;
						}
						else
						{
							if(q < 0) visible = false;
						}

						return visible;
					}

				public :
					bool clip_segment(const B& b, S& s, bool& sp1_clipped, bool& sp2_clipped) const
					{
						typedef typename select_coordinate_type<B, S>::type T;

						double t1 = 0;
						double t2 = 1;

						T dx = get<1, 0>(s) - get<0, 0>(s);
						T dy = get<1, 1>(s) - get<0, 1>(s);

						T p1 = -dx;
						T p2 = dx;
						T p3 = -dy;
						T p4 = dy;

						T q1 = get<0, 0>(s) - get<min_corner, 0>(b);
						T q2 = get<max_corner, 0>(b) - get<0, 0>(s);
						T q3 = get<0, 1>(s) - get<min_corner, 1>(b);
						T q4 = get<max_corner, 1>(b) - get<0, 1>(s);

						if(check_edge(p1, q1, t1, t2)  // left
								&& check_edge(p2, q2, t1, t2) // right
								&& check_edge(p3, q3, t1, t2) // bottom
								&& check_edge(p4, q4, t1, t2) // top
								)
						{
							sp1_clipped = t1 > 0;
							sp2_clipped = t2 < 1;

							// Todo, maybe: round coordinates in case of integer? define some round_traits<> or so?
							// Take boost-round of Fernando
							if (sp2_clipped)
							{
								set<1, 0>(s, get<0, 0>(s) + t2 * dx);
								set<1, 1>(s, get<0, 1>(s) + t2 * dy);
							}

							if(sp1_clipped)
							{
								set<0, 0>(s, get<0, 0>(s) + t1 * dx);
								set<0, 1>(s, get<0, 1>(s) + t1 * dy);
							}

							return true;
						}

						return false;
					}

					template<typename L, typename O_IT>
					inline void add(L& line_out, O_IT out) const
					{
						if (! boost::empty(line_out))
						{
							*out = line_out;
							out++;
							geometry::clear(line_out);
						}
					}
			};
		}
	}


	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		namespace intersection
		{


			/*!
				\brief Clips a linestring, or pair of iterators, with a box
				\details A linestring, defined by an iterator pair, is intersected (clipped) by the specified box
				and the resulting linestring, or pieces of linestrings, are sent to the specified output operator.
				\tparam C container type, for example a vector of points, matching the output-iterator type,
				     the points should also match the input-iterator type
				\tparam B box type
				\tparam IT in iterator type
				\tparam O_IT output iterator type, which outputs linestrings
				\tparam S strategy, a clipping strategy which should implement the methods "clip_segment" and "add"
			*/
			template <typename L, typename B, typename O_IT, typename S>
			O_IT clip_linestring_with_box(const B& b, const L& linestring, O_IT out, const S& strategy)
			{
				if (boost::begin(linestring) == boost::end(linestring))
				{
					return (out);
				}
				typedef typename point_type<L>::type P;
				typedef segment<P> SEG;

				L line_out;

				typename boost::range_const_iterator<L>::type vertex = boost::begin(linestring);
				typename boost::range_const_iterator<L>::type previous = vertex++;
				while(vertex != boost::end(linestring))
				{
					P p1 = *previous;
					P p2 = *vertex;

					// Clip the segment. Five situations:
					// 1. Segment is invisible, finish line if any (shouldn't occur)
					// 2. Segment is completely visible. Add (p1)-p2 to line
					// 3. Point 1 is invisible (clipped), point 2 is visible. Start new line from p1-p2...
					// 4. Point 1 is visible, point 2 is invisible (clipped). End the line with ...p2
					// 5. Point 1 and point 2 are both invisible (clipped). Start/finish an independant line p1-p2
					//
					// This results in:
					// a. if p1 is clipped, start new line
					// b. if segment is partly or completely visible, add the segment
					// c. if p2 is clipped, end the line

					bool c1, c2;
					SEG s(p1, p2);
					if (! strategy.clip_segment(b, s, c1, c2))
					{
						strategy.add(line_out, out);
					}
					else
					{
						// a. If necessary, finish the line and add a start a new one
						if (c1)
						{
							strategy.add(line_out, out);
						}

						// b. Add p1 only if it is the first point, then add p2
						if (boost::empty(line_out))
						{
							geometry::append(line_out, p1);
						}
						geometry::append(line_out, p2);

						// c. If c2 is clipped, finish the line
						if (c2)
						{
							strategy.add(line_out, out);
						}
					}
					previous = vertex++;
				}
				// Add last part
				strategy.add(line_out, out);
				return (out);
			}



		} // namespace intersection
	} // namespace impl
	#endif



} // namespace


#endif //_GEOMETRY_INTERSECTION_LINESTRING_HPP
