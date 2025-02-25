#include "Headers\toolspch.h"
#include "Engine\Map\basemap.h"
#include "Engine\Drawers\DrawerContext.h"
#include "Engine\Drawers\RootDrawer.h"
#include "Engine\Drawers\SimpleDrawer.h" 
#include "Boxdrawer.h"
#include "Engine\Drawers\ComplexDrawer.h"
#include "Engine\Drawers\ZValueMaker.h"

using namespace ILWIS;

ILWIS::NewDrawer *createBoxDrawer(DrawerParameters *parms) {
	return new BoxDrawer(parms);
}

BoxDrawer::BoxDrawer(DrawerParameters *parms) : 
SimpleDrawer(parms,"BoxDrawer"),
drawColor(Color(30,30,30,30))
{
	id = String("%d",BOX_DRAWER_ID);
}

BoxDrawer::~BoxDrawer() {
	clear();
}

void BoxDrawer::clear() {
}

void BoxDrawer::setBox(const CoordBounds& cbOuter, const CoordBounds& cbInner) {
	if ( cbInner.fUndef()) {
		boxes.push_back(cbOuter);
	}
	else {
		boxes.clear();
		CoordBounds cb1(Coord(cbOuter.MinX(), cbOuter.MinY()), Coord(cbInner.MinX(), cbOuter.MaxY()));
		CoordBounds cb2(Coord(cbInner.MinX(), cbOuter.MaxY()), Coord(cbInner.MaxX(), cbInner.MaxY()));
		CoordBounds cb3(Coord(cbInner.MaxX(), cbOuter.MaxY()), Coord(cbOuter.MaxX(), cbOuter.MinY()));
		CoordBounds cb4(Coord(cbInner.MaxX(), cbInner.MinY()), Coord(cbInner.MinX(), cbOuter.MinY()));
		boxes.push_back(cb1);
		boxes.push_back(cb2);
		boxes.push_back(cb3);
		boxes.push_back(cb4);
	}

}

bool BoxDrawer::draw(const DrawLoop drawLoop, const CoordBounds& cbArea) const{
	if ((drawLoop == drl3DOPAQUE && drawColor.alpha() < 255) || (drawLoop == drl3DTRANSPARENT && drawColor.alpha() == 255))
		return false;
	if (boxes.size() == 0)
		return false;
	bool is3D = getRootDrawer()->is3D();
	if (is3D) // draw this box always at level 1 (this box is used in HighlightTool.cpp and AnnotationDrawers.cpp)
		glDepthRange(0.01 - (getRootDrawer()->getZIndex() + 1) * 0.0005, 1.0 - (getRootDrawer()->getZIndex() + 1) * 0.0005);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glColor4d(drawColor.redP(), drawColor.greenP(), drawColor.blueP(), drawColor.alphaP());
	for(int i=0; i< boxes.size(); ++i) {
		Coord c1 = boxes[i].cMin;
		Coord c2 = boxes[i].cMax;
		glBegin(GL_QUADS);
		glVertex3d(c1.x, c1.y,0);
		glVertex3d(c1.x, c2.y,0);
		glVertex3d(c2.x, c2.y,0);
		glVertex3d(c2.x, c1.y,0);
		glVertex3d(c1.x, c1.y,0);
		glEnd();
	}
	glDisable(GL_BLEND);

	return true;
}


void BoxDrawer::prepare(PreparationParameters *p){
	SimpleDrawer::prepare(p);
	((ComplexDrawer *)parentDrawer)->getZMaker()->setZOrder(getRootDrawer()->getZIndex());
}

void BoxDrawer::setDrawColor(const Color& col) {
	drawColor = col;
}








