
#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H

enum eUpdateResponse {
	UPDATE_ERROR,
	UPDATE_FOUND,
	UPDATE_NOTFOUND,
	UPDATE_SERVER_OUTOFDATE
};

class CFFUpdateThread : public CThread
{
	//DECLARE_CLASS_SIMPLE( CFFUpdateThread, CThread );
	
	public:
		int Run();
		bool IsRunning() { return m_bIsRunning; };
		static CFFUpdateThread& GetInstance()
		{
			static CFFUpdateThread updatethread;
			return updatethread;
		};
		char m_szServerVersion[8];
		
	protected:
		CFFUpdateThread( void )
		{
			SetName("UpdateThread");
			m_szServerVersion[0] = 0;
			m_bIsRunning = false;
			//Start();
		};
		~CFFUpdateThread( void ) { };

	private:
		bool m_bIsRunning;
};

eUpdateResponse wyUpdateAvailable();
eUpdateResponse sockUpdateAvailable( const char *pszServerVersion );
void wyInstallUpdate();

#endif /* AUTOUPDATE_H */