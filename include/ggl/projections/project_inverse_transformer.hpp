// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_STRATEGY_PROJECT_INVERSE_TRANSFORMER_HPP
#define GGL_STRATEGY_PROJECT_INVERSE_TRANSFORMER_HPP


#include <boost/shared_ptr.hpp>

#include <ggl/core/coordinate_dimension.hpp>
#include <ggl/projections/factory.hpp>
#include <ggl/projections/parameters.hpp>
#include <ggl/util/copy.hpp>


namespace ggl { namespace projection
{


/*!
    \brief Transformation strategy to do transform using a Map Projection
    \ingroup transform
    \tparam Cartesian first point type
    \tparam LatLong second point type
 */
template <typename Cartesian, typename LatLong>
struct project_inverse_transformer
{
    typedef boost::shared_ptr<projection<LatLong, Cartesian> > projection_ptr;

    projection_ptr m_prj;

    /// Constructor using a shared-pointer-to-projection_ptr
    inline project_inverse_transformer(projection_ptr& prj)
        : m_prj(prj)
    {}

    /// Constructor using a string
    inline project_inverse_transformer(const std::string& par)
    {
        factory<LatLong, Cartesian, parameters> fac;
        m_prj.reset(fac.create_new(init(par)));
    }

    /// Constructor using Parameters
    template <typename Parameters>
    inline project_inverse_transformer(const Parameters& par)
    {
        factory<LatLong, Cartesian, Parameters> fac;
        m_prj.reset(fac.create_new(par));
    }

    /// Transform operator
    inline bool operator()(const Cartesian& p1, LatLong& p2) const
    {
        // Latlong (LL -> XY) will be projected, rest will be copied.
        // So first copy third or higher dimensions
        ggl::detail::copy::copy_coordinates<Cartesian, LatLong, 2,
                ggl::dimension<Cartesian>::value> ::copy(p1, p2);
        return m_prj->inverse(p1, p2);
    }

};

}} // namespace ggl::projection


#endif // GGL_STRATEGY_PROJECT_INVERSE_TRANSFORMER_HPP
