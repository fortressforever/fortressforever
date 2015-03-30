
// ff_scheduleman.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "ff_menuman.h"
#include "ff_entity_system.h"
#include "ff_scriptman.h"
#include "ff_luacontext.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern CFFPlayer *ToFFPlayer( CBaseEntity *pEntity );

/////////////////////////////////////////////////////////////////////////////
CFFLuaMenuManager _menuman;

extern CRC32_t ComputeChecksum(const char* szBuffer);
extern bool CRC32_LessFunc(const CRC32_t& a, const CRC32_t& b);

/////////////////////////////////////////////////////////////////////////////
// CFFScheduleCallback
/////////////////////////////////////////////////////////////////////////////
CFFLuaMenu::CFFLuaMenu(const char *szIdentifier )
{
	Q_strcpy(m_szIdentifier, szIdentifier);
	m_flDisplayTime = LUAMENU_DEFAULT_DISPLAYTIME;
	m_szMenuTitle[0] = 0;
	m_bMenuActive = false;
	m_iNumPlayersSent = 0;
	m_iNumPlayersActive = 0;
	ResetAllOptions();
}

/////////////////////////////////////////////////////////////////////////////
CFFLuaMenu::CFFLuaMenu( const char *szIdentifier, 
					   float flDisplayTime )
{
	Q_strcpy(m_szIdentifier, szIdentifier);
	m_flDisplayTime = flDisplayTime;
	m_szMenuTitle[0] = 0;
	m_bMenuActive = false;
	m_iNumPlayersSent = 0;
	m_iNumPlayersActive = 0;
	ResetAllOptions();
}

/////////////////////////////////////////////////////////////////////////////
CFFLuaMenu::CFFLuaMenu( const char *szIdentifier, 
					   const char *szMenuTitle )
{
	Q_strcpy(m_szIdentifier, szIdentifier);
	m_flDisplayTime = LUAMENU_DEFAULT_DISPLAYTIME;
	Q_strcpy(m_szMenuTitle, szMenuTitle);
	m_bMenuActive = false;
	m_iNumPlayersSent = 0;
	m_iNumPlayersActive = 0;
	ResetAllOptions();
}

/////////////////////////////////////////////////////////////////////////////
CFFLuaMenu::CFFLuaMenu( const char *szIdentifier, 
					   const char *szMenuTitle,
					   float flDisplayTime )
{
	Q_strcpy(m_szIdentifier, szIdentifier);
	m_flDisplayTime = flDisplayTime;
	Q_strcpy(m_szMenuTitle, szMenuTitle);
	m_bMenuActive = false;
	m_iNumPlayersSent = 0;
	m_iNumPlayersActive = 0;
	ResetAllOptions();
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenu::ResetAllOptions()
{
	for (int i=0; i<10; i++)
	{
		m_MenuOptions[i].szText[0] = 0;
		m_MenuOptions[i].iCount = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenu::ResetAllOptionCounts()
{
	for (int i=0; i<10; i++)
		m_MenuOptions[i].iCount = 0;
}

/////////////////////////////////////////////////////////////////////////////
bool CFFLuaMenu::Update()
{
	if (m_bMenuActive)
	{
		if(m_iNumPlayersActive <= 0)
		{
			CFFLuaSC hContext( 0 );
			hContext.Push( m_szIdentifier );
			hContext.Push( m_iNumPlayersSent );
			
			_scriptman.RunPredicates_LUA( NULL, &hContext, "menu_onexpire" );

			ResetAllOptionCounts();
			m_iNumPlayersSent = 0;
			m_bMenuActive = false;
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenu::Display(IRecipientFilter &filter)
{
	unsigned short optionBits = 0;

	char *szMenuString = new char[512];

	if (*m_szMenuTitle)
	{
		Q_snprintf(szMenuString, MAX_MENU_STRING, "%s\n", m_szMenuTitle);
	}

	for (int i=1; i<10; i++)
	{
		if (m_MenuOptions[i].szText[0])
		{
			optionBits |= 0x1 << (i-1);
			Q_snprintf(szMenuString, MAX_MENU_STRING, "%s->%d) %s", szMenuString, i, m_MenuOptions[i].szText);
		}
		Q_snprintf(szMenuString, MAX_MENU_STRING, "%s\n", szMenuString);
	}
	
	if (m_MenuOptions[0].szText[0])
	{
		optionBits |= 0x1 << 9;
		Q_snprintf(szMenuString, MAX_MENU_STRING, "%s->%d) %s\n", szMenuString, 0, m_MenuOptions[0].szText);
	}
	
	int len = strlen(szMenuString);
	
	char save = '\0';
	
	bool bMoreToCome = true;
	while (bMoreToCome)
	{
		if (len > 240)
		{
			save = szMenuString[240];
			szMenuString[240] = '\0';
		}
		else
		{
			bMoreToCome = false;
		}

		UserMessageBegin(filter, "ShowMenu");
			WRITE_WORD(optionBits);
			WRITE_CHAR(m_flDisplayTime);
			WRITE_BYTE(bMoreToCome ? 0xFF : 0x00);
			WRITE_STRING(szMenuString);
		MessageEnd();

		if (len > 240)
		{
			szMenuString[240] = save;
			szMenuString = &szMenuString[240];
			len -= 240;
		}
	}
	
	// set each players menu
	int c = filter.GetRecipientCount();

	m_bMenuActive = true;
	
	for ( int i = c - 1; i >= 0; i-- )
	{
		int index = filter.GetRecipientIndex( i );

		CBasePlayer *ent = UTIL_PlayerByIndex( index );
		if (ent && ent->IsPlayer())
		{
			CFFPlayer *pPlayer = ToFFPlayer( ent );

			if ( !pPlayer )
				continue;

			if ( Q_strcmp( pPlayer->GetCurrentLuaMenu(), m_szIdentifier ) == 0 )
				continue;

			pPlayer->SetCurrentLuaMenu( m_szIdentifier );
			m_iNumPlayersActive++;
			m_iNumPlayersSent++;
		}
	}

	//Msg("active: %d sent: %d\n", m_iNumPlayersActive, m_iNumPlayersSent);

	delete [] szMenuString;
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenu::AddMenuOption( int iSlot, const char *szOptionText )
{
	if (iSlot > 9 || iSlot < 0)
		return;

	Q_strcpy( m_MenuOptions[iSlot].szText, szOptionText );
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenu::RemoveMenuOption( int iSlot )
{
	if (iSlot > 9 || iSlot < 0)
		return;

	m_MenuOptions[iSlot].szText[0] = 0;
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenu::PlayerMenuExpired()
{
	m_iNumPlayersActive--;
	Update();
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenu::PlayerOptionSelected(int iSlot)
{
	if (iSlot > 9 || iSlot < 0)
		return;

	//m_MenuOptions[iSlot].iCount++;
	m_iNumPlayersActive--;
	Update();
}

/////////////////////////////////////////////////////////////////////////////
// CFFLuaMenuManager
/////////////////////////////////////////////////////////////////////////////
CFFLuaMenuManager::CFFLuaMenuManager()
{
	m_menus.SetLessFunc(CRC32_LessFunc);
}

/////////////////////////////////////////////////////////////////////////////
CFFLuaMenuManager::~CFFLuaMenuManager()
{

}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenuManager::AddLuaMenu(const char* szMenuName)
{
	CRC32_t id = ComputeChecksum(szMenuName);

	// check if the menu of the specified name already exists
	if(m_menus.IsValidIndex(m_menus.Find(id)))
		return;

	// add a new menu to the list
	CFFLuaMenu* pMenu = new CFFLuaMenu( szMenuName );

	m_menus.Insert(id, pMenu);
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenuManager::AddLuaMenu(const char* szMenuName,
									float flDisplayTime)
{
	CRC32_t id = ComputeChecksum(szMenuName);

	// check if the menu of the specified name already exists
	if(m_menus.IsValidIndex(m_menus.Find(id)))
		return;

	// add a new menu to the list
	CFFLuaMenu* pMenu = new CFFLuaMenu( szMenuName, flDisplayTime );

	m_menus.Insert(id, pMenu);
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenuManager::AddLuaMenu(const char* szMenuName,
									const char *szMenuTitle)
{
	CRC32_t id = ComputeChecksum(szMenuName);

	// check if the menu of the specified name already exists
	if(m_menus.IsValidIndex(m_menus.Find(id)))
		return;

	// add a new menu to the list
	CFFLuaMenu* pMenu = new CFFLuaMenu( szMenuName, szMenuTitle );

	m_menus.Insert(id, pMenu);
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenuManager::AddLuaMenu(const char* szMenuName,
									const char *szMenuTitle,
									float flDisplayTime)
{
	CRC32_t id = ComputeChecksum(szMenuName);

	// check if the menu of the specified name already exists
	if(m_menus.IsValidIndex(m_menus.Find(id)))
		return;

	// add a new menu to the list
	CFFLuaMenu* pMenu = new CFFLuaMenu( szMenuName, szMenuTitle, flDisplayTime );

	m_menus.Insert(id, pMenu);
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenuManager::Init()
{
	Shutdown();
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenuManager::RemoveLuaMenu(const char* szMenuName)
{
	CRC32_t id = ComputeChecksum(szMenuName);

	// remove the schedule from the list
	unsigned short it = m_menus.Find(id);
	if(m_menus.IsValidIndex(it))
	{
		delete m_menus[it];
		m_menus.RemoveAt(it);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenuManager::DisplayLuaMenu(IRecipientFilter &filter, const char *szMenuName)
{
	CRC32_t id = ComputeChecksum(szMenuName);

	unsigned short it = m_menus.Find(id);
	if(m_menus.IsValidIndex(it))
	{
		CFFLuaMenu* pMenu = m_menus.Element(it);
		pMenu->Display( filter );
	}
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenuManager::AddLuaMenuOption(const char *szMenuName, int iSlot, const char *szOptionText)
{
	CRC32_t id = ComputeChecksum(szMenuName);

	unsigned short it = m_menus.Find(id);
	if(m_menus.IsValidIndex(it))
	{
		CFFLuaMenu* pMenu = m_menus.Element(it);
		pMenu->AddMenuOption( iSlot, szOptionText );
	}
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenuManager::RemoveLuaMenuOption(const char *szMenuName, int iSlot)
{
	CRC32_t id = ComputeChecksum(szMenuName);

	unsigned short it = m_menus.Find(id);
	if(m_menus.IsValidIndex(it))
	{
		CFFLuaMenu* pMenu = m_menus.Element(it);
		pMenu->RemoveMenuOption( iSlot );
	}
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenuManager::SetLuaMenuTitle(const char *szMenuName, const char *szMenuTitle)
{
	CRC32_t id = ComputeChecksum(szMenuName);

	unsigned short it = m_menus.Find(id);
	if(m_menus.IsValidIndex(it))
	{
		CFFLuaMenu* pMenu = m_menus.Element(it);
		pMenu->SetMenuTitle( szMenuTitle );
	}
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenuManager::PlayerOptionSelected(const char *szMenuName, int iSlot)
{
	CRC32_t id = ComputeChecksum(szMenuName);

	unsigned short it = m_menus.Find(id);
	if(m_menus.IsValidIndex(it))
	{
		CFFLuaMenu* pMenu = m_menus.Element(it);
		pMenu->PlayerOptionSelected( iSlot );
	}
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenuManager::PlayerMenuExpired(const char *szMenuName)
{
	CRC32_t id = ComputeChecksum(szMenuName);

	unsigned short it = m_menus.Find(id);
	if(m_menus.IsValidIndex(it))
	{
		CFFLuaMenu* pMenu = m_menus.Element(it);
		pMenu->PlayerMenuExpired();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenuManager::Shutdown()
{
	unsigned short i = m_menus.FirstInorder();
	while ( i != m_menus.InvalidIndex() )
	{
		delete m_menus[i];
		i = m_menus.NextInorder( i );
	}

	m_menus.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
void CFFLuaMenuManager::Update()
{
	// update each item in the schedule list
	unsigned short it = m_menus.FirstInorder();
	while(m_menus.IsValidIndex(it))
	{
		CFFLuaMenu* pMenu = m_menus.Element(it);
		bool isExpired = pMenu->Update();

		if(isExpired)
		{
			// nothing
		}
		else
		{
			it = m_menus.NextInorder(it);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
