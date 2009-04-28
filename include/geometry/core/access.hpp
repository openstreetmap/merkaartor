// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_ACCESS_HPP
#define _GEOMETRY_ACCESS_HPP


#include <boost/type_traits/remove_const.hpp>
// #include <boost/mpl/bool.hpp>

#include <geometry/core/coordinate_type.hpp>
#include <geometry/core/point_type.hpp>
#include <geometry/core/tag.hpp>



namespace geometry
{
	static const int min_corner = 0;
	static const int max_corner = 1;


	namespace traits
	{
		/*!
			\brief Traits class which gives access (get,set) to points
			\ingroup traits
			\par Geometries:
				- point
				- n-sphere (circle,sphere) for their center
			\par Specializations should provide:
				- static inline T get<I>(const G&)
				- static inline void set<I>(G&, const T&)
			\tparam G geometry
		*/
		template <typename G>
		struct access {};



		/*!
			\brief Traits class defining "get" and "set" to get and set point coordinate values
			\tparam G geometry (box, segment)
			\tparam I index (min_corner/max_corner for box, 0/1 for segment)
			\tparam D dimension
			\par Geometries:
				- box
				- segment
			\par Specializations should provide:
				- static inline T get(const G&)
				- static inline void set(G&, const T&)
			\ingroup traits
		*/
		template <typename G, size_t I, size_t D>
		struct indexed_access {};




		/*!
			\brief Traits class, optional, indicating that the std-library should be used
			\details The default geometry (linestring, ring, multi*) follow std:: for
				its modifying operations (push_back, clear, size, resize, reserve, etc)
				If they NOT follow the std:: library they should specialize this traits
				class
			\ingroup traits
			\par Geometries:
				- linestring
				- linear_ring
			\par Specializations should provide:
				- value (defaults to true)
		 */
		template <typename G>
		struct use_std
		{
			static const bool value = true;
		};

	}




	#ifndef DOXYGEN_NO_DISPATCH
	namespace core_dispatch
	{
		template <typename TAG, typename G, typename T, size_t D>
		struct access
		{
			//static inline T get(const G& ) {}
			//static inline void set(G& g, const T& value) {}
		};

		template <typename TAG, typename G, typename T, size_t I, size_t D>
		struct indexed_access
		{
			//static inline T get(const G& ) {}
			//static inline void set(G& g, const T& value) {}
		};



		template <typename P, typename T, size_t D>
		struct access<point_tag, P, T, D>
		{
			static inline T get(const P& p)
			{
				return traits::access<P>::template get<D>(p);
			}
			static inline void set(P& p, const T& value)
			{
				traits::access<P>::template set<D>(p, value);
			}
		};


		template <typename S, typename T, size_t D>
		struct access<nsphere_tag, S, T, D>
		{
			static inline T get(const S& s)
			{
				return traits::access<S>::template get<D>(s);
			}
			static inline void set(S& s, const T& value)
			{
				traits::access<S>::template set<D>(s, value);
			}
		};


		template <typename B, typename T, size_t I, size_t D>
		struct indexed_access<box_tag, B, T, I, D>
		{
			static inline T get(const B& b)
			{
				return traits::indexed_access<B, I, D>::get(b);
			}
			static inline void set(B& b, const T& value)
			{
				traits::indexed_access<B, I, D>::set(b, value);
			}
		};

		template <typename S, typename T, size_t I, size_t D>
		struct indexed_access<segment_tag, S, T, I, D>
		{
			static inline T get(const S& segment)
			{
				return traits::indexed_access<S, I, D>::get(segment);
			}
			static inline void set(S& segment, const T& value)
			{
				traits::indexed_access<S, I, D>::set(segment, value);
			}
		};


	} // namespace core_dispatch
	#endif


	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		// Two dummy tags to distinguish get/set variants below.
		// They don't have to be specified by the user. The functions are distinguished
		// by template signature also, but for e.g. GCC this is not enough. So give them
		// a different signature.
		struct signature_getset_dimension {};
		struct signature_getset_index_dimension {};
	} // namespace impl
	#endif


	/*!
		\brief get a coordinate value of a point / nsphere
		\return coordinate value
		\ingroup access
		\tparam D dimension
		\tparam G geometry
		\param geometry geometry to get coordinate value from
		\param dummy does not have to be specified
	*/
	template <size_t D, typename G>
	inline typename coordinate_type<G>::type get(const G& geometry
			, impl::signature_getset_dimension* /* dummy */ = NULL
			)
	{
		typedef typename boost::remove_const<G>::type NCG;
		return core_dispatch::access<typename tag<G>::type, NCG,
				typename coordinate_type<NCG>::type, D>::get(geometry);
	};


	/*!
		\brief assign coordinate value to a point / sphere
		\ingroup access
		\tparam D dimension
		\tparam G geometry
		\param geometry geometry to assign coordinate to
		\param value coordinate value to assign
		\param dummy does not have to be specified
	*/
	template <size_t D, typename G>
	inline void set(G& geometry, const typename coordinate_type<G>::type& value
			, impl::signature_getset_dimension* /* dummy */ = NULL
			)
	{
		typedef typename boost::remove_const<G>::type NCG;
		core_dispatch::access<typename tag<G>::type, NCG,
				typename coordinate_type<NCG>::type, D>::set(geometry, value);
	}


	// Note: doxygen needs a construct to distinguish get/set (like the gcc compiler)


	/*!
		\brief get a coordinate value of a box / segment
		\return coordinate value
		\ingroup access
		\tparam I index, for boxes: min_corner or max_corner. For segment: 0 / 1
		\tparam D dimension
		\tparam G geometry
		\param geometry geometry to get coordinate value from
		\param dummy does not have to be specified
	*/
	template <size_t I, size_t D, typename G>
	inline typename coordinate_type<G>::type get(const G& geometry
			, impl::signature_getset_index_dimension* /* dummy */ = NULL
			)
	{
		typedef typename boost::remove_const<G>::type NCG;
		return core_dispatch::indexed_access<typename tag<G>::type, NCG,
				typename coordinate_type<NCG>::type, I, D>::get(geometry);
	};

	/*!
		\brief assign a coordinate value of a box / segment
		\ingroup access
		\tparam I index, for boxes: min_corner or max_corner. For segment: 0 / 1
		\tparam D dimension
		\tparam G geometry
		\param geometry geometry to assign coordinate to
		\param value coordinate value to assign
		\param dummy does not have to be specified
	*/
	template <size_t I, size_t D, typename G>
	inline void set(G& geometry, const typename coordinate_type<G>::type& value
			, impl::signature_getset_index_dimension* /* dummy */= NULL
			)
	{
		typedef typename boost::remove_const<G>::type NCG;
		core_dispatch::indexed_access<typename tag<G>::type, NCG,
				typename coordinate_type<NCG>::type, I, D>::set(geometry, value);
	}




}


#endif
