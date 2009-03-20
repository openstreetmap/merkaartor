// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_INTERSECTION_SEGMENT_HPP
#define _GEOMETRY_INTERSECTION_SEGMENT_HPP

#include <limits>
#include <vector>

#include <geometry/core/access.hpp>
#include <geometry/core/concepts/segment_concept.hpp>

#include <geometry/util/math.hpp>
#include <geometry/util/promotion_traits.hpp>


namespace geometry
{



	/*!
	\brief Enumeration containing intersection types
	*/
	enum intersection_type {is_intersect_no,
			is_intersect,
			is_parallel,
			is_collinear_no, is_collinear_one, is_collinear_connect, is_collinear_overlap,
			is_collinear_overlap_opposite, is_collinear_connect_opposite};

	/*!
	\brief Enumeration containing connection types of intersected segments
	*/
	enum connection_type { is_connect_no,
			is_connect_s1p1, is_connect_s1p2,
			is_connect_s2p1, is_connect_s2p2
	};

	/*!
	\brief Encapsulates the result of a segment intersection.
	\details The class contains the intersection type and two parameters
	ra and rb, indicating how the two segments connect (if they connect).
	This connection type can be examined with the method connection_type.
	*/
	struct intersection_result
	{
		intersection_result()
			: is_type(is_intersect_no)
			, ra(0)
			, rb(0)
		{}

		intersection_type is_type;
		double ra;
		double rb;

		inline connection_type get_connection_type()
		{
			if (is_type == is_intersect)
			{
				if (equals(rb, 0.0)) return is_connect_s2p1;
				if (equals(rb, 1.0)) return is_connect_s2p2;
				if (equals(ra, 0.0)) return is_connect_s1p1;
				if (equals(ra, 1.0)) return is_connect_s1p2;
			}
			return is_connect_no;
		}

	};




	/*!
		\brief Calculate zero, one or two intersections of two linesegments, using Cramer's rule.
		\ingroup intersection
		\details Two intersections are returned if segments are on same line and (partially) overlap.
		One intersection is returned if segments intersect
		Intersection points is vector of points, might be yet another pointtype then points making up segment
		\tparam P type of outputted intersection point(s). Must be specified, cannot be deducted
		from parameters. Type might be different from segment points.
		\tparam S1 first segment type
		\tparam S2 second segment type
		\tparam O_IT output iterator
		\param s1 first segment
		\param s2 second segment
		\param out output iterator
		\return the intersection result, a struct containing the intersection type, and two calculated
		values which can be examined further if necessary
		\note <b>The algorithm is currently only implemented for 2D Cartesian points. It
		will be redesigned to work with strategies.</b> Algorithms to calculate segment intersections
		on a sphere are available, see http://williams.best.vwh.net/avform.htm.
		\par Example:
		Example showing intersection of two segments
		\dontinclude doxygen_examples.cpp
		\skip example_intersection_segment1
		\line {
		\until }
	*/
	template <typename P, typename S1, typename S2, typename O_IT>
	inline intersection_result intersection_segment(const S1& s1, const S2& s2, O_IT out)
	{
		BOOST_CONCEPT_ASSERT( (Point<P>) );

		BOOST_CONCEPT_ASSERT( (ConstSegment<S1>) );
		BOOST_CONCEPT_ASSERT( (ConstSegment<S2>) );

		intersection_result result;

		typedef typename select_coordinate_type<S1, S2>::type T;

		T dx1 = get<1, 0>(s1) - get<0, 0>(s1);
		T dx2 = get<1, 0>(s2) - get<0, 0>(s2);
		T dy1 = get<1, 1>(s1) - get<0, 1>(s1);
		T dy2 = get<1, 1>(s2) - get<0, 1>(s2);

		T wx = get<0, 0>(s1) - get<0, 0>(s2);
		T wy = get<0, 1>(s1) - get<0, 1>(s2);

		// Calculate determinants - Cramers rule
        T d = (dy2 * dx1) - (dx2 * dy1);
        T a = (dx2 * wy) - (dy2 * wx);
        T b = (dx1 * wy) - (dy1 * wx);

        if(equals(d, 0))
        {
            if(equals(a, 0) && equals(b, 0))
            {
				bool vertical = equals(dx2, 0);

				// Maybe change this by boost::minmax construct(s)

				if (! vertical
						&& (std::min(get<0, 0>(s1), get<1, 0>(s1)) > std::max(get<0, 0>(s2), get<1, 0>(s2))
							|| std::min(get<0, 0>(s2), get<1, 0>(s2)) > std::max(get<0, 0>(s1), get<1, 0>(s1))))
				{
					// No overlap, segments are not connected
					result.is_type = is_collinear_no;
					return result;
				}
				else if (vertical
						&& (std::min(get<0, 1>(s1), get<1, 1>(s1)) > std::max(get<0, 1>(s2), get<1, 1>(s2))
						|| std::min(get<0, 1>(s2), get<1, 1>(s2)) > std::max(get<0, 1>(s1), get<1, 1>(s1))))
				{
					// No overlap, vertical segments are not connected
					result.is_type = is_collinear_no;
					return result;
				}
				else
				{
					// Partial or full overlap, calculate ratio in x-direction, for vertical lines in y-direction
					// Adapted from http://www.geometryalgorithms.com/Archive/algorithm_0104/algorithm_0104B.htm
							// Copyright 2001, softSurfer (www.softsurfer.com)
							// This code may be freely used and modified for any purpose
							// providing that this copyright notice is included with it.
							// SoftSurfer makes no warranty for this code, and cannot be held
							// liable for any real or imagined damage resulting from its use.
							// Users of this code must verify correctness for their application.

					// Segments of single point, with overlap
					if (equals(dx1, 0) && equals(dy1, 0))
					{
						// Take point on first segment
						// TODO: copy constructor only works for points with same type. Change.
						P p(s1.first);
						*out = p;
						out++;
						result.is_type = is_collinear_one;
						return result;
					}
					if (equals(dx2, 0) && equals(dy2, 0))
					{
						P p(s2.first);
						*out = p;
						out++;
						result.is_type = is_collinear_one;
						return result;
					}

					result.ra = vertical
									? wy / dy2
									: wx / dx2;
					result.rb = vertical
									? (get<1, 1>(s1) - get<0, 1>(s2)) / dy2
									: (get<1, 0>(s1) - get<0, 0>(s2)) / dx2;

					result.is_type = is_collinear_overlap;
					// Make sure a<b and a>= 0 and b<=1
					if (result.ra > result.rb)
					{
						std::swap(result.ra, result.rb);
						result.is_type = is_collinear_overlap_opposite;
					}
					if (result.ra < 0.0) result.ra = 0.0;
					if (result.rb > 1.0) result.rb = 1.0;

					// Calculate first intersection point
					*out = P(get<0, 0>(s2) + result.ra * dx2, get<0, 1>(s2) + result.ra * dy2);
					out++;

					if (equals(result.ra, result.rb))
					{
						// Intersect with overlap at one point, segments connect
						// They might connect in opposite direction and give different result (TODO)
						result.is_type = is_collinear_connect;
						return result;
					}

					// Calculate second intersection point
					*out = P(get<0, 0>(s2) + result.rb * dx2, get<0, 1>(s2) + result.rb * dy2);
					out++;
					return result;
				}
            }

			// Segments are parallel
            result.is_type = is_parallel;
			return result;
        }

		// Segments do intersect. Determinant d is nonzero.
        result.ra = double(a) / double(d);
        result.rb = double(b) / double(d);

		// Check if intersection is on segment
        if(result.ra >= 0.0 && result.ra <= 1.0
			&& result.rb >= 0.0 && result.rb <= 1.0)
        {
            // This is the normal case. All above are exceptions. Calculate intersection point.
			*out = P(get<0, 0>(s1) + result.ra * dx1, get<0, 1>(s1) + result.ra * dy1);
			out++;

			result.is_type = is_intersect;
			return result;
        }

        result.is_type = is_intersect_no;
		return result;
	}


} // namespace


#endif //_GEOMETRY_INTERSECTION_SEGMENT_HPP


