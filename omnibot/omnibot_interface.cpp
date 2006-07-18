//////////////////////////////////////////////////////////////////////////
// Bot-Related Includes
#include "cbase.h"
#include "ammodef.h"
#include "in_buttons.h"
#include "playerinfomanager.h"
#include "filesystem.h"
#include "ai_basenpc.h"

#include "ff_utils.h"

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
	float	color[3];
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
		// TF_WP_UMBRELLA,
		"ff_weapon_umbrella",
		// TF_WP_AXE,
		"",
		// TF_WP_CROWBAR,
		"ff_weapon_crowbar",
		// TF_WP_MEDKIT,
		"ff_weapon_medkit",
		// TF_WP_KNIFE,
		"ff_weapon_knife",
		// TF_WP_SPANNER,
		"ff_weapon_spanner",
		// TF_WP_SHOTGUN,
		"ff_weapon_shotgun",
		// TF_WP_SUPERSHOTGUN,
		"ff_weapon_supershotgun",
		// TF_WP_NAILGUN,
		"ff_weapon_nailgun",
		// TF_WP_SUPERNAILGUN,
		"ff_weapon_supernailgun",
		// TF_WP_GRENADE_LAUNCHER,
		"ff_weapon_grenadelauncher",
		// TF_WP_ROCKET_LAUNCHER,
		"ff_weapon_rpg",
		// TF_WP_SNIPER_RIFLE,
		"ff_weapon_sniperrifle",
		// TF_WP_RADIOTAG_RIFLE
		"ff_weapon_radiotagrifle",
		// TF_WP_RAILGUN,
		"ff_weapon_railgun",
		// TF_WP_FLAMETHROWER,
		"ff_weapon_flamethrower",
		// TF_WP_MINIGUN,
		"ff_weapon_assaultcannon",
		// TF_WP_AUTORIFLE,
		"ff_weapon_autorifle",
		// TF_WP_DARTGUN,
		"ff_weapon_tranquiliser",
		// TF_WP_PIPELAUNCHER,
		"ff_weapon_pipelauncher",
		// TF_WP_NAPALMCANNON,
		"ff_weapon_ic"
	};
	//
	int obUtilGetWeaponId(const char *_weaponName)
	{
		if(_weaponName)
		{
			for(int i = 1; i < TF_WP_MAX; ++i)
			{			
				if(!Q_strcmp(g_Weapons[i], _weaponName))
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

	static void obAddTempDisplayLine(const float _start[3], const float _end[3], const float _color[3])
	{
		Vector vStart(_start[0], _start[1], _start[2]);
		Vector vEnd(_end[0], _end[1], _end[2]);
		debugoverlay->AddLineOverlay(vStart, 
			vEnd, 
			_color[0] * 255, 
			_color[1] * 255, 
			_color[2] * 255, 
			false, 
			2.0);
	}

	//-----------------------------------------------------------------

	void obAddDisplayRadius(const float _pos[3], const float _radius, const float _color[3])
	{
		Vector pos(_pos[0], _pos[1], _pos[2] + 40);
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
				_color[0] * 255,
				_color[1] * 255,
				_color[2] * 255, 
				false,
				2.0f);
		}
	}

	//-----------------------------------------------------------------

	static void _AddDebugLineToDraw(LineType _type, const float _start[3], const float _end[3], const float _color[3])
	{
		debugLines_t line;
		line.type = _type;
		line.start[0] = _start[0];
		line.start[1] = _start[1];
		line.start[2] = _start[2];
		line.end[0] = _end[0];
		line.end[1] = _end[1];
		line.end[2] = _end[2];
		line.color[0] = _color[0];
		line.color[1] = _color[1];
		line.color[2] = _color[2];

		if(_type == LINE_BLOCKABLE)
		{		
			g_BlockableDebugLines.push_back(line);
		}
		else
		{
			g_DebugLines.push_back(line);
		}	
	}

	static void obAddDisplayLine(int _type, const float _start[3], const float _end[3], const float _color[3])
	{
		static float fStartOffset = 64.0f;
		static float fEndOffset = 0.0f;
		static float fStartPathOffset = 60.0f;
		static float fEndPathOffset = 40.0f;

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

	static void obPrintScreenText(const int _client, const float _pos[3], const float _color[3], const char *_msg)
	{	
		if(_msg && _pos)
		{
			Vector vPosition(_pos[0],_pos[1],_pos[2]);		
			debugoverlay->AddTextOverlay(vPosition, 2.0f, _msg);
		}
	}

	//-----------------------------------------------------------------

	static void obBotDoCommand(int _client, const char *_cmd)
	{
		edict_t *pEdict = INDEXENT(_client);
		assert(!FNullEnt(pEdict) && "Null Ent!");
		serverpluginhelpers->ClientCommand(pEdict, _cmd);
	}

	//-----------------------------------------------------------------

	static obResult obChangeTeam(int _client, int _newteam, const MessageHelper *_data)
	{
		edict_t *pEdict = INDEXEDICT(_client);

		if(pEdict)
		{
			CBaseEntity *pEntity = CBaseEntity::Instance( pEdict );
			CFFPlayer *pFFPlayer = dynamic_cast<CFFPlayer*>(pEntity);
			ASSERT(pFFPlayer);

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

	static obResult obGetEntityFlags(const GameEntity _ent, UserFlags64 &_entflags)
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

	static obResult obGetEntityPowerups(const GameEntity _ent, UserFlags64 &_powerups)
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
		assert(!FNullEnt(pEnt) && "Null Ent!");

		CBaseEntity *pEntity = CBaseEntity::Instance( (edict_t*)pEnt );
		if(pEntity)
		{
			CBasePlayer *pPlayer = pEntity->MyCharacterPointer();
			return pPlayer ? pPlayer->GetPlayerName() : 0;
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
		return ENTINDEX((edict_t*)_ent);
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

	static obResult obTraceLine(BotTraceResult *_result, const float _start[3], const float _end[3], 
		const AABB *_pBBox, int _mask, int _user, obBool _bUsePVS)
	{
		int iMask = 0;
		Ray_t ray;
		trace_t trace;

		// TODO: possibly use other types of filters?
		CTraceFilterWorldAndPropsOnly traceFilter;

		Vector start(_start[0],_start[1],_start[2]);
		Vector end(_end[0],_end[1],_end[2]);

		byte pvs[ MAX_MAP_CLUSTERS/8 ];
		int iPVSCluster = engine->GetClusterForOrigin(start);
		int iPVSLength = engine->GetPVSForCluster(iPVSCluster, sizeof(pvs), pvs);

		bool bInPVS = _bUsePVS ? engine->CheckOriginInPVS(end, pvs, iPVSLength) : true;

		if(bInPVS)
		{
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

			enginetrace->TraceRay(ray, iMask, &traceFilter, &trace);

			if(trace.DidHit() && trace.m_pEnt && (trace.m_pEnt->entindex() != 0))
				_result->m_HitEntity = trace.m_pEnt->edict();
			else
				_result->m_HitEntity = 0;

			// Fill in the bot traceflag.			
			_result->m_Fraction = trace.fraction;
			_result->m_StartSolid = trace.startsolid;
			_result->m_iUser2 = trace.contents;
			_result->m_Endpos[0] = trace.endpos.x;
			_result->m_Endpos[1] = trace.endpos.y;
			_result->m_Endpos[2] = trace.endpos.z;
			_result->m_Normal[0] = trace.plane.normal.x;
			_result->m_Normal[1] = trace.plane.normal.y;
			_result->m_Normal[2] = trace.plane.normal.z;	

			return Success;
		}

		// No Hit or Not in PVS
		_result->m_Fraction = 0.0f;
		_result->m_HitEntity = 0;	

		return bInPVS ? Success : OutOfPVS;
	}

	//-----------------------------------------------------------------

	static GameEntity obFindEntityByClassName(GameEntity _pStart, const char *_name)
	{
		if(!_pStart)
			return INDEXENT(1);
		else
			return 0;

		// TODO: FIX THIS
		int iStartIndex = _pStart ? ENTINDEX((edict_t*)_pStart) : 1;
		int iNumEntities = engine->GetEntityCount();
		for( ; iStartIndex <= iNumEntities; ++iStartIndex)
		{
			edict_t *pEdict = INDEXENT(iStartIndex);
			if(!FNullEnt(pEdict) && !Q_stricmp(pEdict->GetClassName(), _name))
				return pEdict;
		}
		return 0;//(GameEntity)G_Find((gentity_t*)(*_pStart), FOFS(classname), _name);
	}

	//-----------------------------------------------------------------

	static obResult obGetThreats()
	{
		EntityInfo info;

		for ( CBaseEntity *pEntity = gEntList.FirstEnt(); pEntity != NULL; pEntity = gEntList.NextEnt(pEntity) )
		{
			// default data.
			info.m_EntityFlags.ClearAll();
			info.m_EntityCategory = 0;
			info.m_EntityClass = TF_CLASS_NONE;
			
			switch(pEntity->Classify())
			{
			case CLASS_PLAYER:
			case CLASS_PLAYER_ALLY:
				{
					CFFPlayer *pFFPlayer = static_cast<CFFPlayer*>(pEntity);
					ASSERT(pFFPlayer);
					info.m_EntityClass = obUtilGetBotClassFromGameClass(pFFPlayer->GetClassSlot());				
					info.m_EntityCategory = ENT_CAT_SHOOTABLE | ENT_CAT_PLAYER;
					obGetEntityFlags(pFFPlayer->edict(), info.m_EntityFlags);
					break;
				}
			case CLASS_DISPENSER:
				info.m_EntityClass = TF_CLASSEX_DISPENSER;
				info.m_EntityCategory = ENT_CAT_SHOOTABLE;
				break;
			case CLASS_SENTRYGUN:
				info.m_EntityClass = TF_CLASSEX_SENTRY;
				info.m_EntityCategory = ENT_CAT_SHOOTABLE;
				break;
			case CLASS_DETPACK:
				info.m_EntityCategory = ENT_CAT_PROJECTILE | ENT_CAT_AVOID;
				info.m_EntityClass = TF_CLASSEX_DETPACK;
				break;
			case CLASS_GREN:
				info.m_EntityCategory = ENT_CAT_PROJECTILE | ENT_CAT_AVOID;
				info.m_EntityClass = TF_CLASSEX_GRENADE;
				break;
			case CLASS_GREN_EMP:
				info.m_EntityCategory = ENT_CAT_PROJECTILE | ENT_CAT_AVOID;
				info.m_EntityClass = TF_CLASSEX_EMP_GRENADE;
				break;
			case CLASS_GREN_NAIL:
				info.m_EntityCategory = ENT_CAT_PROJECTILE | ENT_CAT_AVOID;
				info.m_EntityClass = TF_CLASSEX_NAIL_GRENADE;
				break;
			case CLASS_GREN_MIRV:
				info.m_EntityCategory = ENT_CAT_PROJECTILE | ENT_CAT_AVOID;
				info.m_EntityClass = TF_CLASSEX_MIRV_GRENADE;
				break;
			case CLASS_GREN_MIRVLET:
				info.m_EntityCategory = ENT_CAT_PROJECTILE | ENT_CAT_AVOID;
				info.m_EntityClass = TF_CLASSEX_MIRVLET_GRENADE;
				break;
			case CLASS_GREN_NAPALM:
				info.m_EntityCategory = ENT_CAT_PROJECTILE | ENT_CAT_AVOID;
				info.m_EntityClass = TF_CLASSEX_NAPALM_GRENADE;
				break;
			case CLASS_GREN_GAS:
				info.m_EntityCategory = ENT_CAT_PROJECTILE | ENT_CAT_AVOID;
				info.m_EntityClass = TF_CLASSEX_GAS_GRENADE;
				break;
			case CLASS_GREN_CONC:
				info.m_EntityCategory = ENT_CAT_PROJECTILE | ENT_CAT_AVOID;
				info.m_EntityClass = TF_CLASSEX_CONC_GRENADE;
				break;
			case CLASS_GREN_CALTROP:
				info.m_EntityCategory = ENT_CAT_PROJECTILE | ENT_CAT_AVOID;
				info.m_EntityClass = TF_CLASSEX_CALTROP;
				break;
			case CLASS_PIPEBOMB:
				info.m_EntityCategory = ENT_CAT_PROJECTILE | ENT_CAT_AVOID;
				info.m_EntityClass = TF_CLASSEX_PIPE;
				break;
				// TODO: rocket, gl grenade
			default:
				continue;
			}

			if(g_BotFunctions.pfnBotAddThreatEntity)
				g_BotFunctions.pfnBotAddThreatEntity((GameEntity)pEntity->edict(), &info);
		}
		return Success;
	}

	//-----------------------------------------------------------------

	static obResult obGetEntityPosition(const GameEntity _ent, float _pos[3])
	{	
		edict_t *pEdict = (edict_t *)_ent;
		CBaseEntity *pEntity = pEdict ? CBaseEntity::Instance( pEdict ) : 0;
		assert(pEntity && "Null Ent!");
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
		assert(pEntity && "Null Ent!");
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
		assert(pEntity && "Null Ent!");
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
		assert(pEntity && "Null Ent!");
		if(pEntity)
		{
			QAngle viewAngles = pEntity->EyeAngles();
			AngleVectors(viewAngles, (Vector*)_fwd, (Vector*)_right, (Vector*)_up);
			return Success;
		}
		return InvalidEntity;
	}

	//-----------------------------------------------------------------

	static GameEntity obFindEntityInSphere(const float _pos[3], float _radius, GameEntity _pStart, const char *_name)
	{
		// square it to avoid the square root in the distance check.
		float sqRad = _radius * _radius;

		Vector start(_pos[0], _pos[1], _pos[2]);
		float edict_pos[3];

		int iStartIndex = _pStart ? ENTINDEX((edict_t*)_pStart) : 0;
		for( ; iStartIndex <= engine->GetEntityCount(); ++iStartIndex)
		{
			edict_t *pEdict = INDEXENT(iStartIndex);
			if(!Q_stricmp(pEdict->GetClassName(), _name))
			{
				obGetEntityPosition(pEdict, edict_pos);
				if(start.DistToSqr(Vector(edict_pos[0],edict_pos[1],edict_pos[2])) < sqRad)
					return pEdict;
			}
		}

		return 0;
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
		assert(pEntity && "Null Ent!");
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
				if(pEntity->entindex() == 0)
					return InvalidEntity;
				vMins = pEntity->WorldAlignMins();
				vMaxs = pEntity->WorldAlignMaxs();
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
		assert(pEntity && "Null Ent!");
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
		// todo: get bone position based on bone id
		//idEntity *pEnt = (idEntity*)_ent;
		//idPlayer.GetAnimator()
		return obGetEntityPosition(_ent, _pos);
	}

	//-----------------------------------------------------------------

	static int obGetEntityOwner(const GameEntity _ent)
	{
		// TODO:
		//gentity_t *pEnt = (gentity_t *)(*_ent);
		//// -1 means theres no owner.
		//return ((pEnt->s.otherEntityNum == MAX_CLIENTS) ? -1 : pEnt->s.otherEntityNum);
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
		case GEN_MSG_GETPOINTCONTENTS:
			{
				Msg_PointContents *pMsg = _data.Get<Msg_PointContents>();
				if(pMsg)
				{
					// TODO:
					pMsg->m_Contents = 0;
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
		case GEN_MSG_GETCURRENTCLASS:
			{
				Msg_EntityClass *pMsg = _data.Get<Msg_EntityClass>();
				if(pMsg)
				{
					pMsg->m_Class = 0;
				}
				break;
			}
		case GEN_MSG_GETCURRENTTEAM:
			{
				Msg_EntityTeam *pMsg = _data.Get<Msg_EntityTeam>();
				if(pMsg)
				{
					pMsg->m_Team = pEnt ? obUtilGetBotTeamFromGameTeam(pEnt->GetTeamNumber()) : 0;;
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
				if(pMsg)
				{
					pMsg->m_NumPipes = 0;
					pMsg->m_MaxPipes = 0;
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
					pMsg->m_MaxPipesPerPiper = 0;
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

			// doesn't have clips.
			*_curclip = pWeapon->Clip1();
			*_maxclip = pWeapon->GetMaxClip1();

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
			//		assert(0);		// |-- Mirv: Melee weapons are breaking this
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
		assert(!FNullEnt(pEdict) && "Null Ent!");
		CBaseEntity *pEntity = CBaseEntity::Instance(pEdict);
		CBasePlayer *pPlayer = pEntity ? pEntity->MyCharacterPointer() : 0;
		if(pPlayer && pPlayer->IsBot())
		{
			CBotCmd cmd;

			// Process the bot keypresses.
			if(BOT_CHECK_BUTTON(_input->m_ButtonFlags, BOT_BUTTON_ATTACK1))
				cmd.buttons |= IN_ATTACK;
			if(BOT_CHECK_BUTTON(_input->m_ButtonFlags, BOT_BUTTON_WALK))
				cmd.buttons |= IN_RUN;
			if(BOT_CHECK_BUTTON(_input->m_ButtonFlags, BOT_BUTTON_USE))
				cmd.buttons |= IN_USE;
			if(BOT_CHECK_BUTTON(_input->m_ButtonFlags, BOT_BUTTON_JUMP))
				cmd.buttons |= IN_JUMP;
			if(BOT_CHECK_BUTTON(_input->m_ButtonFlags, BOT_BUTTON_CROUCH))
				cmd.buttons |= IN_DUCK;
			if(BOT_CHECK_BUTTON(_input->m_ButtonFlags, BOT_BUTTON_RELOAD))
				cmd.buttons |= IN_RELOAD;

			// Store the facing.
			Vector vFacing(_input->m_Facing[0], _input->m_Facing[1], _input->m_Facing[2]);
			VectorAngles(vFacing, cmd.viewangles);

			// Calculate the movement vector, taking into account the view direction.
			Vector vForward, vRight, vUp;
			Vector vMoveDir(_input->m_MoveDir[0],_input->m_MoveDir[1],_input->m_MoveDir[2]);
			AngleVectors(cmd.viewangles, &vForward, &vRight, &vUp);

			cmd.forwardmove = vForward.Dot(vMoveDir) * 600.0f;
			cmd.sidemove = vRight.Dot(vMoveDir) * 600.0f;
			cmd.upmove = vUp.Dot(vMoveDir) * 600.0f;

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
			pPlayer->GetBotController()->PostClientMessagesSent();
		}
	}

	//-----------------------------------------------------------------

	static void obGetMapExtents(AABB *_aabb)
	{		
		memset(_aabb, 0, sizeof(AABB));
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
						obAddTempDisplayLine(g_DebugLines[i].start, g_DebugLines[i].end, g_DebugLines[i].color);

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
						obAddTempDisplayLine(g_DebugLines[i].start, (float*)&vLineEnd, g_DebugLines[i].color);
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
				g_BotFunctions.pfnBotConsoleCommand(engine->Cmd_Args(), strlen(engine->Cmd_Args()));
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
			else
			{		
				obPrintMessage( "Omni-bot Not Loaded\n" );
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
			else
			{		
				obPrintMessage( "Omni-bot Not Loaded\n" );
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
			else
			{		
				obPrintMessage( "Omni-bot Not Loaded\n" );
			}
		}
	}

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
		g_InterfaceFunctions.pfnPrintEntitiesInRadius		= obPrintEntitiesInRadius;

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
			Bot_Interface_SendGlobalEvent(GAME_ID_ENDGAME, -1, 0, 0);
			g_BotFunctions.pfnBotShutdown();
			memset(&g_BotFunctions, 0, sizeof(g_BotFunctions));
			obPrintMessage( "Omni-bot Shut Down Successfully\n" );
			obPrintMessage( "---------------------------------------------\n" );
		}
		else
		{
			obPrintMessage( "Omni-bot Not Loaded\n" );
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
					g_BotFunctions.pfnBotUpdate();
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Message Helpers

	void Notify_ClientConnected(CBasePlayer *_player, bool _isbot)
	{
		int iGameId = _player->entindex()-1;
		if (_isbot)
			omnibot_interface::Bot_Interface_SendGlobalEvent(GAME_ID_BOTCONNECTED, iGameId, 100, NULL);
		else
			omnibot_interface::Bot_Interface_SendGlobalEvent(GAME_ID_CLIENTCONNECTED, iGameId, 100, NULL);
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

	void Notify_Death(CBasePlayer *_player, edict_t *_attacker)
	{
		int iGameId = _player->entindex()-1;
		BotUserData bud((GameEntity)_attacker);
		omnibot_interface::Bot_Interface_SendEvent(MESSAGE_DEATH, iGameId, 0, 0, &bud);
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

	//////////////////////////////////////////////////////////////////////////
};
