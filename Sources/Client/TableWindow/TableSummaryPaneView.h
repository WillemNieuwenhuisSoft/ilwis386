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
// TableSummaryPaneView.h: interface for the TableSummaryPaneView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TABLESUMMARYPANEVIEW_H__73D1ED23_A7D0_11D3_B827_00A0C9D5342F__INCLUDED_)
#define AFX_TABLESUMMARYPANEVIEW_H__73D1ED23_A7D0_11D3_B827_00A0C9D5342F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class SelectionStatistics
{
public:
	double avg, sum, stdev;
	RangeReal rr;
	Column col;
};

class TableSummaryPaneView: public BaseTablePaneView  
{
public:
	TableSummaryPaneView();
	virtual ~TableSummaryPaneView();
	TableDoc* GetDocument();
	const TableDoc* GetDocument() const;
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void InitColPix(CDC* cdc);
  virtual long iCols() const;
  virtual long iRows() const;
  virtual String sRowButton(long iRow) const;
  virtual String sDescrRowButton(long iRow) const;
  virtual String sField(int iCol, long iRow) const;
  virtual String sDescrField(int iCol, long iRow) const;
  void selectFeatures(const RowSelectInfo& inf);
protected:
	vector<long> selectedInMainTable;
	//{{AFX_MSG(TablePaneView)
	afx_msg void OnUpdateProp(CCmdUI* pCmdUI);
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
private:
	SelectionStatistics * selStatistics;

	DECLARE_DYNCREATE(TableSummaryPaneView)
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_TABLESUMMARYPANEVIEW_H__73D1ED23_A7D0_11D3_B827_00A0C9D5342F__INCLUDED_)
