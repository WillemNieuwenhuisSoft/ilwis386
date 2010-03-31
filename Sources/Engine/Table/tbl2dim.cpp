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
/* Table2Dim, Table2DimPtr
   Copyright Ilwis System Development ITC
   dec. 1996, Jelle Wind
	Last change:  WK    9 Sep 97    3:41 pm
*/

#include "Engine\Table\tbl2dim.h"
#include "Engine\Domain\dmsort.h"
#include "Engine\Domain\Dmvalue.h"
#include "Engine\Domain\dminfo.h"
#include "Engine\Base\DataObjects\ObjectStructure.h"
#include "Engine\Table\COLSTORE.H"
#include "Engine\Base\DataObjects\ERR.H"
#include "Engine\Base\System\mutex.h"
#include "Headers\Err\Ilwisapp.err"
#include "Headers\Hs\Table.hs"

Table2Dim::Table2Dim()
: Table()
{}

Table2Dim::Table2Dim(const Table2Dim& tb2)
: Table(tb2)
{}

Table2Dim::Table2Dim(const FileName& fn)
: Table()
{
    FileName fil = FileName(fn, ".ta2");
//    CMutex mut(TRUE, fn.sFullName().scVal());
    IlwisObjectPtr *iptr=pGet(fil);
    if (iptr==NULL)
    {
      MutexFileName mut(fil);
      IlwisObjectPtr *iptr=pGet(fil);
      if (iptr != NULL) {
        SetPointer(iptr);
        return;
      }
      SetPointer(Table2DimPtr::create(fil));
      ptr()->Load();
    }
    else
        SetPointer(iptr);
        
}

Table2Dim::Table2Dim(const FileName& fn, const DomainValueRangeStruct& dvrs1,
            const DomainValueRangeStruct& dvrs2,
            const DomainValueRangeStruct& dvs)
: Table() //FileName(fn, ".ta2", true), Domain("none"))
{
  SetPointer(new Table2DimPtr(fn, dvrs1, dvrs2, dvs));
}

Table2DimPtr* Table2DimPtr::create(const FileName& fn)
{
  return new Table2DimPtr(fn);
}

String Table2DimPtr::sType() const
{
  return "2-Dimensional Table";
}

Table2DimPtr::Table2DimPtr(const FileName& fn)
: TablePtr(fn, String())
{
  Load();
  colVal = col("Value");
  ReadElement("Table2Dim", "Domain1", dom1);
  iSortSize1 = iReadElement("Table2Dim", "DomainSize1");
  ReadElement("Table2Dim", "Domain2", dom2);
  iSortSize2 = iReadElement("Table2Dim", "DomainSize2");
  pdsrt1 = dom1->pdsrt();
  pdsrt2 = dom2->pdsrt();
  AdjustForDomainChange();
}

Table2DimPtr::Table2DimPtr(const FileName& fn, const DomainValueRangeStruct& dvrs1,
               const DomainValueRangeStruct& dvrs2,  const DomainValueRangeStruct& dvs)
: TablePtr(FileName(fn, ".ta2"), FileName(fn, ".t2#"), Domain("none"), String())
{
  dom1 = dvrs1.dm();
  dom2 = dvrs2.dm();
  pdsrt1 = dom1->pdsrt();
  pdsrt2 = dom2->pdsrt();
	if (!pdsrt1)
		if (dom1.fValid())
			throw ErrorObject(String(STB2dErrDomainSortRequired_S.scVal(), dom1->sName()));
		else
			throw ErrorObject(STB2dErrDomainSortRequired);
	if (!pdsrt2)
		if (dom2.fValid())
			throw ErrorObject(String(STB2dErrDomainSortRequired_S.scVal(), dom2->sName()));
		else
			throw ErrorObject(STB2dErrDomainSortRequired);
  assert((0 != pdsrt1) && (0 != pdsrt2)); // proper error message needed
  {
    Table tbl;
    tbl.SetPointer(this);
    iRef++;
    colVal = Column(tbl, "Value", dvs);
    AddCol(colVal);
    colVal->SetOwnedByTable(true);
  }  
  iRef--;
  if (0 != pdsrt1)
    iSortSize1 = pdsrt1->iSize();
  else
    iSortSize2 = 0;
  if (0 != pdsrt1)
    iSortSize2 = pdsrt2->iSize();
  else 
    iSortSize2 = 0;
  iRecNew(iSortSize1 * iSortSize2);
  for (long i=1; i <= iRecs(); ++i)
    colVal->PutRaw(i, iUNDEF);
}

Table2DimPtr::~Table2DimPtr()
{
}

void Table2DimPtr::Store()
{
  TablePtr::Store();
//  StoreAsBinary(sDataFile);
  WriteElement("Table", "Type", "Table2Dim");
  WriteElement("Table2Dim", "Domain1", dom1);
  DomainInfo(dom1).Store(fnObj, "Table2Dim", "DomainInfo1");
  WriteElement("Table2Dim", "DomainSize1", iSortSize1);
  WriteElement("Table2Dim", "Domain2", dom2);
  WriteElement("Table2Dim", "DomainSize2", iSortSize2);
  DomainInfo(dom2).Store(fnObj, "Table2Dim", "DomainInfo2");
}
/*
bool Table2DimPtr::fChangeReadOnlyAllowed(const Column& col)
{
  if (colVal.fValid())
    if (colVal->sName() == col->sName())
      return false;
  return true;
}*/

long Table2DimPtr::iRaw(long iKey1, long iKey2) const
{
  return colVal->iRaw(iRec(iKey1, iKey2));
}

long Table2DimPtr::iValue(long iKey1, long iKey2) const
{
  return colVal->iValue(iRec(iKey1, iKey2));
}

double Table2DimPtr::rValue(long iKey1, long iKey2) const
{
  return colVal->rValue(iRec(iKey1, iKey2));
}

String Table2DimPtr::sValue(long iKey1, long iKey2, short iWid, short iDec) const
{
  return colVal->sValue(iRec(iKey1, iKey2), iWid, iDec);
}

Coord Table2DimPtr::cValue(long iKey1, long iKey2) const
{
  return colVal->cValue(iRec(iKey1, iKey2));
}

long Table2DimPtr::iRaw(const String& sKey1, const String& sKey2) const
{
  return colVal->iRaw(iRec(sKey1.iVal(), sKey2.iVal()));
}

long Table2DimPtr::iValue(const String& sKey1, const String& sKey2) const
{
  return colVal->iValue(iRec(sKey1.iVal(), sKey2.iVal()));
}

double Table2DimPtr::rValue(const String& sKey1, const String& sKey2) const
{
  return colVal->rValue(iRec(sKey1.iVal(), sKey2.iVal()));
}

String Table2DimPtr::sValue(const String& sKey1, const String& sKey2, short iWid, short iDec) const
{
  return colVal->sValue(iRec(sKey1.iVal(), sKey2.iVal()), iWid, iDec);
}

Coord Table2DimPtr::cValue(const String& sKey1, const String& sKey2) const
{
  return colVal->cValue(iRec(sKey1.iVal(), sKey2.iVal()));
}

long Table2DimPtr::iRec(const String& sKey1, const String& sKey2) const
{
  if ((0 == pdsrt1) || (0 == pdsrt2))
    return iUNDEF;
  return iRec(pdsrt1->iKey(pdsrt1->iOrd(sKey1)), pdsrt2->iKey(pdsrt2->iOrd(sKey1)));
}

long Table2DimPtr::iRec(long iKey1, long iKey2) const
{
  if ((0 == pdsrt1) || (0 == pdsrt2))
    return iUNDEF;
  if (iKey1 == iUNDEF || iKey2 == iUNDEF)
    return iUNDEF;
  if ((iSortSize1 < pdsrt1->iSize()) || (iSortSize2 < pdsrt2->iSize()))
    (const_cast<Table2DimPtr *> (this))->AdjustForDomainChange();
  return (iKey1-1) * iSortSize2 + iKey2;
}

long Table2DimPtr::iKey1(long iRec) const
{
  return 1 + (iRec-1) / iSortSize2;  
}

long Table2DimPtr::iKey2(long iRec) const
{
  return 1 + (iRec-1) % iSortSize2;  
}

String Table2DimPtr::sKey1(long iRec) const
{
  return dom1->sValueByRaw(iKey1(iRec));
}

String Table2DimPtr::sKey2(long iRec) const
{
  return dom2->sValueByRaw(iKey2(iRec));
}

void Table2DimPtr::PutRaw(long iRaw1, long iRaw2, long iRaw)
{
  colVal->PutRaw(iRec(iRaw1, iRaw2), iRaw);
  fChanged = true;
}

void Table2DimPtr::PutVal(long iRaw1, long iRaw2, long iValue)
{
  colVal->PutVal(iRec(iRaw1, iRaw2), iValue);
  fChanged = true;
}

void Table2DimPtr::PutVal(long iRaw1, long iRaw2, double rValue)
{
  colVal->PutVal(iRec(iRaw1, iRaw2), rValue);
  fChanged = true;
}

void Table2DimPtr::PutVal(long iRaw1, long iRaw2, const String& sValue)
{
  colVal->PutVal(iRec(iRaw1, iRaw2), sValue);
  colVal->Updated();
}

void Table2DimPtr::PutVal(long iRaw1, long iRaw2, const Coord& cValue)
{
  colVal->PutVal(iRec(iRaw1, iRaw2), cValue);
  fChanged = true;
}

void Table2DimPtr::PutRaw(const String& sKey1, const String& sKey2, long iRaw)
{
  colVal->PutRaw(iRec(sKey1, sKey2), iRaw);
  fChanged = true;
}

void Table2DimPtr::PutVal(const String& sKey1, const String& sKey2, long iValue)
{
  colVal->PutVal(iRec(sKey1, sKey2), iValue);
  fChanged = true;
}

void Table2DimPtr::PutVal(const String& sKey1, const String& sKey2, double rValue)
{
  colVal->PutVal(iRec(sKey1, sKey2), rValue);
  fChanged = true;
}

void Table2DimPtr::PutVal(const String& sKey1, const String& sKey2, const String& sValue)
{
  colVal->PutVal(iRec(sKey1, sKey2), sValue);
  fChanged = true;
}

void Table2DimPtr::PutVal(const String& sKey1, const String& sKey2, const Coord& cValue)
{
  colVal->PutVal(iRec(sKey1, sKey2), cValue);
  fChanged = true;
}

DomainValueRangeStruct Table2DimPtr::dvrsVal() const
{
  return colVal->dvrs();
}

void Table2DimPtr::AdjustForDomainChange()
{
  if ((0==pdsrt1) || (0==pdsrt2))
    return;
  if (iSortSize1 < pdsrt1->iSize()) {
    iRecNew(pdsrt1->iSize() * iSortSize2 - iRecs());
    for (long i = iSortSize1 * iSortSize2 + 1;
         i <= pdsrt1->iSize() * iSortSize2; ++i)
      colVal->PutRaw(i, iUNDEF);
  }
  iSortSize1 = pdsrt1->iSize();
  if (iSortSize2 < pdsrt2->iSize()) {
    iRecNew(iSortSize1 * pdsrt2->iSize() - iRecs());
    long iShift = pdsrt2->iSize()-iSortSize2;
    if (dvrsVal().fRealValues()) {
      for (long i = iSortSize1; i >= 1; --i) {
	    long j=0;
        for (; j < iShift; ++j)
          colVal->PutVal(i*pdsrt2->iSize()-j, rUNDEF);
        for (; j < pdsrt2->iSize(); ++j)
          colVal->PutVal(i*pdsrt2->iSize()-j,
                colVal->rValue(i*iSortSize2-j+iShift));
      }
    }
    else {
      for (long i = iSortSize1; i >= 1; --i) {
        long j=0;
        for (; j < iShift; ++j)
          colVal->PutRaw(i*pdsrt2->iSize()-j, iUNDEF);
        for (; j < pdsrt2->iSize(); ++j)
          colVal->PutRaw(i*pdsrt2->iSize()-j,
                colVal->iRaw(i*iSortSize2-j+iShift));
      }
    }
  }
  iSortSize2 = pdsrt2->iSize();
}

void Table2DimPtr::GetObjectStructure(ObjectStructure& os)
{
	TablePtr::GetObjectStructure(os);
	if ( os.fGetAssociatedFiles())
	{
		os.AddFile(fnObj, "Table2Dim", "Domain1");
		os.AddFile(fnObj, "Table2Dim", "Domain2");	
	}		
}	



