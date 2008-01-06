////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: drevil $
// $LastChangedDate: 2007-12-06 08:58:50 -0800 (Thu, 06 Dec 2007) $
// $LastChangedRevision: 2251 $
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __BASE_MESSAGES_H__
#define __BASE_MESSAGES_H__

#include "Omni-Bot_Types.h"

#pragma pack(push)
#pragma pack(4)

//////////////////////////////////////////////////////////////////////////

struct Msg_Addbot
{
	int			m_Team;
	int			m_Class;
	char		m_Name[64];
	char		m_Model[64];
	char		m_Skin[64];
	char		m_SpawnPointName[64];
	char		m_Profile[64];

	Msg_Addbot()
		: m_Team(RANDOM_TEAM_IF_NO_TEAM)
		, m_Class(RANDOM_CLASS_IF_NO_CLASS)
	{
		m_Name[0] = m_Model[0] = m_Skin[0] = m_SpawnPointName[0] = m_Profile[0] = 0;
	}
};

struct Msg_ChangeName
{
	char	m_NewName[64];
};

struct Msg_PlayerChooseEquipment
{
	enum { NumItems = 16 };
	int			m_WeaponChoice[NumItems];
	int			m_ItemChoice[NumItems];
};

struct Msg_PlayerHealthArmor
{
	int			m_CurrentHealth;
	int			m_MaxHealth;
	int			m_CurrentArmor;
	int			m_MaxArmor;
};

struct Msg_PlayerMaxSpeed
{
	float		m_MaxSpeed;
};

struct Msg_IsAlive
{	
	obBool		m_IsAlive;
};

struct Msg_IsAllied
{
	GameEntity	m_TargetEntity;
	obBool		m_IsAllied;
};

struct Msg_IsOutside
{
	float		m_Position[3];
	obBool		m_IsOutside;
};

struct Msg_PointContents
{
	int			m_Contents;
	float		x,y,z;
};

struct Msg_ReadyToFire
{
	obBool		m_Ready;
};

struct Msg_Reloading
{
	obBool		m_Reloading;
};

struct Msg_FlagState
{
	FlagState	m_FlagState;
	GameEntity	m_Owner;
};

struct Msg_GameState
{
	GameState	m_GameState;
	float		m_TimeLeft;
};

struct Msg_EntityStat
{
	char		m_StatName[64];
	obUserData m_Result;
};

struct Msg_TeamStat
{
	int			m_Team;
	char		m_StatName[64];
	obUserData	m_Result;
};

struct Msg_ServerCommand
{
	char		m_Command[256];
};

struct WeaponCharging
{
	int			m_Weapon;
	FireMode	m_FireMode;
	obBool		m_IsCharged;
};

struct WeaponCharged
{
	int			m_Weapon;
	FireMode	m_FireMode;
	obBool		m_IsCharged;
};

struct WeaponHeatLevel
{
	FireMode	m_FireMode;
	float		m_CurrentHeat;
	float		m_MaxHeat;
};

struct TurretProperties
{
	float		m_DefaultAimVector[3];
	float		m_CurrentAimVector[3];
	float		m_MinYaw;
	float		m_MaxYaw;
	float		m_MinPitch;
	float		m_MaxPitch;
	int			m_WeaponId;
};

struct VehicleInfo
{
	int			m_Type;
	GameEntity	m_Entity;
	GameEntity	m_Weapon;
	GameEntity	m_Driver;
	int			m_VehicleHealth;
	int			m_VehicleMaxHealth;
	float		m_Armor;

	VehicleInfo()
	{
		m_Type = 0;
		m_Entity = GameEntity();
		m_Weapon = GameEntity();
		m_Driver = GameEntity();
		m_VehicleHealth = 0;
		m_VehicleMaxHealth = 0;
		m_Armor = 0.f;
	}
};

struct ControllingTeam
{
	int		m_ControllingTeam;
};

struct WeaponStatus
{
	int			m_WeaponId;
	//FireMode	m_FireMode;

	WeaponStatus() : m_WeaponId(0) {}

	bool operator==(const WeaponStatus &_w2)
	{
		return m_WeaponId == _w2.m_WeaponId;
	}
	bool operator!=(const WeaponStatus &_w2)
	{
		return !(*this == _w2);
	}
};

struct WeaponLimits
{
	float		m_CenterFacing[3];
	float		m_MinHorizontalArc, m_MaxHorizontalArc;
	float		m_MinVerticalArc, m_MaxVerticalArc;
	int			m_WeaponId;
	obBool		m_Limited;
};

struct Msg_KillEntity
{
	GameEntity	m_WhoToKill;
};

struct Event_PlaySound
{
	char		m_SoundName[128];
};

struct Event_StopSound
{
	char		m_SoundName[128];
};

struct Event_ScriptEvent
{
	char		m_FunctionName[64];
	char		m_EntityName[64];
	char		m_Param1[64];
	char		m_Param2[64];
	char		m_Param3[64];
};

struct Msg_GotoWaypoint
{
	int			m_UID;
	float		m_Origin[3];
};

//////////////////////////////////////////////////////////////////////////
// Events

struct Event_SystemThreadCreated
{
	int			m_ThreadId;
};

struct Event_SystemThreadDestroyed
{
	int			m_ThreadId;
};

struct Event_SystemClientConnected
{
	int			m_GameId;
	obBool		m_IsBot;
	int			m_DesiredClass;
	int			m_DesiredTeam;

	Event_SystemClientConnected() 
		: m_GameId(-1)
		, m_IsBot(False)
		, m_DesiredClass(RANDOM_CLASS_IF_NO_CLASS)
		, m_DesiredTeam(RANDOM_TEAM_IF_NO_TEAM)
	{
	}
};

struct Event_SystemClientDisConnected
{
	int			m_GameId;
};

struct Event_SystemGravity
{
	float		m_Gravity;
};

struct Event_SystemCheats
{
	obBool		m_Enabled;
};

struct Event_EntityCreated
{
	GameEntity		m_Entity;
	BitFlag32		m_EntityCategory;
	int				m_EntityClass;	
};

struct EntityInstance
{
	GameEntity		m_Entity;
	BitFlag32		m_EntityCategory;
	int				m_EntityClass;	
};

struct Event_EntityDeleted
{
	GameEntity		m_Entity;
};

//////////////////////////////////////////////////////////////////////////

struct Event_Death
{
	GameEntity	m_WhoKilledMe;
	char		m_MeansOfDeath[32];
};

struct Event_KilledSomeone
{
	GameEntity	m_WhoIKilled;
	char		m_MeansOfDeath[32];
};

struct Event_TakeDamage
{
	GameEntity	m_Inflictor;
};

struct Event_Healed
{
	GameEntity	m_WhoHealedMe;
};

struct Event_Revived
{
	GameEntity	m_WhoRevivedMe;
};

struct Event_ChangeTeam
{
	int			m_NewTeam;
};

struct Event_WeaponChanged
{
	int			m_WeaponId;
};

struct Event_ChangeClass
{
	int			m_NewClass;
};

struct Event_Spectated
{
	int			m_WhoSpectatingMe;
};

struct Event_AddWeapon
{
	int			m_WeaponId;
};

struct Event_RemoveWeapon
{
	int			m_WeaponId;
};

struct Event_WeaponFire
{
	int			m_WeaponId;
	FireMode	m_FireMode;
	GameEntity	m_Projectile;
};

struct Event_WeaponChange
{
	int			m_WeaponId;
};

struct Event_ChatMessage
{
	GameEntity	m_WhoSaidIt;
	char		m_Message[512];
};

struct Event_VoiceMacro
{
	GameEntity	m_WhoSaidIt;
	char		m_MacroString[64];
};

struct Event_PlayerUsed
{
	GameEntity	m_WhoDidIt;
};

struct Event_Sound
{
	char		m_SoundName[64];
	float		m_Origin[3];
	GameEntity	m_Source;
	int			m_SoundType;
};

struct Event_EntitySensed
{
	int			m_EntityClass;
	GameEntity	m_Entity;
};

struct Event_ScriptMessage
{
	char		m_MessageName[64];
	char		m_MessageData1[64];
	char		m_MessageData2[64];
	char		m_MessageData3[64];
};

struct Event_ScriptSignal
{
	char		m_SignalName[64];
};

#pragma pack(pop)

#endif
