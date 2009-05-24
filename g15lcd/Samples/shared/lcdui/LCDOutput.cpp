//************************************************************************
//
// LCDOutput.cpp
//
// The CLCDOutput class manages LCD hardware enumeration and screen 
// management.
// 
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#include <windows.h>
#include <assert.h>
#include "LCDOutput.h"

#pragma comment(lib, "lgLcd.lib")

// to keep track of clients that use multiple CLCDOutput instances
// within the same app
static LONG lInitCount = 0;

//************************************************************************
//
// CLCDOutput::CLCDOutput
//
//************************************************************************

CLCDOutput::CLCDOutput()
{
    m_pActiveScreen = NULL;
    m_bLocked = FALSE;
    m_hDevice = LGLCD_INVALID_DEVICE;
    m_hConnection = LGLCD_INVALID_CONNECTION;
    m_nPriority = LGLCD_PRIORITY_ALERT;
    ZeroMemory(&m_lcdConnectCtx, sizeof(m_lcdConnectCtx));
    m_bDisplayLocked = FALSE;
}

//************************************************************************
//
// CLCDOutput::~CLCDOutput
//
//************************************************************************

CLCDOutput::~CLCDOutput()
{

}


//************************************************************************
//
// CLCDOutput::Initialize
//
//************************************************************************

HRESULT CLCDOutput::Initialize()
{
    return Initialize(NULL, FALSE);
}


//************************************************************************
//
// CLCDOutput::Initialize
//
// NOTE: Initialize should always return S_OK
//************************************************************************

HRESULT CLCDOutput::Initialize(lgLcdConnectContext* pContext, BOOL bUseWindow)
{    

    UNREFERENCED_PARAMETER(bUseWindow);

    DWORD res = ERROR_SUCCESS;

    res = CLCDManager::Initialize();
	if (ERROR_SUCCESS != res)
		return res;

    // initialize our screens
    LCD_MGR_LIST::iterator it = m_LCDMgrList.begin();
    while(it != m_LCDMgrList.end())
    {
        CLCDManager *pMgr = *it;
        assert(NULL != pMgr);

        pMgr->Initialize();
        ++it;
    }

    // LCD Stuff
    assert(lInitCount >= 0);
    if(1 == InterlockedIncrement(&lInitCount))
    {
        // need to call lgLcdInit once
        res = lgLcdInit();
        if (ERROR_SUCCESS != res)
        {
            InterlockedDecrement(&lInitCount);
            return E_FAIL;
        }
    }

    
    m_lcdConnectCtx.appFriendlyName = "My App";
    m_lcdConnectCtx.isPersistent = FALSE;
    m_lcdConnectCtx.isAutostartable = FALSE;
    m_lcdConnectCtx.connection = LGLCD_INVALID_CONNECTION;

    // if user passed in the context, fill it up
    if (NULL != pContext)
    {
        memcpy(&m_lcdConnectCtx, pContext, sizeof(lgLcdConnectContext));
    }

    return S_OK;
}

//************************************************************************
//
// CLCDOutput::Shutdown
//
//************************************************************************

void CLCDOutput::Shutdown(void)
{
    CloseAndDisconnect();
    if(0 == InterlockedDecrement(&lInitCount))
    {
        lgLcdDeInit();
    }
    assert(lInitCount >= 0);
}


//************************************************************************
//
// CLCDOutput::Draw
//
//************************************************************************

HRESULT CLCDOutput::Draw()
{
    DWORD dwPriorityToUse;
    
    if (m_pActiveScreen)
    {
        m_pActiveScreen->Draw();
        dwPriorityToUse = LGLCD_ASYNC_UPDATE(m_nPriority);
    }
    else
    {
        dwPriorityToUse = LGLCD_ASYNC_UPDATE(LGLCD_PRIORITY_IDLE_NO_SHOW);
    }
    
    lgLcdBitmap160x43x1* pScreen = GetLCDScreen();
    if (pScreen && (LGLCD_INVALID_DEVICE != m_hDevice))
    {
        DWORD res = ERROR_SUCCESS;
        res = lgLcdUpdateBitmap(m_hDevice, &pScreen->hdr, dwPriorityToUse);
        
        HandleErrorFromAPI(res);

        // read the soft buttons
        ReadButtons();
    }

    return S_OK;
}


//************************************************************************
//
// CLCDOutput::Update
//
//************************************************************************

void CLCDOutput::Update(DWORD dwTimestamp)
{
    if (m_pActiveScreen)
    {
        m_pActiveScreen->Update(dwTimestamp);
    }

    // check for expiration
    if (m_pActiveScreen && m_pActiveScreen->HasExpired())
    {
        m_pActiveScreen = NULL;
        //m_nPriority = LGLCD_PRIORITY_FYI; -> needs to go so that if a 
		// program sets priority to LGLCD_PRIORITY_BACKGROUND, that 
		// priority sticks.

        OnScreenExpired(m_pActiveScreen);

        // find the next active screen
        LCD_MGR_LIST::iterator it = m_LCDMgrList.begin();
        while(it != m_LCDMgrList.end())
        {
            CLCDManager *pMgr = *it;
            assert(NULL != pMgr);

            if (!pMgr->HasExpired())
            {
                ActivateScreen(pMgr);
                //m_nPriority = LGLCD_PRIORITY_FYI;  -> needs to go so that if a 
				// program sets priority to LGLCD_PRIORITY_BACKGROUND, that 
				// priority sticks.
                break;
            }

            ++it;
        }
    }

    // check for lcd devices
    if (LGLCD_INVALID_DEVICE == m_hDevice)
    {
        EnumerateDevices();
    }
}


//************************************************************************
//
// CLCDOutput::HasHardwareChanged
//
//************************************************************************

BOOL CLCDOutput::HasHardwareChanged(void)
{
    if(LGLCD_INVALID_DEVICE != m_hDevice)
    {
        // ping to see whether we're still alive
        DWORD dwButtonState = 0;
        
        DWORD res = lgLcdReadSoftButtons(m_hDevice, &dwButtonState);

        HandleErrorFromAPI(res);
    }

    // check for lcd devices
    if (LGLCD_INVALID_DEVICE == m_hDevice)
    {
        EnumerateDevices();
    }
    else
    {
        // we still have our device;
        return FALSE;
    }

    // we got a new device
    return LGLCD_INVALID_DEVICE != m_hDevice;
}


//************************************************************************
//
// CLCDOutput::GetLCDScreen
//
//************************************************************************

lgLcdBitmap160x43x1 *CLCDOutput::GetLCDScreen(void)
{
    return m_pActiveScreen ? m_pActiveScreen->GetLCDScreen() : CLCDManager::GetLCDScreen();
}


//************************************************************************
//
// CLCDOutput::GetBitmapInfo
//
//************************************************************************

BITMAPINFO *CLCDOutput::GetBitmapInfo(void)
{
    return m_pActiveScreen ? m_pActiveScreen->GetBitmapInfo() : CLCDManager::GetBitmapInfo();
}


//************************************************************************
//
// CLCDOutput::ReadButtons
//
//************************************************************************

void CLCDOutput::ReadButtons()
{
    if(IsOpened())
    {
        DWORD dwButtonState = 0;
        
        DWORD res = lgLcdReadSoftButtons(m_hDevice, &dwButtonState);
        if (ERROR_SUCCESS != res)
        {
            HandleErrorFromAPI(res);
        }
        
        if (m_dwButtonState == dwButtonState)
            return;
        
        // handle the buttons
        HandleButtonState(dwButtonState, LGLCDBUTTON_BUTTON0);
        HandleButtonState(dwButtonState, LGLCDBUTTON_BUTTON1);
        HandleButtonState(dwButtonState, LGLCDBUTTON_BUTTON2);
        HandleButtonState(dwButtonState, LGLCDBUTTON_BUTTON3);
        
        m_dwButtonState = dwButtonState;
    }
}


//************************************************************************
//
// CLCDOutput::HandleButtonState
//
//************************************************************************

void CLCDOutput::HandleButtonState(DWORD dwButtonState, DWORD dwButton)
{
    if ( (m_dwButtonState & dwButton) && !(dwButtonState & dwButton) )
    {
        OnLCDButtonUp(dwButton);
    }
    if ( !(m_dwButtonState & dwButton) && (dwButtonState & dwButton) )
    {
        OnLCDButtonDown(dwButton);
    }
}


//************************************************************************
//
// CLCDOutput::OnLCDButtonDown
//
//************************************************************************

void CLCDOutput::OnLCDButtonDown(int nButton)
{
    if (m_pActiveScreen)
    {
        m_pActiveScreen->OnLCDButtonDown(nButton);
    }
}


//************************************************************************
//
// CLCDOutput::OnLCDButtonUp
//
//************************************************************************

void CLCDOutput::OnLCDButtonUp(int nButton)
{
    if (m_pActiveScreen)
    {
        m_pActiveScreen->OnLCDButtonUp(nButton);
    }
}


//************************************************************************
//
// CLCDOutput::ActivateScreen
//
//************************************************************************

void CLCDOutput::ActivateScreen(CLCDManager* pScreen)
{
    if (m_bLocked)
        return;
    m_pActiveScreen = pScreen;
}


//************************************************************************
//
// CLCDOutput::LockScreen
//
//************************************************************************

void CLCDOutput::LockScreen(CLCDManager* pScreen)
{
    if (m_bLocked)
        return;

    m_pActiveScreen = pScreen;
    m_bLocked = TRUE;
}


//************************************************************************
//
// CLCDOutput::UnlockScreen
//
//************************************************************************

void CLCDOutput::UnlockScreen()
{
    m_bLocked = FALSE;
    m_pActiveScreen = NULL;
}


//************************************************************************
//
// CLCDOutput::IsLocked
//
//************************************************************************

BOOL CLCDOutput::IsLocked()
{
    return m_bLocked;
}


//************************************************************************
//
// CLCDOutput::AddScreen
//
//************************************************************************

void CLCDOutput::AddScreen(CLCDManager* pScreen)
{
    m_LCDMgrList.push_back(pScreen);
}


//************************************************************************
//
// CLCDOutput::EnumerateDevices
//
//************************************************************************

void CLCDOutput::EnumerateDevices()
{
    lgLcdDeviceDesc desc;

    if (LGLCD_INVALID_CONNECTION == m_hConnection)
    {
        if (ERROR_SUCCESS == lgLcdConnect(&m_lcdConnectCtx))
        {
            // make sure we don't work with a stale device handle
            m_hConnection = m_lcdConnectCtx.connection;
            m_hDevice = LGLCD_INVALID_CONNECTION;
        }
        else
        {
            return;
        }
    }

    // close the lcd device before we open up another
    if (LGLCD_INVALID_DEVICE != m_hDevice)
    {
        lgLcdClose(m_hDevice);
        m_hDevice = LGLCD_INVALID_DEVICE;
    }
    
    ZeroMemory(&desc, sizeof(desc));
    DWORD res = ERROR_SUCCESS;
    
    res = lgLcdEnumerate(m_hConnection, 0, &desc);
    if (ERROR_SUCCESS != res)
    {
        if(ERROR_NO_MORE_ITEMS != res)
        {
            // something happened. Let's close this.
            CloseAndDisconnect();
        }
        return;
    }

    lgLcdOpenContext open_ctx;
    ZeroMemory(&open_ctx, sizeof(open_ctx));

    open_ctx.connection = m_hConnection;
    open_ctx.index = 0;
    res = lgLcdOpen(&open_ctx);
    if (ERROR_SUCCESS != res)
        return;
    m_hDevice = open_ctx.device;
    m_dwButtonState = 0;
}


//************************************************************************
//
// CLCDOutput::HandleErrorFromAPI
//
//************************************************************************
void CLCDOutput::HandleErrorFromAPI(DWORD dwRes)
{
    switch(dwRes)
    {
        // all is well
    case ERROR_SUCCESS:
        break;
        // we lost our device
    case ERROR_DEVICE_NOT_CONNECTED:
        OnClosingDevice(m_hDevice);
        break;
    default:
        OnClosingDevice(m_hDevice);
        OnDisconnecting(m_hConnection);
        // something else happened, such as LCDMon that was terminated
        break;
    }
}

//************************************************************************
//
// CLCDOutput::SetScreenPriority
//
//************************************************************************
void CLCDOutput::SetScreenPriority(DWORD priority)
{
	m_nPriority = priority;
}


//************************************************************************
//
// CLCDOutput::CloseAndDisconnect
//
//************************************************************************

void CLCDOutput::CloseAndDisconnect()
{
    OnClosingDevice(m_hDevice);
    OnDisconnecting(m_hConnection);
}


//************************************************************************
//
// CLCDOutput::OnScreenExpired
//
//************************************************************************

void CLCDOutput::OnScreenExpired(CLCDManager* pScreen)
{
    UNREFERENCED_PARAMETER(pScreen);
    UnlockScreen();
}


//************************************************************************
//
// CLCDOutput::OnClosingDevice
//
//************************************************************************

void CLCDOutput::OnClosingDevice(int hDevice)
{
    UNREFERENCED_PARAMETER(hDevice);
    if (IsOpened())
    {
        lgLcdClose(m_hDevice);
        m_hDevice = LGLCD_INVALID_DEVICE;
    }
}

//************************************************************************
//
// CLCDOutput::OnDisconnecting
//
//************************************************************************

void CLCDOutput::OnDisconnecting(int hConnection)
{
    UNREFERENCED_PARAMETER(hConnection);
    // let's hope our device is already gone
    assert(!IsOpened());

    if (LGLCD_INVALID_CONNECTION != m_hConnection)
    {
        lgLcdDisconnect(m_hConnection);
        m_hConnection = LGLCD_INVALID_CONNECTION;
    }
}

//************************************************************************
//
// CLCDOutput::IsOpened
//
//************************************************************************

BOOL CLCDOutput::IsOpened()
{
	return (LGLCD_INVALID_DEVICE != m_hDevice);
}

//** end of LCDOutput.cpp ************************************************
