
#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H

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
		
	protected:
		CFFUpdateThread( void )
		{
			SetName("UpdateThread");
			m_bIsRunning = false;
			Start();
		};
		~CFFUpdateThread( void ) { };

	private:
		bool m_bIsRunning;
};

bool wyUpdateAvailable();
void wyInstallUpdate();

#endif /* AUTOUPDATE_H */