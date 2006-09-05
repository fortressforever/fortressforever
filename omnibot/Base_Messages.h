////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-09-04 09:36:01 -0700 (Mon, 04 Sep 2006) $
// $LastChangedRevision: 1265 $
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
	int			m_Kills;
	int			m_Deaths;
} Msg_EntityScore;

typedef struct 
{
	int			m_Team;
	int			m_Score;
} Msg_Score;

//////////////////////////////////////////////////////////////////////////

#pragma pack(pop)

#endif
