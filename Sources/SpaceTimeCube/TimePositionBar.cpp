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
// TimePositionBar.cpp: implementation of the TimePositionBar class.
//
//////////////////////////////////////////////////////////////////////

#include "Client\Headers\formelementspch.h"
#include "TimePositionBar.h"
#include "TimeBounds.h"
#include "Client\Mapwindow\InfoLine.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using namespace ILWIS;

BEGIN_MESSAGE_MAP(TimeSliderCtrl, CSliderCtrl)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_VSCROLL_REFLECT()
END_MESSAGE_MAP()

TimeSliderCtrl::TimeSliderCtrl()
: CSliderCtrl()
, info(0)
, fDragging(false)
, sTimePosText(0)
{
}

TimeSliderCtrl::~TimeSliderCtrl()
{
	if (info)
		delete info;
}

BOOL TimeSliderCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	if (CSliderCtrl::Create(dwStyle, rect, pParentWnd, nID)) {
		info = new InfoLine(this);
		return TRUE;
	} else
		return FALSE;
}

void TimeSliderCtrl::SetTimePosText(String * _sTimePosText)
{
	sTimePosText = _sTimePosText;
}

void TimeSliderCtrl::SetTimeBounds(TimeBounds * _timeBounds)
{
	timeBounds = _timeBounds;
}

void TimeSliderCtrl::VScroll(UINT nSBCode, UINT nPos)
{
	if (timeBounds->fValid()) {
		if (nSBCode == TB_THUMBTRACK || nSBCode == TB_THUMBPOSITION)
			SendTimeMessage(timeBounds->tMin() + (ILWIS::Time)((timeBounds->tMax() - timeBounds->tMin()) * (sliderRange - nPos) / (double)sliderRange), long(this));
		else
			SendTimeMessage(timeBounds->tMin() + (ILWIS::Time)((timeBounds->tMax() - timeBounds->tMin()) * (sliderRange - GetPos()) / (double)sliderRange), long(this));
	} else {
		if (nSBCode == TB_THUMBTRACK || nSBCode == TB_THUMBPOSITION)
			SendTimeMessage((sliderRange - nPos) / (double)sliderRange, long(this));
		else
			SendTimeMessage((sliderRange - GetPos()) / (double)sliderRange, long(this));
	}
}

void TimeSliderCtrl::SetTime(ILWIS::Time time, long sender)
{
	if (sender == (long) this)
		return;

	double timePerc;
	if (timeBounds->fValid())
		timePerc = (time - timeBounds->tMin()) / (timeBounds->tMax() - timeBounds->tMin());
	else
		timePerc = time;

	if (timePerc > 1)
		timePerc = 1;
	else if (timePerc < 0)
		timePerc = 0;

	SetPos(sliderRange - timePerc * sliderRange);
}

void TimeSliderCtrl::ShowInfoText()
{
	CRect rect;
	GetThumbRect(rect);
	if (sTimePosText != 0)
		info->text(rect.BottomRight(), *sTimePosText);
	else
		info->text(rect.BottomRight(), "");
}

void TimeSliderCtrl::HideInfoText()
{
	CRect rect;
	GetThumbRect(rect);
	info->text(rect.BottomRight(), "");
}

void TimeSliderCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (fDragging)
		ShowInfoText();
	CSliderCtrl::OnMouseMove(nFlags, point);
}

void TimeSliderCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	fDragging = true;
	ShowInfoText();
	CSliderCtrl::OnLButtonDown(nFlags, point);
}

void TimeSliderCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	fDragging = false;
	HideInfoText();
	CSliderCtrl::OnLButtonUp(nFlags, point);
}

BEGIN_MESSAGE_MAP( TimePositionBar, CSizingControlBar )
	//{{AFX_MSG_MAP( TimePositionBar )
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

TimePositionBar::TimePositionBar(TimeBounds & timeBounds)
: timeBounds(timeBounds)
{
}

TimePositionBar::~TimePositionBar()
{
}

BOOL TimePositionBar::Create(CWnd* pParent)
{
	CRect rect;
	pParent->GetWindowRect(rect);
	if (!CSizingControlBar::Create(TR("Time Position").c_str(), pParent, CSize(25, rect.Height()), TRUE, 123))
		return FALSE;
	m_dwSCBStyle |= SCBS_SHOWEDGES;

	SetBarStyle(GetBarStyle() | CBRS_SIZE_DYNAMIC);
	EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);

	//CSize cz = cdc->GetTextExtent("gk");
	//CRect scrollRect(2, cz.cy + 6, 20, rct.Height());
	//CRect scrollRect;
	//GetWindowRect(scrollRect);
	//CRect scrollRect (1, 1, 30, 100);
	CRect scrollRect(0, 0, 0, 0);
	slider.Create(WS_CHILD | WS_VISIBLE | TBS_VERT | TBS_NOTICKS | TBS_RIGHT, scrollRect, this, ID_STC_TIMEPOSITIONSLIDER);
	slider.SetRange(0, sliderRange);
	slider.SetPos(sliderRange);
	slider.SetTimeBounds(&timeBounds);
	//slider.SetTicFreq(20);
	//slider.ShowWindow(SW_SHOW);

	return TRUE;
}

void TimePositionBar::OnSize(UINT nType, int cx, int cy)
{
	if (::IsWindow(slider.GetSafeHwnd()))
		//slider.SetWindowPos(this, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER);
		slider.MoveWindow(0, 0, cx, cy);
}

void TimePositionBar::SetTime(ILWIS::Time time)
{
	slider.SetTime(time, 0);
}

void TimePositionBar::SetTimePosText(String * _sTimePosText)
{
	slider.SetTimePosText(_sTimePosText);
}


