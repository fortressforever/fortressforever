/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file ff_grenade_emp.cpp
/// @author Shawn Smith (L0ki)
/// @date Apr. 23, 2005
/// @brief emp grenade class
/// 
/// Implementation of the CFFGrenadeEmp class. This is the secondary grenade type for engineer
/// 
/// Revisions
/// ---------
/// Apr. 23, 2005	L0ki: Initial creation

#ifndef FF_GRENADE_EMP_H
#define FF_GRENADE_EMP_H

#ifdef _WIN32
#pragma once
#endif

#include "ff_grenade_base.h"

#define EMPGRENADE_MODEL	"models/grenades/emp/emp.mdl"
#define EMP_SOUND			"EmpGrenade.Explode"
#define EMP_EFFECT			"FF_EmpZap"

#ifdef CLIENT_DLL
#define CFFGrenadeEmp C_FFGrenadeEmp
#endif

class CFFGrenadeEmp : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeEmp,CFFGrenadeBase);
	DECLARE_NETWORKCLASS();

	virtual void Precache();
	virtual Class_T Classify( void ) { return CLASS_GREN_EMP; }

	virtual float GetShakeAmplitude( void ) { return 0.0f; }	// remove the shake
	virtual float GetGrenadeRadius() { return 240.0f; }
	virtual float GetGrenadeDamage() { return 90.0f; }
	virtual const char *GetBounceSound() { return "EmpGrenade.Bounce"; }

	virtual color32 GetColour() { color32 col = { 225, 225, 0, GREN_ALPHA_DEFAULT }; return col; }

#ifdef CLIENT_DLL
	CFFGrenadeEmp() {}
	CFFGrenadeEmp( const CFFGrenadeEmp& ) {}
#else
	virtual void Spawn();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);
	void SetWarned( void ) { m_bWarned = true; }

	void GrenadeThink( void );
	bool m_bWarned;
#endif
};

#endif // FF_GRENADE_EMP_H
