// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_COMBINE_HPP
#define _GEOMETRY_COMBINE_HPP


#include <boost/numeric/conversion/cast.hpp>

#include <geometry/core/concepts/point_concept.hpp>
#include <geometry/core/concepts/box_concept.hpp>

#include <geometry/arithmetic/arithmetic.hpp>

#include <geometry/util/assign_box_corner.hpp>
#include <geometry/util/promotion_traits.hpp>

/*!
\defgroup combine combine: add a geometry to a bounding box
\par Geometries:
- BOX + BOX -> BOX: the box will be combined with the other box \image html combine_box_box.png
- BOX + POINT -> BOX: the box will combined with the point  \image html combine_box_point.png
\note Previously called "grow"
*/
namespace geometry
{

	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		namespace combine
		{

			template <typename B, typename P, size_t D, size_t N>
			struct point_loop
			{
				static inline void check(B& box, const P& source)
				{
					typedef typename coordinate_type<P>::type T;
					const T& coordinate = get<D>(source);

					if (coordinate < get<min_corner, D>(box))
					{
						set<min_corner, D>(box, coordinate);
					}
					if (coordinate > get<max_corner, D>(box))
					{
						set<max_corner, D>(box, coordinate);
					}

					point_loop<B, P, D + 1, N>::check(box, source);
				}
			};

			template <typename B, typename P, size_t N>
			struct point_loop<B, P, N, N>
			{
				static inline void check(B&, const P&) {}
			};


			template <typename B1, typename B2, size_t C, size_t D, size_t N>
			struct box_loop
			{
				typedef typename select_coordinate_type<B1, B2>::type T;

				static inline void run(B1& box, const B2& source)
				{
					T coordinate = get<C, D>(source);

					if (coordinate < get<min_corner, D>(box))
					{
						set<min_corner, D>(box, coordinate);
					}
					if (coordinate > get<max_corner, D>(box))
					{
						set<max_corner, D>(box, coordinate);
					}

					box_loop<B1, B2, C, D + 1, N>::run(box, source);
				}
			};

			template <typename B1, typename B2, size_t C, size_t N>
			struct box_loop<B1, B2, C, N, N>
			{
				static inline void run(B1&, const B2&) {}
			};


			// Changes a box b such that it also contains point p
			template<typename B, typename P>
			inline void combine_box_with_point(B& b, const P& p)
			{
				point_loop<B, P, 0, dimension<P>::value>::check(b, p);
			}

			// Changes a box such that the other box is also contained by the box
			template<typename B1, typename B2>
			inline void combine_box_with_box(B1& b, const B2& other)
			{
				typedef typename point_type<B2>::type P;
				box_loop<B1, B2, min_corner, 0, dimension<P>::value>::run(b, other);
				box_loop<B1, B2, max_corner, 0, dimension<P>::value>::run(b, other);
			}



		} // namespace combine
	} // namespace impl
	#endif

	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{
		template <typename TAG, typename B, typename G>
		struct combine
		{
		};


		// Box + point -> new box containing also point
		// Currently implemented for boxes of same type
		template <typename B, typename P>
		struct combine<point_tag, B, P>
		{
			static inline void calculate(B& box_out, const P& point)
			{
				impl::combine::combine_box_with_point(box_out, point);
			}
		};

		// Box + box -> new box just containing two input boxes
		template <typename B, typename B_IN>
		struct combine<box_tag,  B, B_IN>
		{
			static inline void calculate(B& box_out, const B_IN& box)
			{
				impl::combine::combine_box_with_box(box_out, box);
			}
		};


	} // namespace dispatch
	#endif


	/*!
		\brief Combines a box with another geometry (box, point)
		\ingroup combine
		\tparam B type of first geometry
		\tparam G type of second geometry
		\tparam G3 type of output geometry
		\param box box to combine another geometry with, might be changed
		\param geometry other geometry
	 */
	template <typename B, typename G>
	inline void combine(B& box, const G& geometry)
	{
		dispatch::combine<typename tag<G>::type, B, G>::calculate(box, geometry);
	}

};

#endif // _GEOMETRY_COMBINE_HPP
