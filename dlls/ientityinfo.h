////////////////////////////////////////////////////////////////////////////
//
//#ifndef __IENTITYINFO_H__
//#define __IENTITYINFO_H__
//
//class CBasePlayer;
//class CBaseEntity;
//
//#define INTERFACEVERSION_ENTITYINFOMANAGER "EntityInfoManager001"
//class IEntityInfoManager
//{
//public:
//	virtual const QAngle& GetAbsAngles(const CBaseEntity *_ent) = 0;
//	virtual const QAngle& GetLocalAngles(const CBaseEntity *_ent) = 0;
//	virtual const Vector& GetAbsOrigin(const CBaseEntity *_ent) = 0;
//	virtual const Vector& GetLocalOrigin(const CBaseEntity *_ent) = 0;
//	virtual const Vector& GetAbsVelocity(const CBaseEntity *_ent) = 0;
//	virtual const Vector& GetLocalVelocity(const CBaseEntity *_ent) = 0;
//	virtual const QAngle& GetLocalAngularVelocity(const CBaseEntity *_ent) = 0;
//	virtual const Vector GetEyePosition(const CBaseEntity *_ent) = 0;
//	virtual const Vector GetEarPosition(const CBaseEntity *_ent) = 0;
//	virtual const int GetEntityFlags(const CBaseEntity *_ent) = 0;
//	virtual const int GetTeamIndex(const CBaseEntity *_ent) = 0;
//	virtual const char *GetEntityTypeName(CBaseEntity *_ent) = 0;
//
//	// Player info
//	virtual CBasePlayer *GetBasePlayer(CBaseEntity *_ent) = 0;
//	virtual CBaseEntity *GetPlayerVehicle(CBasePlayer *_ent) = 0;
//	virtual const char *GetCurrentWeapon(CBasePlayer *_ent) = 0;
//	virtual const CBaseEntity *HasItem(CBasePlayer *_ent, const char *_itemname) = 0;
//	virtual void GetAmmoCount(CBasePlayer *_ent, int _ammoindex, int &_cur, int &_max) = 0;
//	virtual void GetWeaponAmmo(CBasePlayer *_ent, int _weapon, int *_curammo, int *_maxammo, int *_curclip, int *_maxclip) = 0;	
//
//	virtual const char *GetName(CBasePlayer *_ent) = 0;
//	virtual const char *GetNetworkIDString(CBasePlayer *_ent) = 0;
//	virtual void ChangeTeam(CBasePlayer *_ent, int _newteam) = 0;
//	virtual void ChangeClass(CBasePlayer *_ent, int _newclass) = 0;
//	virtual int	GetFragCount(const CBasePlayer *_ent) = 0;
//	virtual int	GetDeathCount(const CBasePlayer *_ent) = 0;
//	virtual bool IsConnected(const CBasePlayer *_ent) = 0;
//	virtual int	GetArmorValue(const CBasePlayer *_ent) = 0;
//	virtual bool IsHLTV(const CBasePlayer *_ent) = 0;
//	virtual bool IsPlayer(const CBasePlayer *_ent) = 0;
//	virtual bool IsFakeClient(CBasePlayer *_ent) = 0;
//	virtual bool IsDead(const CBasePlayer *_ent) = 0;
//	virtual bool IsInAVehicle(const CBasePlayer *_ent) = 0;
//	virtual bool IsObserver(const CBasePlayer *_ent) = 0;
//	virtual Vector GetPlayerMins(const CBasePlayer *_ent) = 0;
//	virtual Vector GetPlayerMaxs(const CBasePlayer *_ent) = 0;
//	virtual bool IsAllied(CBaseEntity *_ent, CBaseEntity *_ent2) = 0;
//};
//
//#endif
