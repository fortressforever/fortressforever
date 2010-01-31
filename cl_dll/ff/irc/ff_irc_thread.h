//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_irc_thread.h
//	@author Ryan Liptak (squeek)
//	@date 30/01/2010
//	@brief Thread for the IRC server responses
//
//	REVISIONS
//	---------
//	30/01/2010, squeek: 
//		First created

#ifndef FF_IRC_THREAD_H
#define FF_IRC_THREAD_H

//-----------------------------------------------------------------------------
// Thread Test
//-----------------------------------------------------------------------------

class CFFIRCThread : public CThread
{
	DECLARE_CLASS_SIMPLE( CFFIRCThread, CThread );
	
	public:
		int Run();
		bool IsRunning() { return m_bIsRunning; };
		static CFFIRCThread& GetInstance(); 
		
	protected:
		CFFIRCThread( void );
		~CFFIRCThread( void );

	private:
		bool m_bIsRunning;
};

#endif /* FF_IRC_THREAD_H */
