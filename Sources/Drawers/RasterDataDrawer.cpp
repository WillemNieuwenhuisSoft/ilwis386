#include "Headers\toolspch.h"
#include "Engine\Map\basemap.h"
#include "Engine\Map\Point\ilwPoint.h"
#include "Engine\Drawers\ComplexDrawer.h"
#include "Engine\Spatialreference\gr.h"
#include "Engine\Map\Raster\Map.h"
#include "Engine\Base\System\RegistrySettings.h"
#include "Engine\Drawers\RootDrawer.h"
#include "Engine\Drawers\SpatialDataDrawer.h"
#include "Drawers\LayerDrawer.h"
#include "Drawers\RasterDataDrawer.h"
#include "Engine\Drawers\ZValueMaker.h"
#include "Drawers\RasterLayerDrawer.h"
#include "drawers\DrawingColor.h"
#include "Engine\Domain\dmsort.h"

using namespace ILWIS;

ILWIS::NewDrawer *createRasterDataDrawer(DrawerParameters *parms) {
	return new RasterDataDrawer(parms);
}

RasterDataDrawer::RasterDataDrawer(DrawerParameters *parms) : 
	SpatialDataDrawer(parms,"RasterDataDrawer")
{
	setAlpha(1);
}

RasterDataDrawer::~RasterDataDrawer(){
}

void RasterDataDrawer::prepare(PreparationParameters *pp){
	SpatialDataDrawer::prepare(pp);
	if ( pp->type & RootDrawer::ptGEOMETRY ) {
		if ( !(pp->type & NewDrawer::ptANIMATION))
			clear();
		BaseMapPtr *bmptr = getBaseMap();
		BaseMap basemap;
		basemap.SetPointer(bmptr);
		RasterLayerDrawer *rsd;
		ILWIS::DrawerParameters dp(getRootDrawer(), this);
		IlwisObject::iotIlwisObjectType otype = IlwisObject::iotObjectType(basemap->fnObj);
		if ( otype == IlwisObject::iotRASMAP) {
				if ( mpl.fValid() )
					rsd = (RasterLayerDrawer *)NewDrawer::getDrawer("ColorCompositeDrawer", pp, &dp);
				else if ( ((MapPtr*)basemap.ptr())->gr()->pgOSM())
					rsd = (RasterLayerDrawer *)NewDrawer::getDrawer("OpenstreetmapDrawer", pp, &dp);
				else if ( ((MapPtr*)basemap.ptr())->gr()->pgWMS())
					rsd = (RasterLayerDrawer *)NewDrawer::getDrawer("WMSDrawer", pp, &dp);
				else
					rsd = (RasterLayerDrawer *)NewDrawer::getDrawer("RasterLayerDrawer", pp, &dp);
				RangeReal rrMinMax (0, 255);
				Domain dm = basemap->dm();
				if (dm.fValid() && (dm->pdbit() || dm->pdbool()))
					rrMinMax = RangeReal(1,2);
				else if ( basemap->dm()->pdv()) {
					if (mpl.fValid()) { // ColorComposite; the (total) rrMinMax is not used, but here we force the calculation of MinMax for all bands in the maplist
						for (int i = 0; i < mpl->iSize(); ++i) {
							RangeReal rrMinMaxMap = mpl->map(i)->rrMinMax(BaseMapPtr::mmmCALCULATE);
							if (rrMinMaxMap.rLo() > rrMinMaxMap.rHi())
								rrMinMaxMap = mpl->map(i)->vr()->rrMinMax();
							if (i > 0)
								rrMinMax += rrMinMaxMap;
							else
								rrMinMax = rrMinMaxMap;
						}
					} else {
						rrMinMax = basemap->rrMinMax(BaseMapPtr::mmmCALCULATE); // not mmmSAMPLED here, to get a more accurate result, otherwise there's a high chance of artifacts since the sampling is only done on this one band
						if (rrMinMax.rLo() > rrMinMax.rHi())
							rrMinMax = basemap->vr()->rrMinMax();
					}
				} else {
					if ( rsd->useAttributeColumn()) {
						if (rsd->getAtttributeColumn()->dm()->pdv())
							rrMinMax = rsd->getAtttributeColumn()->vr()->rrMinMax();
						else if (rsd->getAtttributeColumn()->dm()->pdsrt())
							rrMinMax = RangeReal(1, rsd->getAtttributeColumn()->dm()->pdsrt()->iSize());
					} else if (dm->pdsrt())
						rrMinMax = RangeReal(1, dm->pdsrt()->iSize());
				}
				rsd->setMinMax(rrMinMax);
				rsd->SetPaletteOwner(); // this set has the only available palette
				addLayerDrawer(basemap,pp,rsd);
		}
	}
	if ( pp->type & RootDrawer::ptRESTORE) {
		BaseMapPtr *bmptr = getBaseMap();
		BaseMap basemap;
		basemap.SetPointer(bmptr);
		RasterLayerDrawer *rsd = (RasterLayerDrawer*)getDrawer(0);
		if ( rsd ) {
			rsd->addDataSource(mpl.fValid() ? (void *)&mpl : (void *)&basemap);  // color composte or normal
			RangeReal rrMinMax (0, 255);
			Domain dm = basemap->dm();
			if (dm.fValid() && (dm->pdbit() || dm->pdbool()))
				rrMinMax = RangeReal(1,2);
			else if ( basemap->dm()->pdv()) {
				rrMinMax = basemap->rrMinMax(BaseMapPtr::mmmCALCULATE); // not mmmSAMPLED here, to get a more accurate result, otherwise there's a high chance of artifacts since the sampling is only done on this one band
				if (rrMinMax.rLo() > rrMinMax.rHi())
					rrMinMax = basemap->vr()->rrMinMax();
			} else {
				if ( rsd->useAttributeColumn()) {
					if (rsd->getAtttributeColumn()->dm()->pdv())
						rrMinMax = rsd->getAtttributeColumn()->vr()->rrMinMax();
					else if (rsd->getAtttributeColumn()->dm()->pdsrt())
						rrMinMax = RangeReal(1, rsd->getAtttributeColumn()->dm()->pdsrt()->iSize());
				} else if (dm->pdsrt())
					rrMinMax = RangeReal(1, dm->pdsrt()->iSize());
			}
			rsd->setMinMax(rrMinMax);
			rsd->SetPaletteOwner(); // this set has the only available palette
		}
	}
	if ( pp->type & RootDrawer::ptRENDER || pp->type & RootDrawer::ptRESTORE) {
		for(int i = 0; i < drawers.size(); ++i) {
			RasterDataDrawer *rsd = (RasterDataDrawer *)drawers.at(i);
			PreparationParameters fp((int)pp->type, 0);
			rsd->prepare(&fp);
		}
	}
}

void RasterDataDrawer::addLayerDrawer(const BaseMap& basemap,PreparationParameters *pp,ILWIS::LayerDrawer *rsd, const String& name) {
	PreparationParameters fp((int)pp->type, 0);
	//fp.rootDrawer = getRootDrawer();
	//fp.parentDrawer = this;
	fp.csy = basemap->cs();
	rsd->setName(name);
	rsd->setRepresentation(basemap->dm()->rpr()); //  default choice
	rsd->getZMaker()->setSpatialSource(basemap, getRootDrawer()->getMapCoordBounds());
	//rsd->getZMaker()->setDataSourceMap(basemap);
	rsd->addDataSource(mpl.fValid() ? (void *)&mpl : (void *)&basemap);  // color composte or normal
	if (rsd->getDrawMethod() == drmNOTSET)
		rsd->setDrawMethod(drmINIT);
	rsd->prepare(&fp);
	addDrawer(rsd);
}

void RasterDataDrawer::addDataSource(void *bmap, int options){
	SpatialDataDrawer::addDataSource(bmap, options);
}

bool RasterDataDrawer::draw(int drawerIndex, const DrawLoop drawLoop, const CoordBounds& cbArea) const{
	SpatialDataDrawer::draw(drawLoop, cbArea);
	return true;
}

void RasterDataDrawer::setTresholdColor(const Color&clr){
	for(int i=0; i < drawers.size(); ++i) {
		RasterLayerDrawer *rsd = dynamic_cast<RasterLayerDrawer *> (drawers[i]);
		if ( rsd) {
			rsd->getDrawingColor()->setTresholdColor(clr);
			return;
		}
	}
}

void RasterDataDrawer::setTresholdRange(const RangeReal& rr, bool single){
		for(int i=0; i < drawers.size(); ++i) {
		RasterLayerDrawer *rsd = dynamic_cast<RasterLayerDrawer *> (drawers[i]);
		if ( rsd) {
			rsd->getDrawingColor()->setTresholdRange(rr, single);
			return;
		}
	}
}

String RasterDataDrawer::store(const FileName& fnView, const String& section) const{
	String currentSection = section + ":RasterData";
	SpatialDataDrawer::store(fnView, currentSection);

	return currentSection;
}

void RasterDataDrawer::load(const FileName& fnView, const String& section){
	SpatialDataDrawer::load(fnView, section);
}
