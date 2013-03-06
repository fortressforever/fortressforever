/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file ff_grenade_emp.cpp
/// @author Shawn Smith (L0ki)
/// @date Apr. 23, 2005
/// @brief emp grenade class
/// 
/// Implementation of the CFFGrenadeShockEmp class. This is the secondary grenade type for engineer
/// 
/// Revisions
/// ---------
/// Apr. 23, 2005	L0ki: Initial creation

#ifndef FF_GRENADE_SHOCKEMP_H
#define FF_GRENADE_SHOCKEMP_H

#ifdef _WIN32
#pragma once
#endif

#include "ff_grenade_base.h"

#define SHOCKEMPGRENADE_MODEL	"models/grenades/emp/emp.mdl"
#define SHOCKEMP_SOUND			"EmpGrenade.Explode"
#define SHOCKEMP_EFFECT			"FF_EmpZap"

#ifdef CLIENT_DLL
#define CFFGrenadeShockEmp C_FFGrenadeShockEmp
#endif

class CFFGrenadeShockEmp : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeShockEmp,CFFGrenadeBase);
	DECLARE_NETWORKCLASS();

	virtual void Precache();
	virtual Class_T Classify( void ) { return CLASS_GREN_SHOCKEMP; }

	virtual float GetShakeAmplitude( void ) { return 0.0f; }	// remove the shake
	virtual float GetGrenadeRadius() { return 240.0f; }
	virtual float GetGrenadeDamage() { return 90.0f; }
	virtual const char *GetBounceSound() { return "EmpGrenade.Bounce"; }

	virtual color32 GetColour() { color32 col = { 225, 225, 0, GREN_ALPHA_DEFAULT }; return col; }

#ifdef CLIENT_DLL
	CFFGrenadeShockEmp() {}
	CFFGrenadeShockEmp( const CFFGrenadeShockEmp& ) {}
#else
	virtual void Spawn();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);
	void SetWarned( void ) { m_bWarned = true; }

	void GrenadeThink( void );
	bool m_bWarned;
#endif
};

#endif // FF_GRENADE_EMP_H
