//************************************************************************
//
// LCDBase.cpp
//
// The CLCDBase class is the generic base class for all lcd ui objects
// 
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#include <windows.h>
#include "LCDBase.h"


//************************************************************************
//
// CLCDBase::CLCDBase
//
//************************************************************************

CLCDBase::CLCDBase(void)
{
    m_Size.cx = 0;
    m_Size.cy = 0;
    m_Origin.x = 0;
    m_Origin.y = 0;
    m_bVisible = TRUE;
    m_bInverted = FALSE;
    ZeroMemory(&m_ptLogical, sizeof(m_ptLogical));
    ZeroMemory(&m_sizeLogical, sizeof(m_sizeLogical));
    m_nBkMode = TRANSPARENT;
    m_objectType = LG_UNKNOWN;
}


//************************************************************************
//
// CLCDBase::~CLCDBase
//
//************************************************************************

CLCDBase::~CLCDBase(void)
{
}


//************************************************************************
//
// CLCDBase::Initialize
//
//************************************************************************

HRESULT CLCDBase::Initialize(void)
{
    return S_OK;
}


//************************************************************************
//
// CLCDBase::Shutdown
//
//************************************************************************

void CLCDBase::Shutdown(void)
{
}


//************************************************************************
//
// CLCDBase::SetOrigin
//
//************************************************************************

void CLCDBase::SetOrigin(POINT pt)
{
    m_Origin = pt;
}


//************************************************************************
//
// CLCDBase::SetOrigin
//
//************************************************************************

void CLCDBase::SetOrigin(int nX, int nY)
{
    POINT pt = { nX, nY };
    SetOrigin(pt);
}


//************************************************************************
//
// CLCDBase::GetOrigin
//
//************************************************************************

POINT& CLCDBase::GetOrigin(void)
{
    return m_Origin;
}


//************************************************************************
//
// CLCDBase::SetSize
//
//************************************************************************

void CLCDBase::SetSize(SIZE& size)
{
    m_Size = size;
    SetLogicalSize(m_Size);
}


//************************************************************************
//
// CLCDBase::SetSize
//
//************************************************************************

void CLCDBase::SetSize(int nCX, int nCY)
{
    SIZE size = { nCX, nCY };
    SetSize(size);
}


//************************************************************************
//
// CLCDBase::GetSize
//
//************************************************************************

SIZE& CLCDBase::GetSize(void)
{
    return m_Size;
}


//************************************************************************
//
// CLCDBase::SetLogicalOrigin
//
//************************************************************************

void CLCDBase::SetLogicalOrigin(POINT& pt)
{
    m_ptLogical = pt;
}


//************************************************************************
//
// CLCDBase::SetLogicalOrigin
//
//************************************************************************

void CLCDBase::SetLogicalOrigin(int nX, int nY)
{
    m_ptLogical.x = nX;
    m_ptLogical.y = nY;
}


//************************************************************************
//
// CLCDBase::GetLogicalOrigin
//
//************************************************************************

POINT& CLCDBase::GetLogicalOrigin(void)
{
    return m_ptLogical;
}


//************************************************************************
//
// CLCDBase::SetLogicalSize
//
//************************************************************************

void CLCDBase::SetLogicalSize(SIZE& size)
{
    m_sizeLogical = size;
}


//************************************************************************
//
// CLCDBase::SetLogicalSize
//
//************************************************************************

void CLCDBase::SetLogicalSize(int nCX, int nCY)
{
    m_sizeLogical.cx = nCX;
    m_sizeLogical.cy = nCY;
}


//************************************************************************
//
// CLCDBase::GetLogicalSize
//
//************************************************************************

SIZE& CLCDBase::GetLogicalSize(void)
{
    return m_sizeLogical;
}


//************************************************************************
//
// CLCDBase::Show
//
//************************************************************************

void CLCDBase::Show(BOOL bShow)
{
    m_bVisible = bShow;
}


//************************************************************************
//
// BOOL CLCDBase::
//
//************************************************************************

BOOL CLCDBase::IsVisible(void)
{
    return m_bVisible;
}


//************************************************************************
//
// CLCDBase::Invert
//
//************************************************************************

void CLCDBase::Invert(BOOL bEnable)
{
    m_bInverted = bEnable;
}


//************************************************************************
//
// CLCDBase::ResetUpdate
//
//************************************************************************

void CLCDBase::ResetUpdate(void)
{
    // do nothing
}


//************************************************************************
//
// CLCDBase::OnUpdate
//
//************************************************************************

void CLCDBase::OnUpdate(DWORD dwTimestamp)
{
    UNREFERENCED_PARAMETER(dwTimestamp);
}


//************************************************************************
//
// CLCDBase::SetBackgroundMode
//
//************************************************************************

void CLCDBase::SetBackgroundMode(int nMode)
{
    m_nBkMode = nMode;
}


//************************************************************************
//
// CLCDBase::GetBackgroundMode
//
//************************************************************************

int CLCDBase::GetBackgroundMode()
{
    return m_nBkMode;
}

//************************************************************************
//
// CLCDBase::GetObjectType
//
//************************************************************************

const LGObjectType CLCDBase::GetObjectType()
{
    return m_objectType;
}

//************************************************************************
//
// CLCDBase::SetObjectType
//
//************************************************************************

void CLCDBase::SetObjectType(const LGObjectType type)
{
    m_objectType = type;
}

//** end of LCDBase.cpp **************************************************
