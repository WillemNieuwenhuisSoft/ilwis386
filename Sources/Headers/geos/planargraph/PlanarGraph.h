/**********************************************************************
 * $Id: PlanarGraph.h 1820 2006-09-06 16:54:23Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_PLANARGRAPH_PLANARGRAPH_H
#define GEOS_PLANARGRAPH_PLANARGRAPH_H

#include <geos/planargraph/NodeMap.h> // for composition

#include <vector> // for typedefs

// Forward declarations
namespace geos {
	namespace geom { 
		class Coordinate;
	}
	namespace planargraph { 
		class DirectedEdge;
		class Edge;
		class Node;
	}
}

namespace geos {
namespace planargraph { // geos.planargraph

/**
 * \class PlanarGraph planargraph.h geos/planargraph.h
 * \brief
 * Represents a directed graph which is embeddable in a planar surface.
 * 
 * This class and the other classes in this package serve as a framework for
 * building planar graphs for specific algorithms. This class must be
 * subclassed to expose appropriate methods to construct the graph. This allows
 * controlling the types of graph components (DirectedEdge, Edge and Node)
 * which can be added to the graph. An application which uses the graph
 * framework will almost always provide subclasses for one or more graph
 * components, which hold application-specific data and graph algorithms.
 */
class PlanarGraph {

protected:

	std::vector<Edge*> edges;
	std::vector<DirectedEdge*> dirEdges;
	NodeMap nodeMap;

	/**
	 * \brief
	 * Adds a node to the std::map, replacing any that is already at that
	 * location.
	 *
	 * Only subclasses can add Nodes, to ensure Nodes are
	 * of the right type.
	 * @return the added node
	 */
	void add(Node *node) {
		nodeMap.add(node);
	}

	/**
	 * \brief
	 * Adds the Edge and its DirectedEdges with this PlanarGraph.
	 *
	 * Assumes that the Edge has already been created with its associated
	 * DirectEdges.
	 * Only subclasses can add Edges, to ensure the edges added are of
	 * the right class.
	 */
	void add(Edge *edge);

	/**
	 * \brief
	 * Adds the Edge to this PlanarGraph.
	 *
	 * Only subclasses can add DirectedEdges,
	 * to ensure the edges added are of the right class.
	 */
	void add(DirectedEdge *dirEdge) {
		dirEdges.push_back(dirEdge);
	}

public:

	typedef std::vector<Edge *> EdgeContainer;
	typedef EdgeContainer::iterator EdgeIterator;


	/**
	 * \brief
	 * Constructs a PlanarGraph without any Edges, DirectedEdges, or Nodes.
	 */
	PlanarGraph() {}

	virtual ~PlanarGraph() {}

	/**
	 * \brief
	 * Returns the Node at the given location,
	 * or null if no Node was there.
	 */
	Node* findNode(const geom::Coordinate& pt) {
		return nodeMap.find(pt);
	}

	/**
	 * \brief
	 * Returns an Iterator over the Nodes in this PlanarGraph.
	 */
	NodeMap::container::iterator nodeIterator() {
		return nodeMap.begin();
	}

	NodeMap::container::iterator nodeBegin() {
		return nodeMap.begin();
	}

	NodeMap::container::const_iterator nodeBegin() const {
		return nodeMap.begin();
	}

	NodeMap::container::iterator nodeEnd() {
		return nodeMap.end();
	}

	NodeMap::container::const_iterator nodeEnd() const {
		return nodeMap.end();
	}

	/**
	 * \brief
	 * Returns the Nodes in this PlanarGraph.
	 */  
	std::vector<Node*>* getNodes() { return nodeMap.getNodes(); }

	/**
	 * \brief
	 * Returns an Iterator over the DirectedEdges in this PlanarGraph,
	 * in the order in which they were added.
	 *
	 * @see add(Edge)
	 * @see add(DirectedEdge)
	 */
	std::vector<DirectedEdge*>::iterator dirEdgeIterator() {
		return dirEdges.begin();
	}

	/// Alias for edgeBegin()
	std::vector<Edge*>::iterator edgeIterator() {
		return edges.begin();
	}

	/// Returns an iterator to first Edge in this graph.
	//
	/// Edges are stored in the order they were added.
	/// @see add(Edge)
	///
	std::vector<Edge*>::iterator edgeBegin() {
		return edges.begin();
	}

	/// Returns an iterator to one-past last Edge in this graph.
	//
	/// Edges are stored in the order they were added.
	/// @see add(Edge)
	///
	std::vector<Edge*>::iterator edgeEnd() {
		return edges.end();
	}

	/**
	 * \brief
	 * Returns the Edges that have been added to this PlanarGraph
	 * @see #add(Edge)
	 */
	std::vector<Edge*>* getEdges() {
		return &edges;
	}

	/**
	 * \brief
	 * Removes an Edge and its associated DirectedEdges from their
	 * from-Nodes and from this PlanarGraph.
	 *
	 * Note: This method does not remove the Nodes associated
	 * with the Edge, even if the removal of the Edge reduces the
	 * degree of a Node to zero.
	 */
	void remove(Edge *edge);

	/**
	 * \brief
	 * Removes DirectedEdge from its from-Node and from this PlanarGraph.
	 *
	 * Note:
	 * This method does not remove the Nodes associated with the
	 * DirectedEdge, even if the removal of the DirectedEdge reduces
	 * the degree of a Node to zero.
	 */
	void remove(DirectedEdge *de);

	/**
	 * \brief
	 * Removes a node from the graph, along with any associated
	 * DirectedEdges and Edges.
	 */
	void remove(Node *node);

	/**
	 * \brief
	 * Returns all Nodes with the given number of Edges around it.
 	 * The return value is a newly allocated vector of existing nodes
	 */
	std::vector<Node*>* findNodesOfDegree(size_t degree);
};

} // namespace geos::planargraph
} // namespace geos

#endif // GEOS_PLANARGRAPH_PLANARGRAPH_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/06/12 10:49:43  strk
 * unsigned int => size_t
 *
 * Revision 1.1  2006/03/21 21:42:54  strk
 * planargraph.h header split, planargraph:: classes renamed to match JTS symbols
 *
 **********************************************************************/

