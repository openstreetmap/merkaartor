// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_SECTIONALIZE_HPP
#define GGL_ALGORITHMS_SECTIONALIZE_HPP

#include <cstddef>
#include <vector>

#include <boost/concept/requires.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/algorithms/assign.hpp>
#include <ggl/algorithms/combine.hpp>

#include <ggl/core/access.hpp>
#include <ggl/core/exterior_ring.hpp>

#include <ggl/iterators/point_const_iterator.hpp>

#include <ggl/util/assign_box_corner.hpp>
#include <ggl/geometries/segment.hpp>


/*!
\defgroup sectionalize sectionalize: split a geometry (polygon,linestring,etc) into monotonic sections

\par Geometries:
- LINESTRING:
- RING:
- POLYGON:
*/

namespace ggl
{

template <typename B, std::size_t N>
struct section
{
    typedef B box_type;

    int directions[N];
    int ring_index;
    int multi_index;
    B bounding_box;

    int begin_index;
    int end_index;
    int count; // might be not necessary

    inline section()
        : ring_index(-99)
        , multi_index(-99)
        , begin_index(-1)
        , end_index(-1)
        , count(0)
    {
        assign_inverse(bounding_box);
    }
};

template <typename B, std::size_t N>
struct sections : std::vector<section<B, N> >
{
    typedef B box_type;
    static const std::size_t value = N;
};

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace sectionalize {

template <typename Segment, std::size_t D, std::size_t N>
struct get_direction_loop
{
    typedef typename coordinate_type<Segment>::type coordinate_type;

    static inline void apply(Segment const& seg, int directions[N])
    {
        coordinate_type const diff = ggl::get<1, D>(seg) - ggl::get<0, D>(seg);
        directions[D] = diff > 0 ? 1 : (diff < 0 ? -1 : 0);

        get_direction_loop<Segment, D + 1, N>::apply(seg, directions);
    }
};

template <typename Segment, std::size_t N>
struct get_direction_loop<Segment, N, N>
{
    static inline void apply(Segment const& seg, int directions[N])
    {
        boost::ignore_unused_variable_warning(seg);
        boost::ignore_unused_variable_warning(directions);
    }
};

template <typename T, std::size_t D, std::size_t N>
struct copy_loop
{
    static inline void apply(const T source[N], T target[N])
    {
        target[D] = source[D];
        copy_loop<T, D + 1, N>::apply(source, target);
    }
};

template <typename T, std::size_t N>
struct copy_loop<T, N, N>
{
    static inline void apply(const T source[N], T target[N])
    {
        boost::ignore_unused_variable_warning(source);
        boost::ignore_unused_variable_warning(target);
    }
};

template <typename T, std::size_t D, std::size_t N>
struct compare_loop
{
    static inline bool apply(const T source[N], const T target[N])
    {
        bool const not_equal = target[D] != source[D];

        return not_equal ? false : compare_loop<T, D + 1, N>::apply(source, target);
    }
};

template <typename T, std::size_t N>
struct compare_loop<T, N, N>
{
    static inline bool apply(const T source[N], const T target[N])
    {
        boost::ignore_unused_variable_warning(source);
        boost::ignore_unused_variable_warning(target);

        return true;
    }
};

template <typename R, typename P, typename S, std::size_t N>
struct sectionalize_range
{
    static inline void apply(R const& range, S& sections, int ring_index = -1, int multi_index = -1)
    {
        typedef segment<const P> segment_type;

        std::size_t const n = boost::size(range);
        if (n == 0)
        {
            // Zero points, no section
            return;
        }

        if (n == 1)
        {
            // Line with one point ==> no sections
            return;
        }

        int i = 0;

        typedef typename boost::range_value<S>::type sections_range_type;
        sections_range_type section;

        typedef typename boost::range_const_iterator<R>::type iterator_type;
        iterator_type it = boost::begin(range);

        for(iterator_type previous = it++; it != boost::end(range); previous = it++, i++)
        {
            segment_type s(*previous, *it);

            int direction_classes[N] = {0};
            get_direction_loop<segment_type, 0, N>::apply(s, direction_classes);

            if (section.count > 0
                && !compare_loop<int, 0, N>::apply(direction_classes, section.directions))
            {
                sections.push_back(section);
                section = sections_range_type();
            }

            if (section.count == 0)
            {
                section.begin_index = i;
                section.ring_index = ring_index;
                section.multi_index = multi_index;
                copy_loop<int, 0, N>::apply(direction_classes, section.directions);
                ggl::combine(section.bounding_box, *previous);
            }

            ggl::combine(section.bounding_box, *it);
            section.end_index = i + 1;
            section.count++;
        }

        if (section.count > 0)
        {
            sections.push_back(section);
        }
    }
};

template <typename P, typename S, std::size_t N>
struct sectionalize_polygon
{
    static inline void apply(P const& poly, S& sections, int multi_index = -1)
    {
        typedef typename point_type<P>::type point_type;
        typedef typename ring_type<P>::type ring_type;
        typedef sectionalize_range<ring_type, point_type, S, N> sectionalizer_type;
        typedef typename boost::range_const_iterator
            <
            typename interior_type<P>::type
            >::type iterator_type;

        sectionalizer_type::apply(exterior_ring(poly), sections, -1, multi_index);

        int i = 0;
        for (iterator_type it = boost::begin(interior_rings(poly));
             it != boost::end(interior_rings(poly));
             ++it, ++i)
        {
            sectionalizer_type::apply(*it, sections, i, multi_index);
        }
    }
};

template <typename B, typename S, std::size_t N>
struct sectionalize_box
{
    static inline void apply(B const& box, S& sections)
    {
        typedef typename point_type<B>::type point_type;

        assert_dimension<B, 2>();

        // Add all four sides of the 2D-box as separate section. Easiest is to convert it to a polygon.
        // However, we don't have the polygon type (or polygon would be a helper-type).
        // Therefore we mimic a linestring/std::vector of 5 points

        point_type ll, lr, ul, ur;
        assign_box_corners(box, ll, lr, ul, ur);

        std::vector<point_type> points;
        points.push_back(ll);
        points.push_back(ul);
        points.push_back(ur);
        points.push_back(lr);
        points.push_back(ll);

        sectionalize_range<std::vector<point_type>, point_type, S, N>::apply(points, sections);
    }
};

}} // namespace detail::sectionalize
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Tag, typename G, typename S, std::size_t N>
struct sectionalize
{
};

template <typename B, typename S, std::size_t N>
struct sectionalize<box_tag, B, S, N>
    : detail::sectionalize::sectionalize_box<B, S, N> { };

template <typename L, typename S, std::size_t N>
struct sectionalize<linestring_tag, L, S, N>
    : detail::sectionalize::sectionalize_range<L, typename point_type<L>::type, S, N> { };

template <typename R, typename S, std::size_t N>
struct sectionalize<ring_tag, R, S, N>
    : detail::sectionalize::sectionalize_range<R, typename point_type<R>::type, S, N> { };

template <typename P, typename S, std::size_t N>
struct sectionalize<polygon_tag, P, S, N>
    : detail::sectionalize::sectionalize_polygon<P, S, N> { };

} // namespace dispatch
#endif


/*!
    \brief Split a geometry into monotonic sections
    \ingroup sectionalize
    \tparam G type of geometry to check
    \tparam S type of sections to create
    \param geometry geometry which might be located in the neighborhood
    \param section structure with sections

 */
template<typename G, typename S>
inline void sectionalize(G const& geometry, S& sections)
{
    typedef dispatch::sectionalize
        <
            typename tag<G>::type,
            G,
            S,
            S::value
        > sectionalizer_type;

    sections.clear();
    sectionalizer_type::apply(geometry, sections);
}











// will be moved soon to "get_section"


template <typename Tag, typename Geometry, typename Section>
struct get_section
{
    typedef typename ggl::point_const_iterator<Geometry>::type iterator_type;
    static inline void apply(Geometry const& geometry, Section const& section,
                iterator_type& begin, iterator_type& end)
    {
        begin = boost::begin(geometry) + section.begin_index;
        end = boost::begin(geometry) + section.end_index + 1;
    }
};

template <typename Polygon, typename Section>
struct get_section<polygon_tag, Polygon, Section>
{
    typedef typename ggl::point_const_iterator<Polygon>::type iterator_type;
    static inline void apply(Polygon const& polygon, Section const& section,
                iterator_type& begin, iterator_type& end)
    {
        typedef typename ggl::ring_type<Polygon>::type ring_type;
        ring_type const& ring = section.ring_index < 0
            ? ggl::exterior_ring(polygon)
            : ggl::interior_rings(polygon)[section.ring_index];

        begin = boost::begin(ring) + section.begin_index;
        end = boost::begin(ring) + section.end_index + 1;
    }
};



} // namespace ggl

#endif // GGL_ALGORITHMS_SECTIONALIZE_HPP
