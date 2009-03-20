// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_FOREACH_HPP
#define _GEOMETRY_FOREACH_HPP

/*!
\defgroup loop loops and for-each functionality
There are several algorithms provided which walk through the points or segments
of linestrings and polygons. They are called for_each_point, for_each_segment, after
the standard library, and \b loop which is more adapted and of which the functor
could break out if necessary.
Of the for_each algorithms there is a \b const and a non-const version provided.
*/


#include <algorithm>

#include <boost/concept/requires.hpp>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <geometry/core/concepts/point_concept.hpp>
#include <geometry/core/exterior_ring.hpp>
#include <geometry/core/interior_rings.hpp>

#include <geometry/geometries/segment.hpp>

namespace geometry
{
	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		namespace for_each
		{

			template <typename P, typename F>
			struct fe_point
			{
				static inline F for_each_const_point(const P& p, F f)
				{
					f(p);
					return (f);
				}
				static inline F for_each_point(P& p, F f)
				{
					f(p);
					return (f);
				}
			};


			template <typename R, typename F>
			struct fe_range
			{
				static inline F for_each_const_point(const R& range, F f)
				{
					return (std::for_each(boost::begin(range), boost::end(range), f));
				}

				static inline F for_each_point(R& range, F f)
				{
					return (std::for_each(boost::begin(range), boost::end(range), f));
				}

				static inline F for_each_const_segment(const R& range, F f)
				{
					typedef typename boost::range_const_iterator<R>::type IT;
					IT it = boost::begin(range);
					IT previous = it++;
					while(it != boost::end(range))
					{
						segment<typename point_type<R>::type> s(*previous, *it);
						f(s);
						previous = it++;
					}

					return (f);
				}

				static inline F for_each_segment(R& range, F f)
				{
					typedef typename boost::range_iterator<R>::type IT;
					IT it = boost::begin(range);
					IT previous = it++;
					while(it != boost::end(range))
					{
						segment<typename point_type<R>::type> s(*previous, *it);
						f(s);
						previous = it++;
					}

					return (f);
				}
			};


			template <typename P, typename F>
			struct fe_polygon
			{
				static inline F for_each_const_point(const P& poly, F f)
				{
					typedef typename ring_type<P>::type R;
					typedef typename boost::range_const_iterator<typename interior_type<P>::type>::type IT;
					f = fe_range<R, F>::for_each_const_point(exterior_ring(poly), f);
					for (IT it = boost::begin(interior_rings(poly)); it != boost::end(interior_rings(poly)); it++)
					{
						f = fe_range<R, F>::for_each_const_point(*it, f);
					}
					return (f);
				}
				static inline F for_each_point(P& poly, F f)
				{
					typedef typename ring_type<P>::type R;
					typedef typename boost::range_iterator<typename interior_type<P>::type>::type IT;
					f = fe_range<R, F>::for_each_point(exterior_ring(poly), f);
					for (IT it = boost::begin(interior_rings(poly)); it != boost::end(interior_rings(poly)); it++)
					{
						f = fe_range<R, F>::for_each_point(*it, f);
					}
					return (f);
				}

				static inline F for_each_const_segment(const P& poly, F f)
				{
					typedef typename ring_type<P>::type R;
					typedef typename boost::range_const_iterator<typename interior_type<P>::type>::type IT;
					f = fe_range<R, F>::for_each_const_segment(exterior_ring(poly), f);
					for (IT it = boost::begin(interior_rings(poly)); it != boost::end(interior_rings(poly)); it++)
					{
						f = fe_range<R, F>::for_each_const_segment(*it, f);
					}
					return (f);
				}
				static inline F for_each_segment(P& poly, F f)
				{
					typedef typename ring_type<P>::type R;
					typedef typename boost::range_iterator<typename interior_type<P>::type>::type IT;
					f = fe_range<R, F>::for_each_segment(exterior_ring(poly), f);
					for (IT it = boost::begin(interior_rings(poly)); it != boost::end(interior_rings(poly)); it++)
					{
						f = fe_range<R, F>::for_each_segment(*it, f);
					}
					return (f);
				}
			};


		} // namespace for_each

	} // namespace impl
	#endif


	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{
		template <typename TAG, typename G, typename F>
		struct for_each {};

		template <typename P, typename F>
		struct for_each<point_tag, P, F> : impl::for_each::fe_point<P, F> {};

		template <typename L, typename F>
		struct for_each<linestring_tag, L, F> : impl::for_each::fe_range<L, F> {};

		template <typename R, typename F>
		struct for_each<ring_tag, R, F> : impl::for_each::fe_range<R, F> {};

		template <typename P, typename F>
		struct for_each<polygon_tag, P, F> : impl::for_each::fe_polygon<P, F> {};

	} // namespace dispatch
	#endif



	/*!
		\brief Calls functor for geometry
		\ingroup loop
		\param geometry geometry to loop through
		\param f functor to use
		\details Calls the functor the specified \b const geometry
	*/
	template<typename G, typename F>
	inline F for_each_point(const G& geometry, F f)
	{
		return (dispatch::for_each<typename tag<G>::type, G, F>::for_each_const_point(geometry, f));
	}

	/*!
		\brief Calls functor for geometry
		\ingroup loop
		\param geometry geometry to loop through
		\param f functor to use
		\details Calls the functor for the specified geometry
	*/
	template<typename G, typename F>
	inline F for_each_point(G& geometry, F f)
	{
		return (dispatch::for_each<typename tag<G>::type, G, F>::for_each_point(geometry, f));
	}

	/*!
		\brief Calls functor for segments on linestrings, rings, polygons, ...
		\ingroup loop
		\param geometry geometry to loop through
		\param f functor to use
		\details Calls the functor all \b const segments of the specified \b const geometry
	*/
	template<typename G, typename F>
	inline F for_each_segment(const G& geometry, F f)
	{
		return (dispatch::for_each<typename tag<G>::type, G, F>::for_each_const_segment(geometry, f));
	}


	/*!
		\brief Calls functor for segments on linestrings, rings, polygons, ...
		\ingroup loop
		\param geometry geometry to loop through
		\param f functor to use
		\details Calls the functor all segments of the specified geometry
	*/
	template<typename G, typename F>
	inline F for_each_segment(G& geometry, F f)
	{
		return (dispatch::for_each<typename tag<G>::type, G, F>::for_each_segment(geometry, f));
	}

}

#endif // _GEOMETRY_FOREACH_HPP
