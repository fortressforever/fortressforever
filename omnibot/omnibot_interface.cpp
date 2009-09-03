//////////////////////////////////////////////////////////////////////////
// Bot-Related Includes
#include "cbase.h"
#include "ammodef.h"
#include "in_buttons.h"
#include "playerinfomanager.h"
#include "filesystem.h"
#include "Color.h"
#include "world.h"
#include "ff_item_flag.h"
#include "triggers.h"
#include "movevars_shared.h"
#include "nav.h"
#include "nav_ladder.h"

#include "ff_scriptman.h"
#include "ff_luacontext.h"
#include "ff_utils.h"
#include "ff_gamerules.h"
extern ConVar mp_prematch;

// Mirv: Just added this to stop all the redefinition warnings whenever i do a full recompile
#pragma warning(disable: 4005)

//#pragma optimize("", off)

#include "BotExports.h"

#include "omnibot_interface.h"
#include "omnibot_eventhandler.h"
#include "Omni-Bot_Events.h"

#include <vector>

ConVar	omnibot_enable( "omnibot_enable", "1", FCVAR_ARCHIVE | FCVAR_PROTECTED);
ConVar	omnibot_path( "omnibot_path", "omni-bot", FCVAR_ARCHIVE | FCVAR_PROTECTED);
ConVar	omnibot_nav( "omnibot_nav", "1", FCVAR_ARCHIVE | FCVAR_PROTECTED);
ConVar	omnibot_debug( "omnibot_debug", "0", FCVAR_ARCHIVE | FCVAR_PROTECTED);

#define OMNIBOT_MODNAME "Fortress Forever"

extern IServerPluginHelpers *serverpluginhelpers;

//////////////////////////////////////////////////////////////////////////

struct BotGoalInfo
{
	char		m_GoalName[64];
	int			m_GoalType;
	int			m_GoalTeam;
	edict_t		*m_Entity;
};

const int		MAX_DEFERRED_GOALS = 64;
int				g_DeferredGoalIndex = 0;
BotGoalInfo		g_DeferredGoals[MAX_DEFERRED_GOALS] = {};
bool			g_Started = false;

struct BotSpawnInfo
{
	char			m_Name[64];
	int				m_Team;
	int				m_Class;
	CFFInfoScript	*m_SpawnPoint;
};

BotSpawnInfo g_DeferredSpawn[MAX_PLAYERS] = {0};
int g_DeferredBotSpawnIndex = 0;

//////////////////////////////////////////////////////////////////////////
template <int NUM_ENTITIES = 2048>
class BotEntityHandles
{
public:
	enum { NumEntities = NUM_ENTITIES, InvalidSerial = -1 };
	obint16 SerialForIndex(obint16 index)
	{
		return m_EntSerials[index].m_Used ? m_EntSerials[index].m_HandleSerial : InvalidSerial;
	}
	void AllocForIndex(obint16 index)
	{
		m_EntSerials[index].m_NewEntity = false;
		m_EntSerials[index].m_Used = true;
		m_EntSerials[index].m_HandleSerial++;
	}
	void QueueForIndex(obint16 index)
	{
		m_EntSerials[index].m_NewEntity = true;
		m_EntSerials[index].m_Used = false;
		m_EntSerials[index].m_HandleSerial++;
	}
	void FreeForIndex(obint16 index)
	{
		while(++m_EntSerials[index].m_HandleSerial==0) {}
		m_EntSerials[index].m_NewEntity = false;
		m_EntSerials[index].m_Used = false;
	}
	bool IsIndexNew(obint16 index)
	{
		return m_EntSerials[index].m_NewEntity!=0;
	}
	void ClearIndexNew(obint16 index)
	{
		m_EntSerials[index].m_NewEntity = false;
	}
	void Reset()
	{
		for(int i = 0; i < NumEntities; ++i)
			m_EntSerials[i] = EntSerial();
	}
private:
	struct EntSerial
	{
		obint16	m_HandleSerial : 14;
		obint16	m_NewEntity : 1;
		obint16	m_Used : 1;

		EntSerial() : m_HandleSerial(InvalidSerial), m_NewEntity(false), m_Used(false)
		{
		}
	};	

	EntSerial m_EntSerials[NUM_ENTITIES];
};
//////////////////////////////////////////////////////////////////////////
//struct BotEntity
//{
//	obint16	m_HandleSerial;
//	bool	m_NewEntity : 1;
//	bool	m_Used : 1;
//};
//
//BotEntity DefaultBotEntity()
//{
//	BotEntity e;
//	e.m_HandleSerial = 1;
//	e.m_NewEntity = false;
//	e.m_Used = false;
//	return e;
//}

typedef BotEntityHandles<4096> EntSerials;
EntSerials g_EntSerials;

//const int MAX_ENTITIES = 4096;
//BotEntity		m_EntityHandles[MAX_ENTITIES] = {DefaultBotEntity()};

//////////////////////////////////////////////////////////////////////////
void NormalizeAngles( QAngle& angles )
{
	// Normalize angles to -180 to 180 range
	for (int i = 0; i < 3; i++ )
	{
		if ( angles[i] > 180.0 )
		{
			angles[i] -= 360.0;
		}
		else if ( angles[i] < -180.0 )
		{
			angles[i] += 360.0;
		}
	}
}
//////////////////////////////////////////////////////////////////////////

void Omnibot_Load_PrintMsg(const char *_msg)
{
	Msg("Omni-bot Loading: %s\n", _msg);
}

void Omnibot_Load_PrintErr(const char *_msg)
{
	Warning("Omni-bot Loading: %s\n", _msg);
}

//////////////////////////////////////////////////////////////////////////

class OmnibotServerPlugin : public IServerPluginCallbacks
{
public:
	OmnibotServerPlugin() {}
	~OmnibotServerPlugin() {}

	virtual bool			Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
	{
		return true;
	}
	virtual void			Unload( void ) {}
	virtual void			Pause( void ) {}
	virtual void			UnPause( void ) {}
	virtual const char     *GetPluginDescription( void ) { return "Omni-bot Internal Interface"; }      
	virtual void			LevelInit( char const *pMapName ) { }
	virtual void			ServerActivate( edict_t *pEdictList, int edictCount, int clientMax ) {}
	virtual void			GameFrame( bool simulating ) {}
	virtual void			LevelShutdown( void ) {}
	virtual void			ClientActive( edict_t *pEntity ) {}
	virtual void			ClientDisconnect( edict_t *pEntity ) {}
	virtual void			ClientPutInServer( edict_t *pEntity, char const *playername ) {}
	virtual void			SetCommandClient( int index ) {}
	virtual void			ClientSettingsChanged( edict_t *pEdict ) {}
	virtual PLUGIN_RESULT	ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen ) 
	{
		return PLUGIN_CONTINUE;
	}
	virtual PLUGIN_RESULT	ClientCommand( edict_t *pEntity )
	{
		return PLUGIN_CONTINUE;
	}
	virtual PLUGIN_RESULT	NetworkIDValidated( const char *pszUserName, const char *pszNetworkID )
	{
		return PLUGIN_CONTINUE;
	}

	//virtual int GetCommandIndex() { return m_iClientCommandIndex; }
private:
};

OmnibotServerPlugin g_ServerPlugin;

//////////////////////////////////////////////////////////////////////////

namespace Omnibot
{
#include "TF_Messages.h"

	CON_COMMAND( bot, "Omni-Bot Commands" )
	{
		omnibot_interface::OmnibotCommand();
	}

	//-----------------------------------------------------------------

	const char *g_Weapons[TF_WP_MAX] =
	{
		0,		
		"ff_weapon_umbrella", // TF_WP_UMBRELLA
		0, // TF_WP_AXE
		"ff_weapon_crowbar", // TF_WP_CROWBAR
		"ff_weapon_medkit", // TF_WP_MEDKIT
		"ff_weapon_knife", // TF_WP_KNIFE
		"ff_weapon_spanner", // TF_WP_SPANNER
		"ff_weapon_shotgun", // TF_WP_SHOTGUN
		"ff_weapon_supershotgun", // TF_WP_SUPERSHOTGUN	
		"ff_weapon_nailgun", // TF_WP_NAILGUN
		"ff_weapon_supernailgun", // TF_WP_SUPERNAILGUN		
		"ff_weapon_grenadelauncher", // TF_WP_GRENADE_LAUNCHER
		"ff_weapon_rpg", // TF_WP_ROCKET_LAUNCHER
		"ff_weapon_sniperrifle", // TF_WP_SNIPER_RIFLE		
		"ff_weapon_railgun", // TF_WP_RAILGUN
		"ff_weapon_flamethrower", // TF_WP_FLAMETHROWER
		"ff_weapon_assaultcannon", // TF_WP_MINIGUN
		"ff_weapon_autorifle", // TF_WP_AUTORIFLE
		"ff_weapon_tranq", // TF_WP_DARTGUN
		"ff_weapon_pipelauncher", // TF_WP_PIPELAUNCHER
		"ff_weapon_ic", // TF_WP_NAPALMCANNON
		"ff_weapon_tommygun", // TF_WP_TOMMYGUN
		"ff_weapon_deploysentrygun", // TF_WP_DEPLOY_SG
		"ff_weapon_deploydispenser", // TF_WP_DEPLOY_DISP
		"ff_weapon_deploydetpack", // TF_WP_DEPLOY_DETP
		"ff_weapon_deploymancannon", // TF_WP_DEPLOY_JUMPPAD
		"ff_weapon_flag", // TF_WP_FLAG
	};

	int obUtilGetWeaponId(const char *_weaponName)
	{		
		if(_weaponName)
		{
			for(int i = 1; i < TF_WP_MAX; ++i)
			{			
				if(g_Weapons[i] && !Q_strcmp(g_Weapons[i], _weaponName))
					return i;
			}		
		}
		return TF_WP_NONE;
	}

	const char *obUtilGetStringFromWeaponId(int _weaponId)
	{
		if(_weaponId > TF_WP_NONE && _weaponId < TF_WP_MAX)
		{
			return g_Weapons[_weaponId];
		}	
		return 0;
	}

	const int obUtilGetBotTeamFromGameTeam(int _team)
	{
		switch(_team)
		{
		case TEAM_BLUE:
			return TF_TEAM_BLUE;
		case TEAM_RED:
			return TF_TEAM_RED;
		case TEAM_YELLOW:
			return TF_TEAM_YELLOW;
		case TEAM_GREEN:	
			return TF_TEAM_GREEN;
		}
		return TF_TEAM_NONE;
	}

	const int obUtilGetGameTeamFromBotTeam(int _team)
	{
		switch(_team)
		{
		case TF_TEAM_BLUE:
			return TEAM_BLUE;
		case TF_TEAM_RED:
			return TEAM_RED;
		case TF_TEAM_YELLOW:
			return TEAM_YELLOW;
		case TF_TEAM_GREEN:	
			return TEAM_GREEN;
		}
		return TEAM_UNASSIGNED;
	}

	const int obUtilGetGameClassFromBotClass(int _class)
	{
		switch(_class)
		{		
		case TF_CLASS_SCOUT:
			return CLASS_SCOUT;
		case TF_CLASS_SNIPER:
			return CLASS_SNIPER;
		case TF_CLASS_SOLDIER:
			return CLASS_SOLDIER;
		case TF_CLASS_DEMOMAN:
			return CLASS_DEMOMAN;
		case TF_CLASS_MEDIC:
			return CLASS_MEDIC;
		case TF_CLASS_HWGUY:
			return CLASS_HWGUY;
		case TF_CLASS_PYRO:
			return CLASS_PYRO;
		case TF_CLASS_SPY:
			return CLASS_SPY;
		case TF_CLASS_ENGINEER:
			return CLASS_ENGINEER;
		case TF_CLASS_CIVILIAN:
			return CLASS_CIVILIAN;
		}
		return -1;
	}

	const int obUtilGetBotClassFromGameClass(int _class)
	{
		switch(_class)
		{		
		case CLASS_SCOUT:
			return TF_CLASS_SCOUT;
		case CLASS_SNIPER:
			return TF_CLASS_SNIPER;
		case CLASS_SOLDIER:
			return TF_CLASS_SOLDIER;
		case CLASS_DEMOMAN:
			return TF_CLASS_DEMOMAN;
		case CLASS_MEDIC:
			return TF_CLASS_MEDIC;
		case CLASS_HWGUY:
			return TF_CLASS_HWGUY;
		case CLASS_PYRO:
			return TF_CLASS_PYRO;
		case CLASS_SPY:
			return TF_CLASS_SPY;
		case CLASS_ENGINEER:
			return TF_CLASS_ENGINEER;
		case CLASS_CIVILIAN:
			return TF_CLASS_CIVILIAN;
		}
		return TF_CLASS_NONE;
	}

	const int obUtilGetBotWeaponFromGameWeapon(int _gameWpn)
	{
		switch(_gameWpn)
		{
		case FF_WEAPON_CROWBAR: 
			return TF_WP_CROWBAR;
		case FF_WEAPON_KNIFE: 
			return TF_WP_KNIFE;
		case FF_WEAPON_MEDKIT: 
			return TF_WP_MEDKIT;
		case FF_WEAPON_SPANNER: 
			return TF_WP_SPANNER;
		case FF_WEAPON_UMBRELLA: 
			return TF_WP_UMBRELLA;
		case FF_WEAPON_FLAG:
			return TF_WP_FLAG;
		case FF_WEAPON_SHOTGUN: 
			return TF_WP_SHOTGUN;
		case FF_WEAPON_SUPERSHOTGUN: 
			return TF_WP_SUPERSHOTGUN;
		case FF_WEAPON_NAILGUN: 
			return TF_WP_NAILGUN;
		case FF_WEAPON_SUPERNAILGUN: 
			return TF_WP_SUPERNAILGUN;
		case FF_WEAPON_GRENADELAUNCHER: 
			return TF_WP_GRENADE_LAUNCHER;
		case FF_WEAPON_PIPELAUNCHER:
			return TF_WP_PIPELAUNCHER;
		case FF_WEAPON_AUTORIFLE: 
			return TF_WP_AUTORIFLE;
		case FF_WEAPON_SNIPERRIFLE: 
			return TF_WP_SNIPER_RIFLE;
		case FF_WEAPON_FLAMETHROWER: 
			return TF_WP_FLAMETHROWER;
		case FF_WEAPON_IC: 
			return TF_WP_NAPALMCANNON;
		case FF_WEAPON_RAILGUN: 
			return TF_WP_RAILGUN;
		case FF_WEAPON_TRANQUILISER: 
			return TF_WP_DARTGUN;
		case FF_WEAPON_ASSAULTCANNON: 
			return TF_WP_MINIGUN;
		case FF_WEAPON_RPG: 
			return TF_WP_ROCKET_LAUNCHER;
		case FF_WEAPON_DEPLOYDISPENSER: 
			return TF_WP_DEPLOY_DISP;
		case FF_WEAPON_DEPLOYSENTRYGUN: 
			return TF_WP_DEPLOY_SG;
		case FF_WEAPON_DEPLOYDETPACK: 
			return TF_WP_DEPLOY_DETP;
		default:
			return TF_WP_NONE;
		}		
	}

	const int obUtilGetGameWeaponFromBotWeapon(int _botWpn)
	{
		switch(_botWpn)
		{
		case TF_WP_CROWBAR: 
			return FF_WEAPON_CROWBAR;
		case TF_WP_KNIFE: 
			return FF_WEAPON_KNIFE;
		case TF_WP_MEDKIT: 
			return FF_WEAPON_MEDKIT;
		case TF_WP_SPANNER: 
			return FF_WEAPON_SPANNER;
		case TF_WP_UMBRELLA: 
			return FF_WEAPON_UMBRELLA;
		case TF_WP_FLAG:
			return FF_WEAPON_FLAG;
		case TF_WP_SHOTGUN: 
			return FF_WEAPON_SHOTGUN;
		case TF_WP_SUPERSHOTGUN: 
			return FF_WEAPON_SUPERSHOTGUN;
		case TF_WP_NAILGUN: 
			return FF_WEAPON_NAILGUN;
		case TF_WP_SUPERNAILGUN: 
			return FF_WEAPON_SUPERNAILGUN;
		case TF_WP_GRENADE_LAUNCHER: 
			return FF_WEAPON_GRENADELAUNCHER;
		case TF_WP_PIPELAUNCHER:
			return FF_WEAPON_PIPELAUNCHER;
		case TF_WP_AUTORIFLE: 
			return FF_WEAPON_AUTORIFLE;
		case TF_WP_SNIPER_RIFLE: 
			return FF_WEAPON_SNIPERRIFLE;
		case TF_WP_FLAMETHROWER: 
			return FF_WEAPON_FLAMETHROWER;
		case TF_WP_NAPALMCANNON: 
			return FF_WEAPON_IC;
		case TF_WP_RAILGUN: 
			return FF_WEAPON_RAILGUN;
		case TF_WP_DARTGUN: 
			return FF_WEAPON_TRANQUILISER;
		case TF_WP_MINIGUN: 
			return FF_WEAPON_ASSAULTCANNON;
		case TF_WP_ROCKET_LAUNCHER: 
			return FF_WEAPON_RPG;
		case TF_WP_DEPLOY_DISP: 
			return FF_WEAPON_DEPLOYDISPENSER;
		case TF_WP_DEPLOY_SG: 
			return FF_WEAPON_DEPLOYSENTRYGUN;
		case TF_WP_DEPLOY_DETP: 
			return FF_WEAPON_DEPLOYDETPACK;
		default:
			return FF_WEAPON_NONE;
		}
	}

	int obUtilBotContentsFromGameContents(int _contents)
	{
		int iBotContents = 0;
		if(_contents & CONTENTS_SOLID)
			iBotContents |= CONT_SOLID;
		if(_contents & CONTENTS_WATER)
			iBotContents |= CONT_WATER;
		if(_contents & CONTENTS_SLIME)
			iBotContents |= CONT_SLIME;
		if(_contents & CONTENTS_LADDER)
			iBotContents |= CONT_LADDER;
		if(_contents & CONTENTS_MOVEABLE)
			iBotContents |= CONT_MOVABLE;
		return iBotContents;
	}

	const char *GetGameClassNameFromBotClassId(int _classId)
	{
		switch(_classId)
		{
		case TF_CLASS_SCOUT:
		case TF_CLASS_SNIPER:
		case TF_CLASS_SOLDIER:
		case TF_CLASS_DEMOMAN:
		case TF_CLASS_MEDIC:
		case TF_CLASS_HWGUY:
		case TF_CLASS_PYRO:
		case TF_CLASS_SPY:
		case TF_CLASS_ENGINEER:
		case TF_CLASS_CIVILIAN:
			return "player";			
		case TF_CLASSEX_SENTRY:
			return "FF_SentryGun";
		case TF_CLASSEX_DISPENSER:
			return "FF_Dispenser";
		case TF_CLASSEX_BACKPACK_AMMO:
		case TF_CLASSEX_BACKPACK_HEALTH:
		case TF_CLASSEX_BACKPACK_ARMOR:
		case TF_CLASSEX_BACKPACK_GRENADES:
			return "ff_item_backpack";
		case TF_CLASSEX_DETPACK:
			return "FF_Detpack";
		case TF_CLASSEX_GRENADE:
			return "normalgrenade";
		case TF_CLASSEX_EMP_GRENADE:
			return "empgrenade";
		case TF_CLASSEX_NAIL_GRENADE:
			return "nailgrenade";
		case TF_CLASSEX_MIRV_GRENADE:
			return "mirvgrenade";
		case TF_CLASSEX_MIRVLET_GRENADE:
			return "mirvlet";
		case TF_CLASSEX_NAPALM_GRENADE:
			return "napalmgrenade";
		case TF_CLASSEX_GAS_GRENADE:
			return "gasgrenade";
		case TF_CLASSEX_CONC_GRENADE:
			return "concussiongrenade";
		case TF_CLASSEX_CALTROP:
			return "caltrop";
		case TF_CLASSEX_PIPE:
			return "pipebomb";
		case TF_CLASSEX_GLGRENADE:
			return "glgrenade";
		case TF_CLASSEX_ROCKET:
			return "rocket";
		case TF_CLASSEX_TURRET:
			return "ff_miniturret";
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
			/*case ENT_CLASS_GENERIC_PLAYERSTART:
			"info_player_coop"
			"info_player_start"
			"info_player_deathmatch"*/
			/*case ENT_CLASS_GENERIC_BUTTON:
			case ENT_CLASS_GENERIC_HEALTH:
			case ENT_CLASS_GENERIC_AMMO:
			case ENT_CLASS_GENERIC_ARMOR:*/
		case ENT_CLASS_GENERIC_LADDER:
			return "func_useableladder";
			/*case ENT_CLASS_GENERIC_TELEPORTER:
			case ENT_CLASS_GENERIC_LIFT:
			case ENT_CLASS_GENERIC_MOVER:*/
		}
		return 0;
	}

	void Bot_Queue_EntityCreated(CBaseEntity *pEnt);
	void Bot_Event_EntityCreated(CBaseEntity *pEnt);
	void Bot_Event_EntityDeleted(CBaseEntity *pEnt);

	edict_t* INDEXEDICT(int iEdictNum)		
	{ 
		return engine->PEntityOfEntIndex(iEdictNum); 
	}

	int ENTINDEX(const edict_t *pEdict)		
	{ 
		return engine->IndexOfEdict(pEdict);
	}

	CBaseEntity *EntityFromHandle(GameEntity _ent)
	{
		obint16 index = _ent.GetIndex();
		if(g_EntSerials.SerialForIndex(index) == _ent.GetSerial())
		{
			edict_t *edict = INDEXEDICT(index);
			return edict ? CBaseEntity::Instance(edict) : NULL;
		}
		return NULL;
	}

	GameEntity HandleFromEntity(CBaseEntity *_ent)
	{
		if(_ent)
		{
			int index = ENTINDEX(_ent->edict());
			return GameEntity(index, g_EntSerials.SerialForIndex(index));
		}
		else
			return GameEntity();
	}

	//////////////////////////////////////////////////////////////////////////

	class FFInterface : public IEngineInterface
	{
	public:
		int AddBot(const MessageHelper &_data)
		{
			OB_GETMSG(Msg_Addbot);

			int iClientNum = -1;

			edict_t *pEdict = engine->CreateFakeClient( pMsg->m_Name );
			if (!pEdict)
			{
				PrintError("Unable to Add Bot!");
				return -1;
			}

			// Allocate a player entity for the bot, and call spawn
			CBasePlayer *pPlayer = ((CBasePlayer*)CBaseEntity::Instance( pEdict ));
			
			CFFPlayer *pFFPlayer = ToFFPlayer(pPlayer);
			if(pFFPlayer && pMsg->m_SpawnPointName[0])
			{
				CBaseEntity *pSpawnPt = gEntList.FindEntityByName(NULL, pMsg->m_SpawnPointName);
				if(pSpawnPt)
					pFFPlayer->m_SpawnPointOverride = pSpawnPt;
				else
					Warning("Bot Spawn Point Not Found: %s", pMsg->m_SpawnPointName);
			}

			pPlayer->ClearFlags();
			pPlayer->AddFlag( FL_CLIENT | FL_FAKECLIENT );

			pPlayer->ChangeTeam( TEAM_UNASSIGNED );
			pPlayer->RemoveAllItems( true );
			pPlayer->Spawn();

			// Get the index of the bot.
			iClientNum = engine->IndexOfEdict(pEdict);

			//////////////////////////////////////////////////////////////////////////
			if(g_EntSerials.IsIndexNew(iClientNum))
			{
				g_EntSerials.ClearIndexNew(iClientNum);
				CBaseEntity *pEnt = CBaseEntity::Instance(iClientNum);
				if(pEnt)
					Bot_Event_EntityCreated(pEnt);
			}
			//////////////////////////////////////////////////////////////////////////
			// Success!, return its client num.
			return iClientNum;
		}

		void RemoveBot(const MessageHelper &_data)
		{
			OB_GETMSG(Msg_Kickbot);
			if(pMsg->m_GameId != Msg_Kickbot::InvalidGameId)
			{
				CBasePlayer *ent = UTIL_PlayerByIndex(pMsg->m_GameId);
				if(ent && ent->IsBot())					
					engine->ServerCommand(UTIL_VarArgs("kick %s\n", ent->GetPlayerName()));
			}
			else
			{
				CBasePlayer *ent = UTIL_PlayerByName(pMsg->m_Name);
				if(ent && ent->IsBot())
					engine->ServerCommand(UTIL_VarArgs("kick %s\n", ent->GetPlayerName()));
			}
		}

		obResult ChangeTeam(int _client, int _newteam, const MessageHelper *_data)
		{
			edict_t *pEdict = INDEXEDICT(_client);

			if(pEdict)
			{
				const char *pTeam = "auto";
				switch(obUtilGetGameTeamFromBotTeam(_newteam))
				{
				case TEAM_BLUE:
					pTeam = "blue";
					break;
				case TEAM_RED:
					pTeam = "red";
					break;
				case TEAM_YELLOW:
					pTeam = "yellow";
					break;
				case TEAM_GREEN:
					pTeam = "green";
					break;
				default: 
					{
						// pick a random available team
						int iRandTeam = UTIL_PickRandomTeam();
						switch(iRandTeam)
						{
						case TEAM_BLUE:
							pTeam = "blue";
							break;
						case TEAM_RED:
							pTeam = "red";
							break;
						case TEAM_YELLOW:
							pTeam = "yellow";
							break;
						case TEAM_GREEN:
							pTeam = "green";
							break;
						}
						break;
					}
				}
				serverpluginhelpers->ClientCommand(pEdict, UTIL_VarArgs( "team %s", pTeam ));
				return Success;
			}
			return InvalidEntity;
		}

		obResult ChangeClass(int _client, int _newclass, const MessageHelper *_data)
		{
			edict_t *pEdict = INDEXEDICT(_client);

			if(pEdict)
			{
				CBaseEntity *pEntity = CBaseEntity::Instance( pEdict );
				CFFPlayer *pFFPlayer = dynamic_cast<CFFPlayer*>(pEntity);
				ASSERT(pFFPlayer);
				if(pFFPlayer)
				{
					const char *pClassName = "randompc";
					switch(obUtilGetGameClassFromBotClass(_newclass))
					{
					case CLASS_SCOUT:
						pClassName = "scout";
						break;
					case CLASS_SNIPER:
						pClassName = "sniper";
						break;
					case CLASS_SOLDIER:
						pClassName = "soldier";
						break;
					case CLASS_DEMOMAN:
						pClassName = "demoman";
						break;
					case CLASS_MEDIC:
						pClassName = "medic";
						break;
					case CLASS_HWGUY:
						pClassName = "hwguy";
						break;
					case CLASS_PYRO:
						pClassName = "pyro";
						break;
					case CLASS_SPY:
						pClassName = "spy";
						break;
					case CLASS_ENGINEER:
						pClassName = "engineer";
						break;
					case CLASS_CIVILIAN:
						pClassName = "civilian";
						break;
					default:
						{
							int iRandTeam = UTIL_PickRandomClass(pFFPlayer->GetTeamNumber());
							pClassName = Class_IntToString(iRandTeam);
							break;
						}
					}
					serverpluginhelpers->ClientCommand(pEdict, UTIL_VarArgs( "class %s", pClassName ));
					return Success;
				}
			}
			return InvalidEntity;
		}

		void UpdateBotInput(int _client, const ClientInput &_input)
		{
			edict_t *pEdict = INDEXENT(_client);
			CBaseEntity *pEntity = pEdict && !FNullEnt(pEdict) ? CBaseEntity::Instance(pEdict) : 0;
			CFFPlayer *pPlayer = pEntity ? ToFFPlayer(pEntity) : 0;
			if(pPlayer && pPlayer->IsBot())
			{				
				CBotCmd cmd;
				//CUserCmd cmd;

				// Process the bot keypresses.
				if(_input.m_ButtonFlags.CheckFlag(BOT_BUTTON_ATTACK1))
					cmd.buttons |= IN_ATTACK;
				if(_input.m_ButtonFlags.CheckFlag(BOT_BUTTON_ATTACK2))
					cmd.buttons |= IN_ATTACK2;
				if(_input.m_ButtonFlags.CheckFlag(BOT_BUTTON_WALK))
					cmd.buttons |= IN_SPEED;
				if(_input.m_ButtonFlags.CheckFlag(BOT_BUTTON_USE))
					cmd.buttons |= IN_USE;
				if(_input.m_ButtonFlags.CheckFlag(BOT_BUTTON_JUMP))
					cmd.buttons |= IN_JUMP;
				if(_input.m_ButtonFlags.CheckFlag(BOT_BUTTON_CROUCH))
					cmd.buttons |= IN_DUCK;
				if(_input.m_ButtonFlags.CheckFlag(BOT_BUTTON_RELOAD))
					cmd.buttons |= IN_RELOAD;
				if(_input.m_ButtonFlags.CheckFlag(BOT_BUTTON_RESPAWN))
					cmd.buttons |= IN_ATTACK;

				if(_input.m_ButtonFlags.CheckFlag(BOT_BUTTON_AIM))
					cmd.buttons |= IN_ZOOM;

				if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_GREN1))
					serverpluginhelpers->ClientCommand(pEdict, "primeone");
				else if(pPlayer->IsGrenade1Primed())
					serverpluginhelpers->ClientCommand(pEdict, "throwgren");

				if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_GREN2))
					serverpluginhelpers->ClientCommand(pEdict, "primetwo");
				else if(pPlayer->IsGrenade2Primed())
					serverpluginhelpers->ClientCommand(pEdict, "throwgren");
                
				if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_DROPITEM))
					serverpluginhelpers->ClientCommand(pEdict, "dropitems");
				if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_DROPAMMO))
					serverpluginhelpers->ClientCommand(pEdict, "discard");

				if(!pPlayer->IsBuilding())
				{
					if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_BUILDSENTRY))
						serverpluginhelpers->ClientCommand(pEdict, "sentrygun");
					if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_AIMSENTRY))
						serverpluginhelpers->ClientCommand(pEdict, "aimsentry");
					if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_BUILDDISPENSER))
						serverpluginhelpers->ClientCommand(pEdict, "dispenser");
					if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_BUILDDETPACK_5))
						serverpluginhelpers->ClientCommand(pEdict, "detpack 5");
					if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_BUILDDETPACK_10))
						serverpluginhelpers->ClientCommand(pEdict, "detpack 10");
					if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_BUILDDETPACK_20))
						serverpluginhelpers->ClientCommand(pEdict, "detpack 20");
					if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_BUILDDETPACK_30))
						serverpluginhelpers->ClientCommand(pEdict, "detpack 30");
				}
				else
				{
					if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_CANCELBUILD))
					{
						if(pPlayer->IsBuilding())
						{
							switch(pPlayer->GetCurBuild())
							{
							case FF_BUILD_DISPENSER: pPlayer->Command_BuildDispenser(); break;
							case FF_BUILD_SENTRYGUN: pPlayer->Command_BuildSentryGun(); break;
							case FF_BUILD_DETPACK: pPlayer->Command_BuildDetpack(); break; break;
							case FF_BUILD_MANCANNON: pPlayer->Command_BuildManCannon(); break;
							}
							return;
						}
					}
				}

				if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_DETSENTRY))
					serverpluginhelpers->ClientCommand(pEdict, "detsentry");
				if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_DETDISPENSER))
					serverpluginhelpers->ClientCommand(pEdict, "detdispenser");
				if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_DETPIPES))
					cmd.buttons |= IN_ATTACK2;
				if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_CALLFORMEDIC))
					serverpluginhelpers->ClientCommand(pEdict, "saveme");
				if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_CALLFORENGY))
					serverpluginhelpers->ClientCommand(pEdict, "engyme");
				if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_SABOTAGE_SENTRY))
					serverpluginhelpers->ClientCommand(pEdict, "sentrysabotage");
				if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_SABOTAGE_DISPENSER))
					serverpluginhelpers->ClientCommand(pEdict, "dispensersabotage");				
				if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_CLOAK))
					serverpluginhelpers->ClientCommand(pEdict, "cloak");
				if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_SILENT_CLOAK))
					serverpluginhelpers->ClientCommand(pEdict, "scloak");
				if(_input.m_ButtonFlags.CheckFlag(TF_BOT_BUTTON_RADAR))
					serverpluginhelpers->ClientCommand(pEdict, "radar");

				// Convert the facing vector to angles.
				const QAngle currentAngles = pPlayer->EyeAngles();
				Vector vFacing(_input.m_Facing[0], _input.m_Facing[1], _input.m_Facing[2]);
				VectorAngles(vFacing, cmd.viewangles);
				NormalizeAngles(cmd.viewangles);

				// Any facings that go abive the clamp need to have their yaw fixed just in case.
				if(cmd.viewangles[PITCH] > 89 || cmd.viewangles[PITCH] < -89)
					cmd.viewangles[YAW] = currentAngles[YAW];

				//cmd.viewangles[PITCH] = clamp(cmd.viewangles[PITCH],-89,89);

				// Calculate the movement vector, taking into account the view direction.
				QAngle angle2d = cmd.viewangles; angle2d.x = 0;

				Vector vForward, vRight, vUp;
				Vector vMoveDir(_input.m_MoveDir[0],_input.m_MoveDir[1],_input.m_MoveDir[2]);
				AngleVectors(angle2d, &vForward, &vRight, &vUp);

				const Vector worldUp(0.f, 0.f, 1.f);
				cmd.forwardmove = vForward.Dot(vMoveDir) * pPlayer->MaxSpeed();
				cmd.sidemove = vRight.Dot(vMoveDir) * pPlayer->MaxSpeed();
				cmd.upmove = worldUp.Dot(vMoveDir) * pPlayer->MaxSpeed();

				if(_input.m_ButtonFlags.CheckFlag(BOT_BUTTON_MOVEUP))
					cmd.upmove = 127;
				else if(_input.m_ButtonFlags.CheckFlag(BOT_BUTTON_MOVEDN))
					cmd.upmove = -127;

				if(cmd.sidemove > 0)
					cmd.buttons |= IN_MOVERIGHT;
				else if(cmd.sidemove < 0)
					cmd.buttons |= IN_MOVELEFT;

				if(pPlayer->IsOnLadder())
				{
					if(cmd.upmove > 0)
						cmd.buttons |= IN_FORWARD;
					else if(cmd.upmove < 0)
						cmd.buttons |= IN_BACK;
				}
				else
				{
					if(cmd.forwardmove > 0)
						cmd.buttons |= IN_FORWARD;
					else if(cmd.forwardmove < 0)
						cmd.buttons |= IN_BACK;
				}

				// Do we have this weapon?
				const char *pNewWeapon = obUtilGetStringFromWeaponId(_input.m_CurrentWeapon);
				CBaseCombatWeapon *pCurrentWpn = pPlayer->GetActiveWeapon();

				if(pNewWeapon && (!pCurrentWpn || !FStrEq(pCurrentWpn->GetClassname(), pNewWeapon)))
				{
					CBaseCombatWeapon *pWpn = pPlayer->Weapon_OwnsThisType(pNewWeapon);
					if(pWpn != pCurrentWpn)
					{
						pPlayer->Weapon_Switch(pWpn);
					}
				}

				pPlayer->GetBotController()->RunPlayerMove(&cmd);
				//pPlayer->ProcessUsercmds(&cmd, 1, 1, 0, false);
				pPlayer->GetBotController()->PostClientMessagesSent();
			}
		}

		void BotCommand(int _client, const char *_cmd)
		{
			edict_t *pEdict = INDEXENT(_client);
			if(pEdict && !FNullEnt(pEdict))
			{
				serverpluginhelpers->ClientCommand(pEdict, _cmd);
			}
		}

		obBool IsInPVS(const float _pos[3], const float _target[3])
		{
			Vector start(_pos[0],_pos[1],_pos[2]);
			Vector end(_target[0],_target[1],_target[2]);

			byte pvs[ MAX_MAP_CLUSTERS/8 ];
			int iPVSCluster = engine->GetClusterForOrigin(start);
			int iPVSLength = engine->GetPVSForCluster(iPVSCluster, sizeof(pvs), pvs);

			return engine->CheckOriginInPVS(end, pvs, iPVSLength) ? True : False;
		}

		obResult TraceLine(obTraceResult &_result, const float _start[3], const float _end[3], 
			const AABB *_pBBox , int _mask, int _user, obBool _bUsePVS)
		{
			Vector start(_start[0],_start[1],_start[2]);
			Vector end(_end[0],_end[1],_end[2]);

			byte pvs[ MAX_MAP_CLUSTERS/8 ];
			int iPVSCluster = engine->GetClusterForOrigin(start);
			int iPVSLength = engine->GetPVSForCluster(iPVSCluster, sizeof(pvs), pvs);

			bool bInPVS = _bUsePVS ? engine->CheckOriginInPVS(end, pvs, iPVSLength) : true;
			if(bInPVS)
			{
				int iMask = 0;
				Ray_t ray;
				trace_t trace;

				// Set up the collision masks
				if(_mask & TR_MASK_ALL)
					iMask |= MASK_ALL;
				else
				{
					if(_mask & TR_MASK_SOLID)
						iMask |= MASK_SOLID;
					if(_mask & TR_MASK_PLAYER)
						iMask |= MASK_PLAYERSOLID;
					if(_mask & TR_MASK_SHOT)
						iMask |= MASK_SHOT;
					if(_mask & TR_MASK_OPAQUE)
						iMask |= MASK_OPAQUE;
					if(_mask & TR_MASK_WATER)
						iMask |= MASK_WATER;
					if(_mask & TR_MASK_FLOODFILL)
						iMask |= MASK_NPCWORLDSTATIC;
				}

				// Initialize a ray with or without a bounds
				if(_pBBox)
				{
					Vector mins(_pBBox->m_Mins[0],_pBBox->m_Mins[1],_pBBox->m_Mins[2]);
					Vector maxs(_pBBox->m_Maxs[0],_pBBox->m_Maxs[1],_pBBox->m_Maxs[2]);
					ray.Init(start, end, mins, maxs);
				}
				else
				{
					ray.Init(start, end);
				}

				CBaseEntity *pIgnoreEnt = _user > 0 ? CBaseEntity::Instance(_user) : 0;
				CTraceFilterSimple traceFilter(pIgnoreEnt, iMask);
				enginetrace->TraceRay(ray, iMask, &traceFilter, &trace);

				if(trace.DidHit() && trace.m_pEnt && (trace.m_pEnt->entindex() != 0))
					_result.m_HitEntity = HandleFromEntity(trace.m_pEnt);
				else
					_result.m_HitEntity = GameEntity();

				// Fill in the bot traceflag.			
				_result.m_Fraction = trace.fraction;
				_result.m_StartSolid = trace.startsolid;			
				_result.m_Endpos[0] = trace.endpos.x;
				_result.m_Endpos[1] = trace.endpos.y;
				_result.m_Endpos[2] = trace.endpos.z;
				_result.m_Normal[0] = trace.plane.normal.x;
				_result.m_Normal[1] = trace.plane.normal.y;
				_result.m_Normal[2] = trace.plane.normal.z;
				_result.m_Contents = obUtilBotContentsFromGameContents(trace.contents);
				return Success;
			}

			// No Hit or Not in PVS
			_result.m_Fraction = 0.0f;
			_result.m_HitEntity = GameEntity();

			return bInPVS ? Success : OutOfPVS;
		}

		int GetPointContents(const float _pos[3])
		{
			int iContents = UTIL_PointContents(Vector(_pos[0], _pos[1], _pos[2]));
			return obUtilBotContentsFromGameContents(iContents);
		}

		GameEntity GetLocalGameEntity()
		{
			if(!engine->IsDedicatedServer())
			{
				CBasePlayer *localPlayer = UTIL_PlayerByIndex( 1 );
				if(localPlayer)
					return HandleFromEntity(localPlayer);
			}
			return GameEntity();
		}

		GameEntity FindEntityInSphere(const float _pos[3], float _radius, GameEntity _pStart, int classId)
		{
			Vector start(_pos[0], _pos[1], _pos[2]);
			CBaseEntity *pEntity = _pStart.IsValid() ? EntityFromHandle(_pStart) : 0;

			do 
			{
				const char *pClassName = GetGameClassNameFromBotClassId(classId);
				pEntity = pClassName ? gEntList.FindEntityByClassnameWithin(pEntity, pClassName, start, _radius) : 0;
				// special case, if it's a player, don't consider spectators
				if(pEntity && !Q_strcmp(pClassName, "player"))
				{
					CBasePlayer *pPlayer = ToBasePlayer(pEntity);
					if(pPlayer && !pPlayer->IsObserver())
						break;
				}
				else
				{
					break;
				}
			} while(1);		

			return pEntity ? HandleFromEntity(pEntity) : GameEntity();
		}

		int GetEntityClass(const GameEntity _ent)
		{
			CBaseEntity *pEntity = EntityFromHandle(_ent);
			if(pEntity)
			{
				switch(pEntity->Classify())
				{
				case CLASS_PLAYER:
				case CLASS_PLAYER_ALLY:
					{
						CFFPlayer *pFFPlayer = ToFFPlayer(pEntity);
						if(pFFPlayer)
						{
							if(pFFPlayer->GetTeamNumber() <= TEAM_SPECTATOR)
								return ENT_CLASS_GENERIC_SPECTATOR;

							return obUtilGetBotClassFromGameClass(pFFPlayer->GetClassSlot());
						}
						break;
					}
				case CLASS_DISPENSER:
					return TF_CLASSEX_DISPENSER;
				case CLASS_SENTRYGUN:
					return TF_CLASSEX_SENTRY;
				case CLASS_DETPACK:
					return TF_CLASSEX_DETPACK;
				case CLASS_GREN:
					return TF_CLASSEX_GRENADE;
				case CLASS_GREN_EMP:
					return TF_CLASSEX_EMP_GRENADE;
				case CLASS_GREN_NAIL:
					return TF_CLASSEX_NAIL_GRENADE;
				case CLASS_GREN_MIRV:
					return TF_CLASSEX_MIRV_GRENADE;
				case CLASS_GREN_MIRVLET:
					return TF_CLASSEX_MIRVLET_GRENADE;
				case CLASS_GREN_NAPALM:
					return TF_CLASSEX_NAPALM_GRENADE;
				case CLASS_GREN_GAS:
					return TF_CLASSEX_GAS_GRENADE;
				case CLASS_GREN_CONC:
					return TF_CLASSEX_CONC_GRENADE;
				case CLASS_GREN_CALTROP:	
					return TF_CLASSEX_CALTROP;
				case CLASS_PIPEBOMB:
					return TF_CLASSEX_PIPE;
				case CLASS_GLGRENADE:
					return TF_CLASSEX_GLGRENADE;
				case CLASS_ROCKET:
					return TF_CLASSEX_ROCKET;
				case CLASS_TURRET:
					return TF_CLASSEX_TURRET;
				case CLASS_BACKPACK:
					return TF_CLASSEX_BACKPACK;
				case CLASS_INFOSCRIPT:
					{
						CFFInfoScript *pFFScript = static_cast<CFFInfoScript*>(pEntity);
						if(pFFScript)
						{
							switch(pFFScript->GetBotGoalType())
							{
							case Omnibot::kBackPack_Grenades:
								return TF_CLASSEX_BACKPACK_GRENADES;
							case Omnibot::kBackPack_Health:
								return TF_CLASSEX_BACKPACK_HEALTH;
							case Omnibot::kBackPack_Armor:
								return TF_CLASSEX_BACKPACK_ARMOR;
							case Omnibot::kBackPack_Ammo:
								return TF_CLASSEX_BACKPACK_AMMO;
							case Omnibot::kFlag:
								return ENT_CLASS_GENERIC_FLAG;
							case Omnibot::kFlagCap:
								return ENT_CLASS_GENERIC_FLAGCAPPOINT;
							case Omnibot::kHuntedEscape:
								return TF_CLASSEX_HUNTEDESCAPE;								
							}
						}
						break;
					}
				case CLASS_TRIGGERSCRIPT:
					{
						CFuncFFScript *pFFScript = static_cast<CFuncFFScript*>(pEntity);
						if(pFFScript)
						{
							switch(pFFScript->GetBotGoalType())
							{
							case Omnibot::kBackPack_Grenades:
								return TF_CLASSEX_BACKPACK_GRENADES;
							case Omnibot::kBackPack_Health:
								return TF_CLASSEX_BACKPACK_HEALTH;
							case Omnibot::kBackPack_Armor:
								return TF_CLASSEX_BACKPACK_ARMOR;
							case Omnibot::kBackPack_Ammo:
								return TF_CLASSEX_BACKPACK_AMMO;
							case Omnibot::kFlag:
								return ENT_CLASS_GENERIC_FLAG;
							case Omnibot::kFlagCap:
								return ENT_CLASS_GENERIC_FLAGCAPPOINT;
							case Omnibot::kHuntedEscape:
								return TF_CLASSEX_HUNTEDESCAPE;
							}
						}
						break;
					}
				}
			}
			return 0;
		}

		obResult GetEntityCategory(const GameEntity _ent, BitFlag32 &_category)
		{
			CBaseEntity *pEntity = EntityFromHandle(_ent);
			if(pEntity)
			{
				switch(pEntity->Classify())
				{
				case CLASS_PLAYER:
				case CLASS_PLAYER_ALLY:
					_category.SetFlag(ENT_CAT_SHOOTABLE);
					_category.SetFlag(ENT_CAT_PLAYER);
					_category.SetFlag(ENT_CAT_AVOID);
					break;
				case CLASS_DISPENSER:
					_category.SetFlag(TF_ENT_CAT_BUILDABLE);
					_category.SetFlag(ENT_CAT_SHOOTABLE);
					_category.SetFlag(ENT_CAT_AVOID);
					break;
				case CLASS_SENTRYGUN:
					_category.SetFlag(TF_ENT_CAT_BUILDABLE);
					_category.SetFlag(ENT_CAT_SHOOTABLE);
					_category.SetFlag(ENT_CAT_AVOID);
					break;
				case CLASS_DETPACK:
					_category.SetFlag(TF_ENT_CAT_BUILDABLE);
					break;
				case CLASS_GREN:
				case CLASS_GREN_EMP:
				case CLASS_GREN_NAIL:
				case CLASS_GREN_MIRV:
				case CLASS_GREN_MIRVLET:
				case CLASS_GREN_NAPALM:
				case CLASS_GREN_GAS:
				case CLASS_GREN_CONC:
				case CLASS_GREN_CALTROP:
				case CLASS_PIPEBOMB:
				case CLASS_GLGRENADE:
				case CLASS_ROCKET:
					_category.SetFlag(ENT_CAT_PROJECTILE);
					_category.SetFlag(ENT_CAT_AVOID);
					break;
				case CLASS_TURRET:
					_category.SetFlag(ENT_CAT_AUTODEFENSE);
					_category.SetFlag(ENT_CAT_STATIC);
					break;
				case CLASS_BACKPACK:
					_category.SetFlag(ENT_CAT_PICKUP);
					break;
				case CLASS_INFOSCRIPT:
					{
						CFFInfoScript *pFFScript = static_cast<CFFInfoScript*>(pEntity);
						if(pFFScript)
						{
							switch(pFFScript->GetBotGoalType())
							{
							case Omnibot::kBackPack_Grenades:
							case Omnibot::kBackPack_Health:
							case Omnibot::kBackPack_Armor:
							case Omnibot::kBackPack_Ammo:
							case Omnibot::kFlag:
								_category.SetFlag(ENT_CAT_PICKUP);
								_category.SetFlag(ENT_CAT_STATIC);
								break;
							case Omnibot::kFlagCap:
								_category.SetFlag(ENT_CAT_TRIGGER);
								break;
							case Omnibot::kHuntedEscape:
								_category.SetFlag(ENT_CAT_TRIGGER);
							}
						}
						break;
					}
				case CLASS_TRIGGERSCRIPT:
					{
						CFuncFFScript *pFFScript = static_cast<CFuncFFScript*>(pEntity);
						if(pFFScript)
						{
							switch(pFFScript->GetBotGoalType())
							{
							case Omnibot::kBackPack_Grenades:
							case Omnibot::kBackPack_Health:
							case Omnibot::kBackPack_Armor:
							case Omnibot::kBackPack_Ammo:
							case Omnibot::kFlag:
								_category.SetFlag(ENT_CAT_PICKUP);
								_category.SetFlag(ENT_CAT_STATIC);
								break;
							case Omnibot::kFlagCap:
								_category.SetFlag(ENT_CAT_TRIGGER);
								break;
							case Omnibot::kHuntedEscape:
								_category.SetFlag(ENT_CAT_TRIGGER);
								break;
							}
						}
						break;
					}
				default:
					return InvalidEntity;
				}
			}
			return Success;
		}

		obResult GetEntityFlags(const GameEntity _ent, BitFlag64 &_flags)
		{
			CBaseEntity *pEntity = EntityFromHandle(_ent);
			if(pEntity)
			{
				switch(pEntity->Classify())
				{
				case CLASS_PLAYER:
				case CLASS_PLAYER_ALLY:
					{
						_flags.SetFlag(ENT_FLAG_VISTEST);

						if(!pEntity->IsAlive() || pEntity->GetHealth() <= 0 || 
							pEntity->GetTeamNumber() == TEAM_SPECTATOR)
							_flags.SetFlag(ENT_FLAG_DEAD);

						if(pEntity->GetFlags() & FL_DUCKING)
							_flags.SetFlag(ENT_FLAG_CROUCHED);

						CFFPlayer *pffPlayer = ToFFPlayer(pEntity);
						if(pffPlayer)
						{
							CBaseCombatWeapon *pWpn = pffPlayer->GetActiveWeapon();
							if(pWpn && pWpn->m_bInReload)
								_flags.SetFlag(ENT_FLAG_RELOADING);
							if(pffPlayer->IsOnLadder())
								_flags.SetFlag(ENT_FLAG_ONLADDER);
							if(!pffPlayer->IsBot())
								_flags.SetFlag(ENT_FLAG_HUMANCONTROLLED);
							if(pffPlayer->IsSpeedEffectSet(SE_SNIPERRIFLE))
								_flags.SetFlag(ENT_FLAG_IRONSIGHT);
							if(pffPlayer->IsSpeedEffectSet(SE_ASSAULTCANNON))
								_flags.SetFlag(TF_ENT_FLAG_ASSAULTFIRING);
							if(pffPlayer->IsRadioTagged())
								_flags.SetFlag(TF_ENT_FLAG_RADIOTAGGED);
							if(pffPlayer->m_hSabotaging)
								_flags.SetFlag(TF_ENT_FLAG_SABOTAGING);
							
							if(pffPlayer->IsInfected())
								_flags.SetFlag(TF_ENT_FLAG_INFECTED);
							if(pffPlayer->IsBurning())
								_flags.SetFlag(TF_ENT_FLAG_BURNING);
							if(pffPlayer->IsSpeedEffectSet(SE_LEGSHOT))
								_flags.SetFlag(TF_ENT_FLAG_LEGSHOT);
							if(pffPlayer->IsSpeedEffectSet(SE_TRANQ))
								_flags.SetFlag(TF_ENT_FLAG_TRANQED);
							if(pffPlayer->IsSpeedEffectSet(SE_CALTROP))
								_flags.SetFlag(TF_ENT_FLAG_CALTROP);
							if(pffPlayer->IsGassed())
								_flags.SetFlag(TF_ENT_FLAG_GASSED);
							
							if(pffPlayer->IsBuilding())
							{
								switch(pffPlayer->GetCurBuild())
								{
								case FF_BUILD_DISPENSER:
									_flags.SetFlag(TF_ENT_FLAG_BUILDING_DISP);
									break;
								case FF_BUILD_SENTRYGUN:
									_flags.SetFlag(TF_ENT_FLAG_BUILDING_SG);
									break;
								case FF_BUILD_DETPACK:
									_flags.SetFlag(TF_ENT_FLAG_BUILDING_DETP);
									break;
								}
							}
						}
						break;
					}
					//////////////////////////////////////////////////////////////////////////
				case CLASS_INFOSCRIPT:
					{
						CFFInfoScript *pFFScript = static_cast<CFFInfoScript*>(pEntity);
						if(pFFScript)
						{
							if(pFFScript->IsEffectActive(EF_NODRAW) || pFFScript->IsRemoved())
								_flags.SetFlag(ENT_FLAG_DISABLED);

							/*switch(pFFScript->GetBotGoalType())
							{
							case Omnibot::kBackPack_Grenades:
							case Omnibot::kBackPack_Health:
							case Omnibot::kBackPack_Armor:
							case Omnibot::kBackPack_Ammo:
							case Omnibot::kFlag:
							case Omnibot::kFlagCap:
							}*/
						}
						break;
					}
				case CLASS_TRIGGERSCRIPT:
					{
						CFuncFFScript *pFFScript = static_cast<CFuncFFScript*>(pEntity);
						if(pFFScript)
						{
							if(pFFScript->IsEffectActive(EF_NODRAW) || pFFScript->IsRemoved())
								_flags.SetFlag(ENT_FLAG_DISABLED);

							/*switch(pFFScript->GetBotGoalType())
							{
							case Omnibot::kBackPack_Grenades:
							case Omnibot::kBackPack_Health:
							case Omnibot::kBackPack_Armor:
							case Omnibot::kBackPack_Ammo:
							case Omnibot::kFlag:
							case Omnibot::kFlagCap:
							}*/
						}
						break;
					}
					//////////////////////////////////////////////////////////////////////////
				case CLASS_DISPENSER:
					{
						CFFDispenser *pBuildable = static_cast<CFFDispenser*>(pEntity);
						if(!pBuildable->IsBuilt())
							_flags.SetFlag(TF_ENT_FLAG_BUILDINPROGRESS);

						_flags.SetFlag(ENT_FLAG_VISTEST);
						break;
					}
				case CLASS_SENTRYGUN:
					{
						CFFSentryGun *pBuildable = static_cast<CFFSentryGun*>(pEntity);
						if(!pBuildable->IsBuilt())
							_flags.SetFlag(TF_ENT_FLAG_BUILDINPROGRESS);
						if(pBuildable->GetLevel()==2)
							_flags.SetFlag(TF_ENT_FLAG_LEVEL2);
						else if(pBuildable->GetLevel()==3)
							_flags.SetFlag(TF_ENT_FLAG_LEVEL3);
						_flags.SetFlag(ENT_FLAG_VISTEST);
						break;
					}
				case CLASS_DETPACK:
					{
						_flags.SetFlag(ENT_FLAG_VISTEST);
						CFFDetpack *pBuildable = static_cast<CFFDetpack*>(pEntity);
						if(!pBuildable->IsBuilt())
							_flags.SetFlag(TF_ENT_FLAG_BUILDINPROGRESS);
						break;
					}
				case CLASS_GREN:
				case CLASS_GREN_EMP:
				case CLASS_GREN_NAIL:
				case CLASS_GREN_MIRV:
				case CLASS_GREN_MIRVLET:
				case CLASS_GREN_NAPALM:
				case CLASS_GREN_GAS:
				case CLASS_GREN_CONC:
				case CLASS_GREN_CALTROP:
				case CLASS_PIPEBOMB:
				case CLASS_GLGRENADE:
				case CLASS_ROCKET:
					_flags.SetFlag(ENT_FLAG_VISTEST);
					break;
				case CLASS_TURRET:					
					break;
				case CLASS_BACKPACK:
					_flags.SetFlag(ENT_FLAG_VISTEST);
					break;
				}

				// Common flags.
				int iWaterLevel = pEntity->GetWaterLevel();
				if(iWaterLevel == 3)
					_flags.SetFlag(ENT_FLAG_UNDERWATER);
				else if(iWaterLevel >= 2)
					_flags.SetFlag(ENT_FLAG_INWATER);

				if(pEntity->GetFlags() & FL_ONGROUND)
					_flags.SetFlag(ENT_FLAG_ONGROUND);

				if(pEntity->Classify()==CLASS_SENTRYGUN||
					pEntity->Classify()==CLASS_DISPENSER||
					pEntity->Classify()==CLASS_DETPACK)
				{
					CFFBuildableObject *pBuildable = dynamic_cast<CFFBuildableObject*>(pEntity);
					if(pBuildable)
					{
						if(pBuildable->CanSabotage())
							_flags.SetFlag(TF_ENT_FLAG_CAN_SABOTAGE);
						if(pBuildable->IsSabotaged())
							_flags.SetFlag(TF_ENT_FLAG_SABOTAGED);
						// need one for when shooting teammates?
					}
				}
				return Success;
			}
			return InvalidEntity;
		}

		obResult GetEntityPowerups(const GameEntity _ent, BitFlag64 &_flags)
		{
			CBaseEntity *pEntity = EntityFromHandle(_ent);
			if(pEntity)
			{
				switch(pEntity->Classify())
				{
				case CLASS_PLAYER:
					{
						CFFPlayer *pffPlayer = ToFFPlayer(pEntity);

						if(pffPlayer->IsCloaked())
							_flags.SetFlag(TF_PWR_CLOAKED);

						// Disguises
						if(pffPlayer)
						{
							switch(pffPlayer->GetDisguisedTeam())
							{
							case TEAM_BLUE:
								_flags.SetFlag(TF_PWR_DISGUISE_BLUE);
								break;
							case TEAM_RED:
								_flags.SetFlag(TF_PWR_DISGUISE_RED);
								break;
							case TEAM_YELLOW:
								_flags.SetFlag(TF_PWR_DISGUISE_YELLOW);
								break;
							case TEAM_GREEN:
								_flags.SetFlag(TF_PWR_DISGUISE_GREEN);
								break;
							}
							switch(pffPlayer->GetDisguisedClass())
							{
							case CLASS_SCOUT:
								_flags.SetFlag(TF_PWR_DISGUISE_SCOUT);
								break;
							case CLASS_SNIPER:
								_flags.SetFlag(TF_PWR_DISGUISE_SNIPER);
								break;
							case CLASS_SOLDIER:
								_flags.SetFlag(TF_PWR_DISGUISE_SOLDIER);
								break;
							case CLASS_DEMOMAN:
								_flags.SetFlag(TF_PWR_DISGUISE_DEMOMAN);
								break;
							case CLASS_MEDIC:
								_flags.SetFlag(TF_PWR_DISGUISE_MEDIC);
								break;
							case CLASS_HWGUY:
								_flags.SetFlag(TF_PWR_DISGUISE_HWGUY);
								break;
							case CLASS_PYRO:
								_flags.SetFlag(TF_PWR_DISGUISE_PYRO);
								break;
							case CLASS_SPY:
								_flags.SetFlag(TF_PWR_DISGUISE_SPY);
								break;
							case CLASS_ENGINEER:
								_flags.SetFlag(TF_PWR_DISGUISE_ENGINEER);
								break;
							case CLASS_CIVILIAN:
								_flags.SetFlag(TF_PWR_DISGUISE_CIVILIAN);
								break;
							}
						}
						break;
					}
					/*case CLASS_SENTRYGUN:
					case CLASS_DISPENSER:*/
				}
				return Success;
			}
			return InvalidEntity;
		}

		obResult GetEntityEyePosition(const GameEntity _ent, float _pos[3])
		{
			CBaseEntity *pEntity = EntityFromHandle(_ent);
			if(pEntity)
			{
				Vector vPos = pEntity->EyePosition();
				_pos[0] = vPos.x;
				_pos[1] = vPos.y;
				_pos[2] = vPos.z;
				return Success;
			}
			return InvalidEntity;
		}

		obResult GetEntityBonePosition(const GameEntity _ent, int _boneid, float _pos[3])
		{
			CBaseEntity *pEntity = EntityFromHandle(_ent);
			CBasePlayer *pPlayer = pEntity ? pEntity->MyCharacterPointer() : 0;
			if(pPlayer)
			{
				int iBoneIndex = -1;
				switch(_boneid)
				{
				case BONE_TORSO:
					iBoneIndex = pPlayer->LookupBone("ffSkel_Spine3");
					break;
				case BONE_PELVIS:
					iBoneIndex = pPlayer->LookupBone("ffSkel_Hips");
					break;
				case BONE_HEAD:
					iBoneIndex = pPlayer->LookupBone("ffSkel_Head");
					break;
				case BONE_RIGHTARM:
					iBoneIndex = pPlayer->LookupBone("ffSkel_RightForeArm");
					break;
				case BONE_LEFTARM:
					iBoneIndex = pPlayer->LookupBone("ffSkel_LeftForeArm");
					break;
				case BONE_RIGHTHAND:
					iBoneIndex = pPlayer->LookupBone("ffSkel_RightHand");
					break;
				case BONE_LEFTHAND:
					iBoneIndex = pPlayer->LookupBone("ffSkel_LeftHand");
					break;
				case BONE_RIGHTLEG:
					iBoneIndex = pPlayer->LookupBone("ffSkel_RightLeg");
					break;
				case BONE_LEFTLEG:
					iBoneIndex = pPlayer->LookupBone("ffSkel_LeftLeg");
					break;
				case BONE_RIGHTFOOT:
					iBoneIndex = pPlayer->LookupBone("ffSkel_RightFoot");
					break;
				case BONE_LEFTFOOT:
					iBoneIndex = pPlayer->LookupBone("ffSkel_LeftFoot");
					break;
					//"ffSg_Yaw"
				}

				if(iBoneIndex != -1)
				{
					Vector vBonePos;
					QAngle boneAngle;

					pPlayer->GetBonePosition(iBoneIndex, vBonePos, boneAngle);

					_pos[0] = vBonePos.x;
					_pos[1] = vBonePos.y;
					_pos[2] = vBonePos.z;

					return Success;
				}
				return InvalidParameter;
			}
			return InvalidEntity;
		}

		obResult GetEntityOrientation(const GameEntity _ent, float _fwd[3], float _right[3], float _up[3])
		{
			CBaseEntity *pEntity = EntityFromHandle(_ent);
			if(pEntity)
			{
				QAngle viewAngles = pEntity->EyeAngles();
				AngleVectors(viewAngles, (Vector*)_fwd, (Vector*)_right, (Vector*)_up);
				return Success;
			}
			return InvalidEntity;
		}

		obResult GetEntityVelocity(const GameEntity _ent, float _velocity[3])
		{
			CBaseEntity *pEntity = EntityFromHandle(_ent);
			if(pEntity)
			{
				const Vector &vVelocity = pEntity->GetAbsVelocity();
				_velocity[0] = vVelocity.x;
				_velocity[1] = vVelocity.y;
				_velocity[2] = vVelocity.z;
				return Success;
			}
			return InvalidEntity;
		}

		obResult GetEntityPosition(const GameEntity _ent, float _pos[3])
		{	
			CBaseEntity *pEntity = EntityFromHandle(_ent);
			if(pEntity)
			{
				const Vector &vPos = pEntity->GetAbsOrigin();
				_pos[0] = vPos.x;
				_pos[1] = vPos.y;
				_pos[2] = vPos.z;
				return Success;
			}
			return InvalidEntity;
		}

		obResult GetEntityWorldAABB(const GameEntity _ent, AABB &_aabb)
		{
			CBaseEntity *pEntity = EntityFromHandle(_ent);
			if(pEntity)
			{
				Vector vMins, vMaxs;

				CBasePlayer *pPlayer = pEntity->MyCharacterPointer();
				if(pPlayer)
				{
					Vector vOrig = pPlayer->GetAbsOrigin();
					vMins = vOrig + pPlayer->GetPlayerMins();
					vMaxs = vOrig + pPlayer->GetPlayerMaxs();
				}
				else
				{
					if(!pEntity->CollisionProp() || pEntity->entindex() == 0)
						return InvalidEntity;

					pEntity->CollisionProp()->WorldSpaceAABB(&vMins, &vMaxs);
				}

				_aabb.m_Mins[0] = vMins.x;
				_aabb.m_Mins[1] = vMins.y;
				_aabb.m_Mins[2] = vMins.z;
				_aabb.m_Maxs[0] = vMaxs.x;
				_aabb.m_Maxs[1] = vMaxs.y;
				_aabb.m_Maxs[2] = vMaxs.z;
				return Success;
			}
			return InvalidEntity;
		}

		obResult GetEntityGroundEntity(const GameEntity _ent, GameEntity &moveent)
		{
			CBaseEntity *pEntity = EntityFromHandle(_ent);
			if(pEntity)
			{
				CBaseEntity *pEnt = pEntity->GetGroundEntity();
				if(pEnt && pEnt != GetWorldEntity())
					moveent = HandleFromEntity(pEnt);
				return Success;
			}
			return InvalidEntity;
		}

		GameEntity GetEntityOwner(const GameEntity _ent)
		{
			GameEntity owner;

			CBaseEntity *pEntity = EntityFromHandle(_ent);
			if(pEntity)
			{
				switch(pEntity->Classify())
				{
				case CLASS_DISPENSER:
				case CLASS_SENTRYGUN:
				case CLASS_DETPACK:
					{
						CFFBuildableObject *pBuildable = static_cast<CFFBuildableObject*>(pEntity);
						CFFPlayer *pOwner = pBuildable->GetOwnerPlayer();
						if(pOwner)
							owner = HandleFromEntity(pOwner);
						break;
					}
				default:
					{
						CBaseEntity *pOwner = pEntity->GetOwnerEntity();
						if(pOwner)
							owner = HandleFromEntity(pOwner);
						break;
					}
				}
			}
			return owner;
		}

		int GetEntityTeam(const GameEntity _ent)
		{
			CBaseEntity *pEntity = EntityFromHandle(_ent);
			if(pEntity)
			{
				return obUtilGetBotTeamFromGameTeam(pEntity->GetTeamNumber());
			}
			return 0;
		}

		const char *GetEntityName(const GameEntity _ent)
		{
			const char *pName = 0;
			CBaseEntity *pEntity = EntityFromHandle(_ent);
			if(pEntity)
			{
				CBasePlayer *pPlayer = pEntity->MyCharacterPointer();
				if(pPlayer)
					pName = pPlayer->GetPlayerName();
				else
					pName = pEntity->GetName();
			}
			return pName ? pName : "";
		}

		obResult GetCurrentWeaponClip(const GameEntity _ent, FireMode _mode, int &_curclip, int &_maxclip)
		{
			CBaseEntity *pEntity = EntityFromHandle(_ent);
			CBasePlayer *pPlayer = pEntity ? pEntity->MyCharacterPointer() : 0;
			if(pPlayer)
			{
				CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
				if(pWeapon)
				{
					_curclip = pWeapon->Clip1();
					_maxclip = pWeapon->GetMaxClip1();
				}
				return Success;
			}
			return InvalidEntity;
		}

		obResult GetCurrentAmmo(const GameEntity _ent, int _ammotype, int &_cur, int &_max)
		{
			CBaseEntity *pEntity = EntityFromHandle(_ent);
			CBasePlayer *pPlayer = pEntity ? pEntity->MyCharacterPointer() : 0;
			if(pPlayer)
			{
				CFFPlayer *pFFPlayer = static_cast<CFFPlayer*>(pPlayer);
				switch(_ammotype)
				{
				case TF_AMMO_SHELLS:
					_cur = pFFPlayer->GetAmmoCount(AMMO_SHELLS);
					_max = pFFPlayer->GetMaxShells();
					break;
				case TF_AMMO_NAILS:
					_cur = pFFPlayer->GetAmmoCount(AMMO_NAILS);
					_max = pFFPlayer->GetMaxNails();
					break;
				case TF_AMMO_ROCKETS:
					_cur = pFFPlayer->GetAmmoCount(AMMO_ROCKETS);
					_max = pFFPlayer->GetMaxRockets();
					break;
				case TF_AMMO_CELLS:
					_cur = pFFPlayer->GetAmmoCount(AMMO_CELLS);
					_max = pFFPlayer->GetMaxCells();
					break;
				case TF_AMMO_MEDIKIT:
					_cur = -1;
					_max = -1;
					break;
				case TF_AMMO_DETPACK:
					_cur = pFFPlayer->GetAmmoCount(AMMO_DETPACK);
					_max = pFFPlayer->GetMaxDetpack();
					break;
				case TF_AMMO_GRENADE1:
					_cur = pFFPlayer->GetPrimaryGrenades();
					_max = 4;
					break;
				case TF_AMMO_GRENADE2:
					_cur = pFFPlayer->GetSecondaryGrenades();
					_max = 3;
					break;
				case -1:
					_cur = -1;
					_max = -1;
					break;
				default:
					return InvalidParameter;
				}

				return Success;
			}

			_cur = 0;
			_max = 0;

			return InvalidEntity;
		}

		int GetGameTime()
		{	
			return int(gpGlobals->curtime * 1000.0f);
		}

		void GetGoals()
		{
		}

		void GetPlayerInfo(obPlayerInfo &info)
		{
			for(int i = TEAM_BLUE; i <= TEAM_GREEN; ++i)
			{
				CFFTeam *pTeam = GetGlobalFFTeam(i);
				if(pTeam->GetTeamLimits()>0)
					info.m_AvailableTeams |= (1<<obUtilGetBotTeamFromGameTeam(i));
			}
			info.m_MaxPlayers = gpGlobals->maxClients;
			for(int i = 1; i <= gpGlobals->maxClients; ++i)
			{
				CBasePlayer	*pEnt = UTIL_PlayerByIndex(i);
				if(pEnt)
				{
					GameEntity ge = HandleFromEntity(pEnt);
					info.m_Players[i].m_Team = GetEntityTeam(ge);
					info.m_Players[i].m_Class = GetEntityClass(ge);
					info.m_Players[i].m_Controller = pEnt->IsBot()?obPlayerInfo::Bot:obPlayerInfo::Human;
				}
			}
		}

		obResult InterfaceSendMessage(const MessageHelper &_data, const GameEntity _ent)
		{
			CBaseEntity *pEnt = EntityFromHandle(_ent);
			CBasePlayer *pPlayer = pEnt ? pEnt->MyCharacterPointer() : 0;

#pragma warning(default: 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled
			switch(_data.GetMessageId())
			{
				///////////////////////
				// General Messages. //
				///////////////////////
			case GEN_MSG_ISALIVE:
				{
					OB_GETMSG(Msg_IsAlive);
					if(pMsg)
					{
						pMsg->m_IsAlive = pEnt && pEnt->IsAlive() && pEnt->GetHealth() > 0 ? True : False;
					}
					break;
				}
			case GEN_MSG_ISRELOADING:
				{
					OB_GETMSG(Msg_Reloading);
					if(pMsg)
					{
						pMsg->m_Reloading = pPlayer && pPlayer->IsPlayingGesture(ACT_GESTURE_RELOAD) ? True : False;
					}
					break;
				}
			case GEN_MSG_ISREADYTOFIRE:
				{
					OB_GETMSG(Msg_ReadyToFire);
					if(pMsg)
					{
						CBaseCombatWeapon *pWeapon = pPlayer ? pPlayer->GetActiveWeapon() : 0;
						pMsg->m_Ready = pWeapon && (pWeapon->m_flNextPrimaryAttack <= gpGlobals->curtime) ? True : False;
					}
					break;
				}
			case GEN_MSG_ISALLIED:
				{
					OB_GETMSG(Msg_IsAllied);
					if(pMsg)
					{
						CBaseEntity *pEntOther = EntityFromHandle(pMsg->m_TargetEntity);
						if(pEnt && pEntOther)
						{
							pMsg->m_IsAllied = g_pGameRules->PlayerRelationship(pEnt, pEntOther) != GR_NOTTEAMMATE ? True : False;
							if(pMsg->m_IsAllied && pEntOther->Classify() == CLASS_SENTRYGUN)
							{
								CFFSentryGun *pSentry = static_cast<CFFSentryGun*>(pEntOther);
								if(pSentry->IsMaliciouslySabotaged())
									pMsg->m_IsAllied = False;
							}
						}
					}
					break;
				}
			case GEN_MSG_GETEQUIPPEDWEAPON:
				{
					OB_GETMSG(WeaponStatus);
					if(pMsg)
					{
						CBaseCombatWeapon *pWeapon = pPlayer ? pPlayer->GetActiveWeapon() : 0;
						pMsg->m_WeaponId = pWeapon ? obUtilGetWeaponId(pWeapon->GetName()) : 0;
					}
					break;
				}
			case GEN_MSG_GETMOUNTEDWEAPON:
				{
					break;
				}
			case GEN_MSG_GETHEALTHARMOR:
				{
					OB_GETMSG(Msg_HealthArmor);
					if(pMsg)
					{
						//if(pPlayer)
						if(pEnt)
						{
							pMsg->m_CurrentHealth = pEnt->GetHealth();
							pMsg->m_MaxHealth = pEnt->GetMaxHealth();
							pMsg->m_CurrentArmor = pEnt->GetArmor();
							pMsg->m_MaxArmor = pEnt->GetMaxArmor();
						}						
					}
					break;
				}
			case GEN_MSG_GETFLAGSTATE:
				{
					OB_GETMSG(Msg_FlagState);
					if(pMsg)
					{
						if(pEnt && pEnt->Classify() == CLASS_INFOSCRIPT)
						{
							CFFInfoScript *pInfoScript = static_cast<CFFInfoScript*>(pEnt);
							if(pInfoScript->IsReturned())
								pMsg->m_FlagState = S_FLAG_AT_BASE;
							else if(pInfoScript->IsDropped())
								pMsg->m_FlagState = S_FLAG_DROPPED;
							else if(pInfoScript->IsCarried())
								pMsg->m_FlagState = S_FLAG_CARRIED;
							else if(pInfoScript->IsRemoved())
								pMsg->m_FlagState = S_FLAG_UNAVAILABLE;
							pMsg->m_Owner = HandleFromEntity(pEnt->GetOwnerEntity());
						}
					}
					break;
				}
			case GEN_MSG_GAMESTATE:
				{
					OB_GETMSG(Msg_GameState);
					if(pMsg)
					{
						CFFGameRules *pRules = FFGameRules();
						if(pRules)
						{
							pMsg->m_TimeLeft = (mp_timelimit.GetFloat() * 60.0f) - gpGlobals->curtime;
							if(pRules->HasGameStarted())
							{
								if(g_fGameOver && gpGlobals->curtime < pRules->m_flIntermissionEndTime)
									pMsg->m_GameState = GAME_STATE_INTERMISSION;
								else
									pMsg->m_GameState = GAME_STATE_PLAYING;
							}
							else
							{
								float flPrematch = pRules->GetRoundStart() + mp_prematch.GetFloat() * 60.0f;
								if( gpGlobals->curtime < flPrematch )
								{
									float flTimeLeft = flPrematch - gpGlobals->curtime;
									if(flTimeLeft < 10)
										pMsg->m_GameState = GAME_STATE_WARMUP_COUNTDOWN;
									else
										pMsg->m_GameState = GAME_STATE_WARMUP;
								}
								else
								{
									pMsg->m_GameState = GAME_STATE_WAITINGFORPLAYERS;
								}
							}
						}
					}
					break;
				}
			case GEN_MSG_GETWEAPONLIMITS:
				{
					WeaponLimits *pMsg = _data.Get<WeaponLimits>();
					if(pMsg)
						pMsg->m_Limited = False;
					break;
				}
			case GEN_MSG_GETMAXSPEED:
				{
					OB_GETMSG(Msg_PlayerMaxSpeed);
					if(pMsg && pPlayer)
					{
						pMsg->m_MaxSpeed = pPlayer->MaxSpeed();
					}
					break;
				}
			case GEN_MSG_ENTITYSTAT:
				{
					OB_GETMSG(Msg_EntityStat);
					if(pMsg)
					{
						if(pPlayer && !Q_strcmp(pMsg->m_StatName, "kills"))
							pMsg->m_Result = obUserData(pPlayer->FragCount());
						else if(pPlayer && !Q_strcmp(pMsg->m_StatName, "deaths"))
							pMsg->m_Result = obUserData(pPlayer->DeathCount());
						else if(pPlayer && !Q_strcmp(pMsg->m_StatName, "score"))
							pMsg->m_Result = obUserData(0); // TODO:
					}
					break;
				}
			case GEN_MSG_TEAMSTAT:
				{
					OB_GETMSG(Msg_TeamStat);
					if(pMsg)
					{
						CTeam *pTeam = GetGlobalTeam( obUtilGetGameTeamFromBotTeam(pMsg->m_Team) );
						if(pTeam)
						{
							if(!Q_strcmp(pMsg->m_StatName, "score"))
								pMsg->m_Result = obUserData(pTeam->GetScore());
							else if(!Q_strcmp(pMsg->m_StatName, "deaths"))
								pMsg->m_Result = obUserData(pTeam->GetDeaths());
						}
					}
					break;
				}
			case GEN_MSG_WPCHARGED:
				{
					OB_GETMSG(WeaponCharged);
					if(pMsg && pPlayer)
					{
						pMsg->m_IsCharged = True;
					}
					break;
				}
			case GEN_MSG_WPHEATLEVEL:
				{
					OB_GETMSG(WeaponHeatLevel);
					if(pMsg && pPlayer)
					{
						CBaseCombatWeapon *pWp = pPlayer->GetActiveWeapon();
						if(pWp)
						{
							pWp->GetHeatLevel(pMsg->m_FireMode, pMsg->m_CurrentHeat, pMsg->m_MaxHeat);
						}
					}
					break;
				}
			case GEN_MSG_ENTITYKILL:
				{
					break;
				}
			case GEN_MSG_SERVERCOMMAND:
				{
					OB_GETMSG(Msg_ServerCommand);
					if(pMsg && pMsg->m_Command[0] && sv_cheats->GetBool())
					{
						const char *cmd = pMsg->m_Command;
						while(*cmd && *cmd==' ')
							++cmd;
						if(cmd && *cmd)
						{
							engine->ServerCommand(UTIL_VarArgs("%s\n", cmd));
						}
					}
					break;
				}
			case GEN_MSG_PLAYSOUND:
				{
					struct LastSound { char m_SoundName[64]; };
					static LastSound m_LastPlayerSound[MAX_PLAYERS] = {};
					
					OB_GETMSG(Event_PlaySound);
					if(pPlayer)
						pPlayer->EmitSound(pMsg->m_SoundName);
					/*else
						FFLib::BroadcastSound(pMsg->m_SoundName);*/
					break;
				}
			case GEN_MSG_STOPSOUND:
				{
					OB_GETMSG(Event_StopSound);
					if(pPlayer)
						pPlayer->StopSound(pMsg->m_SoundName);
					/*else
					FFLib::BroadcastSound(pMsg->m_SoundName);*/
					break;
				}
			case GEN_MSG_SCRIPTEVENT:
				{
					OB_GETMSG(Event_ScriptEvent);
					
					CFFLuaSC hEvent;
					if(pMsg->m_Param1[0])
						hEvent.Push(pMsg->m_Param1);
					if(pMsg->m_Param2[0])
						hEvent.Push(pMsg->m_Param2);
					if(pMsg->m_Param3[0])
						hEvent.Push(pMsg->m_Param3);
					
					CBaseEntity *pEnt = NULL;
					if(pMsg->m_EntityName[0])
						pEnt = gEntList.FindEntityByName(NULL, pMsg->m_EntityName);
					_scriptman.RunPredicates_LUA(pEnt, &hEvent, pMsg->m_FunctionName);
					break;
				}
			case GEN_MSG_MOVERAT:
				{
					OB_GETMSG(Msg_MoverAt);
					if(pMsg)
					{
						Vector org(
							pMsg->m_Position[0],
							pMsg->m_Position[1],
							pMsg->m_Position[2]);
						Vector under(
							pMsg->m_Under[0],
							pMsg->m_Under[1],
							pMsg->m_Under[2]);

						trace_t tr;
						unsigned int iMask = MASK_PLAYERSOLID_BRUSHONLY;
						UTIL_TraceLine(org, under, iMask, NULL, COLLISION_GROUP_PLAYER_MOVEMENT, &tr);

						if(tr.DidHitNonWorldEntity() && 
							!tr.m_pEnt->IsPlayer() &&
							!tr.startsolid)
						{
							pMsg->m_Entity = HandleFromEntity(tr.m_pEnt);
						}
					}
					break;
				}
				//////////////////////////////////
				// Game specific messages next. //
				//////////////////////////////////
			case TF_MSG_GETBUILDABLES:
				{
					OB_GETMSG(TF_BuildInfo);
					if(pMsg)
					{
						CFFPlayer *pFFPlayer = static_cast<CFFPlayer*>(pPlayer);
						if(pFFPlayer)
						{
							CBaseAnimating *pSentry = pFFPlayer->GetSentryGun();
							pMsg->m_Sentry = pSentry ? HandleFromEntity(pSentry) : GameEntity();
							CBaseAnimating *pDispenser = pFFPlayer->GetDispenser();
							pMsg->m_Dispenser = pDispenser ? HandleFromEntity(pDispenser) : GameEntity();
							CBaseAnimating *pDetpack = pFFPlayer->GetDetpack();
							pMsg->m_Detpack = pDetpack ? HandleFromEntity(pDetpack) : GameEntity();
						}
					}
					break;
				}		
			case TF_MSG_PLAYERPIPECOUNT:
				{
					OB_GETMSG(TF_PlayerPipeCount);
					if(pMsg && pEnt)
					{
						int iNumPipes = 0;
						CBaseEntity *pPipe = 0;
						while((pPipe = gEntList.FindEntityByClassT(pPipe, CLASS_PIPEBOMB)) != NULL) 
						{
							if (pPipe->GetOwnerEntity() == pEnt)
								++iNumPipes;
						}

						pMsg->m_NumPipes = iNumPipes;
						pMsg->m_MaxPipes = 8;
					}
					break;
				}
			case TF_MSG_TEAMPIPEINFO:
				{
					OB_GETMSG(TF_TeamPipeInfo);
					if(pMsg)
					{
						pMsg->m_NumTeamPipes = 0;
						pMsg->m_NumTeamPipers = 0;
						pMsg->m_MaxPipesPerPiper = 8;
					}
					break;
				}
			case TF_MSG_CANDISGUISE:
				{
					OB_GETMSG(TF_DisguiseOptions);
					if(pMsg)
					{
						const int iCheckTeam = obUtilGetGameTeamFromBotTeam(pMsg->m_CheckTeam);
						for(int t = TEAM_BLUE; t <= TEAM_GREEN; ++t)
						{
							CFFTeam *pTeam = GetGlobalFFTeam(t);
							pMsg->m_Team[obUtilGetBotTeamFromGameTeam(t)] = 
								(pTeam && (pTeam->GetTeamLimits() != -1)) ? True : False;

							if(pTeam && (t == iCheckTeam))
							{
								for(int c = CLASS_SCOUT; c <= CLASS_CIVILIAN; ++c)
								{
									pMsg->m_Class[obUtilGetBotClassFromGameClass(c)] = 
										(pTeam->GetClassLimit(c) != -1) ? True : False;
								}
							}
						}
					}
					break;
				}
			case TF_MSG_DISGUISE:
				{
					OB_GETMSG(TF_Disguise);
					if(pMsg)
					{
						int iTeam = obUtilGetGameTeamFromBotTeam(pMsg->m_DisguiseTeam);
						int iClass = obUtilGetGameClassFromBotClass(pMsg->m_DisguiseClass);
						if(iTeam != TEAM_UNASSIGNED && iClass != -1)
						{
							serverpluginhelpers->ClientCommand(pPlayer->edict(), 
								UTIL_VarArgs("disguise %d %d", iTeam-1, iClass));
						}
						else
						{
							return InvalidParameter;
						}
					}
					break;
				}
			case TF_MSG_CLOAK:
				{
					OB_GETMSG(TF_FeignDeath);
					if(pMsg)
					{
						serverpluginhelpers->ClientCommand(pPlayer->edict(), 
							pMsg->m_Silent ? "scloak" : "cloak");
					}
					break;
				}
			case TF_MSG_LOCKPOSITION:
				{
					OB_GETMSG(TF_LockPosition);
					if(pMsg)
					{
						CBaseEntity *pEnt = EntityFromHandle(pMsg->m_TargetPlayer);
						if(pEnt)
						{
							if(pMsg->m_Lock == True)
								pEnt->AddFlag( FL_FROZEN );
							else
								pEnt->RemoveFlag( FL_FROZEN );
							pMsg->m_Succeeded = True;
						}
					}
					break;
				}
			case TF_MSG_HUDHINT:
				{
					OB_GETMSG(TF_HudHint);
					CBaseEntity *pEnt = EntityFromHandle(pMsg->m_TargetPlayer);
					pPlayer = pEnt ? pEnt->MyCharacterPointer() : 0;
					if(pMsg && ToFFPlayer(pPlayer))
					{					
						FF_HudHint(ToFFPlayer(pPlayer), 0, pMsg->m_Id, pMsg->m_Message);
					}
					break;
				}
			case TF_MSG_HUDMENU:
				{
					OB_GETMSG(TF_HudMenu);
					pEnt = EntityFromHandle(pMsg->m_TargetPlayer);
					pPlayer = pEnt ? pEnt->MyCharacterPointer() : 0;
					if(pMsg && ToFFPlayer(pPlayer))
					{
						KeyValues *kv = new KeyValues( "menu" );
						kv->SetString( "title", pMsg->m_Title );
						kv->SetInt( "level", pMsg->m_Level );
						kv->SetColor( "color", Color( pMsg->m_Color.r(), pMsg->m_Color.g(), pMsg->m_Color.b(), pMsg->m_Color.a() ));
						kv->SetInt( "time", pMsg->m_TimeOut );
						kv->SetString( "msg", pMsg->m_Message );

						for(int i = 0; i < 10; ++i)
						{
							if(pMsg->m_Option[i][0])
							{
								char num[10];
								Q_snprintf( num, sizeof(num), "%i", i );
								KeyValues *item1 = kv->FindKey( num, true );
								item1->SetString( "msg", pMsg->m_Option[i] );
								item1->SetString( "command", pMsg->m_Command[i] );
							}
						}

						DIALOG_TYPE type = DIALOG_MSG;
						switch(pMsg->m_MenuType)
						{
						case TF_HudMenu::GuiAlert:
							type = DIALOG_MSG; // just an on screen message
							break;
						case TF_HudMenu::GuiMenu:
							type = DIALOG_MENU; // an options menu
							break;
						case TF_HudMenu::GuiTextBox:
							type = DIALOG_TEXT; // a richtext dialog
							break;
						}					
						serverpluginhelpers->CreateMessage(pPlayer->edict(), type, kv, &g_ServerPlugin);
						kv->deleteThis();
					}
					break;
				}
			case TF_MSG_HUDTEXT:
				{
					OB_GETMSG(TF_HudText);
					pEnt = EntityFromHandle(pMsg->m_TargetPlayer);
					pPlayer = pEnt ? pEnt->MyCharacterPointer() : 0;
					if(pMsg && pMsg->m_Message[0])
					{
						int iDest = HUD_PRINTCONSOLE;
						switch(pMsg->m_MessageType)
						{
						case TF_HudText::MsgConsole:
							{
								iDest = HUD_PRINTCONSOLE;
								break;
							}
						case TF_HudText::MsgHudCenter:
							{
								iDest = HUD_PRINTCENTER;
								break;
							}
						}
						ClientPrint(pPlayer, iDest, pMsg->m_Message);
					}
					break;
				}
			default:
				{
					assert(0 && "Unknown Interface Message");
					return InvalidParameter;
				}
			}
#pragma warning(disable: 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled
			return Success;
		}

		bool DebugLine(const float _start[3], const float _end[3], const obColor &_color, float _time)
		{
			if(debugoverlay)
			{
				Vector vStart(_start[0], _start[1], _start[2]);
				Vector vEnd(_end[0], _end[1], _end[2]);
				debugoverlay->AddLineOverlay(vStart, 
					vEnd, 
					_color.r(), 
					_color.g(), 
					_color.b(), 
					false, 
					_time);
			}
			return true;
		}

		bool DebugRadius(const float _pos[3], const float _radius, const obColor &_color, float _time)
		{
			if(debugoverlay)
			{
				Vector pos(_pos[0], _pos[1], _pos[2] + 4);
				Vector start;
				Vector end;
				start.Init();
				end.Init();
				start.y += _radius;
				end.y -= _radius;

				float fStepSize = 180.0f / 8.0f;
				for(int i = 0; i < 8; ++i)
				{
					VectorYawRotate(start, fStepSize, start);
					VectorYawRotate(end, fStepSize, end);

					debugoverlay->AddLineOverlay(
						pos + start,
						pos + end,
						_color.r(), 
						_color.g(), 
						_color.b(), 
						false,
						_time);
				}
			}
			return true;
		}

		bool DebugPolygon(const obVec3 *_verts, const int _numverts, const obColor &_color, float _time, int _flags)
		{
			if(debugoverlay)
			{
				if(_numverts >= 3)
				{
					Vector 
						p1(_verts[0].x, _verts[0].y, _verts[0].z), 
						p2(_verts[1].x, _verts[1].y, _verts[1].z), 
						p3(_verts[2].x, _verts[2].y, _verts[2].z);

					debugoverlay->AddTriangleOverlay(p3, p2, p1, 
						_color.r(), 
						_color.g(), 
						_color.b(), 
						_color.a(), _flags&IEngineInterface::DR_NODEPTHTEST, _time);
					debugoverlay->AddTriangleOverlay(p1, p2, p3, 
						_color.r(), 
						_color.g(), 
						_color.b(), 
						_color.a(), _flags&IEngineInterface::DR_NODEPTHTEST, _time);

					for(int p = 3; p < _numverts; ++p)
					{
						p2 = p3;

						p3 = Vector(_verts[p].x, _verts[p].y, _verts[p].z);

						debugoverlay->AddTriangleOverlay(p3, p2, p1, 
							_color.r(), 
							_color.g(), 
							_color.b(), 
							_color.a(), _flags&IEngineInterface::DR_NODEPTHTEST, _time);
						debugoverlay->AddTriangleOverlay(p1, p2, p3, 
							_color.r(), 
							_color.g(), 
							_color.b(), 
							_color.a(), _flags&IEngineInterface::DR_NODEPTHTEST, _time);
					}
				}
			}
			return true;
		}

		void PrintError(const char *_error)
		{
			if(_error)
				Warning("%s\n", _error);
		}

		void PrintMessage(const char *_msg)
		{
			if(_msg)
				Msg("%s\n", _msg);
		}

		void PrintScreenText(const float _pos[3], float _duration, const obColor &_color, const char *_msg)
		{
			if(_msg)
			{
				if(_pos)
				{
					Vector vPosition(_pos[0],_pos[1],_pos[2]);		
					debugoverlay->AddTextOverlay(vPosition, _duration, _msg);
				}
				else
				{
					float fVertical = 0.75;

					// Handle newlines
					char buffer[1024] = {};
					Q_strncpy(buffer, _msg, 1024);
					char *pbufferstart = buffer;

					int iLength = Q_strlen(buffer);
					for(int i = 0; i < iLength; ++i)
					{
						if(buffer[i] == '\n' || buffer[i+1] == '\0')
						{
							buffer[i++] = 0;
							debugoverlay->AddScreenTextOverlay(0.3f, fVertical, _duration,
								_color.r(), _color.g(), _color.b(), _color.a(), pbufferstart);
							fVertical += 0.02f;
							pbufferstart = &buffer[i];
						}
					}
				}
			}
		}

		const char *GetMapName()
		{
			static char mapname[256] = {0};
			if(gpGlobals->mapname.ToCStr())
			{
				Q_snprintf( mapname, sizeof( mapname ), STRING( gpGlobals->mapname ) );
			}
			return mapname;
		}

		void GetMapExtents(AABB &_aabb)
		{
			memset(&_aabb, 0, sizeof(AABB));

			CWorld *world = GetWorldEntity();
			if(world)
			{
				Vector mins, maxs;
				world->GetWorldBounds(mins, maxs);

				for(int i = 0; i < 3; ++i)
				{
					_aabb.m_Mins[i] = mins[i];
					_aabb.m_Maxs[i] = maxs[i];
				}
			}
		}

		GameEntity EntityFromID(const int _gameId)
		{
			CBaseEntity *pEnt = CBaseEntity::Instance(_gameId);
			return HandleFromEntity(pEnt);
		}

		GameEntity EntityByName(const char *_name)
		{
			CBaseEntity *pEnt = _name ? gEntList.FindEntityByName(NULL, _name, NULL) : NULL;
			return HandleFromEntity(pEnt);
		}
		
		int IDFromEntity(const GameEntity _ent)
		{
			CBaseEntity *pEnt = EntityFromHandle(_ent);
			return pEnt ? ENTINDEX(pEnt->edict()) : -1;
		}

		bool DoesEntityStillExist(const GameEntity &_hndl)
		{
			return _hndl.IsValid() ? EntityFromHandle(_hndl) != NULL : false;
		}

		int GetAutoNavFeatures(AutoNavFeature *_feature, int _max)
		{
			Vector vForward, vRight, vUp;

			int iNumFeatures = 0;

			CBaseEntity *pEnt = gEntList.FirstEnt();
			while ( pEnt )
			{
				_feature[iNumFeatures].m_Type = 0;

				if(iNumFeatures >= _max)
					return iNumFeatures;

				Vector vPos = pEnt->GetAbsOrigin();
				AngleVectors(pEnt->GetAbsAngles(), &vForward, &vRight, &vUp);
				for(int j = 0; j < 3; ++j)
				{
					_feature[iNumFeatures].m_Position[j] = vPos[j];
					_feature[iNumFeatures].m_TargetPosition[j] = vPos[j];
					_feature[iNumFeatures].m_Facing[j] = vForward[j];

					_feature[iNumFeatures].m_Bounds.m_Mins[j] = 0.f;
					_feature[iNumFeatures].m_Bounds.m_Maxs[j] = 0.f;

					_feature[iNumFeatures].m_TargetBounds.m_Mins[j] = 0.f;
					_feature[iNumFeatures].m_TargetBounds.m_Maxs[j] = 0.f;
				}

				//////////////////////////////////////////////////////////////////////////

				if(FClassnameIs(pEnt,"info_player_coop") ||
					FClassnameIs(pEnt,"info_player_deathmatch") ||
					FClassnameIs(pEnt,"info_player_start") ||
					FClassnameIs(pEnt,"info_ff_teamspawn"))
				{
					_feature[iNumFeatures].m_Type = ENT_CLASS_GENERIC_PLAYERSTART;
				}
				else if(FClassnameIs(pEnt,"trigger_teleport"))
				{
					CBaseEntity *pTarget = pEnt->GetNextTarget();
					if(pTarget)
					{
						Vector vTargetPos = pTarget->GetAbsOrigin();
						for(int j = 0; j < 3; ++j)
						{
							_feature[iNumFeatures].m_TargetPosition[j] = vTargetPos[j];
						}
						_feature[iNumFeatures].m_Type = ENT_CLASS_GENERIC_TELEPORTER;
					}
				}
				else if(FClassnameIs(pEnt,"info_ladder"))
				{
					CInfoLadder *pLadder = dynamic_cast<CInfoLadder*>(pEnt);
					if(pLadder)
					{
						for(int j = 0; j < 3; ++j)
						{
							_feature[iNumFeatures].m_Bounds.m_Mins[j] = pLadder->mins[j];
							_feature[iNumFeatures].m_Bounds.m_Maxs[j] = pLadder->maxs[j];
						}
						_feature[iNumFeatures].m_Bounds.CenterBottom(_feature[iNumFeatures].m_Position);
						_feature[iNumFeatures].m_Bounds.CenterBottom(_feature[iNumFeatures].m_TargetPosition);
						_feature[iNumFeatures].m_Type = ENT_CLASS_GENERIC_LADDER;
					}
				}
				else if(FClassnameIs(pEnt,"info_ff_script"))
				{
					CFFInfoScript *pInfo = dynamic_cast<CFFInfoScript*>(pEnt);
					if(pInfo->GetBotGoalType() == Omnibot::kTrainerSpawn)
						_feature[iNumFeatures].m_Type = ENT_CLASS_GENERIC_PLAYERSTART;
				}

				if(_feature[iNumFeatures].m_Type != 0)
				{
					++iNumFeatures;
				}
				pEnt = gEntList.NextEnt(pEnt);
			}
			return iNumFeatures;
		}

		const char *GetGameName()
		{
			return "Halflife 2";
		}

		const char *GetModName()
		{
			return g_pGameRules ? g_pGameRules->GetGameDescription() : "unknown";
		}

		const char *GetModVers()
		{
			static char buffer[256];
			engine->GetGameDir(buffer, 256);
			return buffer;
		}

		const char *GetBotPath()
		{
			return Omnibot_GetLibraryPath();
		}

		const char *GetLogPath()
		{
			static char botPath[512] = {0};

			char buffer[512] = {0};
			filesystem->GetLocalPath(
				UTIL_VarArgs("%s/%s", omnibot_path.GetString(), "omnibot_ff.dll"), buffer, 512);

			Q_ExtractFilePath(buffer, botPath, 512);
			return botPath;
		}
	};

	//-----------------------------------------------------------------

	void omnibot_interface::OnDLLInit()
	{
		assert(!g_pEventHandler);
		if(!g_pEventHandler)
		{
			g_pEventHandler = new omnibot_eventhandler;
			g_pEventHandler->ExtractEvents();
			g_pEventHandler->RegisterEvents();
		}		
	}

	void omnibot_interface::OnDLLShutdown()
	{
		if(g_pEventHandler)
		{
			g_pEventHandler->UnRegisterEvents();
			delete g_pEventHandler;
			g_pEventHandler = 0;
		}		
	}

	//-----------------------------------------------------------------

	void omnibot_interface::OmnibotCommand() 
	{
		if(IsOmnibotLoaded())
		{
			Arguments args;
			for(int i = 0; i < engine->Cmd_Argc(); ++i)
			{
				Q_strncpy(args.m_Args[args.m_NumArgs++], engine->Cmd_Argv(i), Arguments::MaxArgLength);
			}
			g_BotFunctions.pfnBotConsoleCommand(args);
		}
		else
			Warning("Omni-bot Not Loaded\n");
	}

	void omnibot_interface::Trigger(CBaseEntity *_ent, CBaseEntity *_activator, const char *_tagname, const char *_action)
	{
		if(IsOmnibotLoaded())
		{
			if(g_BotFunctions.pfnBotSendTrigger)
			{
				TriggerInfo ti;
				ti.m_Entity = HandleFromEntity(_ent);
				ti.m_Activator = HandleFromEntity(_activator);
				Q_strncpy(ti.m_Action,_action,TriggerBufferSize);
				Q_strncpy(ti.m_TagName,_tagname,TriggerBufferSize);
				g_BotFunctions.pfnBotSendTrigger(ti);
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////

	class OmnibotEntityListener : public IEntityListener
	{
		virtual void OnEntityCreated( CBaseEntity *pEntity )
		{
			if(!IsOmnibotLoaded())
				Bot_Queue_EntityCreated(pEntity);
			else
				Bot_Event_EntityCreated(pEntity);
		}
		virtual void OnEntitySpawned( CBaseEntity *pEntity )
		{
		}
		virtual void OnEntityDeleted( CBaseEntity *pEntity )
		{
			Bot_Event_EntityDeleted(pEntity);
		}
	};

	OmnibotEntityListener gBotEntityListener;

	//////////////////////////////////////////////////////////////////////////
	// Interface Functions
	void omnibot_interface::LevelInit()
	{
		// done here because map loads before InitBotInterface is called.
		g_EntSerials.Reset();
	}
	bool omnibot_interface::InitBotInterface()
	{
		if(!omnibot_enable.GetBool())
		{
			Msg( "Omni-bot Currently Disabled. Re-enable with cvar omnibot_enable\n" );
			return false;
		}

		/*if( !gameLocal.isServer )
		return false;*/

		Msg("-------------- Omni-bot Init ----------------\n");

		// Look for the bot dll.
		const int BUF_SIZE = 1024;
		char botFilePath[BUF_SIZE] = {0};
		char botPath[BUF_SIZE] = {0};

		filesystem->GetLocalPath(
			UTIL_VarArgs("%s/%s", omnibot_path.GetString(), "omnibot_ff.dll"), botFilePath, BUF_SIZE);		
		Q_ExtractFilePath(botFilePath, botPath, BUF_SIZE);
		botPath[strlen(botPath)-1] = 0;
		Q_FixSlashes(botPath);

		g_InterfaceFunctions = new FFInterface;
		eomnibot_error err = Omnibot_LoadLibrary(FF_VERSION_LATEST, "omnibot_ff", Omnibot_FixPath(botPath));
		if(err == BOT_ERROR_NONE)
		{
			g_Started = false;
			gEntList.RemoveListenerEntity(&gBotEntityListener);
			gEntList.AddListenerEntity(&gBotEntityListener);
		}
		Msg( "---------------------------------------------\n" );
		return err == BOT_ERROR_NONE;
	}

	void omnibot_interface::ShutdownBotInterface()
	{
		gEntList.RemoveListenerEntity(&gBotEntityListener);
		if(IsOmnibotLoaded())
		{
			Msg( "------------ Omni-bot Shutdown --------------\n" );
			Notify_GameEnded(0);		
			g_BotFunctions.pfnBotShutdown();
			Omnibot_FreeLibrary();
			Msg( "Omni-bot Shut Down Successfully\n" );
			Msg( "---------------------------------------------\n" );
		}			

		// Temp fix?
		if( debugoverlay )
			debugoverlay->ClearAllOverlays();
	}

	void omnibot_interface::UpdateBotInterface()
	{
		VPROF_BUDGET( "Omni-bot::Update", _T("Omni-bot") );

		if(IsOmnibotLoaded())
		{
			static float serverGravity = 0.0f;
			if(serverGravity != sv_gravity.GetFloat())
			{
				Event_SystemGravity d = { -sv_gravity.GetFloat() };
				g_BotFunctions.pfnBotSendGlobalEvent(MessageHelper(GAME_GRAVITY, &d, sizeof(d)));
				serverGravity = sv_gravity.GetFloat();
			}
			static bool cheatsEnabled = false;
			if(sv_cheats->GetBool() != cheatsEnabled)
			{
				Event_SystemCheats d = { sv_cheats->GetBool()?True:False };
				g_BotFunctions.pfnBotSendGlobalEvent(MessageHelper(GAME_CHEATS, &d, sizeof(d)));
				cheatsEnabled = sv_cheats->GetBool();
			}
			//////////////////////////////////////////////////////////////////////////
			if(!engine->IsDedicatedServer())
			{
				for ( int i = 1; i <= gpGlobals->maxClients; i++ )
				{
					CBasePlayer *pPlayer = UTIL_PlayerByIndex(i);
					if (pPlayer && !pPlayer->IsBot() &&
						(pPlayer->GetObserverMode() != OBS_MODE_ROAMING &&
						pPlayer->GetObserverMode() != OBS_MODE_DEATHCAM))
					{
						CBasePlayer *pSpectatedPlayer = ToBasePlayer(pPlayer->GetObserverTarget());
						if(pSpectatedPlayer)
						{
							Omnibot::Notify_Spectated(pPlayer, pSpectatedPlayer);								
						}
					}
				}
			}
			//////////////////////////////////////////////////////////////////////////
			if(!g_Started)
			{
				Omnibot::Notify_GameStarted();
				g_Started = true;
			}
			
			if(g_DeferredGoalIndex > 0)
			{
				for(int i = 0; i < g_DeferredGoalIndex; ++i)
				{
					CBaseEntity *pGoalEnt = CBaseEntity::Instance(g_DeferredGoals[i].m_Entity);
					if(pGoalEnt)
					{
						MapGoalDef goaldef;
						goaldef.m_Entity = HandleFromEntity(pGoalEnt);
						goaldef.m_GoalType = g_DeferredGoals[i].m_GoalType;
						goaldef.m_Team = g_DeferredGoals[i].m_GoalTeam;
						V_strncpy(goaldef.m_TagName,g_DeferredGoals[i].m_GoalName,MapGoalDef::BufferSize);
						g_BotFunctions.pfnBotAddGoal(goaldef);
					}
				}
				g_DeferredGoalIndex = 0;
			}

			if(g_DeferredBotSpawnIndex > 0)
			{
				for(int i = 0; i < g_DeferredBotSpawnIndex; ++i)
				{
					SpawnBot(
						g_DeferredSpawn[i].m_Name, 
						g_DeferredSpawn[i].m_Team, 
						g_DeferredSpawn[i].m_Class,
						g_DeferredSpawn[i].m_SpawnPoint);
				}
				g_DeferredBotSpawnIndex = 0;
			}
			
			//////////////////////////////////////////////////////////////////////////
			// Register any pending entity updates.
			for(int i = 0; i < EntSerials::NumEntities; ++i)
			{
				if(g_EntSerials.IsIndexNew(i))
				{
					g_EntSerials.ClearIndexNew(i);

					CBaseEntity *pEnt = CBaseEntity::Instance(i);
					if(pEnt)
						Bot_Event_EntityCreated(pEnt);
				}
			}

			g_BotFunctions.pfnBotUpdate();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Message Helpers
	void Notify_GameStarted()
	{
		if(!IsOmnibotLoaded())
			return;

		g_BotFunctions.pfnBotSendGlobalEvent(MessageHelper(GAME_STARTGAME));
	}

	void Notify_GameEnded(int _winningteam)
	{
		if(!IsOmnibotLoaded())
			return;

		g_BotFunctions.pfnBotSendGlobalEvent(MessageHelper(GAME_ENDGAME));
	}

	void Notify_ChatMsg(CBasePlayer *_player, const char *_msg)
	{
		if(!IsOmnibotLoaded())
			return;

		Event_ChatMessage d;
		d.m_WhoSaidIt = HandleFromEntity(_player);
		Q_strncpy(d.m_Message, _msg ? _msg : "<unknown>",
			sizeof(d.m_Message) / sizeof(d.m_Message[0]));
		g_BotFunctions.pfnBotSendGlobalEvent(MessageHelper(PERCEPT_HEAR_GLOBALCHATMSG, &d, sizeof(d)));
	}

	void Notify_TeamChatMsg(CBasePlayer *_player, const char *_msg)
	{
		if(!IsOmnibotLoaded())
			return;

		Event_ChatMessage d;
		d.m_WhoSaidIt = HandleFromEntity(_player);
		Q_strncpy(d.m_Message, _msg ? _msg : "<unknown>",
			sizeof(d.m_Message) / sizeof(d.m_Message[0]));

		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer *pPlayer = UTIL_PlayerByIndex(i);

			// Check player classes on this player's team
			if (pPlayer && pPlayer->IsBot() && pPlayer->GetTeamNumber() == _player->GetTeamNumber())
			{
				g_BotFunctions.pfnBotSendEvent(pPlayer->entindex(), 
					MessageHelper(PERCEPT_HEAR_TEAMCHATMSG, &d, sizeof(d)));
			}
		}
	}

	void Notify_Spectated(CBasePlayer *_player, CBasePlayer *_spectated)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_spectated->IsBot())
			return;

		if(_spectated && _spectated->IsBot())
		{
			int iGameId = _spectated->entindex();
			Event_Spectated d = { _player->entindex()-1 };
			g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(MESSAGE_SPECTATED, &d, sizeof(d)));
		}
	}

	void Notify_AddWeapon(CBasePlayer *_player, const char *_item)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		int iWeaponId = obUtilGetWeaponId(_item);
		if(iWeaponId != TF_WP_NONE)
		{
			Event_AddWeapon d = { iWeaponId };
			g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(MESSAGE_ADDWEAPON, &d, sizeof(d)));
		}
		else
		{
			// Added newline since this was showing up
			Warning("Invalid Weapon Id: Notify_AddWeapon\n");
		}
	}

	void Notify_RemoveWeapon(CBasePlayer *_player, const char *_item)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		int iWeaponId = obUtilGetWeaponId(_item);		
		if(iWeaponId != TF_WP_NONE)
		{
			Event_RemoveWeapon d = { iWeaponId };
			g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(MESSAGE_REMOVEWEAPON, &d, sizeof(d)));
		}
		else
		{
			Warning("Invalid Weapon Id: Notify_RemoveWeapon\n");
		}
	}

	void Notify_RemoveAllItems(CBasePlayer *_player)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(MESSAGE_RESETWEAPONS));
	}

	void Notify_ClientConnected(CBasePlayer *_player, bool _isbot, int _team, int _class)
	{
		if(!IsOmnibotLoaded())
			return;

		int iGameId = _player->entindex();
		Event_SystemClientConnected d;
		d.m_GameId = iGameId;
		d.m_IsBot = _isbot ? True : False;
		d.m_DesiredTeam = _team;
		d.m_DesiredClass = _class;
		
		g_BotFunctions.pfnBotSendGlobalEvent(MessageHelper(GAME_CLIENTCONNECTED, &d, sizeof(d)));
	}

	void Notify_ClientDisConnected(CBasePlayer *_player)
	{
		if(!IsOmnibotLoaded())
			return;

		int iGameId = _player->entindex();
		Event_SystemClientDisConnected d = { iGameId };
		g_BotFunctions.pfnBotSendGlobalEvent(MessageHelper(GAME_CLIENTDISCONNECTED, &d, sizeof(d)));

	}

	void Notify_Hurt(CBasePlayer *_player, CBaseEntity *_attacker)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_TakeDamage d = { HandleFromEntity(_attacker) };
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(PERCEPT_FEEL_PAIN, &d, sizeof(d)));
	}

	void Notify_Death(CBasePlayer *_player, CBaseEntity *_attacker, const char *_weapon)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();

		Event_Death d;
		d.m_WhoKilledMe = HandleFromEntity(_attacker);
		Q_strncpy(d.m_MeansOfDeath, _weapon ? _weapon : "<unknown>", sizeof(d.m_MeansOfDeath));
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(MESSAGE_DEATH, &d, sizeof(d)));
	}

	void Notify_KilledSomeone(CBasePlayer *_player, CBaseEntity *_victim, const char *_weapon)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_KilledSomeone d;
		d.m_WhoIKilled = HandleFromEntity(_victim);
		Q_strncpy(d.m_MeansOfDeath, _weapon ? _weapon : "<unknown>", sizeof(d.m_MeansOfDeath));
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(MESSAGE_KILLEDSOMEONE, &d, sizeof(d)));

	}

	void Notify_ChangedTeam(CBasePlayer *_player, int _newteam)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_ChangeTeam d = { _newteam };
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(MESSAGE_CHANGETEAM, &d, sizeof(d)));

	}

	void Notify_ChangedClass(CBasePlayer *_player, int _oldclass, int _newclass)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_ChangeClass d = { _newclass };
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(MESSAGE_CHANGECLASS, &d, sizeof(d)));

	}

	void Notify_Build_MustBeOnGround(CBasePlayer *_player, int _buildable)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_BUILD_MUSTBEONGROUND));

	}

	void Notify_Build_CantBuild(CBasePlayer *_player, int _buildable)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		int iMsg = 0;
		switch(_buildable)
		{
		case FF_BUILD_DETPACK:
			iMsg = TF_MSG_DETPACK_CANTBUILD;
			break;
		case FF_BUILD_DISPENSER:
			iMsg = TF_MSG_DISPENSER_CANTBUILD;
			break;
		case FF_BUILD_SENTRYGUN:
			iMsg = TF_MSG_SENTRY_CANTBUILD;
			break;
		}
		if(iMsg != 0)
		{
			g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(iMsg));
		}

	}

	void Notify_Build_AlreadyBuilt(CBasePlayer *_player, int _buildable)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		int iMsg = 0;
		switch(_buildable)
		{
		case FF_BUILD_DETPACK:
			iMsg = TF_MSG_DETPACK_ALREADYBUILT;
			break;
		case FF_BUILD_DISPENSER:
			iMsg = TF_MSG_DISPENSER_ALREADYBUILT;
			break;
		case FF_BUILD_SENTRYGUN:
			iMsg = TF_MSG_SENTRY_ALREADYBUILT;
			break;
		}
		if(iMsg != 0)
		{
			g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(iMsg));
		}

	}

	void Notify_Build_NotEnoughAmmo(CBasePlayer *_player, int _buildable)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		int iMsg = 0;
		switch(_buildable)
		{
		case FF_BUILD_DETPACK:
			iMsg = TF_MSG_DETPACK_NOTENOUGHAMMO;
			break;
		case FF_BUILD_DISPENSER:
			iMsg = TF_MSG_DISPENSER_NOTENOUGHAMMO;
			break;
		case FF_BUILD_SENTRYGUN:
			iMsg = TF_MSG_SENTRY_NOTENOUGHAMMO;
			break;
		}
		if(iMsg != 0)
		{
			g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(iMsg));
		}
	}

	void Notify_Build_BuildCancelled(CBasePlayer *_player, int _buildable)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		int iMsg = 0;
		switch(_buildable)
		{
		case FF_BUILD_DETPACK:
			iMsg = TF_MSG_DETPACK_BUILDCANCEL;
			break;
		case FF_BUILD_DISPENSER:
			iMsg = TF_MSG_DISPENSER_BUILDCANCEL;
			break;
		case FF_BUILD_SENTRYGUN:
			iMsg = TF_MSG_SENTRY_BUILDCANCEL;
			break;
		}
		if(iMsg != 0)
		{
			g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(iMsg));
		}
	}

	void Notify_CantDisguiseAsTeam(CBasePlayer *_player, int _disguiseTeam)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_CantDisguiseTeam_TF d = { obUtilGetBotTeamFromGameTeam(_disguiseTeam) };
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_CANTDISGUISE_AS_TEAM, &d, sizeof(d)));
	}

	void Notify_CantDisguiseAsClass(CBasePlayer *_player, int _disguiseClass)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_CantDisguiseClass_TF d = { obUtilGetBotClassFromGameClass(_disguiseClass) };
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_CANTDISGUISE_AS_CLASS, &d, sizeof(d)));
	}

	void Notify_Disguising(CBasePlayer *_player, int _disguiseTeam, int _disguiseClass)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_Disguise_TF d;
		d.m_ClassId = _disguiseClass;
		d.m_TeamId = _disguiseTeam;
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_DISGUISING, &d, sizeof(d)));
	}

	void Notify_Disguised(CBasePlayer *_player, int _disguiseTeam, int _disguiseClass)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_Disguise_TF d;
		d.m_ClassId = _disguiseClass;
		d.m_TeamId = _disguiseTeam;
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_DISGUISING, &d, sizeof(d)));
	}

	void Notify_DisguiseLost(CBasePlayer *_player)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_DISGUISE_LOST));
	}

	void Notify_UnCloaked(CBasePlayer *_player)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_UNCLOAKED));
	}

	void Notify_CantCloak(CBasePlayer *_player)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_CANT_CLOAK));
	}

	void Notify_Cloaked(CBasePlayer *_player)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_CLOAKED));
	}

	void Notify_RadarDetectedEnemy(CBasePlayer *_player, CBaseEntity *_ent)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_RadarUpdate_TF d = { HandleFromEntity(_ent) };
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_RADAR_DETECT_ENEMY, &d, sizeof(d)));
	}

	void Notify_RadioTagUpdate(CBasePlayer *_player, CBaseEntity *_ent)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_RadarUpdate_TF d = { HandleFromEntity(_ent) };
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_RADIOTAG_UPDATE, &d, sizeof(d)));
	}

	void Notify_BuildableDamaged(CBasePlayer *_player, int _type, CBaseEntity *_buildEnt)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iMsg = 0;
		switch(_type)
		{
		case CLASS_DISPENSER:
			iMsg = TF_MSG_DISPENSER_DAMAGED;
			break;
		case CLASS_SENTRYGUN:
			iMsg = TF_MSG_SENTRY_DAMAGED;
			break;
		default:
			return;
		}			

		if(iMsg != 0)
		{
			int iGameId = _player->entindex();
			Event_BuildableDamaged_TF d = { HandleFromEntity(_buildEnt) };
			g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(iMsg, &d, sizeof(d)));
		}
	}

	void Notify_DispenserBuilding(CBasePlayer *_player, CBaseEntity *_buildEnt)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_DispenserBuilding_TF d = { HandleFromEntity(_buildEnt) };
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_DISPENSER_BUILDING, &d, sizeof(d)));
	}

	void Notify_DispenserBuilt(CBasePlayer *_player, CBaseEntity *_buildEnt)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_DispenserBuilt_TF d = { HandleFromEntity(_buildEnt) };
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_DISPENSER_BUILT, &d, sizeof(d)));
	}

	void Notify_DispenserEnemyUsed(CBasePlayer *_player, CBaseEntity *_enemyUser)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_DispenserEnemyUsed_TF d = { HandleFromEntity(_enemyUser) };
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_DISPENSER_ENEMYUSED, &d, sizeof(d)));
	}

	void Notify_DispenserDestroyed(CBasePlayer *_player, CBaseEntity *_attacker)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_BuildableDestroyed_TF d = { HandleFromEntity(_attacker) };
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_DISPENSER_DESTROYED, &d, sizeof(d)));
	}

	void Notify_SentryUpgraded(CBasePlayer *_player, int _level)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_SentryUpgraded_TF d = { _level };
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_SENTRY_UPGRADED, &d, sizeof(d)));
	}

	void Notify_SentryBuilding(CBasePlayer *_player, CBaseEntity *_buildEnt)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_SentryBuilding_TF d = { HandleFromEntity(_buildEnt) };
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_SENTRY_BUILDING, &d, sizeof(d)));
	}

	void Notify_SentryBuilt(CBasePlayer *_player, CBaseEntity *_buildEnt)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_SentryBuilt_TF d = { HandleFromEntity(_buildEnt) };
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_SENTRY_BUILT, &d, sizeof(d)));
	}
	
	void Notify_SentryDestroyed(CBasePlayer *_player, CBaseEntity *_attacker)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_BuildableDestroyed_TF d = { HandleFromEntity(_attacker) };
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_SENTRY_DESTROYED, &d, sizeof(d)));
	}

	void Notify_SentrySpottedEnemy(CBasePlayer *_player)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_SentrySpotEnemy_TF d = { GameEntity() }; // TODO
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_SENTRY_SPOTENEMY, &d, sizeof(d)));
	}

	void Notify_SentryAimed(CBasePlayer *_player, CBaseEntity *_buildEnt, const Vector &_dir)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_SentryAimed_TF d;
		d.m_Sentry = HandleFromEntity(_buildEnt);
		d.m_Direction[0] = _dir[0];
		d.m_Direction[1] = _dir[1];
		d.m_Direction[2] = _dir[2];
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_SENTRY_AIMED, &d, sizeof(d)));
	}

	void Notify_DetpackBuilding(CBasePlayer *_player, CBaseEntity *_buildEnt)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_DetpackBuilding_TF d = { HandleFromEntity(_buildEnt) };
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_DETPACK_BUILDING, &d, sizeof(d)));
	}

	void Notify_DetpackBuilt(CBasePlayer *_player, CBaseEntity *_buildEnt)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_DetpackBuilt_TF d = { HandleFromEntity(_buildEnt) };
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_DETPACK_BUILT, &d, sizeof(d)));
	}

	void Notify_DetpackDetonated(CBasePlayer *_player)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_DETPACK_DETONATED));
	}

	void Notify_DispenserSabotaged(CBasePlayer *_player, CBaseEntity *_saboteur)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_BuildableSabotaged_TF d = { HandleFromEntity(_saboteur) };
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_SABOTAGED_DISPENSER, &d, sizeof(d)));
	}

	void Notify_SentrySabotaged(CBasePlayer *_player, CBaseEntity *_saboteur)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		Event_BuildableSabotaged_TF d = { HandleFromEntity(_saboteur) };
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_SABOTAGED_SENTRY, &d, sizeof(d)));
	}

	void Notify_DispenserDetonated(CBasePlayer *_player)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_DISPENSER_DETONATED));
	}

	void Notify_DispenserDismantled(CBasePlayer *_player)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_DISPENSER_DISMANTLED));
	}

	void Notify_SentryDetonated(CBasePlayer *_player)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_SENTRY_DETONATED));
	}

	void Notify_SentryDismantled(CBasePlayer *_player)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		int iGameId = _player->entindex();
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_SENTRY_DISMANTLED));
	}

	void Notify_PlayerShoot(CBasePlayer *_player, int _weaponId, CBaseEntity *_projectile)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;
		
		//////////////////////////////////////////////////////////////////////////
		// send the created event if not sent already
		if(_projectile && g_EntSerials.IsIndexNew(_projectile->entindex()))
		{
			g_EntSerials.ClearIndexNew(_projectile->entindex());
			Bot_Event_EntityCreated(_projectile);
		}
		//////////////////////////////////////////////////////////////////////////

		int iGameId = _player->entindex();
		Event_WeaponFire d = { };
		d.m_WeaponId = _weaponId;
		d.m_Projectile = HandleFromEntity(_projectile);
		d.m_FireMode = Primary;
		g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(ACTION_WEAPON_FIRE, &d, sizeof(d)));
	}

	void Notify_PlayerUsed(CBasePlayer *_player, CBaseEntity *_entityUsed)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		CBasePlayer *pUsedPlayer = ToBasePlayer(_entityUsed);
		if(pUsedPlayer && pUsedPlayer->IsBot())
		{
			int iGameId = pUsedPlayer->entindex();
			Event_PlayerUsed d = { HandleFromEntity(_player) };
			g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(PERCEPT_FEEL_PLAYER_USE, &d, sizeof(d)));
		}
	}

	void Notify_GotSpannerArmor(CBasePlayer *_target, CBasePlayer *_engy, int _before, int _after)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_target->IsBot())
			return;

		if(_target && _target->IsBot())
		{
			int iGameId = _target->entindex();
			Event_GotEngyArmor d = { HandleFromEntity(_engy), _before, _after };
			g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_GOT_ENGY_ARMOR, &d, sizeof(d)));
		}
	}

	void Notify_GaveSpannerArmor(CBasePlayer *_engy, CBasePlayer *_target, int _before, int _after)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_engy->IsBot())
			return;

		if(_engy && _engy->IsBot())
		{
			int iGameId = _target->entindex();
			Event_GaveEngyArmor d = { HandleFromEntity(_target), _before, _after };
			g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_GAVE_ENGY_ARMOR, &d, sizeof(d)));
		}
	}

	void Notify_GotMedicHealth(CBasePlayer *_target, CBasePlayer *_medic, int _before, int _after)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_target->IsBot())
			return;

		if(_target && _target->IsBot())
		{
			int iGameId = _target->entindex();
			Event_GotMedicHealth d = { HandleFromEntity(_medic), _before, _after };
			g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_GOT_MEDIC_HEALTH, &d, sizeof(d)));
		}
	}

	void Notify_GaveMedicHealth(CBasePlayer *_medic, CBasePlayer *_target, int _before, int _after)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_medic->IsBot())
			return;

		if(_medic && _medic->IsBot())
		{
			int iGameId = _target->entindex();
			Event_GaveMedicHealth d = { HandleFromEntity(_target), _before, _after };
			g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_GAVE_MEDIC_HEALTH, &d, sizeof(d)));
		}
	}

	void Notify_Infected(CBasePlayer *_target, CBasePlayer *_infector)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_target->IsBot())
			return;

		if(_target && _target->IsBot())
		{
			int iGameId = _target->entindex();
			Event_Infected d = { HandleFromEntity(_infector) };
			g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_INFECTED, &d, sizeof(d)));
		}
	}

	void Notify_Cured(CBasePlayer *_curee, CBasePlayer *_curer)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_curee->IsBot())
			return;

		if(_curee && _curee->IsBot())
		{
			int iGameId = _curee->entindex();
			Event_Cured d = { HandleFromEntity(_curer) };
			g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_CURED, &d, sizeof(d)));
		}
	}

	void Notify_BurnLevel(CBasePlayer *_target, CBasePlayer *_burner, int _burnlevel)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_target->IsBot())
			return;

		if(_target && _target->IsBot())
		{
			int iGameId = _target->entindex();
			Event_Burn d = { HandleFromEntity(_burner), _burnlevel };
			g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_BURNLEVEL, &d, sizeof(d)));
		}
	}

	void Notify_GotDispenserAmmo(CBasePlayer *_player)
	{
		if(!IsOmnibotLoaded())
			return;
		if(!_player->IsBot())
			return;

		if(_player)
		{
			int iGameId = _player->entindex();
			g_BotFunctions.pfnBotSendEvent(iGameId, MessageHelper(TF_MSG_GOT_DISPENSER_AMMO));
		}
	}

	void Notify_Sound(CBaseEntity *_source, int _sndtype, const char *_name)
	{
		if (IsOmnibotLoaded())
		{
			Event_Sound d = {};
			d.m_Source = HandleFromEntity(_source);
			d.m_SoundType = _sndtype;
			Vector v = _source->GetAbsOrigin();
			d.m_Origin[0] = v[0];
			d.m_Origin[1] = v[1];
			d.m_Origin[2] = v[2];
			Q_strncpy(d.m_SoundName, _name ? _name : "<unknown>", sizeof(d.m_SoundName) / sizeof(d.m_SoundName[0]));
			g_BotFunctions.pfnBotSendGlobalEvent(MessageHelper(GAME_SOUND, &d, sizeof(d)));
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void Notify_GoalInfo(CBaseEntity *_entity, int _type, int _teamflags)
	{
		BotGoalInfo gi;

		//////////////////////////////////////////////////////////////////////////
		const int iAllTeams = (1<<TF_TEAM_BLUE)|(1<<TF_TEAM_RED)|(1<<TF_TEAM_YELLOW)|(1<<TF_TEAM_GREEN);
		gi.m_GoalTeam = _teamflags;
		//////////////////////////////////////////////////////////////////////////

		if(gi.m_GoalTeam != 0 || _type == Omnibot::kTrainerSpawn)
		{
			gi.m_Entity = _entity->edict();
			const char *pName = _entity->GetName();
			Q_strncpy(gi.m_GoalName, pName ? pName : "", sizeof(gi.m_GoalName));

			switch(_type)
			{
			case Omnibot::kBackPack_Grenades:
				{
					Bot_Queue_EntityCreated(_entity);
					return;
				}
			case Omnibot::kBackPack_Health:
				{
					Bot_Queue_EntityCreated(_entity);
					return;
				}
			case Omnibot::kBackPack_Armor:
				{
					Bot_Queue_EntityCreated(_entity);
					return;
				}
			case Omnibot::kBackPack_Ammo:
				{
					Bot_Queue_EntityCreated(_entity);
					return;
				}
			case Omnibot::kFlag:
				{
					gi.m_GoalType = GOAL_CTF_FLAG;					
					break;
				}
			case Omnibot::kFlagCap:
				{
					gi.m_GoalTeam ^= iAllTeams;
					gi.m_GoalType = GOAL_CTF_FLAGCAP;
					break;
				}
			case Omnibot::kTrainerSpawn:
				{
					gi.m_GoalType = GOAL_TRAININGSPAWN;
					break;
				}
			case Omnibot::kHuntedEscape:
				{
					gi.m_GoalType = TF_GOAL_HUNTEDESCAPE;
					break;
				}
			default:
				return;
			}

			if(g_DeferredGoalIndex < MAX_DEFERRED_GOALS-1)
			{
				g_DeferredGoals[g_DeferredGoalIndex++] = gi;
			}
			else
			{
				g_InterfaceFunctions->PrintError("Omni-bot: Out of deferred goal slots!");
			}
		}
		else
		{
			g_InterfaceFunctions->PrintError("Invalid Goal Entity");
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void Notify_ItemRemove(CBaseEntity *_entity)
	{
		if(!IsOmnibotLoaded())
			return;

		omnibot_interface::Trigger(_entity,NULL,_entity->GetName(),"item_removed");
	}
	void Notify_ItemRestore(CBaseEntity *_entity)
	{
		if(!IsOmnibotLoaded())
			return;

		omnibot_interface::Trigger(_entity,NULL,_entity->GetName(),"item_restored");
	}
	void Notify_ItemDropped(CBaseEntity *_entity)
	{
		if(!IsOmnibotLoaded())
			return;

		omnibot_interface::Trigger(_entity,NULL,_entity->GetName(),"item_dropped");
	}
	void Notify_ItemPickedUp(CBaseEntity *_entity, CBaseEntity *_whodoneit)
	{
		if(!IsOmnibotLoaded())
			return;

		omnibot_interface::Trigger(_entity,_whodoneit,_entity->GetName(),"item_pickedup");
	}
	void Notify_ItemRespawned(CBaseEntity *_entity)
	{
		if(!IsOmnibotLoaded())
			return;

		omnibot_interface::Trigger(_entity,NULL,_entity->GetName(),"item_respawned");
	}
	void Notify_ItemReturned(CBaseEntity *_entity)
	{
		if(!IsOmnibotLoaded())
			return;

		omnibot_interface::Trigger(_entity,NULL,_entity->GetName(),"item_returned");
	}
	void Notify_FireOutput(const char *_entityname, const char *_output)
	{
		if(!IsOmnibotLoaded())
			return;

		CBaseEntity *pEnt = _entityname ? gEntList.FindEntityByName(NULL, _entityname, NULL) : NULL;
		omnibot_interface::Trigger(pEnt,NULL,_entityname,_output);
	}
	void BotSendTriggerEx(const char *_entityname, const char *_action)
	{
		if(!IsOmnibotLoaded())
			return;

		omnibot_interface::Trigger(NULL,NULL,_entityname,_action);
	}
	void SendBotSignal(const char *_signal)
	{
		if(!IsOmnibotLoaded())
			return;

		Event_ScriptSignal d;
		memset(&d, 0, sizeof(d));
		Q_strncpy(d.m_SignalName, _signal, sizeof(d.m_SignalName));
		g_BotFunctions.pfnBotSendGlobalEvent(MessageHelper(GAME_SCRIPTSIGNAL, &d, sizeof(d)));
	}

	void SpawnBotAsync(const char *_name, int _team, int _class, CFFInfoScript *_spawnpoint)
	{
		if(g_DeferredBotSpawnIndex < MAX_PLAYERS-1)
		{
			Q_strncpy(g_DeferredSpawn[g_DeferredBotSpawnIndex].m_Name, _name?_name:"NoName", 64);
			g_DeferredSpawn[g_DeferredBotSpawnIndex].m_Team = _team;
			g_DeferredSpawn[g_DeferredBotSpawnIndex].m_Class = _class;
			g_DeferredSpawn[g_DeferredBotSpawnIndex].m_SpawnPoint = _spawnpoint;
			++g_DeferredBotSpawnIndex;
		}
	}

	void SpawnBot(const char *_name, int _team, int _class, CFFInfoScript *_spawnpoint)
	{
		if(!IsOmnibotLoaded())
			return;

		edict_t *pEdict = engine->CreateFakeClient( _name?_name:"NoName" );
		if (!pEdict)
		{
			g_InterfaceFunctions->PrintError("Unable to Add Bot!");
			return;
		}

		// Allocate a player entity for the bot, and call spawn
		CBasePlayer *pPlayer = ((CBasePlayer*)CBaseEntity::Instance( pEdict ));
		CFFPlayer *pFFPlayer = ToFFPlayer(pPlayer);
		ASSERT(pFFPlayer);
		if(pFFPlayer)
			pFFPlayer->m_SpawnPointOverride = _spawnpoint;

		pPlayer->ClearFlags();
		pPlayer->AddFlag( FL_CLIENT | FL_FAKECLIENT );

		pPlayer->ChangeTeam( TEAM_UNASSIGNED );
		pPlayer->RemoveAllItems( true );
		pPlayer->Spawn();
	
		Omnibot::Notify_ClientConnected(
			pPlayer, 
			true, 
			obUtilGetBotTeamFromGameTeam(_team),
			_class>0?obUtilGetBotClassFromGameClass(_class):RANDOM_CLASS);
	}
	//////////////////////////////////////////////////////////////////////////

	void Bot_Queue_EntityCreated(CBaseEntity *pEnt)
	{
		if(pEnt)
			g_EntSerials.QueueForIndex(ENTINDEX(pEnt));
	}

	void Bot_Event_EntityCreated(CBaseEntity *pEnt)
	{
		if(pEnt && IsOmnibotLoaded())
		{
			// Get common properties.
			GameEntity ent = HandleFromEntity(pEnt);
			int iClass = g_InterfaceFunctions->GetEntityClass(ent);
			if(iClass)
			{
				Event_EntityCreated d;

				int index = ENTINDEX(pEnt);
				g_EntSerials.AllocForIndex(index);

				d.m_Entity = GameEntity(index, g_EntSerials.SerialForIndex(index));
				d.m_EntityClass = iClass;
				g_InterfaceFunctions->GetEntityCategory(ent, d.m_EntityCategory);
				g_BotFunctions.pfnBotSendGlobalEvent(MessageHelper(GAME_ENTITYCREATED, &d, sizeof(d)));
			}
		}
	}

	void Bot_Event_EntityDeleted(CBaseEntity *pEnt)
	{
		if(pEnt && IsOmnibotLoaded())
		{
			Event_EntityDeleted d;

			int index = ENTINDEX(pEnt);
			d.m_Entity = GameEntity(index, g_EntSerials.SerialForIndex(index));
			g_BotFunctions.pfnBotSendGlobalEvent(MessageHelper(GAME_ENTITYDELETED, &d, sizeof(d)));
			g_EntSerials.FreeForIndex(index);
		}
	}

};

void CFFPlayer::SendBotMessage(const char *_msg, const char *_d1, const char *_d2, const char *_d3)
{
	if(!IsBot() || !IsOmnibotLoaded())
		return;

	Omnibot::Event_ScriptMessage d;
	memset(&d, 0, sizeof(d));
	Q_strncpy(d.m_MessageName, _msg, sizeof(d.m_MessageName));
	if(_d1) Q_strncpy(d.m_MessageData1, _d1, sizeof(d.m_MessageData1));
	if(_d2) Q_strncpy(d.m_MessageData2, _d2, sizeof(d.m_MessageData2));
	if(_d3) Q_strncpy(d.m_MessageData3, _d3, sizeof(d.m_MessageData3));
	g_BotFunctions.pfnBotSendEvent(entindex(), MessageHelper(MESSAGE_SCRIPTMSG, &d, sizeof(d)));
}

void CFFSentryGun::SendStatsToBot( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::SendStatsTobot", VPROF_BUDGETGROUP_FF_BUILDABLE );

	if(!IsOmnibotLoaded())
		return;

	CFFPlayer *pOwner = static_cast<CFFPlayer *>( m_hOwner.Get() );
	if (pOwner && pOwner->IsBot()) 
	{
		int iGameId = pOwner->entindex();

		const Vector &vPos = GetAbsOrigin();
		QAngle viewAngles = EyeAngles();

		Vector vFacing;
		AngleVectors(viewAngles, &vFacing, 0, 0);		

		Omnibot::Event_SentryStatus_TF d;
		d.m_Entity = Omnibot::HandleFromEntity(this);
		d.m_Health = m_iHealth;
		d.m_MaxHealth = m_iMaxHealth;
		d.m_Shells[0] = m_iShells;
		d.m_Shells[1] = m_iMaxShells;
		d.m_Rockets[0] = m_iRockets;
		d.m_Rockets[1] = m_iMaxRockets;
		d.m_Level = m_iLevel;
		d.m_Position[0] = vPos.x;
		d.m_Position[1] = vPos.y;
		d.m_Position[2] = vPos.z;
		d.m_Facing[0] = vFacing.x;
		d.m_Facing[1] = vFacing.y;
		d.m_Facing[2] = vFacing.z;
		g_BotFunctions.pfnBotSendEvent(iGameId, 
			MessageHelper(Omnibot::TF_MSG_SENTRY_STATS, &d, sizeof(d)));
	}
}

void CFFDispenser::SendStatsToBot()
{
	VPROF_BUDGET( "CFFDispenser::SendStatsToBot", VPROF_BUDGETGROUP_FF_BUILDABLE );

	if(!IsOmnibotLoaded())
		return;

	CFFPlayer *pOwner = static_cast<CFFPlayer*>(m_hOwner.Get());
	if(pOwner && pOwner->IsBot())
	{
		int iGameId = pOwner->entindex();

		const Vector &vPos = GetAbsOrigin();
		QAngle viewAngles = EyeAngles();

		Vector vFacing;
		AngleVectors(viewAngles, &vFacing, 0, 0);		

		Omnibot::Event_DispenserStatus_TF d;
		d.m_Entity = Omnibot::HandleFromEntity(this);
		d.m_Health = m_iHealth;
		d.m_Shells = m_iShells;
		d.m_Nails = m_iNails;
		d.m_Rockets = m_iRockets;
		d.m_Cells = m_iCells;
		d.m_Armor = m_iArmor;
		d.m_Position[0] = vPos.x;
		d.m_Position[1] = vPos.y;
		d.m_Position[2] = vPos.z;
		d.m_Facing[0] = vFacing.x;
		d.m_Facing[1] = vFacing.y;
		d.m_Facing[2] = vFacing.z;
		g_BotFunctions.pfnBotSendEvent(iGameId, 
			MessageHelper(Omnibot::TF_MSG_DISPENSER_STATS, &d, sizeof(d)));
	}
}
