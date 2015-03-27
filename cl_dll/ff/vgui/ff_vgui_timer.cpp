/********************************************************************
	created:	2006/07/07
	created:	7:7:2006   15:36
	filename: 	f:\ff-svn\code\trunk\cl_dll\vgui_control_timer.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll
	file base:	vgui_control_timer
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	A timer for the vgui
*********************************************************************/

#include "cbase.h"
#include <vgui_controls/Label.h>

#include "ff_vgui_timer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
Timer::Timer(Panel *parent, const char *panelName) : Label(parent, panelName, "--:--")
{
	m_flLastTick = 0.0f;
	m_flTimerSpeed = 1.0f;

	m_bStarted = false;
}

//-----------------------------------------------------------------------------
// Purpose: Start or pause the timer
//-----------------------------------------------------------------------------
void Timer::StartTimer(bool bStarted)
{
	m_bStarted = bStarted;

	m_flLastTick = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: Set the timer's value
//-----------------------------------------------------------------------------
void Timer::SetTimerValue(float flTimerValue)
{
	m_flTimerValue = flTimerValue;

	m_flLastTick = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 1.0 is a normal speed timer going forward at 1 tick per second
//-----------------------------------------------------------------------------
void Timer::SetTimerSpeed(float flTimerSpeed)
{
	m_flTimerSpeed = flTimerSpeed;
}

//-----------------------------------------------------------------------------
// Purpose: Timer can be drawn as N or MM:SS
//-----------------------------------------------------------------------------
void Timer::SetTimerDrawClockStyle(bool bDrawClockStyle)
{
	m_bDrawClockStyle = bDrawClockStyle;
}

//-----------------------------------------------------------------------------
// Purpose: Update all the counts and textual representation
//-----------------------------------------------------------------------------
void Timer::UpdateTimer()
{
	// Update the timer value while it is running
	if (m_bStarted)
	{
		float flDeltaTime = gpGlobals->curtime - m_flLastTick;
		m_flTimerValue += flDeltaTime * m_flTimerSpeed;
	}

	m_flLastTick = gpGlobals->curtime;
	
	// We're going to clamp to positive values only for now
	if (m_flTimerValue < 0)
		m_flTimerValue = 0;

	char szText[128];

	int iTimeForDisplay = m_flTimerSpeed > 0 ? floor( m_flTimerValue ) : ceil( m_flTimerValue ); 

	// Timers may be draw in clock style (MM:SS) or just as a counter (n)
	if (m_bDrawClockStyle)
		Q_snprintf(szText, 128, "%d:%02d", iTimeForDisplay / 60, iTimeForDisplay % 60);

	else
		Q_snprintf(szText, 128, "%d", iTimeForDisplay);

	SetText(szText);
}

//-----------------------------------------------------------------------------
// Purpose: Catch the paint so we can update the timer
//-----------------------------------------------------------------------------
void Timer::Paint()
{
	UpdateTimer();

	Label::Paint();
}