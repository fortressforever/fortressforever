#include "G15.h"

#include <list>
#include <windows.h>
#include "../g15_resources/resource.h"
#include "LCDOutput.h"
#include "LCDManager.h"
#include "LCDText.h"
#include "LCDIcon.h"
#include "LCDBitmap.h"
#include "LCDScrollingText.h"

#include "sharedInterface.h"
#include "filesystem.h"

// fuck you valve
#undef GetTickCount

#define DISABLE_G15 1

namespace G15
{
	//////////////////////////////////////////////////////////////////////////
	HMODULE g_ResourceDll = NULL;
	//////////////////////////////////////////////////////////////////////////

	struct FortressPts
	{
		char	m_Message[1024];
	};
	typedef std::list<FortressPts> FortressPtQueue;
	FortressPtQueue		g_PointsQueue;

	//////////////////////////////////////////////////////////////////////////
	class FFTitleScreen : public CLCDManager  
	{
	public:
		FFTitleScreen() {}
		virtual ~FFTitleScreen() {}

		virtual HRESULT Initialize()
		{
			m_BackGround.Initialize();
			m_BackGround.SetSize(160, 43);
			m_BackGround.SetOrigin(0,0);
			m_BackGround.Show(TRUE);
			m_BackGround.SetBitmap(IDB_FF_SPLASH, 160, 43);

			AddObject(&m_BackGround);

			return CLCDManager::Initialize();
		}
		/*virtual void OnLCDButtonDown(int nButton)
		{
		}
		virtual void OnLCDButtonUp(int nButton)
		{
		}*/
	protected:
		CLCDBitmap m_BackGround;
	};
	//////////////////////////////////////////////////////////////////////////
	class FFMainStatusScreen : public CLCDManager  
	{
	public:
		FFMainStatusScreen() {}
		virtual ~FFMainStatusScreen() {}

		virtual HRESULT Initialize()
		{
			m_FortressScroller.Initialize();
			m_FortressScroller.SetOrigin(0, 0);
			m_FortressScroller.SetSize(160, 43);
			m_FortressScroller.SetWordWrap(FALSE);
			m_FortressScroller.SetAlignment(DT_TOP|DT_LEFT);
			m_FortressScroller.SetWordWrap(TRUE);
			m_FortressScroller.SetText(_T("Sample Scrolling Text"));
			m_FortressScroller.EnableRepeat(FALSE);

			m_Health.Initialize();
			m_Health.SetOrigin(0, 25);
			m_Health.SetSize(160, 43);
			m_Health.SetAlignment(DT_BOTTOM|DT_LEFT);
			m_Health.SetWordWrap(TRUE);
			m_Health.SetText(_T("H: 100"));

			m_Armor.Initialize();
			m_Armor.SetOrigin(0, 25);
			m_Armor.SetSize(160, 43);
			m_Armor.SetAlignment(DT_BOTTOM|DT_RIGHT);
			m_Armor.SetWordWrap(TRUE);
			m_Armor.SetText(_T("A: 100"));

			AddObject(&m_FortressScroller);
			AddObject(&m_Health);
			AddObject(&m_Armor);

			return CLCDManager::Initialize();
		}

		void Update(DWORD dwTimestamp)
		{
			CLCDManager::Update(dwTimestamp);

			if(m_FortressScroller.IsScrollingDone())
			{
				if(!g_PointsQueue.empty())
				{
					const FortressPts &pts = g_PointsQueue.front();
					m_FortressScroller.SetText(pts.m_Message);
					g_PointsQueue.pop_front();
				}
			}
		}
		/*virtual void OnLCDButtonDown(int nButton)
		{
		}
		virtual void OnLCDButtonUp(int nButton)
		{
		}*/
	protected:
		CLCDText			m_Health;
		CLCDText			m_Armor;
		CLCDScrollingText	m_FortressScroller;
	};
	//////////////////////////////////////////////////////////////////////////

	LPCSTR  g_AppName = "Fortress Forever";

	bool	g_G15Initialized = false;

	lgLcdConnectContext g_LcdContext;
	CLCDOutput			g_LCDOutput;

	//////////////////////////////////////////////////////////////////////////
	// Screens
	FFTitleScreen		g_TitleScreen;
	FFMainStatusScreen	g_MainScreen;

	//////////////////////////////////////////////////////////////////////////
	bool IsEnabled() { return g_G15Initialized; }
  
	void Initialize()
	{
		if(g_G15Initialized)
			return;

#if(DISABLE_G15)
		return;
#endif

		// set up the LCD context as non-autostart, non-persist, callbacked
		memset(&g_LcdContext, 0, sizeof(g_LcdContext));
		g_LcdContext.appFriendlyName = g_AppName;
		g_LcdContext.isAutostartable = FALSE;
		g_LcdContext.isPersistent = FALSE;
		//g_LcdContext.onConfigure.configCallback = LcdOnConfigureCB;
		//g_LcdContext.onConfigure.configContext = this;
		g_LcdContext.onConfigure.configCallback = 0;
		g_LcdContext.onConfigure.configContext = 0;

		// Initialize the output object
		if(ERROR_SUCCESS != g_LCDOutput.Initialize(&g_LcdContext))
			return;		

		// Load the resrouce dll.
		const int BUF_SIZE = 1024;
		char resDllFile[BUF_SIZE] = {0};
		filesystem->GetLocalPath("bin/g15_resources.dll", resDllFile, BUF_SIZE);
		
		if(resDllFile)
			g_ResourceDll = LoadLibrary(resDllFile);

		if(!g_ResourceDll)
		{
			Warning("Could not find Resource Dll!");
			return;
		}

		// Initialize the screen
		g_TitleScreen.Initialize();
		g_TitleScreen.SetExpiration(5000);

		g_MainScreen.Initialize();
		g_MainScreen.SetExpiration(INFINITE);

		// Add and lock the screen onto our output manager
		g_LCDOutput.AddScreen(&g_TitleScreen);
		g_LCDOutput.AddScreen(&g_MainScreen);

		g_LCDOutput.LockScreen(&g_TitleScreen);

		g_G15Initialized = true;
	}

	void Shutdown()
	{
		if(!IsEnabled())
			return;

		g_LCDOutput.Shutdown();	

		if(g_ResourceDll)
		{
			FreeLibrary(g_ResourceDll);
			g_ResourceDll = NULL;
		}
	}

	void Update()
	{
		if(!IsEnabled())
			return;

		g_LCDOutput.Update(GetTickCount());
		g_LCDOutput.Draw();
	}

	//////////////////////////////////////////////////////////////////////////
	void UpdateHealth(int _current, int _max)
	{

	}

	void UpdateArmor(int _current, int _max)
	{

	}

	void AddFortPoints(const char *_msg, const char *_score)
	{
		if(!IsEnabled())
			return;

		FortressPts fpts;
		Q_snprintf(fpts.m_Message, sizeof(fpts.m_Message), "%s %s", _msg, _score);
        g_PointsQueue.push_back(fpts);
	}
};

//DWORD LcdOnConfigureCB(int connection, const PVOID pContext)
//{
//
//	// NOTE: This callback may occur in the LCD Manager's thread context
//
//	// try and bring ourselves to the foreground
//	CFrameworkDlg* pThis = (CFrameworkDlg*)pContext;
//	pThis->SetWindowPos(&wndTopMost, -1, -1, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
//	pThis->SetWindowPos(&wndNoTopMost, -1, -1, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
//	pThis->ShowWindow(SW_SHOWNORMAL);
//	return ERROR_SUCCESS;
//}
