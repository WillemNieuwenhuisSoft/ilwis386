#ifndef TIMEPROFILETOOL_H
#define TIMEPROFILETOOL_H

ILWIS::DrawerTool *createTimeProfileTool(ZoomableView* zv, LayerTreeView *view, ILWIS::NewDrawer *drw);

#include "Client\FormElements\FieldGraph.h"
#include "Engine\Base\Algorithm\SimpleFunction.h"

class InfoLine;

namespace ILWIS {

	class TimeProfileTool : public DrawerTool {
	public:
		TimeProfileTool(ZoomableView* zv, LayerTreeView *view, NewDrawer *drw);
		virtual ~TimeProfileTool();
		bool isToolUseableFor(ILWIS::DrawerTool *tool);
		HTREEITEM configure( HTREEITEM parentItem);
		String getMenuString() const;
	private:
		void startTimeProfileForm();
		SpaceTimePathDrawer *stpdrw;
	};

	class ProfileGraphWindow : public SimpleGraphWindowWrapper
	{
	public:
		ProfileGraphWindow(FormEntry *f, SpaceTimePathDrawer *_stpdrw);
		virtual ~ProfileGraphWindow();
		void SetFunctions(SimpleFunction * funPtr, int _iNrFunctions);
		void SetGridTicks(vector<double> & gridXNodes, vector<double> & gridXTicks, vector<double> & gridYTicks);
		void SetGrid(bool gridXN, bool gridXT, bool gridYT);
		void SelectFeatures(RowSelectInfo & inf);
		virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);

	protected:
		virtual void DrawFunction(CDC* pDC, const SimpleFunction * pFunc);
		virtual void DrawSelectedFunctions(CDC* pDC, const SimpleFunction * pFunc);
		virtual void DrawAxes(CDC* pDC);
		virtual void DrawMouse(CDC* pDC);
		virtual void SetDirty(bool fRedraw = false);
		virtual void StartDrag(CPoint point);
		virtual void Drag(CPoint point);
		virtual void EndDrag(CPoint point);
		String getInfo(CPoint point);
		void SelectionChanged();
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg void OnDestroy();
		vector<double> m_gridXNodes;
		vector<double> m_gridXTicks;
		vector<double> m_gridYTicks;
		bool m_gridXN;
		bool m_gridXT;
		bool m_gridYT;
		int iNrFunctions;
		InfoLine* info;
		String sInfo;
		CPoint mousePos;
	 	CBitmap* m_bmMemoryGraph;
		CBitmap* m_bmOldBitmapGraph;
		CDC* m_dcMemoryGraph;
		bool fDrawAxes;
		bool m_fSelectionChanged;
		bool m_fAbortSelectionThread;
		CWinThread * m_selectionThread;
		CCriticalSection csSelectionThread;

	private:
		SpaceTimePathDrawer *stpdrw;
		static bool fPointOnLine(CPoint c, int ax, int ay, int bx, int by);
		static UINT PaintInThread(LPVOID pParam);
		static UINT SelectionChangedInThread(LPVOID pParam);

		DECLARE_MESSAGE_MAP()
	};

	class ProfileFieldGraph : public FieldGraph  
	{
	public:
		ProfileFieldGraph(FormEntry* parent, SpaceTimePathDrawer* _stpdrw);
		void create();             // overriden
		void SetFunctions(SimpleFunction * funPtr, int iNrFunctions);
		void SetGridTicks(vector<double> & gridXNodes, vector<double> & gridXTicks, vector<double> & gridYTicks);
		void SetGrid(bool gridXN, bool gridXT, bool gridYT);
		void SelectFeatures(RowSelectInfo & inf);
	private:
		SpaceTimePathDrawer* stpdrw;
	};

	class ProfileGraphFunction : public SimpleFunction
	{
	public:
		ProfileGraphFunction();
		virtual ~ProfileGraphFunction();
		virtual double rGetFx(double x) const; // the most important function of this class that applies f to x and returns y
		virtual void SetDefaultAnchors(); // also call SolveParams in the implementation of this function
		void SetData(vector<double> & dataX, vector<double> & dataY);
		void SetColors(vector<Color> & colors);
		void SetRaws(vector<long> & iRaws);
		Color GetColor(double x) const;
		virtual void SetAnchor(DoublePoint pAnchor);
		void SetDomain(double l, double t, double r, double b);
		double rFirstX();
		double rLastX();
		void SetSelected(bool fSelected);
		bool fSelected();
		vector<long> & iRaws();
	protected:
		virtual void SolveParams();
	private:
		vector <double> m_rDataX;
		vector <double> m_rDataY;
		vector <Color> m_colors;
		vector <long> m_iRaws;
		bool m_fSelected;
	};

	class _export TimeProfileForm: public FormWithDest
	{
	public:
	  TimeProfileForm(CWnd* mw, SpaceTimePathDrawer * stp);
	  virtual ~TimeProfileForm();
	protected:
		afx_msg LONG OnSelectFeatures(UINT, LONG lParam);
		virtual void shutdown(int iReturn=IDCANCEL);
	private:
		void ComputeGraphs();
		int CallBackAnchorChangedInGraph(Event*);
		int CallBackSegmentMapChanged(Event*);
		int CallBackXNGrid(Event*);
		int CallBackXTGrid(Event*);
		int CallBackYTGrid(Event*);
		String sSegmentMapProfile;
		FieldSegmentMap * fsm;
		ProfileFieldGraph * fgFunctionGraph;
		ProfileGraphFunction * m_functions;
		SpaceTimePathDrawer *stpdrw;
		CheckBox * cbXNgrid;
		CheckBox * cbXTgrid;
		CheckBox * cbYTgrid;
		CheckBox * cbRadiusThreshold;
		FieldReal * frRadiusThreshold;
		bool m_gridXN;
		bool m_gridXT;
		bool m_gridYT;
		bool m_fUseRadiusThreshold;
		double m_radiusThreshold;

		DECLARE_MESSAGE_MAP()
	};
}

#endif // TIMEPROFILETOOL_H