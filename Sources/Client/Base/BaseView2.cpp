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
#include "Client\Headers\formelementspch.h"
#include "Headers\constant.h"
#include "Client\ilwis.h"
#include "Engine\Base\AssertD.h"
#include "Client\Base\IlwisDocument.h"
#include "Client\Base\Framewin.h"
#include "Engine\Base\System\RegistrySettings.h"
#include "Client\Base\BaseView2.h"
#include "Headers\messages.h"
#include "Headers\Hs\Userint.hs"

static const int STRING_NOT_FOUND = -1;

//--[ BaseView2 ]---------------------------------------------------------------------
IMPLEMENT_DYNCREATE(BaseView2, CView)

BEGIN_MESSAGE_MAP(BaseView2, CView)
	ON_WM_CREATE()
	ON_COMMAND(ID_EDIT_PASTE,           OnPaste)  
	ON_COMMAND(ID_EDIT_CUT,             OnCut)  
	ON_COMMAND(ID_EDIT_COPY,            OnCopy) 
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY,  OnUpdateClip)	
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateClip)	
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT,   OnUpdateClip)	
END_MESSAGE_MAP()


BaseView2::BaseView2() : CView()
{

}

BaseView2::~BaseView2()
{}


void BaseView2::OnInitialUpdate() 
{
	CView::OnInitialUpdate() ;
}

string BaseView2::sGetDescription()
{
	return descBar.sGetText();
}

void BaseView2::SetDescription(string s)
{
	descBar.SetText(s);
}

void BaseView2::CreateDescriptionBar()
{
	CFrameWnd* frm = GetParentFrame();
	BOOL fRet = descBar.Create(frm, GetDocument());
	ISTRUE(fINotEqual, fRet, FALSE);
	frm->EnableDocking(CBRS_ALIGN_ANY);
	frm->DockControlBar(&descBar);
}

// finds a string sStr in the current submenu. It disregards the '&';
int BaseView2::iFindMenuString(const CMenu *menu, const String& sStr)
{
	if (!menu) return STRING_NOT_FOUND;

	String sFindStr = sStr.sHead("&") + sStr.sTail("&");
	int iCount = menu->GetMenuItemCount();
	for ( int i = 0; i < iCount; ++i)
	{
		CString sIt;
		menu->GetMenuString(i, sIt, MF_BYPOSITION);
		String sItem(sIt);
		sItem = sItem.sHead("&") + sItem.sTail("&");
							
		if ( sFindStr == sItem )
			return i;
	}
	return STRING_NOT_FOUND;
}

// searches for a popup menu with the name sPopUp;
CMenu *BaseView2::FindMenu(CMenu *menu, const string& sPopUp)
{
	if ( !menu ) return NULL;

	int iIndex; 
	if ((iIndex=iFindMenuString(menu, sPopUp)) != STRING_NOT_FOUND)
		return menu->GetSubMenu(iIndex);
	return NULL;
}

// adds a menu item to the menu. If the main menu does not exist it also creates this
void BaseView2::AddMenuItem(CMenu *menu, CMenu *subMenu, const String sItem,
													 unsigned int iType, unsigned int iID)
{
	ISTRUE(fINotEqual, GetParentFrame(), (CFrameWnd *)NULL);

	if (iType != MF_SEPARATOR)
	{
		ISTRUE(fIEqual, iFindMenuString(menu,sItem.c_str()) ,STRING_NOT_FOUND);
	}

	if ( menu == NULL) // only true in the case of the mainmenu (adding the first element)
	{
		CMenu mainMenu;
		mainMenu.CreateMenu();
		AddMenuItem(&mainMenu, subMenu,sItem, MF_POPUP, iID);
		GetParentFrame()->SetMenu(&mainMenu);
		mainMenu.Detach();
	}
	else 
	{
		if ( iType != MF_POPUP )
			menu->AppendMenu(iType, iID, sItem.c_str());
		else
			menu->AppendMenu(MF_POPUP, (UINT)subMenu->GetSafeHmenu(), sItem.c_str());
	}
}

// accepts string of the fromat "[<submenu>,]*[<menuitem>]" and constructs a menu (tree) from it
bool BaseView2::AppendMenuItem(const String sMenus, unsigned int iID, unsigned int iType, CMenu *menu)
{
	ISTRUE(fINotEqual, GetParentFrame(), (CFrameWnd *)NULL);

	if (sMenus == "") 
		return false; // end of string reached, end of recursion

	if (menu == NULL) 
		menu = GetParentFrame()->GetMenu(); // mainmenu, parentFrame->GetMenu() will only return 0 in case of the first append

	CMenu *subMenu=FindMenu(menu, sMenus.sHead(",")); // can we find a submenu of the right name
	if ( subMenu) // if subMenu exists go further down this branch
	{
		if (!AppendMenuItem(sMenus.sTail(","), iID, iType, subMenu)) // recurse further down the tree, false means end of line
 			AddMenuItem(menu, subMenu, sMenus.sHead(","), iType, iID);
 	}
	else // no submenu of the right name found, one has to be created
	{
		if (iType != MF_SEPARATOR)
		{
			ISTRUE(fIEqual, iFindMenuString(menu,sMenus.sHead(",").c_str()) ,STRING_NOT_FOUND); // equal names in one submenu are not allowed
		}

		CMenu subMenu;
		subMenu.CreateMenu();
		bool fEndOfString = sMenus.sTail(",") == "";
		if (!fEndOfString ) // menu created, but the menu string still contains items
			AppendMenuItem(sMenus.sTail(","), iID, iType, &subMenu); // recurse further down the chain

		AddMenuItem(menu, &subMenu, sMenus.sHead(","), fEndOfString ? iType : MF_POPUP, iID);

		subMenu.Detach();
	}

	return true;
}	

void BaseView2::InsertMenuItem(unsigned int iCommand, unsigned int iBefore)
{
	CMenu *menu=GetParentFrame()->GetMenu();
	ISTRUE(fINotEqual, menu, (CMenu *)NULL);

	String sText(ILWSF("men",iCommand));

	ISTRUE(fINotEqual, sText, String(""));

	BOOL iRet=menu->InsertMenu(iBefore, MF_BYCOMMAND, iCommand, sText.c_str());

	ISTRUE(fINotEqual, iRet, 0);
}

void BaseView2::AddToFileMenu(unsigned int iID, unsigned int iType)
{
	String sSubMenu(ILWSF("men",ID_MEN_FILE));
	AddToMenu(sSubMenu, iID, iType);
}

void BaseView2::AddToEditMenu(unsigned int iID, unsigned int iType)
{
	String sSubMenu(ILWSF("men",ID_MEN_EDIT));
	AddToMenu(sSubMenu, iID, iType);
}

void BaseView2::AddToOptionsMenu(unsigned int iID, unsigned int iType)
{
	String sSubMenu(ILWSF("men",ID_MEN_OPTIONS));
	AddToMenu(sSubMenu, iID, iType);
}

void BaseView2::AddToViewMenu(unsigned int iID, unsigned int iType)
{
	String sSubMenu(ILWSF("men",ID_MEN_VIEW));
	AddToMenu(sSubMenu, iID, iType);
}

void BaseView2::AddToHelpMenu(unsigned int iID, unsigned int iType)
{
	String sSubMenu(ILWSF("men",ID_MEN_HELP));
	AddToMenu(sSubMenu, iID, iType);
}

void BaseView2::AddToMenu(const String sSubMenu, unsigned int iID, unsigned int iType)
{
	if ( iType != MF_SEPARATOR)
	{
		String sSubItem(ILWSF("men",iID));
		String sItem("%S,%S", sSubMenu, sSubItem);
		AppendMenuItem(sItem, iID, iType);
	}
	else
	{
		CMenu *menu=GetParentFrame()->GetMenu();
		if (menu)
		{
			CMenu *submenu=FindMenu(menu, sSubMenu);
			if ( submenu )
			{
				int iCount = submenu->GetMenuItemCount();
				CString s;
				submenu->GetMenuString(iCount-1, s, MF_BYPOSITION);
				String sItem=sSubMenu + String(",%s", s);
				AppendSeparatorAfter(sItem);
			}
		}
	}
}
bool BaseView2::AppendSeparatorAfter(const String sMenus)
{
	return AppendMenuItem(sMenus, -1, MF_SEPARATOR, NULL);
}

void BaseView2::DeleteMenuItem(unsigned int iItem)
{
	CMenu *menu=GetParentFrame()->GetMenu();
	ISTRUE(fINotEqual, menu, (CMenu *)NULL);
	menu->DeleteMenu(iItem, MF_BYCOMMAND);
}

bool BaseView2::DeleteMenuItem(CMenu *menu, const String sMenus)
{
	CMenu *subMenu = FindMenu(menu, sMenus.sHead(","));
	if (!subMenu ) 
		return false;
	if (!DeleteMenuItem(subMenu, sMenus.sTail(",")))
	{
		int iIndex=iFindMenuString(subMenu, sMenus.sTail(","));
		if ( iIndex != STRING_NOT_FOUND )
			subMenu->DeleteMenu(iIndex, MF_BYPOSITION);
	}
	
	return true;
}

void BaseView2::SetMenuState(unsigned int iCommand, unsigned int iFlags)
{
	CMenu *menu=GetParentFrame()->GetMenu();
	ISTRUE(fINotEqual, menu, (CMenu *)NULL);

	if ( iFlags & MF_CHECKED )
		menu->CheckMenuItem(iCommand, MF_BYCOMMAND | MF_CHECKED);
	else
		menu->CheckMenuItem(iCommand, MF_BYCOMMAND | MF_UNCHECKED);
	if ( iFlags & MF_DISABLED )
		menu->EnableMenuItem(iCommand, MF_BYCOMMAND | MF_DISABLED);
	else
		menu->EnableMenuItem(iCommand, MF_BYCOMMAND | MF_ENABLED);
	if ( iFlags & MF_GRAYED )
		menu->EnableMenuItem(iCommand, MF_BYCOMMAND | MF_GRAYED);
}

UINT BaseView2::GetMenuState(unsigned int iCommand)
{
	CMenu *menu=GetParentFrame()->GetMenu();
	ISTRUE(fINotEqual, menu, (CMenu *)NULL);

	return menu->GetMenuState(iCommand, MF_BYCOMMAND);
}

LRESULT BaseView2::OnDescriptionChanged(WPARAM wP, LPARAM lP)
{
	CDocument *d = GetDocument();
	IlwisDocument *doc = dynamic_cast<IlwisDocument *> (d);
	ISTRUE(fINotEqual, doc, (IlwisDocument *)NULL);

	CString *str = static_cast<CString *> ( (void *)wP);
	String s(*str);
	doc->obj()->SetDescription(s);
	doc->obj()->Updated();
	return TRUE;
}

LRESULT BaseView2::OnViewSettings(WPARAM wP, LPARAM lP)
{
	FrameWindow* pfrm = dynamic_cast<FrameWindow*>(GetParentFrame());
	switch (wP)
	{
		case LOADSETTINGS:	LoadState(IlwisSettings(pfrm->sWindowName()));
							break;
		case SAVESETTINGS:	SaveState(IlwisSettings(pfrm->sWindowName()));
							break;
	}
	return TRUE;
}

void BaseView2::LoadState(IlwisSettings& settings)
{
}

void BaseView2::SaveState(IlwisSettings& settings)
{
}

void BaseView2::OnDraw(CDC *pdc)
{
}

int BaseView2::OnCreate(LPCREATESTRUCT lpcs)
{
	return CView::OnCreate(lpcs);
}

void BaseView2::OnUpdateClip(CCmdUI *cmd)
{
	CWnd *wnd = GetFocus();
	CEdit * ed = dynamic_cast<CEdit *> (wnd);
	if ( !ed)
	{
		cmd->Enable(false);
	  return;
	}

	int rep = (int)wnd->SendMessage(EM_GETSEL, 0, 0);
	zRange rr(LOWORD(rep),HIWORD(rep));
	bool fEnable = rr.hi() - rr.lo() > 0 ? true : false;
	switch(cmd->m_nID)
	{
		case ID_EDIT_CUT:
    case ID_EDIT_COPY:
			cmd->Enable(fEnable);
			break;
		case ID_EDIT_PASTE:
		{
			zClipboard clip(wnd);
			cmd->Enable(clip.isTextAvail());
		}
	}
}

void BaseView2::OnPaste()
{
 	CWnd *wnd = GetFocus();
	if (wnd) 
		wnd->PostMessage(WM_PASTE, 0, 0);
}

void BaseView2::OnCopy()
{
	CWnd *wnd = GetFocus();
	if (wnd)
		wnd->PostMessage( WM_COPY, 0, 0);
}

void BaseView2::OnCut()
{
	CWnd *wnd = GetFocus();
	if (wnd)
		wnd->PostMessage(WM_CUT, 0, 0);
}
