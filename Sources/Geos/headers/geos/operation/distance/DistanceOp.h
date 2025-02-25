/**********************************************************************
 * $Id: DistanceOp.h 2560 2009-06-08 10:29:54Z strk $
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
 **********************************************************************
 *
 * Last port: operation/distance/DistanceOp.java rev 1.21 (JTS-1.10)
 *
 **********************************************************************/

#ifndef GEOS_OP_DISTANCE_DISTANCEOP_H
#define GEOS_OP_DISTANCE_DISTANCEOP_H

#include <geos/export.h>
#include <geos/platform.h> 
#include <geos/algorithm/PointLocator.h> // for composition

#include <vector>

// Forward declarations
namespace geos {
	namespace geom { 
		class Coordinate;
		class Polygon;
		class LineString;
		class Point;
		class Geometry;
		class CoordinateSequence;
	}
	namespace operation { 
		namespace distance { 
			class GeometryLocation;
		}
	}
}


namespace geos {
namespace operation { // geos::operation
namespace distance { // geos::operation::distance

/**
 * \brief
 * Find two points on two {@link Geometry}s which lie
 * within a given distance, or else are the nearest points
 * on the geometries (in which case this also
 * provides the distance between the geometries).
 * 
 * The distance computation also finds a pair of points in the
 * input geometries which have the minimum distance between them.
 * If a point lies in the interior of a line segment,
 * the coordinate computed is a close
 * approximation to the exact point.
 * 
 * The algorithms used are straightforward O(n^2)
 * comparisons.  This worst-case performance could be improved on
 * by using Voronoi techniques or spatial indexes.
 *
 */
class GEOS_DLL DistanceOp {
public:
	/**
	 * \brief
	 * Compute the distance between the nearest points of two geometries.
	 *
	 * @param g0 a {@link Geometry}
	 * @param g1 another {@link Geometry}
	 * @return the distance between the geometries
	 */
	_export static double distance(const geom::Geometry& g0,
	                       const geom::Geometry& g1);

	/// @deprecated, use the version taking references
	_export static double distance(const geom::Geometry *g0,
	                        const geom::Geometry *g1);

	/**
	 * \brief
	 * Test whether two geometries lie within a given distance of
	 * each other.
	 *
	 * @param g0 a {@link Geometry}
	 * @param g1 another {@link Geometry}
	 * @param distance the distance to test
	 * @return true if g0.distance(g1) <= distance
	 */
	_export static bool isWithinDistance(const geom::Geometry& g0,
	                             const geom::Geometry& g1,
	                                            double distance);

	/**
	 * Compute the the nearest points of two geometries.
	 *
	 * The points are presented in the same order as the input Geometries.
	 *
	 * @param g0 a {@link Geometry}
	 * @param g1 another {@link Geometry}
	 *
	 * @return the nearest points in the geometries, ownership to caller.
	 *         A NULL return means one of the geometries is empty.
	 *
	 */
	_export static geom::CoordinateSequence* nearestPoints(
	                                        const geom::Geometry *g0,
	                                        const geom::Geometry *g1);

	/**
	 * Compute the the closest points of two geometries.
	 *
	 * The points are presented in the same order as the input Geometries.
	 *
	 * @param g0 a {@link Geometry}
	 * @param g1 another {@link Geometry}
	 *
	 * @return the closest points in the geometries, ownership to caller.
	 *         A NULL return means one of the geometries is empty.
	 *
	 * @deprecated renamed to nearestPoints
	 */
	_export static geom::CoordinateSequence* closestPoints(
	                                        const geom::Geometry *g0,
	                                        const geom::Geometry *g1);

	/// @deprecated use the one taking references
	_export DistanceOp(const geom::Geometry *g0, const geom::Geometry *g1);

	/**
	 * \brief
	 * Constructs a DistanceOp that computes the distance and
	 * nearest points between the two specified geometries.
	 *
	 * @param g0 a Geometry
	 * @param g1 a Geometry
	 */
	_export DistanceOp(const geom::Geometry& g0, const geom::Geometry& g1);

	/**
	 * \brief
	 * Constructs a DistanceOp that computes the distance and nearest
	 * points between the two specified geometries.
	 *
	 * @param g0 a Geometry
	 * @param g1 a Geometry
	 * @param terminateDistance the distance on which to terminate
	 *                          the search
	 */
	_export DistanceOp(const geom::Geometry& g0, const geom::Geometry& g1,
	                                      double terminateDistance);

	_export ~DistanceOp();

	/**
	 * Report the distance between the closest points on the input geometries.
	 *
	 * @return the distance between the geometries
	 */
	_export double distance();

	/**
	 * Report the coordinates of the closest points in the input geometries.
	 * The points are presented in the same order as the input Geometries.
	 *
	 * @return a pair of {@link Coordinate}s of the closest points
	 *         as a newly allocated object (ownership to caller)
	 *
	 * @deprecated renamed to nearestPoints
	 */
	_export geom::CoordinateSequence* closestPoints();

	/**
	 * Report the coordinates of the nearest points in the input geometries.
	 * The points are presented in the same order as the input Geometries.
	 *
	 * @return a pair of {@link Coordinate}s of the nearest points
	 *         as a newly allocated object (ownership to caller)
	 *
	 */
	_export geom::CoordinateSequence* nearestPoints();

private:

	/**
	 * Report the locations of the closest points in the input geometries.
	 * The locations are presented in the same order as the input
	 * Geometries.
	 *
	 * @return a pair of {@link GeometryLocation}s for the closest points.
	 *         Ownership of returned object is left to this instance and 
	 *         it's reference will be alive for the whole lifetime of it.
	 *
	 * NOTE: this is public in JTS, but we aim at API reduction here...
	 *
	 */
	std::vector<GeometryLocation*>* nearestLocations();

	// input (TODO: use two references instead..)
	std::vector<geom::Geometry const*> geom;
	double terminateDistance; 

	// working 
	algorithm::PointLocator ptLocator;
	// TODO: use auto_ptr
	std::vector<GeometryLocation*> *minDistanceLocation;
	double minDistance;

	// memory management
	std::vector<geom::Coordinate *> newCoords;


	void updateMinDistance(std::vector<GeometryLocation*>& locGeom,
	                       bool flip);

	void computeMinDistance();

	void computeContainmentDistance();

	void computeInside(std::vector<GeometryLocation*> *locs,
			const std::vector<const geom::Polygon*>& polys,
			std::vector<GeometryLocation*> *locPtPoly);

	void computeInside(GeometryLocation *ptLoc,
			const geom::Polygon *poly,
			std::vector<GeometryLocation*> *locPtPoly);

	/**
	 * Computes distance between facets (lines and points)
	 * of input geometries.
	 */
	void computeFacetDistance();

	void computeMinDistanceLines(
			const std::vector<const geom::LineString*>& lines0,
			const std::vector<const geom::LineString*>& lines1,
			std::vector<GeometryLocation*>& locGeom);

	void computeMinDistancePoints(
			const std::vector<const geom::Point*>& points0,
			const std::vector<const geom::Point*>& points1,
			std::vector<GeometryLocation*>& locGeom);

	void computeMinDistanceLinesPoints(
			const std::vector<const geom::LineString*>& lines0,
			const std::vector<const geom::Point*>& points1,
			std::vector<GeometryLocation*>& locGeom);

	void computeMinDistance(const geom::LineString *line0,
			const geom::LineString *line1,
			std::vector<GeometryLocation*>& locGeom);

	void computeMinDistance(const geom::LineString *line,
			const geom::Point *pt,
			std::vector<GeometryLocation*>& locGeom);
};


} // namespace geos::operation::distance
} // namespace geos::operation
} // namespace geos

#endif // GEOS_OP_DISTANCE_DISTANCEOP_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/21 17:55:01  strk
 * opDistance.h header split
 *
 **********************************************************************/

