/**********************************************************************
 * $Id: EdgeList.h 2337 2009-04-08 16:52:28Z strk $
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
 * Last port: geomgraph/EdgeList.java rev. 1.4 (JTS-1.9)
 *
 **********************************************************************/


#ifndef GEOS_GEOMGRAPH_EDGELIST_H
#define GEOS_GEOMGRAPH_EDGELIST_H

#include <vector>
#include <map>
#include <string>
#include <iostream>

#include <geos/noding/OrientedCoordinateArray.h> // for map comparator

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace index {
		class SpatialIndex;
	}
	namespace geomgraph {
		class Edge;
	}
}

namespace geos {
namespace geomgraph { // geos.geomgraph

/** 
 * A EdgeList is a list of Edges. 
 *
 * It supports locating edges
 * that are pointwise equals to a target edge.
 */
class EdgeList {

private:

	std::vector<Edge*> edges;

	struct OcaCmp {
		bool operator()(
			const noding::OrientedCoordinateArray *oca1,
			const noding::OrientedCoordinateArray *oca2) const
		{
			return oca1->compareTo(*oca2)<0;
		}
	};

	/**
	 * An index of the edges, for fast lookup.
	 * 
	 * OrientedCoordinateArray objects are owned by us.
	 * TODO: optimize by dropping the OrientedCoordinateArray
	 *       construction as a whole, and use CoordinateSequence
	 *       directly instead..
	 */
	typedef std::map<noding::OrientedCoordinateArray*, Edge*, OcaCmp> EdgeMap;
	EdgeMap ocaMap;

public:
	friend std::ostream& operator<< (std::ostream& os, const EdgeList& el);

	EdgeList()
		:
		edges(),
		ocaMap()
	{}

	virtual ~EdgeList();

	/**
	 * Insert an edge unless it is already in the list
	 */
	void add(Edge *e);

	void addAll(const std::vector<Edge*> &edgeColl);

	std::vector<Edge*> &getEdges() { return edges; }

	Edge* findEqualEdge(Edge* e);

	Edge* get(int i);

	int findEdgeIndex(Edge *e);

	std::string print();

        void clearList();

};

std::ostream& operator<< (std::ostream& os, const EdgeList& el);


} // namespace geos.geomgraph
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geomgraph/EdgeList.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_EDGELIST_H

/**********************************************************************
 * $Log$
 * Revision 1.3  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.2  2006/03/14 11:03:15  strk
 * Added operator<< for Edge and EdgeList
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

