//************************************************************************
//
// LCDBitmap.cpp
//
// The CLCDBitmap class draws bitmaps onto the LCD.
// 
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#include <windows.h>
#include <assert.h>
#include "LCDBitmap.h"


//************************************************************************
//
// CLCDBitmap::CLCDBitmap
//
//************************************************************************

CLCDBitmap::CLCDBitmap()
{
    m_hBitmap = NULL;
    m_dwROP = SRCCOPY;
}


//************************************************************************
//
// CLCDBitmap::CLCDBitmap
//
//************************************************************************

CLCDBitmap::~CLCDBitmap()
{

}


//************************************************************************
//
// CLCDBitmap::SetBitmap
//
//************************************************************************

void CLCDBitmap::SetBitmap(HBITMAP hBitmap)
{
    assert(NULL != hBitmap);
    m_hBitmap = hBitmap;
}

void CLCDBitmap::SetBitmap(ResourceId _id, int _sx, int _sy) 
{ 
	m_hBitmap = ((HBITMAP)LoadImage(GetModuleHandle("g15_resources.dll"), 
		MAKEINTRESOURCE(_id.GetId()),  
		IMAGE_BITMAP, _sx, _sy,  
		LR_MONOCHROME)); 
}
//************************************************************************
//
// CLCDBitmap::SetBitmap
//
//************************************************************************

void CLCDBitmap::SetROP(DWORD dwROP)
{
    m_dwROP = dwROP;
}


//************************************************************************
//
// CLCDBitmap::OnDraw
//
//************************************************************************

void CLCDBitmap::OnDraw(CLCDGfx &rGfx)
{
    if(m_hBitmap)
    {
        HDC hCompatibleDC = CreateCompatibleDC(rGfx.GetHDC());
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hCompatibleDC, m_hBitmap);
        
        BitBlt(rGfx.GetHDC(), 0, 0, m_Size.cx, m_Size.cy, hCompatibleDC, m_ptLogical.x, m_ptLogical.y, m_dwROP);
        
        // restores
        SelectObject(hCompatibleDC, hOldBitmap);
        DeleteDC(hCompatibleDC);
    }
}


//** end of LCDBitmap.cpp ************************************************
