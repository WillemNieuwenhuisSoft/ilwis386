#include "Client\Headers\formelementspch.h"
#include "SpaceTimeCubeTool.h"
#include "Client\Mapwindow\LayerTreeItem.h" 
#include "Client\Mapwindow\MapPaneView.h"
#include "DrawersUI\ThreeDGlobalTool.h"
#include "DrawersUI\LayerDrawerTool.h"
#include "DrawersUI\SetDrawerTool.h"
#include "Engine\Drawers\DrawerContext.h"
#include "Client\Mapwindow\MapCompositionDoc.h"
#include "TimePositionBar.h"
#include "TemporalDrawer.h"
#include "SortableDrawer.h"
#include "GroupableDrawer.h"
#include "SizableDrawer.h"
#include "PreTimeOffsetDrawer.h"
#include "PostTimeOffsetDrawer.h"
#include "Client\FormElements\fldcol.h"
#include "Client\FormElements\FldOneSelectTextOnly.h"
#include "Client\FormElements\objlist.h"
#include "Client\FormElements\FieldIntSlider.h"
#include "TimeZoomTool.h"
#include "CubeElementsTool.h"
#include "Client\Base\datawind.h"
#include "CubeDrawer.h"
#include "SpaceTimeElementsDrawer.h"
#include "Drawers\LayerDrawer.h"
#include "Drawers\FeatureLayerDrawer.h"
#include "SpaceTimeDrawer.h"

LayerData::LayerData(NewDrawer *drw)
: drawerId(drw->getId())
, plotOption("<regular>")
, fSort(false)
, fGroup(false)
, fSize(false)
, fSelfTime(false)
, fTime2(false)
, fFeatureMap(false)
, fPointMap(false)
, drawer(drw)
{
	if ( drw->getType() == "FeatureDataDrawer") {
		BaseMapPtr *bmp = ((FeatureDataDrawer *)drw)->getBaseMap();
		fnBaseMap = bmp->fnObj;
		fFeatureMap = IOTYPEFEATUREMAP(fnBaseMap);
		fPointMap = IOTYPE(fnBaseMap) == IlwisObject::iotPOINTMAP;
		fSelfTime = bmp->dm()->pdtime() != 0;
		if (fSelfTime)
			m_rrTimeMinMax = bmp->rrMinMax();
		else if (bmp->fTblAtt()) {
			attTable = bmp->tblAtt();
			for (int i = 0; i < attTable->iCols(); ++i) {
				Column col = attTable->col(i);
				if (col->dm()->pdtime())
					temporalColumns.push_back(col);
			}
			if (temporalColumns.size() == 1) {
				temporalColumn = temporalColumns[0];
				m_rrTimeMinMax = temporalColumn->rrMinMax();
			}
		}
		if (bmp->fTblAtt()) {
			attTable = bmp->tblAtt();
			for (int i = 0; i < attTable->iCols(); ++i) {
				Column col = attTable->col(i);
				if (col->dm()->pdv()) // || col->dm()->pds())
					sortColumns.push_back(col);
				if (col->dm()->pdv() || col->dm()->pds() || col->dm()->pdc())
					groupColumns.push_back(col);
				if (col->dm()->pdv())
					sizeColumns.push_back(col);
			}
			if (sizeColumns.size() > 0) {
				sizeColumn = sizeColumns[0];
				m_rrSizeMinMax = sizeColumn->rrMinMax();
				size2Column = sizeColumns[0];
			}
		}
		updateFromLayer();
	} else
		fFeatureMap = false;
}

bool LayerData::hasTime() {
	return fFeatureMap && (fSelfTime || (temporalColumns.size() > 0));
}

bool LayerData::hasSort() {
	return fFeatureMap && (sortColumns.size() > 0);
}

bool LayerData::hasGroup() {
	return fFeatureMap && (groupColumns.size() > 0);
}

bool LayerData::hasSize() {
	return fFeatureMap && (sizeColumns.size() > 0);
}

String LayerData::sPlotOption() {
	return (fSelfTime || temporalColumn.fValid()) ? plotOption : "<regular>";
}

bool LayerData::fUseSort() {
	return fSort && sortColumn.fValid();
}

bool LayerData::fUseGroup() {
	return fGroup && groupColumn.fValid();
}

void LayerData::setUseTime2(bool _fTime2)
{
	fTime2 = _fTime2;
	m_rrTimeMinMax = temporalColumn->rrMinMax();
	if (fTime2 && temporalColumn2.fValid())
		m_rrTimeMinMax += temporalColumn2->rrMinMax();
}

bool LayerData::fUseSize() {
	return fSize && sizeColumn.fValid();
}

bool LayerData::fUseTime2() {
	return fTime2 && temporalColumn2.fValid();
}

void LayerData::setTimeColumn(String sColName) {
	temporalColumn = attTable->col(sColName);
	m_rrTimeMinMax = temporalColumn->rrMinMax();
	if (fTime2 && temporalColumn2.fValid())
		m_rrTimeMinMax += temporalColumn2->rrMinMax();
}

void LayerData::setTimeColumn2(String sColName) {
	temporalColumn2 = attTable->col(sColName);
	m_rrTimeMinMax = temporalColumn->rrMinMax();
	if (fTime2 && temporalColumn2.fValid())
		m_rrTimeMinMax += temporalColumn2->rrMinMax();
}

void LayerData::setSortColumn(String sColName) {
	sortColumn = attTable->col(sColName);
}

void LayerData::setGroupColumn(String sColName) {
	groupColumn = attTable->col(sColName);
}

void LayerData::setSizeColumn(String sColName) {
	sizeColumn = attTable->col(sColName);
	m_rrSizeMinMax = sizeColumn->rrMinMax();
}

void LayerData::setSize2Column(String sColName) {
	size2Column = attTable->col(sColName);
}

void LayerData::updateFromLayer() {
	if (drawer != 0) {
		NewDrawer * childDrawer = ((ComplexDrawer*)drawer)->getDrawer(0);
		if (childDrawer != 0) {
			String subType = childDrawer->getSubType();
			if (fCIStrEqual(subType.sLeft(5), "Cube:")) {
				plotOption = subType.sSub(5, subType.length() - 5);
				if (hasSort()) {
					SortableDrawer * sortableDrawer = dynamic_cast<SortableDrawer*>(childDrawer);
					if (sortableDrawer) {
						fSort = sortableDrawer->fGetUseSort();
						sortColumn = sortableDrawer->getSortAttribute();
					}
				}
				if (hasGroup()) {
					GroupableDrawer * groupableDrawer = dynamic_cast<GroupableDrawer*>(childDrawer);
					if (groupableDrawer) {
						fGroup = groupableDrawer->fGetUseGroup();
						groupColumn = groupableDrawer->getGroupAttribute();
					}
				}
				if (hasSize()) {
					SizableDrawer * sizableDrawer = dynamic_cast<SizableDrawer*>(childDrawer);
					if (sizableDrawer) {
						fSize = sizableDrawer->fGetUseSize();
						sizeColumn = sizableDrawer->getSizeAttribute();
						size2Column = sizableDrawer->getSize2Attribute();
					}
				}
			} else
				plotOption = "<regular>";
		}
	}
}

//------------------------------------------------------

map<ZoomableView*, SpaceTimeCube*> SpaceTimeCube::spaceTimeCubes;

SpaceTimeCube * SpaceTimeCube::getSpaceTimeCube(ZoomableView* mpv, LayerTreeView * tree, NewDrawer *drw)
{
	map<ZoomableView*, SpaceTimeCube*>::iterator it = spaceTimeCubes.find(mpv);
	if (it != spaceTimeCubes.end())
		return (*it).second;
	else {
		SpaceTimeCube* stc = new SpaceTimeCube(mpv, tree, drw);
		spaceTimeCubes[mpv] = stc;
		return stc;
	}
}

void SpaceTimeCube::deleteSpaceTimeCube(ZoomableView* mpv)
{
	map<ZoomableView*, SpaceTimeCube*>::iterator it = spaceTimeCubes.find(mpv);
	if (it != spaceTimeCubes.end()) {
		SpaceTimeCube * stc = (*it).second;
		spaceTimeCubes.erase(it);
		delete stc;
	}
}

SpaceTimeCube::SpaceTimeCube(ZoomableView * _mpv, LayerTreeView * _tree, NewDrawer *drw)
: mpv(_mpv)
, tree(_tree)
, rootDrawer((RootDrawer*)(drw->getRootDrawer()))
, useSpaceTimeCube(false)
, timePosBar(0)
, layerOptionsForm(0)
, timePos(0)
, timeOffset(0)
, timeShift(0)
{
}

SpaceTimeCube::~SpaceTimeCube()
{
	if (timePosBar) {
		delete timePosBar;
		timePosBar = 0;
	}
	if (layerOptionsForm) {
		delete layerOptionsForm;
		layerOptionsForm = 0;
	}
}

vector<String> gatherOwnIDs(ComplexDrawer * drw) {
	vector<String> result;
	PreTimeOffsetDrawer *predrw = dynamic_cast<PreTimeOffsetDrawer*>(drw->getDrawer(0, ComplexDrawer::dtPRE));
	if (predrw != 0)
		result.push_back(predrw->getId());
	PostTimeOffsetDrawer *postdrw = dynamic_cast<PostTimeOffsetDrawer*>(drw->getDrawer(999, ComplexDrawer::dtPOST));
	if (postdrw != 0)
		result.push_back(postdrw->getId());
	for (int i = 0; i < drw->getDrawerCount(); ++i) {
		ComplexDrawer *childDrw = dynamic_cast<ComplexDrawer*>(drw->getDrawer(i));
		if (childDrw != 0) {
			vector<String> subResult = gatherOwnIDs(childDrw);
			for (vector<String>::iterator s = subResult.begin(); s != subResult.end(); ++s)
				result.push_back(*s);
		}
	}
	return result;
}

void SpaceTimeCube::loadMapview() {
	ownDrawerIDs = gatherOwnIDs(rootDrawer);
}

void SpaceTimeCube::update(bool fFillForm) {
	vector<LayerData> newLayerList;
	for(int i = 0 ; i < rootDrawer->getDrawerCount(); ++i) {
		NewDrawer *drw = rootDrawer->getDrawer(i);
		if (find(ownDrawerIDs.begin(), ownDrawerIDs.end(), drw->getId()) == ownDrawerIDs.end()) {
			int j = 0;
			// if it is in the old list, copy it
			for (; j < layerList.size(); ++j) {
				if (layerList[j].getDrawerId() == drw->getId()) {
					if (fFillForm)
						layerList[j].updateFromLayer();
					newLayerList.push_back(layerList[j]);
					break;
				}
			}
			// if not found in the old list, add a new one
			if (j >= layerList.size()) {
				LayerData layerData (drw);
				if (layerData.hasTime())
					newLayerList.push_back(layerData);
			}
		}
	}
	layerList = newLayerList;
}

void SpaceTimeCube::setUseSpaceTimeCube(bool yesno) {
	useSpaceTimeCube = yesno;
	if (useSpaceTimeCube) {
		timePosBar = new TimePositionBar(timeBoundsZoom);
		timePosBar->Create(mpv->dwParent());
		timePosBar->SetTime(timeBoundsZoom.fValid() ? timeBoundsZoom.tMin() + (ILWIS::Time)((timeBoundsZoom.tMax() - timeBoundsZoom.tMin()) * timePos) : timePos);
		timePosBar->SetTimePosText(&sTimePosText);
		mpv->GetParentFrame()->DockControlBar(timePosBar, AFX_IDW_DOCKBAR_LEFT);
		refreshDrawerList(false);
	}
	else {
		refreshDrawerList(false);
		delete timePosBar;
		timePosBar = 0;
		mpv->dwParent()->RecalcLayout();
	}
}

bool SpaceTimeCube::fUseSpaceTimeCube()
{
	return useSpaceTimeCube;
}

MapCompositionDoc *SpaceTimeCube::getDocument() const {
	return (MapCompositionDoc *)mpv->GetDocument();
}

void SpaceTimeCube::refreshDrawerList(bool fFromForm) {
	CoordBounds cbZoom = rootDrawer->getCoordBoundsZoom(); // backup values
	double rotX, rotY, rotZ, transX, transY, transZ;
	rootDrawer->getRotationAngles(rotX, rotY, rotZ);
	rootDrawer->getTranslate(transX, transY, transZ);
	double zoom3D = rootDrawer->getZoom3D();

	for (int i = 0; i < ownDrawerIDs.size(); ++i) {
		rootDrawer->removeDrawer(ownDrawerIDs[i]);
		for(int j = 0 ; j < rootDrawer->getDrawerCount(); ++j) {
			ComplexDrawer *drw = dynamic_cast<ComplexDrawer*>(rootDrawer->getDrawer(j));
			if (drw != 0) {
				drw->removeDrawer(ownDrawerIDs[i]);
			}
		}
	}

	ownDrawerIDs.clear();

	update(false);		

	timeBoundsFullExtent.Reset();
	sizeStretch = RangeReal();

	mpv->noTool(0); // ensure the mapview has no tools
	for (int i = 0; i < layerList.size(); ++i) {
		SpatialDataDrawer * drawer = (SpatialDataDrawer*)(rootDrawer->getDrawer(layerList[i].getDrawerId()));
		if (drawer == 0)
			continue; // skip it .. drawer was probably removed from the layers
		String sPlotOption = layerList[i].sPlotOption();
		if (!useSpaceTimeCube)
			sPlotOption = "<regular>";
		NewDrawer * childDrawer = drawer->getDrawer(0);
		if (childDrawer != 0) {
			String subType = childDrawer->getSubType();
			bool fChangeType = (fCIStrEqual(subType, "ilwis38") && (sPlotOption != "<regular>"));
			fChangeType |= ((subType == "Cube:<stp>") && (sPlotOption != "<stp>"));
			fChangeType |= ((subType == "Cube:<stations>") && (sPlotOption != "<stations>"));
			fChangeType |= ((subType == "Cube:<rose>") && (sPlotOption != "<rose>"));
			if (fChangeType) {
				DeleteDrawerTools(tree->getRootTool(), childDrawer);
				((ComplexDrawer*)childDrawer)->clear();
				drawer->removeDrawer(childDrawer->getId()); // remove the old drawer
				childDrawer = 0;
			}
		}
		if (childDrawer == 0) {
			PreparationParameters pp(NewDrawer::ptALL);
			if (sPlotOption != "<regular>")
				pp.subType = "Cube:" + sPlotOption;
			else
				pp.subType = "ilwis38";
			drawer->prepare(&pp);
			replaceTreeItem(drawer, (SpatialDataDrawer*)drawer, rootDrawer->getDrawerIndex(drawer));
		}
		PreparationParameters pp(NewDrawer::ptALL);
		TemporalDrawer * temporalDrawer = dynamic_cast<TemporalDrawer*>(((ComplexDrawer*)drawer)->getDrawer(0));
		if (temporalDrawer) {
			temporalDrawer->SetTimeBounds(&timeBoundsZoom);
			RangeReal rrMinMax (layerList[i].rrTimeMinMax());
			timeBoundsFullExtent.AddMinMax(Time(rrMinMax.rLo()), Time(rrMinMax.rHi()));
			if (layerList[i].isSelfTime())
				temporalDrawer->SetSelfTime();
			else {
				temporalDrawer->SetTimeAttribute(layerList[i].getTimeColumn());
				temporalDrawer->SetTimeAttribute2(layerList[i].getTimeColumn2());
			}
			DrawerParameters dp(rootDrawer, rootDrawer);
			PreparationParameters pp(NewDrawer::ptALL);
			AddTimeOffsetDrawers((ComplexDrawer*)drawer->getDrawer(0), &timeShift, dp, pp);
			((SpaceTimeDrawer*)(drawer->getDrawer(0)))->SetTimePosVariable(&timePos);

			SpaceTimeElementsDrawer * spaceTimeElementsDrawer = ((SpaceTimeDrawer*)(drawer->getDrawer(0)))->getAdditionalElementsDrawer();
			if (spaceTimeElementsDrawer) {
				spaceTimeElementsDrawer->clear();
				AddTimeOffsetDrawers(spaceTimeElementsDrawer, &timeOffset, dp, pp); // should we prevent letting this function add to ownDrawerIDs?
			}
		}
		SortableDrawer * sortableDrawer = dynamic_cast<SortableDrawer*>(((ComplexDrawer*)drawer)->getDrawer(0));
		if (sortableDrawer) {
			if (layerList[i].fUseSort())
				sortableDrawer->SetSortAttribute(layerList[i].getSortColumn());
			else
				sortableDrawer->SetNoSort();
		}
		GroupableDrawer * groupableDrawer = dynamic_cast<GroupableDrawer*>(((ComplexDrawer*)drawer)->getDrawer(0));
		if (groupableDrawer) {
			if (layerList[i].fUseGroup())
				groupableDrawer->SetGroupAttribute(layerList[i].getGroupColumn());
			else
				groupableDrawer->SetNoGroup();
		}
		SizableDrawer * sizableDrawer = dynamic_cast<SizableDrawer*>(((ComplexDrawer*)drawer)->getDrawer(0));
		if (sizableDrawer) {
			RangeReal rrMinMax (layerList[i].rrSizeMinMax());
			sizeStretch += rrMinMax;
			if (fFromForm) {
				bool fFormSizePropertiesChanged = (layerList[i].fUseSize() && !sizableDrawer->fGetUseSize());
				fFormSizePropertiesChanged |= (!layerList[i].fUseSize() && sizableDrawer->fGetUseSize());
				if (layerList[i].fUseSize()) {
					fFormSizePropertiesChanged |= (layerList[i].getSizeColumn().fValid() && !sizableDrawer->getSizeAttribute().fValid());
					fFormSizePropertiesChanged |= (!layerList[i].getSizeColumn().fValid() && sizableDrawer->getSizeAttribute().fValid());
					fFormSizePropertiesChanged |= (layerList[i].getSizeColumn().fValid() && sizableDrawer->getSizeAttribute().fValid() && (layerList[i].getSizeColumn() != sizableDrawer->getSizeAttribute()));
				}
				if (fFormSizePropertiesChanged) {
					if (layerList[i].fUseSize())
						sizableDrawer->SetSizeAttribute(layerList[i].getSizeColumn());
					else
						sizableDrawer->SetNoSize();
					sizableDrawer->SetSizeStretch(&sizeStretch);
				}
				sizableDrawer->SetSize2Attribute(layerList[i].getSize2Column());
			}
		}
		pp = PreparationParameters (NewDrawer::pt3D); // re-prepare otherwise sort/nosort etc has no effect
		drawer->prepare(&pp);
	}

	if (useSpaceTimeCube) {

		DrawerParameters dp(rootDrawer, rootDrawer);
		PreparationParameters pp(NewDrawer::ptALL);

		for(int i = 0 ; i < rootDrawer->getDrawerCount(); ++i) {
			ComplexDrawer *drw = dynamic_cast<ComplexDrawer*>(rootDrawer->getDrawer(i));
			if (drw != 0) {
				if (find(ownDrawerIDs.begin(), ownDrawerIDs.end(), drw->getId()) == ownDrawerIDs.end()) {
					int j = 0;
					// if it is in the layerList, and it uses time, leave it
					for (; j < layerList.size(); ++j) {
						if (layerList[j].getDrawerId() == drw->getId()) {
							if (layerList[j].sPlotOption() == "<regular>")
								j = layerList.size();
							else
								break; // keep j same
						}
					}
					// if not found in the layerList, or it does not use time, handle it
					if (j >= layerList.size())
						AddTimeOffsetDrawers(drw, &timeOffset, dp, pp);
				}
			}
		}

		timeBoundsZoom = timeBoundsFullExtent;

		CubeDrawer * cube = (CubeDrawer*)(rootDrawer->getDrawer("CubeDrawer"));
		if (cube == 0) {
			cube = (CubeDrawer*)(NewDrawer::getDrawer("CubeDrawer", "Cube", &dp));
			rootDrawer->insertDrawer(0, cube);
		}
		TemporalDrawer * temporalDrawer = dynamic_cast<TemporalDrawer*>(cube);
		temporalDrawer->SetTimeBounds(&timeBoundsZoom);
		cube->SetTimePosVariables(&timePos, &sTimePosText); // before prepare!! (so that "prepare" can take care of these variables as well)
		cube->prepare(&pp);
		AddTimeOffsetDrawers(cube, &timeShift, dp, pp);
	} else {
		CubeDrawer * cube = (CubeDrawer*)(rootDrawer->getDrawer("CubeDrawer"));
		if (cube != 0)
			rootDrawer->removeDrawer(cube->getId());
	}

	// disable the background drawer in Space Time Cube mode
	NewDrawer * backgroundDrawer = rootDrawer->getBackgroundDrawer();
	if (backgroundDrawer) {
		HTREEITEM hItemBackground = findTreeItem(backgroundDrawer);
		if (hItemBackground)
			tree->GetTreeCtrl().SetCheck(hItemBackground, !useSpaceTimeCube );
		backgroundDrawer->setActive(!useSpaceTimeCube);
	}
	
	// restore (because the drAppend changed it all)
	rootDrawer->setCoordBoundsZoom(cbZoom);
	rootDrawer->setRotationAngles(rotX, rotY, rotZ);
	rootDrawer->setTranslate(transX, transY, transZ);
	rootDrawer->setZoom3D(zoom3D);

	mpv->Invalidate();
}

void SpaceTimeCube::AddTimeOffsetDrawers(ComplexDrawer * drw, double * timeOffsetVariable, DrawerParameters & dp, PreparationParameters & pp)
{
	PreTimeOffsetDrawer* preTimeOffset = dynamic_cast<PreTimeOffsetDrawer*>(NewDrawer::getDrawer("PreTimeOffsetDrawer", "Cube", &dp));
	ownDrawerIDs.push_back(preTimeOffset->getId());
	TemporalDrawer * temporalDrawer = dynamic_cast<TemporalDrawer*>(preTimeOffset);
	temporalDrawer->SetTimeBounds(&timeBoundsZoom);
	preTimeOffset->prepare(&pp);
	preTimeOffset->SetTimeOffsetVariable(timeOffsetVariable);
	PostTimeOffsetDrawer* postTimeOffset = dynamic_cast<PostTimeOffsetDrawer*>(NewDrawer::getDrawer("PostTimeOffsetDrawer", "Cube", &dp));
	ownDrawerIDs.push_back(postTimeOffset->getId());
	temporalDrawer = dynamic_cast<TemporalDrawer*>(postTimeOffset);
	temporalDrawer->SetTimeBounds(&timeBoundsZoom);
	postTimeOffset->prepare(&pp);
	drw->addPreDrawer(0, preTimeOffset);
	drw->addPostDrawer(999, postTimeOffset);
}

void SpaceTimeCube::replaceDrawer(NewDrawer * oldDrw, NewDrawer * newDrw)
{
	oldDrw->setActive(false);
	int index = rootDrawer->getDrawerIndex(oldDrw);
	if (iUNDEF != index) {
		rootDrawer->removeDrawer(oldDrw->getId(), true);
		rootDrawer->insertDrawer(index, newDrw);
		newDrw->setActive(true);
	}
}

bool SpaceTimeCube::replaceTreeItem(NewDrawer * oldDrw, SpatialDataDrawer * newDrw, int index)
{
	HTREEITEM layerItem = findTreeItem(oldDrw);
	if (layerItem != 0) { // change drawer depending on status
		HTREEITEM previous = tree->GetTreeCtrl().GetNextItem(layerItem, TVGN_PREVIOUS);
		tree->DeleteAllItems(layerItem, false);
		tree->addMapItem(newDrw, previous, index);
		return true;
	} else
		return false;
}

void SpaceTimeCube::DeleteDrawerTools(DrawerTool * tool, NewDrawer * drawer)
{
	if (tool->getDrawer() == drawer)
		tool->removeTool(0);
	else {
		for (int i = 0; i < tool->getToolCount(); ++i)
			DeleteDrawerTools(tool->getTool(i), drawer);
	}
}

HTREEITEM SpaceTimeCube::findTreeItem(NewDrawer* drwFind)
{
	HTREEITEM hti= tree->GetTreeCtrl().GetNextItem(TVGN_ROOT, TVGN_CHILD);
	while ( hti) {
		LayerTreeItem *data = (LayerTreeItem*)tree->GetTreeCtrl().GetItemData(hti);
		if ( data) {
			DrawerLayerTreeItem *dlti = dynamic_cast<DrawerLayerTreeItem *>(data);
			if (dlti) {
				if (dlti->drw() == drwFind)
					return hti;
			} else {
				DisplayOptionTreeItem *doti = dynamic_cast<DisplayOptionTreeItem *>(data);
				if (doti) {
					if ( doti->drw() == drwFind)
						return hti;
				}
			}
		}
		hti= tree->GetTreeCtrl().GetNextItem(hti, TVGN_NEXT);
	}
	return 0;
}

void SpaceTimeCube::startLayerOptionsForm()
{
	update(true);
	CRect rect;
	bool fRestorePosition = false;
	if (layerOptionsForm) {
		fRestorePosition = true;
		layerOptionsForm->GetWindowRect(&rect);
		delete layerOptionsForm;
	}
	layerOptionsForm = new LayerOptionsForm(tree, *this, layerList);
	if (fRestorePosition)
		layerOptionsForm->SetWindowPos(tree, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOREPOSITION);
}

void SpaceTimeCube::setFormAutoDeleted()
{
	layerOptionsForm = 0;
}

bool SpaceTimeCube::showingLayerOptionsForm()
{
	return layerOptionsForm != 0;
}

void SpaceTimeCube::SetTime(ILWIS::Time time, long sender) {
	bool fShiftDown = GetKeyState(VK_SHIFT) & 0x8000 ? true : false;
	double timePosNew = timeBoundsZoom.fValid() ? (time - timeBoundsZoom.tMin()) / (timeBoundsZoom.tMax() - timeBoundsZoom.tMin()) : time;
	if (timePosNew >= 0 && timePosNew <= 1) {
		if (fShiftDown)
			timeShift += timePos - timePosNew;
		else 
			timeOffset += timePosNew - timePos;
		timePos = timePosNew;
		if (timeBoundsZoom.tMin().isValid() && timeBoundsZoom.tMax().isValid()) {
			Time tPos (timeBoundsZoom.tMin() + (Time)((timeBoundsZoom.tMax() - timeBoundsZoom.tMin()) * timePos));
			sTimePosText = tPos.toString();
		} else
			sTimePosText = "";
	}

	mpv->Invalidate();
}

TimeBounds & SpaceTimeCube::getTimeBoundsZoom()
{
	return timeBoundsZoom;
}

const TimeBounds & SpaceTimeCube::getTimeBoundsFullExtent() const
{
	return timeBoundsFullExtent;
}

//------------------------------------------------------

LayerOptionsForm::LayerOptionsForm(CWnd *wPar, SpaceTimeCube & _spaceTimeCube, vector<LayerData> & layerList)
: DisplayOptionsForm(0, wPar, TR("Select Layer Options"))
, spaceTimeCube(_spaceTimeCube)
, m_layerList(layerList)
, fFirstTime(true)
{
	if (layerList.size() == 0) {
		new StaticText(root, "None of the opened layers has a Time attribute");
	} else {
		StaticText * stPrevious;
		int nrLayers = layerList.size();
		vsPlotMethod.resize(nrLayers);
		vsTimeColumnNames.resize(nrLayers);
		vsTime2ColumnNames.resize(nrLayers);
		vsSortColumnNames.resize(nrLayers);
		vsGroupColumnNames.resize(nrLayers);
		vsSizeColumnNames.resize(nrLayers);
		vsAngleColumnNames.resize(nrLayers);
		vsSpeedColumnNames.resize(nrLayers);
		vbTime2.resize(nrLayers);
		vbSort.resize(nrLayers);
		vbGroup.resize(nrLayers);
		vbSize.resize(nrLayers);
		for(int i=0; i < nrLayers; ++i) {
			LayerData & layerData = layerList[i];
			StaticText * stLayerName = new StaticText(root, layerData.fnObj().sFile);
			if (i > 0)
				stLayerName->Align(stPrevious, AL_UNDER);
			stPrevious = stLayerName;
			vsPlotMethod[i] = layerData.sPlotOption();
			FieldOneSelectTextOnly* fosPM = new FieldOneSelectTextOnly(root, &vsPlotMethod[i], false);
			//fosPM->SetWidth(100);
			fosPM->SetCallBack((NotifyProc)&LayerOptionsForm::ComboCallBackFunc);
			fosPM->Align(stLayerName, AL_AFTER);
			fosPlotMethod.push_back(fosPM);
			vbTime2[i] = new bool;
			vbSort[i] = new bool;
			vbGroup[i] = new bool;
			vbSize[i] = new bool;
			*vbTime2[i] = layerData.fUseTime2();
			*vbSort[i] = layerData.fUseSort();
			*vbGroup[i] = layerData.fUseGroup();
			*vbSize[i] = layerData.fUseSize();
			FormEntry * feTime;
			if (layerData.isSelfTime()) {
				StaticText *stDummy = new StaticText(root, "");
				feTime = stDummy;
				fcTimeColumn.push_back(0);
			}
			else {
				if (layerData.getTimeColumn().fValid())
					vsTimeColumnNames[i] = layerData.getTimeColumn()->sName();
				FieldColumn *fcol = new FieldColumn(root, "time", layerData.getAttTable(), &vsTimeColumnNames[i], dmTIME);
				fcTimeColumn.push_back(fcol);
				feTime = fcol;
			}
			feTime->Align(fosPM, AL_AFTER);
			FormEntry * feTime21;
			FormEntry * feTime22;
			if (layerData.isSelfTime()) {
				feTime21 = new StaticText(root, "");
				feTime22 = new StaticText(root, "");
				feTime22->Align(feTime21, AL_AFTER);
				cbTime2.push_back(0);
				fcTimeColumn2.push_back(0);
			} else {
				CheckBox * cbtime2 = new CheckBox(root, "end time", vbTime2[i]);
				cbTime2.push_back(cbtime2);
				if (layerData.getTimeColumn2().fValid())
					vsTime2ColumnNames[i] = layerData.getTimeColumn2()->sName();
				FieldColumn * fcol = new FieldColumn(cbtime2, "", layerData.getAttTable(), &vsTime2ColumnNames[i], dmTIME);
				fcTimeColumn2.push_back(fcol);
				fcol->Align(cbtime2, AL_AFTER);
				feTime21 = cbtime2;
				feTime22 = fcol;
			}
			feTime21->Align(feTime, AL_AFTER);
			FormEntry * feSort1;
			FormEntry * feSort2;
			if (!layerData.hasSort()) {
				feSort1 = new StaticText(root, "");
				feSort2 = new StaticText(root, "");
				feSort2->Align(feSort1, AL_AFTER);
				cbSort.push_back(0);
				fcSortColumn.push_back(0);
			} else {
				CheckBox * cbsort = new CheckBox(root, "sort by", vbSort[i]);
				cbSort.push_back(cbsort);
				if (layerData.getSortColumn().fValid())
					vsSortColumnNames[i] = layerData.getSortColumn()->sName();
				FieldColumn * fcol = new FieldColumn(cbsort, "", layerData.getAttTable(), &vsSortColumnNames[i], dmVALUE | dmCLASS | dmIDENT);
				fcSortColumn.push_back(fcol);
				fcol->Align(cbsort, AL_AFTER);
				feSort1 = cbsort;
				feSort2 = fcol;
			}
			feSort1->Align(feTime, AL_AFTER);
			FormEntry * feGroup1;
			FormEntry * feGroup2;
			if (!layerData.hasGroup()) {
				feGroup1 = new StaticText(root, "");
				feGroup2 = new StaticText(root, "");
				feGroup2->Align(feGroup1, AL_AFTER);
				cbGroup.push_back(0);
				fcGroupColumn.push_back(0);
			} else {
				CheckBox * cbgroup = new CheckBox(root, "group by", vbGroup[i]);
				cbGroup.push_back(cbgroup);
				if (layerData.getGroupColumn().fValid())
					vsGroupColumnNames[i] = layerData.getGroupColumn()->sName();
				FieldColumn * fcol = new FieldColumn(cbgroup, "", layerData.getAttTable(), &vsGroupColumnNames[i], dmVALUE | dmCLASS | dmSTRING);
				fcGroupColumn.push_back(fcol);
				fcol->Align(cbgroup, AL_AFTER);
				feGroup1 = cbgroup;
				feGroup2 = fcol;
			}
			feGroup1->Align(feSort2, AL_AFTER);
			FormEntry * feSize1;
			FormEntry * feSize2;
			if (!layerData.hasSize()) {
				feSize1 = new StaticText(root, "");
				feSize2 = new StaticText(root, "");
				feSize2->Align(feSize1, AL_AFTER);
				cbSize.push_back(0);
				fcSizeColumn.push_back(0);
			}
			else {
				CheckBox * cbsize = new CheckBox(root, "size", vbSize[i]);
				cbSize.push_back(cbsize);
				if (layerData.getSizeColumn().fValid())
					vsSizeColumnNames[i] = layerData.getSizeColumn()->sName();
				FieldColumn *fcol = new FieldColumn(cbsize, "", layerData.getAttTable(), &vsSizeColumnNames[i], dmVALUE);
				fcSizeColumn.push_back(fcol);
				fcol->Align(cbsize, AL_AFTER);
				feSize1 = cbsize;
				feSize2 = fcol;
			}
			feSize1->Align(feGroup2, AL_AFTER);
			if (!layerData.hasSize()) {
				fcAngleColumn.push_back(0);
				fcSpeedColumn.push_back(0);
			} else {
				if (layerData.getSize2Column().fValid())
					vsAngleColumnNames[i] = layerData.getSize2Column()->sName();
				FieldColumn *fcol1 = new FieldColumn(root, "angle", layerData.getAttTable(), &vsAngleColumnNames[i], dmVALUE);
				fcAngleColumn.push_back(fcol1);				
				if (layerData.getSizeColumn().fValid())
					vsSpeedColumnNames[i] = layerData.getSizeColumn()->sName();
				FieldColumn *fcol2 = new FieldColumn(root, "speed", layerData.getAttTable(), &vsSpeedColumnNames[i], dmVALUE);
				fcSpeedColumn.push_back(fcol2);
				fcol2->Align(fcol1, AL_AFTER);
				feSize1 = fcol1;
				feSize2 = fcol2;
			}
			feSize1->Align(feGroup2, AL_AFTER);
		}
	}

	create();
}

LayerOptionsForm::~LayerOptionsForm()
{
	for (int i = 0; i < vbSort.size(); ++i)
		delete vbSort[i];
	for (int i = 0; i < vbGroup.size(); ++i)
		delete vbGroup[i];
	for (int i = 0; i < vbSize.size(); ++i)
		delete vbSize[i];
	for (int i = 0; i < vbTime2.size(); ++i)
		delete vbTime2[i];
}

int LayerOptionsForm::ComboCallBackFunc(Event*)
{
	if (fFirstTime)
	{
		fFirstTime = false;
		for(int i=0; i < m_layerList.size(); ++i) {
			LayerData & layerData = m_layerList[i];
			fosPlotMethod[i]->AddString("<regular>");
			if (layerData.isPointMap()) {
				fosPlotMethod[i]->AddString("<stp>");
				fosPlotMethod[i]->AddString("<stations>");
				fosPlotMethod[i]->AddString("<rose>");
			}
			fosPlotMethod[i]->SelectItem(vsPlotMethod[i]);
		}
	}

	for (int i=0; i < m_layerList.size(); ++i) {
		if (fosPlotMethod[i]->sGetText() == "<regular>") {
			if (fcTimeColumn[i]) fcTimeColumn[i]->Hide();
			if (cbTime2[i]) cbTime2[i]->Hide();
			if (cbSort[i]) cbSort[i]->Hide();
			if (cbGroup[i]) cbGroup[i]->Hide();
			if (cbSize[i]) cbSize[i]->Hide();
			if (fcAngleColumn[i]) fcAngleColumn[i]->Hide();
			if (fcSpeedColumn[i]) fcSpeedColumn[i]->Hide();
		} else if (fosPlotMethod[i]->sGetText() == "<stp>") {
			if (fcTimeColumn[i]) fcTimeColumn[i]->Show();
			if (cbTime2[i]) cbTime2[i]->Hide();
			if (cbSort[i]) cbSort[i]->Show();
			if (cbGroup[i]) cbGroup[i]->Show();
			if (cbSize[i]) cbSize[i]->Show();
			if (fcAngleColumn[i]) fcAngleColumn[i]->Hide();
			if (fcSpeedColumn[i]) fcSpeedColumn[i]->Hide();
		} else if (fosPlotMethod[i]->sGetText() == "<stations>") {
			if (fcTimeColumn[i]) fcTimeColumn[i]->Show();
			if (cbTime2[i]) cbTime2[i]->Show();
			if (cbSort[i]) cbSort[i]->Hide();
			if (cbGroup[i]) cbGroup[i]->Hide();
			if (cbSize[i]) cbSize[i]->Show();
			if (fcAngleColumn[i]) fcAngleColumn[i]->Hide();
			if (fcSpeedColumn[i]) fcSpeedColumn[i]->Hide();
		} else if (fosPlotMethod[i]->sGetText() == "<rose>") {
			if (fcTimeColumn[i]) fcTimeColumn[i]->Show();
			if (cbTime2[i]) cbTime2[i]->Hide();
			if (cbSort[i]) cbSort[i]->Show();
			if (cbGroup[i]) cbGroup[i]->Show();
			if (cbSize[i]) cbSize[i]->Hide();
			if (fcAngleColumn[i]) fcAngleColumn[i]->Show();
			if (fcSpeedColumn[i]) fcSpeedColumn[i]->Show();
		}
	}

	return 0;
}

void LayerOptionsForm::apply() {

	for(int i=0; i < m_layerList.size(); ++i) {
		LayerData & layerData = m_layerList[i];
		if (fcTimeColumn[i] && !layerData.isSelfTime()) {
			fcTimeColumn[i]->StoreData();
			layerData.setTimeColumn(vsTimeColumnNames[i]);
		}
		if (cbTime2[i]) {
			cbTime2[i]->StoreData();
			layerData.setUseTime2(*vbTime2[i]);
			if (*vbTime2[i]) {
				fcTimeColumn2[i]->StoreData();
				layerData.setTimeColumn2(vsTime2ColumnNames[i]);
			}
		}
		if (cbSort[i]) {
			cbSort[i]->StoreData();
			layerData.setUseSort(*vbSort[i]);
			if (*vbSort[i]) {
				fcSortColumn[i]->StoreData();
				layerData.setSortColumn(vsSortColumnNames[i]);
			}
		}
		if (cbGroup[i]) {
			cbGroup[i]->StoreData();
			layerData.setUseGroup(*vbGroup[i]);
			if (*vbGroup[i]) {
				fcGroupColumn[i]->StoreData();
				layerData.setGroupColumn(vsGroupColumnNames[i]);
			}
		}
		if (fosPlotMethod[i]->sGetText() == "<rose>") {
			layerData.setUseSize(true);
			fcSpeedColumn[i]->StoreData();
			layerData.setSizeColumn(vsSpeedColumnNames[i]);
			fcAngleColumn[i]->StoreData();
			layerData.setSize2Column(vsAngleColumnNames[i]);
		} else if (cbSize[i]) {
			cbSize[i]->StoreData();
			layerData.setUseSize(*vbSize[i]);
			if (*vbSize[i]) {
				fcSizeColumn[i]->StoreData();
				layerData.setSizeColumn(vsSizeColumnNames[i]);
			}
		}
		fosPlotMethod[i]->StoreData();
		layerData.setPlotOption(vsPlotMethod[i]);
	}

	spaceTimeCube.refreshDrawerList(true);

	//PreparationParameters pp(NewDrawer::ptGEOMETRY, 0);
	//drw->prepareChildDrawers(&pp);
	//updateMapView();

	
	//PreparationParameters pp(NewDrawer::ptRENDER, 0);
	//drw->prepareChildDrawers(&pp);
	//updateMapView();
}

void LayerOptionsForm::OnOK() {
	spaceTimeCube.setFormAutoDeleted();
	DisplayOptionsForm::OnOK();
}

void LayerOptionsForm::OnCancel() {
	spaceTimeCube.setFormAutoDeleted();
	DisplayOptionsForm::OnCancel();
}

//------------------------------------------------------

DrawerTool *createSpaceTimeCubeTool(ZoomableView* zv, LayerTreeView *view, NewDrawer *drw) {
	return new SpaceTimeCubeTool(zv, view, drw);
}

SpaceTimeCubeTool::SpaceTimeCubeTool(ZoomableView* zv, LayerTreeView *view, NewDrawer *drw)
: DrawerTool("SpaceTimeCubeTool",zv, view, drw)
, stc(SpaceTimeCube::getSpaceTimeCube(zv, view, drw))
, htiElements(0)
{
}

SpaceTimeCubeTool::~SpaceTimeCubeTool() {
	if (!IsWindow(tree->GetTreeCtrl().m_hWnd)) { // Clean-up if the user closed the mapwindow.
		SpaceTimeCube::deleteSpaceTimeCube(mpv);
	}
}

bool SpaceTimeCubeTool::isToolUseableFor(ILWIS::DrawerTool *tool) { 
	bool ok =  dynamic_cast<ThreeDGlobalTool *>(tool) != 0;
	if ( ok) {
		parentTool = tool;
		bool fUseSpaceTimeCube = 0 != drawer->getRootDrawer()->getDrawer("CubeDrawer");
		if (fUseSpaceTimeCube) {
			stc->loadMapview();
			if (!stc->fUseSpaceTimeCube())
				stc->setUseSpaceTimeCube(true);
		}
	}
	return ok;
}

SpaceTimeCube * SpaceTimeCubeTool::getSpaceTimeCube() const
{
	return stc;
}

HTREEITEM SpaceTimeCubeTool::configure( HTREEITEM parentItem) {
	DisplayOptionTreeItem *item = new DisplayOptionTreeItem(tree,parentItem,drawer);
	item->setCheckAction(this, 0,(DTSetCheckFunc)&SpaceTimeCubeTool::setUseSpaceTimeCube);
	item->setDoubleCickAction(this, (DTDoubleClickActionFunc)&SpaceTimeCubeTool::startLayerOptionsForm);
	htiNode =  insertItem(TR("SpaceTimeCube"),"SpaceTimeCube",item,stc->fUseSpaceTimeCube());
	DrawerTool::configure(htiNode);
	if (stc->fUseSpaceTimeCube()) {
		stc->refreshDrawerList(false);
		addTools();		
	}
	if (stc->showingLayerOptionsForm())
		stc->startLayerOptionsForm();
	return htiNode;
}

void SpaceTimeCubeTool::makeActive(void *v, HTREEITEM) {
	bool yesno = *(bool*)v;
}

String SpaceTimeCubeTool::getMenuString() const {
	return TR("SpaceTimeCube");
}

void SpaceTimeCubeTool::startLayerOptionsForm() {
	stc->startLayerOptionsForm();
}

void SpaceTimeCubeTool::addTools()
{
	// NewDrawer *drw = drawer->getRootDrawer()->getDrawer("CubeDrawer");
	// drawer = drw; // NO!! drawer must be RootDrawer. Those tools outlive the CubeDrawer. Inside the tools, RootDrawer is used to find the applicable drawer
	TimeZoomTool * tzTool = new TimeZoomTool(mpvGetView(), getDocument()->ltvGetView(), drawer);
	addTool(tzTool);
	htiElements.push_back(tzTool->configure(htiNode));
	CubeElementsTool * ceTool = new CubeElementsTool(mpvGetView(), getDocument()->ltvGetView(), drawer);
	addTool(ceTool);
	htiElements.push_back(ceTool->configure(htiNode));
}

void SpaceTimeCubeTool::removeTools()
{
	for (vector<HTREEITEM>::iterator it = htiElements.begin(); it != htiElements.end(); ++it)
		tree->GetTreeCtrl().DeleteItem(*it);
	htiElements.clear();
	removeTool(0);
}

void SpaceTimeCubeTool::setUseSpaceTimeCube(void *v, HTREEITEM) {
	bool useSpaceTimeCube = *(bool *)v;
	stc->setUseSpaceTimeCube(useSpaceTimeCube);
	if (useSpaceTimeCube) {
		addTools();
	} else {
		removeTools();
	}
}

