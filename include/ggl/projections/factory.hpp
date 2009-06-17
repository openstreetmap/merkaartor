// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_PROJECTIONS_FACTORY_HPP
#define GGL_PROJECTIONS_FACTORY_HPP

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

#include <ggl/projections/impl/factory_entry.hpp>
#include <ggl/projections/parameters.hpp>
#include <ggl/projections/proj/aea.hpp>
#include <ggl/projections/proj/aeqd.hpp>
#include <ggl/projections/proj/airy.hpp>
#include <ggl/projections/proj/aitoff.hpp>
#include <ggl/projections/proj/august.hpp>
#include <ggl/projections/proj/bacon.hpp>
#include <ggl/projections/proj/bipc.hpp>
#include <ggl/projections/proj/boggs.hpp>
#include <ggl/projections/proj/bonne.hpp>
#include <ggl/projections/proj/cass.hpp>
#include <ggl/projections/proj/cc.hpp>
#include <ggl/projections/proj/cea.hpp>
#include <ggl/projections/proj/chamb.hpp>  // control points XY
#include <ggl/projections/proj/collg.hpp>
#include <ggl/projections/proj/crast.hpp>
#include <ggl/projections/proj/denoy.hpp>
#include <ggl/projections/proj/eck1.hpp>
#include <ggl/projections/proj/eck2.hpp>
#include <ggl/projections/proj/eck3.hpp>
#include <ggl/projections/proj/eck4.hpp>
#include <ggl/projections/proj/eck5.hpp>
#include <ggl/projections/proj/eqc.hpp>
#include <ggl/projections/proj/eqdc.hpp>
#include <ggl/projections/proj/fahey.hpp>
#include <ggl/projections/proj/fouc_s.hpp>
#include <ggl/projections/proj/gall.hpp>
#include <ggl/projections/proj/geocent.hpp>
#include <ggl/projections/proj/geos.hpp>
#include <ggl/projections/proj/gins8.hpp>
#include <ggl/projections/proj/gn_sinu.hpp>
#include <ggl/projections/proj/gnom.hpp>
#include <ggl/projections/proj/goode.hpp> // includes two other projections
#include <ggl/projections/proj/gstmerc.hpp>
#include <ggl/projections/proj/hammer.hpp>
#include <ggl/projections/proj/hatano.hpp>
#include <ggl/projections/proj/krovak.hpp>
#include <ggl/projections/proj/imw_p.hpp> // xy functions after inverse
#include <ggl/projections/proj/laea.hpp>
#include <ggl/projections/proj/labrd.hpp>
#include <ggl/projections/proj/lagrng.hpp>
#include <ggl/projections/proj/larr.hpp>
#include <ggl/projections/proj/lask.hpp>
#include <ggl/projections/proj/latlong.hpp>
#include <ggl/projections/proj/lcc.hpp>
#include <ggl/projections/proj/lcca.hpp>
#include <ggl/projections/proj/loxim.hpp>
#include <ggl/projections/proj/lsat.hpp>
#include <ggl/projections/proj/mbtfpp.hpp>
#include <ggl/projections/proj/mbtfpq.hpp>
#include <ggl/projections/proj/mbt_fps.hpp>
#include <ggl/projections/proj/merc.hpp>
#include <ggl/projections/proj/mill.hpp>
#include <ggl/projections/proj/mod_ster.hpp>
#include <ggl/projections/proj/moll.hpp>
#include <ggl/projections/proj/nell.hpp>
#include <ggl/projections/proj/nell_h.hpp>
#include <ggl/projections/proj/nocol.hpp>
#include <ggl/projections/proj/nsper.hpp>
#include <ggl/projections/proj/nzmg.hpp>
#include <ggl/projections/proj/ob_tran.hpp> // includes other projection
#include <ggl/projections/proj/ocea.hpp>
#include <ggl/projections/proj/oea.hpp>
#include <ggl/projections/proj/omerc.hpp>
#include <ggl/projections/proj/ortho.hpp>
#include <ggl/projections/proj/poly.hpp>
#include <ggl/projections/proj/putp2.hpp>
#include <ggl/projections/proj/putp3.hpp>
#include <ggl/projections/proj/putp4p.hpp>
#include <ggl/projections/proj/putp5.hpp>
#include <ggl/projections/proj/putp6.hpp>
#include <ggl/projections/proj/robin.hpp>
#include <ggl/projections/proj/rouss.hpp>
#include <ggl/projections/proj/rpoly.hpp>
#include <ggl/projections/proj/sconics.hpp>
#include <ggl/projections/proj/somerc.hpp>
#include <ggl/projections/proj/stere.hpp>
#include <ggl/projections/proj/sterea.hpp>
#include <ggl/projections/proj/sts.hpp>
#include <ggl/projections/proj/tcc.hpp>
#include <ggl/projections/proj/tcea.hpp>
#include <ggl/projections/proj/tmerc.hpp>
#include <ggl/projections/proj/tpeqd.hpp>
#include <ggl/projections/proj/urm5.hpp>
#include <ggl/projections/proj/urmfps.hpp>
#include <ggl/projections/proj/vandg.hpp>
#include <ggl/projections/proj/vandg2.hpp>
#include <ggl/projections/proj/vandg4.hpp>
#include <ggl/projections/proj/wag2.hpp>
#include <ggl/projections/proj/wag3.hpp>
#include <ggl/projections/proj/wag7.hpp>
#include <ggl/projections/proj/wink1.hpp>
#include <ggl/projections/proj/wink2.hpp>

namespace ggl { namespace projection
{

template <typename LatLong, typename Cartesian, typename Parameters = parameters>
class factory : public impl::base_factory<LatLong, Cartesian, Parameters>
{
private:

    typedef std::map<std::string, boost::shared_ptr<impl::factory_entry<LatLong, Cartesian, Parameters> > > prj_registry;
    prj_registry m_registry;

public:

    factory()
    {
        impl::aea_init(*this);
        impl::aeqd_init(*this);
        impl::airy_init(*this);
        impl::aitoff_init(*this);
        impl::august_init(*this);
        impl::bacon_init(*this);
        impl::bipc_init(*this);
        impl::boggs_init(*this);
        impl::bonne_init(*this);
        impl::cass_init(*this);
        impl::cc_init(*this);
        impl::cea_init(*this);
        impl::chamb_init(*this);
        impl::collg_init(*this);
        impl::crast_init(*this);
        impl::denoy_init(*this);
        impl::eck1_init(*this);
        impl::eck2_init(*this);
        impl::eck3_init(*this);
        impl::eck4_init(*this);
        impl::eck5_init(*this);
        impl::eqc_init(*this);
        impl::eqdc_init(*this);
        impl::fahey_init(*this);
        impl::fouc_s_init(*this);
        impl::gall_init(*this);
        impl::geocent_init(*this);
        impl::geos_init(*this);
        impl::gins8_init(*this);
        impl::gn_sinu_init(*this);
        impl::gnom_init(*this);
        impl::goode_init(*this);
        impl::gstmerc_init(*this);
        impl::hammer_init(*this);
        impl::hatano_init(*this);
        impl::krovak_init(*this);
        impl::imw_p_init(*this);
        impl::labrd_init(*this);
        impl::laea_init(*this);
        impl::lagrng_init(*this);
        impl::larr_init(*this);
        impl::lask_init(*this);
        impl::latlong_init(*this);
        impl::lcc_init(*this);
        impl::lcca_init(*this);
        impl::loxim_init(*this);
        impl::lsat_init(*this);
        impl::mbtfpp_init(*this);
        impl::mbtfpq_init(*this);
        impl::mbt_fps_init(*this);
        impl::merc_init(*this);
        impl::mill_init(*this);
        impl::mod_ster_init(*this);
        impl::moll_init(*this);
        impl::nell_init(*this);
        impl::nell_h_init(*this);
        impl::nocol_init(*this);
        impl::nsper_init(*this);
        impl::nzmg_init(*this);
        impl::ob_tran_init(*this);
        impl::ocea_init(*this);
        impl::oea_init(*this);
        impl::omerc_init(*this);
        impl::ortho_init(*this);
        impl::poly_init(*this);
        impl::putp2_init(*this);
        impl::putp3_init(*this);
        impl::putp4p_init(*this);
        impl::putp5_init(*this);
        impl::putp6_init(*this);
        impl::robin_init(*this);
        impl::rouss_init(*this);
        impl::rpoly_init(*this);
        impl::sconics_init(*this);
        impl::somerc_init(*this);
        impl::stere_init(*this);
        impl::sterea_init(*this);
        impl::sts_init(*this);
        impl::tcc_init(*this);
        impl::tcea_init(*this);
        impl::tmerc_init(*this);
        impl::tpeqd_init(*this);
        impl::urm5_init(*this);
        impl::urmfps_init(*this);
        impl::vandg_init(*this);
        impl::vandg2_init(*this);
        impl::vandg4_init(*this);
        impl::wag2_init(*this);
        impl::wag3_init(*this);
        impl::wag7_init(*this);
        impl::wink1_init(*this);
        impl::wink2_init(*this);
    }

    virtual ~factory() {}

    virtual void add_to_factory(const std::string& name, impl::factory_entry<LatLong, Cartesian, Parameters>* sub)
    {
        m_registry[name].reset(sub);
    }

    inline projection<LatLong, Cartesian>* create_new(const Parameters& parameters)
    {
        typename prj_registry::iterator it = m_registry.find(parameters.name);
        if (it != m_registry.end())
        {
            return it->second->create_new(parameters);
        }

        return 0;
    }
};

}} // namespace ggl::projection

#endif // GGL_PROJECTIONS_FACTORY_HPP
