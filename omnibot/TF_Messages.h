////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-08-03 00:47:21 -0700 (Thu, 03 Aug 2006) $
// $LastChangedRevision: 1241 $
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

// struct: TF_WeaponCharging
//		m_IsCharging - true if the weapon is charging.
typedef struct 
{
	obBool		m_IsCharging;
} TF_WeaponCharging;

// struct: TF_BuildInfo
//		m_Sentry - Sentry Entity.
//		m_Dispenser - Dispenser Entity.
//		m_Detpack - Detpack Entity.
typedef struct 
{
	GameEntity			m_Sentry;
	GameEntity			m_Dispenser;
	GameEntity			m_Detpack;
} TF_BuildInfo;

// struct: TF_PlayerPipeCount
//		m_NumPipes - Current number of player pipes.
//		m_MaxPipes - Max player pipes.
typedef struct 
{
	obint32		m_NumPipes;
	obint32		m_MaxPipes;
} TF_PlayerPipeCount;

// struct: TF_TeamPipeInfo
//		m_NumTeamPipes - Current number of team pipes.
//		m_NumTeamPipers - Current number of team pipers(demo-men).
//		m_MaxPipesPerPiper - Max pipes per piper
typedef struct  
{
	obint32		m_NumTeamPipes;
	obint32		m_NumTeamPipers;
	obint32		m_MaxPipesPerPiper;
} TF_TeamPipeInfo;

// struct: TF_Disguise
//		m_DisguiseTeam - Team disguised as.
//		m_DisguiseClass - Class disguised as.
typedef struct  
{
	obint32		m_DisguiseTeam;
	obint32		m_DisguiseClass;
} TF_Disguise;

// struct: TF_FeignDeath
//		m_SilentFeign - Silent feign or not.
typedef struct  
{
	obBool		m_SilentFeign;
} TF_FeignDeath;

// struct: TF_HudHint
//		m_TargetPlayer - Target player entity for the hint.
//		m_Id - Id for the hint.
//		m_Message[1024] - Hint message.
typedef struct  
{
	GameId		m_TargetPlayer;
	obint32		m_Id;
	char		m_Message[1024];
} TF_HudHint;

// struct: TF_HudMenu
//		m_TargetPlayer - Target player entity for the hint.
//		m_MenuType - The type of menu.
//		m_Title[32] - Title of the menu.
//		m_Caption[32] - Caption of the menu.
//		m_Message[512] - Message of the menu.
//		m_Option[10][64] - Array of options, max 10.
//		m_Command[10][64] - Array of commands, max 10.
//		m_Level - Menu level.
//		m_TimeOut - Duration of the menu.
//		m_Color - Text color.
typedef struct  
{
	enum GuiType
	{
		GuiAlert,
		GuiMenu,
		GuiTextBox,
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

// struct: TF_HudText
//		m_TargetPlayer - Target player entity for the message.
//		m_Message - Text to display.
typedef struct  
{
	enum MsgType
	{
		MsgConsole,
		MsgHudCenter,
	};
	GameId		m_TargetPlayer;
	MsgType		m_MessageType;
	char		m_Message[512];
} TF_HudText;

// struct: TF_LockPosition
//		m_TargetPlayer - Target player entity for the hint.
//		m_Lock - Lock the player or not.
//		m_Succeeded - Status result.
typedef struct  
{
	GameEntity	m_TargetPlayer;
	obBool		m_Lock;
	obBool		m_Succeeded;
} TF_LockPosition;

//////////////////////////////////////////////////////////////////////////

#pragma pack(pop)

#endif
