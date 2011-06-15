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
// ScriptForm.cpp: implementation of the ScriptForm class.
//
//////////////////////////////////////////////////////////////////////

#include "Client\Headers\AppFormsPCH.h"
#include "Engine\Scripting\Script.h"
#include "Client\ilwis.h"
#include "Client\Forms\ScriptForm.h"
#include "Client\FormElements\fldval.h"
#include "Client\FormElements\fldtbl.h"
#include "Client\Help\helpwindow.h"
#include "Headers\Hs\Script.hs"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(ScriptForm, FormBaseDialog)
  ON_COMMAND(IDHELP, OnHelp)
  ON_COMMAND(ID_HELP, OnHelp)
END_MESSAGE_MAP()    

ScriptForm::ScriptForm(const Script& script)
:	FormWithDest(0, 0 != script->sDescr().length() ? script->sDescr() : script->sTypeName(), fbsSHOWALWAYS & ~fbsMODAL),
	scr(script)
{
	iParams = scr->iParams();
	sVal = new String[iParams];
	for (int i = 0; i < iParams; ++i) {
		String sQuestion = scr->sParam(i);
		String sDefault = scr->sDefaultValue(i);
		sVal[i] = sDefault;
		switch (scr->ptParam(i))
		{
			case ScriptPtr::ptSTRING:
				new FieldString(root, sQuestion, &sVal[i]);
				break;
			case ScriptPtr::ptFILENAME:
				new FieldString(root, sQuestion, &sVal[i]);
				break;
			case ScriptPtr::ptCOLUMN:
				new FieldString(root, sQuestion, &sVal[i]);
				break;
			case ScriptPtr::ptVALUE:	{
				DomainValueRangeStruct dvrs(-1e9,1e9,0.001);
				new FieldVal(root, sQuestion, dvrs, &sVal[i]);
				break; }
			case ScriptPtr::ptDOMAIN:
				new FieldDomain(root, sQuestion, &sVal[i]);
				break;
			case ScriptPtr::ptRPR:
				new FieldRepresentation(root, sQuestion, &sVal[i]);
				break;
			case ScriptPtr::ptGEOREF:
				new FieldGeoRefExisting(root, sQuestion, &sVal[i]);
				break;
			case ScriptPtr::ptCOORDSYS:
				new FieldCoordSystem(root, sQuestion, &sVal[i]);
				break;
			case ScriptPtr::ptRASMAP:
				new FieldMap(root, sQuestion, &sVal[i]);
				break;
			case ScriptPtr::ptSEGMAP:
				new FieldSegmentMap(root, sQuestion, &sVal[i]);
				break;
			case ScriptPtr::ptPOLMAP:
				new FieldPolygonMap(root, sQuestion, &sVal[i]);
				break;
			case ScriptPtr::ptPNTMAP:
				new FieldPointMap(root, sQuestion, &sVal[i]);
				break;
			case ScriptPtr::ptTABLE:
		    new FieldDataType(root, sQuestion, &sVal[i], ".TBT", true);
				break;
			case ScriptPtr::ptMAPVIEW:
		    new FieldDataType(root, sQuestion, &sVal[i], ".MPV", true);
				break;
			case ScriptPtr::ptMAPLIST:
		    new FieldDataType(root, sQuestion, &sVal[i], ".MPL", true);
				break;
			case ScriptPtr::ptTBL2D:
		    new FieldDataType(root, sQuestion, &sVal[i], ".TA2", true);
				break;
			case ScriptPtr::ptANNTXT:
		    new FieldDataType(root, sQuestion, &sVal[i], ".ATX", true);
				break;
			case ScriptPtr::ptSMS:
		    new FieldDataType(root, sQuestion, &sVal[i], ".SMS", true);
				break;
			case ScriptPtr::ptMATRIX:
		    new FieldDataType(root, sQuestion, &sVal[i], ".MAT", true);
				break;
			case ScriptPtr::ptFILTER:
		    new FieldDataType(root, sQuestion, &sVal[i], ".FIL", true);
				break;
			case ScriptPtr::ptFUNCTION:
		    new FieldDataType(root, sQuestion, &sVal[i], ".FUN", true);
				break;
			case ScriptPtr::ptSCRIPT:
		    new FieldDataType(root, sQuestion, &sVal[i], ".ISL", true);
				break;
		}
	}
	//IlwWinApp()->setHelpItem(""); // set help button on
	create();
  ShowWindow(SW_SHOW);
}

ScriptForm::~ScriptForm()
{
	delete [] sVal;
}

String ScriptForm::sExec()
{
	String sRes;
	for (int i = 0; i < iParams; ++i) {
		sRes &= " ";
    String s = sVal[i];
    switch (scr->ptParam(i)) {
      case ScriptPtr::ptSTRING: {
        if ((s[0] != '\"') && (s[s.length()-1] != '\"'))
          s = String("\"%S\"", s);
        break;
      }
      case ScriptPtr::ptVALUE: { // do nothing more
        break;
      }
      case ScriptPtr::ptFILENAME: 
			{
				FileName fn(s);
        s = fn.sRelativeQuoted();
        break;
      }
      case ScriptPtr::ptCOLUMN: {
        s = s.sQuote();
        break;
      }
      default: { // all others are ilwis objects; remove default path
        FileName fn(s);
        bool fExt = scr->fParamIncludeExtension(i);
        s = fn.sRelativeQuoted(fExt, scr->fnObj.sPath());
      }
    }
    sRes &= s;
	}
	return sRes;
}

int ScriptForm::exec()
{
  FormWithDest::exec();

	String sDir = IlwWinApp()->sGetCurDir();
	String sCommand("run %S %S",scr->sNameQuoted(), sExec());
	IlwWinApp()->Execute(sCommand);

  return 1;
}

void ScriptForm::OnHelp()
{
	FileName fn = scr->fnObj;

	FileName fnCss = fn;
	fnCss.sFile = "ilwis";
	fnCss.sExt = ".css";
	if (!File::fExist(fnCss)) {
		String sOrig = IlwWinApp()->Context()->sIlwDir() + "Scripts\\ilwis.css";
		CopyFile(sOrig.scVal(), fnCss.sFullPath().scVal(), TRUE);
	}

	fn.sExt = ".htm";
	if (!File::fExist(fn)) {
		ofstream os(fn.sFullName().scVal());
		os << "<html>\n<head>\n<title>" 
			<< fn.sFile.scVal() << "</title>\n"
			<< "<meta name=\"Generator\" content=\"ILWIS Script Form\">\n"
			<< "<link rel=stylesheet type=\"text/css\" href=\"ilwis.css\">\n"
			<< "</head>\n"
			<< "<body text=\"#000000\" bgcolor=\"#FFFFFF\">\n"
			<< "<h1 class=only1>Script " << fn.sFile.scVal() << "</h1>\n"
			<< "<p>" << scr->sDescription.scVal() << "</p>\n"
			<< "<p class=defnewpar>Some more explanation could be written.</p>\n"
			<< "<p class=diakopje>Dialog box options:</p>\n"
			<< "<table cellspacing=0>\n";
		int iParams = scr->iParams();
		for (int i = 0; i < iParams; ++i) {
			String sParam = scr->sParam(i);
			ScriptPtr::ParamType pt = scr->ptParam(i);
			String sDefault = scr->sDefaultValue(i);
			String sType;
			switch (pt) 
			{
				case ScriptPtr::ptRASMAP: sType =  SSCRPtRasMap; break;
				case ScriptPtr::ptSEGMAP: sType =  SSCRPtSegMap; break;
				case ScriptPtr::ptPOLMAP: sType =  SSCRPtPolMap; break;
				case ScriptPtr::ptPNTMAP: sType =  SSCRPtPntMap; break;
				case ScriptPtr::ptTABLE: sType =  SSCRPtTable; break;
				case ScriptPtr::ptCOLUMN: sType =  SSCRPtColumn; break;
				case ScriptPtr::ptMAPLIST: sType =  SSCRPtMapList; break;
				case ScriptPtr::ptMAPVIEW: sType =  SSCRPtMapView; break;
				case ScriptPtr::ptCOORDSYS: sType =  SSCRPtCoordSys; break;
				case ScriptPtr::ptGEOREF: sType =  SSCRPtGeoRef; break;
				case ScriptPtr::ptDOMAIN: sType =  SSCRPtDomain; break;
				case ScriptPtr::ptRPR: sType =  SSCRPtRpr; break;
				case ScriptPtr::ptFILTER: sType =  SSCRPtFilter; break;
				case ScriptPtr::ptSCRIPT: sType =  SSCRPtScript; break;
				case ScriptPtr::ptFUNCTION: sType =  SSCRPtFunction; break;
				case ScriptPtr::ptMATRIX: sType =  SSCRPtMatrix; break;
				case ScriptPtr::ptSMS: sType =  SSCRPtSms; break;
				case ScriptPtr::ptTBL2D: sType =  SSCRPtTbl2d; break;
				case ScriptPtr::ptANNTXT: sType =  SSCRPtAnnTxt; break;
				case ScriptPtr::ptSTRING : sType =  SSCRPtString; break;
				case ScriptPtr::ptVALUE  : sType =  SSCRPtValue; break;
				case ScriptPtr::ptFILENAME: sType =  SSCRPtFileName; break;
			}
			FileName fnType(sType);
			sType = fnType.sFile;
			os << "<tr><td valign=\"top\" width=\"30%\"><p class=diabox>" << sParam.scVal() << ":</p></td>\n<td valign=\"top\"><p class=diabox>" 
				<< "Select a " << sType << " parameter to be used as " << sParam.scVal() << ".\n";
			if (sDefault != "")
				os << "Default is <i>" << sDefault.scVal() << "</i>";
			else 
				os << "There is no default value, you always have to specify this parameter yourself.";
			os << "</p></td>\n";
		}
		os << "</table>\n"
			<< "<p class=tip>Tip:</p>\n"
			<< "<p class=tiptext>This help text is automatically generated and can be changed by the script author.</p>\n"
			<< "<p class=seealso>See also:</p>\n"
			<< "<p class=seealsolinks><a href=\"ilwis:/ilwis/script_editor_functionality.htm\">Script Editor Functionality</a></p>\n"
			<< "<p class=seealsolinks><a href=\"ilwis:/ilwis/how_to_use_parameters_in_scripts.htm\">How to use parameters in Scripts</a></p>\n"
			<< "<p class=seealsolinks><a href=\"http://www.itc.nl/ilwis\">ILWIS on the Web</a></p>\n"
			<< "</body>\n"
			<< "</html>";
	}
	String sFile = fn.sFullPath();
	new HelpWindow(this, fn.sFile.scVal(), sFile.scVal());
}


