#ifndef _PROJECTIONS_FACTORY_HPP
#define _PROJECTIONS_FACTORY_HPP

// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry/projections/parameters.hpp>
#include <geometry/projections/impl/factory_entry.hpp>

#include <geometry/projections/proj/aea.hpp>
#include <geometry/projections/proj/aeqd.hpp>
#include <geometry/projections/proj/airy.hpp>
#include <geometry/projections/proj/aitoff.hpp>
#include <geometry/projections/proj/august.hpp>
#include <geometry/projections/proj/bacon.hpp>
#include <geometry/projections/proj/bipc.hpp>
#include <geometry/projections/proj/boggs.hpp>
#include <geometry/projections/proj/bonne.hpp>
#include <geometry/projections/proj/cass.hpp>
#include <geometry/projections/proj/cc.hpp>
#include <geometry/projections/proj/cea.hpp>
#include <geometry/projections/proj/chamb.hpp>  // control points XY
#include <geometry/projections/proj/collg.hpp>
#include <geometry/projections/proj/crast.hpp>
#include <geometry/projections/proj/denoy.hpp>
#include <geometry/projections/proj/eck1.hpp>
#include <geometry/projections/proj/eck2.hpp>
#include <geometry/projections/proj/eck3.hpp>
#include <geometry/projections/proj/eck4.hpp>
#include <geometry/projections/proj/eck5.hpp>
#include <geometry/projections/proj/eqc.hpp>
#include <geometry/projections/proj/eqdc.hpp>
#include <geometry/projections/proj/fahey.hpp>
#include <geometry/projections/proj/fouc_s.hpp>
#include <geometry/projections/proj/gall.hpp>
#include <geometry/projections/proj/geocent.hpp>
#include <geometry/projections/proj/geos.hpp>
#include <geometry/projections/proj/gins8.hpp>
#include <geometry/projections/proj/gn_sinu.hpp>
#include <geometry/projections/proj/gnom.hpp>
#include <geometry/projections/proj/goode.hpp> // includes two other projections
#include <geometry/projections/proj/gstmerc.hpp>
#include <geometry/projections/proj/hammer.hpp>
#include <geometry/projections/proj/hatano.hpp>
#include <geometry/projections/proj/krovak.hpp>
#include <geometry/projections/proj/imw_p.hpp> // xy functions after inverse
#include <geometry/projections/proj/laea.hpp>
#include <geometry/projections/proj/labrd.hpp>
#include <geometry/projections/proj/lagrng.hpp>
#include <geometry/projections/proj/larr.hpp>
#include <geometry/projections/proj/lask.hpp>
#include <geometry/projections/proj/latlong.hpp>
#include <geometry/projections/proj/lcc.hpp>
#include <geometry/projections/proj/lcca.hpp>
#include <geometry/projections/proj/loxim.hpp>
#include <geometry/projections/proj/lsat.hpp>
#include <geometry/projections/proj/mbtfpp.hpp>
#include <geometry/projections/proj/mbtfpq.hpp>
#include <geometry/projections/proj/mbt_fps.hpp>
#include <geometry/projections/proj/merc.hpp>
#include <geometry/projections/proj/mill.hpp>
#include <geometry/projections/proj/mod_ster.hpp>
#include <geometry/projections/proj/moll.hpp>
#include <geometry/projections/proj/nell.hpp>
#include <geometry/projections/proj/nell_h.hpp>
#include <geometry/projections/proj/nocol.hpp>
#include <geometry/projections/proj/nsper.hpp>
#include <geometry/projections/proj/nzmg.hpp>
#include <geometry/projections/proj/ob_tran.hpp> // includes other projection
#include <geometry/projections/proj/ocea.hpp>
#include <geometry/projections/proj/oea.hpp>
#include <geometry/projections/proj/omerc.hpp>
#include <geometry/projections/proj/ortho.hpp>
#include <geometry/projections/proj/poly.hpp>
#include <geometry/projections/proj/putp2.hpp>
#include <geometry/projections/proj/putp3.hpp>
#include <geometry/projections/proj/putp4p.hpp>
#include <geometry/projections/proj/putp5.hpp>
#include <geometry/projections/proj/putp6.hpp>
#include <geometry/projections/proj/robin.hpp>
#include <geometry/projections/proj/rouss.hpp>
#include <geometry/projections/proj/rpoly.hpp>
#include <geometry/projections/proj/sconics.hpp>
#include <geometry/projections/proj/somerc.hpp>
#include <geometry/projections/proj/stere.hpp>
#include <geometry/projections/proj/sterea.hpp>
#include <geometry/projections/proj/sts.hpp>
#include <geometry/projections/proj/tcc.hpp>
#include <geometry/projections/proj/tcea.hpp>
#include <geometry/projections/proj/tmerc.hpp>
#include <geometry/projections/proj/tpeqd.hpp>
#include <geometry/projections/proj/urm5.hpp>
#include <geometry/projections/proj/urmfps.hpp>
#include <geometry/projections/proj/vandg.hpp>
#include <geometry/projections/proj/vandg2.hpp>
#include <geometry/projections/proj/vandg4.hpp>
#include <geometry/projections/proj/wag2.hpp>
#include <geometry/projections/proj/wag3.hpp>
#include <geometry/projections/proj/wag7.hpp>
#include <geometry/projections/proj/wink1.hpp>
#include <geometry/projections/proj/wink2.hpp>

#include <boost/shared_ptr.hpp>

#include <map>

namespace projection
{
	template <typename LL, typename XY, typename PAR = parameters>
	class factory : public impl::base_factory<LL, XY, PAR>
	{
		private :
			typedef std::map<std::string, boost::shared_ptr<impl::factory_entry<LL, XY, PAR> > > MAP;
			MAP m_registry;

		public :
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

			virtual void add_to_factory(const std::string& name, impl::factory_entry<LL, XY, PAR>* sub)
			{
				m_registry[name].reset(sub);
			}

			inline projection<LL, XY>* create_new(const PAR& parameters)
			{
				typename MAP::iterator it = m_registry.find(parameters.name);
				if (it != m_registry.end())
				{
					return it->second->create_new(parameters);
				}
				return NULL;
			}


	};


};

#endif
