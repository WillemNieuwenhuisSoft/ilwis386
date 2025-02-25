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
/*/ Revision 1.5  1998/09/16 17:35:47  Wim
// 22beta2
//
// Revision 1.4  1997/09/27 12:22:55  Wim
// Disable samplesetstatistics window when processing a command
//
// Revision 1.3  1997-08-18 19:51:33+02  Wim
// Implemented Clear() which updates sample set
//
/* SampleSetEditor
   by Wim Koolhoven, sep. 1994
   (c) Ilwis System Development ITC
	Last change:  WK   11 Aug 98    6:04 pm
*/

#include "Client\Headers\formelementspch.h"
#include "Client\Mapwindow\MapPaneView.h"
#include "Engine\Map\Segment\Seg.h"
#include "Engine\Map\Polygon\POL.H"
#include "Client\Mapwindow\Positioner.h"
#include "Client\Editors\Editor.h"
#include "Client\Base\ButtonBar.h"
#include "Client\ilwis.h"
#include "Headers\Hs\Editor.hs"
#include "Client\FormElements\syscolor.h"
#include "Client\Mapwindow\AreaSelector.h"
#include "Client\Mapwindow\IlwisClipboardFormat.h"
#include "Client\Base\IlwisDocument.h"
#include "Client\MainWindow\Catalog\CatalogDocument.h"
#include "Engine\Map\Mapview.h"
#include "Client\Mapwindow\MapCompositionDoc.h"
#include "Headers\constant.h"
#include "Engine\Domain\dmsort.h"
#include "Headers\Htp\Ilwis.htp"
#include "Client\Mapwindow\MapWindow.h"
#include "Client\FormElements\fldclass.h"
#include "Headers\Hs\Sample.hs"
#include "Client\Editors\SampleSet\SampleSetEditor.h"
#include "Client\Editors\SampleSet\SampleStatWindow.h"
#include "Client\Editors\SampleSet\FSWindow.h"
#include "Client\Editors\Utils\GeneralBar.h"
#include "Client\GraphWindow\GraphView.h"
//#include "dsp/graphfsp.h"
//#include "dsp/secewind.h"

BEGIN_MESSAGE_MAP(SampleSetEditor, PixelEditor)
  //{{AFX_MSG_MAP(SampleSetEditor)
  ON_COMMAND(ID_DELCLASS, OnDelClass)
  ON_COMMAND(ID_MERGECLASS, OnMergeClass)
  ON_COMMAND(ID_DOMRPR, OnShowRpr)
  ON_COMMAND(ID_FEATURESPACE, OnFeatureSpace)
  ON_COMMAND(ID_SMPLSTATPANE, OnStatisticsPane)
  ON_UPDATE_COMMAND_UI(ID_SMPLSTATPANE, OnUpdateStatisticsPane)
  ON_UPDATE_COMMAND_UI(ID_EDIT, OnUpdateEdit)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define sMen(ID) ILWSF("men",ID).c_str()
#define addmen(ID) men.AppendMenu(MF_STRING, ID, sMen(ID)); 

SampleSetEditor::SampleSetEditor(MapPaneView* pane, const SampleSet& smss)
: PixelEditor(pane, smss->map(), 8000), sms(smss)

{
	Map mp = smss->map();
	help = "ilwis\\sample_set_editor_functionality1.htm";
	rFactVisibleLimit = 0.1;
	String winTitle = String(TR("Sample Set Editor: %S"));
	String s(winTitle.c_str(), sms->sName());
	pane->mwParent()->SetWindowText(s.c_str());
	fOk = sms->fInitStat();
	if (!fOk)
		return;
	sms->SetSlct(rcSelect);
	wSmplStat = new SampleStatWindow(sms);
	wSmplStat->Create(pane->mwParent());
	pane->mwParent()->EnableDocking(CBRS_ALIGN_ANY);
	// position the statistics window
	CRect rectWin;
	pane->mwParent()->GetWindowRect(&rectWin); // this is not yet it's final size
	CPoint pnt(rectWin.left, rectWin.top); // so only can use top left corner
	pnt += CPoint(100, 100);
	pane->mwParent()->FloatControlBar(wSmplStat, pnt);
	CMenu men;
	men.CreateMenu();

	addmen(ID_OPENPIXELINFO);
	addmen(ID_EXITEDITOR);
	hmenFile = men.GetSafeHmenu();
	men.Detach();

	men.CreateMenu();

	addmen(ID_SELECTAREA);
	addmen(ID_EDIT);
	bool fEdit = rcSelect.iSize() != 0;
	men.EnableMenuItem(ID_EDIT, fEdit);
	men.AppendMenu(MF_SEPARATOR);
	addmen(ID_FEATURESPACE);
	addmen(ID_DELCLASS);
	addmen(ID_MERGECLASS);
	men.AppendMenu(MF_SEPARATOR);
	//  addmen(ID_OPENCLOSELEGEND);
	//  bool fLegendAct = 0 != legWnd;
	//  men.EnableMenuItem(ID_OPENCLOSELEGEND, fLegendAct ? MF_ENABLED : MF_GRAYED);
	addmen(ID_DOMRPR);
	men.AppendMenu(MF_SEPARATOR);
	addmen(ID_EXITEDITOR);

	hmenEdit = men.GetSafeHmenu();
	men.Detach();

	UpdateMenu();
	DataWindow* dw = mpv->dwParent();
	if (dw) {
		dw->bbDataWindow.LoadButtons("smpledit.but");
		dw->RecalcLayout();
		CMenu* men = dw->GetMenu();
		int iNr = men->GetMenuItemCount();
		CMenu* menOptions = men->GetSubMenu(iNr-2);
		menOptions->AppendMenu(MF_STRING, ID_SMPLSTATPANE, sMen(ID_SMPLSTATPANE)); 
	}
	int iFSWin;
	ObjectInfo::ReadElement("SampleSet", "FSWindows", sms->fnObj, iFSWin);
	for (int i=0; i < iFSWin; i++) {
		FeatureSpaceWindow* fsw = new FeatureSpaceWindow();
		fsw->LoadSettings(sms->fnObj, i);
		String sSection("FSWindow%i", i);
		int iID = 0;
		ObjectInfo::ReadElement(sSection.c_str(), "ID", sms->fnObj, iID);
		if (iID <= 0)
			iID = dw->iNewBarID();
		MakeFSWindow(fsw, CPoint(100+50*i,100+50*i), iID);
	}
	drawSelect();
}

void SampleSetEditor::OnUpdateEdit(CCmdUI* pCmdUI)
{
	bool fEdit=true;
	pCmdUI->Enable(fEdit);
}

SampleSetEditor::~SampleSetEditor()
{
	delete wSmplStat;
	vector<GeneralBar*>::iterator iter;
	for (iter = vgb.begin(); iter != vgb.end(); ++iter) {
		GeneralBar* gb = *iter;
		if (IsWindow(gb->m_hWnd)) {
			if (IsWindow(gb->view->m_hWnd)) 
				delete gb->view;
			delete gb;
		}
	}
	CFrameWnd* fw = mpv->GetTopLevelFrame();
	if (fw) {
		CMenu* men = fw->GetMenu();
		men->RemoveMenu(ID_SMPLSTATPANE, MF_BYCOMMAND);
	}
}

bool SampleSetEditor::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CMenu men;
	men.CreatePopupMenu();
	addmen(ID_NORMAL);
	addmen(ID_ZOOMIN);
	addmen(ID_ZOOMOUT);
	addmen(ID_PANAREA);
	men.AppendMenu(MF_SEPARATOR);
	addmen(ID_EDIT);
	bool fEdit = rcSelect.iSize() != 0;
	men.EnableMenuItem(ID_EDIT, fEdit ? MF_ENABLED : MF_GRAYED);
	addmen(ID_FEATURESPACE);
	addmen(ID_DELCLASS);
	addmen(ID_MERGECLASS);
	men.AppendMenu(MF_SEPARATOR);
	addmen(ID_DOMRPR);
	men.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, pWnd);
	return true;
}

class AskFeatureSpaceForm: public FormWithDest
{
public:
	AskFeatureSpaceForm(CWnd* wPar, const MapList& ml, int* b1, int* b2)
		: FormWithDest(wPar, TR("Feature Space"))
	{
		new FieldMapFromMapList(root, TR("Band &1"), ml, b1);
		new FieldMapFromMapList(root, TR("Band &2"), ml, b2);
		SetHelpItem("ilwismen\\sample_set_editor_feature_space.htm");
		create();
	}
};

class AskMergeClassForm: public FormWithDest
{
public:
	AskMergeClassForm(CWnd* wPar, const SampleSet& sms, long* iClass1, long* iClass2)
		: FormWithDest(wPar, TR("Merge Classes"))
	{
		new FieldClass(root, TR("&Merge Class"), iClass2, sms->dm()->pdsrt());
		new FieldClass(root, TR("&Into Class"), iClass1, sms->dm()->pdsrt());
		SetHelpItem("ilwismen\\sample_set_editor_merge_classes.htm");
		create();
	}
};

class AskDeleteClassForm: public FormWithDest
{
public:
	AskDeleteClassForm(CWnd* wPar, const SampleSet& sms, long* iClass)
		: FormWithDest(wPar, TR("Clear Class"))
	{
		new FieldClass(root, TR("&Class"), iClass, sms->dm()->pdsrt());
		SetHelpItem("ilwismen\\sample_set_editor_delete_class.htm");
		create();
	}
};

void SampleSetEditor::OnDelClass()
{
	long iClass=1;
	AskDeleteClassForm frm(mpv, sms, &iClass);
	if (frm.fOkClicked()) {
		wSmplStat->EnableWindow(false);
		sms->DelClass(byteConv(iClass));
		mpv->GetDocument()->UpdateAllViews(0,6);
		//      mpv->Invalidate();
		wSmplStat->EnableWindow(true);
	}  
}

void SampleSetEditor::OnMergeClass()
{
	long iClass1=1, iClass2=2;
	AskMergeClassForm frm(mpv, sms, &iClass1, &iClass2);
	if (frm.fOkClicked()) {
		wSmplStat->EnableWindow(false);
		sms->MrgClass(byteConv(iClass1), byteConv(iClass2));
		mpv->GetDocument()->UpdateAllViews(0,6);
		//    mpv->Invalidate();
		wSmplStat->EnableWindow(true);
	}  
}

void SampleSetEditor::OnShowRpr()
{
	//Representation rpr = sms->dm()->rpr();
	//  if (!rpr.fValid())
	//    return;
	FileName fn = _rpr->fnObj;
	String s = "show ";
	s &= fn.sFullPathQuoted(true);
	IlwWinApp()->Execute(s);
}

void SampleSetEditor::OnFeatureSpace()
{
	int b1 = sms->mpl()->iLower();
	int b2 = b1 + 1;
	AskFeatureSpaceForm frm(mpv, sms->mpl(), &b1, &b2);
	if (frm.fOkClicked()) {
		FeatureSpaceWindow* fsw = new FeatureSpaceWindow(sms, b1, b2);
		MakeFSWindow(fsw,CPoint(100,100), ((MapPaneView*)mpv)->mwParent()->iNewBarID());
	}
}

void SampleSetEditor::MakeFSWindow(FeatureSpaceWindow* fsw, CPoint p, int iID)
{
	GeneralBar* gb = new GeneralBar;
	gb->view = fsw;
	gb->Create(mpv->mwParent(), iID, CSize(400,400));
	gb->EnableDocking(0);
	fsw->Create(gb);
	vgb.push_back(gb);
	int iLower = sms->mpl()->iLower();
	Map b1(sms->mpl()->map(iLower + fsw->iBand1()));
	Map b2(sms->mpl()->map(iLower + fsw->iBand2()));
	String s(TR("Feature space for %S and %S").c_str(), b1->sName(), b2->sName());
	gb->SetWindowText(s.c_str());
	mpv->GetDocument()->AddView(fsw);
	fsw->OnInitialUpdate();
	mpv->mwParent()->FloatControlBar(gb,p);
	mpv->mwParent()->ShowControlBar(gb,TRUE,FALSE);
}

int SampleSetEditor::Edit(const Coord& c)
{
	wSmplStat->EnableWindow(false);
	sms->SaveOldSampleMapValues(rcSelect);
	if (0 == PixelEditor::Edit(c))
		return 0;
	sms->AppClass(sValue);
	sms->UpdateStat(rcSelect);
	sms->SetSlct(rcSelect);
	mpv->GetDocument()->UpdateAllViews(mpv,6);  
	wSmplStat->AddUnique(sValue);
	wSmplStat->SelChange();
	wSmplStat->EnableWindow(true);
	return 1;
}

void SampleSetEditor::Paste()
{
	//// Paste sets rcSelect so proper updating is not possible.
	sms->SaveOldSampleMapValues(rcSelect);
	wSmplStat->EnableWindow(false);
	//  PixelEditor::Paste();
	wSmplStat->EnableWindow(true);
	sms->UpdateStat(rcSelect);
}

void SampleSetEditor::Clear()
{
	wSmplStat->EnableWindow(false);
	sms->SaveOldSampleMapValues(rcSelect);
	//  PixelEditor::Clear();
	sms->UpdateStat(rcSelect);
	wSmplStat->EnableWindow(true);
}

void SampleSetEditor::EditFieldOK(Coord c, const String& s)
{
	sms->SaveOldSampleMapValues(rcSelect);
	PixelEditor::EditFieldOK(c,s);
	sms->AppClass(s);
	sms->UpdateStat(rcSelect);
	sms->SetSlct(rcSelect);
	mpv->GetDocument()->UpdateAllViews(mpv,6);  
	wSmplStat->AddUnique(s);
	wSmplStat->SelChange();
}

void SampleSetEditor::drawSelect(RowCol rc)
{
	//PixelEditor::drawSelect(rc);
	if ( rcSelect.size()!=0 )
		sms->SetSlct(rcSelect);
	wSmplStat->SelChange();
}

void SampleSetEditor::SelectionChanged()
{
	sms->SetSlct(rcSelect);
	mpv->GetDocument()->UpdateAllViews(mpv,6);
}

void SampleSetEditor::PreSaveState()
{
	int iFSWin = 0;
	// remove all feature space windows
	MapWindow* mw = mpv->mwParent();
	POSITION pos = mw->m_listControlBars.GetHeadPosition();
	while (pos != NULL) {
		CControlBar* pBar =
			(CControlBar*)mw->m_listControlBars.GetNext(pos);
		GeneralBar* gb = dynamic_cast<GeneralBar*>(pBar);
		if (0 == gb)
			continue;
		if (gb->IsWindowVisible()) {
			FeatureSpaceWindow* fsw = dynamic_cast<FeatureSpaceWindow*>(gb->view);
			if (0 == fsw)
				continue;
			String sSection("FSWindow%i", iFSWin);
			ObjectInfo::WriteElement(sSection.c_str(), "ID", sms->fnObj, gb->GetDlgCtrlID());
			fsw->SaveSettings(sms->fnObj, iFSWin++);
		}
	}
	if (iFSWin > 0)
		ObjectInfo::WriteElement("SampleSet", "FSWindows", sms->fnObj, iFSWin);
	else
		ObjectInfo::WriteElement("SampleSet", "FSWindows", sms->fnObj, (char*)0);
}

void SampleSetEditor::OnStatisticsPane()
{
	CFrameWnd* fw = mpv->GetTopLevelFrame();
	if (0 == fw)
		return;
	if (wSmplStat->IsWindowVisible())
		fw->ShowControlBar(wSmplStat,FALSE,FALSE);
	else
		fw->ShowControlBar(wSmplStat,TRUE,FALSE);
}

void SampleSetEditor::OnUpdateStatisticsPane(CCmdUI* pCmdUI)
{
	bool fCheck = wSmplStat->IsWindowVisible() != 0;
	pCmdUI->SetCheck(fCheck);
}

zIcon SampleSetEditor::icon() const
{
	return zIcon("SmsIcon");
}

String SampleSetEditor::sTitle() const
{
  String winTitle = String(TR("Sample Set Editor: %S"));
  String s(winTitle.c_str(), sms->sName());
	return s;
}
