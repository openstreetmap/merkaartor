// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_INTERSECTION_LINESTRING_HPP
#define GGL_ALGORITHMS_INTERSECTION_LINESTRING_HPP

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/algorithms/append.hpp>
#include <ggl/algorithms/clear.hpp>
#include <ggl/util/copy.hpp>
#include <ggl/util/select_coordinate_type.hpp>
#include <ggl/geometries/segment.hpp>

namespace ggl
{

namespace strategy { namespace intersection {

/*!
    \brief Strategy: line clipping algorithm after Liang Barsky
    \ingroup overlay
    \details The Liang-Barsky line clipping algorithm clips a line with a clipping box.
    It is slightly adapted in the sense that it returns which points are clipped
    \tparam B input box type of clipping box
    \tparam P input/output point-type of segments to be clipped
    \note The algorithm is currently only implemented for 2D Cartesian points
    \author Barend Gehrels, and the following recourses
    - A tutorial: http://www.skytopia.com/project/articles/compsci/clipping.html
    - a German applet (link broken): http://ls7-www.cs.uni-dortmund.de/students/projectgroups/acit/lineclip.shtml
*/
template<typename B, typename P>
class liang_barsky
{
private:
    typedef ggl::segment<P> segment_type;

    inline bool check_edge(double const& p, double const& q, double& t1, double& t2) const
    {
        bool visible = true;

        if(p < 0)
        {
            // TODO: Move r definition one scope level up to reuse --mloskot
            double const r = q / p;
            if (r > t2)
                visible = false;
            else if (r > t1)
                t1 = r;
        }
        else if(p > 0)
        {
            double const r = q / p;
            if (r < t1)
                visible = false;
            else if (r < t2)
                t2 = r;
        }
        else
        {
            if (q < 0)
                visible = false;
        }

        return visible;
    }

public:

    bool clip_segment(B const& b, segment_type& s, bool& sp1_clipped, bool& sp2_clipped) const
    {
        typedef typename select_coordinate_type<B, P>::type coordinate_type;

        double t1 = 0;
        double t2 = 1;

        coordinate_type const dx = get<1, 0>(s) - get<0, 0>(s);
        coordinate_type const dy = get<1, 1>(s) - get<0, 1>(s);

        coordinate_type const p1 = -dx;
        coordinate_type const p2 = dx;
        coordinate_type const p3 = -dy;
        coordinate_type const p4 = dy;

        coordinate_type const q1 = get<0, 0>(s) - get<min_corner, 0>(b);
        coordinate_type const q2 = get<max_corner, 0>(b) - get<0, 0>(s);
        coordinate_type const q3 = get<0, 1>(s) - get<min_corner, 1>(b);
        coordinate_type const q4 = get<max_corner, 1>(b) - get<0, 1>(s);

        if (check_edge(p1, q1, t1, t2)      // left
            && check_edge(p2, q2, t1, t2)   // right
            && check_edge(p3, q3, t1, t2)   // bottom
            && check_edge(p4, q4, t1, t2))   // top
        {
            sp1_clipped = t1 > 0;
            sp2_clipped = t2 < 1;

            // TODO: maybe round coordinates in case of integer? define some round_traits<> or so?
            // Take boost-round of Fernando
            if (sp2_clipped)
            {
                set<1, 0>(s, get<0, 0>(s) + t2 * dx);
                set<1, 1>(s, get<0, 1>(s) + t2 * dy);
            }

            if(sp1_clipped)
            {
                set<0, 0>(s, get<0, 0>(s) + t1 * dx);
                set<0, 1>(s, get<0, 1>(s) + t1 * dy);
            }

            return true;
        }

        return false;
    }

    template<typename L, typename OutputIterator>
    inline void add(L& line_out, OutputIterator out) const
    {
        if (!boost::empty(line_out))
        {
            *out = line_out;
            ++out;
            ggl::clear(line_out);
        }
    }
};
}} // namespace strategy::intersection


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace intersection {

/*!
    \brief Clips a linestring with a box
    \details A linestring is intersected (clipped) by the specified box
    and the resulting linestring, or pieces of linestrings, are sent to the specified output operator.
    \tparam OutputLinestring type of the output linestrings
    \tparam OutputIterator an output iterator which outputs linestrings
    \tparam Linestring linestring-type, for example a vector of points, matching the output-iterator type,
         the points should also match the input-iterator type
    \tparam Box box type
    \tparam Strategy strategy, a clipping strategy which should implement the methods "clip_segment" and "add"
*/
template
<
    typename OutputLinestring,
    typename OutputIterator,
    typename Linestring,
    typename Box,
    typename Strategy
>
OutputIterator clip_linestring_with_box(Box const& b, Linestring const& linestring,
            OutputIterator out, Strategy const& strategy)
{
    if (boost::begin(linestring) == boost::end(linestring))
    {
        return out;
    }

    typedef typename point_type<OutputLinestring>::type point_type;

    OutputLinestring line_out;

    typedef typename boost::range_const_iterator<Linestring>::type iterator_type;
    iterator_type vertex = boost::begin(linestring);
    for(iterator_type previous = vertex++;
            vertex != boost::end(linestring);
            previous = vertex++)
    {
        point_type p1, p2;
        copy_coordinates(*previous, p1);
        copy_coordinates(*vertex, p2);

        // Clip the segment. Five situations:
        // 1. Segment is invisible, finish line if any (shouldn't occur)
        // 2. Segment is completely visible. Add (p1)-p2 to line
        // 3. Point 1 is invisible (clipped), point 2 is visible. Start new line from p1-p2...
        // 4. Point 1 is visible, point 2 is invisible (clipped). End the line with ...p2
        // 5. Point 1 and point 2 are both invisible (clipped). Start/finish an independant line p1-p2
        //
        // This results in:
        // a. if p1 is clipped, start new line
        // b. if segment is partly or completely visible, add the segment
        // c. if p2 is clipped, end the line

        bool c1 = false;
        bool c2 = false;
        segment<point_type> s(p1, p2);

        if (!strategy.clip_segment(b, s, c1, c2))
        {
            strategy.add(line_out, out);
        }
        else
        {
            // a. If necessary, finish the line and add a start a new one
            if (c1)
            {
                strategy.add(line_out, out);
            }

            // b. Add p1 only if it is the first point, then add p2
            if (boost::empty(line_out))
            {
                ggl::append(line_out, p1);
            }
            ggl::append(line_out, p2);

            // c. If c2 is clipped, finish the line
            if (c2)
            {
                strategy.add(line_out, out);
            }
        }

    }

    // Add last part
    strategy.add(line_out, out);
    return out;
}

}} // namespace detail::intersection
#endif // DOXYGEN_NO_DETAIL

} // namespace ggl

#endif // GGL_ALGORITHMS_INTERSECTION_LINESTRING_HPP
