/***************************************************************
ILWIS integrates image, vector and thematic data in one unique 
and powerful package on the desktop. ILWIS delivers a wide 
range of feautures including import/export, digitizing, editing, 
analysis and display of data as well as production of 
quality mapsinformation about the sensor mounting platform

Exclusive rights of use by 52�North Initiative for Geospatial 
Open Source Software GmbH 2007, Germany

Copyright (C) 2007 by 52�North Initiative for Geospatial
Open Source Software GmbH

Author: Jan Hendrikse, Willem Nieuwenhuis,Wim Koolhoven 
Bas Restsios, Martin Schouwenburg, Lichun Wang, Jelle Wind 

Contact: Martin Schouwenburg; schouwenburg@itc.nl; 
tel +31-534874371

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (see gnu-gpl v2.txt); if not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA or visit the web page of the Free
Software Foundation, http://www.fsf.org.

Created on: 2007-02-8
***************************************************************/
/*
// $Log: /ILWIS 3.0/Table/Tblhstsg.cpp $
* 
* 7     23-03-01 12:03 Koolhoven
* protected QuickSort() against sorting from 0 to -1 which would cause a
* crash 
* 
* 6     5-03-01 12:38 Koolhoven
* in Erase() use PostMessage() instead of PostThreadMessage() to update
* the catalog
* 
* 5     8-02-00 17:58 Wind
* no update of catalog in Erase() if file was already erased
* 
* 4     14-01-00 17:19 Koolhoven
* Post reread Catalog as thread message
* 
* 3     9/08/99 11:55a Wind
* comments
* 
* 2     9/08/99 10:28a Wind
* adpated to use of quoted file names in sExpression()
*/
// Revision 1.10  1998/09/16 17:25:32  Wim
// 22beta2
//
// Revision 1.9  1998/03/09 16:56:04  janh
// hisl.rLen replaced by hislHash.rLen in first else-block
// after #pragma in function TableHistogramSeg::fCount()
//
// Revision 1.8  1997/09/18 12:20:24  Wim
// Send ILW_READCATALOG message after deleting files in Erase()
//
// Revision 1.7  1997-09-01 15:15:51+02  Wim
// set help topic on in Init()
//
// Revision 1.6  1997-08-27 16:36:47+02  martin
// Domain is now set inthe freezing function
//
// Revision 1.5  1997/08/18 13:51:05  martin
// Removed freeze from constructor
//
// Revision 1.4  1997/08/15 14:20:33  Wim
// Optimized riIndexMinMax(rPerc)
//
// Revision 1.3  1997-08-15 16:09:23+02  Wim
// Simplified code in riIndexMinMax()
//
// Revision 1.2  1997-08-12 18:14:45+02  Wim
// Do not try to fill columns when no pts is available
//
/* TableHistogramSeg
Copyright Ilwis System Development ITC
april 1995, by Jelle Wind
Last change:  WK    1 Jul 98   10:03 am
*/

//////// only properly implementef for byte maps

#include "Engine\Table\TBLHSTSG.H"
#include "Engine\Table\tblstore.h"
#include "Engine\Table\COLSTORE.H"
#include "Engine\Domain\Dmvalue.h"
#include "Engine\Applications\SEGVIRT.H"
#include "Engine\Table\COLLONG.H"
#include "Engine\Table\COLREAL.H"
#include "Engine\Base\DataObjects\valrange.h"
#include "Engine\Base\Algorithm\Qsort.h"
#include "Headers\Htp\Ilwisapp.htp"
#include "Headers\Err\Ilwisapp.err"
#include "Headers\constant.h"
#include "Headers\Hs\tbl.hs"

IlwisObjectPtr * createTableHistogramSeg(const FileName& fn, IlwisObjectPtr& ptr, const String& sExpr, vector<void *> parms ) {
	if ( sExpr == "")
		return (IlwisObjectPtr *)new TableHistogramSeg(fn, (TablePtr &)ptr);

	Array<String> as(1);
	if (!IlwisObjectPtr::fParseParm(sExpr, as))
		throw ErrorExpression(fn.sFullName(), sExpr);
	FileName filnam(as[0], ".hss", true);
	if (File::fExist(filnam))
		// histogram file exists already
		return (IlwisObjectPtr *)new TableHistogramSeg(filnam, (TablePtr&)ptr);
	else // create new histogram from map
		return (IlwisObjectPtr *)new TableHistogramSeg(SegmentMap(String(as[0])), (TablePtr&)ptr);
}

const char* TableHistogramSeg::sSyntax() { return "TableHistogramSeg(map)"; }

TableHistogramSeg::TableHistogramSeg(const FileName& fn, TablePtr& p)
: TableVirtual(fn, p)
{
	FileName fnMap;
	ReadElement("TableHistogramSeg", "Map", fnMap);
	try {
		map = SegmentMap(fnMap);
	}
	catch (const ErrorObject& err) {
		map = SegmentMap();
		throw err;
	}
	objdep.Add(map.ptr());
	fNeedFreeze = true;
	if (0 == pts)
		return;
	Init();
	FillColumns();
}

TableHistogramSeg::TableHistogramSeg(SegmentMap& mp, TablePtr& p)
: TableVirtual(FileName(mp->fnObj, ".hss", true), p, true)
//               FileName(mp->fnObj, ".hs#"), Domain("none")) //mp->dm())
{
	if (mp->dm()->pdi() || mp->dm()->pdsrt() || mp->dm()->pdbit() || mp->dm()->pdp()) {
		ptr.SetDomain(mp->dm());
		if (iRecs() > 16000)
			throw ErrorObject(WhatError(String(TR("Too large domain").c_str(), mp->sName(true, fnObj.sPath())), errTableHistogramSeg), fnObj);
	}  
	else
		ptr.SetDomain("none");
	map = mp;
	objdep.Add(map.ptr());
	if (!fnObj.fValid())
		objtime = objdep.tmNewest();
	fNeedFreeze = true;
	//  Freeze();
}

void TableHistogramSeg::Store()
{
	TableVirtual::Store();
	WriteElement("TableVirtual", "Type", "TableHistogramSeg");
	WriteElement("TableHistogramSeg", "Map", map);
}

bool TableHistogramSeg::fFreezing()
{
	if (map->dm()->pdi() || map->dm()->pdsrt() || map->dm()->pdbit())
		ptr.SetDomain(map->dm());
	else
		ptr.SetDomain("none");

	Init();
	if (!fCount())
		return false;
	FillColumns();
	return true;
}

static bool fLessRealVal(long a, long b, void* p)
{
	TableHistogramSeg *his = static_cast<TableHistogramSeg*>(p);
	if (his->trq.fAborted())
		throw ErrorObject();
	return his->arSort[a] < his->arSort[b];
}

static void SwapRealVal(long a, long b, void* p)
{
	TableHistogramSeg *his = static_cast<TableHistogramSeg*>(p);
	double r = his->arSort[a];
	his->arSort[a] = his->arSort[b];
	his->arSort[b] = r;
}  

class HistItemSegLength {
public:
	HistItemSegLength()
	{ r = rUNDEF; iCount = 0; rLen = 0; }
	HistItemSegLength(double _r)
	{  r = _r; iCount = 0; rLen = 0; }
	bool operator ==(const HistItemSegLength& cc)
	{ return  cc.r == r; }
	double r;
	long iCount;
	double rLen;
};

static  int iHash(const HistItemSegLength& hir)
{
	word *pw = static_cast<word*>((void*)&hir.r);
	word w = *pw;
	++pw;
	for (int i = 1; i <= 3; ++i, ++pw)
		w ^= *pw;
	return w % 16000;
}

bool TableHistogramSeg::fCount()
{
	if (!map->fCalculated())
		map->Calc();
	long iMin = iOffset();
	long iMax = iMin + iRecs() - 1;
	for (long i = iMin; i <= iMax; i++) {
		_colFreq->PutVal(i, 0L);
		_colLength->PutVal(i, 0.0);
	}  
	trq.fText(TR("Determine frequencies"));
	if (0 == dm()->pdnone())
		for (int i=0 ; i < map->iFeatures(); ++i) {
			ILWIS::Segment *seg = (ILWIS::Segment *)map->getFeature(i);
			if ( seg == NULL || !seg->fValid())
				continue;
			if (trq.fAborted())
				return false;
			long iRaw = seg->iValue();
			if (iRaw == iUNDEF)
				continue;
			double rLen = _colLength->rValue(iRaw);
			if (rLen == rUNDEF)
				continue;
			_colLength->PutVal(iRaw, rLen+seg->rLength());
			long iFreq = _colFreq->iValue(iRaw);
			_colFreq->PutVal(iRaw, (long)(iFreq+1));
		}
	else { // use hashtable
		HashTable<HistItemSegLength> htsl(16000);
		trq.SetText(TR("Calculating histogram"));
		trq.HelpEnable(false);
		long iItem = 0;
		for (int i=0 ; i < map->iFeatures(); ++i) {
			ILWIS::Segment *seg = (ILWIS::Segment *)map->getFeature(i);
			if ( seg == NULL || !seg->fValid())
				continue;
			if (trq.fAborted())
				return false;
			double rVal = seg->rValue();
			if ( map->dvrs().fRawAvailable())
				rVal = map->dvrs().rValue(rVal);
			if (rVal == rUNDEF)
				continue;
			HistItemSegLength hisl(rVal);
			HistItemSegLength& hislHash = htsl.get(hisl);
			if (hislHash.iCount == 0) { // not found
				++iItem;
				hisl.iCount++;
				hisl.rLen += seg->rLength();
				htsl.add(hisl);
			}
			else {
				hislHash.iCount++;
				hislHash.rLen += seg->rLength();
			}
		}
		pts->iRecNew(iItem);
		//    _iRecs = iItem;

		// prepare for sorting
		arSort.Resize(iItem);
		trq.fText(TR("Prepare for sorting"));
		long n = 0;
		for (int k = 0; k < htsl.iTabSize; ++k) {
			if (!(n % 100))
				if (trq.fUpdate(n, iItem))
					return false;
			if (0 != htsl.table[k]) 
				for (SLIter<HistItemSegLength> iter(htsl.table[k]); iter.fValid(); ++iter)
					arSort[n++] = iter().r;
		}     
		trq.fUpdate(iItem, iItem);
		trq.fText(TR("Sort values"));
		try {
			if (iItem > 1)
				::QuickSort(0L, iItem-1, fLessRealVal, SwapRealVal, this);
		}  
		catch (const ErrorObject&) {
			arSort.Resize(0);
			return false;
		}  
		trq.fText(TR("Fill table"));
		for (long m=1; m <= iItem; ++m) {
			if (!(m % 100))
				if (trq.fUpdate(m, iItem)) {
					arSort.Resize(0);
					return false;
				}  
				HistItemSegLength hisl(arSort[m-1]);
				HistItemSegLength& hislHash = htsl.get(hisl);
				_colValue->PutVal(m, hislHash.r);
				_colFreq->PutVal(m, hislHash.iCount);
				_colLength->PutVal(m, hislHash.rLen);
		}
		trq.fUpdate(iItem, iItem);
		arSort.Resize(0);
	}
	return true;
}

void TableHistogramSeg::FillColumns()
{
	long i;
	trq.fText(TR("Fill other columns"));
	// reset undefined values
	bool fChanged = false;
	long iMin = iOffset();
	long iMax = iMin + iRecs() - 1;
	iTotalFreq = 0;
	rTotalLength = 0;
	for (i = iMin; i <= iMax; i++) {
		long l = _colFreq->iValue(i);
		if (l < 0)
			_colFreq->PutVal(i, 0L);
		else
			iTotalFreq += l;
		double r = _colLength->rValue(i);
		if (r < 0)
			_colLength->PutVal(i, 0.0);
		else
			rTotalLength += r;
	} 
	_colFreq->CalcMinMax();
	_colLength->CalcMinMax();
	if (colFreqCum().fValid()) {
		long iCum = 0;
		for (i = iMin; i <= iMax; i++) {
			iCum += _colFreq->iValue(i);
			colFreqCum()->PutVal(i, iCum);
			//      trq.fAborted();
		}
		colFreqCum()->CalcMinMax();
	}  
	if (colLengthCum().fValid()) {
		double rCum = 0;
		for (i = iMin; i <= iMax; i++) {
			rCum += _colLength->rValue(i);
			colLengthCum()->PutVal(i, rCum);
			//      trq.fAborted();
		}
		colLengthCum()->CalcMinMax();
	}  
	if (map->dvrs().fValues()) {
		map->SetMinMax(riMinMax(0));
		map->SetMinMax(rrMinMax(0));
		map->SetPerc1(riMinMax(1));
		map->SetPerc1(rrMinMax(1));
	}
	/*  trq.fText("Calc mean and standard deviation");
	rMean = rUNDEF;
	rStd = rUNDEF;
	rPred = rUNDEF;
	iPredCount = iUNDEF;
	rMedian = rrMinMax(50).rLo();
	long iPix;
	if (map->fValues() && iTotalPix > 1) {
	double r = 0;
	if (_colValue.fValid())
	for (long i = iOffset(); i < iOffset()+iRecs()-1; i++) {
	trq.fAborted();
	iPix = colNPix()->iValue(i);
	r += _colValue->rValue(i) * iPix;
	if (iPix > iPredCount) {
	iPredCount = iPix;
	rPred = _colValue->rValue(i);
	}
	}  
	else
	for (long i = iOffset(); i < iOffset()+iRecs()-1; i++) {
	trq.fAborted();
	iPix = colNPix()->iValue(i);
	r += iPix * map->dvrs().iValue(i);
	if (iPix > iPredCount) {
	iPredCount = iPix;
	rPred = map->dvrs().iValue(i);
	}
	}  
	rMean = r / iTotalPix;
	r = 0;
	if (_colValue.fValid())
	for (long i = iOffset(); i < iOffset()+iRecs()-1; i++) {
	r += sqr(rMean - _colValue->rValue(i)) * colNPix()->iValue(i);
	trq.fAborted();
	}  
	else  
	for (long i = iOffset(); i < iOffset()+iRecs()-1; i++) {
	r += sqr(rMean - map->dvrs().iValue(i)) * colNPix()->iValue(i);
	trq.fAborted();
	}  
	rStd = sqrt(r / (iTotalPix - 1));
	}
	else {
	for (long i = iOffset(); i < iOffset()+iRecs()-1; i++) {
	trq.fAborted();
	iPix = colNPix()->iValue(i);
	if (iPix > iPredCount) {
	iPredCount = iPix;
	rPred = i;
	}
	}  
	}*/
}

void TableHistogramSeg::Init()
{
	trq.SetTitle("TableHistogramSeg");
	trq.setHelpItem("ilwisapp\\histogram_algorithm.htm");
	if (pts==0)
		return;
	if (0 != dm()->pdnone()) {
		_colValue = pts->col("Value");
		if (!_colValue.ptr()) {
			_colValue = pts->colNew("Value", map->dvrs());
			_colValue->sDescription = TR("Segment value");
		}
		_colValue->SetOwnedByTable(true);
		_colValue->SetReadOnly(true);
	}
	_colFreq = pts->col("NrSeg");
	if (!_colFreq.ptr()) {
		_colFreq = pts->colNew("NrSeg", Domain("count"));
		_colFreq->sDescription = TR("Number of segments");
	}
	_colFreq->SetOwnedByTable(true);
	_colFreq->SetReadOnly(true);
	if (map->fValues()) {
		_colFreqCum = pts->col("NrSegCum");
		if (!_colFreqCum.ptr()) {
			_colFreqCum = pts->colNew("NrSegCum", Domain("count"));
			_colFreqCum->sDescription = TR("Cumulative number of segments");
		}
		_colFreqCum->SetOwnedByTable(true);
		_colFreqCum->SetReadOnly(true);
	}  
	_colLength = pts->col("Length");
	if (!_colLength.ptr()) {
		_colLength = pts->colNew("Length", Domain("value"), ValueRange(0,1e20,0.01));
		_colLength->sDescription = TR("Length of segments");
	}
	_colLength->SetReadOnly(true);
	_colLength->SetOwnedByTable(true);
	if (map->fValues()) {
		_colLengthCum = pts->col("LengthCum");
		if (!_colLengthCum.ptr()) {
			_colLengthCum = pts->colNew("LengthCum", Domain("value"), ValueRange(0,1e20,0.01));
			_colLengthCum->sDescription = TR("Cumulative length of segments");
		}
		_colLengthCum->SetReadOnly(true);
		_colLengthCum->SetOwnedByTable(true);
	}  
	//  pts->fChanged = false;
	//  pts->objtime = tim;
}


RangeInt TableHistogramSeg::riIndexMinMax(double rPerc) const
{
	long iLo = iOffset();
	long iHi = iRecs() + iOffset() - 1;
	RangeInt ri(iLo, iHi);
	if (rPerc == 0) {
		Column cl = colLength();
		for (long i = iLo; i <= iHi; ++i)
			if (cl->rValue(i) > 0) {
				ri.iLo() = i;
				break;
			}  
			for (long i = iHi; i >= iLo; --i)
				if (cl->rValue(i) > 0) {
					ri.iHi() = i;
					break;
				}
				return ri;
	}
	Column cp = colLengthCum();
	double rPrc = rPerc * rTotalLength / 100;
	for (long i = iLo+1; i <= iHi; ++i)
		if (cp->rValue(i) > rPrc) {
			ri.iLo() = i-1;
			break;
		}
		rPrc = (100 - rPerc) * rTotalLength / 100;
		for (long i = iHi-1; i >= iLo; --i)
			if (cp->rValue(i) < rPrc) {
				ri.iHi() = i+1;
				break;
			}
			/*
			try {
			long iPrevDif = cp->rValue(ri.iLo()-1) - rPrc;
			if (ri.iLo() > iOffset()) {
			if (iPrevDif != 0)
			if (abs(iPrevDif) < abs(cp->rValue(ri.iLo()) - rPrc))
			ri.iLo()--;
			}
			if (iPrevDif != 0)
			ri.iLo()++;
			rPrc = (100 - rPerc) * rTotalLength / 100;
			for (i = iRecs()+iOffset()-1; i >= iOffset(); i--)
			if (cp->rValue(i) <= rPrc) {
			ri.iHi() = i;
			break;
			}
			if (ri.iHi() < iOffset()+iRecs()-1) {
			long iPrevDif = cp->rValue(ri.iHi()+1) - rPrc;
			if (iPrevDif != 0)
			if (abs(iPrevDif) < abs(cp->rValue(ri.iHi()) - rPrc))
			ri.iHi()++;
			}
			}
			catch (const ErrorFloatingPoint&) {
			// for some strange reason floating point errors ocuur in this section
			}
			*/
			return ri;
}

RangeInt TableHistogramSeg::riMinMax(double rPerc) const
{
	if (!map->dvrs().fValues())
		return RangeInt();
	if (map->dvrs().fUseReals()) {
		RangeReal rr = rrMinMax(rPerc);
		return RangeInt(longConv(rr.rLo()), longConv(rr.rHi()));
	}  
	RangeInt ri = riIndexMinMax(rPerc);
	if (_colValue.fValid())
		return RangeInt(_colValue->iValue(ri.iLo()),
		_colValue->iValue(ri.iHi()));
	return RangeInt(map->dvrs().iValue(ri.iLo()),
		map->dvrs().iValue(ri.iHi()));
	//    return ri;
}

RangeReal TableHistogramSeg::rrMinMax(double rPerc) const
{
	if (!map->dvrs().fValues())
		return RangeReal();
	RangeInt ri = riIndexMinMax(rPerc);
	if (_colValue.fValid())
		return RangeReal(_colValue->rValue(ri.iLo()),
		_colValue->rValue(ri.iHi()));
	return RangeReal(map->dvrs().rValue(ri.iLo()),
		map->dvrs().rValue(ri.iHi()));
}

String TableHistogramSeg::sExpression() const
{
	return String("TableHistogramSeg(%S)", map->sNameQuoted(true, fnObj.sPath()));
}


String TableHistogramSeg::sType() const
{
	return "HistogramSeg";
}

void TableHistogramSeg::UnFreeze()
{
	if (_colValue.fValid()) {
		pts->RemoveCol(_colValue);
		_colValue = Column();
	}  
	if (_colFreq.fValid()) {
		pts->RemoveCol(_colFreq);
		_colFreq = Column();
	}  
	if (_colFreqCum.fValid()) {
		pts->RemoveCol(_colFreqCum);
		_colFreqCum = Column();
	}  
	if (_colLength.fValid()) {
		pts->RemoveCol(_colLength);
		_colLength = Column();
	}  
	if (_colLengthCum.fValid()) {
		pts->RemoveCol(_colLengthCum);
		_colLengthCum = Column();
	}  
	TableVirtual::UnFreeze();
}

void TableHistogramSeg::Erase(const FileName& fnMap)
{
	FileName fnHis = fnMap;
	fnHis.sExt = ".hs#"; 
	_unlink(fnHis.sFullName(true).c_str()); // delete data file
	fnHis.sExt = ".hss";
	// delete object def file
	if (0 == _unlink(fnHis.sFullName(true).c_str()))
		AfxGetApp()->GetMainWnd()->PostMessage(ILW_READCATALOG); 
}




