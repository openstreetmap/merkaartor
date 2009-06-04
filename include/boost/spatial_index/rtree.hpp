//
// Boost.SpatialIndex - rtree implementation
//
// Copyright 2008 Federico J. Fernandez.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/ for latest version.
//

#ifndef BOOST_SPATIAL_INDEX_RTREE_HPP
#define BOOST_SPATIAL_INDEX_RTREE_HPP

#include <geometry/algorithms/area.hpp>

#include "helpers.hpp"

#include "rtree_node.hpp"
#include "rtree_leaf.hpp"

namespace boost
{
  namespace spatial_index
  {

    template < typename Point, typename Value > class rtree
    {
    public:
      /**
       * \brief Creates a rtree with M maximum elements per node
       *        and m minimum.
       */
      rtree(const unsigned int &M, const unsigned int &m)
      : element_count_(0), m_(m), M_(M),
        root_(new rtree_node < Point,
              Value > (boost::shared_ptr < rtree_node < Point, Value > >(), 1))
      {
      }


      /**
       * \brief Creates a rtree with M maximum elements per node
       *        and m minimum (b is ignored).
       */
      rtree(const geometry::box < Point > &b, const unsigned int &M,
            const unsigned int &m)
      : element_count_(0), m_(m), M_(M), root_(
        new rtree_node < Point, Value > (boost::shared_ptr <
                                                        rtree_node < Point,
                                                        Value > >(), 1))
      {
      }


      /**
       * \brief Remove the element with key 'k'
       */
      void remove(const Point & k)
      {
        /// it's the same than removing a box of only one point
        this->remove(geometry::box < Point > (k, k));
      }


      /**
       * \brief Remove elements inside the box 'k'
       */
      void remove(const geometry::box < Point > &k)
      {
        try {
          boost::shared_ptr < rtree_node < Point,
            Value > >l(choose_exact_leaf(k));
          typename rtree_leaf < Point, Value >::leaves_map q_leaves;

          l->remove(k);

          if (l->elements() < m_ && elements() > m_) {
            q_leaves = l->get_leaves();

            // we remove the leaf_node in the parent node because now it's empty
            l->get_parent()->remove(l->get_parent()->get_box(l));
          }

          typename rtree_node < Point, Value >::node_map q_nodes;
          condense_tree(l, q_nodes);

          std::vector < std::pair < geometry::box < Point >, Value > >s;
          for(typename rtree_node < Point,
               Value >::node_map::const_iterator it = q_nodes.begin();
               it != q_nodes.end(); ++it) {
            typename rtree_leaf < Point, Value >::leaves_map leaves =
              it->second->get_leaves();

            // reinserting leaves from nodes
            for(typename rtree_leaf < Point,
                 Value >::leaves_map::const_iterator itl = leaves.begin();
                 itl != leaves.end(); ++itl) {
              s.push_back(*itl);
            }
          }

          for(typename std::vector < std::pair < geometry::box < Point >,
               Value > >::const_iterator it = s.begin(); it != s.end(); ++it) {
            element_count_--;
            insert(it->first, it->second);
          }

          // if the root has only one child and the child is not a leaf, 
          // make it the root
          if (root_->elements() == 1) {
            if (!root_->first_element()->is_leaf()) {
              root_ = root_->first_element();
            }
          }
          // reinserting leaves
          for(typename rtree_leaf < Point,
               Value >::leaves_map::const_iterator it = q_leaves.begin();
               it != q_leaves.end(); ++it) {
            element_count_--;
            insert(it->first, it->second);
          }

          element_count_--;

        }
        catch(std::logic_error & e) {
          // not found
          std::cerr << e.what() << std::endl;
          return;
        }
      }


      /**
       * \brief Remove element inside the box 'k' with vale 'v'
       */
      void remove(const geometry::box < Point > &k, const Value &v)
      {
        try {
          boost::shared_ptr < rtree_node < Point, Value > > l;

		  // find possible leaves
          typename std::vector < boost::shared_ptr < rtree_node < Point,
            Value > > > nodes;
            root_->find_leaves(k, nodes);

          // refine the result
          for(typename std::vector < boost::shared_ptr < rtree_node < Point,
               Value > > >::const_iterator it = nodes.begin(); it != nodes.end();
               ++it)
          {
            l = *it;
		    l->remove(v);
          }

		  if (!l)
			  return;

          typename rtree_leaf < Point, Value >::leaves_map q_leaves;

          if (l->elements() < m_ && elements() > m_) {
            q_leaves = l->get_leaves();

            // we remove the leaf_node in the parent node because now it's empty
            l->get_parent()->remove(l->get_parent()->get_box(l));
          }

          typename rtree_node < Point, Value >::node_map q_nodes;
          condense_tree(l, q_nodes);

          std::vector < std::pair < geometry::box < Point >, Value > >s;
          for(typename rtree_node < Point,
               Value >::node_map::const_iterator it = q_nodes.begin();
               it != q_nodes.end(); ++it) {
            typename rtree_leaf < Point, Value >::leaves_map leaves =
              it->second->get_leaves();

            // reinserting leaves from nodes
            for(typename rtree_leaf < Point,
                 Value >::leaves_map::const_iterator itl = leaves.begin();
                 itl != leaves.end(); ++itl) {
              s.push_back(*itl);
            }
          }

          for(typename std::vector < std::pair < geometry::box < Point >,
               Value > >::const_iterator it = s.begin(); it != s.end(); ++it) {
            element_count_--;
            insert(it->first, it->second);
          }

          // if the root has only one child and the child is not a leaf, 
          // make it the root
          if (root_->elements() == 1) {
            if (!root_->first_element()->is_leaf()) {
              root_ = root_->first_element();
            }
          }
          // reinserting leaves
          for(typename rtree_leaf < Point,
               Value >::leaves_map::const_iterator it = q_leaves.begin();
               it != q_leaves.end(); ++it) {
            element_count_--;
            insert(it->first, it->second);
          }

          element_count_--;

        }
        catch(std::logic_error & e) {
          // not found
          std::cerr << e.what() << std::endl;
          return;
        }
      }


      /**
       * \brief Returns the number of elements.
       */
      unsigned int elements(void) const
      {
        return element_count_;
      }


      /**
       * \brief Print Rtree (mainly for debug)
       */
      void print(void)
      {
        std::cerr << "===================================" << std::endl;
        std::cerr << " Min/Max: " << m_ << " / " << M_ << std::endl;
        std::cerr << "Leaves: " << root_->get_leaves().size() << std::endl;
        root_->print();
        std::cerr << "===================================" << std::endl;
      }


      /**
       * \brief Inserts a new element with key 'k' and value 'v'
       */
      void insert(const Point & k, const Value & v)
      {
        // it's the same that inserting a box of only one point
        this->insert(geometry::box < Point > (k, k), v);
      }


      /**
       * \brief Inserts a new element with key 'e' and value 'v'
       */
      void insert(const geometry::box < Point > &e, const Value & v)
      {
        element_count_++;

        boost::shared_ptr < rtree_node < Point,
          Value > >l(choose_corresponding_leaf(e));

        // check if the selected leaf is full to do the split if necessary
        if (l->elements() >= M_) {
          l->insert(e, v);

          // split!
          boost::shared_ptr < rtree_node < Point,
            Value > >n1(new rtree_leaf < Point, Value > (l->get_parent()));
          boost::shared_ptr < rtree_node < Point,
            Value > >n2(new rtree_leaf < Point, Value > (l->get_parent()));

          split_node(l, n1, n2);
          adjust_tree(l, n1, n2);
        } else {
          l->insert(e, v);
          adjust_tree(l);
        }
      }


      /**
       * \brief Insert all the elements in 'values' and 'points'
       */
      void bulk_insert(std::vector < Value > &values,
                       std::vector < Point > &points)
      {
        typename std::vector < Point >::iterator it_point;
        typename std::vector < Value >::iterator it_value;

        // unsigned int count = 0;

        it_point = points.begin();
        it_value = values.begin();
        while (it_value != values.end() && it_point != points.end()) {
          geometry::box < geometry::point_xy < double > > box(*it_point,
                                                            *it_point);
          insert(box, *it_value);

          it_point++;
          it_value++;
          // count++;

          // if(count % 1000 == 0) {
          //  std::cerr << "Count: " << count << std::endl;
          //  print();
          // }
        }
      }

      /**
       * \brief Search for 'k' in the Rtree. If 'k' is found
       *        it returns its value, otherwise it returns Value()
       */
      Value find(const Point & k)
      {
        std::deque < Value > result;
        geometry::box < geometry::point_xy < double > > query_box(k, k);

        root_->find(query_box, result, true);
        if (result.size() >= 1) {
          return result[0];
        }
        return Value();
      }


      /**
       * \brief Returns all the values inside 'r'
       */
      std::deque < Value > find(const geometry::box < Point > &r) {
        std::deque < Value > result;
        root_->find(r, result, false);
        return result;
      }

    private:
      /// number of elements
      unsigned int element_count_;

      /// minimum number of elements per node
      unsigned int m_;

      /// maximum number of elements per node
      unsigned int M_;

      /// tree root
      boost::shared_ptr < rtree_node < Point, Value > >root_;


    private:

      /**
       * \brief Reorganize the tree after a removal. It tries to
       *        join nodes with less elements than m.
       */
      void condense_tree(const boost::shared_ptr < rtree_node < Point,
                         Value > >&l, typename rtree_node < Point,
                         Value >::node_map & q_nodes)
      {
        if (l.get() == root_.get()) {
          // if it's the root we are done
          return;
        }

        boost::shared_ptr < rtree_node < Point, Value > >parent =
          l->get_parent();
        parent->adjust_box(l);

        if (parent->elements() < m_) {
          if (parent.get() == root_.get()) {
            // if the parent is underfull and it's the root we just exit
            return;
          }
          // get the nodes that we should reinsert
          typename rtree_node < Point, Value >::node_map this_nodes =
            parent->get_nodes();
          for(typename rtree_node < Point,
               Value >::node_map::const_iterator it = this_nodes.begin();
               it != this_nodes.end(); ++it) {
            q_nodes.push_back(*it);
          }

          // we remove the node in the parent node because now it should be 
          // re inserted
          parent->get_parent()->remove(parent->get_parent()->get_box(parent));
        }

        condense_tree(parent, q_nodes);
      }


      /**
       * \brief After an insertion splits nodes with more than M elements.
       */
      void adjust_tree(boost::shared_ptr < rtree_node < Point, Value > >&n)
      {
        if (n.get() == root_.get()) {
          // we finished the adjust
          return;
        }
        // as there are no splits just adjust the box of the parent and go on
        boost::shared_ptr < rtree_node < Point, Value > >parent =
          n->get_parent();
        parent->adjust_box(n);
        adjust_tree(parent);
      }

      /**
       * \brief After an insertion splits nodes with more than M elements
       *        (recursive step with subtrees 'n1' and 'n2' to be joined).
       */
      void adjust_tree(boost::shared_ptr < rtree_node < Point, Value > >&l,
                       boost::shared_ptr < rtree_node < Point, Value > >&n1,
                       boost::shared_ptr < rtree_node < Point, Value > >&n2)
      {
        // check if we are in the root and do the split
        if (l.get() == root_.get()) {
          boost::shared_ptr < rtree_node < Point, Value > >new_root(
                 new rtree_node <Point, Value >
                     (boost::shared_ptr<rtree_node< Point, Value > >(),
                      l->get_level() + 1));
          new_root->add_node(n1->compute_box(), n1);
          new_root->add_node(n2->compute_box(), n2);

          n1->set_parent(new_root);
          n2->set_parent(new_root);

          n1->update_parent(n1);
          n2->update_parent(n2);

          root_ = new_root;
          return;
        }
        boost::shared_ptr < rtree_node < Point, Value > >parent =
          l->get_parent();

        parent->replace_node(l, n1);
        parent->add_node(n2->compute_box(), n2);

        // if parent is full, split and readjust
        if (parent->elements() > M_) {

          boost::shared_ptr < rtree_node < Point, Value > >p1(new
                              rtree_node <Point, Value >
                              (parent->get_parent(), parent->get_level()));
          boost::shared_ptr < rtree_node < Point,
            Value > >p2(new rtree_node < Point,
                        Value > (parent->get_parent(), parent->get_level()));

          split_node(parent, p1, p2);
          adjust_tree(parent, p1, p2);
        } else {
          adjust_tree(parent);
        }
      }


      /**
       * \brief Splits 'n' in 'n1' and 'n2'
       */
      void split_node(const boost::shared_ptr < rtree_node < Point, Value > >&n,
                      boost::shared_ptr < rtree_node < Point, Value > >&n1,
                      boost::shared_ptr < rtree_node < Point, Value > >&n2) 
                      const
      {
        unsigned int seed1, seed2;
          std::vector < geometry::box < Point > >boxes = n->get_boxes();

          n1->set_parent(n->get_parent());
          n2->set_parent(n->get_parent());

          linear_pick_seeds(n, seed1, seed2);

        if (n->is_leaf())
        {
          n1->add_value(boxes[seed1], n->get_value(seed1));
          n2->add_value(boxes[seed2], n->get_value(seed2));
        } else
        {
          n1->add_node(boxes[seed1], n->get_node(seed1));
          n2->add_node(boxes[seed2], n->get_node(seed2));
        }

        unsigned int index = 0;

        if (n->is_leaf()) {
          typename rtree_leaf < Point, Value >::leaves_map nodes =
            n->get_leaves();
          unsigned int remaining = nodes.size() - 2;
          for(typename rtree_leaf < Point,
               Value >::leaves_map::const_iterator it = nodes.begin();
               it != nodes.end(); ++it, index++) {

            if (index != seed1 && index != seed2) {
              if (n1->elements() + remaining == m_) {
                n1->add_value(it->first, it->second);
                continue;
              }
              if (n2->elements() + remaining == m_) {
                n2->add_value(it->first, it->second);
                continue;
              }

              remaining--;

              /// current boxes of each group
              geometry::box < Point > b1, b2;

              /// enlarged boxes of each group
              geometry::box < Point > eb1, eb2;
              b1 = n1->compute_box();
              b2 = n2->compute_box();

              /// areas
              double b1_area, b2_area;
              double eb1_area, eb2_area;
              b1_area = geometry::area(b1);
              b2_area = geometry::area(b2);

              eb1_area = compute_union_area(b1, it->first);
              eb2_area = compute_union_area(b2, it->first);

              if (eb1_area - b1_area > eb2_area - b2_area) {
                n2->add_value(it->first, it->second);
              }
              if (eb1_area - b1_area < eb2_area - b2_area) {
                n1->add_value(it->first, it->second);
              }
              if (eb1_area - b1_area == eb2_area - b2_area) {
                if (b1_area < b2_area) {
                  n1->add_value(it->first, it->second);
                }
                if (b1_area > b2_area) {
                  n2->add_value(it->first, it->second);
                }
                if (b1_area == b2_area) {
                  if (n1->elements() > n2->elements()) {
                    n2->add_value(it->first, it->second);
                  } else {
                    n1->add_value(it->first, it->second);
                  }
                }
              }
            }
          }
        } else {
          typename rtree_node < Point, Value >::node_map nodes = n->get_nodes();
          unsigned int remaining = nodes.size() - 2;
          for(typename rtree_node < Point,
               Value >::node_map::const_iterator it = nodes.begin();
               it != nodes.end(); ++it, index++) {

            if (index != seed1 && index != seed2) {

              if (n1->elements() + remaining == m_) {
                n1->add_node(it->first, it->second);
                continue;
              }
              if (n2->elements() + remaining == m_) {
                n2->add_node(it->first, it->second);
                continue;
              }

              remaining--;

              /// current boxes of each group
              geometry::box < Point > b1, b2;

              /// enlarged boxes of each group
              geometry::box < Point > eb1, eb2;
              b1 = n1->compute_box();
              b2 = n2->compute_box();

              /// areas
              double b1_area, b2_area;
              double eb1_area, eb2_area;
              b1_area = geometry::area(b1);
              b2_area = geometry::area(b2);

              eb1_area = compute_union_area(b1, it->first);
              eb2_area = compute_union_area(b2, it->first);

              if (eb1_area - b1_area > eb2_area - b2_area) {
                n2->add_node(it->first, it->second);
              }
              if (eb1_area - b1_area < eb2_area - b2_area) {
                n1->add_node(it->first, it->second);
              }
              if (eb1_area - b1_area == eb2_area - b2_area) {
                if (b1_area < b2_area) {
                  n1->add_node(it->first, it->second);
                }
                if (b1_area > b2_area) {
                  n2->add_node(it->first, it->second);
                }
                if (b1_area == b2_area) {
                  if (n1->elements() > n2->elements()) {
                    n2->add_node(it->first, it->second);
                  } else {
                    n1->add_node(it->first, it->second);
                  }
                }
              }

            }
          }
        }
      }


      /**
       * \brief Choose initial values for the split algorithm (linear version)
       */
      void linear_pick_seeds(const boost::shared_ptr < rtree_node < Point,
                             Value > >&n, unsigned int &seed1,
                             unsigned int &seed2) const
      {
        // get boxes from the node
        std::vector < geometry::box < Point > >boxes = n->get_boxes();
        if (boxes.size() == 0) {
          throw std::logic_error("Empty Node trying to Pick Seeds");
        }
        // only two dim for now
        // unsigned int dimensions = 
        //   geometry::point_traits<Point>::coordinate_count;

        // find the first two elements
        double separation_x, separation_y;
        unsigned int first_x, second_x;
        unsigned int first_y, second_y;
        find_normalized_separations < 0u > (boxes, separation_x, first_x,
                                            second_x);
        find_normalized_separations < 1u > (boxes, separation_y, first_y,
                                            second_y);

        if (separation_x > separation_y) {
          seed1 = first_x;
          seed2 = second_x;
        } else {
          seed1 = first_y;
          seed2 = second_y;
        }
      }


      /**
       * \brief Find distances between possible initial values for the
       *        pick_seeds algorithm.
       */
      template < unsigned int Dimension >
        void find_normalized_separations(const std::vector < geometry::box <
                                         Point > >&boxes, double &separation,
                                         unsigned int &first,
                                         unsigned int &second) const
      {
        if (boxes.size() < 2)
        {
          throw std::logic_error("At least two boxes needed to split");
        }
        // find the lowest high
        typename std::vector < geometry::box < Point > >::const_iterator it =
          boxes.begin();
        double lowest_high = geometry::get < Dimension > (it->max_corner());
        unsigned int lowest_high_index = 0;
        unsigned int index;
        ++it;
        index = 1;
        for(; it != boxes.end(); ++it) {
          if (geometry::get < Dimension > (it->max_corner()) < lowest_high) {
            lowest_high = geometry::get < Dimension > (it->max_corner());
            lowest_high_index = index;
          }
          index++;
        }

        // find the highest low      
        double highest_low;
        unsigned int highest_low_index;
        if (lowest_high_index == 0) {
          highest_low = geometry::get < Dimension > (boxes[1].min_corner());
          highest_low_index = 1;
        } else {
          highest_low = geometry::get < Dimension > (boxes[0].min_corner());
          highest_low_index = 0;
        }
        index = 0;
        for(typename std::vector < geometry::box < Point >
             >::const_iterator it = boxes.begin(); it != boxes.end();
             ++it, index++) {
          if (geometry::get < Dimension > (it->min_corner()) >= highest_low
              && index != lowest_high_index) {
            highest_low = geometry::get < Dimension > (it->min_corner());
            highest_low_index = index;
          }
        }

        // find the lowest low
        it = boxes.begin();
        double lowest_low = geometry::get < Dimension > (it->min_corner());
        ++it;
        for(; it != boxes.end(); ++it) {
          if (geometry::get < Dimension > (it->min_corner()) < lowest_low) {
            lowest_low = geometry::get < Dimension > (it->min_corner());
          }
        }

        // find the highest high
        it = boxes.begin();
        double highest_high = geometry::get < Dimension > (it->max_corner());
        ++it;
        for(; it != boxes.end(); ++it) {
          if (geometry::get < Dimension > (it->max_corner()) > highest_high) {
            highest_high = geometry::get < Dimension > (it->max_corner());
          }
        }

        double width = highest_high - lowest_low;

        separation = (highest_low - lowest_high) / width;
        first = highest_low_index;
        second = lowest_high_index;
      }


      /**
       * \brief Choose one of the possible leaves to make an insertion
       */
      boost::shared_ptr < rtree_node < Point, Value > >
      choose_corresponding_leaf(const geometry::box < Point > e)
      {
        boost::shared_ptr < rtree_node < Point, Value > >N = root_;

        // if the tree is empty add an initial leaf
        if (root_->elements() == 0) {
          boost::shared_ptr < rtree_leaf < Point,
            Value > >new_leaf(new rtree_leaf < Point, Value > (root_));
          root_->add_leaf_node(geometry::box < Point > (), new_leaf);

          return new_leaf;
        }

        while (!N->is_leaf()) {
          /// traverse N's map to see which node we should select
          N = N->choose_node(e);
        }
        return N;
      }

      /**
       * \brief Choose the exact leaf where an insertion should be done
       */
      boost::shared_ptr < rtree_node < Point,
        Value > >choose_exact_leaf(const geometry::box < Point > &e) const
      {
        // find possible leaves
        typename std::vector < boost::shared_ptr < rtree_node < Point,
          Value > > > nodes;
          root_->find_leaves(e, nodes);

        // refine the result
        for(typename std::vector < boost::shared_ptr < rtree_node < Point,
             Value > > >::const_iterator it = nodes.begin(); it != nodes.end();
             ++it)
        {

          typename std::vector < std::pair < geometry::box < Point >,
            Value > >leaves = (*it)->get_leaves();
          for(typename std::vector < std::pair < geometry::box < Point >,
               Value > >::const_iterator itl = leaves.begin();
               itl != leaves.end(); ++itl)
          {

            if (itl->first.max_corner() == e.max_corner() && itl->first.min_corner() == e.min_corner()) {
              return *it;
            }

          }

        }
        throw std::logic_error("Leaf not found");
      }

    };


  }                             // namespace spatial_index
}                               // namespace boost

#endif // BOOST_SPATIAL_INDEX_RTREE_HPP
