/**********************************************************************
 * $Id: MCIndexNoder.h 2319 2009-04-07 19:00:36Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006      Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/MCIndexNoder.java rev. 1.6 (JTS-1.9)
 *
 **********************************************************************/

#ifndef GEOS_NODING_MCINDEXNODER_H
#define GEOS_NODING_MCINDEXNODER_H

#include <geos/inline.h>

#include <geos/index/chain/MonotoneChainOverlapAction.h> // for inheritance
#include <geos/noding/SinglePassNoder.h> // for inheritance
#include <geos/index/strtree/STRtree.h> // for composition
#include <geos/util.h>

#include <vector>
#include <iostream>

// Forward declarations
namespace geos {
	namespace geom {
		class LineSegment;
	}
	namespace noding {
		class SegmentString;
		class SegmentIntersector;
	}
}

namespace geos {
namespace noding { // geos.noding

/** \brief
 * Nodes a set of SegmentString using a index based
 * on index::chain::MonotoneChain and a index::SpatialIndex.
 *
 * The {@link SpatialIndex} used should be something that supports
 * envelope (range) queries efficiently (such as a index::quadtree::Quadtree
 * or index::strtree::STRtree.
 *
 * Last port: noding/MCIndexNoder.java rev. 1.4 (JTS-1.7)
 */
class MCIndexNoder : public SinglePassNoder {

private:
	std::vector<index::chain::MonotoneChain*> monoChains;
	index::strtree::STRtree index;
	int idCounter;
	std::vector<SegmentString*>* nodedSegStrings;
	// statistics
	int nOverlaps;

	void intersectChains();

	void add(SegmentString* segStr);

public:

	MCIndexNoder(SegmentIntersector *nSegInt=NULL)
		:
		SinglePassNoder(nSegInt),
		idCounter(0),
		nodedSegStrings(NULL),
		nOverlaps(0)
	{}

	~MCIndexNoder();

	/// Return a reference to this instance's std::vector of MonotoneChains
	std::vector<index::chain::MonotoneChain*>& getMonotoneChains() { return monoChains; }

	index::SpatialIndex& getIndex();

	std::vector<SegmentString*>* getNodedSubstrings() const;

	void computeNodes(std::vector<SegmentString*>* inputSegmentStrings);

	class SegmentOverlapAction : public index::chain::MonotoneChainOverlapAction {
	private:
		SegmentIntersector& si;
	public:
		SegmentOverlapAction(SegmentIntersector& newSi)
			:
			si(newSi)
		{}

		void overlap(index::chain::MonotoneChain* mc1, int start1,
				index::chain::MonotoneChain* mc2, int start2);

		void overlap(geom::LineSegment* s1, geom::LineSegment* s2)
        {
            UNREFERENCED_PARAMETER(s1);
            UNREFERENCED_PARAMETER(s2);
            assert(0);
        }
	};
	
};

} // namespace geos.noding
} // namespace geos

#ifdef GEOS_INLINE
# include <geos/noding/MCIndexNoder.inl>
#endif

#endif // GEOS_NODING_MCINDEXNODER_H

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.3  2006/03/22 18:12:31  strk
 * indexChain.h header split.
 *
 * Revision 1.2  2006/03/14 12:55:56  strk
 * Headers split: geomgraphindex.h, nodingSnapround.h
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

