// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_MULTI_ALGORITHMS_SECTIONALIZE_HPP
#define GGL_MULTI_ALGORITHMS_SECTIONALIZE_HPP

#include <cstddef>
#include <vector>

#include <boost/concept/requires.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/algorithms/sectionalize.hpp>



namespace ggl
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace sectionalize {


template <typename MultiGeometry, typename Sections, std::size_t DimensionCount, typename Policy>
struct sectionalize_multi
{
    static inline void apply(MultiGeometry const& multi, Sections& sections)
    {
        int multi_index = 0;
        for (typename boost::range_const_iterator<MultiGeometry>::type
                    it = boost::begin(multi);
            it != boost::end(multi);
            ++it, ++multi_index)
        {
            Policy::apply(*it, sections, multi_index);
        }
    }
};




}} // namespace detail::sectionalize
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename MultiPolygon, typename Sections, std::size_t DimensionCount, std::size_t MaxCount>
struct sectionalize<multi_polygon_tag, MultiPolygon, Sections, DimensionCount, MaxCount>
    : detail::sectionalize::sectionalize_multi
        <
            MultiPolygon,
            Sections,
            DimensionCount,
            detail::sectionalize::sectionalize_polygon
                <
                    typename boost::range_value<MultiPolygon>::type,
                    Sections,
                    DimensionCount,
                    MaxCount
                >
        >

{};


} // namespace dispatch
#endif




} // namespace ggl

#endif // GGL_MULTI_ALGORITHMS_SECTIONALIZE_HPP
