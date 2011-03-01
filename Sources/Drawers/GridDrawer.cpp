#include "Client\Headers\formelementspch.h"
#include "Client\FormElements\syscolor.h"
#include "Client\FormElements\FieldIntSlider.h"
#include "Engine\Base\Round.h"
#include "Engine\Map\basemap.h"
#include "Client\Mapwindow\MapPaneView.h"
#include "Engine\Map\Point\ilwPoint.h"
#include "Client\Mapwindow\Drawers\DrawerContext.h"
#include "Client\Mapwindow\Drawers\ComplexDrawer.h"
#include "Client\Mapwindow\Drawers\SimpleDrawer.h" 
#include "Client\Ilwis.h"
#include "Engine\Base\System\RegistrySettings.h"
#include "Client\Mapwindow\MapCompositionDoc.h"
#include "Client\Mapwindow\Drawers\RootDrawer.h"
#include "Client\Mapwindow\MapPaneViewTool.h"
#include "Client\Mapwindow\Drawers\DrawerTool.h"
#include "Client\Mapwindow\LayerTreeView.h"
#include "Client\Mapwindow\LayerTreeItem.h" 
#include "Client\Editors\Utils\line.h"
#include "Drawers\LineDrawer.h"
#include "Drawers\GridDrawer.h"
#include "Client\FormElements\fldcolor.h"
#include "Client\Mapwindow\Drawers\ZValueMaker.h"
#include "Headers\Hs\Drwforms.hs"

using namespace ILWIS;

ILWIS::NewDrawer *createGridDrawer(DrawerParameters *parms) {
	return new GridDrawer(parms);
}

GridDrawer::GridDrawer(DrawerParameters *parms) : 
ComplexDrawer(parms,"GridDrawer")
{
	setDrawMethod(drmSINGLE); // default;
	id = name = "GridDrawer";
	rDist = rUNDEF;
	setActive(false);
	setTransparency(0.2);
	threeD = false;
	lproperties.thickness = 1;
	lproperties.drawColor = Color(0,0,0);
	planeColor = Color(210,210,255);
	lproperties.linestyle = 0xFFFF;
	noOfPlanes = 4;
	zdist = rUNDEF;
	maxz = rUNDEF;
	mode = GridDrawer::mGRID | mVERTICALS | mAXIS;
}

GridDrawer::~GridDrawer() {
	for(int i = 0; i < planeQuads.size(); ++i) {
		planeQuads[i] = new Coord[4];
	}
}


bool GridDrawer::draw(bool norecursion, const CoordBounds& cbArea) const{
	if ( !isActive())
		return false;
	ComplexDrawer::draw(norecursion, cbArea);
	if ( mode & mPLANE) {
		drawPlane(norecursion, cbArea);
	}
	return true;
}

bool GridDrawer::drawPlane(bool norecursion, const CoordBounds& cbArea) const{
	glClearColor(1.0,1.0,1.0,0.0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	ComplexDrawer *cdrw = (ComplexDrawer *)getParentDrawer();
	ZValueMaker *zmaker = cdrw->getZMaker();

	glColor4f(planeColor.redP(),planeColor.greenP(), planeColor.blueP(), getTransparency());

	double zscale = zmaker->getZScale();
	double zoffset = zmaker->getOffset();
	glPushMatrix();
	glScaled(1,1,zscale);
	glTranslated(0,0,zoffset);

	for(int i = 0; i < planeQuads.size(); ++i) {
		glBegin(GL_QUADS);
			Coord c1 = planeQuads[i][0];
			Coord c2 = planeQuads[i][1];
			Coord c3 = planeQuads[i][2];
			Coord c4 = planeQuads[i][3];

			glVertex3f( c1.x, c1.y, c1.z);	
			glVertex3f( c2.x, c2.y, c2.z);	
			glVertex3f( c3.x, c3.y, c3.z);
			glVertex3f( c4.x, c4.y, c4.z);
		glEnd();
	}
	glPopMatrix();
	return true;
}

void GridDrawer::prepare(PreparationParameters *pp) {
	if (  pp->type & RootDrawer::ptGEOMETRY){ 
		String sVal;
		Coord c, cMin, cMax;
		Color clr;
		clear();
		getZMaker()->setThreeDPossible(true);
		CoordBounds cbMap = getRootDrawer()->getMapCoordBounds();
		cMin = cbMap.cMin;
		cMax = cbMap.cMax;
		if ( maxz == rUNDEF)
			maxz = min(cbMap.width(), cbMap.height()) / 2.0;
		if (zdist == rUNDEF) {
			zdist = rRound(maxz / noOfPlanes);
			maxz = zdist * noOfPlanes;
		}
		if ( rDist == rUNDEF)
			rDist = rRound((cMax.x - cMin.x) / 7);

		Coord c1, c2;
	
		if ( mode & GridDrawer::mGRID)
			prepareGrid(rDist,cMax, cMin);
		if ( threeD) {
			if ( mode & GridDrawer::mPLANE)
				preparePlanes(rDist,cMax, cMin);
			if (mode & GridDrawer::mAXIS)
				prepareVAxis(rDist,cMax, cMin);
			if ( mode & GridDrawer::mVERTICALS)
				prepareVerticals(rDist, cMax, cMin);
			if ( mode & GridDrawer::mCUBE)
				prepareCube(rDist, cMax, cMin);
		}
	}
	if ( pp->type & NewDrawer::ptRENDER) {
		for(int i=0; i < drawers.size(); ++i) {
			LineDrawer *ld = (LineDrawer *)drawers.at(i);
			((LineProperties *)ld->getProperties())->drawColor = lproperties.drawColor;
			((LineProperties *)ld->getProperties())->linestyle = lproperties.linestyle;
			((LineProperties *)ld->getProperties())->thickness = lproperties.thickness;
			ld->setTransparency(getTransparency());
		}
	}
}

void GridDrawer::prepareVAxis(double rDist,const Coord& cMax, const Coord& cMin) {
	Coord c1, c2, oldc2,startc2;
	c1 = cMin;
	c1.z  = getRootDrawer()->getFakeZ();
	c2 = c1;
	c2.z = maxz;
	startc2 = oldc2 = c2;
	AddGridLine(c1,c2);
	c1.x = cMin.x;
	c1.y = cMax.y;
	c1.z  = getRootDrawer()->getFakeZ();
	c2 = c1;
	c2.z = maxz;
	AddGridLine(oldc2,c2);
	oldc2 = c2;
	AddGridLine(c1,c2);
	c1 = cMax;
	c1.z  = getRootDrawer()->getFakeZ();
	c2 = c1;
	c2.z = maxz;
	AddGridLine(oldc2,c2);
	oldc2 = c2;
	AddGridLine(c1,c2);
	c1.x = cMax.x;
	c1.y = cMin.y;
	c1.z  = getRootDrawer()->getFakeZ();
	c2 = c1;
	c2.z = maxz;
	AddGridLine(oldc2,c2);
	oldc2 = c2;
	AddGridLine(c1,c2);
	AddGridLine(oldc2, startc2);
	double z = maxz;


	

}
void GridDrawer::prepareVerticals(double rDist,const Coord& cMax, const Coord& cMin) {
	Coord c1, c2;
	for (double x = ceil(cMin.x / rDist) * rDist; x < cMax.x ; x += rDist)
	{
		for (double y = ceil(cMin.y / rDist) * rDist; y < cMax.y ; y += rDist)
		{
			c1.x = x;
			c1.y = y;
			c1.z =  getRootDrawer()->getFakeZ();
			c2.x = x;
			c2.y = y;
			c2.z = maxz;
			AddGridLine(c1, c2);
		}
	}
}
void GridDrawer::preparePlanes(double rDist, const Coord& cMax, const Coord& cMin ) {
	Coord c1, c2;
	double z = 	getRootDrawer()->getFakeZ();
	int zplanes = threeD ? 0.5 + maxz / zdist : 0;
	resizeQuadsVector(zplanes);
	for(int i=0; i <= zplanes; ++i) {
		c1.z = c2.z = z;
		c1.y = cMin.y;
		c2.y = cMax.y;
		planeQuads[i][0] = Coord(cMin.x,cMin.y,z);
		planeQuads[i][1] = Coord(cMin.x,cMax.y,z);
		planeQuads[i][2] = Coord(cMax.x,cMax.y,z);
		planeQuads[i][3] = Coord(cMax.x,cMin.y,z);
	
		z += zdist;
	}
	maxz = z - zdist;
		
}

void GridDrawer::prepareCube(double rDist, const Coord& cMax, const Coord& cMin ) {
	Coord c1, c2;
	double z = 	getRootDrawer()->getFakeZ();

	c1 = cMin;
	c1.z = c2.z = z;
	c2.x = cMin.x;
	c2.y = cMax.y;
	AddGridLine(c1, c2);
	c1 = c2;
	c2.x = cMax.x;
	c2.y = cMax.y;
	AddGridLine(c1, c2);
	c1 = c2;
	c2.x = cMax.x;
	c2.y = cMin.y;
	AddGridLine(c1, c2);
	c1 = c2;
	c2.x = cMin.x;
	c2.y = cMin.y;
	AddGridLine(c1, c2);
	c1 = cMin;
	c1.z = c2.z = maxz;
	c2.x = cMin.x;
	c2.y = cMax.y;
	AddGridLine(c1, c2);
	c1 = c2;
	c2.x = cMax.x;
	c2.y = cMax.y;
	AddGridLine(c1, c2);
	c1 = c2;
	c2.x = cMax.x;
	c2.y = cMin.y;
	AddGridLine(c1, c2);
	c1 = c2;
	c2.x = cMin.x;
	c2.y = cMin.y;
	AddGridLine(c1, c2);
	c1 = c2 = cMin;
	c1.z = z;
	c2.z = maxz;
	AddGridLine(c1, c2);
	c1 = c2 = Coord(cMin.x, cMax.y);
	c1.z = z;
	c2.z = maxz;
	AddGridLine(c1, c2);
	c1 = c2 = Coord(cMax.x, cMax.y);
	c1.z = z;
	c2.z = maxz;
	AddGridLine(c1, c2);
	c1 = c2 = Coord(cMax.x, cMin.y);
	c1.z = z;
	c2.z = maxz;
	AddGridLine(c1, c2);
}

void GridDrawer::prepareGrid(double rDist, const Coord& cMax, const Coord& cMin ) {
	Coord c1, c2;
	double z = 	getRootDrawer()->getFakeZ();
	int zplanes = threeD ? 0.5 + maxz / zdist : 0;
	resizeQuadsVector(zplanes);
	for(int i=0; i <= zplanes; ++i) {
		c1.z = c2.z = z;
		c1.y = cMin.y;
		c2.y = cMax.y;
		for (double x = ceil(cMin.x / rDist) * rDist; x < cMax.x ; x += rDist)
		{
			c1.x = c2.x = x;
			AddGridLine(c1, c2);
		}

		c1.x = cMin.x;
		c2.x = cMax.x;
		for (double y = ceil(cMin.y / rDist) * rDist; y < cMax.y ; y += rDist)
		{
			c1.y = c2.y = y;
			AddGridLine(c1, c2);
		}
		if ( threeD && (mode & GridDrawer::mGRID)) {
			AddGridLine(Coord(cMin.x,cMin.y,z), Coord(cMin.x, cMax.y,z));
			AddGridLine(Coord(cMin.x,cMax.y,z), Coord(cMax.x, cMax.y,z));
			AddGridLine(Coord(cMax.x,cMax.y,z), Coord(cMax.x, cMin.y,z));
			AddGridLine(Coord(cMax.x,cMin.y,z), Coord(cMin.x, cMin.y,z));
		}
		z += zdist;
	}
}

void GridDrawer::resizeQuadsVector(int planes) {
	if ( planes == 0)
		return;

	for(int i = 0; i < planeQuads.size(); ++i) {
		delete [] planeQuads[i];
	}
	planeQuads.resize(planes + 1);
	for(int i = 0; i < planes + 1; ++i) {
		planeQuads[i] = new Coord[4];
	}
}

void GridDrawer::AddGridLine(Coord c1, Coord c2)
{
	ILWIS::DrawerParameters dp(getRootDrawer(), this);
	PreparationParameters pp(NewDrawer::ptGEOMETRY);
	GridLine *line = (GridLine *)IlwWinApp()->getDrawer("GridLine", &pp, &dp);
	line->addDataSource(&c1);
	line->addDataSource(&c2);
	((LineProperties *)line->getProperties())->drawColor = lproperties.drawColor;
	addDrawer(line);
}

//void GridDrawer::DrawCurvedLine(Coord c1, Coord c2)
//{
//	Coord cStep, crd;
//	int iSteps = 500;
//	cStep.x = (c2.x - c1.x) / iSteps;
//	cStep.y = (c2.y - c1.y) / iSteps;
//	p = psn->pntPos(c1);
//	if (p.x == shUNDEF || p.y == shUNDEF)
//		fPointOk = false;
//	else {
//		fPointOk = true;
//		cdc->MoveTo(p);
//	}
//	crd = c1;
//	for (int i = 0; i < iSteps; ++i) {
//		crd += cStep;
//		p = psn->pntPos(crd);
//		if (p.x == shUNDEF || p.y == shUNDEF)
//			fPointOk = false;
//		else if (fPointOk) 
//			cdc->LineTo(p);
//		else {
//			fPointOk = true;
//			cdc->MoveTo(p);
//		}
//	}
//}

String GridDrawer::store(const FileName& fnView, const String& parentSection) const{
	ComplexDrawer::store(fnView, getType());

	return getType();
}

void GridDrawer::load(const FileName& fnView, const String& parenSection){
}


double GridDrawer::getGridSpacing() const{
	return rDist;
}
void GridDrawer::setGridSpacing(double d){
	rDist = d;
}

GeneralDrawerProperties *GridDrawer::getProperties() {
	return &lproperties;
}

int GridDrawer::getMode() const{
	return mode;
}

void GridDrawer::setMode(int m) {
	mode = m;
}

double GridDrawer::getZSpacing() const{
	return zdist;
}
void GridDrawer::setZSpacing(double z){
	zdist = z;
	maxz = zdist * noOfPlanes;
}

Color GridDrawer::getPlaneColor() const {
	return planeColor;
}
void GridDrawer::setPlaneColor(Color clr){
	planeColor = clr;
}

void GridDrawer::set3D(bool yesno){
	threeD = yesno;
}

//---------------------------------------------
ILWIS::NewDrawer *createGridLine(DrawerParameters *parms) {
	return new GridLine(parms);
}

GridLine::GridLine(DrawerParameters *parms) : LineDrawer(parms,"GridLine"){
}

GridLine::~GridLine(){
}

bool GridLine::draw(bool norecursion, const CoordBounds& cbArea) const{
	return LineDrawer::draw(norecursion, cbArea);
}

void GridLine::prepare(PreparationParameters *pp){
	LineDrawer::prepare(pp);
	if ( pp->type & NewDrawer::ptGEOMETRY)
		clear();

}

void GridLine::addDataSource(void *crd, int options) {
	Coord c = *((Coord *)crd);
	cb += c;
	if (lines.size() == 0)
		lines.push_back(new CoordinateArraySequence());
	lines.at(0)->add(c);
}



