//
// Boost.SpatialIndex - geometry helper functions
//
// Copyright 2008 Federico J. Fernandez.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/ for latest version.
//


#ifndef GGL_SPATIAL_INDEX_HELPERS_HPP
#define GGL_SPATIAL_INDEX_HELPERS_HPP

namespace ggl
{
namespace spatial_index
{


/**
 * \brief Given two boxes, returns the minimal box that contains them
 */
// TODO: use ggl::combine
template <typename Box>
inline Box enlarge_box(Box const& b1, Box const& b2)
{
    typedef typename ggl::point_type<Box>::type P;
  P min(ggl::get<min_corner, 0> (b1) < ggl::get<min_corner, 0>
            (b2)? ggl::get<min_corner, 0> (b1) : ggl::get<min_corner, 0>
            (b2),
            ggl::get<min_corner, 1> (b1) < ggl::get<min_corner, 1>
            (b2)? ggl::get<min_corner, 1> (b1) : ggl::get<min_corner, 1>
            (b2));

  P max(ggl::get<max_corner, 0> (b1) > ggl::get<max_corner, 0>
            (b2)? ggl::get<max_corner, 0> (b1) : ggl::get<max_corner, 0>
            (b2),
            ggl::get<max_corner, 1> (b1) > ggl::get<max_corner, 1>
            (b2)? ggl::get<max_corner, 1> (b1) : ggl::get<max_corner, 1>
            (b2));

    return Box (min, max);
}


/**
 * \brief Compute the area of the union of b1 and b2
 */
template <typename Box>
inline double compute_union_area(Box const& b1, Box const& b2)
{
  Box enlarged_box = enlarge_box(b1, b2);
  return ggl::area(enlarged_box);
}


/**
 * \brief Checks if boxes overlap
 */
// TODO: combine with ggl::overlaps but NOTE:
//this definition is slightly different from GGL-definition
template <typename Box>
inline bool is_overlapping(Box const& b1, Box const& b2)
{

  bool overlaps_x, overlaps_y;
  overlaps_x = overlaps_y = false;

  if (ggl::get<min_corner, 0> (b1) >= ggl::get<min_corner, 0> (b2)
      && ggl::get<min_corner, 0> (b1) <= ggl::get<max_corner, 0> (b2))
    {
      overlaps_x = true;
    }
  if (ggl::get<max_corner, 0> (b1) >= ggl::get<min_corner, 0> (b2)
      && ggl::get<min_corner, 0> (b1) <= ggl::get<max_corner, 0> (b2))
    {
      overlaps_x = true;
    }

  if (ggl::get<min_corner, 1> (b1) >= ggl::get<min_corner, 1> (b2)
      && ggl::get<min_corner, 1> (b1) <= ggl::get<max_corner, 1> (b2))
    {
      overlaps_y = true;
    }
  if (ggl::get<max_corner, 1> (b1) >= ggl::get<min_corner, 1> (b2)
      && ggl::get<min_corner, 1> (b1) <= ggl::get<max_corner, 1> (b2))
    {
      overlaps_y = true;
    }

  return overlaps_x && overlaps_y;
}



} // namespace spatial_index
} // namespace ggl

#endif // GGL_SPATIAL_INDEX_HELPERS_HPP
