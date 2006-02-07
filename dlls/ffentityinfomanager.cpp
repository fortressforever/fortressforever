#include "cbase.h"
#include "ffentityinfomanager.h"

// Mod specifics
#include "ff_utils.h"
#include "ff_player.h"
#include "ff_team.h"

EXPOSE_SINGLE_INTERFACE(FFEntityInfoManager, IEntityInfoManager, INTERFACEVERSION_ENTITYINFOMANAGER);

//////////////////////////////////////////////////////////////////////////

const QAngle& FFEntityInfoManager::GetAbsAngles(const CBaseEntity *_ent)
{
	return _ent->GetAbsAngles();
}

const QAngle& FFEntityInfoManager::GetLocalAngles(const CBaseEntity *_ent)
{
	return _ent->GetLocalAngles();
}

const Vector& FFEntityInfoManager::GetAbsOrigin(const CBaseEntity *_ent)
{
	return _ent->GetAbsOrigin();
}

const Vector& FFEntityInfoManager::GetLocalOrigin(const CBaseEntity *_ent)
{
	return _ent->GetLocalOrigin();
}

const Vector& FFEntityInfoManager::GetAbsVelocity(const CBaseEntity *_ent)
{
	return _ent->GetAbsVelocity();
}

const Vector& FFEntityInfoManager::GetLocalVelocity(const CBaseEntity *_ent)
{
	return _ent->GetLocalVelocity();
}

const QAngle& FFEntityInfoManager::GetLocalAngularVelocity(const CBaseEntity *_ent)
{
	return _ent->GetLocalAngularVelocity();
}

const Vector FFEntityInfoManager::GetEyePosition(const CBaseEntity *_ent)
{
	return _ent->EyePosition();
}

const Vector FFEntityInfoManager::GetEarPosition(const CBaseEntity *_ent)
{
	return _ent->EarPosition();
}

const int FFEntityInfoManager::GetEntityFlags(const CBaseEntity *_ent)
{
	return _ent->GetFlags();
}

const int FFEntityInfoManager::GetTeamIndex(const CBaseEntity *_ent)
{ 
	return _ent->GetTeamNumber();
}  

const char *FFEntityInfoManager::GetEntityTypeName(CBaseEntity *_ent)
{ 
	if(_ent)
	{
		CBasePlayer *pPlayer = _ent->MyCharacterPointer();
		if(pPlayer)
		{
			// Mod Specific
			CFFPlayer *pFFPlayer = static_cast<CFFPlayer*>(_ent);
			const CFFPlayerClassInfo&cdata = pFFPlayer->GetFFClassData();
			return cdata.m_szClassName;
		}		

		return _ent->GetClassName();
	}
	return 0;
} 

//////////////////////////////////////////////////////////////////////////

CBasePlayer *FFEntityInfoManager::GetBasePlayer(CBaseEntity *_ent)
{
	return _ent->MyCharacterPointer();
}

CBaseEntity *FFEntityInfoManager::GetPlayerVehicle(CBasePlayer *_ent)
{
	return _ent->GetVehicleEntity();
}

void FFEntityInfoManager::GetAmmoCount(CBasePlayer *_ent, int _ammoindex, int &_cur, int &_max)
{
	_cur = _ent->GetAmmoCount(_ammoindex);
	_max = 100; // FIXME
}

void FFEntityInfoManager::GetWeaponAmmo(CBasePlayer *_ent, int _weapon, int *_curammo, int *_maxammo, int *_curclip, int *_maxclip)
{
	*_curammo = 10;
	*_maxammo = 10;
	*_curclip = 10;
	*_maxclip = 10;
}

const char *FFEntityInfoManager::GetCurrentWeapon(CBasePlayer *_ent)
{
	CBaseCombatWeapon *pWeapon = _ent->GetActiveWeapon();
	return pWeapon ? pWeapon->GetName() : NULL;
}

const CBaseEntity *FFEntityInfoManager::HasItem(CBasePlayer *_ent, const char *_itemname)
{
	return _ent->HasNamedPlayerItem(_itemname);
}

const char *FFEntityInfoManager::GetName(CBasePlayer *_ent)
{
	return _ent->GetPlayerName(); 
}

const char *FFEntityInfoManager::GetNetworkIDString(CBasePlayer *_ent)
{
	return _ent->GetNetworkIDString(); 
}

void FFEntityInfoManager::ChangeTeam(CBasePlayer *_ent, int _newteam) 
{
	// This is probably mod specific in most cases.
	CFFPlayer *pFFPlayer = static_cast<CFFPlayer*>(_ent);
	pFFPlayer->ChangeTeam(_newteam);
}

void FFEntityInfoManager::ChangeClass(CBasePlayer *_ent, int _newclass) 
{
	// This is probably mod specific in most cases.
	CFFPlayer *pFFPlayer = static_cast<CFFPlayer*>(_ent);
	pFFPlayer->ChangeClass(Class_IntToString(_newclass));
}

int	FFEntityInfoManager::GetFragCount(const CBasePlayer *_ent) 
{
	return _ent->FragCount(); 
}

int	FFEntityInfoManager::GetDeathCount(const CBasePlayer *_ent) 
{
	return _ent->DeathCount(); 
}

bool FFEntityInfoManager::IsConnected(const CBasePlayer *_ent) 
{
	return _ent->IsConnected(); 
}

int	FFEntityInfoManager::GetArmorValue(const CBasePlayer *_ent) 
{
	return _ent->ArmorValue(); 
}

bool FFEntityInfoManager::IsHLTV(const CBasePlayer *_ent) 
{
	return _ent->IsHLTV(); 
}

bool FFEntityInfoManager::IsPlayer(const CBasePlayer *_ent) 
{
	return _ent->IsPlayer(); 
}

bool FFEntityInfoManager::IsFakeClient(CBasePlayer *_ent) 
{
	return _ent->IsFakeClient(); 
}

bool FFEntityInfoManager::IsDead(const CBasePlayer *_ent) 
{
	return _ent->IsDead(); 
}

bool FFEntityInfoManager::IsInAVehicle(const CBasePlayer *_ent) 
{
	return _ent->IsInAVehicle(); 
}

bool FFEntityInfoManager::IsObserver(const CBasePlayer *_ent) 
{
	return _ent->IsObserver(); 
}

Vector FFEntityInfoManager::GetPlayerMins(const CBasePlayer *_ent) 
{
	return _ent->GetPlayerMins(); 
}

Vector FFEntityInfoManager::GetPlayerMaxs(const CBasePlayer *_ent) 
{
	return _ent->GetPlayerMaxs(); 
}

bool FFEntityInfoManager::IsAllied(CBaseEntity *_ent, CBaseEntity *_ent2)
{
	CBasePlayer *pPlayer = _ent->MyCharacterPointer();
	if(pPlayer)
	{
		CFFTeam *pPlayerTeam = (CFFTeam *)GetGlobalTeam( pPlayer->GetTeamNumber() );
		return pPlayerTeam ? ((pPlayerTeam->GetAllies() & (1 << _ent2->GetTeamNumber())) != 0) : false;
	}
	return false;
}

