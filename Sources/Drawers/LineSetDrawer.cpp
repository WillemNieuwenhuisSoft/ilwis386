#include "Client\Headers\formelementspch.h"
#include "Engine\Map\basemap.h"
#include "Engine\Map\Point\ilwPoint.h"
#include "Client\MapWindow\Drawers\drawer_n.h"
#include "Client\Mapwindow\Drawers\SimpleDrawer.h" 
#include "Client\Ilwis.h"
#include "Engine\Base\System\RegistrySettings.h"
#include "Client\Mapwindow\MapCompositionDoc.h"
#include "Client\Mapwindow\Drawers\RootDrawer.h"
#include "Client\Mapwindow\Drawers\AbstractObjectdrawer.h"
#include "Client\Mapwindow\Drawers\AbstractMapDrawer.h"
#include "Client\Mapwindow\LayerTreeView.h"
#include "Client\Mapwindow\LayerTreeItem.h" 
#include "Client\Mapwindow\Drawers\FeatureDrawer.h"
#include "Client\Mapwindow\Drawers\SetDrawer.h"
#include "Client\Mapwindow\Drawers\FeatureSetDrawer.h"
#include "Drawers\LineSetDrawer.h"
//#include "Client\Mapwindow\Drawers\PointMapDrawerForm.h"
#include "Headers\Hs\Drwforms.hs"

using namespace ILWIS;

ILWIS::NewDrawer *createLineSetDrawer(DrawerParameters *parms) {
	return new LineSetDrawer(parms);
}

LineSetDrawer::LineSetDrawer(DrawerParameters *parms) : 
	FeatureSetDrawer(parms,"LineSetDrawer")
{
	setDrawMethod(drmSINGLE); // default;
}

LineSetDrawer::~LineSetDrawer() {
}

NewDrawer *LineSetDrawer::createElementDrawer(PreparationParameters *pp, ILWIS::DrawerParameters* parms) const{
	return IlwWinApp()->getDrawer("LineDrawer",pp, parms);

}

HTREEITEM LineSetDrawer:: configure(LayerTreeView  *tv, HTREEITEM parent) {
	HTREEITEM hti = FeatureSetDrawer::configure(tv,parent);
	if ( getUICode() == NewDrawer::ucBOUNDARIES)
		setSingleColor(Color(0,0,0));
	return hti;
}

void LineSetDrawer::setDrawMethod(DrawMethod method) {
	if ( method == drmINIT) {
		if ( useInternalDomain() || !rpr.fValid())
			setDrawMethod(drmSINGLE);
		else 
			setDrawMethod(drmRPR);

	} else
		drm = method;
}
