/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file c_ff_timers.h
/// @author Shawn Smith (L0ki)
/// @date May 13, 2005
/// @brief declaration of the client side timer class
/// 
/// declares the client side timer class
/// 
/// Revisions
/// ---------
/// May 13, 2005	L0ki: Initial Creation

#ifndef C_FF_TIMERS_H
#define C_FF_TIMERS_H

#include "ff_timers_shared.h"

#include "hudelement.h"
#include "hud_macros.h"
#include <vgui_controls/Panel.h>
#include <vgui_controls/ProgressBar.h>
#include <vgui/IBorder.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
using namespace vgui;
#include "iclientmode.h"

class C_FFTimer : public CHudElement, public vgui::Panel, public CFFTimerBase
{
	DECLARE_CLASS_SIMPLE( C_FFTimer, vgui::Panel );

public:
	C_FFTimer( std::string strName, const float flDuration );
	virtual ~C_FFTimer();

	void StartTimer( void );
	void ResetTimer( void );

protected:
	virtual void Paint( void );
	virtual void ApplySchemeSettings( IScheme *pScheme );

private:
	vgui::ProgressBar *m_pProgressBar;
};

typedef std::vector<C_FFTimer*> TimerList;
typedef TimerList::iterator TimerIterator;

class C_FFTimerManager
{
public:
	C_FFTimerManager();
	~C_FFTimerManager();

	C_FFTimer*	Create			( std::string strName, float flDuration );
	C_FFTimer*	FindTimer		( std::string strName );
	void		DeleteTimer		( std::string strName );
	void		DeleteTimer		( C_FFTimer *pTimer );
	void		DeleteAll		( void );
	void		SimulateTimers	( void );
	int			Count			( void );

protected:
	void		RemoveEntry		( TimerIterator ti );

private:
	TimerList		m_vecTimers;
};

extern C_FFTimerManager g_FFTimers;

#endif//C_FF_TIMERS_H