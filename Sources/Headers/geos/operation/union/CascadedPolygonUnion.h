/**********************************************************************
 * $Id: CascadedPolygonUnion.h 2253 2009-01-23 23:58:18Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/
#ifndef GEOS_OP_UNION_CASCADEDPOLYGONUNION_H
#define GEOS_OP_UNION_CASCADEDPOLYGONUNION_H

#include <vector>
#include <algorithm>

// Forward declarations
namespace geos {
    namespace geom {
        class GeometryFactory;
        class Geometry;
        class Polygon;
        class MultiPolygon;
        class Envelope;
    }
    namespace index {
        namespace strtree {
            class ItemsList;
        }
    }
}

namespace geos {
namespace operation { // geos::operation
namespace geounion {  // geos::operation::geounion

/**
 * \brief Helper class holding Geometries, part of which are held by reference
 *        others are held exclusively.
 */
class GeometryListHolder : public std::vector<geom::Geometry*>
{
private:
    typedef std::vector<geom::Geometry*> base_type;

public:
    GeometryListHolder() {}
    ~GeometryListHolder()
    {
        std::for_each(ownedItems.begin(), ownedItems.end(), 
            &GeometryListHolder::deleteItem);
    }

    // items need to be deleted in the end
    void push_back_owned(geom::Geometry* item)
    {
        this->base_type::push_back(item);
        ownedItems.push_back(item);
    }

    geom::Geometry* getGeometry(std::size_t index)
    {
      if (index >= this->base_type::size()) 
          return NULL;
      return (*this)[index];
    }

private:
    static void deleteItem(geom::Geometry* item);

private:
    std::vector<geom::Geometry*> ownedItems;
};

/**
 * \brief 
 * Provides an efficient method of unioning a collection of 
 * {@link Polygonal} geometries.
 * This algorithm is faster and likely more robust than
 * the simple iterated approach of 
 * repeatedly unioning each polygon to a result geometry.
 * <p>
 * The <tt>buffer(0)</tt> trick is sometimes faster, but can be less robust and 
 * can sometimes take an exceptionally long time to complete.
 * This is particularly the case where there is a high degree of overlap
 * between the polygons.  In this case, <tt>buffer(0)</tt> is forced to compute
 * with <i>all</i> line segments from the outset, 
 * whereas cascading can eliminate many segments
 * at each stage of processing.
 * The best case for buffer(0) is the trivial case
 * where there is <i>no</i> overlap between the input geometries. 
 * However, this case is likely rare in practice.
 */
class CascadedPolygonUnion 
{
private:
    std::vector<geom::Polygon*>* inputPolys;
    geom::GeometryFactory const* geomFactory;

    /**
     * The effectiveness of the index is somewhat sensitive
     * to the node capacity.  
     * Testing indicates that a smaller capacity is better.
     * For an STRtree, 4 is probably a good number (since
     * this produces 2x2 "squares").
     */
    static int const STRTREE_NODE_CAPACITY = 4;

public:
    CascadedPolygonUnion();

    /**
     * Computes the union of
     * a collection of {@link Polygonal} {@link Geometry}s.
     * 
     * @param polys a collection of {@link Polygonal} {@link Geometry}s
     */
    static geom::Geometry* Union(std::vector<geom::Polygon*>* polys);

    /**
     * Computes the union of
     * a collection of {@link Polygonal} {@link Geometry}s.
     * 
     * @param polys a collection of {@link Polygonal} {@link Geometry}s
     */
    static geom::Geometry* Union(const geom::MultiPolygon* polys);

    /**
     * Creates a new instance to union
     * the given collection of {@link Geometry}s.
     * 
     * @param geoms a collection of {@link Polygonal} {@link Geometry}s
     */
    CascadedPolygonUnion(std::vector<geom::Polygon*>* polys)
      : inputPolys(polys),
        geomFactory(NULL)
    {}

    /**
     * Computes the union of the input geometries.
     * 
     * @return the union of the input geometries
     * @return null if no input geometries were provided
     */
    geom::Geometry* Union();

private:
    geom::Geometry* unionTree(index::strtree::ItemsList* geomTree);

    /**
     * Unions a list of geometries 
     * by treating the list as a flattened binary tree,
     * and performing a cascaded union on the tree.
     */
    geom::Geometry* binaryUnion(GeometryListHolder* geoms);

    /**
     * Unions a section of a list using a recursive binary union on each half
     * of the section.
     * 
     * @param geoms
     * @param start
     * @param end
     * @return the union of the list section
     */
    geom::Geometry* binaryUnion(GeometryListHolder* geoms, std::size_t start, 
        std::size_t end);

    /**
     * Reduces a tree of geometries to a list of geometries
     * by recursively unioning the subtrees in the list.
     * 
     * @param geomTree a tree-structured list of geometries
     * @return a list of Geometrys
     */
    GeometryListHolder* reduceToGeometries(index::strtree::ItemsList* geomTree);

    /**
     * Computes the union of two geometries, 
     * either of both of which may be null.
     * 
     * @param g0 a Geometry
     * @param g1 a Geometry
     * @return the union of the input(s)
     * @return null if both inputs are null
     */
    geom::Geometry* unionSafe(geom::Geometry* g0, geom::Geometry* g1);

    geom::Geometry* unionOptimized(geom::Geometry* g0, geom::Geometry* g1);

    /**
     * Unions two polygonal geometries.
     * The case of MultiPolygons is optimized to union only 
     * the polygons which lie in the intersection of the two geometry's envelopes.
     * Polygons outside this region can simply be combined with the union result,
     * which is potentially much faster.
     * This case is likely to occur often during cascaded union, and may also
     * occur in real world data (such as unioning data for parcels on different street blocks).
     * 
     * @param g0 a polygonal geometry
     * @param g1 a polygonal geometry
     * @param common the intersection of the envelopes of the inputs
     * @return the union of the inputs
     */
    geom::Geometry* unionUsingEnvelopeIntersection(geom::Geometry* g0, 
        geom::Geometry* g1, geom::Envelope const& common);

    geom::Geometry* extractByEnvelope(geom::Envelope const& env, 
        geom::Geometry* geom, std::vector<geom::Geometry*>& disjointGeoms);

    /**
     * Encapsulates the actual unioning of two polygonal geometries.
     * 
     * @param g0
     * @param g1
     * @return
     */
    static geom::Geometry* unionActual(geom::Geometry* g0, geom::Geometry* g1);
};

} // namespace geos::operation::union
} // namespace geos::operation
} // namespace geos

#endif

/**********************************************************************
 * $Log$
 *
 **********************************************************************/

