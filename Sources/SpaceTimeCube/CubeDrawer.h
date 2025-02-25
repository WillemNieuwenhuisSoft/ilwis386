#pragma once
#include "Engine\Drawers\OpenGLText.h"
#include "TemporalDrawer.h"

ILWIS::NewDrawer *createCubeDrawer(ILWIS::DrawerParameters *parms);

namespace ILWIS {

	struct _export CubeElement {
		CubeElement() {
			color = colorUNDEF;
			alpha = rUNDEF;
			visible = false;
			label = sUNDEF;
		}
		CubeElement(const String& labl, const Color& clr, double alp, bool vis) : 
		color(clr), alpha(alp),visible(vis),label(labl){
		}

		CubeElement(const CubeElement& ce) {
			color = ce.color;
			alpha = ce.alpha;
			visible = ce.visible;
			label = ce.label;
		}
		bool isValid() const{
			return color.fEqual(colorUNDEF) && (alpha == rUNDEF) && !visible;
		}
		static CubeElement undefElement;
		String store(const FileName& fnView, const String& section) const;
		void load(const FileName& fnView, const String& currentSection);

		Color color;
		double alpha;
		bool visible;
		String label;
	};

	struct _export CubeProperties : public GeneralDrawerProperties {
		CubeProperties();
		CubeProperties(CubeProperties *lp);
		
		map<String, CubeElement> elements;
		CubeElement& operator[](const String& key) const;

		String store(const FileName& fnView, const String& parentSection) const;
		void load(const FileName& fnView, const String& parentSection);
	};

	class _export CubeDrawer : public ComplexDrawer, public TemporalDrawer {
	public:
		ILWIS::NewDrawer *createCubeDrawer(DrawerParameters *parms);

		CubeDrawer(DrawerParameters *parms);
		virtual ~CubeDrawer();
		bool draw(const DrawLoop drawLoop, const CoordBounds& cbArea=CoordBounds()) const;
		void prepare(PreparationParameters *pp);
		GeneralDrawerProperties *getProperties();
		void SetTimePosVariables(double * _timePos, String * _sTimePosText);
	protected:
		void drawCube() const;
		void drawTicMarks() const;
		void drawLabels(const DrawLoop drawLoop, const Color & clr) const;
		void drawCoords(const DrawLoop drawLoop, const Color & clr) const;
		void drawTimes(const DrawLoop drawLoop, const Color & clr) const;
		void renderText(const DrawLoop drawLoop, OpenGLText *font, const Coordinate & c, const String & text, bool center=false) const;
		String store(const FileName& fnView, const String& parentSection) const;
		void load(const FileName& fnView, const String& currentSection);
		String sxMin, sxMax, syMin, syMax, stMin, stMax;
		CoordBounds cube;
		double * timePos;
		String * sTimePosText;
		OpenGLText * font;
		OpenGLText * mediumFont;
		CubeProperties properties;
		CCriticalSection * csFont;
	};
}