#include "Client\Headers\formelementspch.h"
#include "Engine\Drawers\RootDrawer.h"
#include "Engine\Drawers\ComplexDrawer.h"
#include "Engine\Drawers\SpatialDataDrawer.h"
#include "Client\Mapwindow\MapPaneView.h"
#include "Drawers\LayerDrawer.h"
#include "Client\Mapwindow\MapPaneViewTool.h"
#include "Client\MapWindow\Drawers\DrawerTool.h"
#include "Drawers\SetDrawer.h"
#include "Client\Ilwis.h"
#include "Client\Mapwindow\MapCompositionDoc.h"
#include "Client\Mapwindow\LayerTreeView.h"
#include "Client\Mapwindow\MapPaneViewTool.h"
#include "Client\MapWindow\Drawers\DrawerTool.h"
#include "Client\Mapwindow\LayerTreeItem.h" 
#include "Engine\Drawers\DrawerContext.h"
#include "DrawersUI\SetDrawerTool.h"
#include "DrawersUI\LayerDrawerTool.h"
#include "Client\FormElements\FieldListView.h"
#include "CrossSectionGraph.h"
#include "Engine\Drawers\ComplexDrawer.h" 
#include "Engine\Drawers\SimpleDrawer.h"
#include "drawers\pointdrawer.h"
#include "DrawersUI\CrossSectionTool.h"
#include "headers\constant.h"

using namespace ILWIS;
ProbeMarkers::ProbeMarkers(DrawerParameters *parms) : 
ComplexDrawer(parms,"ProbeMarkers")
{
}

ProbeMarkers::~ProbeMarkers() {
}


void ProbeMarkers::prepare(PreparationParameters *p){
	//properties.symbol = "crosshair";
	//setSpecialDrawingOptions(NewDrawer::sdoExtrusion, true);
	ComplexDrawer::prepare(p);
}

void ProbeMarkers::addMarker(const Coord& crd) {
	DrawerParameters dp(this->getRootDrawer(), this);
	int currentNr = getDrawerCount();
	Color clr = Representation::clrPrimary(currentNr == 0 ? 1 : currentNr + 3);

	PointDrawer *pdrw = new PointDrawer(&dp);
	PointProperties *prop = (PointProperties *)pdrw->getProperties();
	prop->symbol = "filled-crosshair";
	prop->drawColor = clr;
	pdrw->setCoord(crd);
	PreparationParameters pp(NewDrawer::ptRENDER);
	pdrw->prepare(&pp);
	addDrawer(pdrw);
}


DrawerTool *createCrossSectionTool(ZoomableView* zv, LayerTreeView *view, NewDrawer *drw) {
	return new CrossSectionTool(zv, view, drw);
}


CrossSectionTool::CrossSectionTool(ZoomableView* zv, LayerTreeView *view, NewDrawer *drw) : 
	DrawerTool("CrossSectionTool",zv, view, drw)
{
	active = false;
	stay = true;
	graphForm = 0;
	working = false;
	markers = 0;
}

CrossSectionTool::~CrossSectionTool() {
	if ( markers)
		drawer->getRootDrawer()->removeDrawer(markers->getId(), true);
	sources.clear();
	if ( graphForm && graphForm->m_hWnd != 0 && IsWindow(graphForm->m_hWnd)) {
		graphForm->wnd()->PostMessage(WM_CLOSE);
	}
}

void CrossSectionTool::clear() {
}

bool CrossSectionTool::isToolUseableFor(ILWIS::DrawerTool *tool) { 

	LayerDrawerTool *layerDrawerTool = dynamic_cast<LayerDrawerTool *>(tool);
	SetDrawerTool *setDrawerTool = dynamic_cast<SetDrawerTool *>(tool);
	if (!layerDrawerTool)
		return false;
	LayerDrawer *ldrw = dynamic_cast<LayerDrawer *>(layerDrawerTool->getDrawer());
	if ( !ldrw)
		return false;

	SpatialDataDrawer *sddr = dynamic_cast<SpatialDataDrawer *>(ldrw->getParentDrawer());
	if ( !sddr)
		return false;
	Domain dm;
	dm.SetPointer(sddr->getSourceSupportObject(IlwisObject::iotDOMAIN));
	if (!dm.fValid() )
		return false;

	bool usable= dm->pdv() || dm->pdi() || dm->pdbool();
	if (!usable)
		return false;
	parentTool = tool;
	return true;
}

HTREEITEM CrossSectionTool::configure( HTREEITEM parentItem) {
	if (!markers) {
		DrawerParameters dp(drawer->getRootDrawer(), drawer);
		markers = new ProbeMarkers(&dp);
		markers->setActive(false);
		drawer->getRootDrawer()->addPostDrawer(732,markers);
	}

	checkItem = new DisplayOptionTreeItem(tree,parentItem,drawer);
	checkItem->setDoubleCickAction(this,(DTDoubleClickActionFunc)&CrossSectionTool::displayOptionAddList);
	checkItem->setCheckAction(this,0, (DTSetCheckFunc)&CrossSectionTool::setcheckTool);
	htiNode = insertItem(TR("Cross section"),"CrossSection",checkItem,working);

	DrawerTool::configure(htiNode);

	return htiNode;
}

void CrossSectionTool::setcheckTool(void *w, HTREEITEM item) {
	working = *(bool *)w;
	if ( working) {
		tree->GetDocument()->mpvGetView()->addTool(this, getId());
		if (!graphForm) {
			graphForm = new CrossSectionGraphFrom(tree, (LayerDrawer *)drawer, sources, this);
			for(int i = 0; i < sources.size(); ++i) {
				graphForm->addSourceSet(sources[i]);
			}
		} else {
			graphForm->ShowWindow(SW_SHOW);
		}
		if ( sources.size() == 0)
			displayOptionAddList();

		for(int i = 0; i < sources.size(); ++i)
			graphForm->addSourceSet(sources[i]);
	}
	else {
		tree->GetDocument()->mpvGetView()->noTool(getId());
		if ( graphForm) {
			graphForm->ShowWindow(SW_HIDE);
			if (markers) {
				markers->setActive(false);
				markers->clear();
				graphForm->reset();
			}
		}
	}
}

void CrossSectionTool::uncheckTool() {
	graphForm = 0;
	if (markers) {
		markers->setActive(false);
		markers->clear();
	}
	mpvGetView()->Invalidate();
	if (htiNode) {
		if (checkItem)
			checkItem->SwitchCheckBox(false);
		CTreeCtrl& tc = tree->GetTreeCtrl();
		tc.SetCheck(htiNode, false);
	}
}

String CrossSectionTool::getMenuString() const {
	return TR("Cross Section");
}

void CrossSectionTool::displayOptionAddList() {
	if (graphForm)
		new ChooseCrossSectionForm(graphForm, (LayerDrawer *)drawer, this);
}

bool CrossSectionTool::isUnique(const FileName& fn) {
	
	for(int i=0; i < sources.size(); ++i) {
		if ( sources.at(i)->fnObj == fn)
			return false;
	}
	return true;
}

void CrossSectionTool::addSource(const FileName& fn) {
	IlwisObject obj;
	if ( IOTYPE(fn) == IlwisObject::iotMAPLIST) {
		MapList mpl(fn);
		obj = mpl;
	} else if (IOTYPE(fn) == IlwisObject::iotOBJECTCOLLECTION) {
		ObjectCollection oc(fn);
		obj = oc;
	}
	if( obj.fValid()) {
		if ( isUnique(obj->fnObj)) {
			sources.push_back(obj);
			if (graphForm)
				graphForm->addSourceSet(obj);
		}
	}
}

void CrossSectionTool::OnLButtonDown(UINT nFlags, CPoint pnt) {
	short state = ::GetKeyState(VK_CONTROL);
	Coord c1 = tree->GetDocument()->rootDrawer->screenToWorld(RowCol(pnt.y, pnt.x));
	markers->setActive(true);
	if ( (state & 0x8000)) {
		markers->addMarker(c1);
	} else {
		markers->clear();
		if ( graphForm)
			graphForm->reset();
		markers->addMarker(c1);
	}
	mpvGetView()->Invalidate();
}
void CrossSectionTool::OnLButtonUp(UINT nFlags, CPoint point) {
	if ( mpvGetView()->iActiveTool == ID_ZOOMIN) // during zooming, no message handling
		return;

	Coord c1 = tree->GetDocument()->rootDrawer->screenToWorld(RowCol(point.y, point.x));
	if ( graphForm && working) {
		if ( sources.size() > 0 && working) {
			graphForm->setSelectCoord(c1);
		}
	}
}

//-------------------------------------------------------------------
ChooseCrossSectionForm::ChooseCrossSectionForm(CWnd *wPar, LayerDrawer *dr, CrossSectionTool *t) : 
	DisplayOptionsForm2(dr,wPar,TR("Add data source"),fbsBUTTONSUNDER | fbsSHOWALWAYS | fbsNOCANCELBUTTON | fbsOKHASCLOSETEXT),
	tool(t)
{
	fm = new FieldDataType(root,TR("Data source"),&name, ".mpl.ioc",true);
	new PushButton(root, TR("Add"),(NotifyProc) &ChooseCrossSectionForm::addSource);
	create();
}


int ChooseCrossSectionForm::addSource(Event *ev) {
	fm->StoreData();
	if ( name != "") {
		tool->addSource(FileName(name));
	}
	return 1;
}

int ChooseCrossSectionForm::exec() {
	fm->StoreData();
	if ( name != "") {
		tool->addSource(FileName(name));
	}

	return 1;
}

//========================================================================
CrossSectionGraphFrom::CrossSectionGraphFrom(CWnd *wPar, LayerDrawer *dr, vector<IlwisObject>& sources, CrossSectionTool *t) :
DisplayOptionsForm2(dr,wPar,TR("Cross section Graph"),fbsBUTTONSUNDER | fbsSHOWALWAYS | fbsNOCANCELBUTTON)
, tool(t)
{
	vector<FLVColumnInfo> v;
	v.push_back(FLVColumnInfo("Source", 200));
	v.push_back(FLVColumnInfo("Probe", 40));
	v.push_back(FLVColumnInfo("Index range", 80));
	v.push_back(FLVColumnInfo("Value range", 130));
	v.push_back(FLVColumnInfo("Selected index", 90));
	v.push_back(FLVColumnInfo("Value", 100));
	graph = new CrossSectionGraphEntry(root, sources,dr->getRootDrawer()->getCoordinateSystem());
	FieldListView * flv = new FieldListView(root,v);
	graph->setListView(flv);
	FieldGroup *fg = new FieldGroup(root,true);
	PushButton *pb1 = new PushButton(fg,TR("Save as Table"), (NotifyProc)&CrossSectionGraphFrom::saveAsTable);
	PushButton *pb2 = new PushButton(fg,TR("Save as Spectrum"), (NotifyProc)&CrossSectionGraphFrom::saveAsSpectrum);
	pb2->Align(pb1, AL_AFTER);
	create();
}

int CrossSectionGraphFrom::saveAsTable(Event *ev) {
	graph->graph()->saveAsTbl();
	return 1;
}

int CrossSectionGraphFrom::saveAsSpectrum(Event *ev) {
	graph->graph()->saveAsSpectrum();
	return 1;
}

void CrossSectionGraphFrom::addSourceSet(const IlwisObject& obj) {
	graph->update();
}

void CrossSectionGraphFrom::setSelectCoord(const Coord& crd) {
	graph->setCoord(crd);
}

void CrossSectionGraphFrom::reset() {
	graph->reset();
}

void CrossSectionGraphFrom::shutdown(int iReturn) {
	if (tool)
		tool->uncheckTool();
	DisplayOptionsForm2::shutdown(iReturn);
}

