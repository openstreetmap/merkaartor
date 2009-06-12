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


#ifndef BOOST_SPATIAL_INDEX_HELPERS_HPP
#define BOOST_SPATIAL_INDEX_HELPERS_HPP

namespace boost
{
  namespace spatial_index
  {


    /**
     * \brief Given two boxes, returns the minimal box that contains them
     */
    template < typename Point >
      geometry::box < Point > enlarge_box(const geometry::box < Point > &b1,
                                          const geometry::box < Point > &b2)
    {
      Point min(geometry::get < 0 > (b1.min_corner()) < geometry::get < 0 >
                (b2.min_corner())? geometry::get < 0 > (b1.min_corner()) : geometry::get < 0 >
                (b2.min_corner()),
                geometry::get < 1 > (b1.min_corner()) < geometry::get < 1 >
                (b2.min_corner())? geometry::get < 1 > (b1.min_corner()) : geometry::get < 1 >
                (b2.min_corner()));

      Point max(geometry::get < 0 > (b1.max_corner()) > geometry::get < 0 >
                (b2.max_corner())? geometry::get < 0 > (b1.max_corner()) : geometry::get < 0 >
                (b2.max_corner()),
                geometry::get < 1 > (b1.max_corner()) > geometry::get < 1 >
                (b2.max_corner())? geometry::get < 1 > (b1.max_corner()) : geometry::get < 1 >
                (b2.max_corner()));

        return geometry::box < Point > (min, max);
    }


    /**
     * \brief Compute the area of the union of b1 and b2
     */
    template < typename Point >
      double compute_union_area(const geometry::box < Point > &b1,
                                const geometry::box < Point > &b2)
    {
      geometry::box < Point > enlarged_box = enlarge_box(b1, b2);
      return geometry::area(enlarged_box);
    }


    /**
     * \brief Checks if boxes overlap
     */
    template < typename Point >
      bool overlaps(const geometry::box < Point > &b1,
                    const geometry::box < Point > &b2)
    {

      bool overlaps_x, overlaps_y;
      overlaps_x = overlaps_y = false;

      if (geometry::get < 0 > (b1.min_corner()) >= geometry::get < 0 > (b2.min_corner())
          && geometry::get < 0 > (b1.min_corner()) <= geometry::get < 0 > (b2.max_corner()))
        {
          overlaps_x = true;
        } else
      if (geometry::get < 0 > (b1.max_corner()) >= geometry::get < 0 > (b2.min_corner())
          && geometry::get < 0 > (b1.min_corner()) <= geometry::get < 0 > (b2.max_corner()))
        {
          overlaps_x = true;
        }

      if (geometry::get < 1 > (b1.min_corner()) >= geometry::get < 1 > (b2.min_corner())
          && geometry::get < 1 > (b1.min_corner()) <= geometry::get < 1 > (b2.max_corner()))
        {
          overlaps_y = true;
        } else
      if (geometry::get < 1 > (b1.max_corner()) >= geometry::get < 1 > (b2.min_corner())
          && geometry::get < 1 > (b1.min_corner()) <= geometry::get < 1 > (b2.max_corner()))
        {
          overlaps_y = true;
        }

      return overlaps_x && overlaps_y;
    }


  }                             // namespace spatial_index
}                               // namespace boost

#endif // BOOST_SPATIAL_INDEX_HELPERS_HPP
