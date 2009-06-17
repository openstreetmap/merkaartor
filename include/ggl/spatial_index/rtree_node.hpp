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


#ifndef GGL_SPATIAL_INDEX_RTREE_NODE_HPP
#define GGL_SPATIAL_INDEX_RTREE_NODE_HPP

#include <ggl/algorithms/area.hpp>
#include <ggl/algorithms/equals.hpp>

#include <boost/shared_ptr.hpp>

#include <ggl/spatial_index/helpers.hpp>


namespace ggl
{
namespace spatial_index
{

/// forward declaration
template <typename Box, typename Point, typename Value >
class rtree_leaf;

template <typename Box, typename Point, typename Value >
class rtree_node
{
    public:

        typedef boost::shared_ptr<rtree_node<Box, Point, Value> > node_pointer;
        typedef boost::shared_ptr<rtree_leaf<Box, Point, Value> > leaf_pointer;

      /// type for the node map
      typedef std::vector<std::pair<Box, node_pointer > > node_map;

      /**
       * \brief Creates a default node (needed for the containers)
       */
      rtree_node(void)
      {
      }

      /**
       * \brief Creates a node with 'parent' as parent and 'level' as its level
       */
      rtree_node(node_pointer const& parent,
                 unsigned int const& level)
        : m_parent(parent), m_level(level)
      {
      }


  /**
   * \brief Level projector
   */
  virtual unsigned int get_level() const
  {
    return m_level;
  }

  /**
   * \brief Number of elements in the subtree
   */
  virtual unsigned int elements() const
  {
    return m_nodes.size();
  }


  /**
   * \brief Project first element, to replace root in case of condensing
   */
  inline node_pointer first_element(void) const
  {
    if (m_nodes.size() == 0) {
      throw std::logic_error("first_element in empty node");
    }
    return m_nodes.begin()->second;
  }


  /**
   * \brief True if it is a leaf node
   */
  virtual bool is_leaf() const
  {
    return false;
  }


  /**
   * \brief Proyector for the 'i' node
   */
  node_pointer get_node(unsigned int i)
  {
    return m_nodes[i].second;
  }


  /**
   * \brief Search for elements in 'e' in the Rtree. Add them to r.
   *        If exact_match is true only return the elements having as
   *        key the box 'e'. Otherwise return everything inside 'e'.
   */
  virtual void find(Box const&e, std::deque<Value> &r, const bool exact_match)
  {
    for(typename node_map::const_iterator it = m_nodes.begin();
         it != m_nodes.end(); ++it)
    {
      if (is_overlapping(it->first, e))
      {
        it->second->find(e, r, exact_match);
      }
    }
  }


  /**
   * \brief Return in 'result' all the leaves inside 'e'
   */
  void find_leaves(Box const&e, typename std::vector<node_pointer> &result) const
  {
    for(typename node_map::const_iterator it = m_nodes.begin();
         it != m_nodes.end(); ++it)
    {
      if (is_overlapping(it->first, e))
      {
        if (it->second->is_leaf())
        {
          result.push_back(it->second);
        }
        else
        {
          it->second->find_leaves(e, result);
        }
      }
    }
  }


  /**
   * \brief Compute bounding box for this node
   */
  virtual Box compute_box(void) const
  {
    if (m_nodes.empty())
    {
      return Box ();
    }

    Box r;
    ggl::assign_inverse(r);
    for(typename node_map::const_iterator it = m_nodes.begin(); it != m_nodes.end(); ++it)
    {
        ggl::combine(r, it->first);
    }
    return r;
  }


  /**
   * \brief Insert a value (not allowed for a node, only on leaves)
   */
  virtual void insert(Box const&, Value const&)
  {
    throw std::logic_error("Insert in node!");
  }


  /**
   * \brief Get the envelopes of a node
   */
  virtual std::vector<Box> get_boxes(void) const
  {
    std::vector<Box> r;
    for(typename node_map::const_iterator it = m_nodes.begin(); it != m_nodes.end(); ++it)
    {
      r.push_back(it->first);
    }
    return r;
  }


  /**
   * \brief Recompute the bounding box
   */
  void adjust_box(node_pointer const& n)
  {
    unsigned int index = 0;
    for(typename node_map::iterator it = m_nodes.begin();
         it != m_nodes.end(); ++it, index++)
    {
      if (it->second.get() == n.get())
      {
        m_nodes[index] = std::make_pair(n->compute_box(), n);
        return;
      }
    }
  }


  /**
   * \brief Remove elements inside the box 'k'
   */
  virtual void remove(Box const& k)
  {
    for(typename node_map::iterator it = m_nodes.begin();
         it != m_nodes.end(); ++it)
    {
        if (ggl::equals(it->first, k))
        {
            m_nodes.erase(it);
            return;
        }
    }
  }

  /**
   * \brief Remove value in this leaf
   */
  virtual void remove(const Value &)
  {
    throw std::logic_error("Can't remove a non-leaf node by value.");
  }


  /**
   * \brief Replace the node in the m_nodes vector and recompute the box
   */
  void replace_node(node_pointer const& leaf, node_pointer& new_leave)
  {
    unsigned int index = 0;
    for(typename node_map::iterator it = m_nodes.begin(); it != m_nodes.end(); ++it, index++)
    {
      if (it->second.get() == leaf.get())
      {
        m_nodes[index] = std::make_pair(new_leave->compute_box(), new_leave);
        new_leave->update_parent(new_leave);
        return;
      }
    }
    throw std::logic_error("Node not found.");
  }


  /**
   * \brief Add a child to this node
   */
  virtual void add_node(Box const& b, node_pointer const& n)
  {
    m_nodes.push_back(std::make_pair(b, n));
    n->update_parent(n);
  }


  /**
   * \brief add a value (not allowed in nodes, only on leaves)
   */
  virtual void add_value(Box const&, Value const&)
  {
    throw std::logic_error("Can't add value to non-leaf node.");
  }


  /**
   * \brief Add a child leaf to this node
   */
  inline void add_leaf_node(Box const& b, leaf_pointer const& leaf)
  {
    m_nodes.push_back(std::make_pair(b, leaf));
  }


  /**
   * \brief Choose a node suitable for adding 'e'
   */
  node_pointer choose_node(Box const& e)
  {
    if (m_nodes.size() == 0)
    {
      throw std::logic_error
        ("Empty node trying to choose the least enlargement node.");
    }
    bool first = true;
    double min_area;
    double min_diff_area;
    node_pointer chosen_node;

    // check for the least enlargement
    for(typename node_map::const_iterator it = m_nodes.begin();
         it != m_nodes.end(); ++it) {

      double diff_area =
        compute_union_area(e, it->first) - ggl::area(it->first);

      if (first)
      {
        // it's the first time, we keep the first
        min_diff_area = diff_area;
        min_area = ggl::area(it->first);
        chosen_node = it->second;

        first = false;
      }
      else
      {
        if (diff_area < min_diff_area)
        {
          min_diff_area = diff_area;
          min_area = ggl::area(it->first);
          chosen_node = it->second;
        }
        else
        {
          if (diff_area == min_diff_area)
          {
            if (ggl::area(it->first) < min_area)
            {
              min_diff_area = diff_area;
              min_area = ggl::area(it->first);
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
  virtual void empty_nodes()
  {
    m_nodes.clear();
  }


  /**
   * \brief Projector for parent
   */
  inline node_pointer get_parent() const
  {
    return m_parent;
  }


  /**
   * \brief Update the parent of all the childs
   */
  void update_parent(node_pointer const& p)
  {
    for(typename node_map::iterator it = m_nodes.begin(); it != m_nodes.end(); ++it)
    {
      it->second->set_parent(p);
    }
  }


  /**
   * \brief Set parent
   */
  void set_parent(node_pointer const& p)
  {
    m_parent = p;
  }


  /**
   * \brief Value projector for leaf_node (not allowed for non-leaf nodes)
   */
  virtual Value get_value(unsigned int) const
  {
    throw std::logic_error("No values in a non-leaf node.");
  }

  /**
   * \brief Box projector for node 'i'
   */
  virtual Box get_box(unsigned int i) const
  {
    return m_nodes[i].first;
  }


  /**
   * \brief Box projector for node pointed by 'leaf'
   */
  virtual Box get_box(node_pointer const& leaf) const
  {
    for(typename node_map::const_iterator it = m_nodes.begin();
         it != m_nodes.end(); ++it)
    {
      if (it->second.get() == leaf.get())
      {
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
    return m_nodes;
  }


  /**
   * \brief Get leaves for a node
   */
  virtual std::vector<std::pair<Box, Value> > get_leaves(void) const
  {
    std::vector<std::pair<Box, Value> > leaf;

    for(typename node_map::const_iterator it = m_nodes.begin();
         it != m_nodes.end(); ++it)
    {
      typename std::vector<std::pair<Box, Value> > this_leaves = it->second->get_leaves();

      for(typename std::vector<std::pair<Box, Value> >::iterator it_leaf = this_leaves.begin();
           it_leaf != this_leaves.end(); ++it_leaf)
      {

        leaf.push_back(*it_leaf);

      }

    }

    return leaf;
  }


  /**
   * \brief Print Rtree subtree (mainly for debug)
   */
  virtual void print(void) const
  {
    std::cerr << " --> Node --------" << std::endl;
    std::cerr << "  Address: " << this << std::endl;
    std::cerr << "  Level: " << m_level << std::endl;
    std::cerr << "  Size: " << m_nodes.size() << std::endl;
    std::cerr << "  | ";
    for(typename node_map::const_iterator it = m_nodes.begin();
         it != m_nodes.end(); ++it)
    {

      if (it->second->get_parent().get() != this) {
        std::cerr << "ERROR - " << this << " is not  " << it->second->
          get_parent().get() << " ";
      }

      std::cerr << "( " << ggl::get<min_corner, 0>
        (it->first) << " , " << ggl::get<min_corner, 1>
        (it->first) << " ) x ";
      std::cerr << "( " << ggl::get<max_corner, 0>
        (it->first) << " , " << ggl::get<max_corner, 1>
        (it->first) << " )";
      std::cerr << " | ";
    }
    std::cerr << std::endl;
    std::cerr << " --< Node --------" << std::endl;

    // print child nodes
    std::cerr << " Children: " << std::endl;
    for(typename node_map::const_iterator it = m_nodes.begin();
         it != m_nodes.end(); ++it) {
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
  node_pointer m_parent;

  /// level of this node
  unsigned int m_level;

  /// child nodes
  node_map m_nodes;

};

} // namespace spatial_index
} // namespace ggl

#endif // GGL_SPATIAL_INDEX_RTREE_NODE_HPP
