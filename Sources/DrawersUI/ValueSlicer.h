#pragma once

class ValueSlicerSlider;

namespace ILWIS{
	class DrawingColor;
}

#define ID_TIME_TICK WM_USER+345

class ValueSlicer : public CStatic, public BaseZapp {
public:
   ValueSlicer(ValueSlicerSlider *f, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
   void DrawItem(LPDRAWITEMSTRUCT lpDIS);
   void OnLButtonUp(UINT nFlags, CPoint point);
   void OnLButtonDown(UINT nFlags, CPoint point);
   void OnLButtonDblClk(UINT nFlags, CPoint point);
   void OnMouseMove(UINT nFlags, CPoint point);
   	CRect makeDrawRect();
	void updateRepresentations();
	void setRprBase(const Representation& rprB);
  /* int OnToolHitTest(CPoint point, TOOLINFO *pTI) const;
   void OnToolTipNotify(NMHDR *pNMHDR, LRESULT *pResult);*/

   DECLARE_MESSAGE_MAP();
private:
	void drawRprBase(LPDRAWITEMSTRUCT lpDIS, const CRect rct);
	void drawRpr(LPDRAWITEMSTRUCT lpDIS, const CRect rct);
	bool moveValue(int index, double v);
	ValueSlicerSlider *fldslicer;
	vector<int> ylimits;
	CPoint activePoint;
	Representation rprBase;
	int iDragIndex;
};

class _export ValueSlicerSlider : public FormEntry {
	friend class ValueSlicer;
public:
	ValueSlicerSlider(FormEntry* par, ILWIS::LayerDrawer *sdrw);
	~ValueSlicerSlider();
	void setValueRange(const ValueRange& tbl);
	void setBound(int index, double up);
	void LayerDrawer(ILWIS::LayerDrawer *drwc);
	void setNumberOfBounds(int n);
	void create();
	bool isValid() const;
	void setHighColor(Color c);
	void setLowColor(Color c);
	void setRprBase(const Representation& rprB);
	Representation getRpr() const;
	void setFileNameRpr(const FileName& fn);
	
private:
	void init();
	void insertLimit(double rValue, const Color& clrPrev, const Color& clrNext);
	int insertBound(double rValue);
	Color getColor(int i);
	ValueSlicer *valueslicer;
	ValueRange valrange;
	ILWIS::LayerDrawer *drawer;
	ILWIS::DrawingColor *drawingcolor;
	RepresentationGradual *rprgrad;
	Representation rpr;
	vector<double> bounds;
	vector<int> transparency;
	int selectedIndex;
	Color lowColor;
	Color highColor;
	Representation rprBase;
	int nrBounds;
};


//----------------------------
class SlicingStepColor : public FormWithDest {
public:
	SlicingStepColor(CWnd* parent, Color* clrLow, double *v1, bool _fv1Enabled, Color*clrHigh, double *v2, bool _fv2Enabled);
	virtual BOOL OnInitDialog();
private:
	FieldReal *fr1;
	FieldReal *fr2;
	bool fv1Enabled;
	bool fv2Enabled;
};