//#ifndef __FFENTITYINFOMANAGER_H__
//#define __FFENTITYINFOMANAGER_H__
//
//#include "ientityinfo.h"
//
//class FFEntityInfoManager : public IEntityInfoManager
//{
//public:
//	// General Entity Information
//	const QAngle& GetAbsAngles(const CBaseEntity *_ent);
//	const QAngle& GetLocalAngles(const CBaseEntity *_ent);
//	const Vector& GetAbsOrigin(const CBaseEntity *_ent);
//	const Vector& GetLocalOrigin(const CBaseEntity *_ent);
//	const Vector& GetAbsVelocity(const CBaseEntity *_ent);
//	const Vector& GetLocalVelocity(const CBaseEntity *_ent);
//	const QAngle& GetLocalAngularVelocity(const CBaseEntity *_ent);
//	const Vector GetEyePosition(const CBaseEntity *_ent);
//	const Vector GetEarPosition(const CBaseEntity *_ent);
//	const int GetEntityFlags(const CBaseEntity *_ent);
//	const int GetTeamIndex(const CBaseEntity *_ent);
//	const char *GetEntityTypeName(CBaseEntity *_ent);
//
//	// Player stuff.
//	CBasePlayer *GetBasePlayer(CBaseEntity *_ent);
//	CBaseEntity *GetPlayerVehicle(CBasePlayer *_ent);	
//	const char *GetCurrentWeapon(CBasePlayer *_ent);
//	const CBaseEntity *HasItem(CBasePlayer *_ent, const char *_itemname);
//	void GetAmmoCount(CBasePlayer *_ent, int _ammoindex, int &_cur, int &_max);
//	virtual void GetWeaponAmmo(CBasePlayer *_ent, int _weapon, int *_curammo, int *_maxammo, int *_curclip, int *_maxclip);
//
//	const char *GetName(CBasePlayer *_ent);
//	const char *GetNetworkIDString(CBasePlayer *_ent);
//	void ChangeTeam(CBasePlayer *_ent, int _newteam);
//	void ChangeClass(CBasePlayer *_ent, int _newclass);
//	int	GetFragCount(const CBasePlayer *_ent);
//	int	GetDeathCount(const CBasePlayer *_ent);
//	bool IsConnected(const CBasePlayer *_ent);
//	int	GetArmorValue(const CBasePlayer *_ent);
//	bool IsHLTV(const CBasePlayer *_ent);
//	bool IsPlayer(const CBasePlayer *_ent);
//	bool IsFakeClient(CBasePlayer *_ent);
//	bool IsDead(const CBasePlayer *_ent);
//	bool IsInAVehicle(const CBasePlayer *_ent);
//	bool IsObserver(const CBasePlayer *_ent);
//	Vector GetPlayerMins(const CBasePlayer *_ent);
//	Vector GetPlayerMaxs(const CBasePlayer *_ent);
//	bool IsAllied(CBaseEntity *_ent, CBaseEntity *_ent2);
//};
//
//#endif
