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
/* GeoRefSubmap editor
   by Willem Nieuwenhuis, 17 August 1999
   Copyright Ilwis System Development ITC
*/

#include "Client\Headers\formelementspch.h"
#include "Client\ilwis.h"
#include "Client\Base\IlwisDocument.h"
#include "Client\Editors\Georef\GeoRefDoc.h"
#include "Client\Editors\Georef\GeoRefGenerDoc.h"
#include "Client\FormElements\fldgrf.h"
#include "Engine\SpatialReference\Gr.h"
#include "Engine\SpatialReference\Grsub.h"
#include "Headers\Htp\Ilwismen.htp"
#include "Client\Forms\generalformview.h"
#include "Client\Editors\Georef\GrGenerEdit.h"
#include "Client\Editors\Georef\GrSubmapEdit.h"
#include "Headers\Hs\Georef.hs"
#include "Headers\Hs\Appforms.hs"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// GeoRefSubmapView

IMPLEMENT_DYNCREATE(GeoRefSubmapView, GeoRefGeneratedView)

BEGIN_MESSAGE_MAP(GeoRefSubmapView, GeoRefGeneratedView)
END_MESSAGE_MAP()

GeoRefSubmapView::GeoRefSubmapView()
{

}

GeoRefSubmapView::~GeoRefSubmapView()
{
}

bool GeoRefSubmapView::fRemarkNeeded()
{
	return false;
}

void GeoRefSubmapView::FillDerivedFields(FieldGroup* fgGenerRoot)
{
	GeoRefSubMap* pgs = GetDocument()->gr()->pgSub();
	ISTRUE(fINotEqual, pgs, (GeoRefSubMap*)0);

	m_rcOffset = pgs->rcTopLeft();
	// Use user interface version of the offset (i.e starting from one instead of from zero)
	m_rcOffset.Row++;
	m_rcOffset.Col++;

	String sGRSize(SGRRemLinesCols_ii.c_str(), m_rcSize.Row, m_rcSize.Col);
	SetSizeString(sGRSize);
	String sPixSize(SGRRemPixSize_f.c_str(), m_rParentPixSize);
	SetPixelSizeString(sPixSize);

	// Disabled editable Offset and Size fields for now; show R/O only
	if (1) // GetDocument()->gr()->fReadOnly())
	{
		sGRSize = String(SGRInfStartRowCol_ii.c_str(), m_rcOffset.Row, m_rcOffset.Col);
		StaticText* stSize2 = new StaticText(fgGenerRoot, sGRSize);
		stSize2->SetIndependentPos();
		stSize2->psn->SetBound(0, 0, 0, 0);
	}
	else 
	{
		FieldGroup* fgOffset = new FieldGroup(fgGenerRoot);
		fiRow = new FieldInt(fgOffset, SAFUiFirstLine, &m_rcOffset.Row);
		fiCol = new FieldInt(fgOffset, SAFUiFirstCol, &m_rcOffset.Col);
		fiRow->SetCallBack((NotifyProc)&GeoRefSubmapView::CheckWithSize);
		fiCol->SetCallBack((NotifyProc)&GeoRefSubmapView::CheckWithSize);

		FieldGroup* fgGrSize = new FieldGroup(fgGenerRoot);
		fgGrSize->Align(fgOffset, AL_UNDER);
		fiWidth = new FieldInt(fgGrSize, SAFUiNrLines, &m_rcSize.Row);
		fiHeight = new FieldInt(fgGrSize, SAFUiNrCols, &m_rcSize.Col);
		fiWidth->SetCallBack((NotifyProc)&GeoRefSubmapView::CheckWithSize);
		fiHeight->SetCallBack((NotifyProc)&GeoRefSubmapView::CheckWithSize);
	}

	SetMenHelpTopic(htpGrfSubMap);
}

int GeoRefSubmapView::exec()
{
	try
	{
		GeoRefGeneratedView::exec();

		GeoRefSubMap* pgs = GetDocument()->gr()->pgSub();
		ISTRUE(fINotEqual, pgs, (GeoRefSubMap*)0);

		// Adjust the offset to internal format (starting from zero instead of one)
		m_rcOffset.Row--;
		m_rcOffset.Col--;
		pgs->SetTopLeft(m_rcOffset);
		pgs->SetRowCol(m_rcSize);

		pgs->Updated();

		return 0;
	}
	catch (ErrorObject& err)
	{
		err.Show();
	}  
	return 1;
}

int GeoRefSubmapView::CheckWithSize(Event*)
{
	root->StoreData();

	String sGRSize(SGRRemLinesCols_ii.c_str(), m_rcSize.Row, m_rcSize.Col);
	SetSizeString(sGRSize);
	String sPixSize(SGRRemPixSize_f.c_str(), m_rParentPixSize);
	stPixelSize->SetVal(sPixSize);

	return 0;
}
