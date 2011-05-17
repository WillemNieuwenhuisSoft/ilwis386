#pragma once

ILWIS::DrawerTool *createGraticuleTool(ZoomableView* zv, LayerTreeView *view, ILWIS::NewDrawer *drw);

namespace ILWIS {

	class GraticuleDrawer;

	class GraticuleTool : public DrawerTool {
	public:
		GraticuleTool(ZoomableView* zv, LayerTreeView *view, NewDrawer *drw);
		bool isToolUseableFor(ILWIS::NewDrawer *drw);
		HTREEITEM configure( HTREEITEM parentItem);
		virtual ~GraticuleTool();
		String getMenuString() const;
		void toggleGraticule(bool yesno) { hasGraticule = yesno;}
	protected:
		void graticuleActive(void *v, HTREEITEM);
		void GraticuleOptions();
		bool hasGraticule;


	};

	class GraticuleForm: public DisplayOptionsForm
	{
	public:
		GraticuleForm(CWnd *par, GraticuleDrawer *gdr);
		void apply();
	private:
		FieldReal *fr;
		GraticuleDrawer *gd;
		double rDist;
		int dummy;
	};

}