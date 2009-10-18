// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_GEOMETRIES_REGISTER_POINT_HPP
#define GGL_GEOMETRIES_REGISTER_POINT_HPP



// This file implements a "macro party", nevertheless very useful for registration of custom geometry types

#ifndef DOXYGEN_NO_SPECIALIZATIONS

// Starting point, specialize basic traits necessary to register a point
// (the 'accessor' is technically not specialization but definition, specialized in another macro)
#define GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, Dim, CoordinateType, CoordinateSystem) \
    template<> struct tag<Point> { typedef point_tag type; }; \
    template<> struct dimension<Point> : boost::mpl::int_<Dim> {}; \
    template<> struct coordinate_type<Point> { typedef CoordinateType type; }; \
    template<> struct coordinate_system<Point> { typedef CoordinateSystem type; }; \
    template<int I> struct Point##accessor {};


// Non Const version
#define GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS(Point, CoordinateType) \
    template<> struct access<Point> { \
         template <int I> \
        static inline CoordinateType get(const Point& p) { return Point##accessor<I>::get(p); } \
        template <int I> \
        static inline void set(Point& p, const CoordinateType& value) { Point##accessor<I>::set(p, value); } \
    };

// Const version
#define GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST(Point, CoordinateType) \
    template<> struct access<Point> { \
         template <int I> \
        static inline CoordinateType get(const Point& p) { return Point##accessor<I>::get(p); } \
    };




// Specialize the point-specific-accessor class, declared below, per dimension
#define GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR(Point, Dim, CoordinateType, Get, Set) \
    template<> struct Point##accessor< Dim > \
    { \
        static inline CoordinateType get(const Point& p) { return  p. Get; } \
        static inline void set(Point& p, const CoordinateType& value) { p. Set = value; } \
    };


// Const version
#define GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_CONST(Point, Dim, CoordinateType, Get) \
    template<> struct Point##accessor< Dim > \
    { \
        static inline CoordinateType get(const Point& p) { return  p. Get; } \
    };


// Get/set version
#define GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_GET_SET(Point, Dim, CoordinateType, Get, Set) \
    template<> struct Point##accessor< Dim > \
    { \
        static inline CoordinateType get(const Point& p) { return  p. Get (); } \
        static inline void set(Point& p, const CoordinateType& value) { p. Set ( value ); } \
    };



#define GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_2D(Point, CoordinateType, Get0, Get1, Set0, Set1) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR(Point, 0, CoordinateType, Get0, Set0) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR(Point, 1, CoordinateType, Get1, Set1)

#define GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_3D(Point, CoordinateType, Get0, Get1, Get2, Set0, Set1, Set2) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR(Point, 0, CoordinateType, Get0, Set0) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR(Point, 1, CoordinateType, Get1, Set1) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR(Point, 2, CoordinateType, Get2, Set2)

// Const versions
#define GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST_2D(Point, CoordinateType, Get0, Get1) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_CONST(Point, 0, CoordinateType, Get0) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_CONST(Point, 1, CoordinateType, Get1)

#define GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST_3D(Point, CoordinateType, Get0, Get1, Get2) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_CONST(Point, 0, CoordinateType, Get0) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_CONST(Point, 1, CoordinateType, Get1) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_CONST(Point, 2, CoordinateType, Get2)

#endif // DOXYGEN_NO_SPECIALIZATIONS



// Library user macro to register a custom 2D point
#define GEOMETRY_REGISTER_POINT_2D(Point, CoordinateType, CoordinateSystem, Field0, Field1) \
namespace ggl { namespace traits {  \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, 2, CoordinateType, CoordinateSystem) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS(Point, CoordinateType) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_2D(Point, CoordinateType, Field0, Field1, Field0, Field1) \
}}

// Library user macro to register a custom 3D point
#define GEOMETRY_REGISTER_POINT_3D(Point, CoordinateType, CoordinateSystem, Field0, Field1, Field2) \
namespace ggl { namespace traits {  \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, 3, CoordinateType, CoordinateSystem) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS(Point, CoordinateType) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_3D(Point, CoordinateType, Field0, Field1, Field2, Field0, Field1, Field2) \
}}



// Library user macro to register a custom 2D point (CONST version)
#define GEOMETRY_REGISTER_POINT_2D_CONST(Point, CoordinateType, CoordinateSystem, Field0, Field1) \
namespace ggl { namespace traits {  \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, 2, CoordinateType, CoordinateSystem) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST(Point, CoordinateType) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST_2D(Point, CoordinateType, Field0, Field1) \
}}

// Library user macro to register a custom 3D point (CONST version)
#define GEOMETRY_REGISTER_POINT_3D_CONST(Point, CoordinateType, CoordinateSystem, Field0, Field1, Field2) \
namespace ggl { namespace traits {  \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, 3, CoordinateType, CoordinateSystem) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST(Point, CoordinateType) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST_3D(Point, CoordinateType, Field0, Field1, Field2) \
}}


// Library user macro to register a custom 2D point (having separate get/set methods)
#define GEOMETRY_REGISTER_POINT_2D_GET_SET(Point, CoordinateType, CoordinateSystem, Get0, Get1, Set0, Set1) \
namespace ggl { namespace traits {  \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, 2, CoordinateType, CoordinateSystem) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS(Point, CoordinateType) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_GET_SET(Point, 0, CoordinateType, Get0, Set0) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_GET_SET(Point, 1, CoordinateType, Get1, Set1) \
}}


// Library user macro to register a custom 3D point (having separate get/set methods)
#define GEOMETRY_REGISTER_POINT_3D_GET_SET(Point, CoordinateType, CoordinateSystem, Get0, Get1, Get2, Set0, Set1, Set2) \
namespace ggl { namespace traits {  \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, 3, CoordinateType, CoordinateSystem) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS(Point, CoordinateType) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_GET_SET(Point, 0, CoordinateType, Get0, Set0) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_GET_SET(Point, 1, CoordinateType, Get1, Set1) \
    GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_GET_SET(Point, 2, CoordinateType, Get2, Set2) \
}}


#endif // GGL_GEOMETRIES_REGISTER_POINT_HPP
