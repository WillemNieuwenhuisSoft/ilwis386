/**********************************************************************
 * $Id: EdgeRing.h 2366 2009-04-15 09:25:28Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geomgraph/EdgeRing.java rev. 1.9
 *
 **********************************************************************/


#ifndef GEOS_GEOMGRAPH_EDGERING_H
#define GEOS_GEOMGRAPH_EDGERING_H

#include <geos/geomgraph/Label.h> // for composition

#include <geos/inline.h>

#include <vector>
#include <cassert> // for testInvariant
#include <iosfwd> // for operator<<


// Forward declarations
namespace geos {
	namespace geom {
		class GeometryFactory;
		class LinearRing;
		class Polygon;
		class Coordinate;
		class CoordinateSequence;
	}
	namespace geomgraph {
		class DirectedEdge;
		//class Label;
		class Edge;
	}
}

namespace geos {
namespace geomgraph { // geos.geomgraph

class EdgeRing {

public:
	friend std::ostream& operator<< (std::ostream& os, const EdgeRing& er);

	EdgeRing(DirectedEdge *newStart,
			const geom::GeometryFactory *newGeometryFactory);

	virtual ~EdgeRing();

	bool isIsolated();

	bool isHole();

	/*
	 * Return a pointer to the LinearRing owned by
	 * this object. Make a copy if you need it beyond
	 * this objects's lifetime.
	 */
	geom::LinearRing* getLinearRing();

	Label& getLabel();

	bool isShell();

	EdgeRing *getShell();

	void setShell(EdgeRing *newShell);

	void addHole(EdgeRing *edgeRing);

	/**
	 * Return a Polygon copying coordinates from this
	 * EdgeRing and its holes. Caller must remember
	 * to delete the result
	 */
	geom::Polygon* toPolygon(const geom::GeometryFactory* geometryFactory);

	/**
	 * Compute a LinearRing from the point list previously collected.
	 * Test if the ring is a hole (i.e. if it is CCW) and set the hole
	 * flag accordingly.
	 */
	void computeRing();

	virtual DirectedEdge* getNext(DirectedEdge *de)=0;

	virtual void setEdgeRing(DirectedEdge *de, EdgeRing *er)=0;

	/**
	 * Returns the list of DirectedEdges that make up this EdgeRing
	 */
	std::vector<DirectedEdge*>& getEdges();

	int getMaxNodeDegree();

	void setInResult();

	/**
	 * This method will use the computed ring.
	 * It will also check any holes, if they have been assigned.
	 */
	bool containsPoint(const geom::Coordinate& p);

	void testInvariant()
	{
		// pts are never NULL
		assert(pts);

#ifndef NDEBUG
		// If this is not an hole, check that
		// each hole is not null and 
		// has 'this' as it's shell
		if ( ! shell )
		{
			for (std::vector<EdgeRing*>::const_iterator
				it=holes.begin(), itEnd=holes.end();
				it != itEnd;
				++it)
			{
				EdgeRing* hole=*it;
				assert(hole);
				assert(hole->getShell()==this);
			}
		}
#endif // ndef NDEBUG
	}

protected:

	DirectedEdge *startDe; // the directed edge which starts the list of edges for this EdgeRing

	const geom::GeometryFactory *geometryFactory;

	/// throw(const TopologyException &)
	void computePoints(DirectedEdge *newStart);

	void mergeLabel(Label& deLabel);

	/** \brief
	 * Merge the RHS label from a DirectedEdge into the label for
	 * this EdgeRing.
	 *
	 * The DirectedEdge label may be null. 
	 * This is acceptable - it results from a node which is NOT
	 * an intersection node between the Geometries
	 * (e.g. the end node of a LinearRing). 
	 * In this case the DirectedEdge label does not contribute any
	 * information to the overall labelling, and is
	 * simply skipped.
	 */
	void mergeLabel(Label& deLabel, int geomIndex);

	void addPoints(Edge *edge, bool isForward, bool isFirstEdge);

	/// a list of EdgeRings which are holes in this EdgeRing
	std::vector<EdgeRing*> holes;

private:

	int maxNodeDegree;

	/// the DirectedEdges making up this EdgeRing
	std::vector<DirectedEdge*> edges;

	geom::CoordinateSequence* pts;

	// label stores the locations of each geometry on the
	// face surrounded by this ring
	Label label;

	geom::LinearRing *ring;  // the ring created for this EdgeRing

	bool isHoleVar;

	/// if non-null, the ring is a hole and this EdgeRing is its containing shell
	EdgeRing *shell;  

	void computeMaxNodeDegree();

};

std::ostream& operator<< (std::ostream& os, const EdgeRing& er);

} // namespace geos.geomgraph
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geomgraph/EdgeRing.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_EDGERING_H

/**********************************************************************
 * $Log$
 * Revision 1.9  2006/07/08 00:33:55  strk
 *         * configure.in: incremented CAPI minor version, to avoid                        falling behind any future version from the 2.2. branch.
 *         * source/geom/Geometry.cpp, source/geom/GeometryFactory.cpp,
 *         source/geomgraph/EdgeRing.cpp,
 *         source/headers/geos/geom/Geometry.h,
 *         source/headers/geos/geom/GeometryFactory.h,
 *         source/headers/geos/geom/GeometryFactory.inl,
 *         source/headers/geos/geomgraph/EdgeRing.h:
 *         updated doxygen comments (sync with JTS head).
 *         * source/headers/geos/platform.h.in: include <inttypes.h>
 *         rather then <stdint.h>
 *
 * Revision 1.8  2006/04/06 09:41:55  strk
 * Added operator<<, added pts!=NULL assertion in testInvariant() function
 *
 * Revision 1.7  2006/04/05 18:28:42  strk
 * Moved testInvariant() methods from private to public, added
 * some comments about them.
 *
 * Revision 1.6  2006/03/29 13:53:59  strk
 * EdgeRing equipped with Invariant testing function and lots of exceptional assertions. Removed useless heap allocations, and pointers usages.
 *
 * Revision 1.5  2006/03/27 16:02:34  strk
 * Added INL file for MinimalEdgeRing, added many debugging blocks,
 * fixed memory leak in ConnectedInteriorTester (bug #59)
 *
 * Revision 1.4  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.3  2006/03/20 12:32:57  strk
 * Added note about responsibility of return from ::toPolygon
 *
 * Revision 1.2  2006/03/15 17:17:41  strk
 * Added missing forward declarations
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

