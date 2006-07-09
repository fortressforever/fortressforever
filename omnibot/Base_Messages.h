////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-05-07 23:34:09 -0400 (Sun, 07 May 2006) $
// $LastChangedRevision: 1208 $
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
	int			m_Class;
} Msg_EntityClass;

typedef struct 
{
	int			m_Team;
} Msg_EntityTeam;

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
} Msg_GameState;

//////////////////////////////////////////////////////////////////////////

#pragma pack(pop)

#endif
