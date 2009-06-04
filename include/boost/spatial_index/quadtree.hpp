//
// Boost.SpatialIndex - quadtree implementation
//
// Copyright 2008 Federico J. Fernandez.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/ for latest version.
//


#ifndef BOOST_SPATIAL_INDEX_QUADTREE_HPP
#define BOOST_SPATIAL_INDEX_QUADTREE_HPP

#include "quadtree_node.hpp"

// #include <boost/thread/xtime.hpp>

namespace boost
{
  namespace spatial_index
  {


    template < typename Point, typename Value > class quadtree
    {
    public:
      /**
       * \brief Creates a quadtree using r as bounding box
       */
      quadtree(const geometry::box < Point > &r)
               : root(r, 1), element_count(0), node_size_(1)
      {
      }

      /**
       * \brief Creates a quadtree using r as bounding box and M as maximum
       *        number of elements per node.
       */
      quadtree(const geometry::box < Point > &r, const unsigned int M)
               : root(r, M), element_count(0), node_size_(M)
      {
      }

      /**
       * \brief Creates a quadtree using r as bounding box and M as maximum
       *        number of elements per node (m is ignored).
       */
      quadtree(const geometry::box < Point > &r, const unsigned int m,
               const unsigned int M)
               : root(r, M), element_count(0), node_size_(M)
      {
      }


      /**
       * \brief Remove the element with key 'k'
       */
      void remove(const Point & k)
      {
        root.remove(k);
        element_count--;
      }


      /**
       * \brief Inserts a new element with key 'k' and value 'v'
       */
      void insert(const Point & k, const Value & v)
      {
        element_count++;
        root.insert(k, v);
      }


      /**
       * \brief Print Quadtree (mainly for debug)
       */
      void print(void)
      {
        std::cerr << "=================================" << std::endl;
        std::cerr << "Elements: " << elements() << std::endl;
        root.print();
        std::cerr << "=================================" << std::endl;
      }


      /**
       * \brief Insert all the elements in 'values' and 'points'
       */
      void bulk_insert(std::vector < Value > &values,
                       std::vector < Point > &points)
      {
        // boost::xtime xt1, xt2;
        // boost::xtime_get(&xt1, boost::TIME_UTC);

        // unsigned int counter = 0;

        typename std::vector < Point >::iterator it_point;
        typename std::vector < Value >::iterator it_value;
        it_point = points.begin();
        it_value = values.begin();
        while (it_value != values.end() && it_point != points.end()) {
          insert(*it_point, *it_value);

          it_point++;
          it_value++;

          // counter++;
          // if(counter%10000 == 0) {
          // std::cerr << "counter: [" << counter << "]" << std::endl;
          // }
        }
        // boost::xtime_get(&xt2, boost::TIME_UTC);
        // std::cerr << "secs: " << xt2.sec - xt1.sec;
        // std::cerr << " nsecs: " << xt2.nsec - xt1.nsec << std::endl;
      }


      /**
       * \brief Search for 'k' in the QuadTree. If 'k' is in the Quadtree
       *        it returns its value, otherwise it returns Value()
       */
      Value find(const Point & k)
      {
        return root.find(k);
      }


      /**
       * \brief Returns all the values inside 'r' in the Quadtree
       */
      std::deque < Value > find(const geometry::box < Point > &r) {
        std::deque < Value > result;
        root.find(result, r);
        return result;
      }

      /**
       * \brief Returns the number of elements.
       */
      unsigned int elements(void) const
      {
        return element_count;
      }

    private:
      /// quadtree root
      quadtree_node < Point, Value > root;

      /// element count in the Quadtree
      unsigned int element_count;

      /// number of points in each node
      unsigned int node_size_;

    };


  }                             // namespace spatial_index
}                               // namespace boost

#endif // BOOST_SPATIAL_INDEX_QUADTREE_HPP
