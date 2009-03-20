// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_BOX_REGISTER_HPP
#define _GEOMETRY_BOX_REGISTER_HPP


#ifndef DOXYGEN_NO_SPECIALIZATIONS

// box based on point
#define GEOMETRY_DETAIL_SPECIALIZE_BOX_ACCESS(Box, Point, MinCorner, MaxCorner) \
template <size_t C, size_t D> \
struct indexed_access<Box, C, D> \
{ \
	static inline typename coordinate_type<Point>::type get(const Box& b) \
	{ \
		return C == min_corner ? geometry::get<D>(b. MinCorner) : geometry::get<D>(b. MaxCorner); \
	} \
	static inline void set(Box& b, const typename coordinate_type<Point>::type& value) \
	{ \
		if (C == min_corner) geometry::set<D>(b. MinCorner, value); else geometry::set<D>(b. MaxCorner, value); \
	} \
};


#define GEOMETRY_DETAIL_SPECIALIZE_BOX_ACCESS_TEMPLATIZED(Box, Point, MinCorner, MaxCorner) \
template <typename Point, size_t C, size_t D> \
struct indexed_access<Box<Point>, C, D> \
{ \
	static inline typename coordinate_type<Point>::type get(const Box<Point>& b) \
	{ \
		return C == min_corner ? geometry::get<D>(b. MinCorner) : geometry::get<D>(b. MaxCorner); \
	} \
	static inline void set(Box<Point>& b, const typename coordinate_type<Point>::type& value) \
	{ \
		if (C == min_corner) geometry::set<D>(b. MinCorner, value); else geometry::set<D>(b. MaxCorner, value); \
	} \
};


#define GEOMETRY_DETAIL_SPECIALIZE_BOX_ACCESS_4VALUES(Box, Point, Left, Bottom, Right, Top) \
template <size_t C, size_t D> \
struct indexed_access<Box, C, D> \
{ \
	static inline typename coordinate_type<Point>::type get(const Box& b) \
	{ \
		return C == min_corner && D == 0 ? b. Left \
			: C == min_corner && D == 1 ? b. Bottom \
			: C == max_corner && D == 0 ? b. Right \
			: C == max_corner && D == 1 ? b. Top \
			: 0; \
	} \
	static inline void set(Box& b, const typename coordinate_type<Point>::type& value) \
	{ \
		if (C == min_corner && D == 0) b. Left = value; \
		else if (C == min_corner && D == 1) b. Bottom = value; \
		else if (C == max_corner && D == 0) b. Right = value; \
		else if (C == max_corner && D == 1) b. Top = value; \
	} \
};



#define GEOMETRY_DETAIL_SPECIALIZE_BOX_TRAITS(Box, PointType) \
	template<> struct tag<Box > { typedef box_tag type; }; \
	template<> struct point_type<Box > { typedef PointType type; };

#define GEOMETRY_DETAIL_SPECIALIZE_BOX_TRAITS_TEMPLATIZED(Box, PointType) \
	template<typename PointType> struct tag<Box<PointType> > { typedef box_tag type; }; \
	template<typename PointType> struct point_type<Box<PointType> > { typedef PointType type; };

#endif // DOXYGEN_NO_SPECIALIZATIONS



#define GEOMETRY_REGISTER_BOX(Box, PointType, MinCorner, MaxCorner) \
namespace geometry { namespace traits {  \
	GEOMETRY_DETAIL_SPECIALIZE_BOX_TRAITS(Box, PointType) \
	GEOMETRY_DETAIL_SPECIALIZE_BOX_ACCESS(Box, PointType, MinCorner, MaxCorner) \
}}


#define GEOMETRY_REGISTER_BOX_TEMPLATIZED(Box, PointType, MinCorner, MaxCorner) \
namespace geometry { namespace traits {  \
	GEOMETRY_DETAIL_SPECIALIZE_BOX_TRAITS_TEMPLATIZED(Box, PointType) \
	GEOMETRY_DETAIL_SPECIALIZE_BOX_ACCESS_TEMPLATIZED(Box, PointType, MinCorner, MaxCorner) \
}}

#define GEOMETRY_REGISTER_BOX_2D_4VALUES(Box, PointType, Left, Bottom, Right, Top) \
namespace geometry { namespace traits {  \
	GEOMETRY_DETAIL_SPECIALIZE_BOX_TRAITS(Box, PointType) \
	GEOMETRY_DETAIL_SPECIALIZE_BOX_ACCESS_4VALUES(Box, PointType, Left, Bottom, Right, Top) \
}}



// CONST versions are for boxes probably not that common. Leave this for the moment.


#endif // _GEOMETRY_BOX_REGISTER_HPP
