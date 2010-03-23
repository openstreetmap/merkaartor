// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_CORE_EXCEPTION_HPP
#define GGL_CORE_EXCEPTION_HPP

#include <exception>

namespace ggl {

/*!
\brief Base exception class for GGL
\ingroup core
*/
struct exception : public std::exception
{
};

} // namespace ggl

#endif // GGL_CORE_EXCEPTION_HPP
