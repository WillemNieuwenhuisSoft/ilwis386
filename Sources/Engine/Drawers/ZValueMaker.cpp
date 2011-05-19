#include "Headers\toolspch.h"
#include "Engine\Map\basemap.h"
#include "Engine\Table\tbl.h"
#include "Engine\Table\col.h"
#include "Engine\Drawers\ZValueMaker.h"
#include "Engine\Map\Feature.h"
#include "Drawer_n.h"
#include "Engine\Drawers\ComplexDrawer.h"

using namespace ILWIS;

#define DEFAULT_SCALE 1.0

ZValueMaker::ZValueMaker(NewDrawer *drw)  : scalingType(zvsNONE), self(true),threeDPossible(false),offset(0), zscale(DEFAULT_SCALE), zOrder(0), fakeZ(0), isSameCsy(true), associatedDrawer(drw){
	isSetDrawer = false;
	NewDrawer *parentDrw = associatedDrawer->getParentDrawer();
	if (parentDrw && !associatedDrawer->isSimple() && ((ComplexDrawer *)parentDrw)->isSet())
		isSetDrawer = true;
}
void ZValueMaker::setDataSourceMap(const BaseMap& mp){
	threeDPossible =  mp->dm()->dmt() == dmtVALUE || mp->dm()->dmt() == dmtIMAGE;
	datasourcemap = mp;
	addRange(mp);
	table = Table();
	type = IlwisObject::iotObjectType(datasourcemap->fnObj);
	self =  spatialsourcemap == datasourcemap;
	offset = 0;
	isSameCsy = spatialsourcemap->cs() == datasourcemap->cs();
	zscale = DEFAULT_SCALE;
}

void ZValueMaker::addRange(const BaseMap& mp) {
	RangeReal tempRange = mp->rrMinMax(BaseMapPtr::mmmCALCULATE);
	if ( !tempRange.fValid() && cbLimits.fValid()) {
		range = RangeReal(0,min(cbLimits.width(), cbLimits.height()));
	} else {
		range += tempRange;
	}
	if ( isSetDrawer ) {
		NewDrawer *parentDrw = associatedDrawer->getParentDrawer();
		if ( parentDrw)
			((ComplexDrawer *)parentDrw)->getZMaker()->addRange(mp);
	}
}

void ZValueMaker::setSpatialSource(const BaseMap& mp, const CoordBounds& cb) {
	spatialsourcemap = mp;
	cbLimits = cb;
}

void ZValueMaker::setTable(const Table& tbl, const String& colName) {
	table = tbl;
	columns.clear();
	Column column = tbl->col(colName);
	columns.push_back(tbl->col(colName));
	threeDPossible = column->dm()->dmt() != dmtVALUE ? false : true;
	datasourcemap = BaseMap();
	range = column->rrMinMax();
	type = IlwisObject::iotObjectType(table->fnObj);
	offset = 0;
	zscale = DEFAULT_SCALE;
}

void ZValueMaker::store(const FileName& fnView, const String& section) {
	if ( table.fValid())
		ObjectInfo::WriteElement(section.scVal(),"Table",fnView, table);
	if ( spatialsourcemap.fValid()) 
		ObjectInfo::WriteElement(section.scVal(),"Spatialsourcemap",fnView, spatialsourcemap);
	if ( datasourcemap.fValid())
		ObjectInfo::WriteElement(section.scVal(),"Datasourcemap",fnView, datasourcemap);
	if ( range.fValid())
		ObjectInfo::WriteElement(section.scVal(),"Range",fnView, range);
	ObjectInfo::WriteElement(section.scVal(),"Self",fnView, self);
	ObjectInfo::WriteElement(section.scVal(),"Zscale",fnView, zscale);
	ObjectInfo::WriteElement(section.scVal(),"Range",fnView, offset);
}

void ZValueMaker::load(const FileName& fnView, const String& section) {
	ObjectInfo::ReadElement(section.scVal(),"Table",fnView, table);
	FileName fn;
	ObjectInfo::ReadElement(section.scVal(),"Spatialsourcemap",fnView, fn);
	if ( fn.fValid())
		spatialsourcemap = BaseMap(fn);
	ObjectInfo::ReadElement(section.scVal(),"Datasourcemap",fnView, fn);
	if ( fn.fValid())
		datasourcemap = BaseMap(fn);
	ObjectInfo::ReadElement(section.scVal(),"Range",fnView, range);
	ObjectInfo::ReadElement(section.scVal(),"Self",fnView, self);
	ObjectInfo::ReadElement(section.scVal(),"Zscale",fnView, zscale);
	ObjectInfo::ReadElement(section.scVal(),"Range",fnView, offset);
}

void ZValueMaker::setTable(const Table& tbl, const vector<String>& names) {
	table = tbl;
	columns.clear();
	for(int i = 0; i < names.size(); ++i) {
		Column column = tbl->col(names[i]);
		columns.push_back(column);
		threeDPossible = column->dm()->dmt() != dmtVALUE ? false : true;
		datasourcemap = BaseMap();
		range += column->rrMinMax();
	}
	type = IlwisObject::iotObjectType(table->fnObj);
	offset = 0;
	zscale = DEFAULT_SCALE;
}

void ZValueMaker::setScaleType(ZValueTypeScaling t){
	scalingType = t;
}

RangeReal ZValueMaker::getRange() const {
	if ( isSetDrawer) {
		return ((ComplexDrawer *)associatedDrawer->getParentDrawer())->getZMaker()->getRange();
	}
	return range;
}

void ZValueMaker::setRange(const RangeReal& rr) {
	range = rr;
}

double ZValueMaker::getValue(const Coord& crd, Feature *f ){

	if (!threeDPossible)
		return spatialsourcemap->cb().width() * 0.01;
	double value = 0;
	if (self && f && type != IlwisObject::iotRASMAP) {
		//if (!datasourcemap->dvrs().fRawAvailable())
			value =  f->rValue();
		//else
		//	value = datasourcemap->dvrs().rValue(f->rValue());
	}
	if (self && type == IlwisObject::iotRASMAP){
		value = spatialsourcemap->rValue(crd);
	}
	if ( type == IlwisObject::iotRASMAP) {
		Coord c = crd;
		if (!isSameCsy) {
			c = datasourcemap->cs()->cConv( spatialsourcemap->cs(), c);
		}
		value = datasourcemap->rValue(c);
	}
	if (table.fValid()) {
		value =  columns[0]->rValue(f->iValue());
	}
	return scaleValue(value);
}

double ZValueMaker::scaleValue(double value) {
	if ( value == rUNDEF)
		return 0;
	RangeReal scaleRange = getRange();
	double scale = (value - scaleRange.rLo()) / scaleRange.rWidth();
	double zMaxSizeEstimate = (cbLimits.width() + cbLimits.height())/ 2.0;
	double endvalue = scale * zMaxSizeEstimate * 0.15;
	//if ( endvalue <= 0) {
	//	endvalue = zMaxSizeEstimate * 0.01;
	//}
	return endvalue;
}

BaseMapPtr * ZValueMaker::getSourceRasterMap() const { // we return the pointer to avoid copy constructors
	if (type == IlwisObject::iotRASMAP) {
		if (self) {
			if (spatialsourcemap.fValid())
				return spatialsourcemap.ptr();
			else
				return 0;
		}
		else if (datasourcemap.fValid())
			return datasourcemap.ptr();
		else
			return 0;
	} else
		return 0;
}
void ZValueMaker::setOffset(double v, bool useTrueCoords){
	if (useTrueCoords) {
		offset = v;
	} else {
		if ( getRange().rLo() == rUNDEF) {
			v = 0;
		}
		double scale = (v - getRange().rLo()) / getRange().rWidth();
		double zMaxSizeEstimate = (cbLimits.width() + cbLimits.height())/ 2.0;
		offset = scale * zMaxSizeEstimate * 0.25;
	}
}

double ZValueMaker::getOffset() const {
	if (offset != rUNDEF)
		return offset;
	return 0;
}

void ZValueMaker::setThreeDPossible(bool v) {
	if ( datasourcemap.fValid()) {
		threeDPossible = v && (datasourcemap->dm()->dmt() == dmtVALUE || datasourcemap->dm()->dmt() == dmtIMAGE);
	}
	else if ( columns.size() > 0) {
		threeDPossible = v && (columns[0]->dm()->dmt() != dmtVALUE ? false : true); 
	}
	else
		threeDPossible = v;
}

bool ZValueMaker::getThreeDPossible() const
{
	return threeDPossible;
}

double ZValueMaker::getZScale() const{
	return zscale;
}
void ZValueMaker::setZScale(double v){
	zscale = v;
}

void ZValueMaker::setZOrder(int index, double base) {
	zOrder = index;
	fakeZ = (zOrder+1.0) * base;
}

int ZValueMaker::getZOrder() const{
	return zOrder;
}

double ZValueMaker::getZ0(bool is3D) const{
	return is3D ? fakeZ : 0;
}

void ZValueMaker::setBounds(const CoordBounds& bnd) {
	cbLimits = bnd;
}
