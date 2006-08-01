////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2005-12-27 08:19:43 -0500 (Tue, 27 Dec 2005) $
// $LastChangedRevision: 1094 $
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __COLOR_H__
#define __COLOR_H__

#include "Omni-Bot_BasicTypes.h"

// class: obColor
//		Helper class for defining color values.
class obColor
{
public:
	obColor()
	{
		// initialize to white
		cdata.m_RGBA[0] = 255;
		cdata.m_RGBA[1] = 255;
		cdata.m_RGBA[2] = 255;
		cdata.m_RGBA[3] = 255; // 255 is opaque, 0 is transparent
	}
	obColor(unsigned int _color)
	{
		cdata.m_RGBAi = _color;
	}
	obColor(obuint8 _r, obuint8 _g, obuint8 _b, obuint8 _a = 255)
	{
		cdata.m_RGBA[0] = _r;
		cdata.m_RGBA[1] = _g;
		cdata.m_RGBA[2] = _b;
		cdata.m_RGBA[3] = _a; // 255 is opaque, 0 is transparent
	}

	inline obuint8 r() const	{ return cdata.m_RGBA[0]; }
	inline obuint8 g() const	{ return cdata.m_RGBA[1]; }
	inline obuint8 b() const	{ return cdata.m_RGBA[2]; }
	inline obuint8 a() const	{ return cdata.m_RGBA[3]; }

	inline obint32 rgba() const { return cdata.m_RGBAi; }
private:
	union cdatatype
	{
		obuint8		m_RGBA[4];
		obint32		m_RGBAi;
	} cdata;
};

#endif
