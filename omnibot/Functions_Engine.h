////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-09-28 21:48:58 -0700 (Thu, 28 Sep 2006) $
// $LastChangedRevision: 1296 $
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __FUNCTIONS_ENGINE_H__
#define __FUNCTIONS_ENGINE_H__

#include "Omni-Bot_Types.h"
#include "Omni-Bot_BitFlags.h"
#include "Omni-Bot_Color.h"
#include "MessageHelper.h"

// Title: Functions Engine

// struct: EntityInfo
//		Used to store information about an entity
typedef struct 
{
	// int: m_EntityClass
	//		The specific classification of this entity
	int			m_EntityClass;	
	// BitFlag64: m_EntityCategoty
	//		Current category of this entity, see <EntityCategory>
	BitFlag64	m_EntityCategory;
	// BitFlag64: m_EntityFlags
	//		Current flags of this entity, see <EntityFlags>
	BitFlag64	m_EntityFlags;
	// BitFlag64: m_EntityPowerups
	//		Current power-ups of this entity, see <Powerups>
	BitFlag64	m_EntityPowerups;
} EntityInfo;


// struct: ClientInput
//		Generic data structure representing the bots input and movement states
//		Game is responsible for translating this into a format suitable for use
//		by the game.
typedef struct
{
	// float: m_Facing
	//		The direction the bot is facing/aiming
	float		m_Facing[3];
	// float: m_MoveDir
	//		The direction the bot is moving
	float		m_MoveDir[3];
	// int: m_ButtonFlags
	//		32 bit int of bits representing bot keypresses, see <ButtonFlags>
	BitFlag32	m_ButtonFlags;
	// int: m_CurrentWeapon
	//		The current weapon Id this bot wants to use.
	int			m_CurrentWeapon;
} ClientInput;

// class: BotTraceResult
//		This file defines all the common structures used by the game and bot alike.
class BotTraceResult
{
public:
	// float: m_Fraction
	//		0.0 - 1.0 how far the trace went
	float		m_Fraction;
	// float: m_Normal
	//		The plane normal that was struck
	float		m_Normal[3];
	// float: m_Endpos
	//		The end point the trace ended at
	float		m_Endpos[3];
	// var: m_HitEntity
	//		The entity that was hit by the trace
	GameEntity	m_HitEntity;
	// int: m_StartSolid
	//		Did the trace start inside a solid?
	int			m_StartSolid;
	// int: m_Contents
	//		Content flags.
	int			m_Contents;
	// int: m_iUser1
	//		Extra user info from the trace
	int			m_iUser1;

	BotTraceResult() : 
		m_Fraction	(0.f),
		m_HitEntity	(0),
		m_StartSolid(0),
		m_Contents	(0),
		m_iUser1	(0)
	{
	}
};

// typedef: Game_EngineFuncs_t
//		This struct defines all the function pointers that the
//		game will fill in and give to the bot so that the bot may perform generic
//		actions without caring about the underlying engine or game. It is 
//		_ABSOLUTELY REQUIRED_ that the game not leave any of the function pointers
//		set to null, even if that means using empty functions in the interface
//		to set the pointers.
typedef struct 
{
	// Function: pfnAddBot
	//		This function should add a bot to the game with the name specified,
	//		and return the bots GameID
	int (*pfnAddBot)(const char *_name, const MessageHelper *_data);
	
	// Function: pfnRemoveBot
	//		This function should remove/kick a bot from the game by its name
	int (*pfnRemoveBot)(const char *_name);

	// Function: pfnChangeTeam
	//		This function should force a bot to a certain team
	obResult (*pfnChangeTeam)(int _client, int _newteam, const MessageHelper *_data);

	// Function: pfnChangeClass
	//		This function should force a bot to change to a certain class
	obResult (*pfnChangeClass)(int _client, int _newclass, const MessageHelper *_data);

	// Function: pfnUpdateBotInput
	//		This function should interpret and handle the bots input
	void (*pfnUpdateBotInput)(int _client, const ClientInput *_input);

	// Function: pfnBotCommand
	//		This function should perform a bot 'console command'
	void (*pfnBotCommand)(int _client, const char *_cmd);

	// Function: pfnTraceLine
	//		This bot should intepret and perform a traceline, returning
	//		the results into the <BotTraceResult> parameter.
	obResult (*pfnTraceLine)(BotTraceResult *_result, const float _start[3], const float _end[3], const AABB *_pBBox , int _mask, int _user, obBool _bUsePVS);

	// Function: pfnGetPointContents
	//		Gets the content bitflags for a location.
	int (*pfnGetPointContents)(const float _pos[3]);

	// Function: pfnFindEntityByClassName
	//		This function should return entities matching the classname, and should be
	//		compatible with a while loop using the _pStart and returning 0 at the end of search
	GameEntity (*pfnFindEntityByClassName)(GameEntity _pStart, const char *_name);
	
	// Function: pfnFindEntityByClassId
	//		This function should return entities matching the class id, and should be
	//		compatible with a while loop using the _pStart and returning 0 at the end of search
	GameEntity (*pfnFindEntityByClassId)(GameEntity _pStart, int _classId);

	// Function: pfnFindEntityInSphere
	//		This function should return entities matching the classname, and in a radius, and should be
	//		compatible with a while loop using the _pStart and returning 0 at the end of search
	GameEntity (*pfnFindEntityInSphere)(const float _pos[3], float _radius, GameEntity _pStart, const char *_name);

	// Function: pfnFindEntityInSphereId
	//		This function should return entities matching the class id, and in a radius, and should be
	//		compatible with a while loop using the _pStart and returning 0 at the end of search
	GameEntity (*pfnFindEntityInSphereId)(const float _pos[3], float _radius, GameEntity _pStart, int classId);

	// Function: pfnGetEntityFlags
	//		This function should return the entity flags for an entity
	obResult (*pfnGetEntityFlags)(const GameEntity _ent, BitFlag64 &_flags);

	// Function: pfnGetEntityPowerups
	//		This function should return the powerup flags for an entity.
	obResult (*pfnGetEntityPowerups)(const GameEntity _ent, BitFlag64 &_flags);

	// Function: pfnGetEntityEyePosition
	//		This function should return the eye position of an entity
	obResult (*pfnGetEntityEyePosition)(const GameEntity _ent, float _pos[3]);

	// Function: pfnGetEntityEyePosition
	//		This function should return the bone position of an entity
	obResult (*pfnGetEntityBonePosition)(const GameEntity _ent, int _boneid, float _pos[3]);

	// Function: pfnGetEntityOrientation
	//		This function should return the orientation of a <GameEntity> as fwd, right, up vectors
	obResult (*pfnGetEntityOrientation)(const GameEntity _ent, float _fwd[3], float _right[3], float _up[3]);
	
	// Function: pfnGetEntityVelocity
	//		This function should return the velocity of a <GameEntity> in world space
	obResult (*pfnGetEntityVelocity)(const GameEntity _ent, float _velocity[3]);

	// Function: pfnGetEntityPosition
	//		This function should return the position of a <GameEntity> in world space
	obResult (*pfnGetEntityPosition)(const GameEntity _ent, float _pos[3]);

	// Function: pfnGetEntityWorldAABB
	//		This function should return the axis aligned box of a <GameEntity> in world space
	obResult (*pfnGetEntityWorldAABB)(const GameEntity _ent, AABB *_aabb);

	// Function: pfnGetEntityOwner
	//		This function should return the <GameID> of a client that owns this item
	int (*pfnGetEntityOwner)(const GameEntity _ent);

	// Function: pfnGetEntityTeam
	//		This function should return the bot team of the entity.
	int (*pfnGetEntityTeam)(const GameEntity _ent);

	// Function: pfnGetEntityClass
	//		This function should return the bot class of the entity.
	int (*pfnGetEntityClass)(const GameEntity _ent);

	// Function: pfnEntityFromID
	//		This function should return the <GameEntity> that matches the provided Id
	GameEntity (*pfnEntityFromID)(const int _id);

	// Function: pfnIDFromEntity
	//		This function should return the Id that matches the provided <GameEntity>
	int (*pfnIDFromEntity)(const GameEntity _ent);

	// Function: pfnGetClientPosition
	//		This function should return the position of a client in world space
	obResult (*pfnGetClientPosition)(int _client, float _pos[3]);
	
	// Function: pfnGetClientOrientation
	//		This function should return the orientation of a <GameEntity> as fwd, right, up vectors
	obResult (*pfnGetClientOrientation)(int _client, float _fwd[3], float _right[3], float _up[3]);

	// Function: pfnGetClientName
	//		This function should give access to the in-game name of the client
	const char *(*pfnGetClientName)(int _client);

	// Function: pfnBotGetCurrentWeaponClip
	//		This function should update weapon clip count for the current weapon
	obResult (*pfnBotGetCurrentWeaponClip)(int _client, int *_curclip, int *_maxclip);

	// Function: pfnBotGetCurrentAmmo
	//		This function should update ammo stats for a client and ammotype
	obResult (*pfnBotGetCurrentAmmo)(int _client, int _ammotype, int *_cur, int *_max);

	// Function: pfnGetGameTime
	//		This function should return the current game time in milli-seconds
	int (*pfnGetGameTime)();

	// Function: pfnGetGoals
	//		This function should tell the game to register all <MapGoal>s with the bot
	obResult (*pfnGetGoals)();

	// Function: pfnGetThreats
	//		This function should tell the game to register all potential threats with the bot
	//		Threats include potential targets, projectiles, or other entities of interest to the bot
	obResult (*pfnGetThreats)();

	// Function: pfnGetMaxNumPlayers
	//		Gets the currently set maximum number of players from the game
	int (*pfnGetMaxNumPlayers)();

	// Function: pfnGetCurNumPlayers
	//		Gets the current number of players from the game. Combine with above?
	int (*pfnGetCurNumPlayers)();

	// Function: pfnInterfaceSendMessage
	//		This function sends a message to the game with optional <MessageHelper> in/out parameters
	//		to request additional or mod specific info from the game
	obResult (*pfnInterfaceSendMessage)(const MessageHelper &_data, const GameEntity _ent);

	// Function: pfnAddDisplayPath
	//		Adds a line to display between 2 positions, with a specific color, and type that determines how it is drawn
	void (*pfnAddDisplayLine)(int _type, const float _start[3], const float _end[3], const obColor &_color);

	// Function: pfnAddTempDisplayLine
	//		Adds a line to immediately display between 2 positions, with a specific color
	void (*pfnAddTempDisplayLine)(const float _start[3], const float _end[3], const obColor &_color, float _time);

	// Function: pfnAddDisplayRadius
	//		Adds a radius indicator to be displayed at a certain position with radius and color
	void (*pfnAddDisplayRadius)(const float _pos[3], const float _radius, const obColor &_color, float _time);

	// Function: pfnClearDebugLines
	//		This function tells the interface to potentially clear prior navigation or radius indicators
	void (*pfnClearDebugLines)(obBool _navViewEnabled, const BitFlag32 &_flags);

	// Function: pfnPrintError
	//		This function should print an error the the game however desired,
	//		whether it be to the console, messagebox,...
	void (*pfnPrintError)(const char *_error);
	
	// Function: pfnPrintMessage
	//		This function should print a message the the game however desired,
	//		whether it be to the console, messagebox,...
	void (*pfnPrintMessage)(const char *_msg);

	// Function: pfnPrintScreenMessage
	//		This function should print a message the the game screen if possible
	void (*pfnPrintScreenText)(const int _client, const float _pos[3], float _duration, const obColor &_color, const char *_msg);

	// Function: pfnGetMapName
	//		This function should give access to the name of the currently loaded map
	const char *(*pfnGetMapName)();

	// Function: pfnGetMapExtents
	//		This function gets the extents of the current map.
	void (*pfnGetMapExtents)(AABB *_aabb);

	// Function: pfnGetGameName
	//		This function should give access to the name of the currently loaded game
	const char *(*pfnGetGameName)();

	// Function: pfnGetModName
	//		This function should give access to the name of the currently loaded mod
	const char *(*pfnGetModName)();

	// Function: pfnGetModVers
	//		This function should give access to the version of the currently loaded mod
	const char *(*pfnGetModVers)();

	// Function: pfnGetBotPath
	//		This function should get the bot path to the bot dll and base path to supplemental files.
	const char *(*pfnGetBotPath)();

} Game_EngineFuncs_t;

#endif
