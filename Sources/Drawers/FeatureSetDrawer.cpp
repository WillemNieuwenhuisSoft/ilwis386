#include "Client\Headers\formelementspch.h"
#include "Engine\Map\basemap.h"
#include "Engine\Map\Point\ilwPoint.h"
#include "Client\MapWindow\Drawers\ComplexDrawer.h"
#include "Client\Mapwindow\Drawers\SimpleDrawer.h" 
#include "Client\Ilwis.h"
#include "Client\FormElements\FieldRealSlider.h"
#include "Engine\Spatialreference\gr.h"
#include "Engine\Map\Raster\Map.h"
#include "Client\Mapwindow\MapPaneViewTool.h"
#include "Client\Mapwindow\Drawers\DrawerTool.h"
#include "Engine\Base\System\RegistrySettings.h"
#include "Client\Mapwindow\MapPaneView.h"
#include "Engine\Map\basemap.h"
#include "Client\Mapwindow\MapCompositionDoc.h"
#include "Client\Mapwindow\Drawers\RootDrawer.h"
#include "Client\Mapwindow\Drawers\AbstractMapDrawer.h"
#include "Drawers\FeatureLayerDrawer.h"
#include "Drawers\SetDrawer.h"
#include "Drawers\FeatureSetDrawer.h"
#include "Client\Mapwindow\Drawers\ZValueMaker.h"
#include "Client\Mapwindow\Drawers\DrawerContext.h"
#include "Drawers\AnimationDrawer.h"
//#include "Client\Mapwindow\Drawers\PointMapDrawerForm.h"
#include "Headers\Hs\Drwforms.hs"

using namespace ILWIS;

FeatureSetDrawer::FeatureSetDrawer(DrawerParameters *parms, const String& name) : 
	SetDrawer(parms,name),
	singleColor(Color(0,176,20)),
	useMask(false)
{
	setDrawMethod(drmNOTSET); // default
	setInfo(false);
}

FeatureSetDrawer::~FeatureSetDrawer() {

}

void FeatureSetDrawer::addDataSource(void *bmap,int options) {
	fbasemap.SetPointer((BaseMapPtr *)bmap);
}

void *FeatureSetDrawer::getDataSource() const {
	return (void *)&fbasemap;
}

void FeatureSetDrawer::getFeatures(vector<Feature *>& features) const {
	features.clear();
	int numberOfFeatures = fbasemap->iFeatures();
	features.resize(numberOfFeatures);
	for(int i=0; i < numberOfFeatures; ++i) {
		Feature *feature = CFEATURE(fbasemap->getFeature(i));
		features.at(i) = feature;
	}
}

void FeatureSetDrawer::prepare(PreparationParameters *parms){
	/*if ( !isActive())
		return;*/

	clock_t start = clock();
	SetDrawer::prepare(parms);
	FeatureLayerDrawer *mapDrawer = (FeatureLayerDrawer *)parentDrawer;
	if ( getName() == "Unknown")
		setName(mapDrawer->getBaseMap()->sName());
	vector<Feature *> features;
	if ( parms->type & RootDrawer::ptGEOMETRY | parms->type & NewDrawer::ptRESTORE){
		bool isAnimation = mapDrawer->getType() == "AnimationDrawer";
		if ( isAnimation ) {
			getFeatures(features);
		} else {
			mapDrawer->getFeatures(features);
		}
		clear();
		drawers.resize( features.size());
		for(int i=0; i<drawers.size(); ++i)
			drawers.at(i) = 0;
		int count = 0;
		Tranquilizer trq(TR("preparing data"));
		for(int i=0; i < features.size(); ++i) {
			Feature *feature = features.at(i);
			NewDrawer *pdrw;
			if ( feature && feature->fValid()){
				ILWIS::DrawerParameters dp(getRootDrawer(), this);
				pdrw = createElementDrawer(parms, &dp);
				pdrw->addDataSource(feature);
				PreparationParameters fp((int)parms->type, 0);
				pdrw->prepare(&fp);
				setDrawer(i, pdrw);
				++count;
				if ( i % 100 == 0) {
					trq.fUpdate(i,features.size()); 
				}
			}
		}
		if ( count < features.size())
			drawers.resize(count);
	} else {
		if ( parms->type & NewDrawer::ptRENDER || parms->type & NewDrawer::pt3D) {
			prepareChildDrawers(parms);
		}
	}
	clock_t end = clock();
	double duration = 1000.0 * (double)(end - start) / CLOCKS_PER_SEC;
	TRACE("Prepared in %2.2f seconds;\n", duration/1000);
}

String FeatureSetDrawer::getMask() const{
	return mask;
}

void FeatureSetDrawer::setMask(const String& sm){
	mask = sm;
}

void FeatureSetDrawer:: setSingleColor(const Color& c){
	singleColor = c;
	setDrawMethod(drmSINGLE);
}

Color FeatureSetDrawer::getSingleColor() const {
	return singleColor;
}

String FeatureSetDrawer::store(const FileName& fnView, const String& parentSection) const{
	SetDrawer::store(fnView, parentSection);
	ObjectInfo::WriteElement(parentSection.scVal(),"SingleColor",fnView, singleColor);
	return parentSection;
}

void FeatureSetDrawer::load(const FileName& fnView, const String& parentSection){
	SetDrawer::load(fnView, parentSection);
	ObjectInfo::ReadElement(parentSection.scVal(),"SingleColor",fnView, singleColor);

}

