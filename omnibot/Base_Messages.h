////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-11-16 21:34:50 -0800 (Thu, 16 Nov 2006) $
// $LastChangedRevision: 1320 $
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __BASE_MESSAGES_H__
#define __BASE_MESSAGES_H__

#include "Omni-Bot_Types.h"

#pragma pack(push)
#pragma pack(4)

//////////////////////////////////////////////////////////////////////////

typedef struct 
{
	int			m_CurrentHealth;
	int			m_MaxHealth;
	int			m_CurrentArmor;
	int			m_MaxArmor;
} Msg_PlayerHealthArmor;

typedef struct 
{
	float		m_MaxSpeed;
} Msg_PlayerMaxSpeed;

typedef struct 
{	
	obBool		m_IsAlive;
} Msg_IsAlive;

typedef struct 
{
	GameEntity	m_TargetEntity;
	obBool		m_IsAllied;
} Msg_IsAllied;

typedef struct 
{
	obBool		m_IsHuman;
} Msg_IsHuman;

typedef struct 
{
	int			m_Contents;
	float		x,y,z;
} Msg_PointContents;

typedef struct 
{
	int			m_Weapon;
} Msg_EquippedWeapon;

typedef struct 
{
	obBool		m_Ready;
} Msg_ReadyToFire;

typedef struct 
{
	obBool		m_Reloading;
} Msg_Reloading;

typedef struct 
{
	FlagState	m_FlagState;
	GameEntity	m_Owner;
} Msg_FlagState;

typedef struct 
{
	GameState	m_GameState;
	float		m_TimeLeft;
} Msg_GameState;

typedef struct 
{
	char		m_StatName[64];
	BotUserData m_Result;
} Msg_EntityStat;

typedef struct 
{
	int			m_Team;
	char		m_StatName[64];
	BotUserData m_Result;
} Msg_TeamStat;

typedef struct 
{
	int			m_Weapon;
	FireMode	m_FireMode;
	obBool		m_IsCharged;
} WeaponCharged;

typedef struct 
{
	FireMode	m_FireMode;
	float		m_CurrentHeat;
	float		m_MaxHeat;
} WeaponHeatLevel;

//////////////////////////////////////////////////////////////////////////

#pragma pack(pop)

#endif
