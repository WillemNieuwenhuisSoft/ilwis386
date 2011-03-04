#include "Headers\toolspch.h"
#include "Engine\Map\basemap.h"
#include "Engine\Drawers\DrawerContext.h"
#include "Engine\Map\Segment\seg.h"
#include "Engine\Drawers\drawer_n.h"
#include "Engine\Drawers\SimpleDrawer.h" 
#include "Engine\Drawers\TextDrawer.h"
#include "Engine\Spatialreference\gr.h"
#include "Engine\Map\Raster\Map.h"
#include "Engine\Base\System\RegistrySettings.h"
#include "Engine\Drawers\RootDrawer.h"
#include "Engine\Drawers\AbstractMapDrawer.h"
#include "Drawers\DrawingColor.h" 
#include "Drawers\featurelayerdrawer.h"
#include "Drawers\SetDrawer.h"
#include "Drawers\FeatureSetDrawer.h"
#include "drawers\linedrawer.h"
#include "drawers\linefeaturedrawer.h"
#include "Engine\Drawers\ZValueMaker.h"

using namespace ILWIS;

ILWIS::NewDrawer *createLineFeatureDrawer(DrawerParameters *parms) {
	return new LineFeatureDrawer(parms);
}

LineFeatureDrawer::LineFeatureDrawer(DrawerParameters *parms) : 
	LineDrawer(parms,"LineFeatureDrawer"), feature(0)
{
	lproperties.drawColor = Color(0,167,18);
}

LineFeatureDrawer::LineFeatureDrawer(DrawerParameters *parms, const String& name) : 
	LineDrawer(parms,name),feature(0)
{
}

LineFeatureDrawer::~LineFeatureDrawer() {
}

bool LineFeatureDrawer::draw( const CoordBounds& cbArea) const {
	return LineDrawer::draw( cbArea);
}

void LineFeatureDrawer::addDataSource(void *f, int options) {
	feature = (Feature *)f;

}


void LineFeatureDrawer::prepare(PreparationParameters *p){
	LineDrawer::prepare(p);
	FeatureSetDrawer *fdr = dynamic_cast<FeatureSetDrawer *>(parentDrawer);
	if ( p->type & ptGEOMETRY | p->type & ptRESTORE) {
		CoordSystem csy = fdr->getCoordSystem();
		clear();
		cb = feature->cbBounds();
		feature->getBoundaries(lines);
		bool sameCsy = getRootDrawer()->getCoordinateSystem()->fEqual(*(csy.ptr()));
		if ( !sameCsy ) {
			cb = CoordBounds();
			for(int j = 0; j < lines.size(); ++j) {
				CoordinateSequence *seq = lines.at(j);
		
				for(int  i = 0; i < seq->size(); ++i) {
					Coord cOld = seq->getAt(i);
					Coord c = getRootDrawer()->getCoordinateSystem()->cConv( csy, cOld);
					c.z = cOld.z;
					cb += c;
					seq->setAt(c,i);
				}
			}
		}
	}
	if ( p->type & NewDrawer::pt3D) {
		ZValueMaker *zmaker = ((ComplexDrawer *)parentDrawer)->getZMaker();
		for(int j = 0; j < lines.size(); ++j) {
			CoordinateSequence *seq = lines.at(j);
			for(int  i = 0; i < seq->size(); ++i) {
				Coord c = seq->getAt(i);
				double zv = zmaker->getValue(c,feature);
				c.z = zv;
				seq->setAt(c,i);
			}
		}
	}
	if (  p->type & RootDrawer::ptRENDER || p->type & ptRESTORE) {
		lproperties.drawColor = (fdr->getDrawingColor()->clrRaw(feature->iValue(), fdr->getDrawMethod()));
		double tr = fdr->getTransparency();
		setTransparency(tr);
		extrTransparency = fdr->getExtrusionTransparency();
	}
}

Feature *LineFeatureDrawer::getFeature() const {
	return feature;
}