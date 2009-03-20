// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_INTERSECTION_HPP
#define _GEOMETRY_INTERSECTION_HPP

#include <geometry/algorithms/intersection_segment.hpp>
#include <geometry/algorithms/intersection_linestring.hpp>
#include <geometry/algorithms/intersection_polygon.hpp>

// Helper container and helper geometry
#include <vector>
#include <geometry/geometries/segment.hpp>


/*!
\defgroup intersection intersection (AND operation) and clipping
\details The intersection of two geometries A and B is the geometry containing all points of A also belonging to B,
but no other elements. The so-called clip is an intersection of a geometry with a box.
\par Source description:
- OGC: Returns a geometric object that represents the Point set intersection of this geometric object with another Geometry.
\see http://en.wikipedia.org/wiki/Intersection_(set_theory)
\note Any intersection can result in no geometry at all

\note Used strategies still have to be modelled. Working only for cartesian
\par Geometries:
The intersection result is painted with a red outline.
- clip: POLYGON + BOX -> output iterator of polygons
	\image html clip_polygon.png
- clip: LINESTRING + BOX -> output iterator of linestrings
	\image html clip_linestring.png
\note There are some difficulties to model an intersection in the template world. The intersection of two segments can
result into nothing, into a point, into another segment. At compiletime the result type is not known. An output iterator
iterating points is appropriate here.
	\image html clip_segment_segment.png
An intersection of two linestrings can result into nothing, one or more points, one or more segments or one or more
linestrings. So an output iterator will NOT do here.
So the output might be changed into a unified algorithm where the output is a multi-geometry.
For the current clip-only operations the output iterator will do.

\par Example:
Example showing clipping of linestring with box
\dontinclude doxygen_examples.cpp
\skip example_intersection_linestring1
\line {
\until }
\par Example:
Example showing clipping of vector, outputting vectors, with box
\dontinclude doxygen_examples.cpp
\skip example_intersection_linestring2
\line {
\until }
\par Example:
Example showing clipping of polygon with box
\dontinclude doxygen_examples.cpp
\skip example_intersection_polygon1
\line {
\until }
*/


namespace geometry
{

	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{

		template <typename TAG1, typename TAG2, typename G1, typename G2>
		struct intersection {};

		template <typename B, typename L>
		struct intersection<box_tag, linestring_tag, B, L>
		{
			template<typename O_IT>
			static inline O_IT calculate(const B& box, const L& linestring, O_IT out)
			{
				typedef typename point_type<L>::type P;
				typedef segment<P> S;
				strategy::intersection::liang_barsky<B, S> strategy;

				return (impl::intersection::clip_linestring_with_box(box, linestring, out, strategy));
			}
		};

		template <typename B, typename P>
		struct intersection<box_tag, polygon_tag, B, P>
		{
			template<typename O_IT>
			static inline O_IT calculate(const B& box, const P& poly, O_IT out)
			{
				return impl::intersection::poly_weiler_atherton(box, poly, out);
			}
		};

	} // namespace dispatch
	#endif





	/*!
		\brief Intersects two geometries which each other
		\ingroup intersection
		\details A sequence of points is intersected (clipped) by the specified box
		and the resulting linestring, or pieces of linestrings, are sent to the specified output operator.
		\tparam G1 first geometry type
		\tparam G2 second geometry type
		\tparam O_IT output iterator
		\param geometry1 first geometry (currently only a BOX)
		\param geometry2 second geometry (range, linestring, polygon)
		\param out the output iterator, outputting linestrings or polygons
		\return the output iterator
		\note For linestrings: the default clipping strategy, Liang-Barsky, is used. The algorithm is currently only
		implemented for 2D xy points. It could be generic for most ll cases, but not across the 180
		meridian so that issue is still on the todo-list.
	*/
	template <typename G1, typename G2, typename O_IT>
	inline O_IT intersection(const G1& geometry1, const G2& geometry2, O_IT out)
	{
		return dispatch::intersection<typename tag<G1>::type,
			typename tag<G2>::type, G1, G2>::calculate(geometry1, geometry2, out);
	}

}

#endif //_GEOMETRY_INTERSECTION_HPP
