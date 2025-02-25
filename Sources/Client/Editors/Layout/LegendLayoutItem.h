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
// LegendLayoutItem.h: interface for the LegendLayoutItem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LEGENDLAYOUTITEM_H__0FE1ECD5_FF01_11D3_B87D_00A0C9D5342F__INCLUDED_)
#define AFX_LEGENDLAYOUTITEM_H__0FE1ECD5_FF01_11D3_B87D_00A0C9D5342F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

	
class Drawer;					
class BaseMapDrawer;			
class LegendBaseDrawer;
class MapLayoutItem;
class MapCompositionDoc;

class LegendLayoutItem: public TextLayoutItem  
{
public:
	LegendLayoutItem(LayoutDoc* ld, MapLayoutItem*);
	LegendLayoutItem(LayoutDoc* ld);
	virtual ~LegendLayoutItem();
	virtual void ReadElements(ElementContainer&, const char* sSection);
	virtual void WriteElements(ElementContainer&, const char* sSection);
	virtual void OnDraw(CDC* cdc);
	virtual bool fConfigure();
	virtual String sType() const;
	virtual String sName() const;
	virtual void SetPosition(MinMax mm, int iHit=-1);
	virtual bool fDependsOn(LayoutItem*);
  BaseMapDrawer* bmd() const;
	MapCompositionDoc *mcd() const;
  static bool fLegend(Drawer*);
protected:
	void Setup();
	MapLayoutItem* mli;
private:
	LegendBaseDrawer* lbd;	
  BaseMapDrawer* m_bmd;
	DECLARE_MESSAGE_MAP()
	friend class LegendBaseDrawer;
};

#endif // !defined(AFX_LEGENDLAYOUTITEM_H__0FE1ECD5_FF01_11D3_B87D_00A0C9D5342F__INCLUDED_)
