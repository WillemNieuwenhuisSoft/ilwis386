#include "Headers\toolspch.h"
#include "Engine\Map\basemap.h"
#include "ComplexDrawer.h"
#include "Engine\Drawers\DrawerContext.h"
#include "Engine\Drawers\RootDrawer.h"
#include "Engine\Drawers\SpatialDataDrawer.h"
#include "Engine\Drawers\ZValueMaker.h"
#include "Engine\Drawers\OpenGLText.h"

using namespace ILWIS;

#define ROTATIONSWITCH 0.4

RootDrawer::RootDrawer()
: ComplexDrawer(0,"RootDrawer")
, fUseGeoRef(false)
, fAnnotationWhitespace(false)
, fAnnotationBorder(false)
, annotationFont(0)
, fDrawing(false)
, fRequestAnnotationRecompute(false)
{
	drawercontext = new ILWIS::DrawerContext();
	ILWIS::DrawerParameters dp(this, this);
	ILWIS::PreparationParameters pp(RootDrawer::ptALL,0);
	ComplexDrawer *annotationDrawers = (ComplexDrawer *)NewDrawer::getDrawer("AnnotationDrawers", &pp, &dp);
	if ( annotationDrawers) {
		addPostDrawer(800, annotationDrawers);
		annotationDrawers->addPostDrawer(200,NewDrawer::getDrawer("GridDrawer", &pp, &dp));
		annotationDrawers->addPostDrawer(300,NewDrawer::getDrawer("GraticuleDrawer", &pp, &dp));
	}
	backgroundDrawer = NewDrawer::getDrawer("CanvasBackgroundDrawer", &pp, &dp);

	setAlpha(1.0);
	setName("RootDrawer");
	threeD = false;
	windowAspectRatio = 0;
	mapAspectRatio = 0;
	topDrawer = 0;
	rotX = rotZ = 0;
	rotY = 45.0;
	translateX = translateY = translateZ = 0;
	zoom3D = 1.0;
	rootDrawer = this;
	info = true;
	ext = Extension(0,0,0,0);
	skyColor = Color(255,255,255);
}

RootDrawer::~RootDrawer() {
	clear();
	delete backgroundDrawer;
	delete drawercontext; // delete drawers before drawercontext, some drawers still work on the GL context and clean it up
}

void  RootDrawer::prepare(PreparationParameters *pp){
	bool v1 = pp->type & RootDrawer::ptINITOPENGL;
	bool v2 = pp->type & RootDrawer::ptALL;
	if ( pp->dc && (  v1 || v2 )) {
		if ( ((pp->contextMode & DrawerContext::mFORCEINIT)!=0) && getDrawerContext())
			getDrawerContext()->ReleaseContext();
		if ( getDrawerContext()->initOpenGL(pp->dc->m_hDC, pp->dc->GetWindow(), pp->contextMode)) {
			pp->type |= NewDrawer::ptGEOMETRY;
			CWnd * wnd = pp->dc->GetWindow();
			if ( wnd) {
				CRect rct;
				wnd->GetClientRect(&rct);
				RowCol rc(rct.Height(), rct.Width());
				setViewPort(rc, true);
			}
		}
	}
	if ( !(pp->type & RootDrawer::ptINITOPENGL)) {
		backgroundDrawer->prepare(pp);

		for(map<String,NewDrawer *>::iterator cur = preDrawers.begin(); cur != preDrawers.end(); ++cur) {
			(*cur).second->prepare(pp);
		}

		for(int i=0; i < drawers.size(); ++i) {
			drawers[i]->prepare(pp);
		}

		for(map<String,NewDrawer *>::iterator cur = postDrawers.begin(); cur != postDrawers.end(); ++cur) {
			(*cur).second->prepare(pp);
		}
	}
}

String RootDrawer::addDrawer(NewDrawer *drw, bool overrule) {
	SpatialDataDrawer *mapdrw = dynamic_cast<SpatialDataDrawer *>(drw);
	if ( overrule && mapdrw && mapdrw->getBaseMap() != 0) {
		CoordSystem _cs = mapdrw->getBaseMap()->cs();
		CoordBounds ncb = (!cs.fValid() || !_cs.fValid() || cs.fEqual(_cs) || cs->fUnknown() || _cs->fUnknown()) ? mapdrw->cb() : cs->cbConv(_cs, mapdrw->cb());
		//addCoordBounds(mapdrw->getBaseMap()->cs(), cb); // don't call addCoordBounds, as this extends cbMap and we only want to extend cbView here.
		//cb += ncb;
		if (fUseGeoRef) {
			double rRow;
			double rCol;
			gr->Coord2RowCol(ncb.cMin, rRow, rCol);
			ncb.cMin = Coord(rCol, -rRow);
			gr->Coord2RowCol(ncb.cMax, rRow, rCol);
			ncb.cMax = Coord(rCol, -rRow);
		}
		CoordBounds cb = cbView;
		cb += ncb;
		if ( ! (cb == cbView))
			setCoordBoundsView(cb, true);

	}
	return ComplexDrawer::addDrawer(drw);
}

void RootDrawer::addCoordBounds(const CoordSystem& _cs, const CoordBounds& cb, bool extend){
	if ( extend || cbMap.fUndef()) {
		CoordBounds ncb = (!cs.fValid() || !_cs.fValid() || cs.fEqual(_cs) || cs->fUnknown() || _cs->fUnknown()) ? cb : cs->cbConv(_cs, cb);
		if (fUseGeoRef) {
			double rRow;
			double rCol;
			gr->Coord2RowCol(ncb.cMin, rRow, rCol);
			ncb.cMin = Coord(rCol, -rRow);
			gr->Coord2RowCol(ncb.cMax, rRow, rCol);
			ncb.cMax = Coord(rCol, -rRow);
		}
		cbMap += ncb;
	}
	setCoordBoundsView(cbMap, extend);
}

/*
Note: calls to RootDrawer::draw are meaningless without an OpenGL context in the current thread.
Therefore all calls to RootDrawer::draw must be preceded by a call to DrawerContext::TakeContext and followed by a call to ReleaseContext
*/

bool RootDrawer::draw(const CoordBounds& cb) const{
	if ( topDrawer ) {
		topDrawer->draw(drl2D);
	}
	else {
		const_cast<RootDrawer*>(this)->fDrawing = true;
		// Setup
		glClearColor(skyColor.redP(),skyColor.greenP(),skyColor.blueP(),0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
		if (threeD)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
		glViewport(0,0,pixArea.Col, pixArea.Row);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		if ( threeD) {
			double zNear = cbZoom.height() * 3.0 / 4.0;
			double zFar = max(cbZoom.width(), cbZoom.height()) * 4.0;
			if (zoom3D < 1.0) // use Field Of View to zoom-in, and scale to zoom out (FOV distorts when zooming out)
				gluPerspective(30.0 * zoom3D, windowAspectRatio, zNear, zFar);
			else
				gluPerspective(30.0, windowAspectRatio, zNear, zFar);
		} else {
			glOrtho(cbZoomExt.cMin.x,cbZoomExt.cMax.x,cbZoomExt.cMin.y,cbZoomExt.cMax.y,-1,1);
		}
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		if (threeD) {
			gluLookAt(0, 0, cbZoom.width() * 1.5, 0, 0, 0, 0, 1.0, 0 );
			if (zoom3D >= ROTATIONSWITCH)
				glTranslatef(translateX, translateY, translateZ);
			glRotatef(rotY,-1,0,0);				// Rotate on y
			glRotatef(rotX,0,0,-1);				// Rotate on x
			if (zoom3D < ROTATIONSWITCH)
				glTranslatef(translateX, translateY, translateZ);
			if (zoom3D > 1.0)
				glScalef(1.0 / zoom3D, 1.0 / zoom3D, 1.0 / zoom3D);
			glTranslatef(-viewPoint.x, -viewPoint.y, -viewPoint.z);
		}

		// Draw
		if (is3D()) {
			// see http://www.opengl.org/archives/resources/faq/technical/transparency.htm
			// "When using depth buffering in an application, you need to be careful about the order in which you render primitives. Fully opaque primitives need to be rendered first, followed by partially opaque primitives in back-to-front order. If you don't render primitives in this order, the primitives, which would otherwise be visible through a partially opaque primitive, might lose the depth test entirely."
			// The lines below is as good as it gets for now, dividing the drawers in a opaque and a transparent group, and drawing the opaques first.
			// We work around the lack of a "back to front order" (which is too costly to compute) by disabling depth-buffer-writing in the transparent stage (so transparent items will never obstruct eachother).
			// In theory the performance impact should be minimal, as almost all draw() calls only react to one of the two calls below, making the two calls complementary (with currently only one exception, which is when drawing a raster with both opaque and transparent pixels).
			const_cast<RootDrawer *>(this)->setZIndex(0);
			glDepthRange(0.01, 1.0); // for backgroundDrawer
			backgroundDrawer->draw(drl3DOPAQUE, cb);
			ComplexDrawer::draw(drl3DOPAQUE, cb);
			const_cast<RootDrawer *>(this)->setZIndex(0);
			glDepthMask(GL_FALSE);
			glDepthRange(0.01, 1.0); // for backgroundDrawer
			backgroundDrawer->draw(drl3DTRANSPARENT, cb);
			ComplexDrawer::draw(drl3DTRANSPARENT, cb);
			glDepthMask(GL_TRUE);
		} else {
			const_cast<RootDrawer *>(this)->setZIndex(0);
			backgroundDrawer->select(false); // CanvasBackgroundDrawer::select misused; draw only map background
			backgroundDrawer->draw(drl2D, cb);
			ComplexDrawer::draw(drl2D, cb);
			backgroundDrawer->select(true); // CanvasBackgroundDrawer::select misused; draw only map sides
			backgroundDrawer->draw(drl2D, cb);
		}
		const_cast<RootDrawer*>(this)->fDrawing = false;
		if (fRequestAnnotationRecompute) {
			const_cast<RootDrawer*>(this)->RecomputeAnnotationBorder();
			const_cast<RootDrawer*>(this)->fRequestAnnotationRecompute = false;
		}
	}
	return true;
}

int RootDrawer::getZIndex() const {
	return getZMaker()->getZOrder();
}

void RootDrawer::setZIndex(int n) {
	getZMaker()->setZOrder(n);
}

void RootDrawer::addDataSource(void *) {
}

void RootDrawer::timedEvent(UINT timerID) {
	for(DrawerMap::iterator cur = drawersById.begin(); cur != drawersById.end(); ++cur) {
		(*cur).second->timedEvent(timerID);
	}
}

String RootDrawer::store(const FileName& fnView, const String section) const{
	ObjectInfo::WriteElement(section.c_str(),"CoordinateSystem",fnView, getCoordinateSystem());
	ObjectInfo::WriteElement(section.c_str(),"UseGeoRef",fnView, fUseGeoRef);
	if (fUseGeoRef)
		ObjectInfo::WriteElement(section.c_str(),"GeoReference",fnView, getGeoReference());

	backgroundDrawer->store(fnView, section);
	ComplexDrawer::store(fnView, section);

	ObjectInfo::WriteElement(section.c_str(),"ViewPort",fnView, pixArea);
	ObjectInfo::WriteElement(section.c_str(),"CoordBoundsZoom",fnView, getCoordBoundsZoom());
	ObjectInfo::WriteElement(section.c_str(),"CoordBoundsView",fnView, getCoordBoundsView());
	ObjectInfo::WriteElement(section.c_str(),"CoordBoundsMap",fnView, getMapCoordBounds());
	ObjectInfo::WriteElement(section.c_str(),"AnnotationBorder",fnView, fAnnotationBorder);
	ObjectInfo::WriteElement(section.c_str(),"Extension_left",fnView, ext.left);
	ObjectInfo::WriteElement(section.c_str(),"Extension_right",fnView, ext.right);
	ObjectInfo::WriteElement(section.c_str(),"Extension_top",fnView, ext.top);
	ObjectInfo::WriteElement(section.c_str(),"Extension_bottom",fnView, ext.bottom);
	//ObjectInfo::WriteElement(section.c_str(),"AspectRatio",fnView, getAspectRatio());
	
	ObjectInfo::WriteElement(section.c_str(),"XRotation",fnView, rotX);
	ObjectInfo::WriteElement(section.c_str(),"YRotation",fnView, rotY);
	ObjectInfo::WriteElement(section.c_str(),"ZRotation",fnView, rotZ);
	ObjectInfo::WriteElement(section.c_str(),"XTranslation",fnView, translateX);
	ObjectInfo::WriteElement(section.c_str(),"YTranslation",fnView, translateY);
	ObjectInfo::WriteElement(section.c_str(),"ZTranslation",fnView, translateZ);
	ObjectInfo::WriteElement(section.c_str(),"Zoom3D",fnView, zoom3D);
	ObjectInfo::WriteElement(section.c_str(),"ViewPoint",fnView, viewPoint);
	ObjectInfo::WriteElement(section.c_str(),"Is3D",fnView, threeD);

	return section.c_str();
}

void RootDrawer::load(const FileName& fnView, const String section){
	ObjectInfo::ReadElement(section.c_str(),"CoordinateSystem",fnView, cs);
	ObjectInfo::ReadElement(section.c_str(),"UseGeoRef",fnView, fUseGeoRef);
	if (fUseGeoRef)
		ObjectInfo::ReadElement(section.c_str(),"GeoReference",fnView, gr);

	ComplexDrawer::load(fnView,section); // this overwrites cbZoom, cbMap etc, so they have to be loaded later.
	backgroundDrawer->load(fnView, section + ":CanvasBackground");

	CoordBounds cbZ,cbV,cbM;
	ObjectInfo::ReadElement(section.c_str(),"CoordBoundsZoom",fnView, cbZ);
	ObjectInfo::ReadElement(section.c_str(),"CoordBoundsView",fnView, cbV);
	ObjectInfo::ReadElement(section.c_str(),"CoordBoundsMap",fnView, cbM);
	/*double aspect;
	ObjectInfo::ReadElement(section.c_str(),"AspectRatio",fnView, aspect);*/
	RowCol viewPort;
	ObjectInfo::ReadElement(section.c_str(),"ViewPort",fnView, viewPort);
	ObjectInfo::ReadElement(section.c_str(),"Is3D",fnView, threeD);

	setViewPort(viewPort, false); // false, just "assign" to pixArea and windowAspectRatio
	setCoordBoundsMap(cbM);
	setCoordBoundsView(cbV, true);
	//setCoordBoundsZoom(cbZ);
	cbZoom = cbZ;
	ObjectInfo::ReadElement(section.c_str(),"AnnotationBorder",fnView, fAnnotationBorder);
	ObjectInfo::ReadElement(section.c_str(),"Extension_left",fnView, ext.left);
	ObjectInfo::ReadElement(section.c_str(),"Extension_right",fnView, ext.right);
	ObjectInfo::ReadElement(section.c_str(),"Extension_top",fnView, ext.top);
	ObjectInfo::ReadElement(section.c_str(),"Extension_bottom",fnView, ext.bottom);
	setExtension(ext); // sets fAnnotationWhitespace and calls RecomputeAnnotationBorder()

	ObjectInfo::ReadElement(section.c_str(),"XRotation",fnView, rotX); 
	ObjectInfo::ReadElement(section.c_str(),"YRotation",fnView, rotY);
	ObjectInfo::ReadElement(section.c_str(),"ZRotation",fnView, rotZ);
	ObjectInfo::ReadElement(section.c_str(),"XTranslation",fnView, translateX);
	ObjectInfo::ReadElement(section.c_str(),"YTranslation",fnView, translateY);
	ObjectInfo::ReadElement(section.c_str(),"ZTranslation",fnView, translateZ);
	ObjectInfo::ReadElement(section.c_str(),"Zoom3D",fnView, zoom3D);
	ObjectInfo::ReadElement(section.c_str(),"ViewPoint",fnView, viewPoint);
}

void RootDrawer::RecenterZoomHorz(CoordBounds & cbZoom, const CoordBounds & cbMap)
{
	double zwidth = cbZoom.width();
	if (zwidth > cbMap.width()) {
		double delta = (zwidth - cbMap.width()) / 2.0;
		cbZoom.cMin.x = cbMap.cMin.x - delta;
		cbZoom.cMax.x = cbMap.cMax.x + delta;
	} else {
		if ( cbZoom.cMax.x > cbMap.cMax.x) {
			cbZoom.cMax.x = cbMap.cMax.x;
			cbZoom.cMin.x = cbZoom.cMax.x - zwidth;
		}
		if ( cbZoom.cMin.x < cbMap.cMin.x) {
			cbZoom.cMin.x = cbMap.cMin.x;
			cbZoom.cMax.x = cbZoom.cMin.x + zwidth;
		}
	}
}

void RootDrawer::RecenterZoomVert(CoordBounds & cbZoom, const CoordBounds & cbMap)
{
	double zheight = cbZoom.height();
	if (zheight > cbMap.height()) {
		double delta = (zheight - cbMap.height()) / 2.0;
		cbZoom.cMin.y = cbMap.cMin.y - delta;
		cbZoom.cMax.y = cbMap.cMax.y + delta;
	} else {
		if ( cbZoom.cMax.y > cbMap.cMax.y) {
			cbZoom.cMax.y = cbMap.cMax.y;
			cbZoom.cMin.y = cbZoom.cMax.y - zheight;
		}
		if ( cbZoom.cMin.y < cbMap.cMin.y) {
			cbZoom.cMin.y = cbMap.cMin.y;
			cbZoom.cMax.y = cbZoom.cMin.y + zheight;
		}
	}
}

void RootDrawer::modifyZoomX(double rFactor) {
	double deltazoom = cbZoom.width() * rFactor;
	Coord cMiddle = cbZoom.middle();
	cbZoom.cMin.x = cMiddle.x - deltazoom / 2.0;
	cbZoom.cMax.x = cMiddle.x + deltazoom / 2.0;
	RecenterZoomHorz(cbZoom, cbMap);
	if (cbZoom.width() >= cbMap.width()) {
		cbView.cMin.x = cbZoom.cMin.x;
		cbView.cMax.x = cbZoom.cMax.x;
	}
}

void RootDrawer::modifyZoomY(double rFactor) {
	double deltazoom = cbZoom.height() * rFactor;
	Coord cMiddle = cbZoom.middle();
	cbZoom.cMin.y = cMiddle.y - deltazoom / 2.0;
	cbZoom.cMax.y = cMiddle.y + deltazoom / 2.0;
	RecenterZoomVert(cbZoom, cbMap);
	if (cbZoom.height() >= cbMap.height()) {
		cbView.cMin.y = cbZoom.cMin.y;
		cbView.cMax.y = cbZoom.cMax.y;
	}
}

void RootDrawer::setViewPort(const RowCol& rc, bool fNoZoom) {
	windowAspectRatio = (double)(rc.Col) / (double)(rc.Row);
	if (fNoZoom && (pixArea.Col != iUNDEF)) {
		if (rc.Col != pixArea.Col)
			modifyZoomX((double)rc.Col / pixArea.Col);
		if (rc.Row != pixArea.Row)
			modifyZoomY((double)rc.Row / pixArea.Row);
		RecomputeAnnotationBorder();
	}
	pixArea = rc;
}

void RootDrawer::setCoordinateSystem(const CoordSystem& _cs, bool overrule){
	if (overrule || cs->fUnknown()) {
		if (fUseGeoRef)
			clearGeoreference();
		if ( overrule) {
			if (cbZoom.width() < cbMap.width() || cbZoom.height() < cbMap.height()) { // is it actually zoomed in?
				CoordBounds cbZ = cbZoom;
				cbMap = _cs->cbConv(cs, cbMap);
				setCoordBoundsView(cbMap, true);
				cbZ = _cs->cbConv(cs, cbZ);
				setCoordBoundsZoom(cbZ);
			} else { // otherwise leave it as entire-map
				cbMap = _cs->cbConv(cs, cbMap);
				setCoordBoundsView(cbMap, true);
			}
		}
		cs = _cs;
	}
}

void RootDrawer::setGeoreference(const GeoRef& _gr, bool overruleMapBounds) {
	if (_gr.fValid() && (!fUseGeoRef || _gr != gr)) {
		clearGeoreference();
		gr = _gr;
		cs = gr->cs();
		fUseGeoRef = true;
		if (overruleMapBounds) {
			if (!_gr->rcSize().fUndef())
				cbMap = CoordBounds(Coord(0,0,cbMap.cMin.z), Coord(gr->rcSize().Col, -gr->rcSize().Row, cbMap.cMax.z));
			cbZoom = cbMap;
			cbView = cbMap;
		} else {
			CoordBounds cb;
			double rRow;
			double rCol;
			gr->Coord2RowCol(cbMap.cMin, rRow, rCol);
			cb += Coord(rCol, -rRow);
			gr->Coord2RowCol(Coord(cbMap.cMin.x, cbMap.cMax.y), rRow, rCol);
			cb += Coord(rCol, -rRow);
			gr->Coord2RowCol(Coord(cbMap.cMax.x, cbMap.cMin.y), rRow, rCol);
			cb += Coord(rCol, -rRow);
			gr->Coord2RowCol(cbMap.cMax, rRow, rCol);
			cb += Coord(rCol, -rRow);
			cbMap = cb;
			if (!_gr->rcSize().fUndef())
				cbMap += CoordBounds(Coord(0,0,cbMap.cMin.z), Coord(gr->rcSize().Col, -gr->rcSize().Row, cbMap.cMax.z));

			cb = CoordBounds();
			gr->Coord2RowCol(cbZoom.cMin, rRow, rCol);
			cb += Coord(rCol, -rRow);
			gr->Coord2RowCol(Coord(cbZoom.cMin.x, cbZoom.cMax.y), rRow, rCol);
			cb += Coord(rCol, -rRow);
			gr->Coord2RowCol(Coord(cbZoom.cMax.x, cbZoom.cMin.y), rRow, rCol);
			cb += Coord(rCol, -rRow);
			gr->Coord2RowCol(cbZoom.cMax, rRow, rCol);
			cb += Coord(rCol, -rRow);
			cbZoom = cb;

			cb = CoordBounds();
			gr->Coord2RowCol(cbView.cMin, rRow, rCol);
			cb += Coord(rCol, -rRow);
			gr->Coord2RowCol(Coord(cbView.cMin.x, cbView.cMax.y), rRow, rCol);
			cb += Coord(rCol, -rRow);
			gr->Coord2RowCol(Coord(cbView.cMax.x, cbView.cMin.y), rRow, rCol);
			cb += Coord(rCol, -rRow);
			gr->Coord2RowCol(cbView.cMax, rRow, rCol);
			cb += Coord(rCol, -rRow);
			cbView = cb;
		}
		setCoordBoundsView(cbMap, true);
	}
}

void RootDrawer::clearGeoreference() {
	if (fUseGeoRef) {

		CoordBounds cb;
		Coord crd;
		gr->RowCol2Coord(-cbMap.cMin.y, cbMap.cMin.x,crd);
		cb += crd;
		gr->RowCol2Coord(-cbMap.cMin.y, cbMap.cMax.x,crd);
		cb += crd;
		gr->RowCol2Coord(-cbMap.cMax.y, cbMap.cMin.x,crd);
		cb += crd;
		gr->RowCol2Coord(-cbMap.cMax.y, cbMap.cMax.x,crd);
		cb += crd;
		cbMap = cb;

		cb = CoordBounds();
		gr->RowCol2Coord(-cbZoom.cMin.y, cbZoom.cMin.x,crd);
		cb += crd;
		gr->RowCol2Coord(-cbZoom.cMin.y, cbZoom.cMax.x,crd);
		cb += crd;
		gr->RowCol2Coord(-cbZoom.cMax.y, cbZoom.cMin.x,crd);
		cb += crd;
		gr->RowCol2Coord(-cbZoom.cMax.y, cbZoom.cMax.x,crd);
		cb += crd;
		cbZoom = cb;

		cb = CoordBounds();
		gr->RowCol2Coord(-cbView.cMin.y, cbView.cMin.x,crd);
		cb += crd;
		gr->RowCol2Coord(-cbView.cMin.y, cbView.cMax.x,crd);
		cb += crd;
		gr->RowCol2Coord(-cbView.cMax.y, cbView.cMin.x,crd);
		cb += crd;
		gr->RowCol2Coord(-cbView.cMax.y, cbView.cMax.x,crd);
		cb += crd;
		cbView = cb;

		gr = GeoRef();

		fUseGeoRef = false;
		setCoordBoundsView(cbMap, true);
	}
}

bool RootDrawer::fConvNeeded(const CoordSystem& _cs) const {
	return (fUseGeoRef || (cs.fValid() && _cs.fValid() && !cs.fEqual(_cs) && !cs->fUnknown() && !_cs->fUnknown()));
}

Coord RootDrawer::glConv(const CoordSystem& _cs, const Coord& _crd) const {
	Coord crdRet(_crd);
	if (cs.fValid() && _cs.fValid() && !cs.fEqual(_cs) && !cs->fUnknown() && !_cs->fUnknown()) {
		crdRet = cs->cConv(_cs, _crd);
		crdRet.z = _crd.z;
	}
	if (fUseGeoRef) {
		double rRow;
		double rCol;
		gr->Coord2RowCol(crdRet, rRow, rCol);
		crdRet = Coord(rCol, -rRow, _crd.z);
	}
	return crdRet;
}

Coord RootDrawer::glConv(const Coord& _crd) const {
	Coord crdRet (_crd);
	if (fUseGeoRef) {
		double rRow;
		double rCol;
		gr->Coord2RowCol(crdRet, rRow, rCol);
		crdRet = Coord(rCol, -rRow, _crd.z);
	}
	return crdRet;
}

vector<Coord> RootDrawer::glConv(const CoordSystem& _cs, const vector<Coord> & _crds) const {
	if (!cs.fValid() || !_cs.fValid() || cs.fEqual(_cs) || cs->fUnknown() || _cs->fUnknown()) {
		if (fUseGeoRef) {
			vector<Coord> crdsRet;
			double rRow;
			double rCol;
			for (int i = 0; i < _crds.size(); ++i) {
				Coord crd (_crds.at(i));
				gr->Coord2RowCol(crd, rRow, rCol);
				crdsRet.push_back(Coord(rCol, -rRow, crd.z));
			}
			return crdsRet;
		} else
			return _crds;
	} else {
		if (fUseGeoRef) {
			vector<Coord> crdsRet;
			double rRow;
			double rCol;
			for (int i = 0; i < _crds.size(); ++i) {
				Coord crd (_crds.at(i));
				gr->Coord2RowCol(cs->cConv(_cs, crd), rRow, rCol);
				crdsRet.push_back(Coord(rCol, -rRow, crd.z));
			}
			return crdsRet;
		} else {
			vector<Coord> crdsRet;
			for (int i = 0; i < _crds.size(); ++i) {
				Coord crd (_crds.at(i));
				Coord crdR (cs->cConv(_cs, crd));
				crdR.z = crd.z;
				crdsRet.push_back(crdR);
			}
			return crdsRet;
		}
	}
}

vector<Coord> RootDrawer::glConv(const vector<Coord> & _crds) const {
	if (fUseGeoRef) {
		vector<Coord> crdsRet;
		double rRow;
		double rCol;
		for (int i = 0; i < _crds.size(); ++i) {
			Coord crd (_crds.at(i));
			gr->Coord2RowCol(crd, rRow, rCol);
			crdsRet.push_back(Coord(rCol, -rRow, crd.z));
		}
		return crdsRet;
	} else
		return _crds;
}

Coord RootDrawer::glToWorld(const CoordSystem& _cs, const Coord& _crd) const {
	Coord crdRet (_crd);
	if (fUseGeoRef)
		gr->RowCol2Coord(-crdRet.y, crdRet.x, crdRet);
	if (cs.fValid() && _cs.fValid() && !cs.fEqual(_cs) && !cs->fUnknown() && !_cs->fUnknown())
		crdRet = _cs->cConv(cs, crdRet);
	crdRet.z = _crd.z;
	return crdRet;
}

Coord RootDrawer::glToWorld(const Coord& _crd) const {
	Coord crdRet = _crd;
	if (fUseGeoRef) {
		gr->RowCol2Coord(-crdRet.y, crdRet.x, crdRet);
		crdRet.z = _crd.z;
	}
	return crdRet;
}

void RootDrawer::setCoordBoundsView(/*const CoordSystem& _cs,*/ const CoordBounds& cb, bool overrule){
	//CoordBounds cb = cs.fEqual(_cs) ? _cb : cs->cbConv(_cs,_cb);
	if ( overrule || cbView.fUndef()) {
		//cbMap = cb;
		mapAspectRatio = cbMap.width() / cbMap.height();
		double w = cb.width();
		double h = cb.height();
		double delta = 0;
		if ( mapAspectRatio <= 1.0) {
			double pixwidth = (double)pixArea.Row * mapAspectRatio;
			double deltay = 0;
			if ( pixwidth > pixArea.Col) {
				deltay = cb.height() * ( pixwidth / pixArea.Col - 1.0);
				pixwidth = pixArea.Col;
			}
			double fracofWidth = 1.0 - (pixArea.Col - pixwidth) / pixArea.Col;
			double crdWidth = w / fracofWidth;
			double delta = (crdWidth - w) / 2.0;
			cbView = CoordBounds(Coord(cb.MinX() - delta,cb.MinY() - deltay /2.0,0), 
				Coord(cb.MaxX() + delta,cb.MaxY() + deltay/ 2.0,0));
		} else {
			double pixheight = (double)pixArea.Col / mapAspectRatio;
			double deltax = 0;
			if ( pixheight > pixArea.Row) {
				deltax = cb.width() * ( pixheight / pixArea.Row - 1.0);
				pixheight = pixArea.Row;
			}
			double fracofHeight = 1.0 - abs(pixArea.Row - pixheight) / (double)pixArea.Row;
			double crdHeight = h / fracofHeight;
			double delta = (crdHeight - h) / 2.0;
			cbView = CoordBounds(Coord(cb.MinX() - deltax /2.0,cb.MinY()  - delta,0), 
				Coord(cb.MaxX() + deltax /2.0,cb.MaxY()  + delta,0));

		}
		cbZoom = cbView;
		setViewPoint(cbZoom.middle());
	}
	RecomputeAnnotationBorder();
	if ( is3D()) {
		rotX = 0;
		rotY = 45.0;
		translateX = translateY = translateZ = 0;
		zoom3D = 1.0;
		setCoordBoundsZoom(cbView);
	}
}

void RootDrawer::setCoordBoundsZoom(const CoordBounds& cbIn) {
	CoordBounds cb = cbIn;
	if ( cbZoom.fValid()) {
	// zooming never changes the shape of the mapwindow so any incomming zoom rectangle must conform to the shape of the existing mapwindow
		double factCur = cbZoom.width() / cbZoom.height();
		double factIn = cbIn.width() / cbIn.height();
		if ( abs(factCur - factIn) > 0.01 ) {
			if ( factCur < 1.0) {
				double newHeight = cbIn.width() / factCur;
				cb = CoordBounds(cbIn.cMin, Coord(cbIn.cMax.x, cbIn.cMin.y + newHeight, cbIn.cMax.z));
			} else {
				double newWidth = cb.height() * factCur;
				cb = CoordBounds(cbIn.cMin, Coord(cbIn.cMin.x + newWidth, cbIn.cMax.y, cbIn.cMax.z));
			}
		}
	}

	cbZoom = cb;
	setViewPoint(cbZoom.middle());
	RecomputeAnnotationBorder();
}

void RootDrawer::setCoordBoundsMap(const CoordBounds& cb) {
	cbMap = cb;
	RecomputeAnnotationBorder();
}

CoordBounds RootDrawer::getMapCoordBounds() const{
	return cbMap;
}

double RootDrawer::getAspectRatio() const {
	return mapAspectRatio;
}

Coord RootDrawer::screenToOpenGL(const RowCol& rc) {

	GLint viewport[4];
	double modelview[16];
	double projection[16];
	double winX, winY;
	double posX, posY, posZ;
	float winZ;

	if (!drawercontext->TakeContext())
		return Coord();

	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	glGetIntegerv( GL_VIEWPORT, viewport );

	winX = (double)rc.Col;
	winY = (double)viewport[3] - (double)rc.Row;
	glReadPixels( winX, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

	gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

	drawercontext->ReleaseContext();

	double z = is3D() ? posZ : 0;
	return Coord(posX, posY, z); 
}

Coord RootDrawer::screenToWorld(const RowCol& rc) {
	Coord crdRet (glToWorld(screenToOpenGL(rc)));
	return crdRet;
}

RowCol RootDrawer::OpenGLToScreen(const Coord& crd){
	drawercontext->TakeContext();

	GLint viewport[4];
	double modelview[16];
	double projection[16];
	double posX, posY, posZ;

	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	glGetIntegerv( GL_VIEWPORT, viewport );

	gluProject(crd.x, crd.y, crd.z, modelview, projection, viewport,&posX,&posY, &posZ);

	drawercontext->ReleaseContext();
	return RowCol(round(posY), round(posX));
}

RowCol RootDrawer::WorldToScreen(const Coord& crd){
	Coord crdGL = glConv(crd);
	return OpenGLToScreen(crdGL);
}

void RootDrawer::setZoom(const CRect& rect) {
	Coord c1,c2;
	if ( rect.Width() == 0 || rect.Height() == 0) { // case of clicking on the map in zoom mode
		Coord c = screenToOpenGL(RowCol(rect.top, rect.left));
		CoordBounds cb = cbZoom; // == cbView ? cbMap : cbZoom;
		double w = cb.width() / (2.0 * 1.41);
		double h = cb.height() / (2.0 * 1.41);
		c1.x = c.x - w;
		c1.y = c.y - h;
		c2.x = c.x + w;
		c2.y = c.y + h;

	}
	else {
		c1 = screenToOpenGL(RowCol(rect.top, rect.left));
		c2 = screenToOpenGL(RowCol(rect.bottom, rect.right));
	}
	c1.z = c2.z = 0;
	CoordBounds cb(c1,c2);
	setCoordBoundsZoom(cb);
}

CoordBounds RootDrawer::getCoordBoundsZoom() const  {
	return cbZoom;
}

void RootDrawer::set3D(bool yesno) {
	if ( yesno != threeD) {
		threeD = yesno;
	}
}
bool RootDrawer::is3D() const {
	return threeD;
}

void RootDrawer::setViewPoint(const Coord& c){
	viewPoint = c;
}
Coord RootDrawer::getViewPoint() const{
	return viewPoint;
}

void RootDrawer::debug() {
	GLdouble m_projMatrix[16];
	GLdouble m_modelMatrix[16];
	GLint m_viewport[4]; // x,y,width,height
	memset(m_projMatrix, 0, 16 * 8);
	memset(m_modelMatrix, 0, 16 * 8);
	memset(m_viewport, 0, 4 * 4);
	// viewport

	// get the matrices and the viewport
	glGetDoublev(GL_MODELVIEW_MATRIX, m_modelMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, m_projMatrix);
	glGetIntegerv(GL_VIEWPORT, m_viewport);
}


void RootDrawer::setTopDrawer(NewDrawer *newDrawer) {
	// topDrawer is owned by the corresponding tool, dont delete it here
	topDrawer = newDrawer;
}


void RootDrawer::setRotationAngles(double rx, double ry, double rz){
	rotX = rx;
	rotY = ry;
	rotZ = rz;
}
void RootDrawer::getRotationAngles(double& rx, double& ry, double& rz){
	rx = rotX;
	ry = rotY;
	rz = rotZ;
}

void RootDrawer::setTranslate(double tx, double ty, double tz){
	translateX = tx;
	translateY = ty;
	translateZ = tz;
}
void RootDrawer::getTranslate(double& tx, double& ty, double& tz){
	tx = translateX;
	ty = translateY;
	tz = translateZ;
}

void RootDrawer::deltaTranslate(double deltax, double deltay, double deltaz){
	if (zoom3D < ROTATIONSWITCH) {
		// http://stackoverflow.com/questions/14607640/rotating-a-vector-in-3d-space
		// rotation around x-axis
		double shX = deltax;
		double shY = deltay * cos(rotY * M_PI / 180.0) - deltaz * sin(rotY * M_PI / 180.0);
		double shZ = deltay * sin(rotY * M_PI / 180.0) + deltaz * cos(rotY * M_PI / 180.0);

		deltax = shX;
		deltay = shY;
		deltaz = shZ;

		// rotation around z-axis (concatenate)
		shX = deltax * cos(rotX * M_PI / 180.0) - deltay * sin(rotX * M_PI / 180.0);
		shY = deltax * sin(rotX * M_PI / 180.0) + deltay * cos(rotX * M_PI / 180.0);
		shZ = deltaz;

		deltax = shX;
		deltay = shY;
		deltaz = shZ;
	}
	if (zoom3D < 1.0) { // FOV zoom
		translateX += deltax * zoom3D;
		translateY += deltay * zoom3D;
		translateZ += deltaz * zoom3D;
	} else {
		translateX += deltax;
		translateY += deltay;
		translateZ += deltaz;
	}
}

double RootDrawer::getZoom3D() const{
	return zoom3D;
}

void RootDrawer::setZoom3D(double v){
	zoom3D = v;
}

void RootDrawer::deltaZoom3D(double deltav){
	if ((zoom3D < ROTATIONSWITCH) && ((zoom3D * deltav) >= ROTATIONSWITCH)) { // PostTranslate to PreTranslate
		// rotation around z-axis
		double shX = translateX * cos(-rotX * M_PI / 180.0) - translateY * sin(-rotX * M_PI / 180.0);
		double shY = translateX * sin(-rotX * M_PI / 180.0) + translateY * cos(-rotX * M_PI / 180.0);
		double shZ = translateZ;

		translateX = shX;
		translateY = shY;
		translateZ = shZ;
		// rotation around x-axis (concatenate)
		shX = translateX;
		shY = translateY * cos(-rotY * M_PI / 180.0) - translateZ * sin(-rotY * M_PI / 180.0);
		shZ = translateY * sin(-rotY * M_PI / 180.0) + translateZ * cos(-rotY * M_PI / 180.0);

		translateX = shX;
		translateY = shY;
		translateZ = shZ;
	} else if ((zoom3D >= ROTATIONSWITCH) && ((zoom3D * deltav) < ROTATIONSWITCH)) { // PreTranslate to PostTranslate
		// rotation around x-axis
		double shX = translateX;
		double shY = translateY * cos(rotY * M_PI / 180.0) - translateZ * sin(rotY * M_PI / 180.0);
		double shZ = translateY * sin(rotY * M_PI / 180.0) + translateZ * cos(rotY * M_PI / 180.0);

		translateX = shX;
		translateY = shY;
		translateZ = shZ;

		// rotation around z-axis (concatenate)
		shX = translateX * cos(rotX * M_PI / 180.0) - translateY * sin(rotX * M_PI / 180.0);
		shY = translateX * sin(rotX * M_PI / 180.0) + translateY * cos(rotX * M_PI / 180.0);
		shZ = translateZ;
		translateX = shX;
		translateY = shY;
		translateZ = shZ;
	}
	zoom3D *= deltav;
}

void RootDrawer::SetSkyColor(Color & clr) {
	skyColor = clr;
}

void RootDrawer::initLight() {
	glEnable(GL_NORMALIZE);

	// Lights, material properties
	GLfloat	specularProperties[] = {0.1f, 0.1f, 0.1f, 1.0f};
	if ( is3D())
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularProperties);

	glClearColor(1.0,1.0,1.0,0);
	glClearDepth(1.0);

	//GLfloat light0Position[] = {0.0f, 0.0f, 0.0f, 1.0f};
	//GLfloat light0Position[] = {eyePoint.x, eyePoint.y, eyePoint.z, 1.0f};
	//glLightfv(GL_LIGHT0, GL_POSITION, light0Position);
	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_LIGHT0);
	SetAmbientColor();
	SetDiffuseColor();
	SetSpecularColor();
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1.0);

	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	glLineWidth(1.0);
	glPointSize(3.0);

	glShadeModel(GL_SMOOTH);

	if (is3D())
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);

	// Default polygonmode
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	//switch (pApp->m_nPolygonMode)
	//{
	//	case 0:
	//		glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
	//		break;
	//	case 1:
	//		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	//		break;
	//	case 2:
	//		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	//		break;
	//	default:
	//		{}
	//}
}

#define TESTCOL 1.0
void RootDrawer::SetAmbientColor()
{
	GLfloat	ambientLight[]  = {TESTCOL, TESTCOL, TESTCOL, 1.0f};
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
}

void RootDrawer::SetDiffuseColor()
{
	GLfloat	diffuseLight[]  = {TESTCOL, TESTCOL, TESTCOL, 1.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
}

void RootDrawer::SetSpecularColor()
{
	GLfloat	specularLight[] = {1, 1, 1, 1.0f};
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
}

Extension RootDrawer::extension() const{
	return ext;
}

void RootDrawer::setExtension(const Extension& _ext){
	ext = _ext;
	fAnnotationWhitespace = ext.left != 0 || ext.right != 0 || ext.bottom != 0 || ext.top != 0;
	RecomputeAnnotationBorder();
}

void RootDrawer::setAnnotationFont(OpenGLText *f){
	annotationFont = f;
	RecomputeAnnotationBorder();
}

CoordBounds RootDrawer::getCoordBoundsZoomExt() const{
	return cbZoomExt;
}

CoordBounds RootDrawer::getCoordBoundsViewExt() const{
	return cbViewExt;
}

CoordBounds RootDrawer::getMapCoordBoundsExt() const{
	return cbMapExt;
}

void RootDrawer::setCoordBoundsViewExt(CoordBounds & _cb)
{
	cbViewExt = _cb;
}

void RootDrawer::setCoordBoundsZoomExt(CoordBounds & _cb)
{
	cbZoomExt = _cb;
}

void RootDrawer::setAnnotationWhitespace(bool fWhitespace)
{
	fAnnotationWhitespace = fWhitespace;
	RecomputeAnnotationBorder();
}

void RootDrawer::setAnnotationBorder(bool fBorder)
{
	fAnnotationBorder = fBorder;
	RecomputeAnnotationBorder();
}

bool RootDrawer::fWhitespace() const
{
	return fAnnotationWhitespace;
}

void RootDrawer::RecomputeAnnotationBorder(){
	if (fDrawing) { // keep "Ext" values the same during the entire "draw"
		fRequestAnnotationRecompute = true;
		return;
	}
	if (fAnnotationWhitespace || fAnnotationBorder) {
		double rLeft = 0;
		double rRight = 0;
		double rTop = 0;
		double rBottom = 0;
		if (fAnnotationWhitespace) {
			rLeft = cbZoom.width() * ext.left / 100.0;
			rRight = cbZoom.width() * ext.right / 100.0;
			rTop = cbZoom.height() * ext.top / 100.0;
			rBottom = cbZoom.height() * ext.bottom / 100;
		}
		if (fAnnotationBorder && annotationFont != 0) {
			CoordBounds cbTextExtent = annotationFont->getTextExtent("XXXXXXX");
			double width = cbTextExtent.width() + cbZoom.width() * 0.02; // textsize + space around (0.01 on each side) for centering the text
			double height = cbTextExtent.height() + cbZoom.height() * 0.02;
			rLeft += width;
			rRight += width;
			rTop += height;
			rBottom += height;
		}

		cbMapExt = CoordBounds(cbMap.MinX() - rLeft, cbMap.MinY() - rBottom, cbMap.MaxX() + rRight, cbMap.MaxY() + rTop);
		cbZoomExt.cMin.x = min(cbZoom.cMin.x, max(cbMapExt.cMin.x, cbZoom.cMin.x - rLeft));
		cbZoomExt.cMax.x = max(cbZoom.cMax.x, min(cbMapExt.cMax.x, cbZoom.cMax.x + rRight));
		cbZoomExt.cMin.y = min(cbZoom.cMin.y, max(cbMapExt.cMin.y, cbZoom.cMin.y - rBottom));
		cbZoomExt.cMax.y = max(cbZoom.cMax.y, min(cbMapExt.cMax.y, cbZoom.cMax.y + rTop));
		// compensate cbZoom x/y aspect ratio
		if (cbZoomExt.width() / cbZoomExt.height() > windowAspectRatio) { // too wide: increase height
			double delta = cbZoomExt.width() / windowAspectRatio - cbZoomExt.height();
			cbZoomExt.cMin.y -= delta / 2.0;
			cbZoomExt.cMax.y += delta / 2.0;
			cbMapExt.cMin.y = min(cbMapExt.cMin.y, cbZoomExt.cMin.y); // fill cbMap accordingly
			cbMapExt.cMax.y = max(cbMapExt.cMax.y, cbZoomExt.cMax.y);
		} else { // too high: increase width
			double delta = cbZoomExt.height() * windowAspectRatio - cbZoomExt.width();
			cbZoomExt.cMin.x -= delta / 2.0;
			cbZoomExt.cMax.x += delta / 2.0;
			cbMapExt.cMin.x = min(cbMapExt.cMin.x, cbZoomExt.cMin.x); // fill cbMap accordingly
			cbMapExt.cMax.x = max(cbMapExt.cMax.x, cbZoomExt.cMax.x);
		}
		// recenter, otherwise gray area may appear asymmetrically
		RecenterZoomHorz(cbZoomExt, cbMapExt);
		RecenterZoomVert(cbZoomExt, cbMapExt);
		
		cbViewExt.cMin.x = min(cbMapExt.cMin.x, cbZoomExt.cMin.x);
		cbViewExt.cMax.x = max(cbMapExt.cMax.x, cbZoomExt.cMax.x);
		cbViewExt.cMin.y = min(cbMapExt.cMin.y, cbZoomExt.cMin.y);
		cbViewExt.cMax.y = max(cbMapExt.cMax.y, cbZoomExt.cMax.y);
	} else {
		cbZoomExt = cbZoom;
		cbMapExt = cbMap;
		cbViewExt = cbView;
	}
}

bool RootDrawer::fUsesTopDrawer() const {
	return topDrawer != 0;
}
