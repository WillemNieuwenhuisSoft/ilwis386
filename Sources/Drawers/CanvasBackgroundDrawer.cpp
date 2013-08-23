#include "Headers\toolspch.h"
#include "Engine\Map\basemap.h"
#include "Engine\Base\System\RegistrySettings.h"
#include "Engine\Drawers\DrawerContext.h"
#include "Engine\Drawers\ComplexDrawer.h"
#include "Engine\Drawers\RootDrawer.h"
#include "Engine\Drawers\SpatialDataDrawer.h"
#include "CanvasBackgroundDrawer.h"
#include "Engine\Drawers\ZValueMaker.h"


using namespace ILWIS;

ILWIS::NewDrawer *createCanvasBackgroundDrawer(DrawerParameters *parms) {
	return new CanvasBackgroundDrawer(parms);
}

CanvasBackgroundDrawer::CanvasBackgroundDrawer(DrawerParameters *parms) : ComplexDrawer(parms,"CanvasBackgroundDrawer"){
	id = name = "CanvasBackgroundDrawer";
	outside2D = Color(179,179,179);
	inside2D = Color(255,255,255);
	outside3D = Color(235,242,235);
	inside3D = Color(245,245,245);
	sky3D = Color(255,255,255);
}

CanvasBackgroundDrawer::~CanvasBackgroundDrawer() {
}
void  CanvasBackgroundDrawer::prepare(PreparationParameters *pp){
	ComplexDrawer::prepare(pp);

}

bool CanvasBackgroundDrawer::draw(const DrawLoop drawLoop, const CoordBounds& cb) const{
	if ((drawLoop == drl3DOPAQUE && transparency != 1.0) || (drawLoop == drl3DTRANSPARENT && transparency == 1.0))
		return false;

	if (!isActive() || !isValid())
		return false;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	CoordBounds cbView = getRootDrawer()->getCoordBoundsView();
	CoordBounds cbMap = getRootDrawer()->getMapCoordBounds();
	bool is3D = getRootDrawer()->is3D();
	if ( is3D) {
		glColor4d(outside3D.redP(), outside3D.greenP(), outside3D.blueP(),getTransparency());
	}
	else {
		glColor4d(outside2D.redP(), outside2D.greenP(), outside2D.blueP(),getTransparency());
	}
	double z = is3D ? -getRootDrawer()->getFakeZ() : 0;
	glBegin(GL_QUADS);						
		glVertex3f(cbView.MinX(), cbView.MinY(),z);				
		glVertex3f(cbView.MinX(), cbView.MaxY(),z);				
		glVertex3f(cbMap.MinX(), cbView.MaxY(),z);				
		glVertex3f(cbMap.MinX(), cbView.MinY(),z);
	glEnd();

	glBegin(GL_QUADS);						
		glVertex3f(cbMap.MaxX(), cbView.MinY(),z);				
		glVertex3f(cbMap.MaxX(), cbView.MaxY(),z);				
		glVertex3f(cbView.MaxX(), cbView.MaxY(),z);				
		glVertex3f(cbView.MaxX(), cbView.MinY(),z);
	glEnd();

	if ( is3D) {
		glColor4d(inside3D.redP(), inside3D.greenP(), inside3D.blueP(),getTransparency());
	}
	else {
		glColor4d(inside2D.redP(), inside2D.greenP(), inside2D.blueP(),getTransparency());
	}
	glBegin(GL_QUADS);						
		glVertex3f(cbMap.MinX(), cbMap.MinY(),z);				
		glVertex3f(cbMap.MinX(), cbMap.MaxY(),z);				
		glVertex3f(cbMap.MaxX(), cbMap.MaxY(),z);				
		glVertex3f(cbMap.MaxX(), cbMap.MinY(),z);
	glEnd();


	glDisable(GL_BLEND);
	return true;
}

String CanvasBackgroundDrawer::store(const FileName& fnView, const String& parentSection) const{
	ComplexDrawer::store(fnView, getType());
	ObjectInfo::WriteElement(getType().c_str(),"InSideColor2D",fnView, inside2D);
	ObjectInfo::WriteElement(getType().c_str(),"InSideColor3D",fnView, inside3D);
	ObjectInfo::WriteElement(getType().c_str(),"OutSideColor2D",fnView, outside2D);
	ObjectInfo::WriteElement(getType().c_str(),"OutSideColor3D",fnView, outside3D);
	ObjectInfo::WriteElement(getType().c_str(),"SkyColor3D",fnView, sky3D);
	return getType();
}

void CanvasBackgroundDrawer::load(const FileName& fnView, const String& parentSection){
	ComplexDrawer::load(fnView, getType());
	ObjectInfo::ReadElement(getType().c_str(),"InSideColor2D",fnView, inside2D);
	ObjectInfo::ReadElement(getType().c_str(),"InSideColor3D",fnView, inside3D);
	ObjectInfo::ReadElement(getType().c_str(),"OutSideColor2D",fnView, outside2D);
	ObjectInfo::ReadElement(getType().c_str(),"OutSideColor3D",fnView, outside3D);
	if (ObjectInfo::ReadElement(getType().c_str(),"SkyColor3D",fnView, sky3D))
		getRootDrawer()->SetSkyColor(sky3D);
}

Color& CanvasBackgroundDrawer::getColor(ColorLocation cl){
	switch (cl) {
		case clINSIDE2D:
			return inside2D;
		case clOUTSIDE2D:
			return outside2D;
		case clINSIDE3D:
			return inside3D;
		case clOUTSIDE3D:
			return outside3D;
		case clSKY3D:
			return sky3D;
	}
	return inside2D;
}



