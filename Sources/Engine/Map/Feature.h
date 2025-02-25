#pragma once

#include <Geos.h>
#include "Engine\Base\Algorithm\QuadTree.h"

typedef unsigned long FeatureID;

class Mask;
class DomainValueRangeStruct;

class _export Feature {
public:
	enum FeatureType{ftPOINT=1, ftSEGMENT=2, ftPOLYGON=4, ftUNKNOWN=0};
	Feature(QuadTree *tree);


	virtual String sValue(const DomainValueRangeStruct& dvs, short iWidth=-1, short iDec=-1) const = 0;
	virtual long iValue() const = 0;
	virtual double rValue() const = 0;
	virtual void PutVal(long iV) = 0;
	virtual void PutVal(double rV) = 0;
	virtual void PutVal(const DomainValueRangeStruct& dvs, const String &sVal) = 0;
	virtual bool fInMask(const DomainValueRangeStruct& dvs, const Mask& mask) const=0;
	virtual bool fValid() const =0;
	virtual FeatureType getType() const=0;
	void Delete(bool yesno=false);
	bool fDeleted() const;
	FeatureID getGuid() const;
	bool EnvelopeIntersectsWith(Geometry *g2, bool useMargine=false);
	CoordBounds Feature::cbBounds() const;
	virtual Coord centroid() const { return cbBounds().middle(); }
	virtual long nearSection(const Coord& crd, double delta, double& dis) { return iUNDEF;}
	virtual void Feature::getBoundaries(vector<CoordinateSequence*>& boundaries) const = 0;

protected:
	CCriticalSection csAccess;
	QuadTree *spatialIndex;

private:
	bool deleted;
	FeatureID guid;
	Geometry *geom;
	static FeatureID baseid;
};

