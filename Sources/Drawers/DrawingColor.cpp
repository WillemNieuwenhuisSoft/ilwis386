#include "Client\Headers\formelementspch.h"
#include "Client\FormElements\FieldIntSlider.h"
#include "Engine\Base\DataObjects\Color.h"
#include "Engine\Spatialreference\gr.h"
#include "Engine\Map\Raster\Map.h"
#include "Engine\Domain\dmsort.h"
#include "Engine\Base\Algorithm\Random.h"
#include "Engine\Base\System\RegistrySettings.h"
#include "Client\Mapwindow\MapCompositionDoc.h"
#include "Client\Mapwindow\Drawers\RootDrawer.h"
#include "Client\Mapwindow\Drawers\AbstractMapDrawer.h"
#include "Drawers\SetDrawer.h"
#include "Drawers\FeatureSetDrawer.h"
#include "Drawers\DrawingColor.h" 

using namespace ILWIS;

DrawingColor::DrawingColor(SetDrawer *dr) : 
drw(dr),
clr1(0,176,20),
clr2(168,168,168),
iMultColors(0),
gamma(0),
mcd(0)
{
	AbstractMapDrawer *mapDrawer = (AbstractMapDrawer *)drw->getParentDrawer();
	bmap.SetPointer(mapDrawer->getBaseMap());
	type = IlwisObject::iotObjectType(bmap->fnObj);
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
		case SetDrawer::smLINEAR:
			//if ( type != IlwisObject::iotRASMAP)
				cRet = (Color)rpr->clr(rVal, drw->getStretchRangeReal());
			//else
			//	cRet = (Color)rpr->clr(rVal);
			break;
		case  SetDrawer::smLOGARITHMIC:
			{
				RangeReal rr = drw->getStretchRangeReal();
				double rMax = 1 + rr.rHi() - rr.rLo();
				rMax = log(rMax);
				rVal -= rr.rLo();
				rVal = log(rVal);
				rr = RangeReal(0, rMax);
				cRet = (Color)rpr->clr(rVal, rr);
			} break;
		}
	}
	else if (NewDrawer::drmIMAGE == drw->getDrawMethod())
		cRet = (Color)rpr->clr(rVal,RangeReal(0,255));
	else
		cRet = (Color)rpr->clr(rVal);
	return cRet; //.clrDraw(gamma);
}

Color DrawingColor::clrRaw(long iRaw, NewDrawer::DrawMethod drm) const
{
	if (iUNDEF == iRaw)
		return Color(0,0,0);
	Color cRet;
	switch (drm) {
	case NewDrawer::drmRPR:
		if (bmap->dm()->pdv()) {
			double rVal = bmap->dvrs().rValue(iRaw);
			return clrVal(rVal);
		}
		else {
			Representation rpr = drw->getRepresentation();
			cRet = Color(rpr->clrRaw(iRaw));
		}
		break;
	case NewDrawer::drmSINGLE:
		if ((long)clr2 == -1)
			cRet = GetSysColor(COLOR_WINDOWTEXT);
		else {
			FeatureSetDrawer *fdr = dynamic_cast<FeatureSetDrawer *>(drw);
			if ( fdr)
				cRet = fdr->getSingleColor();
		}
		break;
	case NewDrawer::drmMULTIPLE: 
		if (3 == iMultColors)
			cRet = clrRandom(iRaw);
		else {
			int iStep = 7;
			switch (iMultColors) {
	case 0: iStep = 7; break;
	case 1: iStep = 15; break;
	case 2: iStep = 31; break;
			}
			cRet = clrPrimary(1+iRaw%iStep);
		}  
		break;
	case NewDrawer::drmIMAGE: {
		RangeInt riStretch = drw->getStretchRangeInt();
		int iMin = 0, iMax = 255;
		if (drw->isStretched()) {
			iMin = riStretch.iLo();
			iMax = riStretch.iHi();
		}
		int iDiff = iMax - iMin;
		if (iRaw < iMin)
			iRaw = iMin;
		else if (iRaw > iMax)
			iRaw = iMax;
		int iVal = (int)(floor(255 * float(iRaw - iMin) / iDiff));
		cRet = Color(iVal,iVal,iVal);
									  } break;
	case NewDrawer::drmCOLOR:
		cRet = Color(iRaw);
		break;
	case NewDrawer::drmBOOL:
		switch (iRaw) {
	case 0: return Color(0,0,0);
	case 1: cRet = clr1; break;
	case 2: cRet = clr2;  break;
		}
		break;
	}
	if ((long)cRet < 0)
		return Color(0,0,0);
	else
		return cRet;//.clrDraw(gamma);
}

void DrawingColor::clrVal(const double * buf, long * bufOut, long iLen) const
{
	Representation rpr = drw->getRepresentation();
	if (!rpr.fValid())
		return;
	if (drw->isStretched()) {
		switch (drw->getStretchMethod())
		{
		case SetDrawer::smLINEAR:
			{
				RangeReal rr = drw->getStretchRangeReal();
				for (long i = 0; i < iLen; ++i)
					bufOut[i] = rpr->clr(buf[i], rr).iVal();
			} break;
		case SetDrawer::smLOGARITHMIC:
			{
				RangeReal rr = drw->getStretchRangeReal();
				double rMax = 1 + rr.rHi() - rr.rLo();
				rr = RangeReal(0, log(rMax));
				for (long i = 0; i < iLen; ++i)
					bufOut[i] = rpr->clr(log(buf[i] - rr.rLo()), rr).iVal();
			} break;
		}
	}
	else if (NewDrawer::drmIMAGE == drw->getDrawMethod()) {
		RangeReal rr = RangeReal(0, 255);
		for (long i = 0; i < iLen; ++i)
			bufOut[i] = rpr->clr(buf[i], rr).iVal();
	}
	else {
		for (long i = 0; i < iLen; ++i)
			bufOut[i] = rpr->clr(buf[i]).iVal();
	}
}

void DrawingColor::clrRaw(const long * buf, long * bufOut, long iLen, NewDrawer::DrawMethod drm) const
{
	switch (drm) {
	case NewDrawer::drmRPR:
		{
			Representation rpr = drw->getRepresentation();
			if (bmap->dm()->pdv()) {
				if (drw->isStretched()) {
					switch (drw->getStretchMethod())
					{
					case SetDrawer::smLINEAR: {
						RangeReal rr = drw->getStretchRangeReal();
						DomainValueRangeStruct dvrs = bmap->dvrs();
						for (long i = 0; i < iLen; ++i)
							bufOut[i] = rpr->clr(dvrs.rValue(buf[i]), rr).iVal();
					} break;
					case SetDrawer::smLOGARITHMIC:
						{
							RangeReal rr = drw->getStretchRangeReal();
							double rMax = 1 + rr.rHi() - rr.rLo();
							rr = RangeReal(0, log(rMax));
							DomainValueRangeStruct dvrs = bmap->dvrs();
							for (long i = 0; i < iLen; ++i)
								bufOut[i] = rpr->clr(log(dvrs.rValue(buf[i]) - rr.rLo()), rr).iVal();
						} break;
					}
				}
				else if (NewDrawer::drmIMAGE == drw->getDrawMethod()) {
					RangeReal rr = RangeReal(0, 255);
					DomainValueRangeStruct dvrs = bmap->dvrs();
					for (long i = 0; i < iLen; ++i)
						bufOut[i] = rpr->clr(dvrs.rValue(buf[i]), rr).iVal();
				}
				else {
					DomainValueRangeStruct dvrs = bmap->dvrs();
					for (long i = 0; i < iLen; ++i)
						bufOut[i] = rpr->clr(dvrs.rValue(buf[i])).iVal();
				}
			}
			else {
				for (long i = 0; i < iLen; ++i)
					bufOut[i] = rpr->clrRaw(buf[i]).iVal();
			}
		} break;
	case NewDrawer::drmSINGLE: {
		Color col;
		if ((long)clr2 == -1)
			col = GetSysColor(COLOR_WINDOWTEXT);
		else {
			FeatureSetDrawer *fdr = dynamic_cast<FeatureSetDrawer *>(drw);
			if ( fdr)
				col = fdr->getSingleColor();
		}
		for (long i = 0; i < iLen; ++i)
			bufOut[i] = col.iVal(); // you asked for it (!)
	} break;
	case NewDrawer::drmMULTIPLE: 
		if (3 == iMultColors) {
			for (long i = 0; i < iLen; ++i)
				bufOut[i] = clrRandom(buf[i]).iVal();
		}
		else {
			int iStep = 7;
			switch (iMultColors) {
			case 0: iStep = 7; break;
			case 1: iStep = 15; break;
			case 2: iStep = 31; break;
			}
			for (long i = 0; i < iLen; ++i)
				bufOut[i] = clrPrimary(1 + buf[i] % iStep).iVal();
		}  
		break;
	case NewDrawer::drmIMAGE: {
		RangeInt riStretch = drw->getStretchRangeInt();
		int iMin = 0, iMax = 255;
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
	  } break;
	case NewDrawer::drmCOLOR:
		memcpy(bufOut, buf, iLen * sizeof(long)); // no change !!
		break;
	case NewDrawer::drmBOOL: 
		for (long i = 0; i < iLen; ++i) {
			long iRaw = buf[i];
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
	DomainSort* ds = drw->getRepresentation()->dm()->pdsrt();
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

Color DrawingColor::clrPrimary(int iNr) const
{
	switch (iNr%32) {
	case  0: return Color(  0,  0,  0);
	case  1: return Color(255,  0,  0);
	case  2: return Color(255,255,  0);
	case  3: return Color(  0,  0,255);
	case  4: return Color(255,  0,255);
	case  5: return Color(  0,255,255);
	case  6: return Color(  0,255,  0);
	case  7: return Color(128,128,128);
	case  8: return Color(224,224,224);  // was white 255,255,255
	case  9: return Color(128,  0,  0);
	case 10: return Color(128,128,  0);
	case 11: return Color(  0,  0,128);
	case 12: return Color(128,  0,128);
	case 13: return Color(  0,128,128);
	case 14: return Color(  0,128,  0);
	case 15: return Color(255,128,  0);
	case 16: return Color(191,  0,  0);
	case 17: return Color(191,191,  0);
	case 18: return Color(  0,  0,191);
	case 19: return Color(191,  0,191);
	case 20: return Color(  0,191,191);
	case 21: return Color(  0,191,  0);
	case 22: return Color(191,191,191);
	case 23: return Color(192,220,192);
	case 24: return Color( 63,  0,  0);
	case 25: return Color( 63, 63,  0);
	case 26: return Color(  0,  0, 63);
	case 27: return Color( 63,  0, 63);
	case 28: return Color(  0, 63, 63);
	case 29: return Color(  0, 63,  0);
	case 30: return Color( 63, 63, 63);
	case 31: return Color(127, 63,  0);
	}  
	return Color();
}