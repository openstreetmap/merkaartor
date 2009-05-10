#ifndef _PROJECTIONS_BASE_DYNAMIC_HPP
#define _PROJECTIONS_BASE_DYNAMIC_HPP

// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry/projections/projection.hpp>


namespace projection
{

	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		// Base-virtual-forward
		template <typename C, typename LL, typename XY, typename PAR>
		class base_v_f : public projection<LL, XY>
		{
			protected :
				typedef typename projection<LL, XY>::LL_T LL_T;
				typedef typename projection<LL, XY>::XY_T XY_T;
				C m_proj;

			public :
				base_v_f(const PAR& par) : m_proj(par) {}

				virtual PAR params() {return m_proj.params();};
				virtual bool forward(const LL& ll, XY& xy) const
				{
					return m_proj.forward(ll, xy);
				}
				virtual void fwd(LL_T& lp_lon, LL_T& lp_lat, XY_T& xy_x, XY_T& xy_y) const
				{
					m_proj.fwd(lp_lon, lp_lat, xy_x, xy_y);
				}

				virtual bool inverse(const XY& /*xy*/, LL& /*ll*/) const
				{
					// exception?
					return false;
				}
				virtual void inv(XY_T& /*xy_x*/, XY_T& /*xy_y*/, LL_T& /*lp_lon*/, LL_T& /*lp_lat*/) const
				{
					// exception?
				}

				virtual std::string name() const
				{
					return m_proj.name();
				}

		};

		// Base-virtual-forward/inverse
		template <typename C, typename LL, typename XY, typename PAR>
		class base_v_fi : public base_v_f<C, LL, XY, PAR>
		{
				typedef typename base_v_f<C, LL, XY, PAR>::LL_T LL_T;
				typedef typename base_v_f<C, LL, XY, PAR>::XY_T XY_T;

			public :
				base_v_fi(const PAR& par) : base_v_f<C, LL, XY, PAR>(par) {}

				virtual bool inverse(const XY& xy, LL& ll) const
				{
					return this->m_proj.inverse(xy, ll);
				}
				void inv(XY_T& xy_x, XY_T& xy_y, LL_T& lp_lon, LL_T& lp_lat) const
				{
					this->m_proj.inv(xy_x, xy_y, lp_lon, lp_lat);
				}

		};
	}
	#endif

}



#endif

