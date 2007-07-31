////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2007-01-06 17:41:11 -0800 (Sat, 06 Jan 2007) $
// $LastChangedRevision: 1491 $
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
	obColor(obint32 _color)
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

	inline float rF() const	{ return (float)cdata.m_RGBA[0] / 255.0f; }
	inline float gF() const	{ return (float)cdata.m_RGBA[1] / 255.0f; }
	inline float bF() const	{ return (float)cdata.m_RGBA[2] / 255.0f; }
	inline float aF() const	{ return (float)cdata.m_RGBA[3] / 255.0f; }

	inline obint32 rgba() const { return cdata.m_RGBAi; }
private:
	union cdatatype
	{
		obuint8		m_RGBA[4];
		obint32		m_RGBAi;
	} cdata;
};

#endif
