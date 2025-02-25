#include "Client\Headers\formelementspch.h"
#include "Client\FormElements\fldcolor.h"
#include "Client\Mapwindow\Drawers\RootDrawer.h"
#include "Client\MapWindow\Drawers\ComplexDrawer.h"
#include "Drawers\CanvasBackgroundDrawer.h"
#include "Client\Ilwis.h"
#include "Engine\Representation\Rpr.h"
#include "Client\Mapwindow\Drawers\AbstractMapDrawer.h"
#include "Client\Mapwindow\LayerTreeView.h"
#include "Client\Mapwindow\MapPaneViewTool.h"
#include "Client\Mapwindow\Drawers\DrawerTool.h"
#include "Client\Mapwindow\LayerTreeItem.h" 
#include "Client\Mapwindow\Drawers\DrawerContext.h"
#include "Drawers\BackgroundTool.h"
//#include "Drawers\RepresentationTool.h"
//#include "Drawers\StretchTool.h"

DrawerTool *createBackgroundTool(ZoomableView* zv, LayerTreeView *view, NewDrawer *drw) {
	return new BackgroundTool(zv, view, drw);
}

BackgroundTool::BackgroundTool(ZoomableView* zv, LayerTreeView *view, NewDrawer *drw) : 
	DrawerTool("BackgroundTool",zv, view, drw)
{
	drawer = ((RootDrawer *)drw)->getDrawer(1, ComplexDrawer::dtPRE);
}

BackgroundTool::~BackgroundTool() {
}

bool BackgroundTool::isToolUseableFor(ILWIS::NewDrawer *drw) { 

	//return dynamic_cast<RootDrawer *>(drw) != 0;
	return false; // tool will be added by "hand", not automatically
}

HTREEITEM BackgroundTool::configure( HTREEITEM parentItem) {
	htiNode = insertItem(TVI_ROOT,"Background Area","MapPane");
	bool is3D = drawer->getRootDrawer()->is3D();
	CanvasBackgroundDrawer *cbdr = (CanvasBackgroundDrawer *)drawer;

	DisplayOptionColorItem *item = new DisplayOptionColorItem("Outside", tree,htiNode,drawer);
	item->setDoubleCickAction(this, (DTDoubleClickActionFunc)&BackgroundTool::displayOptionOutsideColor);
	item->setColor(is3D ? cbdr->getColor(CanvasBackgroundDrawer::clOUTSIDE3D) :  cbdr->getColor(CanvasBackgroundDrawer::clOUTSIDE2D));
	insertItem("Outside map","SingleColor",item);

	item = new DisplayOptionColorItem("Inside", tree,htiNode,drawer);
	item->setDoubleCickAction(this, (DTDoubleClickActionFunc)&BackgroundTool::displayOptionInsideColor);
	insertItem("Inside map","SingleColor",item);
	item->setColor(is3D ? cbdr->getColor(CanvasBackgroundDrawer::clINSIDE3D) :  cbdr->getColor(CanvasBackgroundDrawer::clINSIDE2D));

	DrawerTool::configure(htiNode);
	isConfigured = true;

	return htiNode;
}

void BackgroundTool::displayOptionOutsideColor() {
	CanvasBackgroundDrawer *cbdr = (CanvasBackgroundDrawer *)drawer;
	Color &clr = cbdr->getRootDrawer()->is3D() ? cbdr->getColor(CanvasBackgroundDrawer::clOUTSIDE3D) :  
		                                  cbdr->getColor(CanvasBackgroundDrawer::clOUTSIDE2D);
	new SetColorForm("Outside map", tree, (CanvasBackgroundDrawer *)drawer,  clr);
}

void BackgroundTool::displayOptionInsideColor() {
	CanvasBackgroundDrawer *cbdr = (CanvasBackgroundDrawer *)drawer;
	Color& clr = cbdr->getRootDrawer()->is3D() ? cbdr->getColor(CanvasBackgroundDrawer::clINSIDE3D) :  
		                                  cbdr->getColor(CanvasBackgroundDrawer::clINSIDE2D);
	new SetColorForm("Inside map", tree, (CanvasBackgroundDrawer *)drawer, clr);
}

String BackgroundTool::getMenuString() const {
	return TR("Background");
}

//------------------------------------------------
SetColorForm::SetColorForm(const String& title, CWnd *wPar, CanvasBackgroundDrawer *dr, Color& color) : 
	DisplayOptionsForm(dr, wPar,title),clr(color), c(color)
{
	fc = new FieldColor(root, "Draw color", &c);
	create();
}

void  SetColorForm::apply() {
	fc->StoreData();
	clr = c;
	updateMapView();
}

