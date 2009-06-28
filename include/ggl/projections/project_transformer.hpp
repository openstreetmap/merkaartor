// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_STRATEGY_PROJECT_TRANSFORMER_HPP
#define GGL_STRATEGY_PROJECT_TRANSFORMER_HPP


#include <boost/shared_ptr.hpp>

#include <ggl/projections/factory.hpp>
#include <ggl/projections/parameters.hpp>



namespace ggl { namespace projection
{
/*!
    \brief Transformation strategy to do transform using a Map Projection
    \ingroup transform
    \tparam LatLong first point type
    \tparam Cartesian second point type
 */
template <typename LatLong, typename Cartesian>
struct project_transformer
{
    typedef boost::shared_ptr<projection<LatLong, Cartesian> > projection_ptr;

    projection_ptr m_prj;

    inline project_transformer(projection_ptr& prj)
        : m_prj(prj)
    {}

    inline project_transformer(const std::string& par)
    {
        factory<LatLong, Cartesian, parameters> fac;
        m_prj.reset(fac.create_new(init(par)));
    }

    inline bool operator()(const LatLong& p1, Cartesian& p2) const
    {
        // Latlong (LatLong -> Cartesian) will be projected, rest will be copied.
        // So first copy third or higher dimensions
        ggl::detail::copy::copy_coordinates<LatLong, Cartesian, 2,
                ggl::dimension<Cartesian>::value> ::copy(p1, p2);
        return m_prj->forward(p1, p2);
    }

};

}} // namespace ggl::projection


#endif // GGL_STRATEGY_PROJECT_TRANSFORMER_HPP
