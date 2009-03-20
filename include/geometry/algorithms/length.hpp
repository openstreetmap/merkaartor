// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_LENGTH_HPP
#define _GEOMETRY_LENGTH_HPP

#include <iterator>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <geometry/core/cs.hpp>

#include <geometry/core/concepts/point_concept.hpp>

#include <geometry/strategies/strategies.hpp>


/*!
\defgroup length length calculation
The length algorithm is implemented for the linestring and the multi_linestring geometry and results
in the length of the linestring. If the points of a linestring have coordinates expressed in kilometers,
the length of the line is expressed in kilometers as well.
\par Example:
Example showing length calculation
\dontinclude doxygen_examples.cpp
\skip example_length_linestring_iterators1
\line {
\until }
*/

namespace geometry
{
	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		namespace length
		{

			/*!
				\brief Internal, calculates length of a linestring using iterator pairs and specified strategy
				\note for_each could be used here, now that point_type is changed by boost range iterator
			*/
			template<typename R, typename S>
			inline double length_range(const R& range, const S& strategy)
			{
				double sum = 0.0;

				typedef typename boost::range_const_iterator<R>::type IT;
				IT it = boost::begin(range);
				if (it != boost::end(range))
				{
					IT previous = it++;
					while(it != boost::end(range))
					{
						// Add point-point distance using the return type belonging to strategy
						sum += strategy(*previous, *it);
						previous = it++;
					}
				}
				return sum;
			}


		}
	} // namespace impl
	#endif



	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{
		template <typename TAG, typename G>
		struct length
		{
		};

		// Partial specializations
		template <typename G>
		struct length<linestring_tag, G>
		{

			inline static double calculate(const G& range)
			{
				typedef typename boost::range_value<G>::type P;
				typedef typename cs_tag<P>::type TAG;
				return impl::length::length_range(range,
					typename strategy_distance<TAG, TAG, P, P>::type());
			}

			template<typename S>
			inline static double calculate(const G& range, const S& strategy)
			{
				return impl::length::length_range(range, strategy);
			}
		};


	} // namespace dispatch
	#endif


	/*!
		\brief Calculate length of a geometry
		\ingroup length
		\details The function length returns the length of a geometry, using the default distance-calculation-strategy
		\param geometry the geometry, being a geometry::linestring, vector, iterator pair, or any other boost compatible range
		\return the length
		Example showing length calculation on a vector
		\dontinclude doxygen_examples.cpp
		\skip example_length_linestring_iterators2
		\line {
		\until }
	 */
	template<typename G>
	inline double length(const G& geometry)
	{
		return dispatch::length<typename tag<G>::type, G>::calculate(geometry);
	}

	/*!
		\brief Calculate length of a geometry
		\ingroup length
		\details The function length returns the length of a geometry, using specified strategy
		\param geometry the geometry, being a geometry::linestring, vector, iterator pair, or any other boost compatible range
		\param strategy strategy to be used for distance calculations.
		\return the length
		\par Example:
		Example showing length calculation using iterators and the Vincenty strategy
		\dontinclude doxygen_examples.cpp
		\skip example_length_linestring_iterators3
		\line {
		\until }
	 */
	template<typename G, typename S>
	inline double length(const G& geometry, const S& strategy)
	{
		return dispatch::length<typename tag<G>::type, G>::calculate(geometry, strategy);
	}


} // namespace geometry


#endif // _GEOMETRY_LENGTH_HPP
