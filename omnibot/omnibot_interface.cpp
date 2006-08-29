//////////////////////////////////////////////////////////////////////////
// Bot-Related Includes
#include "cbase.h"
#include "ammodef.h"
#include "in_buttons.h"
#include "playerinfomanager.h"
#include "filesystem.h"
#include "ai_basenpc.h"
#include "Color.h"
#include "world.h"
#include "ff_item_flag.h"

#include "ff_utils.h"
#include "ff_gamerules.h"
extern ConVar mp_prematch;

#ifdef _WIN32
#define WIN32
#endif

// Mirv: Just added this to stop all the redefinition warnings whenever i do a full recompile
#pragma warning(disable: 4005)

#include "omnibot_interface.h"
#include "omnibot_eventhandler.h"

#include <vector>

typedef struct
{
	float	start[3];
	float	end[3];
	Omnibot::obColor	color;
	float	dist;
	bool	drawme;
	char	type;
} debugLines_t;

std::vector<debugLines_t> g_DebugLines;
std::vector<debugLines_t> g_BlockableDebugLines;

ConVar	omnibot_enable( "omnibot_enable", "1", FCVAR_ARCHIVE | FCVAR_PROTECTED);
ConVar	omnibot_path( "omnibot_path", "omni-bot", FCVAR_ARCHIVE | FCVAR_PROTECTED);
ConVar	omnibot_nav( "omnibot_nav", "1", FCVAR_ARCHIVE | FCVAR_PROTECTED);
ConVar	omnibot_debug( "omnibot_debug", "0", FCVAR_ARCHIVE | FCVAR_PROTECTED);

#define OMNIBOT_MODNAME "Fortress Forever"

extern IServerPluginHelpers *serverpluginhelpers;

//////////////////////////////////////////////////////////////////////////

struct BotGoalInfo
{
	char	m_GoalName[64];
	int		m_GoalType;
	int		m_GoalTeam;
	edict_t *m_Edict;
};

const int MAX_DEFERRED_GOALS = 32;
int	g_DeferredGoalIndex = 0;
BotGoalInfo g_DeferredGoals[MAX_DEFERRED_GOALS] = {};
bool g_ReadyForGoals = false;

//////////////////////////////////////////////////////////////////////////

class CSkeletonServerPlugin : public IServerPluginCallbacks
{
public:
	CSkeletonServerPlugin() {}
	~CSkeletonServerPlugin() {}

	// IServerPluginCallbacks methods
	virtual bool			Load(	CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory )
	{
		return true;
	}
	virtual void			Unload( void ) {}
	virtual void			Pause( void ) {}
	virtual void			UnPause( void ) {}
	virtual const char     *GetPluginDescription( void ) { return "Blah"; }      
	virtual void			LevelInit( char const *pMapName ) {}
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

CSkeletonServerPlugin g_ServerPlugin;

//////////////////////////////////////////////////////////////////////////


namespace Omnibot
{
#include "TF_Messages.h"

#define OB_snprintf Q_snprintf
#include "BotExports.h"


	int s_NextUpdateTime = 0;
	float g_NextWpUpdate = 0.0f;

	CON_COMMAND( bot, "Omni-Bot Commands" )
	{
		omnibot_interface::OmnibotCommand();
	}

	//-----------------------------------------------------------------

	//static int wp_compare(const debugLines_t *_wp1, const debugLines_t *_wp2)
	//{
	//	if(_wp1->type == LINE_NONE)
	//		return 1;
	//	if(_wp2->type == LINE_NONE)
	//		return -1;
	//	if(_wp1->drawme == false)
	//		return 1;
	//	if(_wp2->drawme == false)
	//		return -1;
	//	if(_wp1->dist < _wp2->dist)
	//		return -1;
	//	if(_wp1->dist > _wp2->dist)
	//		return 1;
	//	return 0;
	//}

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
			"ff_weapon_radiotagrifle", // TF_WP_RADIOTAG_RIFLE
			"ff_weapon_railgun", // TF_WP_RAILGUN
			"ff_weapon_flamethrower", // TF_WP_FLAMETHROWER
			"ff_weapon_assaultcannon", // TF_WP_MINIGUN
			"ff_weapon_autorifle", // TF_WP_AUTORIFLE
			"ff_weapon_tranquiliser", // TF_WP_DARTGUN
			"ff_weapon_pipelauncher", // TF_WP_PIPELAUNCHER
			"ff_weapon_ic", // TF_WP_NAPALMCANNON
			"ff_weapon_tommygun", // TF_WP_TOMMYGUN
			"ff_weapon_deploysentrygun", // TF_WP_DEPLOY_SG
			"ff_weapon_deploydispenser", // TF_WP_DEPLOY_DISP
			"ff_weapon_deploydetpack", // TF_WP_DEPLOY_DETP
			"ff_weapon_flag", // TF_WP_FLAG
	};
	//
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

	//-----------------------------------------------------------------

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
		case FF_WEAPON_RADIOTAGRIFLE: 
			return TF_WP_RADIOTAG_RIFLE;
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

	//-----------------------------------------------------------------

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
		case TF_WP_RADIOTAG_RIFLE: 
			return FF_WEAPON_RADIOTAGRIFLE;
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

	//-----------------------------------------------------------------

	edict_t* INDEXEDICT(int iEdictNum)		
	{ 
		return engine->PEntityOfEntIndex(iEdictNum); 
	}
	// get entity index
	int ENTINDEX(const edict_t *pEdict)		
	{ 
		return engine->IndexOfEdict(pEdict);
	}

	//////////////////////////////////////////////////////////////////////////
	// Static functions for use by the bot.
	//-----------------------------------------------------------------

	static void obClearNavLines(int _wpview, int _clearLines, int _clearRadius)
	{
		if(_clearLines)
		{
			g_DebugLines.clear();
		}

		if(_clearRadius)
		{
		}
	}

	//-----------------------------------------------------------------

	static void obAddTempDisplayLine(const float _start[3], const float _end[3], const obColor &_color, float _time)
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

	//-----------------------------------------------------------------

	void obAddDisplayRadius(const float _pos[3], const float _radius, const obColor &_color, float _time)
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

	//-----------------------------------------------------------------

	static void _AddDebugLineToDraw(LineType _type, const float _start[3], const float _end[3], const obColor &_color)
	{
		debugLines_t line;
		line.type = _type;
		line.start[0] = _start[0];
		line.start[1] = _start[1];
		line.start[2] = _start[2];
		line.end[0] = _end[0];
		line.end[1] = _end[1];
		line.end[2] = _end[2];
		line.color = _color;

		if(_type == LINE_BLOCKABLE)
		{		
			g_BlockableDebugLines.push_back(line);
		}
		else
		{
			g_DebugLines.push_back(line);
		}	
	}

	static void obAddDisplayLine(int _type, const float _start[3], const float _end[3], const obColor &_color)
	{
		static float fStartOffset = 36.0f;
		static float fEndOffset = -36.0f;
		static float fStartPathOffset = 32.0f;
		static float fEndPathOffset = 12.0f;

		switch(_type)
		{
		case LINE_NORMAL:
			{
				_AddDebugLineToDraw(LINE_NORMAL, _start, _end, _color);
				break;
			}
		case LINE_WAYPOINT:
			{
				float _startPos[3] = { _start[0], _start[1], _start[2] + fStartOffset };
				float _endPos[3] = { _start[0], _start[1], _start[2] + fEndOffset };
				_AddDebugLineToDraw(LINE_WAYPOINT, _startPos, _endPos, _color);
				break;
			}
		case LINE_PATH:
			{
				float _startPos[3] = { _start[0], _start[1], _start[2] + fStartPathOffset };
				float _endPos[3] = { _end[0], _end[1], _end[2] + fEndPathOffset };
				_AddDebugLineToDraw(LINE_PATH, _startPos, _endPos, _color);
				break;
			}
		case LINE_BLOCKABLE:
			{
				_AddDebugLineToDraw(LINE_BLOCKABLE, _start, _end, _color);
				break;
			}
		}
	}

	//-----------------------------------------------------------------

	void Bot_SendSoundEvent(int _client, int _sndtype, GameEntity _source)
	{
		static BotUserData bud;
		bud.DataType = BotUserData::dtEntity;
		bud.udata.m_Entity = _source;
		omnibot_interface::Bot_Interface_SendEvent(PERCEPT_HEAR_SOUND, _client, _sndtype, 0.0f, &bud);
	}

	//-----------------------------------------------------------------

	static void pfnPrintError(const char *_error)
	{
		if(_error)
			Warning("%s\n", _error);
	}

	//-----------------------------------------------------------------

	static void obPrintMessage(const char *_msg)
	{
		if(_msg)
			Msg("%s\n", _msg);
	}

	//-----------------------------------------------------------------

	static void obPrintScreenText(const int _client, const float _pos[3], float _duration, const obColor &_color, const char *_msg)
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
				float fVertical = 0.5;

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
						debugoverlay->AddScreenTextOverlay(0.5f, fVertical, _duration,
							_color.r(), _color.g(), _color.b(), _color.a(), pbufferstart);
						fVertical += 0.02f;
						pbufferstart = &buffer[i];
					}
				}
			}
		}
	}

	//-----------------------------------------------------------------

	static void obBotDoCommand(int _client, const char *_cmd)
	{
		edict_t *pEdict = INDEXENT(_client);
		if(pEdict && !FNullEnt(pEdict))
		{
			serverpluginhelpers->ClientCommand(pEdict, _cmd);
		}
	}

	//-----------------------------------------------------------------

	static obResult obChangeTeam(int _client, int _newteam, const MessageHelper *_data)
	{
		edict_t *pEdict = INDEXEDICT(_client);

		if(pEdict)
		{
			/*CBaseEntity *pEntity = CBaseEntity::Instance( pEdict );
			CFFPlayer *pFFPlayer = dynamic_cast<CFFPlayer*>(pEntity);
			ASSERT(pFFPlayer);*/

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
			}

			serverpluginhelpers->ClientCommand(pEdict, UTIL_VarArgs( "team %s", pTeam ));
			return Success;
		}
		return InvalidEntity;
	}

	//-----------------------------------------------------------------

	static obResult obChangeClass(int _client, int _newclass, const MessageHelper *_data)
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
				}

				serverpluginhelpers->ClientCommand(pEdict, UTIL_VarArgs( "class %s", pClassName ));
				return Success;
			}
		}
		return InvalidEntity;
	}

	//-----------------------------------------------------------------

	static obResult obGetEntityFlags(const GameEntity _ent, BitFlag64 &_entflags)
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( (edict_t*)_ent );
		if(pEntity)
		{
			switch(pEntity->Classify())
			{
			case CLASS_PLAYER:
				{
					if(pEntity->GetHealth() <= 0 || pEntity->GetTeamNumber() == TEAM_SPECTATOR)
						_entflags.SetFlag(ENT_FLAG_DEAD);

					int iWaterLevel = pEntity->GetWaterLevel();
					if(iWaterLevel == 3)
						_entflags.SetFlag(ENT_FLAG_UNDERWATER);
					else if(iWaterLevel >= 2)
						_entflags.SetFlag(ENT_FLAG_INWATER);

					if(pEntity->GetFlags() & FL_DUCKING)
						_entflags.SetFlag(ENT_FLAG_CROUCHED);

					CFFPlayer *pffPlayer = ToFFPlayer(pEntity);
					if(pffPlayer)
					{
						if(pffPlayer->IsOnLadder())
							_entflags.SetFlag(ENT_FLAG_LADDER);
						if(pffPlayer->IsSpeedEffectSet(SE_SNIPERRIFLE))
							_entflags.SetFlag(TF_ENT_FLAG_SNIPERAIMING);
						if(pffPlayer->IsSpeedEffectSet(SE_ASSAULTCANNON))
							_entflags.SetFlag(TF_ENT_FLAG_ASSAULTFIRING);
						if(pffPlayer->IsSpeedEffectSet(SE_LEGSHOT))
							_entflags.SetFlag(TF_ENT_FLAG_LEGSHOT);
						if(pffPlayer->IsSpeedEffectSet(SE_TRANQ))
							_entflags.SetFlag(TF_ENT_FLAG_TRANQED);
						if(pffPlayer->IsSpeedEffectSet(SE_CALTROP))
							_entflags.SetFlag(TF_ENT_FLAG_CALTROP);
						if(pffPlayer->IsRadioTagged())
							_entflags.SetFlag(TF_ENT_FLAG_RADIOTAGGED);
						if(pffPlayer->m_hSabotaging)
							_entflags.SetFlag(TF_ENT_FLAG_SABOTAGING);

						if(pffPlayer->m_bBuilding)
						{
							switch(pffPlayer->m_iCurBuild)
							{
							case FF_BUILD_DISPENSER:
								_entflags.SetFlag(TF_ENT_FLAG_BUILDING_DISP);
								break;
							case FF_BUILD_SENTRYGUN:
								_entflags.SetFlag(TF_ENT_FLAG_BUILDING_SG);
								break;
							case FF_BUILD_DETPACK:
								_entflags.SetFlag(TF_ENT_FLAG_BUILDING_DETP);
								break;
							}
						}
					}
					break;
				}
				/*case CLASS_SENTRYGUN:
				{
				CFFSentryGun *pSentry = static_cast<CFFSentryGun*>(pEntity);					
				break;
				}
				case CLASS_DISPENSER:
				{
				CFFDispenser *pDispenser = static_cast<CFFDispenser*>(pEntity);
				break;
				}*/
			}

			CFFBuildableObject *pBuildable = dynamic_cast<CFFBuildableObject*>(pEntity);
			if(pBuildable)
			{
				if(pBuildable->CanSabotage())
					_entflags.SetFlag(TF_ENT_FLAG_CAN_SABOTAGE);
				if(pBuildable->IsSabotaged())
					_entflags.SetFlag(TF_ENT_FLAG_SABOTAGED);
				// need one for when shooting teammates?
			}
			return Success;
		}
		return InvalidEntity;
	}

	//-----------------------------------------------------------------

	static obResult obGetEntityPowerups(const GameEntity _ent, BitFlag64 &_powerups)
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( (edict_t*)_ent );
		if(pEntity)
		{
			switch(pEntity->Classify())
			{
			case CLASS_PLAYER:
				{
					CFFPlayer *pffPlayer = ToFFPlayer(pEntity);

					if(pffPlayer->IsFeigned())
						_powerups.SetFlag(TF_PWR_FEIGNED);

					// Disguises
					if(pffPlayer)
					{
						switch(pffPlayer->GetDisguisedTeam())
						{
						case TEAM_BLUE:
							_powerups.SetFlag(TF_PWR_DISGUISE_BLUE);
							break;
						case TEAM_RED:
							_powerups.SetFlag(TF_PWR_DISGUISE_RED);
							break;
						case TEAM_YELLOW:
							_powerups.SetFlag(TF_PWR_DISGUISE_YELLOW);
							break;
						case TEAM_GREEN:
							_powerups.SetFlag(TF_PWR_DISGUISE_GREEN);
							break;
						}
						switch(pffPlayer->GetDisguisedClass())
						{
						case CLASS_SCOUT:
							_powerups.SetFlag(TF_PWR_DISGUISE_SCOUT);
							break;
						case CLASS_SNIPER:
							_powerups.SetFlag(TF_PWR_DISGUISE_SNIPER);
							break;
						case CLASS_SOLDIER:
							_powerups.SetFlag(TF_PWR_DISGUISE_SOLDIER);
							break;
						case CLASS_DEMOMAN:
							_powerups.SetFlag(TF_PWR_DISGUISE_DEMOMAN);
							break;
						case CLASS_MEDIC:
							_powerups.SetFlag(TF_PWR_DISGUISE_MEDIC);
							break;
						case CLASS_HWGUY:
							_powerups.SetFlag(TF_PWR_DISGUISE_HWGUY);
							break;
						case CLASS_PYRO:
							_powerups.SetFlag(TF_PWR_DISGUISE_PYRO);
							break;
						case CLASS_SPY:
							_powerups.SetFlag(TF_PWR_DISGUISE_SPY);
							break;
						case CLASS_ENGINEER:
							_powerups.SetFlag(TF_PWR_DISGUISE_ENGINEER);
							break;
						case CLASS_CIVILIAN:
							_powerups.SetFlag(TF_PWR_DISGUISE_CIVILIAN);
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

	//-----------------------------------------------------------------

	static const char *obGetClientName(int _client)
	{
		edict_t *pEnt = INDEXENT(_client);
		if(pEnt)
		{
			CBaseEntity *pEntity = CBaseEntity::Instance( (edict_t*)pEnt );
			if(pEntity)
			{
				CBasePlayer *pPlayer = pEntity->MyCharacterPointer();
				return pPlayer ? pPlayer->GetPlayerName() : 0;
			}
		}
		return 0;
	}

	//-----------------------------------------------------------------

	static int obAddBot( const char *_name, const MessageHelper *_data )
	{
		int iClientNum = -1;

		edict_t *pEdict = engine->CreateFakeClient( _name );
		if (!pEdict)
		{
			pfnPrintError("Unable to Add Bot!");
			return -1;
		}

		// Allocate a player entity for the bot, and call spawn
		CBasePlayer *pPlayer = ((CBasePlayer*)CBaseEntity::Instance( pEdict ));

		pPlayer->ClearFlags();
		pPlayer->AddFlag( FL_CLIENT | FL_FAKECLIENT );

		pPlayer->ChangeTeam( TEAM_UNASSIGNED );
		pPlayer->RemoveAllItems( true );
		pPlayer->Spawn();

		// Get the index of the bot.
		iClientNum = engine->IndexOfEdict(pEdict);

		// Success!, return its client num.
		return iClientNum;
	}

	//-----------------------------------------------------------------

	static int obRemoveBot(const char *_name)
	{
		engine->ServerCommand(UTIL_VarArgs("kick %s\n", _name));
		return -1;
	}

	//-----------------------------------------------------------------

	GameEntity obEntityFromID(const int _id)
	{
		return INDEXENT(_id);
	}

	//-----------------------------------------------------------------

	int obIDFromEntity(const GameEntity _ent)
	{
		return !FNullEnt((edict_t*)_ent) ? ENTINDEX((edict_t*)_ent) : -1;
	}

	//-----------------------------------------------------------------

	static int obGetGameTime()
	{	
		return int(gpGlobals->curtime * 1000.0f);
	}

	//-----------------------------------------------------------------

	static int obGetMaxNumPlayers()
	{
		// TODO: fix this
		return gpGlobals->maxClients;
	}

	//-----------------------------------------------------------------

	static int obGetCurNumPlayers()
	{
		return 0;
	}

	//-----------------------------------------------------------------

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

	int obGetPointContents(const float _pos[3])
	{
		int iContents = UTIL_PointContents(Vector(_pos[0], _pos[1], _pos[2]));
		return obUtilBotContentsFromGameContents(iContents);
	}

	//-----------------------------------------------------------------

	static obResult obTraceLine(BotTraceResult *_result, const float _start[3], const float _end[3], 
		const AABB *_pBBox, int _mask, int _user, obBool _bUsePVS)
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
				_result->m_HitEntity = trace.m_pEnt->edict();
			else
				_result->m_HitEntity = 0;

			// Fill in the bot traceflag.			
			_result->m_Fraction = trace.fraction;
			_result->m_StartSolid = trace.startsolid;			
			_result->m_Endpos[0] = trace.endpos.x;
			_result->m_Endpos[1] = trace.endpos.y;
			_result->m_Endpos[2] = trace.endpos.z;
			_result->m_Normal[0] = trace.plane.normal.x;
			_result->m_Normal[1] = trace.plane.normal.y;
			_result->m_Normal[2] = trace.plane.normal.z;
			_result->m_Contents = obUtilBotContentsFromGameContents(trace.contents);
			return Success;
		}

		// No Hit or Not in PVS
		_result->m_Fraction = 0.0f;
		_result->m_HitEntity = 0;	

		return bInPVS ? Success : OutOfPVS;
	}

	//-----------------------------------------------------------------

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

	//-----------------------------------------------------------------

	static GameEntity obFindEntityByClassName(GameEntity _pStart, const char *_name)
	{
		CBaseEntity *pEntity = _pStart ? CBaseEntity::Instance( (edict_t*)_pStart ) : 0;

		do 
		{
			pEntity = gEntList.FindEntityByClassname(pEntity, _name);
			// special case, if it's a player, don't consider spectators
			if(pEntity && !Q_strcmp(_name, "player"))
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

		return pEntity ? pEntity->edict() : 0;
	}

	//-----------------------------------------------------------------

	static GameEntity obFindEntityByClassId(GameEntity _pStart, int _classId)
	{
		const char *pEntityTypeName = GetGameClassNameFromBotClassId(_classId);
		return pEntityTypeName ? obFindEntityByClassName(_pStart, pEntityTypeName) : 0;
	}

	//-----------------------------------------------------------------

	static GameEntity obFindEntityInSphere(const float _pos[3], float _radius, GameEntity _pStart, const char *_name)
	{
		Vector start(_pos[0], _pos[1], _pos[2]);
		CBaseEntity *pEntity = _pStart ? CBaseEntity::Instance( (edict_t*)_pStart ) : 0;

		do 
		{
			pEntity = gEntList.FindEntityByClassnameWithin(pEntity, _name, start, _radius);
			// special case, if it's a player, don't consider spectators
			if(pEntity && !Q_strcmp(_name, "player"))
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

		return pEntity ? pEntity->edict() : 0;
	}

	//-----------------------------------------------------------------

	static GameEntity obFindEntityInSphereId(const float _pos[3], float _radius, GameEntity _pStart, int _classId)
	{
		const char *pEntityTypeName = GetGameClassNameFromBotClassId(_classId);
		return pEntityTypeName ? obFindEntityInSphere(_pos, _radius, _pStart, pEntityTypeName) : 0;
	}

	//-----------------------------------------------------------------

	static obResult obGetEntityPosition(const GameEntity _ent, float _pos[3])
	{	
		edict_t *pEdict = (edict_t *)_ent;
		CBaseEntity *pEntity = pEdict ? CBaseEntity::Instance( pEdict ) : 0;
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

	//-----------------------------------------------------------------

	static obResult obGetClientPosition(int _client, float _pos[3])
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( _client );
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

	//-----------------------------------------------------------------

	static obResult obGetEntityOrientation(const GameEntity _ent, float _fwd[3], float _right[3], float _up[3])
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( (edict_t*)_ent );
		if(pEntity)
		{
			QAngle viewAngles = pEntity->EyeAngles();
			AngleVectors(viewAngles, (Vector*)_fwd, (Vector*)_right, (Vector*)_up);
			return Success;
		}
		return InvalidEntity;
	}

	//-----------------------------------------------------------------

	static obResult obGetClientOrientation(int _client, float _fwd[3], float _right[3], float _up[3])
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( _client );
		if(pEntity)
		{
			QAngle viewAngles = pEntity->EyeAngles();
			AngleVectors(viewAngles, (Vector*)_fwd, (Vector*)_right, (Vector*)_up);
			return Success;
		}
		return InvalidEntity;
	}

	//-----------------------------------------------------------------

	static obResult obGetEntityVelocity(const GameEntity _ent, float _velocity[3])
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( (edict_t*)_ent );		
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

	//-----------------------------------------------------------------

	static obResult obGetEntityWorldAABB(const GameEntity _ent, AABB *_aabb)
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( (edict_t*)_ent );
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

				Vector vOrig(0.f, 0.f, 0.f);
				if(pEntity->CollisionProp()->IsBoundsDefinedInEntitySpace())
					vOrig = pEntity->GetAbsOrigin();

				vMins = vOrig + pEntity->CollisionProp()->OBBMins();;
				vMaxs = vOrig + pEntity->CollisionProp()->OBBMaxs();
			}

			_aabb->m_Mins[0] = vMins.x;
			_aabb->m_Mins[1] = vMins.y;
			_aabb->m_Mins[2] = vMins.z;
			_aabb->m_Maxs[0] = vMaxs.x;
			_aabb->m_Maxs[1] = vMaxs.y;
			_aabb->m_Maxs[2] = vMaxs.z;
			return Success;
		}

		return InvalidEntity;
	}

	//-----------------------------------------------------------------

	static obResult obGetEntityEyePosition(const GameEntity _ent, float _pos[3])
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( (edict_t*)_ent );
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

	//-----------------------------------------------------------------

	static obResult obGetEntityBonePosition(const GameEntity _ent, int _boneid, float _pos[3])
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( (edict_t*)_ent );
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

	//-----------------------------------------------------------------

	static int obGetEntityOwner(const GameEntity _ent)
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( (edict_t*)_ent );
		if(pEntity)
		{
			CBaseEntity *pOwner = pEntity->GetOwnerEntity();
			return pOwner ? pOwner->entindex() : -1;
		}
		return -1;
	}

	//-----------------------------------------------------------------

	static int obGetEntityTeam(const GameEntity _ent)
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( (edict_t*)_ent );
		assert(pEntity && "Null Ent!");
		if(pEntity)
		{
			return obUtilGetBotTeamFromGameTeam(pEntity->GetTeamNumber());
		}
		return 0;
	}

	//-----------------------------------------------------------------

	static int obGetEntityClass(const GameEntity _ent)
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( (edict_t*)_ent );
		assert(pEntity && "Null Ent!");
		if(pEntity)
		{
			switch(pEntity->Classify())
			{
			case CLASS_PLAYER:
			case CLASS_PLAYER_ALLY:
				{
					CFFPlayer *pFFPlayer = ToFFPlayer(pEntity);
					if(pFFPlayer)
						return obUtilGetBotClassFromGameClass(pFFPlayer->GetClassSlot());
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
				/*case CLASS_INFOSCRIPT:
				case CLASS_TRIGGERSCRIPT:*/
			}
		}
		return 0;
	}

	//-----------------------------------------------------------------

	static obResult obGetThreats()
	{
		EntityInfo info;

		for ( CBaseEntity *pEntity = gEntList.FirstEnt(); pEntity != NULL; pEntity = gEntList.NextEnt(pEntity) )
		{
			// default data.
			info.m_EntityFlags.ClearAll();
			info.m_EntityCategory.ClearAll();
			info.m_EntityClass = obGetEntityClass(pEntity->edict());

			switch(pEntity->Classify())
			{
			case CLASS_PLAYER:
			case CLASS_PLAYER_ALLY:
				{
					CFFPlayer *pFFPlayer = static_cast<CFFPlayer*>(pEntity);
					ASSERT(pFFPlayer);
					//info.m_EntityClass = obUtilGetBotClassFromGameClass(pFFPlayer->GetClassSlot());				
					info.m_EntityCategory.SetFlag(ENT_CAT_SHOOTABLE);
					info.m_EntityCategory.SetFlag(ENT_CAT_PLAYER);
					obGetEntityFlags(pFFPlayer->edict(), info.m_EntityFlags);
					break;
				}
			case CLASS_DISPENSER:
				//info.m_EntityClass = TF_CLASSEX_DISPENSER;
				info.m_EntityCategory.SetFlag(ENT_CAT_SHOOTABLE);
				break;
			case CLASS_SENTRYGUN:
				//info.m_EntityClass = TF_CLASSEX_SENTRY;
				info.m_EntityCategory.SetFlag(ENT_CAT_SHOOTABLE);
				break;
			case CLASS_DETPACK:
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
				info.m_EntityCategory.SetFlag(ENT_CAT_PROJECTILE);
				info.m_EntityCategory.SetFlag(ENT_CAT_AVOID);
				break;
			case CLASS_TURRET:
				info.m_EntityCategory.SetFlag(ENT_CAT_AUTODEFENSE);
				//info.m_EntityCategory.SetFlag(ENT_CAT_STATIC);
				//info.m_EntityClass = TF_CLASSEX_PIPE;
				break;
			case CLASS_BACKPACK:
				info.m_EntityCategory.SetFlag(ENT_CAT_PICKUP);
				//info.m_EntityClass = TF_CLASSEX_PIPE;
				break;
			//case CLASS_INFOSCRIPT:
			//	info.m_EntityCategory = ENT_CAT_PROJECTILE | ENT_CAT_AVOID;
			//	//info.m_EntityClass = TF_CLASSEX_PIPE;
			//	break;
			//case CLASS_TRIGGERSCRIPT:
			//	info.m_EntityCategory = ENT_CAT_PROJECTILE | ENT_CAT_AVOID;
			//	//info.m_EntityClass = TF_CLASSEX_PIPE;
			//	break;	
			default:
				continue;
			}

			if(g_BotFunctions.pfnBotAddThreatEntity)
				g_BotFunctions.pfnBotAddThreatEntity((GameEntity)pEntity->edict(), &info);
		}
		return Success;
	}

	//-----------------------------------------------------------------

	static obResult obGetGoals()
	{
		return Success;
	}

	//-----------------------------------------------------------------

	static obResult obPrintEntitiesInRadius(const float _pos[3], float _radius)
	{
		return Success;
	}

	//-----------------------------------------------------------------

	static obResult obInterfaceSendMessage(const MessageHelper &_data, const GameEntity _ent)
	{
		CBaseEntity *pEnt = CBaseEntity::Instance( (edict_t*)_ent );
		CBasePlayer *pPlayer = pEnt ? pEnt->MyCharacterPointer() : 0;

		switch(_data.GetMessageId())
		{
			///////////////////////
			// General Messages. //
			///////////////////////
		case GEN_MSG_ISALIVE:
			{
				Msg_IsAlive *pMsg = _data.Get<Msg_IsAlive>();
				if(pMsg)
				{
					pMsg->m_IsAlive = pEnt && pEnt->GetHealth() > 0 ? True : False;
				}
				break;
			}
		case GEN_MSG_ISRELOADING:
			{
				Msg_Reloading *pMsg = _data.Get<Msg_Reloading>();
				if(pMsg)
				{
					pMsg->m_Reloading = pPlayer && pPlayer->IsPlayingGesture(ACT_GESTURE_RELOAD) ? True : False;
				}
				break;
			}
		case GEN_MSG_ISREADYTOFIRE:
			{
				Msg_ReadyToFire *pMsg = _data.Get<Msg_ReadyToFire>();
				if(pMsg)
				{
					CBaseCombatWeapon *pWeapon = pPlayer ? pPlayer->GetActiveWeapon() : 0;
					pMsg->m_Ready = pWeapon && (pWeapon->m_flNextPrimaryAttack <= gpGlobals->curtime) ? True : False;
				}
				break;
			}
		case GEN_MSG_ISALLIED:
			{
				Msg_IsAllied *pMsg = _data.Get<Msg_IsAllied>();
				if(pMsg)
				{
					edict_t *pEntOtherEdict = (edict_t *)(pMsg->m_TargetEntity);
					CBaseEntity *pEntOther = pEntOtherEdict ? CBaseEntity::Instance( pEntOtherEdict ) : 0;
					if(pEnt && pEntOther)
					{						
						pMsg->m_IsAllied = g_pGameRules->PlayerRelationship(pEnt, pEntOther) != GR_NOTTEAMMATE ? True : False;
					}
				}
				break;
			}
		case GEN_MSG_ISHUMAN:
			{
				Msg_IsHuman *pMsg = _data.Get<Msg_IsHuman>();
				if(pMsg)
				{
					bool bIsHuman = true;
					if(pEnt)
					{
						bIsHuman = pPlayer ? !pPlayer->IsFakeClient() : false;
					}
					pMsg->m_IsHuman = bIsHuman ? True : False;
				}
				break;
			}		
		case GEN_MSG_GETEQUIPPEDWEAPON:
			{
				Msg_EquippedWeapon *pMsg = _data.Get<Msg_EquippedWeapon>();
				if(pMsg)
				{
					CBaseCombatWeapon *pWeapon = pPlayer ? pPlayer->GetActiveWeapon() : 0;
					pMsg->m_Weapon = pWeapon ? obUtilGetWeaponId(pWeapon->GetName()) : 0;
				}
				break;
			}
		case GEN_MSG_GETHEALTHARMOR:
			{
				Msg_PlayerHealthArmor *pMsg = _data.Get<Msg_PlayerHealthArmor>();
				if(pMsg && pPlayer)
				{
					pMsg->m_CurrentHealth = pPlayer->GetHealth();
					pMsg->m_MaxHealth = pPlayer->GetMaxHealth();
					pMsg->m_CurrentArmor = pPlayer->GetArmor();
					pMsg->m_MaxArmor = pPlayer->GetMaxArmor();
				}
				break;
			}
		case GEN_MSG_GETFLAGSTATE:
			{
				Msg_FlagState *pMsg = _data.Get<Msg_FlagState>();
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
						pMsg->m_Owner = pEnt->GetOwnerEntity() ? pEnt->GetOwnerEntity()->edict() : 0;
					}
				}
				break;
			}
		case GEN_MSG_GAMESTATE:
			{
				Msg_GameState *pMsg = _data.Get<Msg_GameState>();
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

							}
						}
					}
				}
				break;
			}
		case GEN_MSG_GETMAXSPEED:
			{
				Msg_PlayerMaxSpeed *pMsg = _data.Get<Msg_PlayerMaxSpeed>();
				if(pMsg && pPlayer)
				{
					pMsg->m_MaxSpeed = pPlayer->MaxSpeed();
				}
				break;
			}
			//////////////////////////////////
			// Game specific messages next. //
			//////////////////////////////////
		case TF_MSG_ISGUNCHARGING:
			{
				TF_WeaponCharging *pMsg = _data.Get<TF_WeaponCharging>();
				if(pMsg)
				{
					pMsg->m_IsCharging = False;
				}
				break;
			}
		case TF_MSG_GETBUILDABLES:
			{
				TF_BuildInfo *pMsg = _data.Get<TF_BuildInfo>();
				if(pMsg)
				{
					CFFPlayer *pFFPlayer = static_cast<CFFPlayer*>(pPlayer);
					if(pFFPlayer)
					{
						CAI_BaseNPC *pSentry = pFFPlayer->m_hSentryGun.Get();
						pMsg->m_Sentry = pSentry ? pSentry->edict() : 0;
						CAI_BaseNPC *pDispenser = pFFPlayer->m_hDispenser.Get();
						pMsg->m_Dispenser = pDispenser ? pDispenser->edict() : 0;
						CAI_BaseNPC *pDetpack = pFFPlayer->m_hDetpack.Get();
						pMsg->m_Detpack = pDetpack ? pDetpack->edict() : 0;
					}
				}
				break;
			}		
		case TF_MSG_PLAYERPIPECOUNT:
			{
				TF_PlayerPipeCount *pMsg = _data.Get<TF_PlayerPipeCount>();
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
				TF_TeamPipeInfo *pMsg = _data.Get<TF_TeamPipeInfo>();
				if(pMsg)
				{
					pMsg->m_NumTeamPipes = 0;
					pMsg->m_NumTeamPipers = 0;
					pMsg->m_MaxPipesPerPiper = 8;
				}
				break;
			}
		case TF_MSG_DISGUISE:
			{
				TF_Disguise *pMsg = _data.Get<TF_Disguise>();
				if(pMsg)
				{
					int iTeam = obUtilGetGameTeamFromBotTeam(pMsg->m_DisguiseTeam);
					int iClass = obUtilGetGameClassFromBotClass(pMsg->m_DisguiseClass);
					if(iTeam != TEAM_UNASSIGNED && iClass != -1)
					{
						serverpluginhelpers->ClientCommand(pPlayer->edict(), 
							UTIL_VarArgs("disguise %d %d", iTeam, iClass));
					}
					else
					{
						return InvalidParameter;
					}
				}
				break;
			}
		case TF_MSG_FEIGN:
			{
				TF_FeignDeath *pMsg = _data.Get<TF_FeignDeath>();
				if(pMsg)
				{
					serverpluginhelpers->ClientCommand(pPlayer->edict(), 
						pMsg->m_SilentFeign ? "sfeign" : "feign");                    					
				}
				break;
			}
		case TF_MSG_LOCKPOSITION:
			{
				TF_LockPosition *pMsg = _data.Get<TF_LockPosition>();
				if(pMsg)
				{
					CBaseEntity *pEnt = CBaseEntity::Instance( (edict_t*)pMsg->m_TargetPlayer );
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
				TF_HudHint *pMsg = _data.Get<TF_HudHint>();
				pEnt = CBaseEntity::Instance( pMsg->m_TargetPlayer );
				pPlayer = pEnt ? pEnt->MyCharacterPointer() : 0;
				if(pMsg && ToFFPlayer(pPlayer))
				{					
					FF_HudHint(ToFFPlayer(pPlayer), 0, pMsg->m_Id, pMsg->m_Message);
				}
				break;
			}
		case TF_MSG_HUDMENU:
			{
				TF_HudMenu *pMsg = _data.Get<TF_HudMenu>();
				pEnt = CBaseEntity::Instance( pMsg->m_TargetPlayer );
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
					serverpluginhelpers->CreateMessage( pPlayer->edict(), type, kv, &g_ServerPlugin );
					kv->deleteThis();
				}
				break;
			}
		case TF_MSG_HUDTEXT:
			{
				TF_HudText *pMsg = _data.Get<TF_HudText>();
				pEnt = CBaseEntity::Instance( pMsg->m_TargetPlayer );
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
		return Success;
	}

	//-----------------------------------------------------------------

	static obResult obBotGetCurrentWeaponClip(int _client, int *_curclip, int *_maxclip)
	{
		CBaseEntity *pEntity = CBaseEntity::Instance(_client);
		CBasePlayer *pPlayer = pEntity ? pEntity->MyCharacterPointer() : 0;
		if(pPlayer)
		{
			CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
			if(pWeapon)
			{
				*_curclip = pWeapon->Clip1();
				*_maxclip = pWeapon->GetMaxClip1();
			}
			return Success;
		}
		return InvalidEntity;
	}

	//-----------------------------------------------------------------

	static obResult obBotGetCurrentAmmo(int _client, int _ammotype, int *_cur, int *_max)
	{
		CBaseEntity *pEntity = CBaseEntity::Instance(_client);
		CBasePlayer *pPlayer = pEntity ? pEntity->MyCharacterPointer() : 0;
		if(pPlayer)
		{
			CFFPlayer *pFFPlayer = static_cast<CFFPlayer*>(pPlayer);
			switch(_ammotype)
			{
			case TF_AMMO_SHELLS:
				*_cur = pFFPlayer->GetAmmoCount(AMMO_SHELLS);
				*_max = pFFPlayer->GetMaxShells();
				break;
			case TF_AMMO_NAILS:
				*_cur = pFFPlayer->GetAmmoCount(AMMO_NAILS);
				*_max = pFFPlayer->GetMaxNails();
				break;
			case TF_AMMO_ROCKETS:
				*_cur = pFFPlayer->GetAmmoCount(AMMO_ROCKETS);
				*_max = pFFPlayer->GetMaxRockets();
				break;
			case TF_AMMO_CELLS:
				*_cur = pFFPlayer->GetAmmoCount(AMMO_CELLS);
				*_max = pFFPlayer->GetMaxCells();
				break;
			case TF_AMMO_MEDIKIT:
				*_cur = -1;
				*_max = -1;
				break;
			case TF_AMMO_DETPACK:
				*_cur = pFFPlayer->GetAmmoCount(AMMO_DETPACK);
				*_max = pFFPlayer->GetMaxDetpack();
				break;
			case TF_AMMO_RADIOTAG:
				*_cur = pFFPlayer->GetAmmoCount(AMMO_RADIOTAG);
				*_max = pFFPlayer->GetMaxRadioTag();
				break;
			case TF_AMMO_GRENADE1:
				*_cur = pFFPlayer->GetPrimaryGrenades();
				*_max = 4;
				break;
			case TF_AMMO_GRENADE2:
				*_cur = pFFPlayer->GetSecondaryGrenades();
				*_max = 3;
				break;
			case -1:
				*_cur = -1;
				*_max = -1;
				break;
			default:
				return InvalidParameter;
			}

			return Success;
		}

		*_cur = 0;
		*_max = 0;

		return InvalidEntity;
	}

	//-----------------------------------------------------------------

	static void obUpdateBotInput(int _client, const ClientInput *_input)
	{
		edict_t *pEdict = INDEXENT(_client);
		CBaseEntity *pEntity = pEdict && !FNullEnt(pEdict) ? CBaseEntity::Instance(pEdict) : 0;
		CBasePlayer *pPlayer = pEntity ? pEntity->MyCharacterPointer() : 0;
		if(pPlayer && pPlayer->IsBot())
		{
			CBotCmd cmd;
			//CUserCmd cmd;

			// Process the bot keypresses.
			if(_input->m_ButtonFlags.CheckFlag(BOT_BUTTON_ATTACK1))
				cmd.buttons |= IN_ATTACK;
			if(_input->m_ButtonFlags.CheckFlag(BOT_BUTTON_ATTACK2))
				cmd.buttons |= IN_ATTACK2;
			if(_input->m_ButtonFlags.CheckFlag(BOT_BUTTON_WALK))
				cmd.buttons |= IN_SPEED;
			if(_input->m_ButtonFlags.CheckFlag(BOT_BUTTON_USE))
				cmd.buttons |= IN_USE;
			if(_input->m_ButtonFlags.CheckFlag(BOT_BUTTON_JUMP))
				cmd.buttons |= IN_JUMP;
			if(_input->m_ButtonFlags.CheckFlag(BOT_BUTTON_CROUCH))
				cmd.buttons |= IN_DUCK;
			if(_input->m_ButtonFlags.CheckFlag(BOT_BUTTON_RELOAD))
				cmd.buttons |= IN_RELOAD;

			// Store the facing.
			Vector vFacing(_input->m_Facing[0], _input->m_Facing[1], _input->m_Facing[2]);
			VectorAngles(vFacing, cmd.viewangles);

			// Calculate the movement vector, taking into account the view direction.
			Vector vForward, vRight, vUp;
			Vector vMoveDir(_input->m_MoveDir[0],_input->m_MoveDir[1],_input->m_MoveDir[2]);
			AngleVectors(cmd.viewangles, &vForward, &vRight, &vUp);

			cmd.forwardmove = vForward.Dot(vMoveDir) * pPlayer->MaxSpeed();
			cmd.sidemove = vRight.Dot(vMoveDir) * pPlayer->MaxSpeed();
			cmd.upmove = vUp.Dot(vMoveDir) * pPlayer->MaxSpeed();

			// Do we have this weapon?
			const char *pNewWeapon = obUtilGetStringFromWeaponId(_input->m_CurrentWeapon);
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

	//-----------------------------------------------------------------

	static void obGetMapExtents(AABB *_aabb)
	{
		memset(_aabb, 0, sizeof(AABB));

		CWorld *world = GetWorldEntity();
		if(world)
		{
			Vector mins, maxs;
			world->GetWorldBounds(mins, maxs);

			for(int i = 0; i < 3; ++i)
			{
				_aabb->m_Mins[i] = mins[i];
				_aabb->m_Maxs[i] = maxs[i];
			}
		}
	}

	static const char *obGetMapName()
	{
		static char mapname[256] = {0};
		if(gpGlobals->mapname.ToCStr())
		{
			Q_snprintf( mapname, sizeof( mapname ), STRING( gpGlobals->mapname ) );
		}
		return mapname;
	}

	//-----------------------------------------------------------------

	static const char *obGetGameName() 
	{
		return "Halflife 2";
	}

	//-----------------------------------------------------------------

	static const char *obGetModName() 
	{
		assert(g_pGameRules);
		return g_pGameRules->GetGameDescription();
	}

	//-----------------------------------------------------------------

	static const char *obGetBotPath() 
	{
		static char botPath[512] = {0};

		char buffer[512] = {0};
		filesystem->GetLocalPath(
			UTIL_VarArgs("%s/%s", omnibot_path.GetString(), "omnibot_ff.dll"), buffer, 512);

		Q_ExtractFilePath(buffer, botPath, 512);
		return botPath;
	}

	//-----------------------------------------------------------------

	static const char *obGetModVersion() 
	{
		static char buffer[256];
		engine->GetGameDir(buffer, 256);
		return buffer;
	}

	//-----------------------------------------------------------------

	static void obUpdateDrawnWaypoints(float _radius)
	{		
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(1);
		if(pPlayer && (g_NextWpUpdate < gpGlobals->curtime))
		{
			g_NextWpUpdate = gpGlobals->curtime + 2.0f;

			for(unsigned int i = 0; i < g_DebugLines.size(); ++i)
			{
				// Draw it.
				switch(g_DebugLines[i].type)
				{
				case LINE_NORMAL:
				case LINE_WAYPOINT:
				case LINE_BLOCKABLE:
					{
						obAddTempDisplayLine(g_DebugLines[i].start, g_DebugLines[i].end, g_DebugLines[i].color, 2.0f);

						// Blockables only drawn once.
						if(g_DebugLines[i].type == LINE_BLOCKABLE)
						{
							g_DebugLines[i].type = LINE_NONE;
						}
						break;
					}
				case LINE_PATH:
					{
						// adjust the end of the line for paths so they slant down
						// toward the end of the connection instead of drawing on top
						// of each other.
						Vector vLineEnd(g_DebugLines[i].end[0], g_DebugLines[i].end[1], g_DebugLines[i].end[2]);
						vLineEnd.z -= 30.0f;
						obAddTempDisplayLine(g_DebugLines[i].start, (float*)&vLineEnd, g_DebugLines[i].color, 2.0f);
						break;
					}
				}
			}
		}
	}

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
		//if( gameLocal.isServer )
		{
			if(g_BotFunctions.pfnBotConsoleCommand)
			{
				if(engine->Cmd_Args())
				{
					g_BotFunctions.pfnBotConsoleCommand(engine->Cmd_Args(), strlen(engine->Cmd_Args()));
				}
			}
			else
			{
				obPrintMessage( "Omni-bot Not Loaded\n" );
			}
		}
	}

	//-----------------------------------------------------------------

	void omnibot_interface::Bot_Interface_SendEvent(int _eid, int _dest, int _source, int _msdelay, BotUserData * _data)
	{
		//if( gameLocal.isServer )
		{
			if(g_BotFunctions.pfnBotSendEvent)
			{
				g_BotFunctions.pfnBotSendEvent(_eid, _dest, _source, _msdelay, _data);
			}
		}
	}

	//-----------------------------------------------------------------

	void omnibot_interface::Bot_Interface_SendGlobalEvent(int _eid, int _source, int _msdelay, BotUserData * _data)
	{
		//if( gameLocal.isServer )
		{
			if(g_BotFunctions.pfnBotSendGlobalEvent)
			{
				g_BotFunctions.pfnBotSendGlobalEvent(_eid, _source, _msdelay, _data);
			}
		}
	}
	//-----------------------------------------------------------------

	void omnibot_interface::Bot_SendTrigger(TriggerInfo *_triggerInfo)
	{
		//if( gameLocal.isServer )
		{
			if(g_BotFunctions.pfnBotSendTrigger)
			{
				g_BotFunctions.pfnBotSendTrigger(_triggerInfo);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	class OmnibotEntityListener : public IEntityListener
	{
		virtual void OnEntityCreated( CBaseEntity *pEntity )
		{
			/*int iIndex = pEntity->entindex();
			Msg("Entity Created %s\n", pEntity->GetClassname());*/
		}
		virtual void OnEntitySpawned( CBaseEntity *pEntity )
		{
			/*int iIndex = pEntity->entindex();
			Msg("Entity Spawned %s\n", pEntity->GetClassname());*/
		}
		virtual void OnEntityDeleted( CBaseEntity *pEntity )
		{
			/*int iIndex = pEntity->entindex();
			Msg("Entity Deleted %s\n", pEntity->GetClassname());*/
		}
	};

	OmnibotEntityListener gBotEntityListener;

	//////////////////////////////////////////////////////////////////////////
	// Interface Functions
	bool omnibot_interface::InitBotInterface()
	{
		if(!omnibot_enable.GetBool())
		{
			obPrintMessage( "Omni-bot Currently Disabled. Re-enable with cvar omnibot_enable\n" );
			return false;
		}

		/*if( !gameLocal.isServer )
		return false;*/

		s_NextUpdateTime = 0;
		g_NextWpUpdate = 0.0f;

		obPrintMessage( "-------------- Omni-bot Init ----------------\n" );

		int iLoadResult = -1;
		Nav_EngineFuncs_t navFunctions;
		memset(&navFunctions, 0, sizeof(navFunctions));
		memset(&g_InterfaceFunctions, 0, sizeof(g_InterfaceFunctions));

		g_InterfaceFunctions.pfnAddBot						= obAddBot;
		g_InterfaceFunctions.pfnRemoveBot					= obRemoveBot;	
		g_InterfaceFunctions.pfnChangeClass					= obChangeClass;
		g_InterfaceFunctions.pfnChangeTeam					= obChangeTeam;
		g_InterfaceFunctions.pfnGetClientPosition			= obGetClientPosition;
		g_InterfaceFunctions.pfnGetClientOrientation		= obGetClientOrientation;

		// Set up all the utility functions.
		g_InterfaceFunctions.pfnPrintScreenText				= obPrintScreenText;
		g_InterfaceFunctions.pfnPrintError					= pfnPrintError;
		g_InterfaceFunctions.pfnPrintMessage				= obPrintMessage;	
		g_InterfaceFunctions.pfnTraceLine					= obTraceLine;
		g_InterfaceFunctions.pfnGetPointContents			= obGetPointContents;		

		g_InterfaceFunctions.pfnUpdateBotInput				= obUpdateBotInput;
		g_InterfaceFunctions.pfnBotCommand					= obBotDoCommand;

		g_InterfaceFunctions.pfnGetMapExtents				= obGetMapExtents;
		g_InterfaceFunctions.pfnGetMapName					= obGetMapName;
		g_InterfaceFunctions.pfnGetGameName					= obGetGameName;
		g_InterfaceFunctions.pfnGetModName					= obGetModName;
		g_InterfaceFunctions.pfnGetBotPath					= obGetBotPath;
		g_InterfaceFunctions.pfnGetModVers					= obGetModVersion;
		g_InterfaceFunctions.pfnGetGameTime					= obGetGameTime;
		g_InterfaceFunctions.pfnGetClientName				= obGetClientName;
		g_InterfaceFunctions.pfnGetGoals					= obGetGoals;
		g_InterfaceFunctions.pfnGetThreats					= obGetThreats;
		g_InterfaceFunctions.pfnGetCurNumPlayers			= obGetCurNumPlayers;
		g_InterfaceFunctions.pfnGetMaxNumPlayers			= obGetMaxNumPlayers;
		g_InterfaceFunctions.pfnInterfaceSendMessage		= obInterfaceSendMessage;

		// Entity info.
		g_InterfaceFunctions.pfnGetEntityFlags				= obGetEntityFlags;
		g_InterfaceFunctions.pfnGetEntityEyePosition		= obGetEntityEyePosition;
		g_InterfaceFunctions.pfnGetEntityBonePosition		= obGetEntityBonePosition;
		g_InterfaceFunctions.pfnGetEntityVelocity			= obGetEntityVelocity;
		g_InterfaceFunctions.pfnGetEntityPosition			= obGetEntityPosition;
		g_InterfaceFunctions.pfnGetEntityOrientation		= obGetEntityOrientation;
		g_InterfaceFunctions.pfnGetEntityWorldAABB			= obGetEntityWorldAABB;
		g_InterfaceFunctions.pfnGetEntityOwner				= obGetEntityOwner;
		g_InterfaceFunctions.pfnGetEntityTeam				= obGetEntityTeam;
		g_InterfaceFunctions.pfnGetEntityClass				= obGetEntityClass;
		g_InterfaceFunctions.pfnGetEntityPowerups			= obGetEntityPowerups;

		g_InterfaceFunctions.pfnBotGetCurrentAmmo			= obBotGetCurrentAmmo;
		g_InterfaceFunctions.pfnBotGetCurrentWeaponClip		= obBotGetCurrentWeaponClip;

		g_InterfaceFunctions.pfnEntityFromID				= obEntityFromID;
		g_InterfaceFunctions.pfnIDFromEntity				= obIDFromEntity;

		// Waypoint functions.
		g_InterfaceFunctions.pfnAddDisplayLine				= obAddDisplayLine;
		g_InterfaceFunctions.pfnAddDisplayRadius			= obAddDisplayRadius;
		g_InterfaceFunctions.pfnAddTempDisplayLine			= obAddTempDisplayLine;
		g_InterfaceFunctions.pfnClearDebugLines				= obClearNavLines;
		g_InterfaceFunctions.pfnFindEntityByClassName		= obFindEntityByClassName;
		g_InterfaceFunctions.pfnFindEntityInSphere			= obFindEntityInSphere;
		g_InterfaceFunctions.pfnFindEntityByClassId			= obFindEntityByClassId;
		g_InterfaceFunctions.pfnFindEntityInSphereId		= obFindEntityInSphereId;

		// clear the debug arrays
		//g_DebugLines.Clear();

		// Allow user configurable navigation system
		//int iNavSystem = omnibot_nav.GetInteger();
		NavigatorID navId = NAVID_WP;
		/*switch(iNavSystem)
		{
		case NAVID_WP:
		navId = NAVID_WP;
		break;
		}*/

		// Look for the bot dll.
		const int BUF_SIZE = 512;
		char botFilePath[BUF_SIZE] = {0};
		filesystem->GetLocalPath(
			UTIL_VarArgs("%s/%s", omnibot_path.GetString(), "omnibot_ff.dll"), botFilePath, BUF_SIZE);
		char botPath[BUF_SIZE] = {0};
		Q_ExtractFilePath(botFilePath, botPath, BUF_SIZE);
		botPath[strlen(botPath)-1] = 0;
		Q_FixSlashes(botPath);

		bool bSuccess = true;
		INITBOTLIBRARY(FF_VERSION_LATEST, navId, "omnibot_ff.dll", "omnibot_ff.so", botPath, iLoadResult);
		if(iLoadResult != BOT_ERROR_NONE)
		{
			pfnPrintError(BOT_ERR_MSG(iLoadResult));
			bSuccess = false;
			SHUTDOWNBOTLIBRARY;
		}
		g_ReadyForGoals = false;
		gEntList.AddListenerEntity(&gBotEntityListener);
		obPrintMessage( "---------------------------------------------\n" );
		return bSuccess;
	}

	void omnibot_interface::ShutdownBotInterface()
	{
		/*if( !gameLocal.isServer )
		return;*/
		if(g_BotFunctions.pfnBotShutdown)
		{
			obPrintMessage( "------------ Omni-bot Shutdown --------------\n" );
			gEntList.RemoveListenerEntity(&gBotEntityListener);
			Bot_Interface_SendGlobalEvent(GAME_ID_ENDGAME, -1, 0, 0);
			g_BotFunctions.pfnBotShutdown();
			memset(&g_BotFunctions, 0, sizeof(g_BotFunctions));
			obPrintMessage( "Omni-bot Shut Down Successfully\n" );
			obPrintMessage( "---------------------------------------------\n" );

			// Temp fix?
			if( debugoverlay )
				debugoverlay->ClearAllOverlays();
		}
		SHUTDOWNBOTLIBRARY;		
	}

	void omnibot_interface::UpdateBotInterface()
	{
		//if( gameLocal.isServer )
		{		
			if(gpGlobals->curtime > s_NextUpdateTime)
			{
				s_NextUpdateTime = gpGlobals->curtime + 0;//omnibot_thinkrate.GetInteger();
				obUpdateDrawnWaypoints(2000.0f);

				// Call the libraries update.
				if(g_BotFunctions.pfnBotUpdate)
				{
					VPROF_BUDGET( "Omni-bot::Update", _T("Omni-bot") );
					//////////////////////////////////////////////////////////////////////////
					if(!engine->IsDedicatedServer())
					{
						for ( int i = 1; i <= gpGlobals->maxClients; i++ )
						{
							CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

							if (pPlayer && 
								(pPlayer->GetObserverMode() == OBS_MODE_IN_EYE || 
								pPlayer->GetObserverMode() == OBS_MODE_CHASE))
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

					if(g_ReadyForGoals)
					{
						for(int i = 0; i < g_DeferredGoalIndex; ++i)
						{
							g_BotFunctions.pfnBotAddGoal(
								(GameEntity)g_DeferredGoals[i].m_Edict, 
								g_DeferredGoals[i].m_GoalType, 
								g_DeferredGoals[i].m_GoalTeam, 
								g_DeferredGoals[i].m_GoalName, 
								NULL);
						}
						g_DeferredGoalIndex = 0;
					}					

					g_BotFunctions.pfnBotUpdate();
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Message Helpers
	void Notify_GameStarted()
	{
		omnibot_interface::Bot_Interface_SendGlobalEvent(GAME_ID_STARTGAME, 0, 100, NULL);
		g_ReadyForGoals = true;		
	}

	void Notify_GameEnded(int _winningteam)
	{
		omnibot_interface::Bot_Interface_SendGlobalEvent(GAME_ID_ENDGAME, 0, 0, NULL);
		g_ReadyForGoals = false;
	}

	void Notify_ChatMsg(CBasePlayer *_player, const char *_msg)
	{
		BotUserData bud(_msg);
		int iSourceGameId = _player->entindex()-1;
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer *pPlayer = UTIL_PlayerByIndex(i);

			// Check player classes on this player's team
			if (pPlayer && pPlayer->IsBot())
			{
				int iGameId = pPlayer->entindex()-1;
				if(iGameId != iSourceGameId)
				{
					omnibot_interface::Bot_Interface_SendEvent(PERCEPT_HEAR_GLOBALCHATMSG, 
						iGameId, iSourceGameId, 0, &bud);
				}
			}
		}
	}

	void Notify_TeamChatMsg(CBasePlayer *_player, const char *_msg)
	{
		BotUserData bud(_msg);
		int iSourceGameId = _player->entindex()-1;
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer *pPlayer = UTIL_PlayerByIndex(i);

			// Check player classes on this player's team
			if (pPlayer && pPlayer->IsBot() && pPlayer->GetTeamNumber() == _player->GetTeamNumber())
			{
				int iGameId = pPlayer->entindex()-1;
				if(iGameId != iSourceGameId)
				{
					omnibot_interface::Bot_Interface_SendEvent(PERCEPT_HEAR_TEAMCHATMSG, 
						iGameId, iSourceGameId, 0, &bud);
				}
			}
		}
	}

	void Notify_Spectated(CBasePlayer *_player, CBasePlayer *_spectated)
	{
		if(_spectated && _spectated->IsBot())
		{
			int iGameId = _spectated->entindex()-1;
			omnibot_interface::Bot_Interface_SendEvent(MESSAGE_SPECTATED, iGameId, 0, 0, 0);
		}
	}

	void Notify_AddWeapon(CBasePlayer *_player, const char *_item)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud(obUtilGetWeaponId(_item));
		omnibot_interface::Bot_Interface_SendGlobalEvent(MESSAGE_ADDWEAPON, iGameId, 0, &bud);

		// ERROR DETECTION
		if(bud.GetInt() == TF_WP_NONE)
		{
			// Added newline since this was showing up
			Warning("Invalid Weapon Id: Notify_AddWeapon\n");
		}
	}

	void Notify_RemoveWeapon(CBasePlayer *_player, const char *_item)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud(obUtilGetWeaponId(_item));
		omnibot_interface::Bot_Interface_SendGlobalEvent(MESSAGE_REMOVEWEAPON, iGameId, 0, &bud);

		// ERROR DETECTION
		if(bud.GetInt() == TF_WP_NONE)
		{
			Warning("Invalid Weapon Id: Notify_RemoveWeapon\n");
		}
	}

	void Notify_RemoveAllItems(CBasePlayer *_player)
	{
		int iGameId = _player->entindex()-1;
		omnibot_interface::Bot_Interface_SendGlobalEvent(MESSAGE_RESETWEAPONS, iGameId, 0, NULL);
	}	

	void Notify_ClientConnected(CBasePlayer *_player, bool _isbot)
	{
		/*int iGameId = _player->entindex()-1;
		if (_isbot)
			omnibot_interface::Bot_Interface_SendGlobalEvent(GAME_ID_BOTCONNECTED, iGameId, 100, NULL);
		else
			omnibot_interface::Bot_Interface_SendGlobalEvent(GAME_ID_CLIENTCONNECTED, iGameId, 100, NULL);*/
	}

	void Notify_ClientDisConnected(CBasePlayer *_player)
	{
		int iGameId = _player->entindex()-1;
		omnibot_interface::Bot_Interface_SendGlobalEvent(GAME_ID_CLIENTDISCONNECTED, iGameId, 0, NULL);	
	}

	void Notify_Spawned(CBasePlayer *_player)
	{
		int iGameId = _player->entindex()-1;
		omnibot_interface::Bot_Interface_SendEvent(MESSAGE_SPAWN, iGameId, 0, 0, 0);
	}

	void Notify_Hurt(CBasePlayer *_player, edict_t *_attacker)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud((GameEntity)_attacker);
		omnibot_interface::Bot_Interface_SendEvent(PERCEPT_FEEL_PAIN, iGameId, 0, 0, &bud);
	}

	void Notify_Death(CBasePlayer *_player, edict_t *_attacker, const char *_weapon)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud(_weapon);
		omnibot_interface::Bot_Interface_SendEvent(MESSAGE_DEATH, 
			iGameId, _attacker ? ENTINDEX(_attacker) : -1, 0, &bud);
	}

	void Notify_ChangedTeam(CBasePlayer *_player, int _newteam)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud(_newteam);
		omnibot_interface::Bot_Interface_SendEvent(MESSAGE_CHANGETEAM, iGameId, 0, 0, &bud);
	}

	void Notify_ChangedClass(CBasePlayer *_player, int _oldclass, int _newclass)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud(obUtilGetBotClassFromGameClass(_newclass));
		omnibot_interface::Bot_Interface_SendEvent(MESSAGE_CHANGECLASS, iGameId, 0, 0, &bud);
	}

	void Notify_Build_MustBeOnGround(CBasePlayer *_player, int _buildable)
	{
		int iGameId = _player->entindex()-1;
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_BUILD_MUSTBEONGROUND, iGameId, 0, 0, 0);
	}

	void Notify_Build_CantBuild(CBasePlayer *_player, int _buildable)
	{
		int iGameId = _player->entindex()-1;
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
			omnibot_interface::Bot_Interface_SendEvent(iMsg, iGameId, 0, 0, 0);
		}
	}

	void Notify_Build_AlreadyBuilt(CBasePlayer *_player, int _buildable)
	{
		int iGameId = _player->entindex()-1;
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
			omnibot_interface::Bot_Interface_SendEvent(iMsg, iGameId, 0, 0, 0);
		}
	}

	void Notify_Build_NotEnoughAmmo(CBasePlayer *_player, int _buildable)
	{
		int iGameId = _player->entindex()-1;
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
			omnibot_interface::Bot_Interface_SendEvent(iMsg, iGameId, 0, 0, 0);
		}
	}

	void Notify_Disguising(CBasePlayer *_player, int _disguiseTeam, int _disguiseClass)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud;
		bud.DataType = BotUserData::dt3_4byteFlags;
		bud.udata.m_4ByteFlags[0] = obUtilGetBotTeamFromGameTeam(_disguiseTeam);
		bud.udata.m_4ByteFlags[1] = obUtilGetBotClassFromGameClass(_disguiseClass);
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_DISGUISED, iGameId, 0, 0, &bud);
	}

	void Notify_Disguised(CBasePlayer *_player, int _disguiseTeam, int _disguiseClass)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud;
		bud.DataType = BotUserData::dt3_4byteFlags;
		bud.udata.m_4ByteFlags[0] = obUtilGetBotTeamFromGameTeam(_disguiseTeam);
		bud.udata.m_4ByteFlags[1] = obUtilGetBotClassFromGameClass(_disguiseClass);
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_DISGUISING, iGameId, 0, 0, &bud);
	}

	void Notify_DisguiseLost(CBasePlayer *_player)
	{
		int iGameId = _player->entindex()-1;
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_DISGUISE_LOST, iGameId, 0, 0, 0);
	}

	void Notify_UnFeigned(CBasePlayer *_player)
	{
		int iGameId = _player->entindex()-1;
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_UNFEIGNED, iGameId, 0, 0, 0);
	}

	void Notify_CantFeign(CBasePlayer *_player)
	{
		int iGameId = _player->entindex()-1;
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_CANT_FEIGN, iGameId, 0, 0, 0);
	}

	void Notify_Feigned(CBasePlayer *_player)
	{
		int iGameId = _player->entindex()-1;
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_FEIGNED, iGameId, 0, 0, 0);
	}

	void Notify_RadarDetectedEnemy(CBasePlayer *_player, edict_t *_ent)
	{
		int iGameId =_player->entindex()-1;
		BotUserData bud(_ent);
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_RADAR_DETECT_ENEMY, iGameId, 0, 0, &bud);
	}

	void Notify_RadioTagUpdate(CBasePlayer *_player, edict_t *_ent)
	{
		int iGameId =_player->entindex()-1;
		BotUserData bud(_ent);
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_RADIOTAG_UPDATE, iGameId, 0, 0, &bud);
	}

	void Notify_BuildableDamaged(CBasePlayer *_player, int _type, edict_t *_buildableEnt)
	{
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
			int iGameId = _player->entindex()-1;
			BotUserData bud(_buildableEnt);
			omnibot_interface::Bot_Interface_SendEvent(iMsg, iGameId, 0, 0, &bud);
		}
	}

	void Notify_DispenserBuilding(CBasePlayer *_player, edict_t *_buildEnt)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud(_buildEnt);
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_DISPENSER_BUILDING, iGameId, 0, 0, &bud);
	}

	void Notify_DispenserBuilt(CBasePlayer *_player, edict_t *_buildEnt)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud(_buildEnt);
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_DISPENSER_BUILT, iGameId, 0, 0, &bud);
	}

	void Notify_DispenserEnemyUsed(CBasePlayer *_player, edict_t *_enemyUser)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud(_enemyUser);
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_DISPENSER_ENEMYUSED, iGameId, 0, 0, &bud);
	}

	void Notify_DispenserDestroyed(CBasePlayer *_player, edict_t *_attacker)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud;
		if(_attacker)
		{
			bud.DataType = BotUserData::dtEntity;
			bud.udata.m_Entity = _attacker;
		}
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_DISPENSER_DESTROYED, iGameId, 0, 0, &bud);
	}

	void Notify_SentryUpgraded(CBasePlayer *_player, int _level)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud(_level);
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_SENTRY_UPGRADED, iGameId, 0, 0, &bud);
	}

	void Notify_SentryBuilding(CBasePlayer *_player, edict_t *_buildEnt)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud(_buildEnt);
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_SENTRY_BUILDING, iGameId, 0, 0, &bud);
	}

	void Notify_SentryBuilt(CBasePlayer *_player, edict_t *_buildEnt)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud(_buildEnt);
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_SENTRY_BUILT, iGameId, 0, 0, &bud);
	}

	void Notify_SentryDestroyed(CBasePlayer *_player, edict_t *_attacker)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud;
		if(_attacker)
		{
			bud.DataType = BotUserData::dtEntity;
			bud.udata.m_Entity = _attacker;
		}
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_SENTRY_DESTROYED, iGameId, 0, 0, &bud);		
	}

	void Notify_SentrySpottedEnemy(CBasePlayer *_player)
	{
		int iGameId = _player->entindex()-1;
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_SENTRY_SPOTENEMY, iGameId, 0, 0, 0);
	}

	void Notify_SentryAimed(CBasePlayer *_player)
	{
		int iGameId = _player->entindex()-1;
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_SENTRY_AIMED, iGameId, 0, 0, 0);
	}

	void Notify_DetpackBuilding(CBasePlayer *_player, edict_t *_buildEnt)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud(_buildEnt);
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_DETPACK_BUILDING, iGameId, 0, 0, &bud);
	}

	void Notify_DetpackBuilt(CBasePlayer *_player, edict_t *_buildEnt)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud(_buildEnt);
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_DETPACK_BUILT, iGameId, 0, 0, &bud);
	}

	void Notify_DetpackDetonated(CBasePlayer *_player)
	{
		int iGameId = _player->entindex()-1;
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_DETPACK_DETONATED, iGameId, 0, 0, 0);
	}

	void Notify_DispenserSabotaged(CBasePlayer *_player, edict_t *_saboteur)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud(_saboteur);
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_SABOTAGED_DISPENSER, iGameId, 0, 0, &bud);
	}

	void Notify_SentrySabotaged(CBasePlayer *_player, edict_t *_saboteur)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud(_saboteur);
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_SABOTAGED_DISPENSER, iGameId, 0, 0, &bud);
	}

	void Notify_DispenserDetonated(CBasePlayer *_player)
	{
		int iGameId = _player->entindex()-1;
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_DISPENSER_DETONATED, iGameId, 0, 0, 0);
	}

	void Notify_DispenserDismantled(CBasePlayer *_player)
	{
		int iGameId = _player->entindex()-1;
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_DISPENSER_DISMANTLED, iGameId, 0, 0, 0);
	}

	void Notify_SentryDetonated(CBasePlayer *_player)
	{
		int iGameId = _player->entindex()-1;
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_SENTRY_DETONATED, iGameId, 0, 0, 0);
	}

	void Notify_SentryDismantled(CBasePlayer *_player)
	{
		int iGameId = _player->entindex()-1;
		omnibot_interface::Bot_Interface_SendEvent(TF_MSG_SENTRY_DISMANTLED, iGameId, 0, 0, 0);
	}

	void Notify_PlayerShoot(CBasePlayer *_player, int _weapon)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud(obUtilGetBotWeaponFromGameWeapon(_weapon));
		omnibot_interface::Bot_Interface_SendEvent(ACTION_WEAPON_FIRE, iGameId, 0, 0, 0);
	}

	void Notify_PlayerUsed(CBasePlayer *_player, CBaseEntity *_entityUsed)
	{
		CBasePlayer *pUsedPlayer = ToBasePlayer(_entityUsed);
		if(pUsedPlayer && pUsedPlayer->IsBot())
		{
			int iGameId = pUsedPlayer->entindex()-1;
			BotUserData bud(_player->edict());
			omnibot_interface::Bot_Interface_SendEvent(PERCEPT_FEEL_PLAYER_USE, iGameId, 0, 0, &bud);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void Notify_GoalInfo(CBaseEntity *_entity, int _type, int _team)
	{
		BotGoalInfo gi;

		//////////////////////////////////////////////////////////////////////////
		const int iAllTeams = (1<<TF_TEAM_BLUE)|(1<<TF_TEAM_RED)|(1<<TF_TEAM_YELLOW)|(1<<TF_TEAM_GREEN);
		gi.m_GoalTeam = iAllTeams;
		switch(_team)
		{
		case TEAM_BLUE:
			gi.m_GoalTeam = iAllTeams & ~(1<<TF_TEAM_BLUE);
			break;
		case TEAM_RED:
			gi.m_GoalTeam = iAllTeams & ~(1<<TF_TEAM_RED);
			break;
		case TEAM_YELLOW:
			gi.m_GoalTeam = iAllTeams & ~(1<<TF_TEAM_YELLOW);
			break;
		case TEAM_GREEN:
			gi.m_GoalTeam = iAllTeams & ~(1<<TF_TEAM_GREEN);
			break;
		}
		//////////////////////////////////////////////////////////////////////////

		if(gi.m_GoalTeam != 0)
		{
			gi.m_Edict = _entity->edict();
			const char *pName = _entity->GetName();
			Q_strncpy(gi.m_GoalName, pName ? pName : "", sizeof(gi.m_GoalName));

			switch(_type)
			{
			case Omnibot::kBackPack_Grenades:
				{
					EntityInfo info;
					info.m_EntityClass = TF_CLASSEX_BACKPACK_GRENADES;
					info.m_EntityCategory.SetFlag(ENT_CAT_PICKUP);
					info.m_EntityCategory.SetFlag(ENT_CAT_STATIC);
					if(_entity->IsEffectActive( EF_NODRAW ))
						info.m_EntityFlags.SetFlag(ENT_FLAG_DISABLED);
					if(g_BotFunctions.pfnBotAddThreatEntity)
						g_BotFunctions.pfnBotAddThreatEntity((GameEntity)_entity->edict(), &info);
					return;
				}
			case Omnibot::kBackPack_Health:
				{
					EntityInfo info;
					info.m_EntityClass = TF_CLASSEX_BACKPACK_HEALTH;
					info.m_EntityCategory.SetFlag(ENT_CAT_PICKUP);
					info.m_EntityCategory.SetFlag(ENT_CAT_STATIC);
					if(_entity->IsEffectActive( EF_NODRAW ))
						info.m_EntityFlags.SetFlag(ENT_FLAG_DISABLED);
					if(g_BotFunctions.pfnBotAddThreatEntity)
						g_BotFunctions.pfnBotAddThreatEntity((GameEntity)_entity->edict(), &info);
					return;
				}
			case Omnibot::kBackPack_Armor:
				{
					EntityInfo info;
					info.m_EntityClass = TF_CLASSEX_BACKPACK_ARMOR;
					info.m_EntityCategory.SetFlag(ENT_CAT_PICKUP);
					info.m_EntityCategory.SetFlag(ENT_CAT_STATIC);
					if(_entity->IsEffectActive( EF_NODRAW ))
						info.m_EntityFlags.SetFlag(ENT_FLAG_DISABLED);
					if(g_BotFunctions.pfnBotAddThreatEntity)
						g_BotFunctions.pfnBotAddThreatEntity((GameEntity)_entity->edict(), &info);
					return;
				}
			case Omnibot::kBackPack_Ammo:
				{
					EntityInfo info;
					info.m_EntityClass = TF_CLASSEX_BACKPACK_AMMO;
					info.m_EntityCategory.SetFlag(ENT_CAT_PICKUP);
					info.m_EntityCategory.SetFlag(ENT_CAT_STATIC);
					if(_entity->IsEffectActive( EF_NODRAW ))
						info.m_EntityFlags.SetFlag(ENT_FLAG_DISABLED);
					if(g_BotFunctions.pfnBotAddThreatEntity)
						g_BotFunctions.pfnBotAddThreatEntity((GameEntity)_entity->edict(), &info);
					return;
				}
			case Omnibot::kFlag:
				{
					gi.m_GoalType = TF_GOAL_FLAG;					
					break;
				}
			case Omnibot::kFlagCap:
				{
					gi.m_GoalTeam ^= iAllTeams;
					gi.m_GoalType = TF_GOAL_CAPPOINT;
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
				pfnPrintError("Omni-bot: Out of deferred goal slots!");
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void Notify_ItemDropped(CBaseEntity *_entity)
	{
		TriggerInfo ti;
		ti.m_TagName = _entity->GetName();
		ti.m_Action = "item_dropped";
		ti.m_Entity = _entity ? _entity->edict() : 0;
		ti.m_Activator = 0;
		omnibot_interface::Bot_SendTrigger(&ti);
	}
	void Notify_ItemPickedUp(CBaseEntity *_entity, CBaseEntity *_whodoneit)
	{
		TriggerInfo ti;
		ti.m_TagName = _entity->GetName();
		ti.m_Action = "item_pickedup";
		ti.m_Entity = _entity ? _entity->edict() : 0;
		ti.m_Activator = _whodoneit ? _whodoneit->edict() : 0;
		omnibot_interface::Bot_SendTrigger(&ti);
	}
	void Notify_ItemRespawned(CBaseEntity *_entity)
	{
		TriggerInfo ti;
		ti.m_TagName = _entity->GetName();
		ti.m_Action = "item_respawned";
		ti.m_Entity = _entity ? _entity->edict() : 0;
		ti.m_Activator = 0;
		omnibot_interface::Bot_SendTrigger(&ti);
	}
	void Notify_ItemReturned(CBaseEntity *_entity)
	{
		TriggerInfo ti;
		ti.m_TagName = _entity->GetName();
		ti.m_Action = "item_returned";
		ti.m_Entity = _entity ? _entity->edict() : 0;
		ti.m_Activator = 0;
		omnibot_interface::Bot_SendTrigger(&ti);
	}

	//////////////////////////////////////////////////////////////////////////
};
