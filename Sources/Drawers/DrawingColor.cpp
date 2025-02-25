#include "Headers\toolspch.h"
#include "Engine\Base\DataObjects\Color.h"
#include "Engine\Spatialreference\gr.h"
#include "Engine\Map\Raster\Map.h"
#include "Engine\Domain\dmsort.h"
#include "Engine\Representation\Rpr.h"
#include "Engine\Base\Algorithm\Random.h"
#include "Engine\Base\System\RegistrySettings.h"
#include "Engine\Drawers\RootDrawer.h"
#include "Engine\Drawers\SpatialDataDrawer.h"
#include "Drawers\SetDrawer.h"
#include "LayerDrawer.h"
#include "FeatureLayerDrawer.h"
#include "DrawingColor.h" 

using namespace ILWIS;

// setting the basemap invalidates the column
void IlwisData::setBaseMap(const BaseMap& bm){
	bmap = bm;
	col = Column();
}

// setting the column doesnt invalidate the basemap as we will retain it as the default
void IlwisData::setColumn(const Column& c){
	col = c;
}

DomainValueRangeStruct IlwisData::dvrs() const{
	if ( col.fValid()) {
		return col->dvrs();
	}
	return bmap->dvrs();
}

Domain IlwisData::dm() const{
	if ( col.fValid()) {
		return col->dm();
	}
	return bmap->dm();
}

double IlwisData::rValByRaw(int raw, const RangeReal& range) const{
	if ( col.fValid()) {
		if ( col->dm()->pdv())
			return col->rValue(raw);
		else if ( col->dm()->pdsrt()) {
			/*	String vv = bmap->dm()->pdsrt()->sValueByRaw(raw);
			long iRec = bmap->dm()->pdsrt()->iOrd(raw); */
			String sV = col->sValue(raw);
			long r = col->dm()->pdsrt()->iRaw(sV);
			return r;
		}
	}
	if ( dm()->pdsrt() || dm()->pdp())
		return raw;

	if ( range.fValid()) {
		double step = dvrs().rStep();
		double v = ( raw + dvrs().vr()->getOffset()) * step;
		return v;
	}
	return bmap->dvrs().rValue(raw);
}

long IlwisData::iRawAttr(long iRaw) const {
	if (col.fValid())
		return col->iRaw(iRaw);
	else
		return iRaw;
}

bool IlwisData::fUsesCol() const {
	return col.fValid();
}

const Column IlwisData::getCol() const {
	return col;
}

DrawingColor::DrawingColor(ComplexDrawer *dr, int ind) : 
clr1(168,168,168), // False
clr2(0,176,20), // True
iMultColors(0),
mcd(0),
index(ind),
tresholdColor(Color(255,0,0)),
colorSetIndex(0),
useSingleValueForTreshold(false)
{
	IlwisSettings settings("DefaultSettings");
	Color clr1a = settings.clrValue("NoColor", Color()); // do not compare with colorUNDEF, because then we can't assign the black color; the registry has the inverse alpha
	Color clr2a = settings.clrValue("YesColor", Color());
	if (clr1a != Color()) {
		clr1 = clr1a;
		clr1.alpha() = 255 - clr1.alpha();
	}
	if (clr2a != Color()) {
		clr2 = clr2a;
		clr2.alpha() = 255 - clr2.alpha();
	}

	BaseMap bmap;
	if ( dr->isSet()) {
		setDrawer = (SetDrawer *)dr;
		rangeData = setDrawer->getRange();
		bmap.SetPointer(setDrawer->getBaseMap(index));
		drw = (LayerDrawer *)setDrawer->getDrawer(ind);

	} else {
		drw = (LayerDrawer *)dr;
		SpatialDataDrawer *mapDrawer = (SpatialDataDrawer *)drw->getParentDrawer();
		setDrawer = dynamic_cast<SetDrawer *>(mapDrawer);
		bmap.SetPointer(mapDrawer->getBaseMap(index));
		rangeData = mapDrawer->getRange();
	}

	dataValues.setBaseMap(bmap);
	type = IlwisObject::iotObjectType(bmap->fnObj);
}

void DrawingColor::setDataColumn(const Column& c) {
	dataValues.setColumn(c);
}

RangeReal DrawingColor::getStretchRangeReal() const {
	return setDrawer ? setDrawer->getStretchRangeReal() : drw->getStretchRangeReal();
}
void DrawingColor::color1(const Color& clr){
	clr1 = clr;
}

void DrawingColor::color2(const Color& clr){
	clr2 = clr;
}

Color DrawingColor::getColor1() const {
	return clr1;
}

Color DrawingColor::getColor2() const {
	return clr2;
}

Color DrawingColor::clrVal(double rVal) const
{
	Color cRet;
	Representation rpr = drw->getRepresentation();
	if (!rpr.fValid())
		return cRet;
	if (drw->isStretched()) {
		switch (drw->getStretchMethod())
		{
		case LayerDrawer::smLINEAR:
			cRet = (Color)rpr->clr(rVal, getStretchRangeReal());
			break;
		case  LayerDrawer::smLOGARITHMIC:
			{
				RangeReal rr = getStretchRangeReal();
				double rMax = 1 + rr.rHi() - rr.rLo();
				rMax = log(rMax);
				rr = RangeReal(0, rMax);
				rVal = log(1 + rVal - rr.rLo());
				cRet = (Color)rpr->clr(rVal, rr);
			} break;
		}
	}
	else if (NewDrawer::drmIMAGE == drw->getDrawMethod())
		cRet = (Color)rpr->clr(rVal,RangeReal(0,255));
	else
		cRet = (Color)rpr->clr(rVal);

	setTransparency(rVal, cRet);
	setTresholdColors(rVal, cRet);

	return cRet;
}

Color DrawingColor::clrRaw(long iRaw, NewDrawer::DrawMethod drm) const
{
	if (iUNDEF == iRaw)
		return Color(0,0,0,0);
	Color cRet;
	switch (drm) {
	case NewDrawer::drmRPR: 
		{
			double rVal = dataValues.rValByRaw(iRaw);
			if ( rVal == rUNDEF || rVal == iUNDEF || rVal == shUNDEF)
				return colorUNDEF;

			if (dataValues.dm()->pdv()) {
				cRet =  clrVal(rVal);
			}
			else {
				Representation rpr = drw->getRepresentation();
				if (rpr.fValid())
					cRet = Color(rpr->clrRaw(rVal));
				else
					cRet = iRaw; // shouldn't happen, but prevent crash when the rpr is invalid, and at least show the user that something is wrong
			}
			setTransparency(rVal, cRet);
			setTresholdColors(rVal, cRet);
		}
		break;
	case NewDrawer::drmSINGLE:
		{
			double rVal = dataValues.rValByRaw(iRaw);
			if ( rVal == rUNDEF || rVal == iUNDEF || rVal == shUNDEF)
				return colorUNDEF;
			if (clr2.iVal() == -1)
				cRet = GetSysColor(COLOR_WINDOWTEXT);
			else {
				FeatureLayerDrawer *fdr = dynamic_cast<FeatureLayerDrawer *>(drw);
				if ( fdr)
					cRet = fdr->getSingleColor();
			}
		}
		break;
	case NewDrawer::drmMULTIPLE:
		{
			double rVal = dataValues.rValByRaw(iRaw);
			if ( rVal == rUNDEF || rVal == iUNDEF || rVal == shUNDEF)
				return colorUNDEF;
			iRaw = dataValues.iRawAttr(iRaw);
			if (3 == iMultColors)
				cRet = clrRandom(iRaw);
			else {
				int iStep = 7;
				switch (iMultColors) {
					case 0: iStep = 7; break;
					case 1: iStep = 15; break;
					case 2: iStep = 31; break;
				}
				cRet = clrPrimary(1+iRaw%iStep, colorSetIndex);
			}
		}  
		break;
	case NewDrawer::drmIMAGE: {
		RangeReal rrStretch = getStretchRangeReal();
		int iMin = 0, iMax = 255;
		if (drw->isStretched()) {
			iMin = rrStretch.rLo();
			iMax = rrStretch.rHi();
		}
		int iDiff = iMax - iMin;
		if (iRaw < iMin)
			iRaw = iMin;
		else if (iRaw > iMax)
			iRaw = iMax;
		int iVal = (int)(floor(255 * float(iRaw - iMin) / iDiff));
		cRet = Color(iRaw,iVal,iVal);
		setTransparency(iRaw, cRet);
		setTresholdColors(iRaw, cRet);
							  } break;
	case NewDrawer::drmCOLOR:
		cRet = Color(dataValues.iRawAttr(iRaw));
		break;
	case NewDrawer::drmBOOL:
		switch (dataValues.iRawAttr(iRaw)) {
			case 0: return Color(0,0,0);
			case 1: cRet = clr1; break;
			case 2: cRet = clr2; break;
		}
		break;
	}
	return cRet;
}

void DrawingColor::clrVal(const double * buf, long * bufOut, long iLen) const
{
	Representation rpr = drw->getRepresentation();
	bool done = false;
	if (!rpr.fValid())
		return;
	if (drw->isStretched()) {
		switch (drw->getStretchMethod())
		{
		case LayerDrawer::smLINEAR:
			{
				RangeReal rr = getStretchRangeReal();
				
				for (long i = 0; i < iLen; ++i) {
					double v = buf[i];
					Color clr = rpr->clr(v, rr);
					setTransparency(v, clr);
					if (!done)
						done = setTresholdColors(v, clr);
					bufOut[i] = clr.iVal();
				}
			} break;
		case LayerDrawer::smLOGARITHMIC:
			{
				RangeReal rr = getStretchRangeReal();
				double rMax = 1 + rr.rHi() - rr.rLo();
				rr = RangeReal(0, log(rMax));
				for (long i = 0; i < iLen; ++i) {
					Color clr = rpr->clr(log(1 + buf[i] - rr.rLo()), rr);
					setTransparency(buf[i], clr);
					if (!done)
						done = setTresholdColors(buf[i], clr);
					bufOut[i] = clr.iVal();
				}
			} break;
		}
	}
	else if (NewDrawer::drmIMAGE == drw->getDrawMethod()) {
		RangeReal rr = RangeReal(0, 255);
		for (long i = 0; i < iLen; ++i) {
			long v = buf[i];
			Color clr = rpr->clr(buf[i], rr);
			setTransparency(v, clr);
			if (!done)
				done = setTresholdColors(v, clr);
			bufOut[i] = clr.iVal();
		}
	}
	else {
		for (long i = 0; i < iLen; ++i) {
			double v = buf[i];
			Color clr = rpr->clr(v);
			setTransparency(v, clr);
			if (!done)
				done = setTresholdColors(v, clr);
			bufOut[i] = clr.iVal();
		}
	}
}

inline void DrawingColor::setTransparency(double v, Color& clr) const{
	if ( transpValues.fValid()) {
		if ( transpValues.fContains(v)) {
			clr.alpha() = 0;
		}
	}
}

inline bool DrawingColor::setTresholdColors(double v, Color& clr) const{
	if ( tresholdValues.fValid()) {
		if ( !useSingleValueForTreshold) {
			if ( tresholdValues.fContains(v)) {
				clr = tresholdColor;
			}
		} else {
			if ( v >= tresholdValues.rHi()) {
				clr = tresholdColor;
				return true;
			}
		}
	}
	return false;
}


void DrawingColor::clrRaw(const long * buf, long * bufOut, long iLen, NewDrawer::DrawMethod drm) const
{
	switch (drm) {
	case NewDrawer::drmRPR:
	case NewDrawer::drmIMAGE:
		{
			Representation rpr = drw->getRepresentation();
			if (rpr.fValid()) {
				if (dataValues.dm()->pdv()) {
					if (drw->isStretched()) {
						switch (drw->getStretchMethod())
						{
						case LayerDrawer::smLINEAR: {
							RangeReal rr = getStretchRangeReal();
							DomainValueRangeStruct dvrs = dataValues.dvrs();
							for (long i = 0; i < iLen; ++i) {
								double v = dataValues.rValByRaw(buf[i], rangeData);
								Color clr = rpr->clr(v, rr);
								setTransparency(v, clr);
								setTresholdColors(v, clr);
								bufOut[i] = clr.iVal();
							}
													} break;
						case LayerDrawer::smLOGARITHMIC:
							{
								RangeReal rr = getStretchRangeReal();
								double rMax = 1 + rr.rHi() - rr.rLo();
								rr = RangeReal(0, log(rMax));
								DomainValueRangeStruct dvrs = dataValues.dvrs();
								for (long i = 0; i < iLen; ++i){
									double v = dataValues.rValByRaw(buf[i]);
									Color clr = rpr->clr(log(1 + dvrs.rValue(v) - rr.rLo()), rr);
									setTransparency(v, clr);
									setTresholdColors(v, clr);
									bufOut[i] = clr.iVal();
								}
							} break;
						}
					}
					else if (NewDrawer::drmIMAGE == drw->getDrawMethod()) {
						RangeReal rr = RangeReal(0, 255);
						DomainValueRangeStruct dvrs = dataValues.dvrs();
						for (long i = 0; i < iLen; ++i){
							long v = dvrs.rValue(buf[i]);
							Color clr = rpr->clr(v, rr);
							setTransparency(v, clr);
							setTresholdColors(v, clr);
							bufOut[i] = clr.iVal();
						}
					}
					else {
						DomainValueRangeStruct dvrs = dataValues.dvrs();
						for (long i = 0; i < iLen; ++i){
							double v = dataValues.rValByRaw(buf[i]);
							Color clr = rpr->clr(v);
							setTransparency(v, clr);
							setTresholdColors(v, clr);
							bufOut[i] = clr.iVal();
						}
					}
				}
				else {
					for (long i = 0; i < iLen; ++i) {
						double rVal = dataValues.rValByRaw(buf[i]);
						if ( rVal == rUNDEF || rVal == iUNDEF || rVal == shUNDEF)
							bufOut[i] = colorUNDEF.iVal();
						else {
							Color clr = rpr->clrRaw(rVal).iVal();
							bufOut[i] = clr.iVal();
						}
					}
				}
			} else {
				memcpy(bufOut, buf, iLen * sizeof(long)); // shouldn't happen, but prevent crash when the rpr is invalid, and at least show the user that something is wrong
			}
		} break;
	case NewDrawer::drmSINGLE: {
		Color col;
		if (clr2.iVal() == -1)
			col = GetSysColor(COLOR_WINDOWTEXT);
		else {
			FeatureLayerDrawer *fdr = dynamic_cast<FeatureLayerDrawer *>(drw);
			if ( fdr)
				col = fdr->getSingleColor();
		}
		for (long i = 0; i < iLen; ++i)
			bufOut[i] = col.iVal(); // you asked for it (!)
							   } break;
	case NewDrawer::drmMULTIPLE:
		{
			Representation rpr = drw->getRepresentation();
			bool rprValid = rpr.fValid();
			if (3 == iMultColors) {
				if (rprValid) {
					for (long i = 0; i < iLen; ++i) {
						long iRaw = dataValues.iRawAttr(buf[i]);
						Color clr = clrRandom(iRaw);
						clr.alpha() = rpr->clrRaw(iRaw).alpha();
						bufOut[i] = clr.iVal();
					}
				} else {
					for (long i = 0; i < iLen; ++i) {
						bufOut[i] = clrRandom(dataValues.iRawAttr(buf[i])).iVal();
					}
				}
			} else {
				int iStep = 7;
				switch (iMultColors) {
				case 0: iStep = 7; break;
				case 1: iStep = 15; break;
				case 2: iStep = 31; break;
				}
				if (rprValid) {
					for (long i = 0; i < iLen; ++i) {
						long iRaw = dataValues.iRawAttr(buf[i]);
						Color clr = clrPrimary(1 + dataValues.iRawAttr(buf[i]) % iStep, colorSetIndex);
						clr.alpha() = rpr->clrRaw(iRaw).alpha();
						bufOut[i] = clr.iVal();
					}
				} else {
					for (long i = 0; i < iLen; ++i)
						bufOut[i] = clrPrimary(1 + dataValues.iRawAttr(buf[i]) % iStep, colorSetIndex).iVal();
				}
			}
		}
		break;
		/*case NewDrawer::drmIMAGE: {
		RangeInt riStretch = drw->getStretchRangeInt();
		,		int iMin = 0, iMax = 255;
		if (drw->isStretched()) {
		iMin = riStretch.iLo();
		iMax = riStretch.iHi();
		}
		int iDiff = iMax - iMin;
		for (long i = 0; i < iLen; ++i) {
		long iRaw = buf[i];
		if (iRaw < iMin)
		iRaw = iMin;
		else if (iRaw > iMax)
		iRaw = iMax;
		int iVal = (int)(floor(255 * float(iRaw - iMin) / iDiff));
		bufOut[i] = Color(iVal,iVal,iVal).iVal();
		}
		} break;*/
	case NewDrawer::drmCOLOR:
		if (dataValues.fUsesCol()) {
			for (long i = 0; i < iLen; ++i) {
				Color clr (dataValues.iRawAttr(buf[i]));
				clr.alpha() = 255 - clr.alpha();
				bufOut[i] = clr.iVal();
			}
		} else
			memcpy(bufOut, buf, iLen * sizeof(long)); // no change !!
		break;
	case NewDrawer::drmBOOL: 
		for (long i = 0; i < iLen; ++i) {
			long iRaw = dataValues.iRawAttr(buf[i]);
			bufOut[i] = (iRaw == 1)?clr1.iVal():((iRaw == 2)?clr2.iVal():Color(0,0,0).iVal());
		}
		break;
	}
}

Color DrawingColor::clrRandom(int iRaw) const
{
	iRaw = abs(iRaw);
	if (iRaw > m_clrRandom.size())
		iRaw %= m_clrRandom.size();
	return m_clrRandom[iRaw];
}

void DrawingColor::InitClrRandom()
{ 
	m_clrRandom.clear();
	DomainSort* ds = 0;
	if (drw->useAttributeColumn())
		ds = drw->getAtttributeColumn()->dm()->pdsrt();
	else {
		BaseMap *bmp = (BaseMap *)drw->getDataSource();
		if (bmp && bmp->fValid())
			ds = (*bmp)->dm()->pdsrt();
	}

	int iSize = 1000;
	if (ds)
		iSize = 1 + ds->iSize();
	if ( iSize < 0)
		return;
	m_clrRandom.resize(iSize);
	for (int i = 0; i < iSize; ++i)
	{
		Color clr;
		clr.red() = (byte)random(256);
		clr.green() = (byte)random(256);
		clr.blue() = (byte)random(256);
		m_clrRandom[i] = clr;
	}
}

Color DrawingColor::clrPrimary(int iNr, int set) 
{
	return Representation::clrPrimary(iNr, set);

}

void DrawingColor::setTransparentValues(const RangeReal& rr){
	transpValues = rr;
}

RangeReal DrawingColor::getTransparentValues() const{
	return transpValues;
}

Color DrawingColor::getTresholdColor() const{
	return tresholdColor;
}

void DrawingColor::setTresholdColor(const Color&clr){
	tresholdColor = clr;
}

RangeReal DrawingColor::getTresholdRange() const{
	return tresholdValues;
}

void DrawingColor::setTresholdRange(const RangeReal& rr, bool single){
	tresholdValues = rr;
	useSingleValueForTreshold = single;
}

String DrawingColor::store(const FileName& fnView, const String& section) const{
	String currentSection = section + ":DrawingColor";
	ObjectInfo::WriteElement(currentSection.c_str(),"TransparencyValues",fnView, transpValues);
	ObjectInfo::WriteElement(currentSection.c_str(),"IdColorSet",fnView, colorSetIndex);
	ObjectInfo::WriteElement(currentSection.c_str(),"IdColorNumbers",fnView, iMultColors);
	ObjectInfo::WriteElement(currentSection.c_str(),"color1",fnView, clr1);
	ObjectInfo::WriteElement(currentSection.c_str(),"color2",fnView, clr2);
	if (dataValues.fUsesCol()) {
		ObjectInfo::WriteElement(currentSection.c_str(),"AttributeTable",fnView, dataValues.getCol()->fnTbl.sRelativeQuoted());
		ObjectInfo::WriteElement(currentSection.c_str(),"AttributeColumn",fnView, dataValues.getCol()->sNameQuoted());
	}
	return currentSection;
}

void DrawingColor::load(const FileName& fnView, const String& section){
	String currentSection = section + ":DrawingColor";
	ObjectInfo::ReadElement(currentSection.c_str(),"TransparencyValues",fnView, transpValues);
	ObjectInfo::ReadElement(currentSection.c_str(),"IdColorSet",fnView, colorSetIndex);
	ObjectInfo::ReadElement(currentSection.c_str(),"IdColorNumbers",fnView, iMultColors);
	ObjectInfo::ReadElement(currentSection.c_str(),"color1",fnView, clr1);
	ObjectInfo::ReadElement(currentSection.c_str(),"color2",fnView, clr2);
	Table tbl;
	ObjectInfo::ReadElement(currentSection.c_str(), "AttributeTable", fnView, tbl);
	if (tbl.fValid()) {
		String colname;
		ObjectInfo::ReadElement(currentSection.c_str(),"AttributeColumn",fnView, colname);
		if (colname != "") {
			Column col = tbl->col(colname);
			dataValues.setColumn(col);
		}
	}
	if (iMultColors == 3)
		InitClrRandom();
}

void DrawingColor::setMultiColors(int index, bool fInitRandom){
	if ( index <= 3) {
		iMultColors = index;
		if (fInitRandom && (index == 3))
			InitClrRandom();
		else
			m_clrRandom.clear();
	}
}
int DrawingColor::multiColors() const{
	return iMultColors;
}
void DrawingColor::setColorSet(int index){
	colorSetIndex = index;
}
int DrawingColor::colorSet() const{
	return colorSetIndex;
}

void DrawingColor::copyClrRandomFrom(DrawingColor & dcOther) {
	m_clrRandom = dcOther.m_clrRandom;
}
