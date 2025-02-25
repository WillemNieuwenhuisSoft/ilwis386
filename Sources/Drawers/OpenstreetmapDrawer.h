#pragma once

ILWIS::NewDrawer *createOpenstreetmapDrawer(ILWIS::DrawerParameters *parms);

namespace ILWIS {

	class OpenstreetmapData {
	public:
		CoordBounds cbFullExtent;
	};

	class _export OpenstreetmapDrawer : public RasterLayerDrawer {
	public:
		OpenstreetmapDrawer(DrawerParameters *parms);
		virtual ~OpenstreetmapDrawer();
		virtual bool draw(const DrawLoop drawLoop, const CoordBounds& cbArea) const;
		virtual String getInfo(const Coord& c) const;
	protected:
		void DisplayImagePortion(CoordBounds &cb, unsigned int zoomLevel) const;
		void DisplayTexture(CoordBounds & cb) const;
		void DisplayTexture3D(CoordBounds & cb) const;
		virtual void setData() const;
		virtual void init() const;
		virtual void setup();
		virtual void *getDataSource() const;
		OpenstreetmapData * openstreetmapData;
	};
}