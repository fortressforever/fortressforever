
// ff_scheduleman.h

/////////////////////////////////////////////////////////////////////////////
// includes

#ifndef UTLMAP_H
	#include "utlmap.h"
#endif
#ifndef CHECKSUM_CRC_H
	#include "checksum_crc.h"
#endif
#ifndef LUABIND_OBJECT_050419_HPP
	#include "luabind/object.hpp"
#endif

#include "ff_player.h"

#define MAX_MENU_TITLE 128
#define MAX_MENU_OPTION 64
#define MAX_MENU_STRING	512 // from menu.cpp
#define LUAMENU_DEFAULT_DISPLAYTIME 15

/////////////////////////////////////////////////////////////////////////////
class CFFLuaMenu
{
public:
	// 'structors
	CFFLuaMenu( const char *szIdentifier );

	CFFLuaMenu( const char *szIdentifier, 
				float flDisplayTime );
	
	CFFLuaMenu( const char *szIdentifier, 
				const char *szMenuTitle );

	CFFLuaMenu( const char *szIdentifier, 
				const char *szMenuTitle,
				float flDisplayTime );

	CFFLuaMenu(const CFFLuaMenu& rhs);

	~CFFLuaMenu() {}

public:
	// updates. call only once per frame. returns true if the schedule is
	// complete and should be deleted; otherwise returns false
	bool Update();

	/////////////////////////////////////////////////////
	void Display(IRecipientFilter &filter);

	void AddMenuOption( int iSlot, const char *szOptionText );
	void RemoveMenuOption( int iSlot );
	
	const char *GetIdentifier() { return m_szIdentifier; }

	void SetMenuTitle( const char *szMenuTitle ) { Q_strcpy( m_szMenuTitle, szMenuTitle ); };

	void ResetAllOptions();
	void ResetAllOptionCounts();
	
	void PlayerMenuExpired();
	void PlayerOptionSelected( int iSlot );

private:
	
	struct LuaMenuOption
	{
		char					szText[MAX_MENU_OPTION];
		int						iCount;			// number of times its been selected
		luabind::adl::object	luaFunction;	// handle to the lua function to call
	};

	// private data
	float	m_flDisplayTime;				// display time
	char	m_szMenuTitle[MAX_MENU_TITLE];	// menu title
	LuaMenuOption m_MenuOptions[10];		// params to pass to function
	char	m_szIdentifier[128];			// name given by lua
	int		m_iNumPlayersSent;				// number of players that have been sent the menu
	int		m_iNumPlayersActive;			// number of players have still have the menu
	bool	m_bMenuActive;					// whether or not the menu's been sent
};

/////////////////////////////////////////////////////////////////////////////
class CFFLuaMenuManager
{
public:
	// 'structors
	CFFLuaMenuManager();
	~CFFLuaMenuManager();

public:
	void Init();
	void Shutdown();
	void Update();

public:
	// adds a menu
	void AddLuaMenu(const char* szMenuName);

	void AddLuaMenu(const char* szMenuName,
						float flDisplayTime);
	
	void AddLuaMenu(const char* szMenuName,
						const char *szMenuTitle);
	
	void AddLuaMenu(const char* szMenuName,
						const char *szMenuTitle,
						float flDisplayTime);

	// removes a menu
	void RemoveLuaMenu(const char* szMenuName);

	void DisplayLuaMenu(IRecipientFilter &filter, const char *szMenuName);

	void AddLuaMenuOption( const char* szMenuName, int iSlot, const char *szOptionText );
	void RemoveLuaMenuOption( const char* szMenuName, int iSlot );
	
	void SetLuaMenuTitle( const char* szMenuName, const char *szMenuTitle );

	void PlayerMenuExpired(const char *szMenuName);
	void PlayerOptionSelected(const char *szMenuName, int iSlot);

private:
	// list of menus. key is the checksum of an identifying name
	CUtlMap<CRC32_t, CFFLuaMenu*>	m_menus;
};

/////////////////////////////////////////////////////////////////////////////
extern CFFLuaMenuManager _menuman;

/////////////////////////////////////////////////////////////////////////////