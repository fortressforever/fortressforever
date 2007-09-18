//************************************************************************
//
// LCDGfx.h
//
// The CLCDGfx class abstracts GDI/bitmap details. It is used in the
// OnDraw event.
// 
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#ifndef _LCDGFX_H_INCLUDED_ 
#define _LCDGFX_H_INCLUDED_ 

#include <lglcd.h>

class CLCDGfx
{
public:
    CLCDGfx(void);
    virtual ~CLCDGfx(void);

    HRESULT Initialize(int nWidth, int nHeight);
    void Shutdown(void);

    void BeginDraw(void);
    void ClearScreen(void);
    void SetPixel(int nX, int nY, BYTE bValue);
    void DrawLine(int nX1, int nY1, int nX2, int nY2);
    void DrawFilledRect(int nX, int nY, int nWidth, int nHeight);
    void DrawText(int nX, int nY, LPCTSTR sText);
    void EndDraw(void);

    HDC GetHDC(void);
    lgLcdBitmap160x43x1 *GetLCDScreen(void);
    BITMAPINFO *GetBitmapInfo(void);
    HBITMAP GetHBITMAP(void);
    
protected:
    int m_nWidth;
    int m_nHeight;
    lgLcdBitmap160x43x1 *m_pLCDScreen;
    BITMAPINFO *m_pBitmapInfo;
    HDC m_hDC;
    HBITMAP m_hBitmap;
    HBITMAP m_hPrevBitmap;
    PBYTE m_pBitmapBits;
};


#endif // !_LCDGFX_H_INCLUDED_ 

//** end of LCDGfx.h *****************************************************
