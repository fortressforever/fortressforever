#include "cbase.h"
#include "EntityInfoManager.h"

// Mod specifics
#include "ff_utils.h"
#include "ff_player.h"

EXPOSE_SINGLE_INTERFACE(CEntityInfoManager, IEntityInfoManager, INTERFACEVERSION_ENTITYINFOMANAGER);

//////////////////////////////////////////////////////////////////////////

const QAngle& CEntityInfoManager::GetAbsAngles(const CBaseEntity *_ent) const
{
	return _ent->GetAbsAngles();
}

const QAngle& CEntityInfoManager::GetLocalAngles(const CBaseEntity *_ent) const
{
	return _ent->GetLocalAngles();
}

const Vector& CEntityInfoManager::GetAbsOrigin(const CBaseEntity *_ent) const
{
	return _ent->GetAbsOrigin();
}

const Vector& CEntityInfoManager::GetLocalOrigin(const CBaseEntity *_ent) const
{
	return _ent->GetLocalOrigin();
}

const Vector& CEntityInfoManager::GetAbsVelocity(const CBaseEntity *_ent) const
{
	return _ent->GetAbsVelocity();
}

const Vector& CEntityInfoManager::GetLocalVelocity(const CBaseEntity *_ent) const
{
	return _ent->GetLocalVelocity();
}

const QAngle& CEntityInfoManager::GetLocalAngularVelocity(const CBaseEntity *_ent) const
{
	return _ent->GetLocalAngularVelocity();
}

const Vector CEntityInfoManager::GetEyePosition(const CBaseEntity *_ent) const
{
	return _ent->EyePosition();
}

const Vector CEntityInfoManager::GetEarPosition(const CBaseEntity *_ent) const
{
	return _ent->EarPosition();
}

const int CEntityInfoManager::GetEntityFlags(const CBaseEntity *_ent) const
{
	return _ent->GetFlags();
}

const int CEntityInfoManager::GetTeamIndex(const CBaseEntity *_ent) const
{ 
	return _ent->GetTeamNumber();
}  

//////////////////////////////////////////////////////////////////////////

CBasePlayer *CEntityInfoManager::GetBasePlayer(CBaseEntity *_ent) const
{
	return _ent->MyCharacterPointer();
}

CBaseEntity *CEntityInfoManager::GetPlayerVehicle(CBasePlayer *_ent) const
{
	return _ent->GetVehicleEntity();
}

int CEntityInfoManager::GetAmmoCount(CBasePlayer *_ent, int _ammoindex) const
{
	return _ent->GetAmmoCount(_ammoindex);
}

const char *CEntityInfoManager::GetCurrentWeapon(CBasePlayer *_ent) const
{
	CBaseCombatWeapon *pWeapon = _ent->GetActiveWeapon();
	return pWeapon ? pWeapon->GetName() : NULL;
}

//int GetWeaponCount(CBasePlayer *_ent)
//{
//	return _ent->WeaponCount();
//}
//
//const void GetWeaponInfo(CBasePlayer *_ent, int _weaponIndex, WeaponInfo &_wi)
//{
//	CBaseCombatWeapon *pWeapon = _ent->GetWeapon(_weaponIndex);
//	_wi.m_WeaponName = pWeapon->GetName();
//	_wi.m_Clip1 = pWeapon->Clip1();
//	_wi.m_Clip2 = pWeapon->Clip2();
//}
//
//CBaseCombatWeapon *GetWeapon(CBasePlayer *_ent, int _index)
//{
//	return _ent->GetWeapon(_index);
//}
//
//int GetWeaponClip1(CBaseCombatWeapon *_weapon)
//{
//	return _weapon->Clip1();
//}
//
//int GetWeaponClip2(CBaseCombatWeapon *_weapon)
//{
//	return _weapon->Clip2();
//}

const CBaseEntity *CEntityInfoManager::HasItem(CBasePlayer *_ent, const char *_itemname)
{
	return _ent->HasNamedPlayerItem(_itemname);
}

const char *CEntityInfoManager::GetName(CBasePlayer *_ent)
{
	return _ent->GetPlayerName(); 
}

const char *CEntityInfoManager::GetNetworkIDString(CBasePlayer *_ent)
{
	return _ent->GetNetworkIDString(); 
}

void CEntityInfoManager::ChangeTeam(CBasePlayer *_ent, int _newteam) 
{
	CFFPlayer *pFFPlayer = static_cast<CFFPlayer*>(_ent);
	pFFPlayer->ChangeTeam(_newteam);
}

const char *CEntityInfoManager::GetClassName(CBasePlayer *_ent) 
{

	return NULL;
}

void CEntityInfoManager::ChangeClass(CBasePlayer *_ent, int _newclass) 
{
	CFFPlayer *pFFPlayer = static_cast<CFFPlayer*>(_ent);
	pFFPlayer->ChangeClass(Class_IntToString(_newclass));
}

int	CEntityInfoManager::GetFragCount(const CBasePlayer *_ent) 
{
	return _ent->FragCount(); 
}

int	CEntityInfoManager::GetDeathCount(const CBasePlayer *_ent) 
{
	return _ent->DeathCount(); 
}

bool CEntityInfoManager::IsConnected(const CBasePlayer *_ent) 
{
	return _ent->IsConnected(); 
}

int	CEntityInfoManager::GetArmorValue(const CBasePlayer *_ent) 
{
	return _ent->ArmorValue(); 
}

bool CEntityInfoManager::IsHLTV(const CBasePlayer *_ent) 
{
	return _ent->IsHLTV(); 
}

bool CEntityInfoManager::IsPlayer(const CBasePlayer *_ent) 
{
	return _ent->IsPlayer(); 
}

bool CEntityInfoManager::IsFakeClient(CBasePlayer *_ent) 
{
	return _ent->IsFakeClient(); 
}

bool CEntityInfoManager::IsDead(const CBasePlayer *_ent) 
{
	return _ent->IsDead(); 
}

bool CEntityInfoManager::IsInAVehicle(const CBasePlayer *_ent) 
{
	return _ent->IsInAVehicle(); 
}

bool CEntityInfoManager::IsObserver(const CBasePlayer *_ent) 
{
	return _ent->IsObserver(); 
}

Vector CEntityInfoManager::GetPlayerMins(const CBasePlayer *_ent) 
{
	return _ent->GetPlayerMins(); 
}

Vector CEntityInfoManager::GetPlayerMaxs(const CBasePlayer *_ent) 
{
	return _ent->GetPlayerMaxs(); 
}

bool CEntityInfoManager::IsAllied(CBaseEntity *_ent, CBaseEntity *_ent2)
{
	return _ent->GetTeamNumber() == _ent2->GetTeamNumber();
}

