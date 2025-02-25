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
// MapListMatrixMultiply.h: interface for the MapListMatrixMultiply class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAPLISTMATRIXMULTIPLY_H__31C96260_A433_48B6_8A3E_0E96A9B8E1A3__INCLUDED_)
#define AFX_MAPLISTMATRIXMULTIPLY_H__31C96260_A433_48B6_8A3E_0E96A9B8E1A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if !defined(AFX_MAPLISTVIRTUAL_H__6C4EDC42_939D_4D80_B9BE_4968C8358D22__INCLUDED_)
#include "Engine\Applications\MapListVirtual.h"
#endif

#include "Engine\Base\DataObjects\WPSMetaData.h"

#ifndef ILWMATRIXOBJ_H
#include "Engine\Matrix\Matrxobj.h"
#endif

IlwisObjectPtr * createMapListMatrixMultiply(const FileName& fn, IlwisObjectPtr& ptr, const String& sExpr, vector<void *> parms );
ApplicationMetadata metadataMapListMatrixMultiply(ApplicationQueryData *query);

class MapListMatrixMultiply : public MapListVirtual
{
public:
	MapListMatrixMultiply(const FileName& fn, MapListPtr& ptr);
	MapListMatrixMultiply(const FileName& fn, MapListPtr& ptr, const String& sExpression);
	virtual ~MapListMatrixMultiply();
  virtual void Store();
  virtual bool fFreezing();
  virtual String sExpression() const;
private:
  void Init();
  MapList ml;
  MatrixObject mat;
  int iOutBands;
};

#endif // !defined(AFX_MAPLISTMATRIXMULTIPLY_H__31C96260_A433_48B6_8A3E_0E96A9B8E1A3__INCLUDED_)
