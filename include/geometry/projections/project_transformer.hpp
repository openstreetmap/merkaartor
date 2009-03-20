// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_STRATEGY_PROJECT_TRANSFORMER_HPP
#define _GEOMETRY_STRATEGY_PROJECT_TRANSFORMER_HPP


#include <geometry/projections/factory.hpp>
#include <geometry/projections/parameters.hpp>

#include <boost/shared_ptr.hpp>


namespace projection
{
	/*!
		\brief Transformation strategy to do transform using a Map Projection
		\ingroup transform
		\tparam P1 first point type
		\tparam P2 second point type
	 */
	template <typename LL, typename XY>
	struct project_transformer
	{
		typedef projection<LL, XY> PRJ;

		boost::shared_ptr<PRJ> m_prj;

		inline project_transformer(PRJ* prj)
			: m_prj(prj)
		{}

		inline project_transformer(const std::string& par)
		{
			factory<LL, XY, parameters> fac;
			m_prj.reset(fac.create_new(init(par)));
		}

		inline bool operator()(const LL& p1, XY& p2) const
		{
			return m_prj->forward(p1, p2);
		}

	};
}


#endif // _GEOMETRY_STRATEGY_PROJECT_TRANSFORMER_HPP
