////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-07-20 08:59:11 -0700 (Thu, 20 Jul 2006) $
// $LastChangedRevision: 1234 $
//
// Title: TF Message Structure Definitions
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
	GameEntity			m_Sentry;
	GameEntity			m_Dispenser;
	GameEntity			m_Detpack;
} TF_BuildInfo;

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

typedef struct  
{
	enum GuiType
	{
		GuiAlert,
		GuiMenu,
		GuiTextBox
	};
	GameId		m_TargetPlayer;
	GuiType		m_MenuType;
	char		m_Title[32];
	char		m_Caption[32];
	char		m_Message[512];
	char		m_Option[10][64];
	char		m_Command[10][64];
	int			m_Level;
	float		m_TimeOut;
	obColor		m_Color;
} TF_HudMenu;

typedef struct  
{
	GameEntity	m_TargetPlayer;
	obBool		m_Lock;
	obBool		m_Succeeded;
} TF_LockPosition;

//////////////////////////////////////////////////////////////////////////

#pragma pack(pop)

#endif
