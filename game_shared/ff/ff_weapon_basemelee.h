/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life2 ========
///
/// @file ff_weapon_basemelee.h
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF Melee weapon code, all melee weapons derived from here
///
/// REVISIONS
/// ---------
/// Jan 18, 2005 Mirv: Added to project


#ifndef FF_WEAPON_BASEMELEE_H
#define FF_WEAPON_BASEMELEE_H
#ifdef _WIN32
#pragma once
#endif

#include "ff_weapon_base.h"
#include "effect_dispatch_data.h"
#include "takedamageinfo.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
	#define CFFWeaponMeleeBase C_FFWeaponMeleeBase
	#include "c_ff_player.h"
	#include "c_te_effect_dispatch.h"
#else
	#include "ff_player.h"
	#include "soundent.h"
	#include "te_effect_dispatch.h"
#endif

#define MELEE_IMPACT_FORCE		50.0f

//=========================================================
// CFFWeaponMeleeBase 
//=========================================================

class CFFWeaponMeleeBase : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponMeleeBase, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CFFWeaponMeleeBase();

	virtual	void	Spawn();
	virtual	void	Precache();
	
	//Attack functions
	virtual	void	PrimaryAttack();

	virtual	float	GetFireRate() 								{	return	0.2f;	}
	virtual float	GetRange() 								{	return	32.0f;	}
	virtual	float	GetDamageForActivity(Activity hitActivity) 	{	return	1.0f;	}

protected:
	virtual	void	ImpactEffect(trace_t &trace);
	virtual void	Hit(trace_t &traceHit, Activity nHitActivity);

private:
	bool			ImpactWater(const Vector &start, const Vector &end);
	void			Swing();
	Activity		ChooseIntersectionPointAndActivity(trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner);
};

#endif // FF_WEAPON_BASEMELEE_H
