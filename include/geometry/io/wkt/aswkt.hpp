// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_ASWKT_HPP
#define _GEOMETRY_ASWKT_HPP

#include <iostream>
#include <string>

#include <boost/concept/assert.hpp>


#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <geometry/core/concepts/point_concept.hpp>
#include <geometry/core/ring_type.hpp>
#include <geometry/core/exterior_ring.hpp>
#include <geometry/core/interior_rings.hpp>

#include <geometry/geometries/linear_ring.hpp>
#include <geometry/algorithms/convert.hpp>



/*!
\defgroup wkt wkt: parse and stream WKT (Well-Known Text)
The wkt classes stream the specified geometry as \ref OGC Well Known Text (\ref WKT). It is defined for OGC geometries.
It is therefore not defined for all geometries (e.g. not for circle)
\note The implementation is independant from point type, point_xy and point_ll are supported,
as well as points with more than two coordinates.

*/


namespace geometry
{
	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		namespace wkt
		{
			template <typename P, int I, int Count>
			struct dump_coordinate
			{
				template <typename CH, typename TR>
				inline static void dump(std::basic_ostream<CH, TR>& os, const P& p)
				{
					os << (I > 0 ? " " : "") << get<I>(p);
					dump_coordinate<P, I + 1, Count>::dump(os, p);
				}
			};

			template <typename P, int Count>
			struct dump_coordinate<P, Count, Count>
			{
				template <typename CH, typename TR>
				inline static void dump(std::basic_ostream<CH, TR>&, const P&)
				{}
			};



			/*!
				\brief Stream points as \ref WKT
			*/
			template <typename P>
			struct wkt_point
			{
				template <typename CH, typename TR>
				inline static void stream(std::basic_ostream<CH, TR>& os, const P& p)
				{
					os << "POINT(";
					dump_coordinate<P, 0, dimension<P>::value>::dump(os, p);
					os << ")";
				}

				private :
					BOOST_CONCEPT_ASSERT( (ConstPoint<P>) );
			};


			/*!
				\brief Stream ranges as WKT
				\note CRTP is used to stream prefix/postfix, enabling derived classes to override this
			*/
			template <typename R, typename C>
			struct wkt_range
			{
				template <typename CH, typename TR>
				inline static void stream(std::basic_ostream<CH, TR>& os, const R& range)
				{
					os << C::prefix();

					bool first = true;
					typedef typename boost::range_const_iterator<R>::type IT;
					for (IT it = boost::begin(range); it != boost::end(range); it++)
					{
						os << (first ? "" : ",");
						dump_coordinate<P, 0, dimension<P>::value>::dump(os, *it);
						first = false;
					}

					os << C::postfix();
				}

				private :
					typedef typename boost::range_value<R>::type P;
					BOOST_CONCEPT_ASSERT( (ConstPoint<P>) );
			};

			template <typename R>
			struct wkt_poly_ring : wkt_range<R, wkt_poly_ring<R> >
			{
				inline static const char* prefix() { return "("; }
				inline static const char* postfix() { return ")"; }
			};

			template <typename P>
			struct wkt_poly
			{
				template <typename CH, typename TR>
				inline static void stream(std::basic_ostream<CH, TR>& os, const P& poly)
				{
					os << "POLYGON(";

					typedef typename ring_type<P>::type R;
					wkt_poly_ring<R>::stream(os, exterior_ring(poly));

					typedef typename boost::range_const_iterator<typename interior_type<P>::type>::type IT;
					for (IT it = boost::begin(interior_rings(poly)); it != boost::end(interior_rings(poly)); it++)
					{
						os << ",";
						wkt_poly_ring<R>::stream(os, *it);
					}

					os << ")";
				}

				private :
					BOOST_CONCEPT_ASSERT( (ConstPoint<typename point_type<P>::type>) );
			};

			template <typename B>
			struct wkt_box
			{
				typedef typename point_type<B>::type P;
				template <typename CH, typename TR>
				inline static void stream(std::basic_ostream<CH, TR>& os, const B& box)
				{
					// Convert to linear ring, then stream
					typedef linear_ring<P> R;
					R ring;
					geometry::convert(box, ring);
					os << "POLYGON(";
					wkt_poly_ring<R>::stream(os, ring);
					os << ")";
				}

				private :
					BOOST_CONCEPT_ASSERT( (ConstPoint<P>) );

					inline wkt_box()
					{
						// Only streaming of boxes with two dimensions is support, otherwise it is a polyhedron!
						//assert_dimension<B, 2>();
					}
			};


		} // namespace wkt
	} // namespace impl
	#endif


	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{

		/*!
			\brief Dispatching base struct for WKT streaming, specialized below per geometry type
			\details Specializations should implement a static method "stream" to stream a geometry
			The static method should have the signatur:

			template <typename CH, typename TR>
			inline static void stream(std::basic_ostream<CH, TR>& os, const G& geometry)
		*/
		template <typename T, typename G>
		struct wkt {};


		/*!
			\brief Specialization to stream point as WKT
		*/
		template <typename P>
		struct wkt<point_tag, P> : public impl::wkt::wkt_point<P> {};


		/*!
			\brief Specialization to stream linestring as WKT
		*/
		template <typename R>
		struct wkt<linestring_tag, R> : public impl::wkt::wkt_range<R, wkt<linestring_tag, R> >
		{
			inline static const char* prefix() { return "LINESTRING("; }
			inline static const char* postfix() { return ")"; }
		};

		/*!
			\brief Specialization to stream a ring as WKT
			\details A "linear_ring" does not exist in WKT.
			A linear ring is equivalent to a polygon without inner rings
			It is therefore streamed as a polygon
		*/
		template <typename B>
		struct wkt<box_tag, B> : public impl::wkt::wkt_box<B> {};

		/*!
			\brief Specialization to stream a ring as WKT
			\details A "linear_ring" does not exist in WKT.
			A linear ring is equivalent to a polygon without inner rings
			It is therefore streamed as a polygon
		*/
		template <typename R>
		struct wkt<ring_tag, R> : public impl::wkt::wkt_range<R, wkt<ring_tag, R> >
		{
			// Note, double parentheses are intentional, indicating WKT ring begin/end
			inline static const char* prefix() { return "POLYGON(("; }
			inline static const char* postfix() { return "))"; }
		};

		/*!
			\brief Specialization to stream polygon as WKT
		*/
		template <typename P>
		struct wkt<polygon_tag, P> : public impl::wkt::wkt_poly<P> {};
	} // namespace dispatch
	#endif


	/*!
		\brief Generic geometry template manipulator class, takes corresponding output class from traits class
		\ingroup wkt
		\details Stream manipulator, streams geometry classes as \ref WKT streams
		\par Example:
		Small example showing how to use the wkt class
		\dontinclude doxygen_examples.cpp
		\skip example_as_wkt_point
		\line {
		\until }
		\note the template parameter must be specified. If that is inconvient, users might use streamwkt
		which streams geometries as manipulators, or the object generator make_wkt
	*/
	template <typename G>
	class wkt
	{
		public :
			inline wkt(const G& g) : m_geometry(g) {}

			template <typename CH, typename TR>
			inline friend std::basic_ostream<CH, TR>& operator<<(std::basic_ostream<CH, TR>& os, const wkt& m)
			{
				dispatch::wkt<typename tag<G>::type, G>::stream(os, m.m_geometry);
				os.flush();
				return os;
			}

		private :
			const G& m_geometry;
	};


	/*!
		\brief Object generator to conveniently stream objects without including streamwkt
		\ingroup wkt
		\par Example:
		Small example showing how to use the make_wkt helper function
		\dontinclude doxygen_examples.cpp
		\skip example_as_wkt_vector
		\line {
		\until }
	*/
	template <typename T>
	inline wkt<T> make_wkt(const T& t)
	{
		return wkt<T>(t);
	}



}

#endif // _GEOMETRY_ASWKT_HPP
