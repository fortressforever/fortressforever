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


#include "stdafx.h"
#include "framework.h"
#include "LCDSampleScreen.h"


//************************************************************************
//
// CLCDSampleScreen::CLCDSampleScreen
//
//************************************************************************

CLCDSampleScreen::CLCDSampleScreen()
{
    
}


//************************************************************************
//
// CLCDSampleScreen::~CLCDSampleScreen
//
//************************************************************************

CLCDSampleScreen::~CLCDSampleScreen()
{

}


//************************************************************************
//
// CLCDSampleScreen::Initialize
//
//************************************************************************

HRESULT CLCDSampleScreen::Initialize()
{

    // Initialize the hello world text control
    m_Text.Initialize();
    m_Text.SetOrigin(0, 0);
    m_Text.SetSize(160, 30);
    m_Text.SetAlignment(DT_CENTER);
    m_Text.SetWordWrap(TRUE);
    m_Text.SetText(_T("Hello World"));

    // Initilize the icon
    m_Icon.Initialize();
    m_Icon.SetOrigin(0, 5);
    m_Icon.SetSize(32, 32);
    m_Icon.SetIcon((HICON)LoadImage(AfxGetResourceHandle(),
        MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON,
        32, 32, LR_MONOCHROME), 32, 32);

    // Initialize the bitmap buttons. These bitmaps are hidden and show
    // when the respective lcd button is pressed
    for (int i = 0; i < 4; i++)
    {
        m_Buttons[i].Initialize();
        m_Buttons[i].SetSize(8, 2);
        m_Buttons[i].Show(FALSE);
        m_Buttons[i].SetBitmap((HBITMAP)LoadImage(AfxGetResourceHandle(),
            MAKEINTRESOURCE(IDB_BTN_DN), IMAGE_BITMAP,
            16, 16, LR_MONOCHROME));

        // position the bitmaps by button
        switch(i)
        {
        case 0:
            m_Buttons[0].SetOrigin(10, 40);
            break;
        case 1:
            m_Buttons[1].SetOrigin(50, 40);
            break;
        case 2:
            m_Buttons[2].SetOrigin(110, 40);
            break;
        case 3:
            m_Buttons[3].SetOrigin(150, 40);
            break;
        default:
            break;
        }
    }

    // Add the controls to the screen internal list
    AddObject(&m_Icon);
    AddObject(&m_Text);
    AddObject(&m_Buttons[0]);
    AddObject(&m_Buttons[1]);
    AddObject(&m_Buttons[2]);
    AddObject(&m_Buttons[3]);

    // must call the base class
    return CLCDManager::Initialize();
}


//************************************************************************
//
// CLCDSampleScreen::OnLCDButtonDown
//
// This is called automatically by the CLCDOutput class when a button is
// pressed
//************************************************************************

void CLCDSampleScreen::OnLCDButtonDown(int nButton)
{
    switch(nButton)
    {
    case LGLCDBUTTON_BUTTON0:
        m_Buttons[0].Show(TRUE);
        break;
    case LGLCDBUTTON_BUTTON1:
        m_Buttons[1].Show(TRUE);
        break;
    case LGLCDBUTTON_BUTTON2:
        m_Buttons[2].Show(TRUE);
        break;
    case LGLCDBUTTON_BUTTON3:
        m_Buttons[3].Show(TRUE);
        break;
    default:
        break;
    }
}


//************************************************************************
//
// CLCDSampleScreen::OnLCDButtonUp
//
// This is called automatically by the CLCDOutput class when a button is
// released
//************************************************************************

void CLCDSampleScreen::OnLCDButtonUp(int nButton)
{
    switch(nButton)
    {
    case LGLCDBUTTON_BUTTON0:
        m_Buttons[0].Show(FALSE);
        break;
    case LGLCDBUTTON_BUTTON1:
        m_Buttons[1].Show(FALSE);
        break;
    case LGLCDBUTTON_BUTTON2:
        m_Buttons[2].Show(FALSE);
        break;
    case LGLCDBUTTON_BUTTON3:
        m_Buttons[3].Show(FALSE);
        break;
    default:
        break;
    }
}


//** end of LCDSampleScreen.cpp ******************************************

