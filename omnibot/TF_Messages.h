////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-02-07 23:16:24 -0500 (Tue, 07 Feb 2006) $
// $LastChangedRevision: 1140 $
//
// Title: ET Message Structure Definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __ET_MESSAGES_H__
#define __ET_MESSAGES_H__

#include "Base_Messages.h"

#pragma pack(push)
#pragma pack(4)

//////////////////////////////////////////////////////////////////////////

typedef struct 
{
	obBool		m_IsCharging;
} TF_WeaponCharging;

typedef struct 
{
	TF_BuildingStatus		m_Building;
} TF_Building;

typedef struct 
{
	GameEntity	m_Sentry;
	GameEntity	m_Dispenser;
	GameEntity	m_Detpack;
} TF_Buildables;

typedef struct 
{
	obint32		m_NumPipes;
	obint32		m_MaxPipes;
} TF_PlayerPipeCount;

typedef struct  
{
	obint32		m_NumTeamPipes;
	obint32		m_NumTeamPipers;
	obint32		m_MaxPipesPerPiper;
} TF_TeamPipeInfo;

typedef struct  
{
	obint32		m_DisguiseTeam;
	obint32		m_DisguiseClass;
} TF_Disguise;

typedef struct  
{
	obBool		m_SilentFeign;
} TF_FeignDeath;

typedef struct  
{
	GameId		m_TargetPlayer;
	obint32		m_Id;
	char		m_Message[1024];
} TF_HudHint;

//////////////////////////////////////////////////////////////////////////

#pragma pack(pop)

#endif
