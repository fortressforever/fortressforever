/********************************************************************
	created:	2006/07/07
	created:	7:7:2006   15:33
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff_vgui_timer.h
	file path:	f:\ff-svn\code\trunk\cl_dll
	file base:	ff_vgui_timer
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	A timer for the vgui
*********************************************************************/

#ifndef FF_VGUI_TIMER_H
#define FF_VGUI_TIMER_H

#ifdef _WIN32
#pragma once
#endif

namespace vgui
{
	class Timer : public Label
	{
	public:
		Timer(Panel *parent, const char *panelName);

		void	StartTimer(bool bStarted);

		void	SetTimerValue(float flTimerValue);
		void	SetTimerSpeed(float flTimerSpeed);
		void	SetTimerDrawClockStyle(bool bDrawClockStyle);

		void	UpdateTimer();

		virtual void Paint();

	private:
		float	m_flTimerValue;
		float	m_flLastTick;
		float	m_flTimerSpeed;

		bool	m_bStarted;
		bool	m_bDrawClockStyle;
	};
}

#endif