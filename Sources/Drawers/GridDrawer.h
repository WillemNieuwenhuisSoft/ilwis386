#pragma once


ILWIS::NewDrawer *createGridDrawer(ILWIS::DrawerParameters *parms);
ILWIS::NewDrawer *createGridLine(ILWIS::DrawerParameters *parms);

class FieldColor;
enum LineDspType;

namespace ILWIS{

	class _export GridDrawer : public ComplexDrawer {
	public:
		enum Mode{mGRID=1, mPLANE=2, mMARKERS=4,mAXIS=8,mVERTICALS=16, mCUBE=32, mGROUNDLEVEL=64};
		ILWIS::NewDrawer *createGridDrawer(DrawerParameters *parms);

		GridDrawer(DrawerParameters *parms);
		virtual ~GridDrawer();
		bool draw(const DrawLoop drawLoop, const CoordBounds& cbArea=CoordBounds()) const;
		void prepare(PreparationParameters *pp);
		bool is3DGrd() const { return threeDGrid; }
		int getMode() const;
		void setMode(int m);
		double getZSpacing() const;
		void setZSpacing(double z);
		Color getPlaneColor() const;
		void setPlaneColor(Color clr);
		void set3DGrid(bool yesno);
		double getGridSpacing() const;
		void setGridSpacing(double v);
		GeneralDrawerProperties *getProperties();
		void setNumberOfplanes(int n);
		int getNumberOfPlanes() const;
		void setTransparencyPlane( double v);
		double getAlphaPlane() const;
		void setBounds(const CoordBounds&);


	protected:
		void AddLinearGridLine(Coord c1, Coord c2);
		void AddCurvedGridLine(Coord c1, Coord c2);
		void gridOptions(CWnd *parent);
		void displayOptionGrid3D(CWnd *parent);
		void prepareGrid( double rDist, const Coord& cMin, const Coord& cMax, bool fLinear );
		void prepareVerticals( double rDist,const Coord& cMin, const Coord& cMax);
		String store(const FileName& fnView, const String& parenSection) const;
		void load(const FileName& fnView, const String& parenSection);
		bool drawPlane(const CoordBounds& cbArea) const;
		void resizeQuadsVector(int planes);
		void prepareVAxis(double rDist,const Coord& cMin, const Coord& cMax, bool fLinear);
		void preparePlanes(double rDist, const Coord& cMin, const Coord& cMax );
		void prepareCube(double rDist, const Coord& cMin, const Coord& cMax, bool fLinear );
		void prepareChildDrawers(PreparationParameters *parms);
		void getLayerDistances(vector<double>& dist);
		void calcBounds(const GeoRef& grf, const CoordBounds& cbMap, Coord& cMin, Coord& cMax);

		double          rDist;
		LineProperties	lproperties;
		bool			threeDGrid;
		int				mode;
		Color			planeColor;
		int				noOfPlanes;
		vector< Coord * >   planeQuads;
		vector<double> planeDistances;
		CoordBounds bounds;
		CCriticalSection *csDraw;
	};

	class GridLine: public LineDrawer {
	public:
		GridLine(DrawerParameters *parms);
		virtual ~GridLine();
		bool draw(const DrawLoop drawLoop, const CoordBounds& cbArea=CoordBounds()) const;
		void prepare(PreparationParameters *pp);
		void addDataSource(void *c, int options = 0);
	private:
	};


}