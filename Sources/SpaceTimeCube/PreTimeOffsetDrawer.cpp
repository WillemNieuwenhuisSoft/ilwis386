#include "Headers\toolspch.h"
#include "PreTimeOffsetDrawer.h"
#include "Engine\Drawers\DrawerContext.h"

using namespace ILWIS;

ILWIS::NewDrawer *createPreTimeOffsetDrawer(DrawerParameters *parms) {
	return new PreTimeOffsetDrawer(parms);
}

PreTimeOffsetDrawer::PreTimeOffsetDrawer(DrawerParameters *parms)
: ComplexDrawer(parms,"PreTimeOffsetDrawer")
, timeOffset(0)
{
}

PreTimeOffsetDrawer::~PreTimeOffsetDrawer() {
}

void PreTimeOffsetDrawer::prepare(PreparationParameters *pp) {
	if ((pp->type & RootDrawer::ptGEOMETRY) || (pp->type & NewDrawer::ptRESTORE)) { 
		cube = rootDrawer->getMapCoordBounds();
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
	}
}

void PreTimeOffsetDrawer::SetTime(double time) {
	timeOffset = time;
	getRootDrawer()->getDrawerContext()->doDraw();
}

bool PreTimeOffsetDrawer::draw(const CoordBounds& cbArea) const{
	if ( !isActive() || !isValid())
		return false;
	glPushMatrix();
	glTranslatef(0, 0, cube.altitude() * timeOffset);

	return true;
}
