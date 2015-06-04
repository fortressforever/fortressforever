#ifndef FF_STEAMWORKS_THREAD_H
#define FF_STEAMWORKS_THREAD_H

#include "ff_steamworks_msg.h"

class CFFSteamworksThread : public CThread
{
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
		void KillServerProcess( void );
		bool CreateServerProcess( void );
		void SendMsg( const CFFSteamworksMessage &msg );

		// bit of a hack, these are HANDLEs which are just void* so do this to avoid the windows.h crap here
		void* m_hProcess;
		void* m_hThread;
		
		int m_iPollRate; //ms
		int m_iHeartbeatRate; // per poll
		int m_iHeartbeatCheckCount;

		bool m_bIsShutdown;
		CUtlVector<CFFSteamworksMessage> m_QueuedMessages;
		Socks m_Sock;
		bool m_bIsRunning;
};

#endif /* FF_STEAMWORKS_THREAD_H */
