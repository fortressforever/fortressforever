/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file c_ff_timers.cpp
/// @author Christopher Boylan (Jiggles)
/// @date May 1, 2007
/// @brief implementation of the client side hint timer class
/// 
/// 
/// Revisions
/// ---------
/// May 1, 2007	  Jiggles: Initial Creation -- based on c_ff_timers, but without the vgui code

#ifndef C_FF_HINT_TIMERS_H
#define C_FF_HINT_TIMERS_H

#include "ff_timers_shared.h"

class C_FFHintTimer : public CFFTimerBase
{
	public:
		C_FFHintTimer( string strName, const float flDuration );
		virtual ~C_FFHintTimer();

		void StartTimer( void );
		void ResetTimer( void );

		// Jiggles: Added pausing functions for certain hints that are triggered after playing
		//			a character class for X minutes

		bool IsPaused( void ){ return m_bPaused; }
		void Pause( void );
		void Unpause( void );

	private:
		bool m_bPaused;
		float m_flPausedTime;
};

typedef std::vector<C_FFHintTimer*> HintTimerList;
typedef HintTimerList::iterator HintTimerIterator;

class C_FFHintTimerManager
{
	public:
		C_FFHintTimerManager();
		~C_FFHintTimerManager();

		C_FFHintTimer*	Create			( string strName, float flDuration );
		C_FFHintTimer*	FindTimer		( string strName );
		void		DeleteTimer		( string strName );
		void		DeleteTimer		( C_FFHintTimer *pTimer );
		void		DeleteAll		( void );
		void		SimulateTimers	( void );
		int			Count			( void );

	protected:
		void		RemoveEntry		( HintTimerIterator ti );

	private:
		HintTimerList		m_vecTimers;
};

extern C_FFHintTimerManager g_FFHintTimers;

#endif//C_FF_HINT_TIMERS_H