//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "hud.h"
#include "clientmode_ff.h"
#include "cdll_client_int.h"
#include "iinput.h"
#include "vgui/isurface.h"
#include "vgui/ipanel.h"
#include <vgui_controls/AnimationController.h>
#include "ivmodemanager.h"
//#include "BuyMenu.h"
#include "filesystem.h"
#include "vgui/ivgui.h"
#include "keydefs.h"
#include "hud_chat.h"
#include "view_shared.h"
#include "view.h"
#include "ivrenderview.h"
#include "model_types.h"
#include "iefx.h"
#include "dlight.h"
#include <imapoverview.h>
#include "c_playerresource.h"
#include <keyvalues.h>
#include "text_message.h"
#include "panelmetaclassmgr.h"
#include "c_soundscape.h"
#include "engine/IEngineSound.h"
#include "c_te_legacytempents.h"
#include "physpropclientside.h"
#include "ff_gamerules.h"
#include "usermessages.h"
#include "filesystem.h"
#include "KeyValues.h"

// Bug #0000310: fov doesn't reset |-- Mulch
//ConVar default_fov( "default_fov", "90", FCVAR_NONE );
ConVar default_fov( "default_fov", "90", FCVAR_ARCHIVE, "Default FOV value", true, 80.0, true, 120.0 );
extern ConVar v_viewmodel_fov;

IClientMode *g_pClientMode = NULL;

// --------------------------------------------------------------------------------- //
// CFFModeManager.
// --------------------------------------------------------------------------------- //

class CFFModeManager : public IVModeManager
{
public:
	virtual void	Init();
	virtual void	SwitchMode( bool commander, bool force ) {}
	virtual void	LevelInit( const char *newmap );
	virtual void	LevelShutdown( void );
	virtual void	ActivateMouse( bool isactive ) {}
};

static CFFModeManager g_ModeManager;
IVModeManager *modemanager = ( IVModeManager * )&g_ModeManager;

// --------------------------------------------------------------------------------- //
// CFFModeManager implementation.
// --------------------------------------------------------------------------------- //

#define SCREEN_FILE		"scripts/vgui_screens.txt"

void CFFModeManager::Init()
{
	g_pClientMode = GetClientModeNormal();
	
	PanelMetaClassMgr()->LoadMetaClassDefinitionFile( SCREEN_FILE );
}

void CFFModeManager::LevelInit( const char *newmap )
{
	g_pClientMode->LevelInit( newmap );

	// Reset client timer to zero (otherwise it'll
	// be where we left off with the last round)
	if( FFGameRules() )
		FFGameRules()->SetRoundStart( 0.0f );
}

void CFFModeManager::LevelShutdown( void )
{
	g_pClientMode->LevelShutdown();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ClientModeFFNormal::ClientModeFFNormal()
{
	Q_strcpy(m_szValidationFile, "validationkeys.txt");
}

//-----------------------------------------------------------------------------
// Purpose: If you don't know what a destructor is by now, you are probably going to get fired
//-----------------------------------------------------------------------------
ClientModeFFNormal::~ClientModeFFNormal()
{
}


void ClientModeFFNormal::InitViewport()
{
	m_pViewport = new FFViewport();
	m_pViewport->Start( gameuifuncs, gameeventmanager );
}

ClientModeFFNormal g_ClientModeNormal;

IClientMode *GetClientModeNormal()
{
	return &g_ClientModeNormal;
}


ClientModeFFNormal* GetClientModeFFNormal()
{
	Assert( dynamic_cast< ClientModeFFNormal* >( GetClientModeNormal() ) );

	return static_cast< ClientModeFFNormal* >( GetClientModeNormal() );
}

float ClientModeFFNormal::GetViewModelFOV( void )
{
	return v_viewmodel_fov.GetFloat();
}

int ClientModeFFNormal::GetDeathMessageStartHeight( void )
{
	return m_pViewport->GetDeathMessageStartHeight();
}

void ClientModeFFNormal::PostRenderVGui()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeFFNormal::Init( void )
{
	m_servercriptCRC = 0;
	m_serverScriptValid = false;

	usermessages->HookMessage("FFScriptCRC", &ClientModeFFNormal::FFScriptCRC_MsgHandler);

	gameeventmanager->AddListener( this, "ff_restartround", false );

	ClientModeShared::Init();
}

//-----------------------------------------------------------------------------
// Purpose: Capture FF Specific events
//-----------------------------------------------------------------------------
void ClientModeFFNormal::FireGameEvent( IGameEvent *pEvent )
{
	const char *pszEventName = pEvent->GetName();

	if( Q_strcmp( "ff_restartround", pszEventName ) == 0 )
	{
		// Set up current round offset for client
		float flCurtime = pEvent->GetFloat( "curtime" );

		// Recreate all client side physics props
		C_PhysPropClientside::RecreateAll();

		// Just tell engine to clear decals
		engine->ClientCmd( "r_cleardecals" );

		tempents->Clear();

		// Stop any looping sounds
		enginesound->StopAllSounds( true );

		Soundscape_OnStopAllSounds(); // Tell the soundscape system
	}

	BaseClass::FireGameEvent( pEvent );
}

//-----------------------------------------------------------------------------
void ClientModeFFNormal::SetNextValidationFilePath(const char* szFilePath)
{
	Q_strncpy(m_szValidationFile, szFilePath, sizeof(m_szValidationFile));
}

//-----------------------------------------------------------------------------
void ClientModeFFNormal::FFScriptCRC_MsgHandler(bf_read& msg)
{
	ClientModeFFNormal* pClientMode = GetClientModeFFNormal();

	unsigned long crc = (unsigned long)msg.ReadLong();
	pClientMode->m_servercriptCRC = crc;

	// lookup the valid crc and check against it
	char szLevelName[256];
	Q_strcpy(szLevelName, engine->GetLevelName() + 5); // Skip the "maps/" part
	szLevelName[(int)strlen(szLevelName) - 4] = '\0'; // Skip the ".bsp" part

	char szDescription[256];
	bool bValidated = ValidateLevel(pClientMode->m_szValidationFile,
									szLevelName,
									crc,
									szDescription,
									256);

	Msg("\n");
	Msg("Server Script Validation\n");
	Msg("Level Name:   %s\n", szLevelName);
	Msg("Checksum:     0x%x\n", crc);

	if(bValidated)
	{
		Msg("Validated:    Yes\n");
		Msg("Description:  %s\n", szDescription);
	}
	else
	{
		Msg("Validated:    No\n");
	}

	Msg("\n");

	// reset the validation filepath
	Q_strcpy(pClientMode->m_szValidationFile, "validationkeys.txt");
}

//-----------------------------------------------------------------------------
bool ClientModeFFNormal::ValidateLevel(const char* szValidateFilePath,
									   const char* szLevelName,
									   CRC32_t checksum,
									   char* szDescription,
									   int descMaxLength)
{
	// load validation file
	KeyValues* pKvRoot = new KeyValues("Levels");
	bool bRes = pKvRoot->LoadFromFile(::filesystem, szValidateFilePath);

	if(!bRes)
	{
		Warning("Error loading validation keys file '%s'\n", szValidateFilePath);
		pKvRoot->deleteThis();
		return false;
	}

	bool bValidate = false;

	// find section with level name
	KeyValues* pKvLevel = pKvRoot->FindKey(szLevelName);
	if(pKvLevel)
	{
		// find section with the checksum
		char szChecksum[32];
		Q_snprintf(szChecksum, sizeof(szChecksum), "0x%x", checksum);

		KeyValues* pKvEntry = pKvLevel->FindKey(szChecksum);
		if(pKvEntry)
		{
			// copy the description
			const char* szDesc = pKvEntry->GetString("description", "");
			Q_strncpy(szDescription, szDesc, descMaxLength);
			bValidate = true;
		}
	}

	pKvRoot->deleteThis();
	return bValidate;
}

//-----------------------------------------------------------------------------
CON_COMMAND(ff_validate, "Requests a validation of the crc checksum of the server's current level scripts." )
{
	if(engine->Cmd_Argc() > 1)
	{
		const char* szFilePath = engine->Cmd_Argv(1);
		
		ClientModeFFNormal* pClientMode = GetClientModeFFNormal();
		pClientMode->SetNextValidationFilePath(szFilePath);
	}

	engine->ClientCmd("ff_scriptcrc");
}
