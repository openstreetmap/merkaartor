// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_TAG_HPP
#define _GEOMETRY_TAG_HPP


#include <boost/type_traits/remove_const.hpp>

#include <geometry/core/tags.hpp>

/*!
\defgroup core core: meta-functions for geometry types
*/

namespace geometry
{

	namespace traits
	{

		/*!
			\brief Traits class to attach a tag to a geometry
			\details All geometries should implement a traits::tag<G>::type metafunction to indicate their
				own geometry type.
			\ingroup traits
			\par Geometries:
				- all geometries
			\par Specializations should provide:
				- typedef XXX_tag type; (point_tag, box_tag, ...)
			\tparam G geometry
		*/
		template <typename G>
		struct tag
		{
			typedef geometry_not_recognized_tag type;
		};

	}


	/*!
		\brief Meta-function to get the tag of any geometry type
		\details All geometries tell their geometry type (point, linestring, polygon, etc) by implementing
		  a tag traits class. This meta-function uses that traits class to retrieve the tag.
		  If the input type is not a geometry at all, a geometry_not_recognized_tag will be returned.
		\tparam G geometry
		\ingroup core
	*/
	template <typename G>
	struct tag
	{
		typedef typename traits::tag<typename boost::remove_const<G>::type>::type type;
	};

}


#endif
