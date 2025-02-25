#include "Headers\toolspch.h"
#include "Engine\Drawers\RootDrawer.h"
#include "CubeDrawer.h"
//#include "Engine\Drawers\SimpleDrawer.h"
//#include "Engine\Drawers\TextDrawer.h"


using namespace ILWIS;

CubeElement CubeElement::undefElement = CubeElement(sUNDEF, colorUNDEF,rUNDEF,false);

ILWIS::NewDrawer *createCubeDrawer(DrawerParameters *parms) {
	return new CubeDrawer(parms);
}

CubeDrawer::CubeDrawer(DrawerParameters *parms)
: ComplexDrawer(parms,"CubeDrawer")
, timePos(0)
, sTimePosText(0)
, font(0)
, mediumFont(0)
, csFont(new CCriticalSection())
{
	id = name = "CubeDrawer";
	setAlpha(1); // opaque
}

CubeDrawer::~CubeDrawer() {
	if (font) {
		csFont->Lock();
		delete font;
		delete mediumFont;
		csFont->Unlock();
	}
	delete csFont;
}

void CubeDrawer::prepare(PreparationParameters *pp) {
	if ((pp->type & RootDrawer::ptGEOMETRY) || (pp->type & NewDrawer::ptRESTORE) || (pp->type & NewDrawer::pt3D)) {
		if ((font != 0) && (pp->type & NewDrawer::pt3D)) {
			csFont->Lock();
			delete font;
			delete mediumFont;
			font = new OpenGLText(rootDrawer, "arial.ttf", 28, true, 0, 0, true);
			mediumFont = new OpenGLText(rootDrawer, "arial.ttf", 15, true, 0, 0, true);
			csFont->Unlock();
		} else if (font == 0) {
			font = new OpenGLText(rootDrawer, "arial.ttf", 28, true, 0, 0, true);
			mediumFont = new OpenGLText(rootDrawer, "arial.ttf", 15, true, 0, 0, true);
		}
		cube = rootDrawer->getMapCoordBounds();
		CoordBounds cbMap = cube;
		if (cube.width() > cube.height()) {
			double deltay = cube.width() - cube.height();
			cube.cMin.y = cube.cMin.y - deltay / 2.0;
			cube.cMax.y = cube.cMax.y + deltay / 2.0;
			cube.cMin.z = 0;
			cube.cMax.z = cube.width();
		} else {
			double deltax = cube.height() - cube.width();
			cube.cMin.x = cube.cMin.x - deltax / 2.0;
			cube.cMax.x = cube.cMax.x + deltax / 2.0;
			cube.cMin.z = 0;
			cube.cMax.z = cube.height();
		}
		cbMap.MaxZ() = cube.cMax.z;
		cbMap.MinZ() = cube.cMin.z;
		rootDrawer->setCoordBoundsMap(cube);
		CoordSystem csy = rootDrawer->getCoordinateSystem();
		String sMin = csy->sValue(rootDrawer->glToWorld(cube.cMin), 0);
		String sMax = csy->sValue(rootDrawer->glToWorld(cube.cMax), 0);
		if (csy->pcsLatLon()) {
			sxMin = sMin.sTail(",").sTrimSpaces();
			syMin = sMin.sHead(",").sTrimSpaces();
			sxMax = sMax.sTail(",").sTrimSpaces();
			syMax = sMax.sHead(",").sTrimSpaces();
		} else {
			sxMin = sMin.sTail("(");
			syMin = sxMin.sTail(",").sHead(")").sTrimSpaces();
			sxMin = sxMin.sHead(",").sTrimSpaces();
			sxMax = sMax.sTail("(");
			syMax = sxMax.sTail(",").sHead(")").sTrimSpaces();
			sxMax = sxMax.sHead(",").sTrimSpaces();
		}
	}
	if (pp->type & NewDrawer::pt3D) {
		if (timeBounds != 0) {
			stMin = timeBounds->tMin().isValid() ? timeBounds->tMin().toString() : "";
			stMax = timeBounds->tMax().isValid() ? timeBounds->tMax().toString() : "";

			if (timePos != 0 && sTimePosText != 0) {
				if (timeBounds->tMin().isValid() && timeBounds->tMax().isValid()) {
					Time tPos (timeBounds->tMin() + (Time)((timeBounds->tMax() - timeBounds->tMin()) * *timePos));
					*sTimePosText = tPos.toString();
				} else
					*sTimePosText = "";
			}
		}
	}
	if ( pp->type & NewDrawer::ptOFFSCREENSTART || pp->type & NewDrawer::ptOFFSCREENEND) {
		if ( font)
			font->prepare(pp);
		if ( mediumFont)
			mediumFont->prepare(pp);
	}
}

void CubeDrawer::SetTimePosVariables(double * _timePos, String * _sTimePosText) {
	timePos = _timePos;
	sTimePosText = _sTimePosText;
}

bool CubeDrawer::draw(const DrawLoop drawLoop, const CoordBounds& cbArea) const{
	if ( !isActive())
		return false;

	drawPreDrawers(drawLoop, cbArea);

	glPushMatrix();
	glTranslated(cube.cMin.x + cube.width() / 2.0, cube.cMin.y + cube.height() / 2.0, cube.cMin.z + cube.altitude() / 2.0);
	glScaled(cube.width() / 2.0, cube.height() / 2.0, cube.altitude() / 2.0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); 

	if (properties["cube"].visible) {
		Color clr = properties["cube"].color;
		clr.alpha() = properties["cube"].alpha * 255;
		if ((drawLoop == drl2D) || (drawLoop == drl3DOPAQUE && clr.alpha() == 255) || (drawLoop == drl3DTRANSPARENT && clr.alpha() != 255)) {
			glColor4f(clr.redP(), clr.greenP(), clr.blueP(), clr.alphaP());
			drawCube();
		}
	}
	if (properties["ticks"].visible) {
		Color clr = properties["ticks"].color;
		clr.alpha() = properties["ticks"].alpha * 255;
		if ((drawLoop == drl2D) || (drawLoop == drl3DOPAQUE && clr.alpha() == 255) || (drawLoop == drl3DTRANSPARENT && clr.alpha() != 255)) {
			glColor4f(clr.redP(), clr.greenP(), clr.blueP(), clr.alphaP());
			drawTicMarks();
		}
	}
	csFont->Lock();
	if (properties["labels"].visible) {
		Color clr = properties["labels"].color;
		clr.alpha() = properties["labels"].alpha * 255;
		if ((drawLoop == drl2D) || (drawLoop == drl3DOPAQUE && clr.alpha() == 255) || (drawLoop == drl3DTRANSPARENT && clr.alpha() != 255))
			drawLabels(drawLoop, clr);
	}
	if (properties["coordinates"].visible) {
		Color clr = properties["coordinates"].color;
		clr.alpha() = properties["coordinates"].alpha * 255;
		if ((drawLoop == drl2D) || (drawLoop == drl3DOPAQUE && clr.alpha() == 255) || (drawLoop == drl3DTRANSPARENT && clr.alpha() != 255)) {
			drawCoords(drawLoop, clr);
			drawTimes(drawLoop, clr);
		}
	}
	csFont->Unlock();

	glPopMatrix();

	drawPostDrawers(drawLoop, cbArea);
	return true;
}

void CubeDrawer::drawCube() const {	

	// Front Face
	glBegin(GL_LINE_STRIP);
	glVertex3d(-1.0, -1.0, +1.0);
	glVertex3d(+1.0, -1.0, +1.0);
	glVertex3d(+1.0, +1.0, +1.0);
	glVertex3d(-1.0, +1.0, +1.0);
	glEnd();

	// Back Face
	glBegin(GL_LINE_STRIP);
	glVertex3d(-1.0, -1.0, -1.0);
	glVertex3d(-1.0, +1.0, -1.0);
	glVertex3d(+1.0, +1.0, -1.0);
	glVertex3d(+1.0, -1.0, -1.0);
	glEnd();

	// Top Face
	glBegin(GL_LINE_STRIP);
	glVertex3d(-1.0, +1.0, -1.0);
	glVertex3d(-1.0, +1.0, +1.0);
	glVertex3d(+1.0, +1.0, +1.0);
	glVertex3d(+1.0, +1.0, -1.0);
	glEnd();

	// Bottom Face
	glBegin(GL_LINE_STRIP);
	glVertex3d(-1.0, -1.0, -1.0);
	glVertex3d(+1.0, -1.0, -1.0);
	glVertex3d(+1.0, -1.0, +1.0);
	glVertex3d(-1.0, -1.0, +1.0);
	glEnd();

	// Right Face
	glBegin(GL_LINE_STRIP);
	glVertex3d(+1.0, -1.0, -1.0);
	glVertex3d(+1.0, +1.0, -1.0);
	glVertex3d(+1.0, +1.0, +1.0);
	glVertex3d(+1.0, -1.0, +1.0);
	glEnd();

	// Left Face
	glBegin(GL_LINE_STRIP);
	glVertex3d(-1.0, -1.0, -1.0);
	glVertex3d(-1.0, -1.0, +1.0);
	glVertex3d(-1.0, +1.0, +1.0);
	glVertex3d(-1.0, +1.0, -1.0);
	glEnd();
}

void CubeDrawer::drawTicMarks() const {
	const int nrTicMarks = 11; // use odd number so that there's a tic in the centre of an edge
	const double ticStep = 2.0 / (nrTicMarks-1); // 2.0 is the size of the cube

	for (int i=0; i<nrTicMarks; ++i)
	{
		double ticPos = -1 + i * ticStep; // position of each tic between -1 and +1
		double ticSize = ((i > 0) && (i < (nrTicMarks-1)))?0.025:0.05; // larger tics in corners 
		double ticLine = 1 + ticSize;
		// Tic marks X-axis
		glBegin(GL_LINE_STRIP);
		glVertex3d(ticPos, -ticLine, -1.0);
		glVertex3d(ticPos, -1.0, -1.0);
		glEnd();
		// Tic marks Y-axis
		glBegin(GL_LINE_STRIP);
		glVertex3d(-ticLine, ticPos, -1.0);
		glVertex3d(-1.0, ticPos, -1.0);
		glEnd();
		// Tic marks Z-axis
		glBegin(GL_LINE_STRIP);
		glVertex3d(-ticLine, -1.0, ticPos);
		glVertex3d(-1.0f, -1.0, ticPos);
		glEnd();
	}
}

void CubeDrawer::drawLabels(const DrawLoop drawLoop, const Color & clr) const {
	font->setColor(clr);
	renderText(drawLoop, font,Coordinate(0.0, -1.1, -1.1), "X");
	renderText(drawLoop, font,Coordinate(-1.1, 0.0, -1.1), "Y");
	renderText(drawLoop, font,Coordinate(-1.1, -1.1, 0.0), "T");
}

void CubeDrawer::drawCoords(const DrawLoop drawLoop, const Color & clr) const {
	mediumFont->setColor(clr);
	renderText(drawLoop, mediumFont,Coordinate(-0.9, -1.1, -1.1), sxMin);
	renderText(drawLoop, mediumFont,Coordinate(0.9, -1.1, -1.1), sxMax);
	renderText(drawLoop, mediumFont,Coordinate(-1.1, -0.9, -1.1), syMin);
	renderText(drawLoop, mediumFont,Coordinate(-1.1, 0.9, -1.1), syMax);

	//provisional code for alternative coordinate visualization
	//renderText(mediumFont,Coordinate(-1.0,-1.0,-1.05), String("(%S, %S)", syMin, sxMin));
	//renderText(mediumFont,Coordinate(1.0,-1.0,-1.05), String("(%S, %S)", syMax, sxMin));
	//renderText(mediumFont,Coordinate(1.0,1.0,-1.05), String("(%S, %S)", syMax, sxMax));
	//renderText(mediumFont,Coordinate(-1.0,1.0,-1.05), String("(%S, %S)", syMin, sxMax));
}

void CubeDrawer::drawTimes(const DrawLoop drawLoop, const Color & clr) const {
	mediumFont->setColor(clr);
	renderText(drawLoop, mediumFont,Coordinate(-1.1, -1.1, -0.9), stMin);
	renderText(drawLoop, mediumFont,Coordinate(-1.1, -1.1, 0.9), stMax);

	if (timePos != 0 && sTimePosText != 0)
		renderText(drawLoop, mediumFont, Coordinate(-1.1, -1.1, -0.9 + 1.8 * *timePos), *sTimePosText);
}

void CubeDrawer::renderText(const DrawLoop drawLoop, OpenGLText *fnt,const Coordinate & c, const String & text, bool center) const {
	glPushMatrix();
	glTranslated(c.x, c.y, c.z);
	glScaled(2.0 / cube.width(), 2.0 / cube.height(), 2.0 / cube.altitude());
	fnt->renderText(drawLoop, Coordinate(0, 0, 0), text);
	glPopMatrix();
}

GeneralDrawerProperties *CubeDrawer::getProperties(){
	return &properties;
}

String CubeDrawer::store(const FileName& fnView, const String& section) const {
	String currentSection = section + ":Cube";
	ComplexDrawer::store(fnView, currentSection);
	properties.store(fnView, currentSection);
	return currentSection;
}

void CubeDrawer::load(const FileName& fnView, const String& currentSection){
	ComplexDrawer::load(fnView, currentSection);
	properties.load(fnView, currentSection);
}

//---------------------------------------
CubeProperties::CubeProperties() : GeneralDrawerProperties() {
	elements["cube"] = CubeElement("Cube", Color(0,255,0),1.0, true);
	elements["ticks"] = CubeElement("Ticks", Color(0,255,0),1.0, true);
	elements["coordinates"] = CubeElement("Coordinates", Color(0,0,0),1.0, true);
	elements["labels"] = CubeElement("Labels", Color(0,0,0),1.0, true);
}

CubeProperties::CubeProperties(CubeProperties *lp){
	for(map<String, CubeElement>::const_iterator cur = lp->elements.begin(); cur != lp->elements.end(); ++cur){
		(*this)[(*cur).first] = (*cur).second;
	}

}

CubeElement& CubeProperties::operator[](const String& key) const {
	map<String, CubeElement>::iterator cur = const_cast<CubeProperties *>(this)->elements.find(key) ;
	if ( cur != elements.end())
		return (*cur).second;
	return CubeElement::undefElement;

}

String CubeProperties::store(const FileName& fnView, const String& parentSection) const {
	ObjectInfo::WriteElement(parentSection.c_str(), "NrCubeElements", fnView, (int)(elements.size()));
	int index = 0;
	for (map<String, CubeElement>::const_iterator cur = elements.begin(); cur != elements.end(); ++cur) {
		String elementName = cur->first;
		CubeElement element = cur->second;
		String elementSection ("%S:CubeElement%d", parentSection, index++);
		ObjectInfo::WriteElement(elementSection.c_str(), "Name", fnView, elementName);
		element.store(fnView, elementSection);
	}
	return parentSection;
}

void CubeProperties::load(const FileName& fnView, const String& parentSection) {
	int nrElements;
	if (!ObjectInfo::ReadElement(parentSection.c_str(), "NrCubeElements", fnView, nrElements))
		nrElements = 0;
	for (int index = 0; index < nrElements; ++index) {
		String elementName;
		String elementSection ("%S:CubeElement%d", parentSection, index);
		ObjectInfo::ReadElement(elementSection.c_str(), "Name", fnView, elementName);
		CubeElement element;
		element.load(fnView, elementSection);
		elements[elementName] = element;
	}
}

String CubeElement::store(const FileName& fnView, const String& parentSection) const {
	ObjectInfo::WriteElement(parentSection.c_str(), "Color", fnView, color);
	ObjectInfo::WriteElement(parentSection.c_str(), "Alpha", fnView, alpha);
	ObjectInfo::WriteElement(parentSection.c_str(), "Visible", fnView, visible);
	ObjectInfo::WriteElement(parentSection.c_str(), "Label", fnView, label);
	return parentSection;
}

void CubeElement::load(const FileName& fnView, const String& parentSection) {
	ObjectInfo::ReadElement(parentSection.c_str(), "Color", fnView, color);
	ObjectInfo::ReadElement(parentSection.c_str(), "Alpha", fnView, alpha);
	ObjectInfo::ReadElement(parentSection.c_str(), "Visible", fnView, visible);
	ObjectInfo::ReadElement(parentSection.c_str(), "Label", fnView, label);
}
