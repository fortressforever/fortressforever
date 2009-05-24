#pragma once

#include "ientityinfo.h"
//#include "iplayerinfo.h"

class CEntityInfoManager : public IEntityInfoManager
{
public:
	// General Entity Information
	const QAngle& GetAbsAngles(const CBaseEntity *_ent) const;
	const QAngle& GetLocalAngles(const CBaseEntity *_ent) const;
	const Vector& GetAbsOrigin(const CBaseEntity *_ent) const;
	const Vector& GetLocalOrigin(const CBaseEntity *_ent) const;
	const Vector& GetAbsVelocity(const CBaseEntity *_ent) const;
	const Vector& GetLocalVelocity(const CBaseEntity *_ent) const;
	const QAngle& GetLocalAngularVelocity(const CBaseEntity *_ent) const;
	const Vector GetEyePosition(const CBaseEntity *_ent) const;
	const Vector GetEarPosition(const CBaseEntity *_ent) const;
	const int GetEntityFlags(const CBaseEntity *_ent) const;
	const int GetTeamIndex(const CBaseEntity *_ent) const;

	// Player stuff.
	virtual CBasePlayer *GetBasePlayer(CBaseEntity *_ent) const;
	virtual CBaseEntity *GetPlayerVehicle(CBasePlayer *_ent) const;
	virtual int GetAmmoCount(CBasePlayer *_ent, int _ammoindex) const;
	virtual const char *GetCurrentWeapon(CBasePlayer *_ent) const;
	const CBaseEntity *HasItem(CBasePlayer *_ent, const char *_itemname);

	const char *GetName(CBasePlayer *_ent);
	const char *GetNetworkIDString(CBasePlayer *_ent);
	void ChangeTeam(CBasePlayer *_ent, int _newteam);
	void ChangeClass(CBasePlayer *_ent, int _newclass);
	const char *GetClassName(CBasePlayer *_ent);
	int	GetFragCount(const CBasePlayer *_ent);
	int	GetDeathCount(const CBasePlayer *_ent);
	bool IsConnected(const CBasePlayer *_ent);
	int	GetArmorValue(const CBasePlayer *_ent);
	bool IsHLTV(const CBasePlayer *_ent);
	bool IsPlayer(const CBasePlayer *_ent);
	bool IsFakeClient(CBasePlayer *_ent);
	bool IsDead(const CBasePlayer *_ent);
	bool IsInAVehicle(const CBasePlayer *_ent);
	bool IsObserver(const CBasePlayer *_ent);
	Vector GetPlayerMins(const CBasePlayer *_ent);
	Vector GetPlayerMaxs(const CBasePlayer *_ent);
	bool IsAllied(CBaseEntity *_ent, CBaseEntity *_ent2);
};

//////////////////////////////////////////////////////////////////////////

//class EntityInfo : public IEntityInfo
//{
//public:
//	inline void SetParent( CBaseEntity *_parent ) { m_pParent = _parent; } 
//
//	const char *GetClassName();
//	const QAngle& GetAbsAngles() const;
//	const QAngle& GetLocalAngles() const;
//	const Vector& GetAbsOrigin() const;
//	const Vector& GetLocalOrigin() const;
//	const Vector& GetAbsVelocity() const;
//	const Vector& GetLocalVelocity() const;
//	const QAngle& GetLocalAngularVelocity() const;
//	IPlayerInfo* GetPlayerInfo() const;
//
//	EntityInfo();
//protected:
//	CBaseEntity *m_pParent;
//};

