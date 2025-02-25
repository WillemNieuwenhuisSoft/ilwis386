#pragma once


ILWIS::NewDrawer *createFeatureSetDrawer(ILWIS::DrawerParameters *parms);

class FieldColor;
class DisplayOptionColorItem;

namespace ILWIS{

class _export FeatureSetDrawer : public SetDrawer {
	friend class SetSingleColorForm;

	public:
		ILWIS::NewDrawer *createFeatureSetDrawer(DrawerParameters *parms);

		FeatureSetDrawer(DrawerParameters *parms, const String& name);
		virtual ~FeatureSetDrawer();
		virtual void prepare(PreparationParameters *pp);
		String getMask() const;
		void setMask(const String& sM);
		void setSingleColor(const Color& c);
		Color getSingleColor() const;
		virtual NewDrawer *createElementDrawer(PreparationParameters *pp, ILWIS::DrawerParameters* parms) const { return 0;}
		void getFeatures(vector<Feature *>& features) const;
		virtual void addDataSource(void *,int options=0);
		void *getDataSource() const;

	protected:
		String store(const FileName& fnView, const String& parenSection) const;
		void load(const FileName& fnView, const String& parenSection);

		String mask;
		bool useMask;
		Color singleColor;
		BaseMap fbasemap;
	
	};
}