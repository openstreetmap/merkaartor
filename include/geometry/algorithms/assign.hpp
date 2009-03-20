// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_ASSIGN_HPP
#define _GEOMETRY_ASSIGN_HPP


#include <boost/concept/requires.hpp>

#include <boost/numeric/conversion/bounds.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <geometry/core/tags.hpp>

#include <geometry/core/concepts/point_concept.hpp>

#include <geometry/core/access.hpp>
#include <geometry/core/radius.hpp>
#include <geometry/core/exterior_ring.hpp>

#include <geometry/algorithms/clear.hpp>
#include <geometry/algorithms/append.hpp>


#include <geometry/util/copy.hpp>
#include <geometry/util/for_each_coordinate.hpp>


/*!
\defgroup access access: get/set coordinate values, make objects, clear geometries, append point(s)
\details There are many ways to edit geometries. It is possible to:
- use the geometries themselves, so access point.x(). This is not done inside the library because it is agnostic
     to geometry type. However, library users can use this as it is intuitive.
- use the standard library, so use .push_back(point) or use inserters. This is also avoided inside the library.
However, library users can use it if they are used to the standard library
- use the functionality provided in this geometry library. These are the functions in this module.

The library provides the following functions to edit geometries:
- set to set one coordinate value
- assign to set two or more coordinate values
- make to construct and return geometries with specified coordinates.
- append to append one or more points to a geometry
- clear to remove all points from a geometry

For getting coordinates it is similar:
- get to get a coordinate value
- or use the standard library
- or use the geometries themselves

*/

namespace geometry
{

	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		namespace assign
		{

			template <typename C>
			struct assign_operation
			{
				inline assign_operation(C value) : m_value(value) {}

				template <typename P, size_t I>
				inline void run(P& point) const
				{
					set<I>(point, m_value);
				}

				private :
					C m_value;
			};


			/*!
				\brief Assigns all coordinates of a specific point to a value
				\ingroup access
				\details
				\param p Point
				\param value Value which is assigned to all coordinates of point p
			 */
			template <typename P>
			inline void assign_value(P& p, const typename coordinate_type<P>::type& value)
			{
				for_each_coordinate(p, assign_operation<typename coordinate_type<P>::type>(value));
			}


			template <typename P, typename T, size_t I, size_t D>
			struct assign_point
			{
				typedef typename coordinate_type<P>::type TP;

				static inline void assign(const T values[D], P& point)
				{
					set<I>(point,  boost::numeric_cast<TP>(values[I]));
					assign_point<P, T, I + 1, D>::assign(values, point);
				}
			};

			template <typename P, typename T, size_t D>
			struct assign_point<P, T, D, D>
			{
				static inline void assign(const T values[D], P& point) {}
			};



			template <typename B, size_t C, size_t I, size_t D>
			struct initialize
			{
				typedef typename coordinate_type<B>::type T;

				static inline void init(B& box, const T& value)
				{
					set<C, I>(box, value);
					initialize<B, C, I + 1, D>::init(box, value);
				}
			};

			template <typename B, size_t C, size_t D>
			struct initialize<B, C, D, D>
			{
				typedef typename coordinate_type<B>::type T;
				static inline void init(B& box, const T& value) {}
			};



		} // namespace assign

	} // namespace impl
	#endif


	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{
		template <typename TAG, typename G>
		struct assignment {};


		template <typename P>
		struct assignment<point_tag, P>
		{
			template <typename T>
			static inline void assign(P& point, const T& coor1, const T& coor2)
			{
				assert_dimension<P, 2>();

				typedef typename coordinate_type<P>::type TP;
				set<0>(point, boost::numeric_cast<TP>(coor1));
				set<1>(point, boost::numeric_cast<TP>(coor2));
			}

			template <typename T>
			static inline void assign(P& point, const T& coor1, const T& coor2, const T& coor3)
			{
				assert_dimension<P, 3>();

				typedef typename coordinate_type<P>::type TP;
				set<0>(point, boost::numeric_cast<TP>(coor1));
				set<1>(point, boost::numeric_cast<TP>(coor2));
				set<2>(point, boost::numeric_cast<TP>(coor3));
			}

			static inline void assign_zero(P& point)
			{
				typedef typename coordinate_type<P>::type T;
				impl::assign::assign_value(point, 0);
			}

		};

		template <typename B>
		struct assignment<box_tag, B>
		{
			typedef typename point_type<B>::type P;

			// Here we assign 4 coordinates to a box.
			// -> Most logical is: x1,y1,x2,y2
			// In case the user reverses x1/x2 or y1/y2, we could reverse them (THAT IS NOT IMPLEMENTED)

			// Note also comment in util/assign_box_corner ->
			//   ("Most logical is LOWER, UPPER and sub-order LEFT, RIGHT")
			//   (That is assigning 4 points from a box. So lower-left, lower-right, upper-left, upper-right)
			template <typename T>
			static inline void assign(B& box, const T& x1, const T& y1, const T& x2, const T& y2)
			{
				assert_dimension<B, 2>();

				typedef typename coordinate_type<P>::type TB;
				set<min_corner, 0>(box, boost::numeric_cast<TB>(x1));
				set<min_corner, 1>(box, boost::numeric_cast<TB>(y1));
				set<max_corner, 0>(box, boost::numeric_cast<TB>(x2));
				set<max_corner, 1>(box, boost::numeric_cast<TB>(y2));
			}


			static inline void assign_inverse(B& box)
			{
				typedef typename coordinate_type<P>::type T;
				static const size_t N = dimension<P>::value;
				impl::assign::initialize<B, min_corner, 0, N>::init(box, boost::numeric::bounds<T>::highest());
				impl::assign::initialize<B, max_corner, 0, N>::init(box, boost::numeric::bounds<T>::lowest());
			}

			static inline void assign_zero(B& box)
			{
				typedef typename coordinate_type<P>::type T;
				static const size_t N = dimension<P>::value;
				impl::assign::initialize<B, min_corner, 0, N>::init(box, T());
				impl::assign::initialize<B, max_corner, 0, N>::init(box, T());
			}

		};


		template <typename S>
		struct assignment<nsphere_tag, S>
		{
			typedef typename point_type<S>::type P;

			/// 2-value version for an n-sphere is valid for circle and sets the center
			template <typename T>
			static inline void assign(S& sphercle, const T& coor1, const T& coor2)
			{
				assert_dimension<S, 2>();

				typedef typename coordinate_type<S>::type TS;

				set<0>(sphercle, boost::numeric_cast<TS>(coor1));
				set<1>(sphercle, boost::numeric_cast<TS>(coor2));
			}


			/// 3-value version for an n-sphere is sets the center of a sphere OR the center + radius of a circle
			template <typename T>
			static inline void assign(S& sphercle, const T& coor1, const T& coor2, const T& coor3_or_radius)
			{
				typedef typename coordinate_type<S>::type TS;
				typedef typename coordinate_type<S>::type TR;
				typedef typename radius_type<S>::type R;

				set<0>(sphercle, boost::numeric_cast<TS>(coor1));
				set<1>(sphercle, boost::numeric_cast<TS>(coor2));
				switch(dimension<S>::value)
				{
					case 2 :
						set_radius<0>(sphercle, boost::numeric_cast<R>(coor3_or_radius));
						break;
					case 3 :
						set<2>(sphercle, boost::numeric_cast<TS>(coor3_or_radius));
						break;
				}
			}

			/// 4-value version for an n-sphere is valid for a sphere and sets the center and the radius
			template <typename T>
			static inline void assign(S& sphercle, const T& coor1, const T& coor2, const T& coor3, const T& radius)
			{
				assert_dimension<S, 3>();

				typedef typename coordinate_type<S>::type TS;
				typedef typename coordinate_type<S>::type TR;
				typedef typename radius_type<S>::type R;

				set<0>(sphercle, boost::numeric_cast<TS>(coor1));
				set<1>(sphercle, boost::numeric_cast<TS>(coor2));
				set<2>(sphercle, boost::numeric_cast<TS>(coor3));
				set_radius<0>(sphercle, boost::numeric_cast<R>(radius));
			}

		};

	} // namespace dispatch
	#endif







	/*!
		\brief assign two values to a 2D point
		\ingroup access
	 */
	template <typename G, typename T>
	inline void assign(G& geometry, const T& coor1, const T& coor2)
	{
		dispatch::assignment<typename tag<G>::type, G>::assign(geometry, coor1, coor2);
	}


	/*!
		\brief assign three values to a 3D point or the center + radius to a circle
		\ingroup access
	 */
	template <typename G, typename T>
	inline void assign(G& geometry, const T& coor1, const T& coor2, const T& coor3)
	{
		dispatch::assignment<typename tag<G>::type, G>::assign(geometry, coor1, coor2, coor3);
	}

	/*!
		\brief assign center + radius to a sphere
		\ingroup access
	 */
	template <typename G, typename T>
	inline void assign(G& geometry, const T& coor1, const T& coor2, const T& coor3, const T& coor4)
	{
		dispatch::assignment<typename tag<G>::type, G>::assign(geometry, coor1, coor2, coor3, coor4);
	}

	/*!
		\brief assign a range of points to a linestring, ring or polygon
		\note The point-type of the range might be different from the point-type of the geometry
		\ingroup access
	 */
	template <typename G, typename R>
	inline void assign(G& geometry, const R& range)
	{
		clear(geometry);
		append(geometry, range, -1, 0);
	}


	/*!
		\brief assign to a box inverse infinite
		\details The assign_inverse function initialize a 2D or 3D box with large coordinates, the
		min corner is very large, the max corner is very small. This is a convenient starting point to
		collect the minimum bounding box of a geometry.
		\ingroup access
	 */
	template <typename G>
	inline void assign_inverse(G& geometry)
	{
		dispatch::assignment<typename tag<G>::type, G>::assign_inverse(geometry);
	}


	/*!
		\brief assign zero values to a box, point
		\ingroup access
		\details The assign_zero function initializes a 2D or 3D point or box with coordinates of zero
		\tparam G the geometry type
	 */
	template <typename G>
	inline void assign_zero(G& geometry)
	{
		dispatch::assignment<typename tag<G>::type, G>::assign_zero(geometry);
	}


} // namespace geometry


#endif // _GEOMETRY_ASSIGN_HPP
