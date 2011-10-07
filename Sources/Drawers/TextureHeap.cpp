// TextureHeap.cpp: implementation of the TextureHeap class.
//
//////////////////////////////////////////////////////////////////////

#include "Headers\toolspch.h"
#include "Engine\Drawers\ComplexDrawer.h"
#include "Engine\Map\Raster\Map.h"
#include "Engine\Map\Raster\MapList\maplist.h"
#include "Engine\Drawers\RootDrawer.h"
#include "Drawers\DrawingColor.h" 
#include "Drawers\LayerDrawer.h"
#include "Drawers\RasterLayerDrawer.h"
#include "TextureHeap.h"
#include "Texture.h"
#include "CCTexture.h"
#include "Engine\Drawers\DrawerContext.h"
#include "DrawingColor.h"
#include "LayerDrawer.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using namespace ILWIS;

//////////////////////////////////////////////////
// Palette
//////////////////////////////////////////////////

Palette::Palette()
: rsd(0)
, iPaletteSize(0)
, palette_reds(0)
, palette_greens(0)
, palette_blues(0)
, palette_alphas(0)
{
}

Palette::~Palette()
{
	if (palette_reds)
		delete [] palette_reds;
	if (palette_greens)
		delete [] palette_greens;
	if (palette_blues)
		delete [] palette_blues;
	if (palette_alphas)
		delete [] palette_alphas;
}

void Palette::SetData(const Map & mp, const LayerDrawer * rsd, const unsigned int iPaletteSize, const RangeReal & rrMinMaxMap)
{
	ValueRange vr = mp->vr();
	if (mp->dm()->pdbool())
		vr = ValueRange();
	if (vr.fValid()) // when integers are not good enough to represent the map treat it as a real map
		fRealMap = (vr->rStep() < 1) || (vr->stUsed() == stREAL);
	else
		fRealMap = false;
	
	this->rsd = rsd;
	this->rrMinMaxMap.rLo() = rrMinMaxMap.rLo();
	this->rrMinMaxMap.rHi() = rrMinMaxMap.rHi();

	if (this->iPaletteSize != iPaletteSize && iPaletteSize > 2) {
		this->iPaletteSize = iPaletteSize;
		if (palette_reds)
			delete [] palette_reds;
		if (palette_greens)
			delete [] palette_greens;
		if (palette_blues)
			delete [] palette_blues;
		if (palette_alphas)
			delete [] palette_alphas;
		palette_reds = new float [iPaletteSize];
		palette_greens = new float [iPaletteSize];
		palette_blues = new float [iPaletteSize];
		palette_alphas = new float [iPaletteSize];
		palette_alphas[iPaletteSize - 1] = 0.0; // by definition, last index reserved for UNDEF
	}
}

bool Palette::fValid()
{
	return iPaletteSize > 2; // a vaild palette has at least colors for mapMin, mapMax and UNDEF.
}

void Palette::MakeCurrent() const
{
	glPixelMapfv(GL_PIXEL_MAP_I_TO_R, iPaletteSize, palette_reds);
	glPixelMapfv(GL_PIXEL_MAP_I_TO_G, iPaletteSize, palette_greens);
	glPixelMapfv(GL_PIXEL_MAP_I_TO_B, iPaletteSize, palette_blues);
	glPixelMapfv(GL_PIXEL_MAP_I_TO_A, iPaletteSize, palette_alphas);
}

// Call Refresh() when: 1. Map data has changed, 2. Representation has changed
void Palette::Refresh()
{
	if (fValid()) {
		unsigned int nrMapValues = iPaletteSize - 1;
		double width = rrMinMaxMap.rWidth();
		double minMapVal = rrMinMaxMap.rLo();

		const DrawingColor * drawColor = rsd->getDrawingColor();
		long * bufColor = new long [nrMapValues];

		if (fRealMap) {
			double * buf = new double [nrMapValues];
			for (int i = 0; i < nrMapValues; ++i)
				buf[i] = minMapVal + i * width / (nrMapValues - 1);
			drawColor->clrVal(buf, bufColor, nrMapValues);
			delete [] buf;
		} else {
			const NewDrawer::DrawMethod drm = rsd->getDrawMethod();
			long * buf = new long [nrMapValues];
			for (int i = 0; i < nrMapValues; ++i)
				buf[i] = minMapVal + round(i * width / (nrMapValues - 1));
			drawColor->clrRaw(buf, bufColor, nrMapValues, drm);
			delete [] buf;
		}

		for (int i = 0; i < nrMapValues; ++i) {
			Color col(bufColor[i]);
			palette_reds[i] = col.redP();
			palette_greens[i] = col.greenP();
			palette_blues[i] = col.blueP();
			palette_alphas[i] = col.alphaP();
		}

		delete [] bufColor;
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TextureHeap::TextureHeap()
: texturesArraySize(0)
, readpos(0)
, writepos(0)
, textureThread(0)
, fAbortTexGen(false)
, fStopThread(false)
{
	for (int i = 0; i < 10000; ++i)
		textures[i] = 0;
}

TextureHeap::~TextureHeap()
{
	if (textureThread)
	{
		fStopThread = true;
		textureThread->ResumeThread();
		csThread.Lock(); // wait here til thread exits
		csThread.Unlock();
	}

	ClearQueuedTextures();

	for (int i = 0; i < texturesArraySize; ++i)
		if (textures[i] != 0)
			delete textures[i];
}

void TextureHeap::SetData(const Map & _mp, const DrawingColor * drawColor, const NewDrawer::DrawMethod drm, const unsigned int iPaletteSize, RasterSetData *data, const RangeReal & rrMinMaxMap, DrawerContext * drawerContext)
{
	this->mp.SetPointer(_mp.pointer());
	this->drawColor = drawColor;
	this->drm = drm;
	this->iPaletteSize = iPaletteSize;
	this->imgWidth2 = data->width;
	this->imgHeight2 = data->height;
	this->rrMinMaxMap.rLo() = rrMinMaxMap.rLo();
	this->rrMinMaxMap.rHi() = rrMinMaxMap.rHi();
	this->drawerContext = drawerContext;
}

void TextureHeap::SetData(const MapList & _mpl, const DrawingColor * drawColor, const NewDrawer::DrawMethod drm, const unsigned int iPaletteSize, RasterSetData *_data, const RangeReal & rrMinMaxMap, DrawerContext * drawerContext)
{
	this->mpl.SetPointer(_mpl.pointer());
	this->drawColor = drawColor;
	this->drm = drm;
	this->iPaletteSize = iPaletteSize;
	this->imgWidth2 = _data->width;
	this->imgHeight2 = _data->height;
	this->rrMinMaxMap.rLo() = rrMinMaxMap.rLo();
	this->rrMinMaxMap.rHi() = rrMinMaxMap.rHi();
	this->drawerContext = drawerContext;
	data = _data;
}

bool TextureHeap::fValid()
{
	return drawerContext != 0;
}

void TextureHeap::ClearQueuedTextures()
{
	fAbortTexGen = true;
	csChangeTexCreatorList.Lock(); // wait for TexGen thread to stop
	fAbortTexGen = false;
	while (readpos != writepos)
	{
		delete textureRequest[readpos];
		readpos = (readpos + 1) % BUF_SIZE;
	}
	csChangeTexCreatorList.Unlock();
}

void TextureHeap::RepresentationChanged()
{
	for (int i = 0; i < texturesArraySize; ++i)
		textures[i]->RepresentationChanged();	
}

Texture * TextureHeap::GetTexture(const unsigned int offsetX, const unsigned int offsetY, const unsigned int sizeX, const unsigned int sizeY, unsigned int zoomFactor, const Palette * palette, bool fInThread)
{
	Texture * tex = 0;
	if (fInThread) { // call Invalidate when done, to redraw the mapwindow
		for (int i = 0; i < texturesArraySize; ++i) {
			if (textures[i]->equals(offsetX, offsetY, offsetX + sizeX, offsetY + sizeY, zoomFactor)) {
				textures[i]->BindMe(drawerContext);
				return textures[i];
			} else if (textures[i]->contains(offsetX, offsetY, offsetX + sizeX, offsetY + sizeY)) {
				if (tex != 0) {
					if (tex->getZoomFactor() > textures[i]->getZoomFactor())
						tex = textures[i];
				} else
					tex = textures[i];
			}
		}

		GenerateTexture(offsetX, offsetY, sizeX, sizeY, zoomFactor, palette, fInThread);
	} else { // caller is waiting for the Texture*
		for (int i = 0; i < texturesArraySize; ++i) {
			if (textures[i]->equals(offsetX, offsetY, offsetX + sizeX, offsetY + sizeY, zoomFactor))
				tex = textures[i];
		}
		if (0 == tex)
			tex = GenerateTexture(offsetX, offsetY, sizeX, sizeY, zoomFactor, palette, fInThread);
	}

	if (tex != 0)
		tex->BindMe(drawerContext);

	return tex;
}

Texture * TextureHeap::GenerateTexture(const unsigned int offsetX, const unsigned int offsetY, const unsigned int sizeX, const unsigned int sizeY, unsigned int zoomFactor, const Palette * palette, bool fInThread)
{
	if (((writepos + 1) % BUF_SIZE) != readpos) {
		if ( mp.fValid())
			textureRequest[writepos] = new Texture(mp, drawColor, drm, offsetX, offsetY, sizeX, sizeY, imgWidth2, imgHeight2, zoomFactor, iPaletteSize, rrMinMaxMap, palette);
		else
			textureRequest[writepos] = new CCTexture(mpl, drawColor, drm, offsetX, offsetY, sizeX, sizeY, data, zoomFactor, rrMinMaxMap);
		writepos = (writepos + 1) % BUF_SIZE;
	}
	if (fInThread) {
		if (!textureThread)
			textureThread = AfxBeginThread(GenerateTexturesInThread, this);
		else
			textureThread->ResumeThread();
	} else {
		Texture * tex = GenerateNextTexture(fInThread);
		if (tex != 0)
			return tex;
	}
	return 0;
}

Texture * TextureHeap::GenerateNextTexture(bool fInThread)
{
	Texture * tex = 0;
	csChangeTexCreatorList.Lock();
	if (!fAbortTexGen && !fStopThread && readpos != writepos) {
		tex = textureRequest[readpos];
		textureRequest[readpos] = 0;
		readpos = (readpos + 1) % BUF_SIZE;
	}
	csChangeTexCreatorList.Unlock();

	if (tex != 0) {
		clock_t start = clock();
		tex->CreateTexture(drawerContext, fInThread, &fAbortTexGen);
		clock_t end = clock();
		double duration = 1000.0 * (double)(end - start) / CLOCKS_PER_SEC;
		//TRACE("Texture generated in %2.2f milliseconds;\n", duration);
		if (tex->fValid())
			textures[texturesArraySize++] = tex;
		else {
			delete tex;
			tex = 0;
		}
	}
	return tex;
}

UINT TextureHeap::GenerateTexturesInThread(LPVOID pParam)
{
	TextureHeap * pObject = (TextureHeap*)pParam;
	if (pObject == NULL)
	{
		pObject->textureThread = 0;
		return 1;
	}

	pObject->csThread.Lock();

	while (!pObject->fStopThread)
	{
		Texture * tex = pObject->GenerateNextTexture(true);
		while (tex != 0)
			tex = pObject->GenerateNextTexture(true);
		if (!pObject->fAbortTexGen && !pObject->fStopThread)
			pObject->drawerContext->doDraw();
		if (!pObject->fStopThread)
			pObject->textureThread->SuspendThread(); // wait here, and dont consume CPU time either
	}

	pObject->fStopThread = false;
	pObject->csThread.Unlock();

	return 0;
}

