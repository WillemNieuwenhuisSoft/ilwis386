/**********************************************************************
 * $Id: PolygonBuilder.h 2366 2009-04-15 09:25:28Z strk $
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

#ifndef GEOS_OP_OVERLAY_POLYGONBUILDER_H
#define GEOS_OP_OVERLAY_POLYGONBUILDER_H

#include <vector>

// Forward declarations
namespace geos {
	namespace geom {
		class Geometry;
		class Coordinate;
		class GeometryFactory;
	}
	namespace geomgraph {
		class EdgeRing;
		class Node;
		class PlanarGraph;
		class DirectedEdge;
	}
	namespace operation {
		namespace overlay {
			class MaximalEdgeRing;
			class MinimalEdgeRing;
		}
	}
}

namespace geos {
namespace operation { // geos::operation
namespace overlay { // geos::operation::overlay

/** \brief
 * Forms Polygon out of a graph of geomgraph::DirectedEdge.
 *
 * The edges to use are marked as being in the result Area.
 */
class PolygonBuilder {
public:

	// CGAlgorithms argument is unused
	PolygonBuilder(const geom::GeometryFactory *newGeometryFactory);

	~PolygonBuilder();

	/**
	 * Add a complete graph.
	 * The graph is assumed to contain one or more polygons,
	 * possibly with holes.
	 */
	void add(geomgraph::PlanarGraph *graph);
	    // throw(const TopologyException &)

	/**
	 * Add a set of edges and nodes, which form a graph.
	 * The graph is assumed to contain one or more polygons,
	 * possibly with holes.
	 */
	void add(const std::vector<geomgraph::DirectedEdge*> *dirEdges,
			const std::vector<geomgraph::Node*> *nodes);
			// throw(const TopologyException &)

  	std::vector<geom::Geometry*>* getPolygons();

	/** \brief
	 * Checks the current set of shells (with their associated holes) to
	 * see if any of them contain the point.
	 */
	bool containsPoint(const geom::Coordinate& p);

private:

	const geom::GeometryFactory *geometryFactory;

	std::vector<geomgraph::EdgeRing*> shellList;

	/**
	 * For all DirectedEdges in result, form them into MaximalEdgeRings
	 *
	 * Ownership of the returned vector *and* it's elements is
	 * transferred to caller.
	 */
	std::vector<MaximalEdgeRing*>* buildMaximalEdgeRings(
		const std::vector<geomgraph::DirectedEdge*> *dirEdges);
			// throw(const TopologyException &)

	std::vector<MaximalEdgeRing*>* buildMinimalEdgeRings(
		std::vector<MaximalEdgeRing*> *maxEdgeRings,
		std::vector<geomgraph::EdgeRing*> *newShellList,
		std::vector<geomgraph::EdgeRing*> *freeHoleList);
	/**
	 * This method takes a list of MinimalEdgeRings derived from a
	 * MaximalEdgeRing, and tests whether they form a Polygon. 
	 * This is the case if there is a single shell
	 * in the list.  In this case the shell is returned.
	 * The other possibility is that they are a series of connected
	 * holes, in which case no shell is returned.
	 *
	 * @return the shell geomgraph::EdgeRing, if there is one
	 * @return NULL, if all the rings are holes
	 */
	geomgraph::EdgeRing* findShell(std::vector<MinimalEdgeRing*>* minEdgeRings);

	/**
	 * This method assigns the holes for a Polygon (formed from a list of
	 * MinimalEdgeRings) to its shell.
	 * Determining the holes for a MinimalEdgeRing polygon serves two
	 * purposes:
	 * 
	 *  - it is faster than using a point-in-polygon check later on.
	 *  - it ensures correctness, since if the PIP test was used the point
	 *    chosen might lie on the shell, which might return an incorrect
	 *    result from the PIP test
	 */
	void placePolygonHoles(geomgraph::EdgeRing *shell,
		std::vector<MinimalEdgeRing*> *minEdgeRings);

	/**
	 * For all rings in the input list,
	 * determine whether the ring is a shell or a hole
	 * and add it to the appropriate list.
	 * Due to the way the DirectedEdges were linked,
	 * a ring is a shell if it is oriented CW, a hole otherwise.
	 */
	void sortShellsAndHoles(std::vector<MaximalEdgeRing*> *edgeRings,
		std::vector<geomgraph::EdgeRing*> *newShellList,
		std::vector<geomgraph::EdgeRing*> *freeHoleList);

	/** \brief
	 * This method determines finds a containing shell for all holes
	 * which have not yet been assigned to a shell.
	 *
	 * These "free" holes should all be <b>properly</b> contained in
	 * their parent shells, so it is safe to use the
	 * <code>findEdgeRingContaining</code> method.
	 * This is the case because any holes which are NOT
	 * properly contained (i.e. are connected to their
	 * parent shell) would have formed part of a MaximalEdgeRing
	 * and been handled in a previous step.
	 */
	void placeFreeHoles(std::vector<geomgraph::EdgeRing*>& newShellList,
		std::vector<geomgraph::EdgeRing*>& freeHoleList);
		// throw(const TopologyException&)

	/** \brief
	 * Find the innermost enclosing shell geomgraph::EdgeRing containing the
	 * argument geomgraph::EdgeRing, if any.
	 *
	 * The innermost enclosing ring is the <i>smallest</i> enclosing ring.
	 * The algorithm used depends on the fact that:
	 *
	 * ring A contains ring B iff envelope(ring A)
	 * contains envelope(ring B)
	 * 
	 * This routine is only safe to use if the chosen point of the hole
	 * is known to be properly contained in a shell
	 * (which is guaranteed to be the case if the hole does not touch
	 * its shell)
	 *
	 * @return containing geomgraph::EdgeRing, if there is one
	 * @return NULL if no containing geomgraph::EdgeRing is found
	 */
	geomgraph::EdgeRing* findEdgeRingContaining(geomgraph::EdgeRing *testEr,
		std::vector<geomgraph::EdgeRing*>& newShellList);

	std::vector<geom::Geometry*>* computePolygons(
			std::vector<geomgraph::EdgeRing*>& newShellList);

	/**
	 * Checks the current set of shells (with their associated holes) to
	 * see if any of them contain the point.
	 */

};



} // namespace geos::operation::overlay
} // namespace geos::operation
} // namespace geos

#endif // ndef GEOS_OP_OVERLAY_POLYGONBUILDER_H

/**********************************************************************
 * $Log$
 * Revision 1.3  2006/06/13 23:26:46  strk
 * cleanups
 *
 * Revision 1.2  2006/03/20 12:33:45  strk
 * Simplified some privat methods to use refs instead of pointers, added
 * debugging section for failiures of holes/shells associations
 *
 * Revision 1.1  2006/03/17 13:24:59  strk
 * opOverlay.h header splitted. Reduced header inclusions in operation/overlay implementation files. ElevationMatrixFilter code moved from own file to ElevationMatrix.cpp (ideally a class-private).
 *
 **********************************************************************/

