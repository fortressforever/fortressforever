/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life2 ========
///
/// @file ff_weapon_knife.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF Knife code & class declaration.
///
/// REVISIONS
/// ---------
/// Jan 19, 2005 Mirv: Initial implementation

#include "cbase.h"
#include "ff_weapon_basemelee.h"

#ifdef CLIENT_DLL
	#define CFFWeaponKnife C_FFWeaponKnife
	#include "ff_utils.h"
#endif

//=============================================================================
// CFFWeaponKnife
//=============================================================================

class CFFWeaponKnife : public CFFWeaponMeleeBase
{
public:
	DECLARE_CLASS(CFFWeaponKnife, CFFWeaponMeleeBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CFFWeaponKnife();

	virtual bool Deploy();

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_KNIFE; }

private:
	void Hit(trace_t &traceHit, Activity nHitActivity);

	CFFWeaponKnife(const CFFWeaponKnife &);
};

//=============================================================================
// CFFWeaponKnife tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponKnife, DT_FFWeaponKnife) 

BEGIN_NETWORK_TABLE(CFFWeaponKnife, DT_FFWeaponKnife) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponKnife) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_knife, CFFWeaponKnife);
PRECACHE_WEAPON_REGISTER(ff_weapon_knife);

//=============================================================================
// CFFWeaponKnife implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponKnife::CFFWeaponKnife() 
{
}

//----------------------------------------------------------------------------
// Purpose: Play the unholster sound for a knife
//----------------------------------------------------------------------------
bool CFFWeaponKnife::Deploy() 
{
	WeaponSound(SPECIAL1);

	// A quick hint test
#ifdef CLIENT_DLL
	FF_HudHint(HINT_GENERAL, 54, "Hi there. You seem to have drawn a knife. The knife can backstab and stuff like that - it's awesome!");
#endif
	
	return BaseClass::Deploy();
}

//----------------------------------------------------------------------------
// Purpose: Implement impact function
//----------------------------------------------------------------------------
void CFFWeaponKnife::Hit(trace_t &traceHit, Activity nHitActivity) 
{
	DevMsg("[CFFWeaponKnife] Hit\n");
#ifdef GAME_DLL
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	if (pHitEntity != NULL && pHitEntity->IsPlayer()) 
	{
		CFFPlayer *pTarget = ToFFPlayer(pHitEntity);

		if (g_pGameRules->FPlayerCanTakeDamage(pPlayer, pTarget)) 
		{
			// check to see if we got a backstab

			// get the displacement between the players
			Vector vDisplacement = pTarget->GetAbsOrigin() - pPlayer->GetAbsOrigin();
			vDisplacement.z = 0;
			vDisplacement.NormalizeInPlace();
	
			// get the direction the target is facing
			Vector vFacing;
			AngleVectors(pTarget->GetLocalAngles(), &vFacing);
			vFacing.z = 0;
			vFacing.NormalizeInPlace();

			// see if they are facing the same direction
			float angle = vFacing.Dot(vDisplacement);
			if (angle > .707) // cos(45deg) 
			{
				DevMsg("BACKSTAB!!!!!\n");
				// we get to totally kerplown this guy

				// Mulch: armor doesn't protect against DMG_DIRECT
				pTarget->TakeDamage(CTakeDamageInfo(this, pPlayer, 108, DMG_DIRECT));				

				// we don't need to call BaseClass since we already did damage.
				return;
			}

			//DevMsg("Test Backstab: (%.2f, %.2f) dot(%.2f, %.2f) = %.2f\n", vDisplacement.x, vDisplacement.y, vFacing.x, vFacing.y, angle);

		}
	}
#endif

	BaseClass::Hit(traceHit, nHitActivity);
}
