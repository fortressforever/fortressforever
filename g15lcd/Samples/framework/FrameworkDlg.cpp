//************************************************************************
//
// FrameworkDlg.h
//
// Here we create initialize our Output and Screen classes and use a timer
//    to update the Output class
// Any dialog specific events are handled here. 
// 
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#include "stdafx.h"
#include "framework.h"
#include "frameworkDlg.h"

#ifndef STR_MY_PLUGIN_NAME
    #error "You must define STR_MY_PLUGIN_NAME to be your plugin name string
#endif

//************************************************************************
//
// CFrameworkDlg::CFrameworkDlg
//
//************************************************************************

CFrameworkDlg::CFrameworkDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFrameworkDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFrameworkDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


//************************************************************************
//
// CFrameworkDlg::DoDataExchange
//
//************************************************************************

void CFrameworkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFrameworkDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFrameworkDlg, CDialog)
	//{{AFX_MSG_MAP(CFrameworkDlg)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_TEXT, OnChangeText)
	ON_BN_CLICKED(IDC_FONT, OnFont)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//************************************************************************
//
// CFrameworkDlg::OnInitDialog
//
//************************************************************************

BOOL CFrameworkDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

    // get our applet name
    _tcsncpy(m_szAppletName, STR_MY_PLUGIN_NAME, MAX_PATH);

    // set up the LCD context as non-autostart, non-persist, callbacked
    memset(&m_LcdContext, 0, sizeof(m_LcdContext));
    m_LcdContext.appFriendlyName = m_szAppletName;
    m_LcdContext.isAutostartable = FALSE;
    m_LcdContext.isPersistent = FALSE;
    m_LcdContext.onConfigure.configCallback = LcdOnConfigureCB;
    m_LcdContext.onConfigure.configContext = this;

    // Initialize the output object
    m_LCDOutput.Initialize(&m_LcdContext);

    // Initialize the screen
    m_LCDSampleScreen.Initialize();

    // Set the expiration on the sample screen
    // For our purposes, we will set it to INFINITE
    m_LCDSampleScreen.SetExpiration(INFINITE);

    // Add and lock the screen onto our output manager
    m_LCDOutput.AddScreen(&m_LCDSampleScreen);
    m_LCDOutput.LockScreen(&m_LCDSampleScreen);

    // set the edit box text
    GetDlgItem(IDC_TEXT)->SetWindowText(_T("Hello World"));

    // Set a lcd update timer every 50ms
    // Increase this to update faster
    m_dwTimerId = SetTimer(0xABBAABBA, 50, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}


//************************************************************************
//
// CFrameworkDlg::OnTimer
//
// Invoke the LCDOutput's Update and Draw functions here.
//************************************************************************

void CFrameworkDlg::OnTimer(UINT nIDEvent) 
{

    // This invokes OnUpdate for the active screen
    m_LCDOutput.Update(GetTickCount());

    // This invokes OnDraw for the active screen
    m_LCDOutput.Draw();
	
	CDialog::OnTimer(nIDEvent);
}


//************************************************************************
//
// CFrameworkDlg::OnDestroy
//
// Shutdown the LCDOutput
//************************************************************************

void CFrameworkDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	m_LCDOutput.Shutdown();	
}


//************************************************************************
//
// CFrameworkDlg::LcdOnConfigureCB
//
// Callback function called in the LCD Control Panel
// This is where you bring up Applet specific configuration options
//************************************************************************

DWORD CFrameworkDlg::LcdOnConfigureCB(int connection, const PVOID pContext)
{

    // NOTE: This callback may occur in the LCD Manager's thread context

    // try and bring ourselves to the foreground
    CFrameworkDlg* pThis = (CFrameworkDlg*)pContext;
    pThis->SetWindowPos(&wndTopMost, -1, -1, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
    pThis->SetWindowPos(&wndNoTopMost, -1, -1, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
    pThis->ShowWindow(SW_SHOWNORMAL);
    return ERROR_SUCCESS;
}


//************************************************************************
//
// CFrameworkDlg::OnChangeText
//
// Changes the text on the LCD Text Control
//
//************************************************************************

void CFrameworkDlg::OnChangeText() 
{
	CString strText;
    GetDlgItem(IDC_TEXT)->GetWindowText(strText);
    m_LCDSampleScreen.m_Text.SetText(strText);
}


//************************************************************************
//
// CFrameworkDlg::OnFont
//
// Changes the font on the LCD Text Control
//************************************************************************

void CFrameworkDlg::OnFont() 
{
    LOGFONT lf;
    HFONT hFont = m_LCDSampleScreen.m_Text.GetFont();

    GetObject(hFont, sizeof(LOGFONT), &lf);

	CFontDialog cfd(&lf);
    if (cfd.DoModal() == IDOK)
    {
        cfd.GetCurrentFont(&lf);
        m_LCDSampleScreen.m_Text.SetFont(lf);
    }	
}
