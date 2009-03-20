// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_BUFFER_HPP
#define _GEOMETRY_BUFFER_HPP

// Buffer functions
// Was before: "grow" but then only for box
// Now "buffer", but still only implemented for a box...

#include <boost/numeric/conversion/cast.hpp>

#include <geometry/core/concepts/point_concept.hpp>
#include <geometry/core/concepts/box_concept.hpp>

#include <geometry/arithmetic/arithmetic.hpp>

#include <geometry/util/assign_box_corner.hpp>
#include <geometry/util/promotion_traits.hpp>


/*!
\defgroup buffer buffer calculation
\par Source description:
- OGC: Returns a geometric object that represents all Points whose distance
	from this geometric object is less than or equal to distance. Calculations are in the spatial reference system of
	this geometric object. Because of the limitations of linear interpolation, there will often be some relatively
	small error in this distance, but it should be near the resolution of the coordinates used
\see http://en.wikipedia.org/wiki/Buffer_(GIS)
*/
namespace geometry
{




	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		namespace buffer
		{

			template <typename BOX_IN, typename BOX_OUT, typename T, size_t C, size_t D, size_t N>
			struct box_loop
			{
				typedef typename coordinate_type<BOX_OUT>::type BT;

				static inline void buffer(const BOX_IN& box_in, const T& distance, BOX_OUT& box_out)
				{
					set<C, D>(box_out, boost::numeric_cast<BT>(get<C, D>(box_in) + distance));
					box_loop<BOX_IN, BOX_OUT, T, C, D + 1, N>::buffer(box_in, distance, box_out);
				}
			};

			template <typename BOX_IN, typename BOX_OUT, typename T, size_t C, size_t N>
			struct box_loop<BOX_IN, BOX_OUT, T, C, N, N>
			{
				static inline void buffer(const BOX_IN&, const T&, BOX_OUT&) {}
			};


			// Extends a box with the same amount in all directions
			template<typename BOX_IN, typename BOX_OUT, typename T>
			inline void buffer_box(const BOX_IN& box_in, const T& distance, BOX_OUT& box_out)
			{
				assert_dimension_equal<BOX_IN, BOX_OUT>();

				static const size_t N = dimension<BOX_IN>::value;

				box_loop<BOX_IN, BOX_OUT, T, min_corner, 0, N>::buffer(box_in, -distance, box_out);
				box_loop<BOX_IN, BOX_OUT, T, max_corner, 0, N>::buffer(box_in, +distance, box_out);
			}



		} // namespace buffer
	} // namespace impl
	#endif

	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{

		template <typename TAG_IN, typename TAG_OUT, typename G_IN, typename T, typename G_OUT>
		struct buffer {};


		template <typename BOX_IN, typename T, typename BOX_OUT>
		struct buffer<box_tag, box_tag, BOX_IN, T, BOX_OUT>
		{
			static inline void calculate(const BOX_IN& box_in, const T& distance, const T& chord_length, BOX_IN& box_out)
			{
				impl::buffer::buffer_box(box_in, distance, box_out);
			}
		};

		// Many things to do. Point is easy, other geometries require self intersections
		// For point, note that it should output as a polygon (like the rest). Buffers
		// of a set of geometries are often lateron combined using a "dissolve" operation.
		// Two points close to each other get a combined kidney shaped buffer then.



	} // namespace dispatch
	#endif


	/*!
		\brief Calculate buffer (= new geometry) around specified distance of geometry
		\ingroup buffer
		\param geometry_in input geometry
		\param distance the distance used in buffer
		\param chord_length length of the chord's in the generated arcs around points or bends
		\param geometry_out buffered geometry
		\note Currently only implemented for box, the trivial case, but still useful
		\par Use case:
			BOX + distance -> BOX: it is allowed that "geometry_out" the same object as "geometry_in"
	 */
	template <typename G_IN, typename G_OUT, typename T>
	inline void buffer(const G_IN& geometry_in, G_OUT& geometry_out, const T& distance, const T& chord_length = -1)
	{
		dispatch::buffer<typename tag<G_IN>::type,
				typename tag<G_OUT>::type,
				G_IN, T, G_OUT>::calculate(geometry_in, distance, chord_length, geometry_out);
	}


	/*!
		\brief Calculate and return buffer (= new geometry) around specified distance of geometry
		\ingroup buffer
		\param geometry input geometry
		\param distance the distance used in buffer
		\param chord_length length of the chord's in the generated arcs around points or bends
		\return the buffered geometry
		\note See also: buffer
	 */
	template <typename G_OUT, typename G_IN, typename T>
	G_OUT make_buffer(const G_IN& geometry, const T& distance, const T& chord_length = -1)
	{
		G_OUT out;
		dispatch::buffer<typename tag<G_IN>::type,
				typename tag<G_OUT>::type,
				G_IN, T, G_OUT>::calculate(geometry, distance, chord_length, out);
		return out;
	}

};

#endif // _GEOMETRY_BUFFER_HPP
