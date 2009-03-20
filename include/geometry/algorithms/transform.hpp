// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_TRANSFORM_HPP
#define _GEOMETRY_TRANSFORM_HPP

#include <cmath>
#include <iterator>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <geometry/core/cs.hpp>
#include <geometry/core/ring_type.hpp>
#include <geometry/core/exterior_ring.hpp>
#include <geometry/core/interior_rings.hpp>

#include <geometry/geometries/segment.hpp>

#include <geometry/strategies/strategies.hpp>

#include <geometry/algorithms/clear.hpp>


/*!
\defgroup transform transformations
\brief Transforms from one geometry to another geometry, optionally using a strategy
\details The transform algorithm automatically transforms from one coordinate system to another coordinate system.
If the coordinate system of both geometries are the same, the geometry is copied. All point(s of the geometry)
are transformed.

There is a version without a strategy, transforming automatically, and there is a version with a strategy.

This function has a lot of appliances, for example
- transform from spherical coordinates to cartesian coordinates, and back
- transform from geographic coordinates to cartesian coordinates (projections) and back
- transform from degree to radian, and back
- transform from and to cartesian coordinates (mapping, translations, etc)

The automatic transformations look to the coordinate system family, and dimensions, of the point type and by this
apply the strategy (internally bounded by traits classes).

\par Examples:
The example below shows automatic transformations to go from one coordinate system to another one:
\dontinclude doxygen_2.cpp
\skip example_for_transform()
\skipline XYZ
\until endl;

The next example takes another approach and transforms from Cartesian to Cartesian:
\skipline XY
\until endl;
See also \link p03_projmap_example.cpp the projmap example \endlink
where this last one plus a transformation using a projection are used.

\note Not every possibility is yet worked out, e.g. polar coordinate system is ignored until now
\note This "transform" is broader then geodetic datum transformations, those are currently not worked out

*/

namespace geometry
{
	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		namespace transform
		{
			template <typename P1, typename P2, typename S>
			inline bool transform_point(const P1& p1, P2& p2, const S& strategy)
			{
				return strategy(p1, p2);
			}

			template <typename P_OUT, typename O_IT, typename R, typename S>
			inline bool transform_range_out(const R& range, O_IT out, const S& strategy)
			{
				P_OUT p_out;
				for(typename boost::range_const_iterator<R>::type it = boost::begin(range);
					it != boost::end(range); it++)
				{
					if (! transform_point(*it, p_out, strategy))
					{
						return false;
					}
					*out = p_out;
					out++;
				}
				return true;
			}

			template <typename P1, typename P2, typename S>
			inline bool transform_polygon(const P1& poly1, P2& poly2, const S& strategy)
			{
				typedef typename interior_type<P1>::type IR1;
				typedef typename interior_type<P2>::type IR2;
				typedef typename ring_type<P1>::type R1;
				typedef typename ring_type<P2>::type R2;

				typedef typename point_type<P2>::type POINT2;

				geometry::clear(poly2);

				if (! transform_range_out<POINT2>(exterior_ring(poly1), std::back_inserter(exterior_ring(poly2)), strategy))
				{
					return false;
				}

				interior_rings(poly2).resize(boost::size(interior_rings(poly1)));

				typename boost::range_const_iterator<IR1>::type it1 = boost::begin(interior_rings(poly1));
				typename boost::range_iterator<IR2>::type       it2 = boost::begin(interior_rings(poly2));
				for ( ; it1 != boost::end(interior_rings(poly1)); it1++, it2++)
				{
					if (! transform_range_out<POINT2>(*it1, std::back_inserter(*it2), strategy))
					{
						return false;
					}
				}
				return true;
			}


			template <typename P1, typename P2>
			struct select_strategy
			{
				typedef typename strategy_transform<
							typename cs_tag<P1>::type,
							typename cs_tag<P2>::type,
							typename coordinate_system<P1>::type,
							typename coordinate_system<P2>::type,
							dimension<P1>::value,
							dimension<P2>::value,
							P1, P2>::type type;
			};




			template <typename R1, typename R2>
			struct transform_range
			{
				template <typename S>
				inline static bool calculate(const R1& range1, R2& range2, const S& strategy)
				{
					typedef typename point_type<R2>::type P2;
					clear(range2);
					return transform_range_out<P2>(range1, std::back_inserter(range2), strategy);
				}

				inline static bool calculate(const R1& range1, R2& range2)
				{
					typename select_strategy<typename point_type<R1>::type,
									typename point_type<R2>::type>::type strategy;
					return calculate(range1, range2, strategy);
				}
			};

		} // namespace transform

	} // namespace impl
	#endif


	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{

		template <typename TAG1, typename TAG2, typename G1, typename G2>
		struct transform {};


		template <typename P1, typename P2>
		struct transform<point_tag, point_tag, P1, P2>
		{
			template <typename S>
			inline static bool calculate(const P1& p1, P2& p2, const S& strategy)
			{
				return impl::transform::transform_point(p1, p2, strategy);
			}

			inline static bool calculate(const P1& p1, P2& p2)
			{
				typename impl::transform::select_strategy<P1, P2>::type strategy;
				return calculate(p1, p2, strategy);
			}
		};


		template <typename L1, typename L2>
		struct transform<linestring_tag, linestring_tag, L1, L2> : impl::transform::transform_range<L1, L2> {};


		template <typename R1, typename R2>
		struct transform<ring_tag, ring_tag, R1, R2> : impl::transform::transform_range<R1, R2> {};

		template <typename P1, typename P2>
		struct transform<polygon_tag, polygon_tag, P1, P2>
		{
			template <typename S>
			inline static bool calculate(const P1& p1, P2& p2, const S& strategy)
			{
				return impl::transform::transform_polygon(p1, p2, strategy);
			}

			inline static bool calculate(const P1& p1, P2& p2)
			{
				typename impl::transform::select_strategy<
							typename point_type<P1>::type,
							typename point_type<P2>::type>::type strategy;
				return calculate(p1, p2, strategy);
			}
		};

	} // namespace dispatch
	#endif





	/*!
		\brief Transforms from one geometry to another geometry using a strategy
		\ingroup transform
		\tparam G1 first geometry type
		\tparam G2 second geometry type
		\tparam S strategy
		\param geometry1 first geometry
		\param geometry2 second geometry
		\param strategy the strategy to be used for transformation
	 */
	template <typename G1, typename G2, typename S>
	inline bool transform(const G1& geometry1, G2& geometry2, const S& strategy)
	{
		return dispatch::transform<typename tag<G1>::type,
				typename tag<G2>::type, G1, G2>::calculate(geometry1, geometry2, strategy);
	}

	/*!
		\brief Transforms from one geometry to another geometry using a strategy
		\ingroup transform
		\tparam G1 first geometry type
		\tparam G2 second geometry type
		\param geometry1 first geometry
		\param geometry2 second geometry
		\return true if the transformation could be done
	 */
	template <typename G1, typename G2>
	inline bool transform(const G1& geometry1, G2& geometry2)
	{
		return dispatch::transform<typename tag<G1>::type,
				typename tag<G2>::type, G1, G2>::calculate(geometry1, geometry2);
	}

} // namespace geometry


#endif // _GEOMETRY_TRANSFORM_HPP
