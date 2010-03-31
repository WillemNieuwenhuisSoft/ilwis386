#include "Headers\toolspch.h"
#include "Engine\Base\File\Directory.h"
#include "Engine\Base\System\LOGGER.H"
#include "Engine\Base\System\Engine.h"
#include "Engine\Applications\objvirt.h"
#include "Engine\Base\File\zlib.h"
#include "Engine\Base\File\unzip.h"
#include "engine\base\system\module.h"
#include "Engine\Applications\ModuleMap.h"
#include "Engine\Table\TBLHSTPL.H"
#include "Engine\Table\TBLHSTPL.H"
#include "Engine\Table\TBLHSTSG.H"
#include "Engine\Table\TBLHSTPT.H"
#include "Engine\Table\TBLHIST.H"

using namespace ILWIS;

void ModuleMap::addModules() {
	CFileFind finder;

	String path = getEngine()->getContext()->sIlwDir() + "\\Extensions\\*.zip";
	BOOL fFound = finder.FindFile(path.scVal());
	while(fFound) {
		fFound = finder.FindNextFile();
		if (!finder.IsDirectory())
		{
			FileName fnNew (finder.GetFilePath());
			unzip(fnNew);
			_unlink(fnNew.sFullPath().scVal());
		}
	}   

	path = getEngine()->getContext()->sIlwDir() + "\\Extensions";
	addFolder(path);
	applications.addExtraFunctions();

}

void ModuleMap::addFolder(const String& dir) {
	CFileFind finder;
	String pattern = dir + "\\*.*";
	BOOL fFound = finder.FindFile(pattern.scVal());
	while(fFound) {
		fFound = finder.FindNextFile();
		if (!finder.IsDirectory())
		{
			FileName fnNew (finder.GetFilePath());
			if ( fnNew.sExt == ".dll" || fnNew.sExt == ".DLL")
				addModule(fnNew);
		} else {
			FileName fnNew (finder.GetFilePath());
			if ( fnNew.sFile != "." && fnNew.sFile != ".." && fnNew.sFile != "")
				addFolder(String(fnNew.sFullPath()));
		}
	}
}

void ModuleMap::addModule(const FileName& fnModule) {
	try{
		HMODULE hm = LoadLibrary(fnModule.sFullPath().scVal());
		if ( hm != NULL ) {
			//AppInfo f = (AppInfo)GetProcAddress(hm, "getApplicationInfo");
			ModuleInfo m = (ModuleInfo)GetProcAddress(hm, "getModuleInfo");
			if ( m != NULL) {
				ILWIS::Module *mod = (*m)();
				ILWIS::Module::ModuleInterface type = mod->getInterfaceVersion();
				getEngine()->getVersion()->fSupportsModuleInterfaceVersion(type, mod->getName());
				addModule(mod);
				AppInfo appFunc = (AppInfo)(mod->getMethod(ILWIS::Module::ifGetAppInfo));
				if ( appFunc) {
					InfoVector *infos = (*appFunc)();
					if ( infos->size() > 0)
						applications.addApplications(*infos);
					delete infos;
				}
				ModuleInit initFunc = (ModuleInit)(mod->getMethod(ILWIS::Module::ifInit));
				if ( initFunc) {
					moduleInits.push_back(initFunc);
				}

			}
		}
	}catch(ErrorObject& err){
		err.Show();
	}
}

void ModuleMap::initModules() {
	for(vector<ModuleInit>::iterator cur; cur != moduleInits.end(); ++cur) {
		ModuleInit moduleInit = (*cur);
		moduleInit();
	}
}

ModuleMap::~ModuleMap() {
	for(ModuleIter cur = this->begin(); cur != this->end(); ++cur) {
		Module *mi = (*cur).second;
		delete mi;
	}
}

void ModuleMap::addModule(ILWIS::Module *m) {
	if ( m->getName() == "")
		throw ErrorObject("Improper module definition : No name");
	if ( this->find(m->getName()) == this->end()) {
		(*this)[m->getName()] = m;
	}
}
//-------------------------------------------------------------------------------------------
ApplicationMap::~ApplicationMap() {
	for(AppIter cur = this->begin(); cur != this->end(); ++cur) {
		ApplicationInfo *ai = (*cur).second;
		delete ai;
	}
}

void ApplicationMap::addExtraFunctions() {
	InfoVector *infos = new InfoVector();
	(*infos).push_back(ApplicationMap::newApplicationInfo(createTableHistogram,"TableHistogram"));
	(*infos).push_back(ApplicationMap::newApplicationInfo(createTableHistogramPol,"TableHistogramPol"));
	(*infos).push_back(ApplicationMap::newApplicationInfo(createTableHistogramSeg,"TableHistogramSeg"));
	(*infos).push_back(ApplicationMap::newApplicationInfo(createTableHistogramPnt,"TableHistogramPnt"));

	addApplications(*infos);

	delete infos;
}

ApplicationInfo * ApplicationMap::newApplicationInfo(CreateFunc appFunc, String appName) {
	ApplicationInfo *inf = new ApplicationInfo;
	inf->createFunction = appFunc;
	inf->name = appName.toLower();
	inf->metadata = NULL;

	return inf;
}

ApplicationInfo * ApplicationMap::newApplicationInfo(CreateFunc appFunc, String appName, MetaDataFunc mdFunc) {
	ApplicationInfo *inf = new ApplicationInfo;
	inf->createFunction = appFunc;
	inf->name = appName.toLower();
	inf->metadata = mdFunc;

	return inf;
}

void ApplicationMap::addApplications(InfoVector apps) {
	for(InfoVIter cur = apps.begin(); cur != apps.end(); ++cur) {
		ApplicationInfo *ai = (*cur);
		InfoPair p(ai->name, ai);
		(*this).insert(p);
	}
}

ApplicationInfo * ApplicationMap::operator[](String n) {
	String name = n.toLower();
	AppIter iter = (*this).find(name);
	if ( iter != this->end() )
		return (*iter).second;
	for(AppIter cur = this->begin(); cur != this->end(); ++cur) {
		String entryName = (*cur).first;
		size_type index = entryName.find("___");
		if ( index == string::npos)
			continue;
		unsigned int i = 0;
		for(; i < index; ++i) {
			char c1 = name[i];
			char c2 = entryName[i];
			if ( c1 != c2 )
				break;

		}
		if ( i == index ) {
			return (*cur).second;
		}
	}
	return NULL;
}
