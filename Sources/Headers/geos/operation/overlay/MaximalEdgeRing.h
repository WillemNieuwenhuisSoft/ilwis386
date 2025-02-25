/**********************************************************************
 * $Id: MaximalEdgeRing.h 2366 2009-04-15 09:25:28Z strk $
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

#ifndef GEOS_OP_OVERLAY_MAXIMALEDGERING_H
#define GEOS_OP_OVERLAY_MAXIMALEDGERING_H

#include <vector>

#include <geos/geomgraph/EdgeRing.h> // for inheritance

// Forward declarations
namespace geos {
	namespace geom {
		class GeometryFactory;
	}
	namespace geomgraph {
		class DirectedEdge;
		//class EdgeRing;
	}
	namespace operation {
		namespace overlay {
			class MinimalEdgeRing;
		}
	}
}

namespace geos {
namespace operation { // geos::operation
namespace overlay { // geos::operation::overlay

/** \brief
 * A ring of {@link edges} which may contain nodes of degree > 2.
 *
 * A MaximalEdgeRing may represent two different spatial entities:
 * 
 * - a single polygon possibly containing inversions (if the ring is oriented CW)
 * - a single hole possibly containing exversions (if the ring is oriented CCW)
 * 
 * If the MaximalEdgeRing represents a polygon,
 * the interior of the polygon is strongly connected.
 * 
 * These are the form of rings used to define polygons under some spatial data models.
 * However, under the OGC SFS model, {@link MinimalEdgeRings} are required.
 * A MaximalEdgeRing can be converted to a list of MinimalEdgeRings using the
 * {@link #buildMinimalRings() } method.
 *
 * @see com.vividsolutions.jts.operation.overlay.MinimalEdgeRing
 */
class MaximalEdgeRing: public geomgraph::EdgeRing {

public:

	MaximalEdgeRing(geomgraph::DirectedEdge *start,
		const geom::GeometryFactory *geometryFactory);
			// throw(const TopologyException &)

	virtual ~MaximalEdgeRing();

	geomgraph::DirectedEdge* getNext(geomgraph::DirectedEdge *de);

	void setEdgeRing(geomgraph::DirectedEdge* de, geomgraph::EdgeRing* er);

	/// \brief
	/// This function returns a newly allocated vector of
	/// pointers to newly allocated MinimalEdgeRing objects.
	///
	std::vector<MinimalEdgeRing*>* buildMinimalRings();

	/// \brief
	/// This function pushes pointers to newly allocated  MinimalEdgeRing
	/// objects to the provided vector.
	///
	void buildMinimalRings(std::vector<MinimalEdgeRing*>& minEdgeRings);

	/// \brief
	/// For all nodes in this EdgeRing,
	/// link the DirectedEdges at the node to form minimalEdgeRings
	///
	void linkDirectedEdgesForMinimalEdgeRings();
};


} // namespace geos::operation::overlay
} // namespace geos::operation
} // namespace geos

#endif // ndef GEOS_OP_OVERLAY_MAXIMALEDGERING_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/27 16:02:34  strk
 * Added INL file for MinimalEdgeRing, added many debugging blocks,
 * fixed memory leak in ConnectedInteriorTester (bug #59)
 *
 * Revision 1.1  2006/03/17 13:24:59  strk
 * opOverlay.h header splitted. Reduced header inclusions in operation/overlay implementation files. ElevationMatrixFilter code moved from own file to ElevationMatrix.cpp (ideally a class-private).
 *
 **********************************************************************/

