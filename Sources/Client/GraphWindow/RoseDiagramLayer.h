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
// RoseDiagramLayer.h: interface for the RoseDiagramLayer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ROSEDIAGRAMLAYER_H)
#define AFX_ROSEDIAGRAMLAYER_H


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#undef IMPEXP
#ifdef ILWISCLIENT
#define IMPEXP __declspec(dllexport)
#else
#define IMPEXP __declspec(dllimport)
#endif

class RoseDiagramDrawer;
class GraphLayerOptionsForm;

#include "Client\GraphWindow\GraphLayer.h"

enum RoseDiagramType {rdtContineous=0, rdtStep, rdtNeedle, rdtUnknownType };

class IMPEXP RoseDiagramLayer : public GraphLayer
{
public:
	RoseDiagramLayer(RoseDiagramDrawer* rd, const Table& t, const Column& cX, const Column& cY);
  RoseDiagramLayer(RoseDiagramDrawer*, const FileName& fnObj, const String& sSection);
	virtual ~RoseDiagramLayer();
	virtual String sName();
	virtual void draw(CDC*);
	virtual bool fConfig();
	virtual int iNrPoints();
	virtual double rX(int iRec);
	virtual double rY(int iRec);
	virtual GraphLayerOptionsForm* frmOptions();
	RoseDiagramDrawer* rdd;
  virtual void SaveSettings(const FileName& fn, const String& sSection);
	CPoint ptPos(double x, double y);
  RoseDiagramType rdt;
	Table tbl;
	Column colX, colY;
};

#endif 
