// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_INTERSECTION_POLYGON_HPP
#define _GEOMETRY_INTERSECTION_POLYGON_HPP

#include <geometry/core/access.hpp>
#include <geometry/core/cs.hpp>

#include <geometry/core/exterior_ring.hpp>
#include <geometry/core/interior_rings.hpp>

#include <geometry/core/concepts/point_concept.hpp>
#include <geometry/core/concepts/point_concept.hpp>

#include <geometry/algorithms/area.hpp>
#include <geometry/algorithms/append.hpp>
#include <geometry/algorithms/clear.hpp>
#include <geometry/algorithms/distance.hpp>
#include <geometry/algorithms/within.hpp>



#include <geometry/algorithms/intersection_segment.hpp>

#include <geometry/algorithms/make.hpp>
#include <geometry/util/assign_box_corner.hpp>

#include <geometry/strategies/strategies.hpp>

// Helper geometries
#include <geometry/geometries/point_xy.hpp>
#include <geometry/geometries/segment.hpp>
#include <geometry/geometries/linear_ring.hpp>


#include <exception>


//#define GL_DEBUG_INTERSECT_POLYGON


namespace geometry
{
	// Move elsewhere?
    class intersection_exception : public std::exception
    {
		public:
			intersection_exception(const std::string& text)
				: m_text(text)
			{}
			virtual ~intersection_exception() throw()
			{}

			virtual const char *what() const throw()
			{
				return m_text.c_str();
			}
		private :
			std::string m_text;
    };




	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		namespace intersection
		{


			//------------------------------------------------------------------------------------------------------------------------
			// Weiler-Atherton approach to clip a polygon within a rectangle
			// Can be relatively easily extended to clip polygon-polygon
			// (Somewhat combined with "segment-code" idea of Cohen-Sutherland to see which segment needs clipping)

			// Keep bitwise.
			// See, e.g., http://www.sunshine2k.de/stuff/Java/CohenSutherland/CohenSutherland.html
			const char cohen_sutherland_top    = 1; // 0001
			const char cohen_sutherland_bottom = 2; // 0010
			const char cohen_sutherland_right  = 4; // 0100
			const char cohen_sutherland_left   = 8; // 1000


			const int ring_not_assigned = -99;


			enum clip_polygon_type {clip_intersect, clip_box_within_ring,
				clip_ring_within_box, clip_disjoint};



			// Extend point type with "tag" information
			// Weiler-Atherton:
			// "The new vertices are tagged to indicate they are intersection vertices."  --> intersection_id > 0
			// and
			// "Repeat steps "b" to "e" until the starting point has been reached." --> is_visited
			template<typename P>
			struct weiler_atherton_point : public P
			{
				typedef typename distance_result<P, P>::type DIST;

				public :
					inline weiler_atherton_point()
						: outside(false)
						, ring_index(ring_not_assigned)
						, intersection_id(-1)
						, is_visited(false)
					{
						// Init with zero
						this->template set<0>(0);
						this->template set<1>(0);
					}

					// Constructor with point
					inline weiler_atherton_point(const P& p)
						: P(p)
						, outside(false)
						, ring_index(ring_not_assigned)
						, intersection_id(-1)
						, vertex_index(-1)
						, distance(0)
						, is_visited(false)
					{}

					// Constructor with coordinates
					// Note that "geometry::coordinate_type" cannot be used here, because traits definitions
					// are declared below.
					template <typename T>
					inline weiler_atherton_point(const T& x, const T& y)
						: outside(false)
						, ring_index(ring_not_assigned)
						, intersection_id(-1)
						, vertex_index(-1)
						, distance(0)
						, is_visited(false)
					{
						this->template set<0>(x);
						this->template set<1>(y);
					}

					// Constructor with point, Cohen-Sutherland code and linear_ring-index
					inline weiler_atherton_point(const P& p, bool os, int r)
						: P(p)
						, outside(os)
						, ring_index(r)
						, intersection_id(-1)
						, vertex_index(-1)
						, distance(0)
						, is_visited(false)
					{}

					inline weiler_atherton_point(const P& p, int si, double d)
						: P(p)
						, outside(false)
						, ring_index(ring_not_assigned)
						, intersection_id(-1)
						, vertex_index(si)
						, distance(d)
						, is_visited(false)
					{}


					// Operator to sort on "start vertex", then on distance
					inline bool operator<(const weiler_atherton_point& other) const
					{
						if (other.vertex_index == this->vertex_index)
						{
							// Compare on (possibly squared) distance
							return this->distance < other.distance;
						}
						return this->vertex_index < other.vertex_index;
					}

					int ring_index;
					int vertex_index;
					bool outside;

					bool is_visited;

					// For intersections:
					int intersection_id; // 1-based intersection ID
					DIST distance; // distance to corresponding startpoing of segment

			};


		}
	} // namespace impl (closing to adapt to traits)


	// NOTE: the DOXYGEN_NO_IMPL still applies here


	// Adapt the weiler atherton point to the concept
	namespace traits
	{
		template <typename P>
		struct coordinate_type<geometry::impl::intersection::weiler_atherton_point<P> >
		{ typedef typename geometry::coordinate_type<P>::type type; };

		template <typename P>
		struct coordinate_system<geometry::impl::intersection::weiler_atherton_point<P> >
		{ typedef cs::cartesian type; };

		template <typename P>
		struct dimension<geometry::impl::intersection::weiler_atherton_point<P> >: boost::mpl::int_<2> {};

		template <typename P>
		struct tag<geometry::impl::intersection::weiler_atherton_point<P> >
		{ typedef point_tag type; };

		template <typename P>
		struct access<geometry::impl::intersection::weiler_atherton_point<P> >
		{
			template <int I>
			static typename coordinate_type<P>::type get(const geometry::impl::intersection::weiler_atherton_point<P>& p)
			{ return p.template get<I>(); }

			template <int I>
			static void set(geometry::impl::intersection::weiler_atherton_point<P>& p, const typename coordinate_type<P>::type& value)
			{ p.template set<I>(value); }
		};

	}

	// Continuation of namespace impl
	namespace impl
	{
		namespace intersection
		{

			// Small structure to keep two indices and an ID
			struct intersection_indices
			{
				inline intersection_indices(int an_id)
					: id(an_id)
					, subject_index(-1)
					, clip_index(-1)
				{}

				int id;
				int subject_index;
				int clip_index;
			};
			typedef std::vector<intersection_indices> intersection_index_vector;


#ifdef GL_DEBUG_INTERSECT_POLYGON
			template<typename P>
			inline std::ostream& operator<< (std::ostream &stream, const weiler_atherton_point<P>& p)
			{
				stream << "(" << std::setw(4) << std::setprecision(3) << get<0>(p)
					<< " , " << std::setw(4) << std::setprecision(3) << get<1>(p) << ") "
					<< "; ri=";
				if (p.ring_index == ring_not_assigned)
				{
					stream << "  -";
				}
				else
				{
					stream << std::setw(3) << p.ring_index;
				}
				stream << "; vi=" << std::setw(3) << p.vertex_index;
				stream << "; id=" << std::setw(3) << p.intersection_id;
				stream << "; dist=" << std::setw(4) << std::setprecision(2) << double(p.distance);
				if (p.outside)
				{
					stream << "; outside";
				}
				if (p.is_visited)
				{
					stream << "; visited";
				}
				return stream;
			}
#endif


			template<typename B, typename P>
			inline void _cs_clip_code(const B& b, const P& p,
				char& code, bool& is_inside,
				char& code_and, char& code_or, bool& has_inside, bool& disjoint)
			{
				code = 0;

				// Note: border has to be included because of boundary cases
				// Therefore we need a second variable to see if it is inside (including on boundary)

				// Compare left/right.
				if (get<0>(p) <= get<min_corner, 0>(b)) code = cohen_sutherland_left;
				else if (get<0>(p) >= get<max_corner, 0>(b)) code = cohen_sutherland_right;

				// Compare top/bottom
				if (get<1>(p) <= get<min_corner, 1>(b)) code |= cohen_sutherland_bottom;
				else if (get<1>(p) >= get<max_corner, 1>(b)) code |= cohen_sutherland_top;

				code_and &= code;
				code_or  |= code;

				// Check if point is lying inside clip, or on boundary
				is_inside = geometry::within(p, b);
				if (is_inside)
				{
					has_inside = true;
					disjoint = false;
				}
			}


			template<typename I>
			inline bool on_same_ring(I& it, int ring_index)
			{
				return ring_index == ring_not_assigned
									|| it->ring_index == ring_not_assigned
									|| it->ring_index == ring_index;
			}

			// Iterate along the container. If on end, or on other ring, goto first element of same ring
			template<typename I>
			inline void traverse_along(I& it, const I& begin, const I& end)
			{
				int ri_1 = it->ring_index;
				it++;
				if (it == end || ! on_same_ring(it, ri_1))
				{
#ifdef GL_DEBUG_INTERSECT_POLYGON
					if (it == end) std::cout << "GOTO BEGIN " << *begin << std::endl;
					else std::cout << "FIND RING  " << "RI:" << ri_1 << std::endl;
#endif
					it = begin;
					if (! on_same_ring(it, ri_1))
					{
						for (I tit = begin; tit != end; tit++)
						{
							//std::cout << "evaluate   " << *tit << " RI:" << ri_1 << std::endl;
							if (tit->ring_index == ri_1)
							{
#ifdef GL_DEBUG_INTERSECT_POLYGON
								std::cout << "TAKE       " << *tit << std::endl;
#endif

								it = tit;
								return;
							}
						}

						throw intersection_exception("internal error - no correct point on interator found");
					}
				}
			}

			// Traverse to an unvisited intersection point on the subject polygon.
			template<typename W>
			void traverse_to_intersection(W& subject, W& clip, intersection_index_vector& indices, typename W::iterator& it)
			{
				for (intersection_index_vector::iterator iit = indices.begin(); iit != indices.end(); iit++)
				{
					typename W::iterator next = subject.begin() + iit->subject_index;
					// Check both for being visited - we don't know which one was taken
					if (! next->is_visited && ! clip[iit->clip_index].is_visited)
					{
						it = next;
#ifdef GL_DEBUG_INTERSECT_POLYGON
						std::cout << "Start with " << *it << std::endl;
#endif
						return;
					}
				}
			}

			template<typename W>
			void traverse_to_after_id(W& subject, W& clip,
						intersection_index_vector& indices, bool on_subject, int id, typename W::iterator& it)
			{
				for (intersection_index_vector::iterator iit = indices.begin(); iit != indices.end(); iit++)
				{
					if (iit->id == id)
					{
						if (on_subject)
						{
							it = subject.begin() + iit->subject_index;
							traverse_along(it, subject.begin(), subject.end());
						}
						else
						{
							it = clip.begin() + iit->clip_index;
							traverse_along(it, clip.begin(), clip.end());
						}
						return;
					}
				}
				throw intersection_exception("internal error - intersection ID not found");
			}

			template<typename W>
			void traverse_next(W& subject, W& clip,
						intersection_index_vector& indices, bool& on_subject, typename W::iterator& it)
			{
				// 1. Go to next point
				typename W::iterator next = it;
				if (on_subject)
				{
					traverse_along(next, subject.begin(), subject.end());
				}
				else
				{
					traverse_along(next, clip.begin(), clip.end());
				}

				// 2. If current was not an intersection point,
				//    or next is an intersection point, we take this next point (but only on subject)
				//    or we are on the subject polygon and next is inside the clip
				if (it->intersection_id <= 0
					|| (on_subject && next->intersection_id > 0)
					|| (on_subject && ! next->outside))
				{
					it = next;

#ifdef GL_DEBUG_INTERSECT_POLYGON
					if (it->intersection_id <= 0)
						std::cout << "Take  next " << *it << std::endl;
					else if (on_subject && next->intersection_id > 0) std::cout << "next (sub) " << *it << std::endl;
					else if (on_subject && ! next->outside) std::cout << "next (ins) " << *it << std::endl;
					else
						std::cout << "other      " << *it << std::endl;
#endif
				}
				else
				{
					// we are on subject or on clip, on intersection, next is not an intersection,
					// navigate to the corresponding ID on the other polygon
					traverse_to_after_id(subject, clip, indices, !on_subject, it->intersection_id, next);

					// It may never be outside polygon
					if (!on_subject && next->outside)
					{
#ifdef GL_DEBUG_INTERSECT_POLYGON
						std::cout << "return because of " << *next << std::endl;
#endif
						return;
					}

					on_subject = ! on_subject;
					it = next;

#ifdef GL_DEBUG_INTERSECT_POLYGON
					std::cout << "trav->" << (on_subject ? "subj " : "clip ") << *it << std::endl;
#endif
				}
			}


			template<typename S1, typename S2, typename WAP>
			void segment_weiler_atherton_intersect(
						const S1& subject, int subject_index, std::vector<WAP>& subject_points,
						const S2& clip, int clip_index, std::vector<WAP>& clip_points,
						int& id, intersection_index_vector& indices)
			{
				BOOST_CONCEPT_ASSERT( (Point<WAP>) );
				BOOST_CONCEPT_ASSERT( (ConstSegment<S1>) );
				BOOST_CONCEPT_ASSERT( (ConstSegment<S1>) );


				std::vector<WAP> ip;
				intersection_result r = intersection_segment<WAP>(subject, clip, std::back_inserter(ip));

				// If there are two intersection points (== overlap), discard:
				// 1) it can be in opposite direction, they share their border there
				//    but do not intersect -> return
				// 2) it can be in same direction, it is OK but the intersection points are calculated
				//    before/after overlap (see figure)

				if (ip.size() != 1)
				{
					return;
				}

#ifdef GL_DEBUG_INTERSECT_POLYGON
				std::cout << "   CHECK : " << subject.first << "-" << subject.second
						<< " x " << clip.first << "-" << clip.second << std::endl;
#endif


				// If segment of subject intersects clip, and intersection point is on the clip,
				// we examine further
				if (r.is_type == is_intersect)
				{
					connection_type c = r.get_connection_type();
					if (c != is_connect_no)
					{
#ifdef GL_DEBUG_INTERSECT_POLYGON
						std::cout << "CONNECT: " << subject.first << "-" << subject.second
							<< " x " << clip.first << "-" << clip.second << std::endl;
#endif

						//typedef typename cs_tag<typename S2::point_type>::type PTAG;// doesn't work... todo
						typedef typename strategy_side<cartesian_tag, typename S2::point_type>::type SIDE;


						switch(c)
						{
							// If it connects at at one end, always consider the subject-point:
							// only continue if it is RIGHT of the clip
							case is_connect_s1p1 :
								if (SIDE::side(clip, subject.second) >= 0) return;
								break;
							case is_connect_s1p2 :
								if (SIDE::side(clip, subject.second) >= 0) return;
								break;
							case is_connect_s2p1 :
							case is_connect_s2p2 :
								// Consider both points, at least one must be right of the clip
								if (SIDE::side(clip, subject.first) >= 0 && SIDE::side(clip, subject.second) >= 0)
								{
									return;
								}
								break;
						}
					}

				}

				// Add intersection points, if any
				for (typename std::vector<WAP>::iterator it = ip.begin(); it != ip.end(); it++)
				{
					// Tag the point(s) with unique id
					it->intersection_id = ++id;
					// Add it to intersection
					indices.push_back(intersection_indices(id));
#ifdef GL_DEBUG_INTERSECT_POLYGON
					std::cout << "-> INTERSECTION: " << id << " " << *it << std::endl;
#endif

					// Assign index, calc squared distance and store for addition to subject and to clip lateron
					typedef typename point_type<S1>::type PS1;
					typedef typename strategy_distance<
							typename cs_tag<WAP>::type,
							typename cs_tag<PS1>::type,
							WAP, PS1>::type STRATEGY;

					STRATEGY strategy;

					it->vertex_index = subject_index;
					it->distance = strategy(subject.first, *it);
					subject_points.push_back(*it);

					it->vertex_index = clip_index;
					it->distance = strategy(clip.first, *it);
					clip_points.push_back(*it);
				}
			}


			template<typename B, typename R, typename WAP, typename WAR>
			clip_polygon_type ring_weiler_atherton_intersect(const B& b, const R& in, int ring_index,
					WAR& subject,  std::vector<WAP>& clip, // todo, change to WAR
					int& id, intersection_index_vector& indices)
			{
				// Some book keeping variables
				char code_and = cohen_sutherland_left
						| cohen_sutherland_right
						| cohen_sutherland_top
						| cohen_sutherland_bottom;
				char code_or = 0;
				bool has_inside = false;
				bool is_disjoint = true;

				// Define the 4 points defining the box
				typedef typename boost::range_value<R>::type P;
				P lower_left, upper_left, lower_right, upper_right;
				assign_box_corners(b, lower_left, lower_right, upper_left, upper_right);

				bool first = true;
				char code_previous = 0;
				//R::const_iterator last = in.end() - 1;
#ifdef GL_DEBUG_INTERSECT_POLYGON
				int point_index = 0;
#endif

				size_t subject_original_size = subject.size();

				typename R::const_iterator it = in.begin();
				typename R::const_iterator prev = it++;
				while(it != in.end())
				{
					bool is_inside;
					if (first)
					{
						// Add first point of polygon to subject vector
						_cs_clip_code(b, *prev, code_previous, is_inside, code_and, code_or,
							has_inside, is_disjoint);
						subject.push_back(WAP(*prev, !is_inside, ring_index));
						first = false;
					}

					char code_current;
					_cs_clip_code(b, *it, code_current, is_inside, code_and, code_or,
							has_inside, is_disjoint);

					segment<const P> ss(*prev, *it);

#ifdef GL_DEBUG_INTERSECT_POLYGON
					std::cout << ring_index << "." << point_index++
						<< " SGMNT: " << ss.first << "-" << ss.second
						<< " cs " << int(code_previous) << "," << int(code_current) << std::endl;
#endif


					// If both segments are not in same Cohen-Suth. segment, check for intersections
					// Todo: check more efficient (if they are lying on same side -> no intersection possible)
					if (code_previous != code_current)
					{
						// Intersections are first stored in a vector, then sorted, then added, see Weiler-Atherton:
						// "If care is taken in placement of intersections where the subject and clip polygon
						//  contours are identical in the x-y plane, no degenerate polygons will be produced by
						//  the clipping process."
						std::vector<WAP> ips;

						// Clip all four sides of box
						segment_weiler_atherton_intersect(ss, -1, ips,
										segment<const P>(lower_left, upper_left),
										0, clip, id, indices);
						segment_weiler_atherton_intersect(ss, -1, ips,
										segment<const P>(upper_left, upper_right),
										1, clip, id, indices);
						segment_weiler_atherton_intersect(ss, -1, ips,
										segment<const P>(upper_right, lower_right),
										2, clip, id, indices);
						segment_weiler_atherton_intersect(ss, -1, ips,
										segment<const P>(lower_right, lower_left),
										3, clip, id, indices);

						// Add all found intersection points to subject polygon, after sorting
						// on distance from first point. There might be up to 4 intersection points
						// in rectangular clips, and much more on polygon clips. However, often there are zero or one
						// intersections, sorting is not a big issue here
						std::sort(ips.begin(), ips.end());
						for (typename std::vector<WAP>::const_iterator pit = ips.begin(); pit != ips.end(); pit++)
						{
							indices[pit->intersection_id - 1].subject_index = int(subject.size());
							subject.push_back(*pit);

							is_disjoint = false;
						}
					}

					// After intersections, add endpoint of segment to subject vector
					subject.push_back(WAP(*it, !is_inside, ring_index));

					code_previous = code_current;
					prev = it++;
				}

				// If all points are inside clip-box, output is original (for outer)
				// or should be added completely (for inner)
				if (code_or == 0)
				{
					// Remove from subject
					subject.erase(subject.begin() + subject_original_size, subject.end());
					return clip_ring_within_box;
				}

				// Special case: if no points are inside, the clip box might be inside the polygon
				// -> output clip box and quit
				if (! has_inside)
				{
					// Take any point of clip and check if it is within the linear_ring
					typename point_type<B>::type p;
					assign_box_corner<min_corner, min_corner>(b, p);
					if (geometry::within(p, in))
					{
						return clip_box_within_ring;
					}
					// If clip points are not in the linear_ring, but not all on same side,
					// there might still be an intersection
					// TODO: can be checked more efficiently further
				}


				// Discard disjoint outer/inner rings
				if (is_disjoint)
				{
					// Remove from subject
					subject.erase(subject.begin() + subject_original_size, subject.end());
					return clip_disjoint;
				}

				// Some points are inside, some are outside, clipping should go on
				return clip_intersect;
			}

#ifdef GL_DEBUG_INTERSECT_POLYGON
			template <typename RING>
			void wa_debug_ring(const std::string& title, const RING& ring)
			{
				std::cout << title << ": (" << ring.size() << " points)" << std::endl;
				for (int i = 0; i < ring.size(); i++)
				{
					std::cout << ring[i] << std::endl;
				}
			}
			void wa_debug_clip_result(const std::string& title, clip_polygon_type type)
			{
				std::cout << title << ": ";
				switch(type)
				{
					case clip_box_within_ring : std::cout << "clip within ring"; break;
					case clip_ring_within_box : std::cout << "ring within clip"; break;
					case clip_disjoint : std::cout << "clip disjoint ring"; break;
					case clip_intersect : std::cout << "clip intersects ring"; break;
				}
				std::cout << std::endl;

			}

#endif



			template<typename B, typename POLY, typename O_IT>
			O_IT poly_weiler_atherton(const B& b, const POLY& in, O_IT output)
			{
				typedef weiler_atherton_point<typename point_type<POLY>::type> wap;
				typedef linear_ring<wap> wa_ring;

				// Weiler-Atherton:
				// "A link is established between each pair of new vertices, permitting travel between two
				//  polygons wherever they intersect on the x-y plane."
				// We keep this link in an index-vector
				intersection_index_vector indices;

				int id = 0;

				// 1: Create a processed copy of the polygon (called, after Weiler-Atherton, the "subject"),
				//    which has all points of original AND all intersection points on correct positions.
				//    Create also a processed copy of the clipping box, called, the "clip", also with all found intersection points.
				//    The copy is
				//    Check the Cohen-Suth.segment
				//    - to avoid unnecessary intersections (in a BOX, two consecutive inside points doesn't need to be checked)
				//    - to choose the path, lateron
				wa_ring subject, clip;


				// For outerring and all rings
				// TODO: check result code of function, if no intersections -> add to output iterator and quit
				int ring_index = -1;

				clip_polygon_type outer_result =
					ring_weiler_atherton_intersect(b, exterior_ring(in), ring_index++, subject, clip, id, indices);
#ifdef GL_DEBUG_INTERSECT_POLYGON
				wa_debug_clip_result("exterior_ring", outer_result);
#endif

				switch(outer_result)
				{
					case clip_box_within_ring :
						{
							// If outer polygon completely contains the clipbox, return the clipbox as clip
							POLY out;
							typedef typename point_type<POLY>::type P;
							geometry::append(exterior_ring(out), make<P>(get<min_corner, 0>(b), get<min_corner, 1>(b)));
							geometry::append(exterior_ring(out), make<P>(get<min_corner, 0>(b), get<max_corner, 1>(b)));
							geometry::append(exterior_ring(out), make<P>(get<max_corner, 0>(b), get<max_corner, 1>(b)));
							geometry::append(exterior_ring(out), make<P>(get<max_corner, 0>(b), get<min_corner, 1>(b)));
							geometry::append(exterior_ring(out), make<P>(get<min_corner, 0>(b), get<min_corner, 1>(b)));

							// TODO: all innerrings must be evaluated:
							// - completely within clipbox -> add as innerring
							// - intersecting clipbox -> discard poly above and do complete process
							// - completely outside clipbox (but clipbox still inside exterior ring) -> return


							*output = out;
							output++;

							return output;
						}
					case clip_ring_within_box :
						// Put input polygon to output vector (this assignment includes all inner rings)
						*output = in;
						return output;
					case clip_disjoint :
						// Put nothing to output vector, it is completely outside clipping box
						return output;
				}

				std::vector<int> within_inner_rings;

				typedef typename boost::range_const_iterator<typename interior_type<POLY>::type>::type IIT;

				for (IIT it = boost::begin(interior_rings(in));
						it != boost::end(interior_rings(in)); it++, ring_index++)
				{
					clip_polygon_type inner_result =
						ring_weiler_atherton_intersect(b, *it, ring_index, subject, clip, id, indices);
#ifdef GL_DEBUG_INTERSECT_POLYGON
					wa_debug_clip_result("inner", inner_result);
#endif

					switch(inner_result)
					{
						case clip_ring_within_box :
							// if innerring completly inside box, add lateron as innerring
							within_inner_rings.push_back(ring_index);
							break;
					}
				}


				// If there are clip intersection points, build up the clip polyon. Add all corner points,
				// then sort on segment-index and distance, then the clip is OK
				typename point_type<B>::type lower_left, upper_left, lower_right, upper_right;
				assign_box_corners(b, lower_left, lower_right, upper_left, upper_right);

				clip.push_back(wap(lower_left, 0, 0.0));
				clip.push_back(wap(upper_left, 1, 0.0));
				clip.push_back(wap(upper_right, 2, 0.0));
				clip.push_back(wap(lower_right, 3, 0.0));
				clip.push_back(wap(lower_left, 4, 0.0));

				std::sort(clip.begin(), clip.end());

				// Update the id's of clip intersection points, now we have it
				for (unsigned int j = 0; j < clip.size(); j++)
				{
					if (clip[j].intersection_id > 0)
					{
						indices[clip[j].intersection_id - 1].clip_index = j;
					}
				}

#ifdef GL_DEBUG_INTERSECT_POLYGON
				std::cout << std::endl << "indices of intersections:" << std::endl;
				for (intersection_index_vector::const_iterator it = indices.begin(); it != indices.end(); it++)
				{
					std::cout
						<< "subj:" << it->subject_index << " " << subject[it->subject_index] << std::endl
						<< "clip:" << it->clip_index << " " << clip[it->clip_index] << std::endl;
				}

				wa_debug_ring("SUBJECT", subject);
				wa_debug_ring("CLIP", clip);

#endif

				// 4. build output polygon or polygons, start with an intersected point
				// "4. The actual clipping is now performed
				//     a) An intersection vertex is removed from the first intersection list to be used as a
				//        starting point. If the list is exhausted, the clipping is complete; Go to step 5.
				POLY out;
				typename wa_ring::iterator it = subject.begin();
				traverse_to_intersection(subject, clip, indices, it);
				bool on_subject = true;
				while(! it->is_visited)
				{
					// Add the point, but only if it differs from previous point
					typedef typename point_type<POLY>::type P;
					P p = make<P>(it->x(), it->y());
					if (boost::empty(exterior_ring(out)) || ! (exterior_ring(out).back() == p))
					{
						geometry::append(exterior_ring(out), p);
					}

					it->is_visited = true;

					traverse_next(subject, clip, indices, on_subject, it);

					if (it->is_visited)
					{
						// Close (if not closed)
						typename ring_type<POLY>::type outer_ring = exterior_ring(out);
						if (! boost::empty(outer_ring) && ! (outer_ring.front() == outer_ring.back()))
						{
							geometry::append(outer_ring, outer_ring.front());
						}

						// Add all innerrings which are inside this polygon
						for (unsigned int i = 0; i < within_inner_rings.size(); i++)
						{

							typedef typename boost::range_const_iterator<typename interior_type<POLY>::type>::type IIT;
							IIT it = boost::begin(interior_rings(in)) + within_inner_rings[i];
							// TODO: make more efficient, check using "envelope" of exterior first
							if (geometry::within(it->front(), outer_ring))
							{
								// ASSUMPTION OF STD::CONTAINER HERE which is probably always the case
								interior_rings(out).push_back(*it);
							}
						}


#ifdef GL_DEBUG_INTERSECT_POLYGON
						//std::cout << "out: " << make_as_wkt(out) << std::endl;
#endif

						*output = out;
						output++;

						clear(out);

						// Go to first unvisited intersection point, if any
						// Else the iterator will result is_visited and it will stop
						traverse_to_intersection(subject, clip, indices, it);
						on_subject = true;
					}
				}


#ifdef GL_DEBUG_INTERSECT_POLYGON
				std::cout << std::endl << "AFTER PROCESSING:" << std::endl;
				wa_debug_ring("SUBJECT", subject);
				wa_debug_ring("CLIP", clip);
#endif
				return output;

			}


		}
	} // namespace impl
	#endif


} // namespace


#endif //_GEOMETRY_INTERSECTION_POLYGON_HPP
