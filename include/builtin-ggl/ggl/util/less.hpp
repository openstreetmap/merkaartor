// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_UTIL_LESS_HPP
#define GGL_UTIL_LESS_HPP


namespace ggl
{

/*!
    \brief Less predicate for usage in e.g. std::map
*/

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template <typename P, std::size_t Dimension, std::size_t DimensionCount>
struct less
{
    static inline bool apply(P const& left, P const& right)
    {
        typedef typename ggl::coordinate_type<P>::type coordinate_type;
        coordinate_type const cleft = ggl::get<Dimension>(left);
        coordinate_type const cright = ggl::get<Dimension>(right);

        return ggl::math::equals(cleft, cright)
                ? less<P, Dimension + 1, DimensionCount>::apply(left, right)
                : cleft < cright;
                ;
    }
};

template <typename P, std::size_t DimensionCount>
struct less<P, DimensionCount, DimensionCount>
{
    static inline bool apply(P const&, P const&)
    {
        return false;
    }
};

}
#endif

template <typename P>
struct less
{
    inline bool operator()(P const& left, P const& right) const
    {
        return detail::less<P, 0,
            ggl::dimension<P>::type::value>::apply(left, right);
    }
};

} // namespace ggl

#endif // GGL_UTIL_LESS_HPP
