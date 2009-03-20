// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_FROMWKT_HPP
#define _GEOMETRY_FROMWKT_HPP

#include <string>


#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/concept/requires.hpp>

#include <geometry/core/access.hpp>
#include <geometry/core/exterior_ring.hpp>
#include <geometry/core/interior_rings.hpp>

#include <geometry/core/concepts/point_concept.hpp>

#include <geometry/algorithms/clear.hpp>

namespace geometry
{
	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		namespace wkt
		{
			// (wkt: Well Known Text, defined by OGC for all geometries and implemented by e.g. databases (MySQL, PostgreSQL))

			typedef boost::tokenizer<boost::char_separator<char> > TOK;

			namespace impl
			{
				template <typename P, int I, int N>
				struct parsing_assigner
				{
					static void run(TOK::iterator& it, TOK::iterator end, P& point)
					{
						typedef typename coordinate_type<P>::type ctype;

						// Stop at end of tokens, or at "," ot ")"
						bool finished = (it == end || *it == "," || *it == ")");

						set<I>(point,
							finished ?
							// Initialize missing coordinates to default constructor (zero)
							ctype():
							// Use lexical_cast for conversion to double/int
							// Note that it is much slower than atof. However, it is more standard
							// and in parsing the change in performance falls probably away against
							// the tokenizing
							boost::lexical_cast<ctype>(it->c_str())
						);

						parsing_assigner<P, I+1, N>::run(finished ? it : ++it, end, point);
					}
				};

				template <typename P, int N>
				struct parsing_assigner<P, N, N>
				{
					static void run(TOK::iterator&, TOK::iterator, P&) {}
				};
			}

			/*!
				\brief Internal, parses coordinate sequences, strings are formated like "(1 2,3 4,...)"
				\param it token-iterator, should be pre-positioned at "(", is post-positions after last ")"
				\param end end-token-iterator
				\param out Output itererator receiving coordinates
				\return true if string starts with "(" and ends with ")" and all coordinates are parsed
			*/
			template <typename P, typename O_IT>
			inline bool parse_container(TOK::iterator& it, TOK::iterator end, O_IT out)
			{
				// Start with "("
				if (it == end || *it != "(")
				{
					return false;
				}
				it++;

				// Stop at ")"
				while (it != end && *it != ")")
				{
					P point;
					impl::parsing_assigner<P, 0, dimension<P>::value>::run(it, end, point);
					out = point;
					out++;
					if (it != end && *it == ",")
					{
						it++;
					}
				}
				// Should end with ")"
				if (it != end && *it == ")")
				{
					it++;
					return true;
				}
				return false;
			}

			/*!
				\brief Internal, starts parsing
				\param tokens boost tokens, parsed with separator " " and keeping separator "()"
				\param geometry string to compare with first token
				\return iterator put after geometry and/or optional m, z modifiers
			*/
			inline TOK::iterator parse_begin(const TOK& tokens, const std::string& geometry)
			{
				TOK::iterator it = tokens.begin();
				if (it != tokens.end() && boost::iequals(*it++, geometry))
				{
					while (it != tokens.end() && (boost::iequals(*it, "M")
								|| boost::iequals(*it, "Z")
								|| boost::iequals(*it, "MZ")
								|| boost::iequals(*it, "ZM")))
					{
						it++;
					}
				}
				return it;
			}

			/*!
				\brief Parses point from \ref WKT
				\param wkt string containing Well-Known Text
				\param point point receiving coordinates, if string has less than point, rest is initialized to zero
				\return true if string starts with "POINT(". It is forgiving for last ")"
				\note It is case insensitive and can have the WKT forms "point", "point m", "point z", "point zm", "point mz"
			*/
			template <typename P>
			inline
			BOOST_CONCEPT_REQUIRES(((Point<P>)),
			(bool))
			parse_point(const std::string& wkt, P& point)
			{
				// Break it apart into "POINT" "(" c1 c2 c3 ... ")"
				// WKT Coordinate string is space separated. Points in a sequence are comma separated
				// Token iterator is thus created with " ", ",()"

				TOK tokens(wkt, boost::char_separator<char>(" ", "()"));
				TOK::iterator it = parse_begin(tokens, "point");
				if (it != tokens.end() && *it == "(")
				{
					it++;
				}
				impl::parsing_assigner<P, 0, dimension<P>::value>::run(it, tokens.end(), point);
				return true;
			}



			/*!
				\brief Parses linestring from \ref WKT
				\param wkt string containing Well-Known Text
				\param out appending/inserting iterator which will receive the parsed coordinates
				\return true if string starts with "LINESTRING(" (case-insensitive) and coordinate-string can be parsed
			*/
			template <typename P, typename O_IT>
			inline bool parse_linestring(const std::string& wkt, O_IT out)
			{
				// Break it apart into "LINESTRING" "(" point(s) ")"
				TOK tokens(wkt, boost::char_separator<char>(" ", ",()"));
				TOK::iterator it = parse_begin(tokens, "linestring");
				return parse_container<P>(it, tokens.end(), out);
			}



			/*!
				\brief Parses polygon from \ref WKT
				\param wkt string containing Well-Known Text
				\param poly polygon which will be cleared and set to the specified coordinates
				\return true if string starts with "POLYGON(" (case-insensitive) and rings can be parsed
			*/
			template <typename Y>
			inline bool parse_polygon(const std::string& wkt, Y& poly)
			{
				poly.clear();

				TOK tokens(wkt, boost::char_separator<char>(" ", ",()"));
				TOK::iterator it = parse_begin(tokens, "polygon");
				// Polygon string begin
				if (it != tokens.end() && *it++ == "(")
				{
					int n = -1;
					// For each ring
					while (it != tokens.end())
					{
						if (++n == 0)
						{
							parse_container<typename point_type<Y>::type>(it, tokens.end(), std::back_inserter(exterior_ring(poly)));
						}
						else
						{
							interior_rings(poly).resize(n);
							parse_container<typename point_type<Y>::type>(it, tokens.end(), std::back_inserter(interior_rings(poly).back()));
						}
						if (it != tokens.end())
						{
							// Skip "," or ")" after container is parsed
							it++;
						}
					}
					return true;
				}
				return false;
			}


		} // namespace wkt
	} // namespace impl
	#endif

	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{
		template <typename TAG, typename G>
		struct from_wkt
		{
		};

		// Partial specializations

		/*!
			\brief Build a point from OGC Well-Known Text (\ref WKT)
			\ingroup wkt
			\param wkt string containing \ref WKT
			\param p point which will be cleared and set to the specified point
			\return true if \ref WKT can be parsed
		 */
		template <typename G>
		struct from_wkt<point_tag, G>
		{
			inline static bool parse(const std::string& wkt, G& geometry)
			{
				return impl::wkt::parse_point(wkt, geometry);
			}
		};


		/*!
			\brief Build a linestring from OGC Well-Known Text (\ref WKT)
			\ingroup wkt
			\param wkt string containing \ref WKT
			\param line linestring which will be cleared and set to the specified points
			\return true if \ref WKT can be parsed
		 */
		template <typename L>
		struct from_wkt<linestring_tag, L>
		{
			inline static bool parse(const std::string& wkt, L& linestring)
			{
				geometry::clear(linestring);
				typedef typename boost::range_value<L>::type P;
				return impl::wkt::parse_linestring<P>(wkt, std::back_inserter(linestring));
			}
		};

		/*!
			\brief Build a polygon from OGC Well-Known Text (\ref WKT)
			\ingroup wkt
			\param wkt string containing \ref WKT
			\param poly polygon which will be cleared and set to the specified polygon
			\return true if \ref WKT can be parsed to the polygon (starts with "POLYGON", has valid rings and coordinates)
		 */
		template <typename G>
		struct from_wkt<polygon_tag, G>
		{
			inline static bool parse(const std::string& wkt, G& geometry)
			{
				return impl::wkt::parse_polygon(wkt, geometry);
			}
		};

	} // namespace dispatch
	#endif


	/*!
		\brief Parses OGC Well-Known Text (\ref WKT) into a geometry (any geometry)
		\ingroup wkt
		\param wkt string containing \ref WKT
		\param geometry output geometry
		\return true if \ref WKT can be parsed
		\par Example:
		Small example showing how to use from_wkt to build a point
		\dontinclude doxygen_examples.cpp
		\skip example_from_wkt_point
		\line {
		\until }
		\par Example:
		Small example showing how to use from_wkt to build a linestring
		\dontinclude doxygen_examples.cpp
		\skip example_from_wkt_linestring
		\line {
		\until }
		\par Example:
		Small example showing how to use from_wkt to build a polygon
		\dontinclude doxygen_examples.cpp
		\skip example_from_wkt_polygon
		\line {
		\until }
	 */
	template <typename G>
	inline bool from_wkt(const std::string& wkt, G& geometry)
	{
		return dispatch::from_wkt<typename tag<G>::type, G>::parse(wkt, geometry);
	}


	/*!
		\brief Parses OGC Well-Known Text (\ref WKT) and outputs using an output iterator
		\ingroup wkt
		\param wkt string containing \ref WKT
		\param out output iterator
		\return true if \ref WKT can be parsed
		\note Because the output iterator doesn't always have the type value_type, it should be
		specified in the function call.
		\par Example:
		Small example showing how to use from_wkt with an output iterator
		\dontinclude doxygen_examples.cpp
		\skip example_from_wkt_output_iterator
		\line {
		\until }
	 */
	template <typename P, typename O_IT>
	inline bool from_wkt(const std::string& wkt, O_IT out)
	{
		// Todo: maybe take this from the string, or do not call parse_begin, such that
		// any coordinate string is parsed and outputted
		const std::string& tag = "linestring";

		impl::wkt::TOK tokens(wkt, boost::char_separator<char>(" ", ",()"));
		impl::wkt::TOK::iterator it = impl::wkt::parse_begin(tokens, tag);
		return impl::wkt::parse_container<P>(it, tokens.end(), out);
	}

}

#endif // _GEOMETRY_FROMWKT_HPP
