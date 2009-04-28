// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_COPY_HPP
#define _GEOMETRY_COPY_HPP

#include <boost/concept/requires.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <geometry/core/concepts/point_concept.hpp>

// TODO: merge with "assign"

namespace geometry
{
	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		namespace copy
		{
			template <typename PS, typename PD, size_t D, size_t N>
			struct copy_coordinates
			{
				static inline void copy(const PS& source, PD& dest)
				{
					typedef typename coordinate_type<PD>::type T;
					set<D>(dest, boost::numeric_cast<T>(get<D>(source)));
					copy_coordinates<PS, PD, D + 1, N>::copy(source, dest);
				}
			};

			template <typename PS, typename PD, size_t N>
			struct copy_coordinates<PS, PD, N, N>
			{
				static inline void copy(const PS& /* source */, PD& /* dest */)
				{}
			};

		} // namespace copy

	} // namespace impl
	#endif


	/*!
		\brief Copies coordinates from source to destination point
		\ingroup assign
		\details The function copy_coordinates copies coordinates from one point to another point.
		Source point and destination point might be of different types.
		\param source Source point
		\param dest Destination point
		\note If destination type differs from source type, they must have the same coordinate count
	 */
	template <typename PS, typename PD>
	BOOST_CONCEPT_REQUIRES(((ConstPoint<PS>)) ((Point<PD>)),
	(void)) copy_coordinates(const PS& source, PD& dest)
	{
		//assert_dimension_equal<PD, PS>();
		impl::copy::copy_coordinates<PS, PD, 0, dimension<PS>::value>::copy(source, dest);
	}


} // namespace geometry


#endif // _GEOMETRY_COPY_HPP
