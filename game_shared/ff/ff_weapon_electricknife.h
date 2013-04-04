#include "cbase.h"
#include "ff_weapon_basemelee.h"
#include "ff_utils.h"
#include "ff_gamerules.h"

#ifdef CLIENT_DLL
	#define CFFWeaponElectricKnife C_FFWeaponElectricKnife
#endif

//=============================================================================
// CFFWeaponElectricKnife
//=============================================================================

class CFFWeaponElectricKnife : public CFFWeaponMeleeBase
{
public:
	DECLARE_CLASS(CFFWeaponElectricKnife, CFFWeaponMeleeBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CFFWeaponElectricKnife();

	virtual bool Deploy();
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void ItemPostFrame( void );
	virtual	void PrimaryAttack();
	virtual void Precache();

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_ELECTRICKNIFE; }
	
	void TryStartCharging();
	bool IsRecharged();
	float GetElectrifyPercent();
	float GetCooldownPercent();
	bool IsElectrified( void );

private:

	CFFPlayer *pOwner;
	float m_flStartElectrifyTime;
	float m_flStartRechargeTime;
	float m_flElectrifyPercent;
	float CalculateElectrifyPercent();

	void Update();
	void CancelElectrify();

	void Hit(trace_t &traceHit, Activity nHitActivity);
	void ChargedStabEntity(trace_t &traceHit, CBaseEntity *pHitEntity);
	void UnchargedStabEntity(trace_t &traceHit, CBaseEntity *pHitEntity);
	bool IsHitEntitySpecialCase(CBaseEntity *pHitEntity);
	void ChargedStabPlayer(trace_t &traceHit, CFFPlayer *pTarget );
	float GetChargedPlayerDamage( void );
	Vector GetChargedPlayerForce( Vector hitDirection );
	
	void ChargedStabSentryGun(trace_t &traceHit, CFFSentryGun *pTarget );
	float GetChargedSentryGunDamage( void );
	float GetChargedSentryGunDisableTime( void );

	CFFWeaponElectricKnife(const CFFWeaponElectricKnife &);
};