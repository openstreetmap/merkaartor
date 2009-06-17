//
// Boost.SpatialIndex - rtree leaf implementation
//
// Copyright 2008 Federico J. Fernandez.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/ for latest version.
//


#ifndef GGL_SPATIAL_INDEX_RTREE_LEAF_HPP
#define GGL_SPATIAL_INDEX_RTREE_LEAF_HPP

#include <ggl/algorithms/area.hpp>
#include <ggl/algorithms/assign.hpp>
#include <ggl/algorithms/combine.hpp>

#include <ggl/spatial_index/rtree_node.hpp>

namespace ggl
{
namespace spatial_index
{

template <typename Box, typename Point, typename Value >
class rtree_leaf : public rtree_node<Box, Point, Value>
{
public:
  /// container type for the leaves
    typedef boost::shared_ptr<rtree_node<Box, Point, Value> > node_pointer;
    typedef std::vector< std::pair<Box, Value> > leaf_map;

  /**
   * \brief Creates an empty leaf
   */
  inline rtree_leaf()
  {
  }


  /**
   * \brief Creates a new leaf, with 'parent' as parent
   */
  inline rtree_leaf(node_pointer const& parent)
    : rtree_node<Box, Point, Value> (parent, 0)
  {
  }


  /**
   * \brief Search for elements in 'e' in the Rtree. Add them to r.
   *        If exact_match is true only return the elements having as
   *        key the box 'e'. Otherwise return everything inside 'e'.
   */
  virtual void find(Box const&e, std::deque<Value>& r, const bool exact_match)
  {
    for(typename leaf_map::const_iterator it = m_nodes.begin();
         it != m_nodes.end(); ++it)
    {
      if (exact_match)
      {
          if (ggl::equals(it->first, e))
          {
              r.push_back(it->second);
          }
      }
      else
      {
        if (is_overlapping(it->first, e))
        {
          r.push_back(it->second);
        }
      }
    }
  }


  /**
   * \brief Compute bounding box for this leaf
   */
  virtual Box compute_box(void) const
  {
    if (m_nodes.empty())
    {
      return Box ();
    }

    Box r;
    ggl::assign_inverse(r);
    for(typename leaf_map::const_iterator it = m_nodes.begin(); it != m_nodes.end(); ++it)
    {
        ggl::combine(r, it->first);
    }
    return r;
  }


  /**
   * \brief True if we are a leaf
   */
  virtual bool is_leaf() const
  {
    return true;
  }


  /**
   * \brief Number of elements in the tree
   */
  virtual unsigned int elements() const
  {
    return m_nodes.size();
  }


  /**
   * \brief Insert a new element, with key 'e' and value 'v'
   */
  virtual void insert(Box const&e, Value const& v)
  {
    m_nodes.push_back(std::make_pair(e, v));
  }


  /**
   * \brief Proyect leaves of this node.
   */
  virtual std::vector< std::pair<Box, Value> > get_leaves() const
  {
    return m_nodes;
  }


  /**
   * \brief Add a new child (node) to this node
   */
  virtual void add_node(Box const&, node_pointer const&)
  {
    throw std::logic_error("Can't add node to leaf node.");
  }


  /**
   * \brief Add a new leaf to this node
   */
  virtual void add_value(Box const& b, Value const& v)
  {
    m_nodes.push_back(std::make_pair(b, v));
  }


  /**
   * \brief Proyect value in position 'i' in the nodes container
   */
  virtual Value get_value(unsigned int i) const
  {
    return m_nodes[i].second;
  }


  /**
   * \brief Box projector for leaf
   */
  virtual Box get_box(unsigned int i) const
  {
    return m_nodes[i].first;
  }


  /**
   * \brief Remove value with key 'k' in this leaf
   */
  virtual void remove(Box const& k)
  {

    for(typename leaf_map::iterator it = m_nodes.begin();
         it != m_nodes.end(); ++it)
    {
      if (ggl::equals(it->first, k))
      {
        m_nodes.erase(it);
        return;
      }
    }
    throw std::logic_error("Node not found.");
  }


  /**
   * \brief Remove value in this leaf
   */
  virtual void remove(const Value &v)
  {

    for(typename leaf_map::iterator it = m_nodes.begin();
         it != m_nodes.end(); ++it) {
      if (it->second == v) {
        m_nodes.erase(it);
        return;
      }
    }
    throw std::logic_error("Node not found.");
  }


  /**
   * \brief Proyect boxes from this node
   */
  virtual std::vector<Box> get_boxes() const
  {
    std::vector<Box> r;
    for(typename leaf_map::const_iterator it = m_nodes.begin();
        it != m_nodes.end(); ++it)
    {
      r.push_back(it->first);
    }
    return r;
  }


  /**
   * \brief Print leaf (mainly for debug)
   */
  virtual void print(void) const
  {
    std::cerr << "\t" << " --> Leaf --------" << std::endl;
    std::cerr << "\t" << "  Size: " << m_nodes.size() << std::endl;
    for(typename leaf_map::const_iterator it = m_nodes.begin();
         it != m_nodes.end(); ++it)
    {

      std::cerr << "\t" << "  | ";
      std::cerr << "( " << ggl::get<min_corner, 0>
        (it->first) << " , " << ggl::get<min_corner, 1>
        (it->first) << " ) x ";
      std::cerr << "( " << ggl::get<max_corner, 0>
        (it->first) << " , " << ggl::get<max_corner, 1>
        (it->first) << " )";
      std::cerr << " -> ";
      std::cerr << it->second;
      std::cerr << " | " << std::endl;;
    }
    std::cerr << "\t" << " --< Leaf --------" << std::endl;
  }

private:
  /// leaves of this node
  leaf_map m_nodes;
};



} // namespace spatial_index
} // namespace ggl

#endif // GGL_SPATIAL_INDEX_RTREE_LEAF_HPP
