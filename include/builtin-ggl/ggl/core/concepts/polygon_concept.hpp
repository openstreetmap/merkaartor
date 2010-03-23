// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_CORE_CONCEPTS_POLYGON_CONCEPT_HPP
#define GGL_CORE_CONCEPTS_POLYGON_CONCEPT_HPP

#include <boost/concept_check.hpp>
#include <boost/range/concepts.hpp>

#include <ggl/algorithms/append.hpp>
#include <ggl/algorithms/clear.hpp>
#include <ggl/core/access.hpp>
#include <ggl/core/exterior_ring.hpp>
#include <ggl/core/interior_rings.hpp>
#include <ggl/core/point_type.hpp>
#include <ggl/core/ring_type.hpp>
#include <ggl/core/concepts/point_concept.hpp>
#include <ggl/core/concepts/ring_concept.hpp>

namespace ggl { namespace concept {


#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template <typename P>
struct PolygonChecker
{
    typedef typename point_type<P>::type PNT;
    typedef typename ring_type<P>::type R;
    typedef typename interior_type<P>::type I;

    BOOST_CONCEPT_ASSERT( (boost::RandomAccessRangeConcept<I>) );

    void constraints()
    {
        P* poly;
        R& e = exterior_ring(*poly);
        const R& ce = exterior_ring(*poly);

        I& i = interior_rings(*poly);
        const I& ci = interior_rings(*poly);

        boost::ignore_unused_variable_warning(e);
        boost::ignore_unused_variable_warning(ce);
        boost::ignore_unused_variable_warning(i);
        boost::ignore_unused_variable_warning(ci);
    }
};

} // namespace detail
#endif // DOXYGEN_NO_DETAIL

/*!
    \brief Checks polygon concept, using Boost Concept Check Library and metafunctions
    \ingroup concepts
*/
template <typename P>
struct Polygon : detail::PolygonChecker<P>
{
private:

    typedef typename point_type<P>::type PNT;
    typedef typename ring_type<P>::type R;
    typedef typename interior_type<P>::type I;

    BOOST_CONCEPT_ASSERT( (concept::Point<PNT>) );
    BOOST_CONCEPT_ASSERT( (concept::Ring<R>) );

public:

    /// BCCL macro to check the Polygon concept
    BOOST_CONCEPT_USAGE(Polygon)
    {
        // Check if it can be modified
        P* poly;
        clear(*poly);
        append(*poly, PNT());

        this->constraints();
    }
};


/*!
    \brief Checks Polygon concept (const version)
    \ingroup concepts
    \details The ConstPolygon concept check the same as the Polygon concept,
    but does not check write access.
*/
template <typename P>
struct ConstPolygon : detail::PolygonChecker<P>
{
private:

    typedef typename point_type<P>::type PNT;
    typedef typename ring_type<P>::type R;
    typedef typename interior_type<P>::type I;

    BOOST_CONCEPT_ASSERT( (concept::ConstPoint<PNT>) );
    BOOST_CONCEPT_ASSERT( (concept::ConstRing<R>) );

public:

    /// BCCL macro to check the ConstPolygon concept
    BOOST_CONCEPT_USAGE(ConstPolygon)
    {
        this->constraints();
    }
};

}} // namespace ggl::concept

#endif // GGL_CORE_CONCEPTS_POLYGON_CONCEPT_HPP
