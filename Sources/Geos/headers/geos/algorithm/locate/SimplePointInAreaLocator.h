/**********************************************************************
 * $Id: SimplePointInAreaLocator.h 2120 2008-01-30 22:34:13Z benjubb $
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
 **********************************************************************/

#ifndef GEOS_ALGORITHM_LOCATE_SIMPLEPOINTINAREALOCATOR_H
#define GEOS_ALGORITHM_LOCATE_SIMPLEPOINTINAREALOCATOR_H
#include <geos/platform.h>
#include <geos/algorithm/locate/PointOnGeometryLocator.h> // inherited

// Forward declarations
namespace geos {
	namespace geom {
		class Geometry;
		class Coordinate;
		class Polygon;
	}
}

namespace geos {
namespace algorithm { // geos::algorithm
namespace locate { // geos::algorithm::locate

/** \brief
 * Computes the location of points
 * relative to an areal {@link Geometry},
 * using a simple O(n) algorithm.
 *
 * This algorithm is suitable for use in cases where
 * only one or a few points will be tested against a given area.
 * 
 * The algorithm used is only guaranteed to return correct results
 * for points which are <b>not</b> on the boundary of the Geometry.
 *
 * @version 1.7
 */
class SimplePointInAreaLocator : public PointOnGeometryLocator
{

public:

	_export static int locate(const geom::Coordinate& p,
			const geom::Geometry *geom);

	_export static bool containsPointInPolygon(const geom::Coordinate& p,
			const geom::Polygon *poly);

	_export SimplePointInAreaLocator( const geom::Geometry * g) 
	:	g( g)
	{ }

	_export int locate( const geom::Coordinate * p) 
	{
		return locate( *p, g);
	}

private:

	_export static bool containsPoint(const geom::Coordinate& p,
			const geom::Geometry *geom);

	const geom::Geometry * g;

};

} // geos::algorithm::locate
} // geos::algorithm
} // geos


#endif // GEOS_ALGORITHM_LOCATE_SIMPLEPOINTINAREALOCATOR_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:48  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

