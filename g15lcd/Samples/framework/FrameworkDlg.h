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

#ifndef _FRAMEWORKDLG_H_INCLUDED_
#define _FRAMEWORKDLG_H_INCLUDED_


#include "LCDOutput.h"
#include "LCDManager.h"
#include "LCDSampleScreen.h"

class CFrameworkDlg : public CDialog
{
// Construction
public:
	CFrameworkDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CFrameworkDlg)
	enum { IDD = IDD_FRAMEWORK_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFrameworkDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CFrameworkDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnChangeText();
	afx_msg void OnFont();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    // Update Timer
    DWORD m_dwTimerId;
        
    // LCD Output Object
    CLCDOutput m_LCDOutput;

    // LCD Screen
    CLCDSampleScreen m_LCDSampleScreen;

    // Configuration
    TCHAR m_szAppletName[MAX_PATH];
    lgLcdConnectContext  m_LcdContext;

    // Callback used to allow client to pop up a "configuration panel"
    static DWORD CALLBACK LcdOnConfigureCB(int connection, const PVOID pContext);
};

//{{AFX_INSERT_LOCATION}}
#endif // !_FRAMEWORKDLG_H_INCLUDED_
