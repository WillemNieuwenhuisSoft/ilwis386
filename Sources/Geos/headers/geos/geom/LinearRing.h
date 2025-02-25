/**********************************************************************
 * $Id: LinearRing.h 2556 2009-06-06 22:22:28Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/LinearRing.java rev. 1.32 (JTS-1.10)
 *
 **********************************************************************/

#ifndef GEOS_GEOS_LINEARRING_H
#define GEOS_GEOS_LINEARRING_H

#include <geos/export.h>
#include <string>
#include <vector>
#include <geos/platform.h>
#include <geos/geom/LineString.h>

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geom { // geos::geom
		class Coordinate;
		class CoordinateArraySequence;
	}
}

namespace geos {
namespace geom { // geos::geom

/**
 * \brief 
 * Models an OGC SFS <code>LinearRing</code>.
 *
 * A LinearRing is a LineString which is both closed and simple.
 * In other words,
 * the first and last coordinate in the ring must be equal,
 * and the interior of the ring must not self-intersect.
 * Either orientation of the ring is allowed.
 * 
 * A ring must have either 0 or 4 or more points.
 * The first and last points must be equal (in 2D).
 * If these conditions are not met, the constructors throw
 * an {@link IllegalArgumentException}
 */
class LinearRing : public LineString {

public:

	_export LinearRing(const LinearRing &lr);

	/**
	 * \brief Constructs a <code>LinearRing</code> with the given points.
	 *
	 * @param  points  points forming a closed and simple linestring, or
	 *      <code>null</code> or an empty array to create the empty
	 *      geometry.
	 *      This array must not contain <code>null</code> elements.
	 *	If not null LinearRing will take ownership of points.
	 *
	 * @param newFactory the GeometryFactory used to create this geometry
	 *
	 */
	_export LinearRing(CoordinateSequence* points,
			const GeometryFactory *newFactory);

	/// Hopefully cleaner version of the above
	_export LinearRing(CoordinateSequence::AutoPtr points,
			const GeometryFactory *newFactory);

	_export virtual Geometry *clone() const { return new LinearRing(*this); }

	_export virtual ~LinearRing();

	/** \brief
	 * Returns <code>Dimension.FALSE</code>, since by definition
	 * LinearRings do not have a boundary.
	 *
	 * @return Dimension::False
	 */
	_export int getBoundaryDimension() const;

	/** \brief
	 * Returns <code>true</code>, since by definition LinearRings
	 * are always simple.
	 *
	 * @return <code>true</code>
	 *
	 * @see Geometry::isSimple
	 */
	_export bool isSimple() const;

	_export std::string getGeometryType() const;

	_export virtual GeometryTypeId getGeometryTypeId() const;

	_export void setPoints(CoordinateSequence* cl);

  	_export Geometry* reverse() const;

private:

	void validateConstruction();
};


} // namespace geos::geom
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geom/LinearRing.inl"
//#endif

#endif // ndef GEOS_GEOS_LINEARRING_H

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/04/11 11:16:25  strk
 * Added LineString and LinearRing constructors by auto_ptr
 *
 * Revision 1.3  2006/04/10 17:35:44  strk
 * Changed LineString::points and Point::coordinates to be wrapped
 * in an auto_ptr<>. This should close bugs #86 and #89
 *
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
