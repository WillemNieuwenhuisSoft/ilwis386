#include "Client\Headers\formelementspch.h"
#include "Client\ilwis.h"
#include "Engine\Base\DataObjects\ObjectCollection.h"
#include "Client\FormElements\FieldListView.h"
#include "Engine\Domain\dmcoord.h"
#include "CrossSectionGraph.h"


BEGIN_MESSAGE_MAP(CrossSectionGraph, CStatic)
	ON_WM_LBUTTONUP()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

#define CSGRPAH_SIZE 460
CrossSectionGraph::CrossSectionGraph(CrossSectionGraphEntry *f, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID) : BaseZapp(f) 
{
	fldGraph = f;
	if (!CStatic::Create(0,dwStyle | SS_OWNERDRAW | SS_NOTIFY, rect, pParentWnd, nID))
		throw ErrorObject(TR("Could not create CS graph"));
	ModifyStyleEx(0, WS_EX_NOPARENTNOTIFY, SWP_FRAMECHANGED);
	EnableTrackingToolTips();
}

#define ID_GR_COPY 5100
#define ID_SAVE_AS_CSV 5101
#define ID_SAVE_AS_TABLE 5102

void CrossSectionGraph::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	//if (tools.size() == 0)
	//	return;
	//if (edit && edit->OnContextMenu(pWnd, point))
	//	return;
	CMenu men;
	men.CreatePopupMenu();
	men.AppendMenu(MF_STRING, ID_SAVE_AS_TABLE, TR("Open as Table").c_str());
	int cmd = men.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD, point.x, point.y, pWnd);
	switch (cmd) {
		case ID_GR_COPY:
			break;
		case ID_SAVE_AS_TABLE:
			saveAsTbl();
			break;
	}
}

class TableNameForm : public FormWithDest {
public:
	TableNameForm(CWnd *par,String *name) : FormWithDest(par,TR("Open as table"),fbsSHOWALWAYS | fbsMODAL) {
		new FieldString(root,TR("Table name"),name);
		//create();
	}

	int exec() {
		FormWithDest::exec();
		return 1;
	}
};

void CrossSectionGraph::saveAsTbl() {
	String fname("CrossSection");
	if ( TableNameForm(this, &fname).DoModal() == IDOK) {
		int maxNo = values[0][0].size();
		FileName fnTable = FileName::fnUnique(FileName(fname,".tbt"));
		Table tbl(fnTable,Domain("none"));
		DomainValueRangeStruct dvInt(0 , maxNo);
		Column colIndex = tbl->colNew("Index",dvInt);
		colIndex->SetOwnedByTable();
		Column colMap = tbl->colNew("BaseMap",Domain("String"));
		colMap->SetOwnedByTable();
		Domain dmcrd;
		dmcrd.SetPointer(new DomainCoord(fldGraph->csy->fnObj));
		Column colCrd = tbl->colNew("Coordinate", dmcrd, ValueRange());
		colCrd->SetOwnedByTable();
		MapList mpl;
		ObjectCollection oc;
		for(int m =0; m < fldGraph->sources.size(); ++m) {
			int count = 0;
			Column colValue;
			IlwisObject obj = fldGraph->sources[m];
			String name =  String("%S", obj->fnObj.sFile);
			for(int i = 0; i < fldGraph->crdSelect.size(); ++i) {
				int noMaps = getNumberOfMaps(m);
				if ( noMaps != tbl->iRecs())
					tbl->iRecNew(noMaps);
				for(long j = 0; j < noMaps; ++j) {
					FileName fn = obj->fnObj;
					if ( IOTYPE(fn) == IlwisObject::iotMAPLIST) {
						mpl.SetPointer(obj.pointer());
						RangeReal rr = mpl->getRange();
						ValueRange vr( rr,0);
						colValue = tbl->colNew(name.sQuote(), Domain("value"),vr );
					} else if ( IOTYPE(fn) == IlwisObject::iotOBJECTCOLLECTION) {
						oc.SetPointer(obj.pointer());
						RangeReal rr = oc->getRange();
						ValueRange vr( rr,0);
						colValue = tbl->colNew(name.sQuote(), Domain("value"),vr );
					}

					colValue->SetOwnedByTable();
					BaseMap bmp;
					colCrd->PutVal(count,fldGraph->crdSelect[i]); 
					if ( mpl.fValid()) {
						bmp = mpl[j];
					} else if ( oc.fValid()) {
						IlwisObject objc = oc->ioObj(j);
						if ( IOTYPEBASEMAP(objc->fnObj))
							bmp = BaseMap(objc->fnObj);
					}
					if ( bmp.fValid()) {
						Coord crd = fldGraph->crdSelect[i];
						if ( bmp->cs() != fldGraph->csy)
							crd = bmp->cs()->cConv(fldGraph->csy, crd);
						String v = bmp->sValue(crd);
						colIndex->PutVal(count+1,j);
						colMap->PutVal(count+1,bmp->fnObj.sFile );
						colValue->PutVal(count + 1, v);
						colMap->PutVal(count+1, bmp->fnObj.sFile + bmp->fnObj.sExt);
					}
					++count;
				}
			}
		}
		tbl->Store();
		IlwWinApp()->Execute(String("Open %S",fnTable.sRelative())); 
	}
}


int CrossSectionGraph::getNumberOfMaps(long i) {
	IlwisObject obj = fldGraph->sources[i];
	if ( IOTYPE(obj->fnObj) == IlwisObject::iotMAPLIST) {
		MapList mpl(obj->fnObj);
		return mpl->iSize();

	}
	else if ( IOTYPE(obj->fnObj) == IlwisObject::iotOBJECTCOLLECTION) {
		ObjectCollection oc(obj->fnObj);
		oc->iNrObjects();

	}
	return iUNDEF;
}

BaseMap CrossSectionGraph::getBaseMap(long i, long m) {
	IlwisObject obj = fldGraph->sources[m];
	if ( IOTYPE(obj->fnObj) == IlwisObject::iotMAPLIST) {
		MapList mpl(obj->fnObj);
		return mpl[i];

	}
	else if ( IOTYPE(obj->fnObj) == IlwisObject::iotOBJECTCOLLECTION) {
		ObjectCollection oc(obj->fnObj);
		BaseMap bmp(oc->fnObject(i));
		return bmp;

	}
	return BaseMap();
}


void CrossSectionGraph::DrawItem(LPDRAWITEMSTRUCT lpDIS) {
	bool useDefault = false;
	CRect crct;
	GetClientRect(crct);
	Color c(GetSysColor(COLOR_3DFACE));
	CBrush brushBk(c);
	CPen pen(PS_SOLID,1,c);
	HGDIOBJ oldBrush = SelectObject(lpDIS->hDC, brushBk);
	HGDIOBJ oldPen = SelectObject(lpDIS->hDC, pen);
	::Rectangle(lpDIS->hDC, crct.left, crct.top, crct.right, crct.bottom);
	SelectObject(lpDIS->hDC, oldBrush);
	SelectObject(lpDIS->hDC, oldPen);

	CRect rct = CRect(crct.left, crct.top,crct.right, crct.bottom-20 );
	Color bkColor = GetBkColor(lpDIS->hDC);
	CBrush bbrushBk(RGB(0, 50, 100));
	SelectObject(lpDIS->hDC, bbrushBk);
	::Rectangle(lpDIS->hDC, rct.left,rct.top, rct.right, rct.bottom);
	SelectObject(lpDIS->hDC, oldBrush);


	CFont* fnt = new CFont();
	BOOL vvv = fnt->CreatePointFont(80,"Arial");
	CDC *dc = CDC::FromHandle(lpDIS->hDC);

	HGDIOBJ fntOld = dc->SelectObject(fnt);
	if ( fldGraph->crdSelect.size() == 0)
		return;

	int oldnr = iUNDEF;
	values.clear();
	for(int m =0; m < fldGraph->sources.size(); ++m) {
		int penStyle = m % 4;
		values.resize(fldGraph->sources.size());
		RangeReal rr = fldGraph->getRange(m);
		int numberOfMaps = getNumberOfMaps(m);

		double yscale = rct.Height() / rr.rWidth();
		double y0 = rr.rWidth() * yscale;
		double xscale = (double)rct.Width() / numberOfMaps;
		values[m].resize(fldGraph->crdSelect.size());
		double rx = 0;
		int f = 20;
		for(int p=0; p < fldGraph->crdSelect.size(); ++p) {
			Color clr = Representation::clrPrimary(p< 2 ? p + 1 : p + 2);
			CPen bpen(penStyle, 1, clr);
			SelectObject(lpDIS->hDC, bpen);
			rx = 0;
			for(int i = 0; i < numberOfMaps; ++i) {
				BaseMap bmp = getBaseMap(i, m);
				Coord crd = fldGraph->crdSelect[p];
				if ( bmp->cs() != fldGraph->csy)
					crd = bmp->cs()->cConv(fldGraph->csy, crd);
				double v = bmp->rValue(crd);
				values[m][p].push_back(v);
				int y = y0 - ( v - rr.rLo()) * yscale;
				if ( i == 0)
					dc->MoveTo(rx,y);
				else 
					dc->LineTo(rx,y);
				if ( i % 5 == 0) {
					String s("%d", i);
					CSize sz = dc->GetTextExtent(s.c_str(), s.size());
					dc->TextOut(rx - sz.cx / 2, crct.bottom - 16,s.c_str(),s.size());
				}
				rx += xscale;
			}
		}
		if ( oldnr != numberOfMaps) {
			rx = 0;
			for(int i = 0; i < numberOfMaps; ++i) {

				if ( i % 5 == 0) {
					dc->MoveTo(rx,rct.bottom);
					dc->LineTo(rx,rct.bottom + 3);
				}
				rx += xscale;
			}
		}
		oldnr = numberOfMaps;
	}
	fldGraph->fillList();

	SelectObject(lpDIS->hDC,oldPen);
	SelectObject(lpDIS->hDC, oldBrush);
	SelectObject(lpDIS->hDC, fntOld);
	fnt->DeleteObject();
	delete fnt;
}

void CrossSectionGraph::OnLButtonUp(UINT nFlags, CPoint point) {	
	CWnd *wnd =  GetParent();
	if ( wnd) {
		for(int m =0; m < fldGraph->sources.size(); ++m) {
			RangeReal rr = fldGraph->getRange(m);
			int numberOfMaps = getNumberOfMaps(m);
			CRect rct;
			GetClientRect(rct);
			double xscale = rct.Width() / numberOfMaps;
			fldGraph->currentIndex = point.x / xscale;
			if ( fldGraph->currentIndex >= values[m].size())
				continue;
		}
		fldGraph->fillList();
	}
}

void CrossSectionGraph::PreSubclassWindow() 
{
	EnableToolTips();

	CStatic::PreSubclassWindow();
}
//----------------------------------------------------
CrossSectionGraphEntry::CrossSectionGraphEntry(FormEntry* par, vector<IlwisObject>& _sources, const CoordSystem& cys) :
FormEntry(par,0,true),
crossSectionGraph(0),
listview(0),
currentIndex(iUNDEF),
sources(_sources),
csy(cys)
{
	psn->iMinWidth = psn->iWidth = CSGRPAH_SIZE;
	psn->iMinHeight = psn->iHeight = 140;
	SetIndependentPos();
}

void CrossSectionGraphEntry::create()
{
	zPoint pntFld = zPoint(psn->iPosX,psn->iPosY);
	zDimension dimFld = zDimension(psn->iWidth,psn->iMinHeight);
	crossSectionGraph = new CrossSectionGraph(this, WS_VISIBLE | WS_CHILD ,
		CRect(pntFld, dimFld) , frm()->wnd() , Id());
	crossSectionGraph->SetFont(frm()->fnt);
	CreateChildren();
}

void CrossSectionGraphEntry::fillList() {
	vector<String> dummy;
	listview->setData(-1, dummy);
	int count = max(1, crdSelect.size());
	for(int m =0; m < sources.size(); ++m) {
		IlwisObject obj = sources[m];
		for(int i = 0 ; i < count; ++i) {
			vector<String> v;
			v.push_back(String("%S%S",obj->fnObj.sFile,obj->fnObj.sExt));
			v.push_back(String("%d",i));
			if ( IOTYPE(obj->fnObj) == IlwisObject::iotMAPLIST) {
				MapList mpl(obj->fnObj);
				v.push_back(String("%d:%d",mpl->iLower(), mpl->iUpper()));

			} else if (IOTYPE(obj->fnObj) == IlwisObject::iotOBJECTCOLLECTION) {
				ObjectCollection oc(obj->fnObj);
				v.push_back(String("0:%d",oc->iNrObjects()));
			}
			RangeReal rr = getRange(m);
			v.push_back(String("%S", rr.s()));
			if ( currentIndex != iUNDEF) {
				v.push_back(String("%d", currentIndex));
				v.push_back(String("%g",crossSectionGraph->values[m][i][currentIndex]));
			}
			else{
				v.push_back("");
				v.push_back("");
			}
			listview->AddData(v);
		}
	}
	listview->update();
}

bool CrossSectionGraphEntry::isUnique(const FileName& fn) {

	for(int i=0; i < sources.size(); ++i) {
		if ( sources.at(i)->fnObj == fn)
			return false;
	}
	return true;
}

void CrossSectionGraphEntry::update() {
	if ( crossSectionGraph){
		crossSectionGraph->Invalidate();
	}
	listview->update();

}

void CrossSectionGraphEntry::reset() {
	crdSelect.clear();
	vector<String> dummy;
	listview->setData(-1,dummy);
}
//void CrossSectionGraphEntry::addSourceSet(const IlwisObject& obj){
//	//if ( isUnique(obj->fnObj)) {
//		
//		if (listview) {
//			int count = max(1, crdSelect.size());
//			for(int j = 0 ; j < count; ++j) {
//				sources.push_back(SourceInfo(obj, j));
//				vector<String> v;
//				v.push_back(String("%SS",obj->fnObj.sFile,obj->fnObj.sExt));
//				v.push_back(String("%d",j));
//				if ( IOTYPE(obj->fnObj) == IlwisObject::iotMAPLIST) {
//					MapList mpl(obj->fnObj);
//					v.push_back(String("%d:%d",mpl->iLower(), mpl->iUpper()));
//					RangeReal rr = getRange(sources.size() - 1);
//					v.push_back(String("%S", rr.s()));
//					v.push_back("?");
//					v.push_back("?");
//
//				} else if (IOTYPE(obj->fnObj) == IlwisObject::iotOBJECTCOLLECTION) {
//					ObjectCollection oc(obj->fnObj);
//				}
//				listview->AddData(v);
//			}
//		}
//		if ( crossSectionGraph)
//			crossSectionGraph->Invalidate();
////	}
//}

void CrossSectionGraphEntry::setCoord(const Coord& crd){
	if ( crd == Coord())
		return;

	crdSelect.push_back(crd);
	//int n = sources.size() / crdSelect.size();
	//for(int i=0; i < crdSelect.size(); ++i) {
	//	if ( i != 0) {
	//		int groupStart = n * crdSelect.size();
	//		if (i + groupStart < sources.size())
	//			sources.insert(sources.begin() + i + groupStart, SourceInfo(sources[i * n].obj, crdSelect.size()));
	//		else
	//			sources.push_back(SourceInfo(sources[i * n].obj, crdSelect.size()));
	//	}
	//	update();
	//}
	if ( crossSectionGraph) {
		crossSectionGraph->Invalidate();
	}

}

void CrossSectionGraphEntry::setListView(FieldListView *v) {
	listview = v;
	v->psn->iMinWidth = v->psn->iWidth = CSGRPAH_SIZE;
	v->psn->iMinHeight = v->psn->iHeight = 100;
}


RangeReal CrossSectionGraphEntry::getRange(long i) {
	IlwisObject obj = sources[i];
	if ( IOTYPE(obj->fnObj) == IlwisObject::iotMAPLIST) {
		if ( ranges.size() <= i) {
			MapList mpl(obj->fnObj);
			ranges.push_back(mpl->getRange());
		}

	}
	else if ( IOTYPE(obj->fnObj) == IlwisObject::iotOBJECTCOLLECTION) {
		if ( ranges.size() <= i) {
			ObjectCollection oc(obj->fnObj);
			ranges.push_back(oc->getRange());
		}
	}
	return ranges[i];
}




