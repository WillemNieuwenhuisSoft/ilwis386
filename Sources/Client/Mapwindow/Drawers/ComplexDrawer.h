#pragma once

#include "drawer_n.h" 

typedef map<String, ILWIS::NewDrawer *> DrawerMap;
typedef DrawerMap::iterator DrawerIter;
typedef DrawerMap::const_iterator DrawerIter_C;

class FieldIntSliderEx;
class MapPaneViewTool;
class ZoomableView;

namespace ILWIS {

class ZValueMaker;

class _export ComplexDrawer : public NewDrawer {
		friend class TransparencyForm;

	public:
		enum DrawerType{dtPOST=1, dtPRE=2, dtMAIN=4};
		String getType() const;
		RootDrawer *getRootDrawer();
		RootDrawer *getRootDrawer() const;
		bool draw(bool norecursion = false, const CoordBounds& cb=CoordBounds()) const;
		void prepare(PreparationParameters *);
		virtual void prepareChildDrawers(PreparationParameters *);
		int getDrawerCount(int types=dtMAIN) const;
		NewDrawer * getDrawer(int index, DrawerType type=dtMAIN);
		NewDrawer *getDrawer(const String& did);
		virtual String addDrawer(NewDrawer *drw);
		virtual void setDrawer(int index, NewDrawer *drw);
		virtual int getDrawerIndex(NewDrawer *drw);
		virtual void removeDrawer(const String& did, bool dodelete=true);
		virtual void insertDrawer(int index, NewDrawer *drw);
		virtual void addPostDrawer(int order, NewDrawer *drw);
		virtual void addPreDrawer(int order, NewDrawer *drw);
		void getDrawers(vector<NewDrawer *>&) ;
		virtual void addDataSource(void *,int options=0){}
		virtual void* getDataSource() const {return 0;}
		virtual void removeDataSource(void *) {}
		virtual void clear();
		String getId() const;
		String getName() const;
		virtual void setName(const String&);
		bool isActive() const;
		void setActive(bool yesno);
		DrawMethod getDrawMethod() const;
		void setDrawMethod(DrawMethod method=drmINIT);
		virtual bool isEditable() const;
		virtual void setEditable(bool yesno);
		bool hasInfo() const;
		void setInfo(bool yesno);
		HTREEITEM  configure(LayerTreeView  *tv, HTREEITEM parent);
		void setActiveMode(void *v,LayerTreeView *tv);
		NewDrawer *getParentDrawer() const;
		double getTransparency() const;
		void setTransparency(double value);
		virtual int getUICode() const;
		virtual void setUICode(int c);
		virtual HTREEITEM make3D(bool yeno, LayerTreeView  *tvm);
		bool is3D() const;
		ZValueMaker *getZMaker() const;
		virtual void timedEvent(UINT timerid);
		virtual String description() const { return "?";}
		virtual String iconName(const String& subtype="?") const { return "?";}
		virtual void setSpecialDrawingOptions(int option, bool add, vector<Coord>* coords=NULL) ;
		int getSpecialDrawingOption(int opt=sdoNone) const;
		bool isSimple() const { return false;}
		bool isDirty() const;
		void setDirty(bool yesno);
		String getInfo(const Coord& crd) const;
		//Tranquilizer getTranquilizer() { return trq; }
		long getCurrentIndex() const;
		void setCurrentIndex(long i);
		virtual void getDrawerFor(const Feature* feature,vector<NewDrawer *>& features);
		void shareVertices(vector<Coord *>& coords);
		bool inEditMode() const;
		void setEditMode(bool yesno);
		void drawLegendItem(CDC *dc, const CRect& rct, double rVal) const {}
		HTREEITEM InsertItem(const String& name,const String& icon, DisplayOptionTreeItem *item=0, int checkstatus = -1, HTREEITEM after=TVI_LAST);
		HTREEITEM InsertItem(LayerTreeView  *tv,HTREEITEM parent, const String& name,const String& icon, HTREEITEM after=TVI_LAST);
		HTREEITEM findTreeItemByName(LayerTreeView  *tv, HTREEITEM parent, const String& name) const;
		virtual MapPaneViewTool *createTool(ZoomableView*) { return 0;}
	
	protected:
		vector<NewDrawer *> drawers;
		DrawerMap postDrawers;
		DrawerMap preDrawers;
		DrawerMap drawersById;
		//Tranquilizer trq;
		long currentIndex; // for tranquilizer purposes;

		int uiCode;
		bool info;
		DrawMethod drm;
		double transparency;
		RootDrawer *rootDrawer;
		NewDrawer *parentDrawer;
		String type;
		String id;
		String name;
		bool active;
		bool editable;
		bool editmode;
		ZValueMaker *zmaker;
		bool threeD;
		HTREEITEM itemTransparent;
		int specialOptions;
		bool dirty;

		
		ComplexDrawer(DrawerParameters *context, const String& ty);
		ComplexDrawer();
		virtual ~ComplexDrawer();
		String store(const FileName& fnView, const String& parenSection) const;
		void load(const FileName& fnView, const String& parenSection);
		void displayOptionTransparency(CWnd *parent) ;
		void setInfoMode(void *v,LayerTreeView *tv);
		NewDrawer *loadDrawer(const FileName& fnView, const String& drawerSection);
		bool drawPreDrawers(bool norecursion, const CoordBounds& cb) const;
		bool drawPostDrawers(bool norecursion, const CoordBounds& cb) const;
	private:
		void init();
	} ;

	class _export DisplayOptionsForm : public FormBaseDialog {
	public:
		DisplayOptionsForm(ComplexDrawer *dr,CWnd *par, const String& title);
		afx_msg virtual void OnCancel();
		int exec();
		virtual void apply();
	protected:
		void updateMapView();
		ComplexDrawer *drw;
		LayerTreeView *view;
	};

	class _export DisplayOptionsForm2 : public FormBaseDialog {
	public:
		DisplayOptionsForm2(ComplexDrawer *dr,CWnd *par, const String& title);
		int exec();
	protected:
		void updateMapView();
		ComplexDrawer *drw;
		LayerTreeView *view;
	};

	class TransparencyForm : public DisplayOptionsForm {
		public:
		TransparencyForm(CWnd *wPar, ComplexDrawer *dr);
		void apply(); 
	private:
		int setTransparency(Event *ev);

		int transparency;
		FieldIntSliderEx *slider;
	};
}