/***************************************************************
 ILWIS integrates image, vector and thematic data in one unique 
 and powerful package on the desktop. ILWIS delivers a wide 
 range of feautures including import/export, digitizing, editing, 
 analysis and display of data as well as production of 
 quality mapsinformation about the sensor mounting platform
 
 Exclusive rights of use by 52�North Initiative for Geospatial 
 Open Source Software GmbH 2007, Germany

 Copyright (C) 2007 by 52�North Initiative for Geospatial
 Open Source Software GmbH

 Author: Jan Hendrikse, Willem Nieuwenhuis,Wim Koolhoven 
 Bas Restsios, Martin Schouwenburg, Lichun Wang, Jelle Wind 

 Contact: Martin Schouwenburg; schouwenburg@itc.nl; 
 tel +31-534874371

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program (see gnu-gpl v2.txt); if not, write to
 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA or visit the web page of the Free
 Software Foundation, http://www.fsf.org.

 Created on: 2007-02-8
 ***************************************************************/
/* FormStereoPairCreate
   Copyright Ilwis System Development ITC
   may 2002, by Jan Hendrikse
*/
#include "Client\Headers\AppFormsPCH.h"
#include "Engine\SpatialReference\GrStereoMate.h"
#include "Engine\Stereoscopy\StereoPairFromDTM.h"
#include "Headers\Hs\stereoscopy.hs"
#include "ApplicationsUI\frmstereopaircr.h"
#include "Client\ilwis.h"
#include "Client\FormElements\flddom.h"

FormStereoPairCreate::FormStereoPairCreate(CWnd* mwin, const String& sTitle)
: FormGeneralApplication(mwin, sTitle),
  mw(mwin) 
{
	int iImg = IlwWinApp()->iImage(".stp");
	HICON hIco = IlwWinApp()->ilSmall.ExtractIcon(iImg);
	SetIcon(hIco,FALSE);
}                    


void FormStereoPairCreate::initStereoPairOut()
{
	fstc = new FieldStereoPairCreate(root, TR("Output Stereo Pair"), &sOutStereoPair);
  ///fstc->SetCallBack((NotifyProc)&FormStereoPairCreate::OutStereoPairCallBack);
  FormEntry* fe = fstc;
  StaticText* st = new StaticText(root, TR("&Description:"));
  st->Align(fe, AL_UNDER);
  st->psn->SetBound(0,0,0,0);
  FieldString* fs = new FieldString(root, "", &sDescr);
  fs->SetWidth(120);
  fs->SetIndependentPos();
}

   
void FormStereoPairCreate::execStereoPairOut(const String& sExpr)
{
  FileName fn(sOutStereoPair);
	fn.sExt = ".stp";
	if (sOutStereoPair != "") {
    if (fn.fExist()) {
      String sErr(TR("File %S already exists.\nOverwrite?").c_str(), fn.sFullPath(true));
      int iRet = mw->MessageBox(sErr.c_str(), TR("File already exists").c_str(), MB_YESNO|MB_ICONEXCLAMATION);
      if (iRet != IDYES)
        return;
    }
		String sCmd = fn.sShortNameQuoted();
    sOutStereoPair = sCmd;
    sCmd &= " = ";
    sCmd &= sExpr;
    SetCommand(sCmd);
  }
	try {
    StereoPair stpOut(fn, sExpr);
    if (sDescr[0])
      stpOut->sDescription = sDescr;
    stpOut->fChanged = true;      
    if (fShow) 
		{
			if (stpOut->fnObj.fValid()) {
				String s = "show ";
				s &= fn.sRelativeQuoted();
				stpOut->Store(); // force
				ILWISAPP->Execute(s);
			}
			else {
				String s("show %S", sExpr);
				ILWISAPP->Execute(s);
			}
    }
  }  
  catch (ErrorObject& err) {
    err.Show();
  }  
  return;
}

int FormStereoPairCreate::OutStereoPairCallBack(Event*)
{
  fstc->StoreData();
  // later check on existance etc.  
  return 0;
}

LRESULT Cmdmakestpfromdtm(CWnd *parent, const String& s) {
	new FormStereoPairFromDTM(parent, s.c_str());
	return -1;
}

FormStereoPairFromDTM::FormStereoPairFromDTM(CWnd* mw, const char* sPar)
: FormStereoPairCreate(mw, TR("Stereo Pair from DTM"))
{
	//lmodus = lmBOTH;
	iLookModus = 1; // == lmBOTH (default)
	iLookAngle = 30; // default 
	iRefHeight = 800; // default for the time being, should depend on DTM
  if (sPar) {
    TextInput inp(sPar);
    TokenizerBase tokenizer(&inp);
    String sVal;
    for (;;) {
      Token tok = tokenizer.tokGet();
      sVal = tok.sVal();
      if (sVal == "")
        break;
      FileName fn(sVal);
      if (fn.sExt == "" || fn.sExt == ".mpr")
        if (sSourceMap == "")
          sSourceMap = fn.sFullNameQuoted(true);
        else if (sDTMMap == "") {
          sDTMMap = fn.sFullNameQuoted(true);
        }  
        else  
          sOutStereoPair = fn.sFullName(false);
    }
  }
  ///sDomain = "value.dom";
  ///fldSourceMap = 
		new FieldDataType(root, TR("&Raster Map"), &sSourceMap, 
                                   new MapListerDomainType(".mpr", dmIMAGE|dmCOLOR|dmPICT|dmVALUE, true), true);
  ///fldSourceMap->SetCallBack((NotifyProc)&FormStereoPairFromDTM::SrcMapCallBack);
  ///fldDTMMap = 
	fldDTMMap =	new FieldDataType(root, TR("&DTM"), &sDTMMap, new MapListerDomainType(".mpr", dmVALUE, true), true);
  fldDTMMap->SetCallBack((NotifyProc)&FormStereoPairFromDTM::DTMMapCallBack);
  
	FieldInt* fiLookAngle = new FieldInt(root, TR("Look &Angle"), &iLookAngle, RangeInt(0,90),true);
  ///frLookAngle->Align(fldDTMMap, AL_UNDER);
	fiLookAngle->SetWidth(30);
  fiRefHeight = new FieldInt(root, TR("Reference &Height"), &iRefHeight, RangeInt(0,9999),true);
	fiRefHeight->SetWidth(30);
  fiRefHeight->Align(fiLookAngle, AL_UNDER);
	RadioGroup* rgLookModus = new RadioGroup(root, TR("Look &Modus"), &iLookModus, true);
	(new RadioButton(rgLookModus, "&Left"))->SetIndependentPos();
  (new RadioButton(rgLookModus, "&Both"))->SetIndependentPos(); // default choice
  new RadioButton(rgLookModus, "&Right");
	RadioGroup* rgResampModus = new RadioGroup(root, TR("Resample Modus"), &iResampleModus, true);
	(new RadioButton(rgResampModus, "&Fast"))->SetIndependentPos();// default choice
  new RadioButton(rgResampModus, "&Accurate"); 
  initStereoPairOut();
  SetHelpItem("ilwisapp\\stereopair_from_dtm_dialogbox.htm");
  create();
}                    


int FormStereoPairFromDTM::DTMMapCallBack(Event*)
{
  fldDTMMap->StoreData();
	RangeReal rrMMDTM;
  try {
    FileName fnDTMMap(sDTMMap);
		Map mapDTM(fnDTMMap);
    if (fnDTMMap.sFile == "")
      return 0;
    rrMMDTM = mapDTM->rrMinMax(BaseMapPtr::mmmCALCULATE);  // find minmax height of DTM
		iRefHeight = (int)((rrMMDTM.rLo()+ rrMMDTM.rHi()) / 2);
		fiRefHeight->SetVal(iRefHeight);
  }
  catch (ErrorObject&) {}
  return 0;
}

int FormStereoPairFromDTM::exec() 
{
  FormStereoPairCreate::exec();
  String sExpr;
  FileName fn(sOutStereoPair);
  FileName fnSourceMap(sSourceMap); 
  FileName fnDTMMap(sDTMMap); 
  sSourceMap = fnSourceMap.sRelativeQuoted(false,fn.sPath());
  sDTMMap = fnDTMMap.sRelativeQuoted(false,fn.sPath());
	double rLookAngle = (double)iLookAngle;
	double rRefHeight = (double)iRefHeight;
	String sLookModus;
  switch (iLookModus) {
    case 0: sLookModus = "left"; break; 
    case 1: sLookModus = "both"; break; 
		case 2: sLookModus = "right"; break; 
	} 
  String sResampleModus;
  switch (iResampleModus) {
    case 0: sResampleModus = "fast"; break; 
    case 1: sResampleModus = "accurate"; break; 
	} 
  sExpr = String("StereoPairFromDTM(%S,%S,%.2f,%.2f,%S,%S)", 
                      sSourceMap, sDTMMap,rLookAngle,rRefHeight,sLookModus,sResampleModus);
  execStereoPairOut(sExpr);  
  return 0;
}
