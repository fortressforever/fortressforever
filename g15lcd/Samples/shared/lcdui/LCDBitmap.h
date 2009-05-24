//************************************************************************
//
// LCDBitmap.h
//
// The CLCDBitmap class draws bitmaps onto the LCD.
// 
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#ifndef _LCDBITMAP_H_INCLUDED_ 
#define _LCDBITMAP_H_INCLUDED_ 

#include "LCDBase.h"

class ResourceId
{
public:

	int GetId() const { return m_ResourceId; }
	ResourceId(int _id) : m_ResourceId(_id) {}
private:
	int	m_ResourceId;
};

class CLCDBitmap : public CLCDBase
{
public:
    CLCDBitmap();
    virtual ~CLCDBitmap();

    void SetBitmap(HBITMAP hBitmap);
	void SetBitmap(ResourceId _id, int _sx, int _sy);
    void SetROP(DWORD dwROP);

protected:
    virtual void OnDraw(CLCDGfx &rGfx);
    HBITMAP m_hBitmap;
    DWORD m_dwROP;

private:
};


#endif // !_LCDBITMAP_H_INCLUDED_ 

//** end of LCDBitmap.h **************************************************
