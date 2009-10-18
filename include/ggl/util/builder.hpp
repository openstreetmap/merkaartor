// VERY OLD
// NOT PART OF GGL-REVIEW-TREE


// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_UTIL_BUILDER_HPP
#define GGL_UTIL_BUILDER_HPP

#include <boost/scoped_array.hpp>

#include <ggl/algorithms/make.hpp>
#include <ggl/core/exterior_ring.hpp>
#include <ggl/core/interior_rings.hpp>
#include <ggl/core/ring_type.hpp>

namespace ggl
{

/***

* probably not necessary, in these cases points can be added directly.

// Polygon builder version using a vector, to which points/holes can be added
// Builds on destruction OR on call to build()
template <class P, class Y>
class polygon_builder_vector
{
    public :
        polygon_builder_vector(Y& poly)
            : m_poly(poly)
        {}

        virtual ~polygon_builder_vector()
        {
            build();
        }

        void build()
        {
            m_poly.outer().clear();
            m_interior_rings(poly).resize(m_hole_array.size());

            if (m_point_array.size() <= 0)
            {
                return;
            }

            P& first = m_point_array.front();
            int r = -1;
            int h = 0;
            int i = 0;
            for (std::vector<P>::const_iterator it = m_point_array.begin();
                    it != m_point_array.end(); it++, i++)
            {
                // Check hole: if it is a holeindex, increase index r for rings
                if (m_hole_array.size() > h && i == m_hole_array[h])
                {
                    r = h++;
                }

                if (r == -1)
                {
                    m_poly.outer().push_back(Y::point_type(it->x(), it->y()));
                }
                else
                {
                    // Some files indicate a hole using the very first point of the polygon.
                    // They then also close the complete polygon with the very first point
                    // Those points are skipped
                    if (! (first == *it))
                    {
                        m_interior_rings(poly)[r].push_back(Y::point_type(it->x(), it->y()));
                    }
                }
            }

            m_hole_array.clear();
            m_point_array.clear();
        }

        inline void add_hole(int h)
        {
            m_hole_array.push_back(h);
        }
        inline void add_point(const P& p)
        {
            m_point_array.push_back(p);
        }

    private :
        std::vector<P> m_point_array;
        std::vector<uint32> m_hole_array;
        Y& m_poly;
};
***/


// Many files need a pointer to an array of points and/or holes
// The class below plus the builders below support that

// a boost scoped array, extended with a count and a size
template <typename T>
class scoped_array_with_size : public boost::scoped_array<T>
{
public:

    scoped_array_with_size(int n)
        : m_count(n)
        , boost::scoped_array<T>(n > 0 ? new T[n] : NULL)
    {}

    inline void resize(int n)
    {
        m_count = n;
        reset(n > 0 ? new T[n] : NULL);
    }

    inline int count() const { return m_count; }
    inline int memorysize() const { return m_count * sizeof(T); }

private:

    int m_count;
};


// A linebuilder-pointer class, builds the line on destruction
template <typename P, typename L>
class line_builder
{
public:

    line_builder(L& line, int n)
        : m_point_array(n)
        , m_line(line)
    {
    }

    virtual ~line_builder()
    {
        m_line.clear();
        P* p = m_point_array.get();
        for (int i = 0; i < m_point_array.count(); i++)
        {
            typedef point_type<L>::type LP;
            m_line.push_back(make<LP>(p->x, p->y));
            p++;
        }
    }

    inline int pointarray_size() const { return m_point_array.memorysize(); }
    inline P* pointarray() { return m_point_array.get(); }

private:

    scoped_array_with_size<P> m_point_array;
    L& m_line;
};

// Polygon builder-pointer class, builds polygons on destruction.
// Creates inner rings using array with hole-indexes
template <typename P, typename Y>
class polygon_builder
{
public:

    polygon_builder(Y& poly, int n, int holecount = 0)
        : m_point_array(n)
        , m_hole_array(holecount)
        , m_poly(poly)
    {}

    virtual ~polygon_builder()
    {
        exterior_ring(m_poly).clear();
        interior_rings(m_poly).resize(m_hole_array.count());

        if (m_point_array.count() <= 0)
        {
            return;
        }

        P* p = m_point_array.get();
        P& first = *p;
        int r = -1;
        int h = 0;
        for (int i = 0; i < m_point_array.count(); i++)
        {
            // Check hole: if it is a holeindex, increase index r for rings
            if (m_hole_array.count() > h && i == m_hole_array[h])
            {
                r = h++;
            }

            if (r == -1)
            {
                exterior_ring(m_poly).push_back(Y::point_type(p->x, p->y));
            }
            else
            {
                // Some files indicate a hole using the very first point of the polygon.
                // They then also close the complete polygon with the very first point
                // Those points are skipped
                if (! (first == *p))
                {
                    interior_rings(m_poly)[r].push_back(Y::point_type(p->x, p->y));
                }
            }
            p++;
        }
    }

    inline void set_holecount(int n)
    {
        m_hole_array.resize(n);
    }

    inline int pointarray_size() const { return m_point_array.memorysize(); }
    inline P* pointarray() { return m_point_array.get(); }

    inline int holearray_size() const { return m_hole_array.memorysize(); }
    inline uint32* holearray() { return m_hole_array.get(); }

private:

    scoped_array_with_size<P> m_point_array;
    scoped_array_with_size<uint32> m_hole_array;
    Y& m_poly;
};

// A linebuilder-pointer class, builds the line on destruction
template <typename P, typename L>
class line_extractor
{
public:

    line_extractor(const L& line)
        : m_point_array(line.size())
        , m_line(line)
    {
        P* p = m_point_array.get();
        for(L::const_iterator it = line.begin(); it != line.end(); it++)
        {
            p->x = it->x();
            p->y = it->y();
            p++;
        }
    }

    inline int pointarray_size() const { return m_point_array.memorysize(); }
    inline const P* pointarray() const { return m_point_array.get(); }

private:

    scoped_array_with_size<P> m_point_array;
    const L& m_line;
};

template <typename P, typename Y>
class polygon_extractor
{
public:

    polygon_extractor(const Y& poly)
        : m_poly(poly)
        , m_point_array(0)
        , m_hole_array(0)
    {
        // Calculate total point-size
        int n = exterior_ring(poly).size();
        for (int i = 0; i < interior_rings(poly).size(); i++)
        {
            n += 1 + interior_rings(poly)[i].size();
        }
        m_point_array.resize(n);
        m_hole_array.resize(interior_rings(poly).size());

        // Fill the points with outer/inner arrays
        n = 0;
        add_points(exterior_ring(poly), m_point_array.get(), n);
        uint32* h = m_hole_array.get();
        for (int i = 0; i < interior_rings(poly).size(); i++)
        {
            h[i] = n;
            add_points(interior_rings(poly)[i], m_point_array.get(), n);
            add_very_first(m_point_array.get(), n);
        }
    }

    inline int pointarray_count() const { return m_point_array.count(); }
    inline int pointarray_size() const { return m_point_array.memorysize(); }
    inline const P* pointarray()const  { return m_point_array.get(); }

    inline int holearray_count() const { return m_hole_array.count(); }
    inline int holearray_size() const { return m_hole_array.memorysize(); }
    inline const uint32* holearray()const  { return m_hole_array.get(); }


private:

    typedef typename ring_type<Y>::type ring_type;

    scoped_array_with_size<P> m_point_array;
    scoped_array_with_size<uint32> m_hole_array;
    const Y& m_poly;

    inline void add_points(const ring_type& r, P* p, int& n)
    {
        for (ring_type::const_iterator it = r.begin(); it != r.end(); it++)
        {
            p[n].x = it->x();
            p[n].y = it->y();
            n++;
        }
    }

    // add very first point to denote end-of-hole
    inline void add_very_first(P* p, int& n)
    {
        const P& very_first = *p;
        p[n++] = very_first;
    }
};

} // namespace ggl

#endif // GGL_UTIL_BUILDER_HPP
