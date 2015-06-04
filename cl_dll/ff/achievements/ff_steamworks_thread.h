#ifndef FF_STEAMWORKS_THREAD_H
#define FF_STEAMWORKS_THREAD_H

#include "ff_steamworks_msg.h"

class CFFSteamworksThread : public CThread
{
	//DECLARE_CLASS_SIMPLE( CFFSteamworksThread, CThread );
	
	public:
		int Run();
		bool IsRunning( void ) { return m_bIsRunning; };
		static CFFSteamworksThread& GetInstance(); 
		void QueueMessage( const CFFSteamworksMessage & );
		void ShutdownServer( void );
	protected:
		CFFSteamworksThread( void );
		~CFFSteamworksThread( void );

	private:
		bool m_bIsShutdown;
		CUtlVector<CFFSteamworksMessage> m_QueuedMessages;
		bool CreateServerProcess( void );
		Socks m_Sock;
		bool m_bIsRunning;
};

#endif /* FF_STEAMWORKS_THREAD_H */
