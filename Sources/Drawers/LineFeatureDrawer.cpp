#include "Headers\toolspch.h"
#include "Engine\Map\basemap.h"
#include "Engine\Drawers\DrawerContext.h"
#include "Engine\Map\Segment\seg.h"
#include "Engine\Drawers\drawer_n.h"
#include "Engine\Drawers\SimpleDrawer.h" 
#include "Engine\Spatialreference\gr.h"
#include "Engine\Map\Raster\Map.h"
#include "Engine\Base\System\RegistrySettings.h"
#include "Engine\Drawers\RootDrawer.h"
#include "Engine\Drawers\SpatialDataDrawer.h"
#include "Drawers\DrawingColor.h" 
#include "Drawers\featureDatadrawer.h"
#include "Drawers\LayerDrawer.h"
#include "Drawers\FeatureLayerDrawer.h"
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
	SpatialDataDrawer *datadrw = dynamic_cast<SpatialDataDrawer *>(getParentDrawer()->getParentDrawer());
	if ( datadrw) {
		dmt = datadrw->getBaseMap()->dm()->dmt();
	}
}


void LineFeatureDrawer::prepare(PreparationParameters *p){
	LineDrawer::prepare(p);
	FeatureLayerDrawer *fdr = dynamic_cast<FeatureLayerDrawer *>(parentDrawer);
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
	if ( p->type & NewDrawer::pt3D || p->type & ptRESTORE) {
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
		LineProperties *lparent = (LineProperties *)fdr->getProperties();
		lproperties.linestyle = lparent->linestyle;
		bool isDmSort = dmt == dmtCLASS || dmt == dmtID || dmt == dmtUNIQUEID;
		if (( isDmSort && feature->rValue() == 0) || feature->rValue() == rUNDEF) // 0 == undef for classes
			lproperties.linestyle = 0xF0F0;
		lproperties.thickness = lparent->thickness;
		lproperties.drawColor = (fdr->getDrawingColor()->clrRaw(feature->iValue(), fdr->getDrawMethod()));
		for(int j =0 ; j < p->filteredRaws.size(); ++j) {
			int raw = p->filteredRaws[j];
			if ( getFeature()->rValue() == abs(raw)) {
				setActive(raw > 0);
			}
		}
		if ( specialOptions == 0)
			specialOptions = fdr->getSpecialDrawingOption();
		double tr = fdr->getTransparency();
		setTransparency(tr);
		extrTransparency = fdr->getExtrusionTransparency();
	}
}

Feature *LineFeatureDrawer::getFeature() const {
	return feature;
}