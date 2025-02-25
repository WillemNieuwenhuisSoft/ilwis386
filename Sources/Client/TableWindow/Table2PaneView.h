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
#if !defined(AFX_TABLE2PANEVIEW_H__7DD2579E_1E42_11D3_B776_00A0C9D5342F__INCLUDED_)
#define AFX_TABLE2PANEVIEW_H__7DD2579E_1E42_11D3_B776_00A0C9D5342F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TablePaneView.h : header file
//

#undef IMPEXP
#ifdef ILWISCLIENT
#define IMPEXP __declspec(dllexport)
#else
#define IMPEXP __declspec(dllimport)
#endif

class TableWindow;

/////////////////////////////////////////////////////////////////////////////
// TablePaneView view

class IMPEXP Table2PaneView : public BaseTablePaneView
{
	friend class Table2Window; // for access to recBar
protected:
	Table2PaneView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(Table2PaneView)
public:
	Table2Doc* GetDocument();
	const Table2Doc* GetDocument() const;
//	TableWindow* twParent();
  Table2Window* twParent();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TablePaneView)
	public:
	virtual void InitColPix(CDC* cdc);
  virtual long iCols() const;
  virtual long iRows() const;
  virtual String sColButton(int iCol) const;
  virtual String sRowButton(long iRow) const;
	virtual void OnInitialUpdate();
	virtual String sPrintTitle() const;
	protected:
  virtual void OnFieldPressed(int iCol, long iRow, bool fLeft=true);
  virtual String sField(int iCol, long iRow) const;
  virtual String sDescrULButton() const; // upper left button
  virtual String sDescrColButton(int iCol) const;
  virtual String sDescrRowButton(long iRow) const;
  virtual String sDescrField(int iCol, long iRow) const;
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	afx_msg void OnUpdateEdit(CCmdUI* pCmdUI);
	afx_msg void OnEdit();
	afx_msg void OnEditClear();
	afx_msg void OnEditPaste();
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~Table2PaneView();
	virtual bool fAllowPaste() const;
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(TablePaneView)
	afx_msg void OnContextMenu(CWnd*,CPoint);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABLE2PANEVIEW_H__7DD2579E_1E42_11D3_B776_00A0C9D5342F__INCLUDED_)
