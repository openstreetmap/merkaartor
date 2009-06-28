// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_PROJECTIONS_IMPL_FACTORY_ENTRY_HPP
#define GGL_PROJECTIONS_IMPL_FACTORY_ENTRY_HPP

#include <string>

#include <ggl/projections/projection.hpp>

namespace ggl { namespace projection { namespace detail {

template <typename LL, typename XY, typename P>
class factory_entry
{
public:

    virtual ~factory_entry() {}
    virtual projection<LL, XY>* create_new(const P& par) const = 0;
};

template <typename LL, typename XY, typename P>
class base_factory
{
public:

    virtual ~base_factory() {}
    virtual void add_to_factory(const std::string& name, factory_entry<LL, XY, P>* sub) = 0;
};

}}} // namespace ggl::projection::impl

#endif // GGL_PROJECTIONS_IMPL_FACTORY_ENTRY_HPP
