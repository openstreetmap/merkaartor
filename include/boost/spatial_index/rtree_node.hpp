//
// Boost.SpatialIndex - rtree node implementation
//
// Copyright 2008 Federico J. Fernandez.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/ for latest version.
//


#ifndef BOOST_SPATIAL_INDEX_RTREE_NODE_HPP
#define BOOST_SPATIAL_INDEX_RTREE_NODE_HPP

#include <geometry/algorithms/area.hpp>
#include <geometry/algorithms/overlaps.hpp>

namespace boost
{
  namespace spatial_index
  {

    /// forward declaration
    template < typename Point, typename Value > class rtree_leaf;

    template < typename Point, typename Value > class rtree_node
    {
    public:
      /// type for the node map
      typedef std::vector < std::pair < geometry::box < Point >,
        boost::shared_ptr < rtree_node < Point, Value > > > > node_map;

    public:
      /**
       * \brief Creates a default node (needed for the containers)
       */
      rtree_node(void)
      {
      }

      /**
       * \brief Creates a node with 'parent' as parent and 'level' as its level
       */
      rtree_node(const boost::shared_ptr < rtree_node < Point, Value > >&parent,
                 const unsigned int &level)
        : parent_(parent), level_(level)
      {
      }

      /**
       * \brief Level projector
       */
      virtual unsigned int get_level(void) const
      {
        return level_;
      }

      /**
       * \brief Number of elements in the subtree
       */
      virtual unsigned int elements(void) const
      {
        return nodes_.size();
      }


      /**
       * \brief Project first element, to replace root in case of condensing
       */
      boost::shared_ptr < rtree_node < Point, Value > >first_element(void) const
      {
        if (nodes_.size() == 0) {
          throw std::logic_error("first_element in empty node");
        }
        return nodes_.begin()->second;
      }


      /**
       * \brief True if it is a leaf node
       */
      virtual bool is_leaf(void) const
      {
        return false;
      }


      /**
       * \brief Proyector for the 'i' node
       */
      boost::shared_ptr < rtree_node < Point, Value > >
      get_node(const unsigned int i)
      {
        return nodes_[i].second;
      }


      /**
       * \brief Search for elements in 'e' in the Rtree. Add them to r.
       *        If exact_match is true only return the elements having as
       *        key the box 'e'. Otherwise return everything inside 'e'.
       */
      virtual void find(const geometry::box < Point > &e,
                        std::deque < Value > &r, const bool exact_match)
      {
        for(typename node_map::const_iterator it = nodes_.begin();
             it != nodes_.end(); ++it) {
		  if (geometry::overlaps(it->first, e)) {
            it->second->find(e, r, exact_match);
          }
        }
      }


      /**
       * \brief Return in 'result' all the leaves inside 'e'
       */
      void find_leaves(const geometry::box < Point > &e,
                       typename std::vector < boost::shared_ptr < rtree_node <
                       Point, Value > > > &result) const
      {
        for(typename node_map::const_iterator it = nodes_.begin();
             it != nodes_.end(); ++it)
        {
          if (overlaps(it->first, e)) {
            if (it->second->is_leaf()) {
              result.push_back(it->second);
            } else {
              it->second->find_leaves(e, result);
            }
          }
        }
      }


      /**
       * \brief Compute bounding box for this node
       */
      virtual geometry::box < Point > compute_box(void) const
      {
        if (nodes_.empty()) {
          return geometry::box < Point > ();
        }

        typename node_map::const_iterator it = nodes_.begin();
        geometry::box < Point > r = it->first;
        it++;
        for(; it != nodes_.end(); ++it) {
          r = enlarge_box(r, it->first);
        }
        return r;
      }


      /**
       * \brief Insert a value (not allowed for a node, only on leaves)
       */
      virtual void insert(const geometry::box < Point > &, const Value &)
      {
        throw std::logic_error("Insert in node!");
      }


      /**
       * \brief Get the envelopes of a node
       */
      virtual std::vector < geometry::box < Point > >get_boxes(void) const
      {
        std::vector < geometry::box < Point > >r;
        for(typename node_map::const_iterator it = nodes_.begin();
             it != nodes_.end(); ++it) {
          r.push_back(it->first);
        }
        return r;
      }


      /**
       * \brief Recompute the bounding box
       */
      void adjust_box(const boost::shared_ptr < rtree_node < Point, Value > >&n)
      {
        unsigned int index = 0;
        for(typename node_map::iterator it = nodes_.begin();
             it != nodes_.end(); ++it, index++) {
          if (it->second.get() == n.get()) {
            nodes_[index] = std::make_pair(n->compute_box(), n);
            return;
          }
        }
      }


      /**
       * \brief Remove elements inside the box 'k'
       */
      virtual void remove(const geometry::box < Point > &k)
      {
        for(typename node_map::iterator it = nodes_.begin();
             it != nodes_.end(); ++it) {
          if (it->first.min_corner() == k.min_corner() && it->first.max_corner() == k.max_corner()) {
            nodes_.erase(it);
            return;
          }
        }
      }


      /**
       * \brief Remove value in this leaf
       */
      virtual void remove(const Value &)
      {
      }


      /**
       * \brief Replace the node in the nodes_ vector and recompute the box
       */
      void replace_node(const boost::shared_ptr < rtree_node < Point,
                        Value > >&l, boost::shared_ptr < rtree_node < Point,
                        Value > >&new_l)
      {
        unsigned int index = 0;
        for(typename node_map::iterator it = nodes_.begin();
             it != nodes_.end(); ++it, index++) {
          if (it->second.get() == l.get()) {
            nodes_[index] = std::make_pair(new_l->compute_box(), new_l);
            new_l->update_parent(new_l);
            return;
          }
        }
        throw std::logic_error("Node not found.");
      }


      /**
       * \brief Add a child to this node
       */
      virtual void add_node(const geometry::box < Point > &b,
                            const boost::shared_ptr < rtree_node < Point,
                            Value > >&n)
      {
        nodes_.push_back(std::make_pair(b, n));
        n->update_parent(n);
      }


      /**
       * \brief add a value (not allowed in nodes, only on leaves)
       */
      virtual void add_value(const geometry::box < Point > &, const Value &)
      {
        throw std::logic_error("Can't add value to non-leaf node.");
      }


      /**
       * \brief Add a child leaf to this node
       */
      void add_leaf_node(const geometry::box < Point > &b,
                         const boost::shared_ptr < rtree_leaf < Point,
                         Value > >&l)
      {
        nodes_.push_back(std::make_pair(b, l));
      }


      /**
       * \brief Choose a node suitable for adding 'e'
       */
      boost::shared_ptr < rtree_node < Point, Value > >
      choose_node(const geometry::box < Point > e)
      {
        if (nodes_.size() == 0) {
          throw std::
            logic_error
            ("Empty node trying to choose the least enlargement node.");
        }
        bool first = true;
        double min_area;
        double min_diff_area;
        boost::shared_ptr < rtree_node < Point, Value > >chosen_node;

        // check for the least enlargement
        for(typename node_map::const_iterator it = nodes_.begin();
             it != nodes_.end(); ++it) {

          double diff_area =
            compute_union_area(e, it->first) - geometry::area(it->first);

          if (first) {
            // it's the first time, we keep the first
            min_diff_area = diff_area;
            min_area = geometry::area(it->first);
            chosen_node = it->second;

            first = false;
          } else {
            if (diff_area < min_diff_area) {
              min_diff_area = diff_area;
              min_area = geometry::area(it->first);
              chosen_node = it->second;
            } else {
              if (diff_area == min_diff_area) {
                if (geometry::area(it->first) < min_area) {
                  min_diff_area = diff_area;
                  min_area = geometry::area(it->first);
                  chosen_node = it->second;
                }
              }
            }

          }
        }
        return chosen_node;
      }


      /**
       * \brief Empty the node
       */
      virtual void empty_nodes(void)
      {
        nodes_.clear();
      }


      /**
       * \brief Projector for parent
       */
      boost::shared_ptr < rtree_node < Point, Value > >get_parent(void) const
      {
        return parent_;
      }


      /**
       * \brief Update the parent of all the childs
       */
      void update_parent(const boost::shared_ptr < rtree_node < Point,
                         Value > >&p)
      {
        for(typename node_map::iterator it = nodes_.begin();
             it != nodes_.end(); ++it) {
          it->second->set_parent(p);
        }
      }


      /**
       * \brief Set parent
       */
      void set_parent(const boost::shared_ptr < rtree_node < Point, Value > >&p)
      {
        parent_ = p;
      }


      /**
       * \brief Value projector for leaf_node (not allowed for non-leaf nodes)
       */
      virtual Value get_value(const unsigned int) const
      {
        throw std::logic_error("No values in a non-leaf node.");
      }

      /**
       * \brief Box projector for node 'i'
       */
      virtual geometry::box < Point > get_box(const unsigned int i) const
      {
        return nodes_[i].first;
      }


      /**
       * \brief Box projector for node pointed by 'l'
       */
      virtual geometry::box < Point > get_box(const boost::shared_ptr <
                                              rtree_node < Point,
                                              Value > >&l) const
      {
        for(typename node_map::const_iterator it = nodes_.begin();
             it != nodes_.end(); ++it)
        {
          if (it->second.get() == l.get()) {
            return it->first;
          }
        }
        throw std::logic_error("Node not found");
      }


      /**
       * \brief Children projector
       */
      node_map get_nodes(void) const
      {
        return nodes_;
      }


      /**
       * \brief Get leaves for a node
       */
      virtual std::vector < std::pair < geometry::box < Point >, Value > >
      get_leaves(void) const
      {
        std::vector < std::pair < geometry::box < Point >, Value > >l;

        for(typename node_map::const_iterator it = nodes_.begin();
             it != nodes_.end(); ++it)
        {
          typename std::vector < std::pair < geometry::box < Point >,
            Value > >this_leaves = it->second->get_leaves();

          for(typename std::vector < std::pair < geometry::box < Point >,
               Value > >::iterator it_leaf = this_leaves.begin();
               it_leaf != this_leaves.end(); ++it_leaf)
          {

            l.push_back(*it_leaf);

          }

        }

        return l;
      }


      /**
       * \brief Print Rtree subtree (mainly for debug)
       */
      virtual void print(void) const
      {
        std::cerr << " --> Node --------" << std::endl;
        std::cerr << "  Address: " << this << std::endl;
        std::cerr << "  Level: " << level_ << std::endl;
        std::cerr << "  Size: " << nodes_.size() << std::endl;
        std::cerr << "  | ";
        for(typename node_map::const_iterator it = nodes_.begin();
             it != nodes_.end(); ++it)
        {

          if (it->second->get_parent().get() != this) {
            std::cerr << "ERROR - " << this << " is not  " << it->second->
              get_parent().get() << " ";
          }

          std::cerr << "( " << geometry::get < 0 >
            (it->first.min_corner()) << " , " << geometry::get < 1 >
            (it->first.min_corner()) << " ) x ";
          std::cerr << "( " << geometry::get < 0 >
            (it->first.max_corner()) << " , " << geometry::get < 1 >
            (it->first.max_corner()) << " )";
          std::cerr << " | ";
        }
        std::cerr << std::endl;
        std::cerr << " --< Node --------" << std::endl;

        // print child nodes
        std::cerr << " Children: " << std::endl;
        for(typename node_map::const_iterator it = nodes_.begin();
             it != nodes_.end(); ++it) {
          it->second->print();
        }
      }

      /**
       * \brief destructor (virtual because we have virtual functions)
       */
      virtual ~ rtree_node(void)
      {
      }

    private:

      /// parent node
      boost::shared_ptr < rtree_node < Point, Value > >parent_;

      /// level of this node
      unsigned int level_;

      /// child nodes
      node_map nodes_;

    };

  }                             // namespace spatial_index
}                               // namespace boost

#endif // BOOST_SPATIAL_INDEX_RTREE_NODE_HPP
