#include "Client\Headers\formelementspch.h"
#include "Client\FormElements\FieldRealSlider.h"
#include "Client\FormElements\fldlist.h"
#include "Client\FormElements\objlist.h"
#include "Engine\Drawers\RootDrawer.h"
#include "Engine\Drawers\ComplexDrawer.h"
#include "Engine\Drawers\SpatialDataDrawer.h"
#include "Client\Mapwindow\MapPaneViewTool.h"
#include "Client\MapWindow\Drawers\DrawerTool.h"
#include "Drawers\SetDrawer.h"
#include "Client\Ilwis.h"
#include "Client\Mapwindow\MapCompositionDoc.h"
#include "Client\Mapwindow\LayerTreeView.h"
#include "Client\Mapwindow\MapPaneViewTool.h"
#include "Client\MapWindow\Drawers\DrawerTool.h"
#include "Client\Mapwindow\LayerTreeItem.h" 
#include "Engine\Drawers\RootDrawer.h"
#include "Drawers\DrawingColor.h" 
#include "Drawers\LayerDrawer.h"
#include "Drawers\RasterLayerDrawer.h"
#include "Drawers\ColorCompositeDrawer.h"
#include "Engine\Drawers\DrawerContext.h"
#include "DrawersUI\ColorCompositeTool.h"
#include "DrawersUI\LayerDrawerTool.h"

using namespace ILWIS;

DrawerTool *createColorCompositeTool(ZoomableView* zv, LayerTreeView *view, NewDrawer *drw) {
	return new ColorCompositeTool(zv, view, drw);
}

ColorCompositeTool::ColorCompositeTool(ZoomableView* zv, LayerTreeView *view, NewDrawer *drw) : 
	DrawerTool("ColorCompositeTool",zv, view, drw)
{
}

ColorCompositeTool::~ColorCompositeTool() {
}

void ColorCompositeTool::clear() {
}

bool ColorCompositeTool::isToolUseableFor(ILWIS::DrawerTool *tool) { 
	LayerDrawerTool *sdrwt = dynamic_cast<LayerDrawerTool *>(tool);
	if ( !sdrwt)
		return false;
	ColorCompositeDrawer *rdrw = 0;
	if ( (rdrw = dynamic_cast<ColorCompositeDrawer *>(tool->getDrawer())) == 0)
		return false;
	if ( !rdrw->isColorComposite())
		return false;
	parentTool = tool;

	return true;
}

HTREEITEM ColorCompositeTool::configure( HTREEITEM parentItem) {
	DisplayOptionTreeItem *item = new DisplayOptionTreeItem(tree, parentItem, drawer);
	item->setDoubleCickAction(this, (DTDoubleClickActionFunc)(DisplayOptionItemFunc)&ColorCompositeTool::displayOptionCC);
	htiNode = insertItem(TR("Color Composite"),"ColorComposite",item);
	addBands();
	DrawerTool::configure(htiNode);

	return htiNode;
}

void ColorCompositeTool::addBands() {
	ColorCompositeDrawer *rdrw = (ColorCompositeDrawer *)drawer;
	MapList mpl = rdrw->getMapList();
	if ( mpl.fValid()) {
		DisplayOptionTreeItem *item = new DisplayOptionTreeItem(tree, htiNode, drawer);
		Map mp = mpl[rdrw->getColorCompositeBand(0)];
		insertItem(String(TR("Red: %S").c_str(), mp->fnObj.sFile),".mpr", item);
		item->setDoubleCickAction(this, (DTDoubleClickActionFunc)(DisplayOptionItemFunc)&ColorCompositeTool::stretchCC1);

		item = new DisplayOptionTreeItem(tree, htiNode, drawer);
		mp = mpl[rdrw->getColorCompositeBand(1)];
		insertItem(String(TR("Green: %S").c_str(), mp->fnObj.sFile),".mpr", item);
		item->setDoubleCickAction(this, (DTDoubleClickActionFunc)(DisplayOptionItemFunc)&ColorCompositeTool::stretchCC2);

		item = new DisplayOptionTreeItem(tree, htiNode, drawer);
		mp = mpl[rdrw->getColorCompositeBand(2)];
		insertItem(String(TR("Blue: %S").c_str(), mp->fnObj.sFile),".mpr", item);
		item->setDoubleCickAction(this, (DTDoubleClickActionFunc)(DisplayOptionItemFunc)&ColorCompositeTool::stretchCC3);

	}

}

void ColorCompositeTool::stretchCC1() {
	new SetStretchCCForm(tree,(ColorCompositeDrawer *)getDrawer(),0); 
}

void ColorCompositeTool::stretchCC2() {
	new SetStretchCCForm(tree,(ColorCompositeDrawer *)getDrawer(),1); 
}

void ColorCompositeTool::stretchCC3() {
	new SetStretchCCForm(tree,(ColorCompositeDrawer *)getDrawer(),2); 
}

void ColorCompositeTool::displayOptionCC() {
	new SetBandsForm(tree,this);
}

String ColorCompositeTool::getMenuString() const {
	return TR("Color Composite");
}

//------------------------------------------------
SetBandsForm::SetBandsForm(CWnd *wPar, ColorCompositeTool *_ccTool) : 
DisplayOptionsForm((ComplexDrawer *)_ccTool->getDrawer(), wPar,TR("Select bands for Color Composite")),
ccTool(_ccTool)
{
	exception = false;
	ColorCompositeDrawer *ccDrawer = (ColorCompositeDrawer *)ccTool->getDrawer();
	MapList mpl = ccDrawer->getMapList();
	v1 = ccDrawer->getColorCompositeBand(0);
	v2=  ccDrawer->getColorCompositeBand(1);
	v3 = ccDrawer->getColorCompositeBand(2);
	for(int i = 0; i < mpl->iSize(); ++i) {
		string band1 = mpl[i]->fnObj.sFile + ".mpr";
		names.push_back(band1);
	}
	//string band1 = mpl[v1]->fnObj.sFile + ".mpr";
	//string band2 = mpl[v2]->fnObj.sFile + ".mpr";
	//string band3 = mpl[v3]->fnObj.sFile + ".mpr";
	//names.push_back(band1);
	//names.push_back(band2);
	//names.push_back(band3);

	fm1 = new FieldOneSelectString(root,TR("Red"),&v1,names);
	fm2 = new FieldOneSelectString(root,TR("Green"),&v2,names);
	fm3 = new FieldOneSelectString(root,TR("Blue"),&v3, names);
	fm1->SetComboWidth(120);
	fm2->SetComboWidth(120);
	fm3->SetComboWidth(120);
	//FieldGroup *fg = new FieldGroup(root, true);
	//cb = new CheckBox(fg,TR("Exception Color"),&exception);
	//cb->SetCallBack((NotifyProc)&SetBandsForm::setExc);
	//cb->SetIndependentPos();
	//fi1 = new FieldInt(cb,"",&e1);
	//fi1->Align(cb, AL_AFTER);
	//fi2 = new FieldInt(cb,"",&e2);
	//fi2->Align(fi1, AL_AFTER);
	//fi3 = new FieldInt(cb,"",&e3);
	//fi3->Align(fi2, AL_AFTER);
	//cb->Hide();


	create();
}

int SetBandsForm::setExc(Event *ev) {
	//cb->StoreData();
	if ( exception) {
		ColorCompositeDrawer *rdr = (ColorCompositeDrawer *)drw;
		Color clr = rdr->getExceptionColor();
		if ( clr.fEqual(colorUNDEF))
			clr = Color(0,0,0);
		//fi1->SetVal(clr.red());
		//fi2->SetVal(clr.green());
		//fi3->SetVal(clr.blue());
	}
	return 1;
}

void  SetBandsForm::apply() {
	fm1->StoreData();
	fm2->StoreData();
	fm3->StoreData();
	//cb->StoreData();
	//fi1->StoreData();
	//fi2->StoreData();
	//fi3->StoreData();
	ColorCompositeDrawer *rdr = (ColorCompositeDrawer *)drw;
	rdr->setColorCompositeBand(0,v1);
	rdr->setColorCompositeBand(1,v2);
	rdr->setColorCompositeBand(2,v3);
	//if ( exception)
	//	rdr->setExceptionColor(Color(e1,e2,e3));
	view->DeleteAllItems(ccTool->getTreeItem(), true);
	ccTool->addBands();

	PreparationParameters pp(NewDrawer::ptREDRAW, 0);
	rdr->prepareChildDrawers(&pp);

	updateMapView();
}

//------------------------------------
SetStretchCCForm::SetStretchCCForm(CWnd *wPar, ColorCompositeDrawer *dr,int _index)
	: index(_index)
	, DisplayOptionsForm2((ComplexDrawer *)dr,wPar,"Set stretch")
	, rrAllowedRange (dr->getMapList()[dr->getColorCompositeBand(_index)]->rrMinMax())
	, rStep (dr->getMapList()[dr->getColorCompositeBand(_index)]->dvrs().rStep())
	, inRace(false)
	, fStarting(true)
{	
	RangeReal rrCurrentLoHi = dr->getColorCompositeRange(index);
	if (!rrCurrentLoHi.fValid())
		rrCurrentLoHi = rrAllowedRange;

	low = rrCurrentLoHi.rLo();
	high = rrCurrentLoHi.rHi();
	sliderLow = new FieldRealSliderEx(root,"Lower", &low,ValueRange(rrAllowedRange, rStep),true);
	sliderHigh = new FieldRealSliderEx(root,"Upper", &high,ValueRange(rrAllowedRange, rStep),true);
	sliderHigh->Align(sliderLow, AL_UNDER);
	sliderLow->SetCallBack((NotifyProc)&SetStretchCCForm::check);
	sliderHigh->SetCallBack((NotifyProc)&SetStretchCCForm::check);
	create();
	fStarting = false;
}

FormEntry *SetStretchCCForm::CheckData() {
	sliderLow->StoreData();
	sliderHigh->StoreData();
	if ( low < rrAllowedRange.rLo())
		return sliderLow;
	if ( high > rrAllowedRange.rHi())
		return sliderHigh;
	if ( high < low)
		return sliderHigh;
	return 0;
}
int  SetStretchCCForm::check(Event *) {
	if (fStarting)
		return 1;
	sliderLow->StoreData();
	sliderHigh->StoreData();

	if ( low == rUNDEF || high == rUNDEF)
		return 1;

	if ( low > high && !inRace){
		low = high;
		inRace = true;
		sliderLow->SetVal(low);
	}
	if ( high < low && !inRace){
		high = low;
		inRace = true;
		sliderHigh->SetVal(high);
	}
	
	ColorCompositeDrawer *setdr = (ColorCompositeDrawer *)drw;
	if ( setdr->isColorComposite()) {
		setdr->setColorCompositeRange(index, RangeReal(low, high));
		//setdr->setStretchRangeReal(RangeReal(low,high)); // Obsolete; a color composite does not use the setdrawer's stretch range; each band has its own stretch range (the color composite range, with "index")
		PreparationParameters pp(NewDrawer::ptRENDER, 0);
		setdr->prepareChildDrawers(&pp);
		updateMapView();
	}
	inRace = false;
	return 1;
}

void  SetStretchCCForm::apply() {
	check(0);

}

