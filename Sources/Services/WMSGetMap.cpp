#include "headers/toolspch.h"
#include "Engine\Base\DataObjects\ilwisobj.h"
#include "Engine\Base\System\Engine.h"
#include "Engine\Drawers\ComplexDrawer.h"
#include "Engine\Drawers\DrawerContext.h"
#include "Engine\Drawers\RootDrawer.h"
#include "Headers\version.h"
#include "Engine\Base\DataObjects\Version.h"
#include "HttpServer\IlwisServer.h"
#include "HttpServer\mongoose.h"
#include "Engine\Base\DataObjects\XMLDocument.h"
#include "httpserver\RequestHandler.h"
#include "Services\WMSGetMap.h"
#include "Engine\Map\basemap.h"
#include "Headers\messages.h"

using namespace ILWIS;
long ILWIS::WMSGetMap::idCount = 0;

RequestHandler *WMSGetMap::createHandler(struct mg_connection *c, const struct mg_request_info *request_info, const map<String, String>& kvps, IlwisServer *serv) {
	return new WMSGetMap(c, request_info,kvps, serv);
}


WMSGetMap::WMSGetMap(struct mg_connection *c, const struct mg_request_info *request_info, const map<String, String>& kvps, IlwisServer *serv) :
RequestHandler("WMSGetMapHandler", c,request_info,kvps, serv)
{
	config.add("Services", "WMSHandlers");
	init();
}

void WMSGetMap::writeResponse() const {
	RootDrawer *root = new RootDrawer();
	PreparationParameters pp(NewDrawer::ptRENDER | NewDrawer::ptGEOMETRY);
	root->prepare(&pp);
	for(int i = 0; i < layers.size(); ++i) {
		BaseMap bmp(layers[i]);
		if ( bmp.fValid()) {
			if ( IOTYPE(bmp->fnObj) == IlwisObject::iotRASMAP)
				createBaseMapDrawer(root, bmp, "RasterDataDrawer", "Ilwis38");	
			if ( IOTYPEFEATUREMAP(bmp->fnObj))
				createBaseMapDrawer(root, bmp,"FeatureDataDrawer", "Ilwis38");
		}
	}
	char buffer[500];
	GetTempPath(500, buffer);
	String tempPath("%s\\ILWIS",buffer);
	_mkdir(tempPath.c_str());
	FileName *fn = new FileName(String("%S\\p_%d.mpv",tempPath,idCount));
	root->store(*fn,""); 

	HWND handle = FindWindow(ILWIS_VERSION_NAME,0);
	PostMessage(handle, ILWM_CALLBACK,(WPARAM)fn,0); 
}


void WMSGetMap::createBaseMapDrawer(RootDrawer *rootDrawer, const BaseMap& bmp, const String& type, const String& subtype) const{

	ILWIS::DrawerParameters parms(rootDrawer, rootDrawer);
	ILWIS::NewDrawer *drawer = NewDrawer::getDrawer(type, subtype, &parms);
	drawer->addDataSource((void *)&bmp);
	rootDrawer->setCoordinateSystem(bmp->cs());
	rootDrawer->addCoordBounds(bmp->cs(), bmp->cb(), false);
	ILWIS::PreparationParameters pp(RootDrawer::ptGEOMETRY);
	drawer->prepare(&pp);
	pp.type = RootDrawer::ptRENDER;
	drawer->prepare(&pp);
	rootDrawer->addDrawer(drawer);
}

void WMSGetMap::init() {
	map<String, String>::const_iterator cur = kvps.find("layers"); 
	if ( cur == kvps.end())
		return;

	String layrs = (*cur).second;
	Array<String> parts;
	Split(layrs, parts, ",");
	int n = getConfigValue("WMS:ServiceContext:NumberOfCatalogs").iVal();
	for(int i=0; i < n; ++i) {
		String catalogDef("WMS:%S", getConfigValue(String("WMS:ServiceContext:Catalog%d",i)));
		Array<String> parts2;
		Split(catalogDef,parts2,";");

		String location = parts2[1];
		for(int j=0; j < parts.size(); ++j) {
			FileName fn ( String("%S\\%S", location, parts[j]));
			if ( fn.fExist()) {
				layers.push_back(fn);
			}
		}
		
	}
	cur = kvps.find("width"); 
	if ( cur == kvps.end())
		return;
	width = (*cur).second.iVal();
	cur = kvps.find("height"); 
	if ( cur == kvps.end())
		return;
	height = (*cur).second.iVal();
	cur = kvps.find("bbox"); 
	if ( cur == kvps.end())
		return;
	parts.resize(0);
	Split((*cur).second, parts,",");
	cb = CoordBounds(Coord(parts[0].rVal(),parts[1].rVal()),Coord(parts[2].rVal(),parts[3].rVal()));


}

long WMSGetMap::getNewId() {
	if ( WMSGetMap::idCount > 100000)
		idCount = 0;
	return (++idCount) + 4663;
}

bool WMSGetMap::needsResponse() const{
	return true;
}