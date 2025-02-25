/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2009  Sandro Santilli <strk@keybit.net>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/distance/DiscreteHausdorffDistance.java 1.4 (JTS-1.9)
 *
 **********************************************************************/

#ifndef GEOS_ALGORITHM_DISTANCE_DISCRETEHAUSDORFFDISTANCE_H
#define GEOS_ALGORITHM_DISTANCE_DISCRETEHAUSDORFFDISTANCE_H

#include <geos/algorithm/distance/PointPairDistance.h> // for composition
#include <geos/algorithm/distance/EuclideanDistanceToPoint.h> // for composition
#include <geos/util/IllegalArgumentException.h> // for inlines
#include <geos/geom/Geometry.h> // for inlines
#include <geos/util/math.h> // for inlines
#include <geos/geom/CoordinateFilter.h> // for inheritance
#include <geos/geom/CoordinateSequenceFilter.h> // for inheritance

#include <vector>

namespace geos {
	namespace algorithm {
		//class RayCrossingCounter;
	}
	namespace geom {
		class Geometry;
		class Coordinate; 
		//class CoordinateSequence; 
	}
	namespace index {
		namespace intervalrtree {
			//class SortedPackedIntervalRTree;
		}
	}
}

namespace geos {
namespace algorithm { // geos::algorithm
namespace distance { // geos::algorithm::distance

/** \brief
 * An algorithm for computing a distance metric
 * which is an approximation to the Hausdorff Distance
 * based on a discretization of the input {@link Geometry}.
 *
 * The algorithm computes the Hausdorff distance restricted to discrete points
 * for one of the geometries.
 * The points can be either the vertices of the geometries (the default),
 * or the geometries with line segments densified by a given fraction.
 * Also determines two points of the Geometries which are separated by the
 * computed distance.
 * 
 * This algorithm is an approximation to the standard Hausdorff distance.
 * Specifically,
 * <pre>
 *    for all geometries a, b:    DHD(a, b) <= HD(a, b)
 * </pre>
 * The approximation can be made as close as needed by densifying the
 * input geometries.
 * In the limit, this value will approach the true Hausdorff distance:
 * <pre>
 *    DHD(A, B, densifyFactor) -> HD(A, B) as densifyFactor -> 0.0
 * </pre>
 * The default approximation is exact or close enough for a large subset of
 * useful cases.
 * Examples of these are:
 * 
 * - computing distance between Linestrings that are roughly parallel to
 *   each other, and roughly equal in length.  This occurs in matching
 *   linear networks.
 * - Testing similarity of geometries.
 * 
 * An example where the default approximation is not close is:
 * <pre>
 *   A = LINESTRING (0 0, 100 0, 10 100, 10 100)
 *   B = LINESTRING (0 100, 0 10, 80 10)
 *
 *   DHD(A, B) = 22.360679774997898
 *   HD(A, B) ~= 47.8
 * </pre>
 */
class DiscreteHausdorffDistance
{
public:

	static double distance(const geom::Geometry& g0,
	                       const geom::Geometry& g1);

	static double distance(const geom::Geometry& g0,
	                       const geom::Geometry& g1, double densifyFrac);

	DiscreteHausdorffDistance(const geom::Geometry& g0,
	                          const geom::Geometry& g1)
		:
		g0(g0),
		g1(g1),
		ptDist(),
		densifyFrac(0.0)
	{}

	/**
	 * Sets the fraction by which to densify each segment.
	 * Each segment will be (virtually) split into a number of equal-length
	 * subsegments, whose fraction of the total length is closest
	 * to the given fraction.
	 *
	 * @param dFrac
	 */
	void setDensifyFraction(double dFrac)
	{
		if ( dFrac > 1.0 || dFrac <= 0.0 )
		{
			throw util::IllegalArgumentException(
				"Fraction is not in range (0.0 - 1.0]");
		}

		densifyFrac = dFrac;
	}

	double distance()
	{
		compute(g0, g1);
		return ptDist.getDistance();
	}

	double orientedDistance()
	{
		computeOrientedDistance(g0, g1, ptDist);
		return ptDist.getDistance();
	}

	const std::vector<geom::Coordinate> getCoordinates() const
	{
		return ptDist.getCoordinates();
	}

	class MaxPointDistanceFilter : public geom::CoordinateFilter
	{
	public:
		MaxPointDistanceFilter(const geom::Geometry& geom)
			:
			geom(geom)
		{}

		void filter_ro(const geom::Coordinate* pt)
		{
			minPtDist.initialize();
			EuclideanDistanceToPoint::computeDistance(geom, *pt,
			                                         minPtDist);
			maxPtDist.setMaximum(minPtDist);
		}

		const PointPairDistance& getMaxPointDistance() const
		{
			return maxPtDist;
		}

	private:
		PointPairDistance maxPtDist;
		PointPairDistance minPtDist;
		EuclideanDistanceToPoint euclideanDist;
		const geom::Geometry& geom;
	};

	class MaxDensifiedByFractionDistanceFilter
			: public geom::CoordinateSequenceFilter
	{
	public:

		MaxDensifiedByFractionDistanceFilter(
				const geom::Geometry& geom, double fraction)
			:
			geom(geom),
			numSubSegs( size_t(round(1.0/fraction)) )
		{
		}

		void filter_ro(const geom::CoordinateSequence& seq,
		               size_t index);

		bool isGeometryChanged() const { return false; }

		bool isDone() const { return false; }

		const PointPairDistance& getMaxPointDistance() const {
			return maxPtDist;
		}

	private:
		PointPairDistance maxPtDist;
		PointPairDistance minPtDist;
		const geom::Geometry& geom;
		size_t numSubSegs; // = 0;
		
	};

private:

	void compute(const geom::Geometry& g0,
	             const geom::Geometry& g1)
	{
		computeOrientedDistance(g0, g1, ptDist);
		computeOrientedDistance(g1, g0, ptDist);
	}

	void computeOrientedDistance(const geom::Geometry& discreteGeom,
	                             const geom::Geometry& geom,
	                             PointPairDistance& ptDist)
	{
		MaxPointDistanceFilter distFilter(geom);
		discreteGeom.apply_ro(&distFilter);
		ptDist.setMaximum(distFilter.getMaxPointDistance());

		if (densifyFrac > 0)
		{
			MaxDensifiedByFractionDistanceFilter fracFilter(geom,
			                                          densifyFrac);
			discreteGeom.apply_ro(fracFilter);
			ptDist.setMaximum(fracFilter.getMaxPointDistance());
		}
	}

	const geom::Geometry& g0;

	const geom::Geometry& g1;

	PointPairDistance ptDist;

	/// Value of 0.0 indicates that no densification should take place
	double densifyFrac; // = 0.0;
	
};



} // geos::algorithm::distance
} // geos::algorithm
} // geos

#endif // GEOS_ALGORITHM_DISTANCE_DISCRETEHAUSDORFFDISTANCE_H

/**********************************************************************
 * $Log$
 **********************************************************************/

