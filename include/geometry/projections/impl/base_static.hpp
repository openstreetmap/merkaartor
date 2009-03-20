#ifndef _PROJECTIONS_BASE_STATIC_HPP
#define _PROJECTIONS_BASE_STATIC_HPP

// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry/projections/impl/pj_fwd.hpp>
#include <geometry/projections/impl/pj_inv.hpp>


namespace projection
{

	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		// Base-template-forward
		template <typename PRJ, typename LL, typename XY, typename PAR>
		struct base_t_f
		{
			protected :
				// some projections do not work with float -> wrong results
				// todo: make traits which select <double> from int/float/double and else selects T

				//typedef typename geometry::coordinate_type<LL>::type LL_T;
				//typedef typename geometry::coordinate_type<XY>::type XY_T;
				typedef double LL_T;
				typedef double XY_T;
				PAR m_par;
				const PRJ& m_prj;

			public :
				inline base_t_f(const PRJ& prj, const PAR& par) : m_par(par), m_prj(prj) {}
				inline bool forward(const LL& lp, XY& xy) const
				{
					try
					{
						pj_fwd(m_prj, m_par, lp, xy);
						return true;
					}
					catch(...)
					{
						return false;
					}
				}

				inline std::string name() const { return this->m_par.name; }

		};

		// Base-template-forward/inverse
		template <typename PRJ, typename LL, typename XY, typename PAR>
		struct base_t_fi : public base_t_f<PRJ, LL, XY, PAR>
		{
			public :
				inline base_t_fi(const PRJ& prj, const PAR& par) : base_t_f<PRJ, LL, XY, PAR>(prj, par) {}

				inline bool inverse(const XY& xy, LL& lp) const
				{
					try
					{
						pj_inv(this->m_prj, this->m_par, xy, lp);
						return true;
					}
					catch(...)
					{
						return false;
					}
				}
		};
	}
	#endif

}



#endif

