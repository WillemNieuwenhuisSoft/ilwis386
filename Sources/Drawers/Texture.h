// Texture.h: interface for the Texture class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEXTURE_H__E4F62490_7FB3_423D_B131_349672E6F490__INCLUDED_)
#define AFX_TEXTURE_H__E4F62490_7FB3_423D_B131_349672E6F490__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <GL/gl.h>

#include "Client\MapWindow\Drawers\ComplexDrawer.h" // for DrawMethod
#include "Client\MapWindow\Drawers\Drawer_n.h" // otherwise DrawingColor can't compile
#include "Drawers\DrawingColor.h"

namespace ILWIS {
	class Texture  
	{
	public:
		Texture(const Map & mp, const DrawingColor * drawColor, const ComplexDrawer::DrawMethod drm, const long offsetX, const long offsetY, const long sizeX, const long sizeY, char * scrap_data_mipmap, GLdouble xMin, GLdouble yMin, GLdouble xMax, GLdouble yMax, unsigned int zoomFactor, DrawerContext * drawerContext, bool fInThread, volatile bool* fDrawStop);
		virtual ~Texture();

		void BindMe(); // To be called before glBegin
		void TexCoord2d(GLdouble x, GLdouble y); // To be called repeatedly between glBegin and glEnd // These are in world coordinates !!
		bool equals(GLdouble xMin, GLdouble yMin, GLdouble xMax, GLdouble yMax, unsigned int zoomFactor);
		bool contains(GLdouble xMin, GLdouble yMin, GLdouble xMax, GLdouble yMax);
		unsigned int getZoomFactor();
		bool fValid();

	private:
		void ConvLine(LongBuf& buf, const int iLine, const long texSizeX, char * outbuf);
		void ConvLine(const RealBuf& buf, const int iLine, const long texSizeX, char * outbuf);
		void DrawTexture(long offsetX, long offsetY, long texSizeX, long texSizeY, unsigned int zoomFactor, char * outbuf, volatile bool* fDrawStop);
		Map mp;
		GLuint texture;
		GLdouble xMin, yMin, xMax, yMax; // These are in world coordinates !! These are to be mapped to texture coordinates 0 to 1
		unsigned int zoomFactor;
		const DrawingColor * drawColor;
		const ComplexDrawer::DrawMethod drm;
		bool fValue;
		bool fAttTable;
		bool valid;
	};
}

#endif // !defined(AFX_TEXTURE_H__E4F62490_7FB3_423D_B131_349672E6F490__INCLUDED_)
