// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_PROJECTIONS_IMPL_BASE_DYNAMIC_HPP
#define GGL_PROJECTIONS_IMPL_BASE_DYNAMIC_HPP

#include <string>

#include <boost/concept_check.hpp>

#include <ggl/projections/projection.hpp>

namespace ggl { namespace projection {

#ifndef DOXYGEN_NO_IMPL
namespace impl
{

// Base-virtual-forward
template <typename C, typename LL, typename XY, typename P>
class base_v_f : public projection<LL, XY>
{
protected:

    typedef typename projection<LL, XY>::LL_T LL_T;
    typedef typename projection<LL, XY>::XY_T XY_T;

public:

    base_v_f(const P& params) : m_proj(params) {}

    virtual P params() const {return m_proj.params();}
    
    virtual bool forward(const LL& ll, XY& xy) const
    {
        return m_proj.forward(ll, xy);
    }

    virtual void fwd(LL_T& lp_lon, LL_T& lp_lat, XY_T& xy_x, XY_T& xy_y) const
    {
        m_proj.fwd(lp_lon, lp_lat, xy_x, xy_y);
    }

    virtual bool inverse(const XY& xy, LL& ll) const
    {
        boost::ignore_unused_variable_warning(xy);
        boost::ignore_unused_variable_warning(ll);

		// exception?
        return false;
    }
    virtual void inv(XY_T& xy_x, XY_T& xy_y, LL_T& lp_lon, LL_T& lp_lat) const
    {
        boost::ignore_unused_variable_warning(xy_x);
        boost::ignore_unused_variable_warning(xy_y);
        boost::ignore_unused_variable_warning(lp_lon);
        boost::ignore_unused_variable_warning(lp_lat);
        // exception?
    }

    virtual std::string name() const
    {
        return m_proj.name();
    }

protected:

    C m_proj;
};

// Base-virtual-forward/inverse
template <typename C, typename LL, typename XY, typename P>
class base_v_fi : public base_v_f<C, LL, XY, P>
{
private:

    typedef typename base_v_f<C, LL, XY, P>::LL_T LL_T;
    typedef typename base_v_f<C, LL, XY, P>::XY_T XY_T;

public :

    base_v_fi(const P& params) : base_v_f<C, LL, XY, P>(params) {}

    virtual bool inverse(const XY& xy, LL& ll) const
    {
        return this->m_proj.inverse(xy, ll);
    }

    void inv(XY_T& xy_x, XY_T& xy_y, LL_T& lp_lon, LL_T& lp_lat) const
    {
        this->m_proj.inv(xy_x, xy_y, lp_lon, lp_lat);
    }
};

} // namespace impl
#endif // DOXYGEN_NO_IMPL

}} // namespace ggl::projection

#endif // GGL_PROJECTIONS_IMPL_BASE_DYNAMIC_HPP
