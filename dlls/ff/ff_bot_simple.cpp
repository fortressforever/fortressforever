
#include "cbase.h"
#include "gameinterface.h"
#include "ff_utils.h"

#include "ff_bot_simple.h"

LINK_ENTITY_TO_CLASS( ff_bot, CFFBot );

static int g_CurBotNumber = 1;

void CFFBot::PreThink()
{
	RunNullCommand();
	BaseClass::PreThink();
}

void CFFBot::PostThink()
{
	BaseClass::PostThink();
}

//-----------------------------------------------------------------------------
// Purpose: Create a new Bot and put it in the game.
// Output : Pointer to the new Bot, or NULL if there's no free clients.
//-----------------------------------------------------------------------------
CFFBot *BotPutInServer( int teamNum, int classNum, const char *botname )
{
	if (!botname)
	{
		char generatedBotName[ 64 ];
		Q_snprintf( generatedBotName, sizeof( generatedBotName ), "Bot%02i", g_CurBotNumber );
		botname = generatedBotName;
		g_CurBotNumber++;
	}

	// This trick lets us create a CFFBot for this client instead of the CFFPlayer
	// that we would normally get when ClientPutInServer is called.
	ClientPutInServerOverride( &CBotManager::ClientPutInServerOverride_Bot );
	edict_t *pEdict = engine->CreateFakeClient( botname );
	ClientPutInServerOverride( NULL );

	if (!pEdict)
	{
		Warning( "Failed to create Bot.\n");
		return NULL;
	}

	// Allocate a player entity for the bot, and call spawn
	CFFBot *pBot = ((CFFBot*)CBaseEntity::Instance( pEdict ));

	pBot->ClearFlags();
	pBot->AddFlag( FL_CLIENT | FL_FAKECLIENT );

	pBot->ChangeTeam( teamNum );
	pBot->ChangeClass( Class_IntToString(classNum) );
	pBot->RemoveAllItems( true );
	pBot->Spawn();

	return pBot;
}

// Handler for the "bot" command.
void CC_FF_BotAdd()
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
	{
		Msg( "You must be a server admin to use bot_add\n" );
		return;
	}

	int teamNum = (engine->Cmd_Argc() > 1) ? atoi( engine->Cmd_Argv( 1 ) ) + 1 : TEAM_BLUE;
	int classNum = (engine->Cmd_Argc() > 2) ? atoi( engine->Cmd_Argv( 2 ) ) : CLASS_SCOUT;

	BotPutInServer( teamNum, classNum );
}

ConCommand cc_Bot( "bot_add", CC_FF_BotAdd, "Add a bot.", FCVAR_CHEAT );