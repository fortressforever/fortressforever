//************************************************************************
//
// LCDSampleScreen.h
//
// This is a sample Screen class showing how to initialize screen controls
// and handle LCD button input
// 
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#ifndef _LCDSAMPLESCREEN_H_INCLUDED_
#define _LCDSAMPLESCREEN_H_INCLUDED_

#include "LCDManager.h"
#include "LCDText.h"
#include "LCDIcon.h"
#include "LCDBitmap.h"

class CLCDSampleScreen : public CLCDManager  
{
public:
	CLCDSampleScreen();
	virtual ~CLCDSampleScreen();

    // CLCDManager
    virtual HRESULT Initialize(void);
    virtual void OnLCDButtonDown(int nButton);
    virtual void OnLCDButtonUp(int nButton);

    // LCD Text
    CLCDText m_Text;

    // LCD Icon
    CLCDIcon m_Icon;

protected:
    // LCD Button States
    CLCDBitmap m_Buttons[4];
};

#endif // !_LCDSAMPLESCREEN_H_INCLUDED_

//** end of LCDSampleScreen.h ********************************************

