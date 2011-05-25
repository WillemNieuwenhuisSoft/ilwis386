#include "Headers\toolspch.h"
#include "Engine\Map\basemap.h"
#include "Engine\Drawers\DrawerContext.h"
#include "Engine\Drawers\RootDrawer.h"
#include "Engine\Drawers\SimpleDrawer.h" 
#include "drawers\linedrawer.h"
#include "Engine\Drawers\ComplexDrawer.h"
#include "Engine\Drawers\ZValueMaker.h"

using namespace ILWIS;

ILWIS::NewDrawer *createLineDrawer(DrawerParameters *parms) {
	return new LineDrawer(parms);
}

LineDrawer::LineDrawer(DrawerParameters *parms) : 
SimpleDrawer(parms,"LineDrawer")
{
}

LineDrawer::LineDrawer(DrawerParameters *parms, const String& name) : 
SimpleDrawer(parms,name)
{
}

LineDrawer::~LineDrawer() {
	clear();
}

void LineDrawer::clear() {
	for(int i = 0; i < lines.size(); ++i) {
		delete lines.at(i);
	}
	lines.clear();
	cb = CoordBounds();
}

void LineDrawer::addCoords(const vector<Coord>& v,int options) {
	if ( options == 1){
		clear();
	}
	CoordinateSequence *seq = new CoordinateArraySequence();
	for(int i = 0; i < v.size(); ++i) {
		Coord c = v.at(i);
		seq->add(v.at(i));
		cb+= c;
	}
	lines.push_back(seq);
}

bool LineDrawer::draw( const CoordBounds& cbArea) const{
	if (lines.size() == 0)
		return false;
	if ( !getRootDrawer()->getCoordBoundsZoom().fContains(cb))
		return false;

	CoordBounds cbZoom = getRootDrawer()->getCoordBoundsZoom();

	ComplexDrawer *cdrw = (ComplexDrawer *)getParentDrawer();
	bool is3D = getRootDrawer()->is3D(); 
	bool is3DPossible = cdrw->getZMaker()->getThreeDPossible();
	double zscale, zoffset, fakez=0;
	double transp = getTransparency();

	glColor4f(lproperties.drawColor.redP(),lproperties.drawColor.greenP(), lproperties.drawColor.blueP(),transp );
	glLineWidth(lproperties.thickness);
	if (lproperties.linestyle != 0xFFFF) {
		glEnable (GL_LINE_STIPPLE);
		glLineStipple(1,lproperties.linestyle);
	}
	double z0 = cdrw->getZMaker()->getZ0(getRootDrawer()->is3D());
	if ( isSupportingDrawer && is3D) // supporting drawers need to be slightly above the level of the "main" drawer. OpenGL won't draw them correct if they are in the same plane
		z0 +=  z0;

	if ( is3D) {
		zscale = cdrw->getZMaker()->getZScale();
		zoffset = cdrw->getZMaker()->getOffset();
		glPushMatrix();
		glScaled(1,1,zscale);
		glTranslated(0,0,zoffset);
	}
	for(int j = 0; j < lines.size(); ++j) {
		CoordinateSequence *points = lines.at(j);
		if ( specialOptions & NewDrawer::sdoSELECTED || selectedCoords.size() > 0) {
			glColor4d(1, 0, 0, 1);
			drawSelectedFeature(points, cbZoom, is3D);
			glColor4f(lproperties.drawColor.redP(),lproperties.drawColor.greenP(), lproperties.drawColor.blueP(), transp);
		}
		glBegin(GL_LINE_STRIP);
		for(int i=0; i<points->size(); ++i) {
			Coordinate c = points->getAt(i);
			double z = is3D && is3DPossible ? c.z : z0;
			glVertex3d( c.x, c.y, z);
		}
		glEnd();
		if ( is3D) {
			if ( (specialOptions & NewDrawer::sdoExtrusion) != 0) {
				glColor4f(lproperties.drawColor.redP(),lproperties.drawColor.greenP(), lproperties.drawColor.blueP(), transp);
				Coord cOld;
				Coord cStart = points->getAt(0);
				for(int i=0; i<points->size(); ++i) {
					Coordinate c = points->getAt(i);
					if ( !cOld.fUndef()) {
						glColor4f(lproperties.drawColor.redP(),lproperties.drawColor.greenP(), lproperties.drawColor.blueP(), extrTransparency);
						drawExtrusion(cOld, c, z0 - zoffset, specialOptions);
						glColor4f(lproperties.drawColor.redP(),lproperties.drawColor.greenP(), lproperties.drawColor.blueP(), extrTransparency);
								}
						cOld = c;
				}
			}
		}
	}
	if ( is3D) {
		glPopMatrix();
	}
	glDisable (GL_LINE_STIPPLE);
	glLineWidth(1);
	return true;
}

void LineDrawer::drawSelectedFeature(CoordinateSequence *points, const CoordBounds& cbZoom, bool is3D) const{
	glLineWidth(lproperties.thickness * 1.2 + 3);
	glBegin(GL_LINE_STRIP);
	for(int i=0; i<points->size(); ++i) {
		Coordinate c = points->getAt(i);
		double z = is3D ? c.z : 0;
		glVertex3d( c.x, c.y, z);
	}
	glEnd();
	glLineWidth(lproperties.thickness);
	double symbolScale = cbZoom.width() / 200;
	for(int i=0; i < selectedCoords.size(); ++i) {
		Coordinate c = points->getAt(selectedCoords.at(i));
		double fz = is3D ? c.z : 0;;
		glBegin(GL_QUADS);						
			glVertex3f( c.x - symbolScale, c.y - symbolScale,fz);	
			glVertex3f( c.x - symbolScale, c.y + symbolScale,fz);	
			glVertex3f( c.x + symbolScale, c.y + symbolScale,fz);
			glVertex3f( c.x + symbolScale, c.y - symbolScale,fz);
		glEnd();
	}
	for(int i=0; i<points->size(); ++i) {
		Coordinate c = points->getAt(i);
		double fz = is3D ? c.z : 0;;
		glBegin(GL_LINE_STRIP);						
			glVertex3f( c.x - symbolScale, c.y - symbolScale,fz);	
			glVertex3f( c.x - symbolScale, c.y + symbolScale,fz);	
			glVertex3f( c.x + symbolScale, c.y + symbolScale,fz);
			glVertex3f( c.x + symbolScale, c.y - symbolScale,fz);
			glVertex3f( c.x - symbolScale, c.y - symbolScale,fz);
		glEnd();
	}
}

void LineDrawer::prepare(PreparationParameters *p){
	SimpleDrawer::prepare(p);
}

void LineDrawer::shareVertices(vector<Coord *>& coords) {
	//coords.push_back(&cNorm);

	int count = 0;
	for(int j = 0; j < lines.size(); ++j) {
		CoordinateSequence *points = lines.at(j);
		count += points->size();
	}
	coords.resize(count);
	int index = 0;
	for(int j = 0; j < lines.size(); ++j) {
		CoordinateSequence *points = lines.at(j);
		for(int i = 0; i < points->size(); ++i) {
			const Coord *c = (const Coord *)&(points->getAt(i));
			coords[index++] = const_cast<Coord *>(c);
		}
			
	}
}

void LineDrawer::setSpecialDrawingOptions(int option, bool add, const vector<int>& coords){
	SimpleDrawer::setSpecialDrawingOptions(option, add);
	selectedCoords.clear();
	for(int n=0; n < coords.size(); ++n) {
		selectedCoords.push_back(coords.at(n));
	}
}

GeneralDrawerProperties *LineDrawer::getProperties() {
	return &lproperties;
}

//----------------------------------------
String LineProperties::store(const FileName& fnView, const String& parentSection) const{
	ObjectInfo::WriteElement(parentSection.scVal(),"Thickness",fnView, thickness);
	ObjectInfo::WriteElement(parentSection.scVal(),"LineStyle",fnView, linestyle);
	ObjectInfo::WriteElement(parentSection.scVal(),"DrawColor",fnView, drawColor);
	ObjectInfo::WriteElement(parentSection.scVal(),"IgnoreColor",fnView, ignoreColor);
	return parentSection;
}

void LineProperties::load(const FileName& fnView, const String& parentSection){
	ObjectInfo::ReadElement(parentSection.scVal(),"Thickness",fnView, thickness);
	ObjectInfo::ReadElement(parentSection.scVal(),"LineStyle",fnView, linestyle);
	ObjectInfo::ReadElement(parentSection.scVal(),"DrawColor",fnView, drawColor);
	ObjectInfo::ReadElement(parentSection.scVal(),"IgnoreColor",fnView, ignoreColor);
}



