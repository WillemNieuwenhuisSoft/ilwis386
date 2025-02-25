#pragma once

#include "Engine\Drawers\TimeMessages.h"

#define SLAVE_TIMER_ID 46757736

ILWIS::NewDrawer *createAnimationDrawer(ILWIS::DrawerParameters *parms);
namespace ILWIS{
	class BoxDrawer;
	class AnimationSlicing;
	class FeatureLayerDrawer;
	class LayerDrawer;
	class AnimationDrawer;

	struct _export SlaveProperties {
		SlaveProperties(AnimationDrawer *s, int offset, double step) : slave(s), slaveOffset(offset), slaveStep(step) {}
		AnimationDrawer *slave;
		int slaveOffset;
		double slaveStep;
	};

	class _export AnimationDrawer : public SetDrawer, public TimeListener, public TimeProvider {
	public:
		AnimationDrawer(DrawerParameters *parms);
		virtual ~AnimationDrawer();
		virtual void prepare(PreparationParameters *pp);
		bool draw(int drawerIndex, const DrawLoop drawLoop, const CoordBounds& cbArea) const;
		void timedEvent(UINT _timerid);
		vector<int>& getActiveMaps() { return activeMaps; }
		void animationDefaultView();
		void addDataSource(void *bmap, int options=0);
		void setMapIndex(int ind);
		double getInterval() const { return interval; }
		bool getUseTime() const { return useTime; }
		void setInterval(double intv) { interval = intv; }
		String getTimeColumn() const { return colTime; }
		String description() const;
		void setTimeColumn(const Column& col);
		void setTimeStep(ILWIS::Duration dur);
		ILWIS::Duration getTimeStep() const;
		int getTimerId() const { return timerid; }
		void setTimerId(int tid) { timerid = tid; }
		void setUseTime(bool yesno);
		int getMapIndex() const { return mapIndex; }
		static int getTimerIdCounter(bool increase=false);
		void addSlave(const SlaveProperties& props);
		void removeSlave(AnimationDrawer *drw);
		void setTresholdColor(const Color&clr);
		void setTresholdRange(const RangeReal& rr, bool single);

		static int timerIdCounter;
		CWnd *manager; // basically a UI element, but is sufficiently abstract in this form to be acceptable; no outisde linage needed.
		// a message needs to be send to this window else dangling pointers result;

	protected:
		String store(const FileName& fnView, const String& section) const;
		void load(const FileName& fnView, const String& section);

		virtual String iconName(const String& subtype="?") const;
		bool timerPerIndex();
		bool timerPerTime();
		bool activeOnTime(const Column& col, double currentTime);
		void SetTime(ILWIS::Time time, long sender);
		double interval;
		UINT timerid;
		vector<String> names;
		vector<int> activeMaps;
		vector<SlaveProperties> slaves;
		int index;
		int mapIndex;
		bool useTime;
		String colTime;
		//DisplayOptionsLegend *doLegend;

		ILWIS::Duration timestep;
		CCriticalSection csAccess;

		//void addLayerDrawer(const BaseMap& basemap, ILWIS::PreparationParameters *pp, ILWIS::LayerDrawer *rsd, const String& name="", bool post=false);
	};

}