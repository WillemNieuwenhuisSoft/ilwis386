#include "Client\Headers\formelementspch.h"
#include "Client\FormElements\fldcolor.h"
#include "Client\FormElements\FieldListView.h"
#include "Client\FormElements\FieldIntSlider.h"
#include "Engine\Drawers\RootDrawer.h"
#include "Engine\Drawers\ComplexDrawer.h"
#include "Engine\Drawers\SimpleDrawer.h" 
#include "Engine\Drawers\SpatialDataDrawer.h"
#include "Client\Mapwindow\MapPaneView.h"
#include "Client\Mapwindow\MapPaneViewTool.h"
#include "Client\MapWindow\Drawers\DrawerTool.h"
#include "Drawers\SetDrawer.h"
#include "Engine\Domain\dmclass.h"
#include "Client\Ilwis.h"
#include "Client\Mapwindow\MapCompositionDoc.h"
#include "Client\Mapwindow\LayerTreeView.h"
#include "Client\Mapwindow\MapPaneViewTool.h"
#include "Client\MapWindow\Drawers\DrawerTool.h"
#include "Client\Mapwindow\LayerTreeItem.h" 
#include "Engine\Drawers\DrawerContext.h"
#include "drawers\linedrawer.h"
#include "Drawers\AnnotationDrawers.h"
#include "DrawersUI\AnnotationLegendDrawerTool.h"

using namespace ILWIS;

DrawerTool *createAnnotationLegendDrawerTool(ZoomableView* zv, LayerTreeView *view, NewDrawer *drw) {
	return new AnnotationLegendDrawerTool(zv, view, drw);
}

AnnotationLegendDrawerTool::AnnotationLegendDrawerTool(ZoomableView* zv, LayerTreeView *view, NewDrawer *drw) : 
	DrawerTool(TR("AnnotationLegendDrawerTool"), zv, view, drw), legend(0)
{
}

AnnotationLegendDrawerTool::~AnnotationLegendDrawerTool() {
}

void AnnotationLegendDrawerTool::clear() {
}

bool AnnotationLegendDrawerTool::isToolUseableFor(ILWIS::DrawerTool *drw) { 

	return false;
}

HTREEITEM AnnotationLegendDrawerTool::configure( HTREEITEM parentItem) {
	DisplayOptionTreeItem *item = new DisplayOptionTreeItem(tree, parentItem, drawer);
	item->setCheckAction(this, 0,(DTSetCheckFunc)&AnnotationLegendDrawerTool::makeActive);
	htiNode = insertItem(TR("Legend"),"legend",item, legend && legend->isActive());
	item = new DisplayOptionTreeItem(tree, htiNode, drawer);
	item->setDoubleCickAction(this,(DTDoubleClickActionFunc)&AnnotationLegendDrawerTool::setPosition);
	insertItem(TR("Size & Position"),"Position",item);

	item = new DisplayOptionTreeItem(tree, htiNode, drawer);
	item->setDoubleCickAction(this,(DTDoubleClickActionFunc)&AnnotationLegendDrawerTool::setAppearance);
	insertItem(TR("Size & Position"),"Picture",item);

	DrawerTool::configure(htiNode);
	return htiNode;
}

String AnnotationLegendDrawerTool::getMenuString() const {
	return "Legend Annotation";
}

void AnnotationLegendDrawerTool::setPosition() {
	if ( legend)
		new LegendPosition(tree,legend);
}

void AnnotationLegendDrawerTool::setAppearance() {
	if ( legend)
		new LegendAppearance(tree, legend);
}


void AnnotationLegendDrawerTool::makeActive(void *v, HTREEITEM ) {
	bool act = *(bool *)v;
	if ( legend) {
		legend->setActive(act);
		PreparationParameters pp(NewDrawer::ptGEOMETRY | NewDrawer::ptRENDER);
		legend->prepare(&pp);
	}
	else {
		if ( act) {
			PreparationParameters pp(NewDrawer::ptGEOMETRY | NewDrawer::ptRENDER);
			SpatialDataDrawer *spdr = (SpatialDataDrawer *)(drawer->getParentDrawer());
			BaseMapPtr *bmp = spdr->getBaseMap();
			ILWIS::DrawerParameters dp(drawer->getRootDrawer(), drawer);
			if (  bmp->dm()->pdv())
				legend = (AnnotationLegendDrawer *)NewDrawer::getDrawer("AnnotationValueLegendDrawer","ilwis38",&dp);
			else if (  bmp->dm()->pdc()) {
				legend = (AnnotationLegendDrawer *)NewDrawer::getDrawer("AnnotationClassLegendDrawer","ilwis38",&dp);
			}
			if ( legend) {
				legend->prepare(&pp);
				((ComplexDrawer *)drawer)->addPostDrawer(735, legend);
			}
		}
	}
	mpvGetView()->Invalidate();
}

//--------------------------------------------------------
LegendPosition::LegendPosition(CWnd *wPar, AnnotationLegendDrawer *dr) : 
	DisplayOptionsForm2(dr,wPar,TR("Position of Legend")), rg(0), fiColumns(0)
{
	orientation = dr->getOrientation() ? 1 : 0;
	CoordBounds cbZoom = drw->getRootDrawer()->getCoordBoundsZoom();
	CoordBounds cbBox = dr->getBox();
	cols = dr->noOfColumns();
	x = 100 * ( cbBox.MinX() - cbZoom.MinX() ) / cbZoom.width();
	y = 100 * ( cbBox.MinY() - cbZoom.MinY()) / cbZoom.height();
	sliderH = new FieldIntSliderEx(root,TR("X position"), &x,ValueRange(0,100),true);
	sliderV = new FieldIntSliderEx(root,TR("Y position"), &y,ValueRange(0,100),true);
	sliderV->Align(sliderH, AL_UNDER);
	sliderH->SetCallBack((NotifyProc)&LegendPosition::setPosition);
	sliderH->setContinuous(true);
	sliderV->SetCallBack((NotifyProc)&LegendPosition::setPosition);
	sliderV->setContinuous(true);
	if ( dr->getDomain()->pdv()) {
		rg = new RadioGroup(root,TR("Orientation"),&orientation,true);
		new RadioButton(rg,TR("Horizontal"));
		new RadioButton(rg,TR("Vertical"));
		rg->SetCallBack((NotifyProc)&LegendPosition::setOrientation);
	} else {
		fiColumns = new FieldInt(root,TR("Number of Columns"),&cols);
		fiColumns->SetCallBack((NotifyProc)&LegendPosition::setPosition);
	}
	create();
}

int LegendPosition::setOrientation(Event *ev) {
	if ( rg) {
		rg->StoreData();
		AnnotationLegendDrawer *ld = (AnnotationLegendDrawer *)drw;
		ld->setOrientation(orientation != 0);
		updateMapView();
	}

	return 1;
}

int LegendPosition::setPosition(Event *ev) {
	sliderV->StoreData();
	sliderH->StoreData();
	AnnotationLegendDrawer *ld = (AnnotationLegendDrawer *)drw;
	if ( fiColumns) {
		fiColumns->StoreData();
		if ( cols == 0 || cols > 5)
			cols = 1;
		ld->setNoOfColumns(cols);
		PreparationParameters pp(NewDrawer::ptRENDER);
		ld->prepare(&pp);

	}
	CoordBounds cbBox = ld->getBox();
	CoordBounds cbZoom = drw->getRootDrawer()->getCoordBoundsZoom();
	double newx = cbZoom.width() * x / 100.0 + cbZoom.MinX();
	double newy = cbZoom.height() * y / 100.0 + cbZoom.MinY();
	double w = cbBox.width();
	double h = cbBox.height();
	cbBox.MinX() = newx;
	cbBox.MinY() = newy;
	cbBox.MaxX() = cbBox.MinX() + w ;
	cbBox.MaxY() = cbBox.MinY() + h;
	ld->setBox(cbBox);
	updateMapView();

	return 1;
}

//-------------------------------------------------------------------------------
LegendAppearance::LegendAppearance(CWnd *wPar, AnnotationLegendDrawer *dr) : DisplayOptionsForm(dr,wPar,TR("Appearance of Legend")) 
{
	useBgColor = dr->getUseBackBackground();
	bgColor = dr->getBackgroundColor();
	drawBoundary = dr->getDrawBorder();
	FieldGroup *fg1 = new FieldGroup(root);
	cbColor = new CheckBox(fg1,TR("Background color"),&useBgColor);
	fc = new FieldColor(cbColor,"",&bgColor);
	fc->Align(cbColor, AL_AFTER);
	cbBoundary = new CheckBox(root,TR("Draw Boundary"),&drawBoundary);
	cbBoundary->Align(cbColor, AL_UNDER);
	if ( dr->getDomain()->pdc()) {
		vector<FLVColumnInfo> cols;
		StaticText *st = new StaticText(root,TR("Active classes"));
		cols.push_back(FLVColumnInfo("Name", 150));
		fview = new FieldListView(root,cols,LVS_EX_GRIDLINES);
		fview->Align(st, AL_AFTER);		 
		for(int i=1; i <= dr->getDomain()->pdc()->iSize(); ++i) {
			long iRaw = dr->getDomain()->pdc()->iKey(i);
			if ( iRaw == iUNDEF)
				continue;
			//raws.push_back(iRaw);
			vector<String> values;
			String sV = dr->getDomain()->pdc()->sValueByRaw(iRaw);
			values.push_back(sV);
			fview->AddData(values);
		}
	}
	create();
	if ( dr->getDomain()->pdc()) {
		vector<int> raws;
		((AnnotationClassLegendDrawer *)dr)->getActiveClasses(raws);
		for(int i = 0; i < raws.size(); ++i) {
			raws[i] = dr->getDomain()->pdc()->iOrd(raws[raws.size() - i]);
		}
		fview->setSelectedRows(raws);

	}
}

void LegendAppearance::apply() {
	cbColor->StoreData();
	fc->StoreData();
	vector<int> rows;
	fview->getSelectedRowNumbers(rows);
	cbBoundary->StoreData();

	AnnotationLegendDrawer *andrw = (AnnotationLegendDrawer *)drw;
	andrw->setUseBackground(useBgColor);
	andrw->setBackgroundColor(bgColor);
	andrw->setDrawBorder(drawBoundary);
	if ( andrw->getDomain()->pdc()) {
		vector<int> rws;
		for(int i=0; i < rows.size(); ++i){
			rws.push_back(andrw->getDomain()->pdc()->iKey(rows[i]));	
		}

		((AnnotationClassLegendDrawer *)andrw)->setActiveClasses(rws);
	}

	updateMapView();

}

