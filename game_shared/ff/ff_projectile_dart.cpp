/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_dart.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF dart projectile code.
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation logged


#include "cbase.h"
#include "ff_projectile_dart.h"
#include "effect_dispatch_data.h"
#include "IEffects.h"
#ifdef GAME_DLL
	#include "ff_player.h"
#endif

#define DART_MODEL "models/projectiles/dart/w_dart.mdl"

#ifdef CLIENT_DLL
	#include "c_te_effect_dispatch.h"
#else
	#include "te_effect_dispatch.h"

//=============================================================================
// CFFProjectileDart tables
//=============================================================================

	BEGIN_DATADESC(CFFProjectileDart) 
		DEFINE_THINKFUNC(BubbleThink), 
		DEFINE_ENTITYFUNC(DartTouch), 
	END_DATADESC() 
#endif

LINK_ENTITY_TO_CLASS(dart, CFFProjectileDart);
PRECACHE_WEAPON_REGISTER(dart);

//=============================================================================
// CFFProjectileDart implementation
//=============================================================================

#ifdef GAME_DLL

	//----------------------------------------------------------------------------
	// Purpose: Spawn a dart, set up model, size, etc
	//----------------------------------------------------------------------------
	void CFFProjectileDart::Spawn() 
	{
		// Setup
		SetModel(DART_MODEL);
		SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
		SetSize(-Vector(1, 1, 1), Vector(1, 1, 1));
		SetSolid(SOLID_BBOX);
		SetGravity(0.01f);
		
		// Set the correct think & touch for the dart
		SetTouch(&CFFProjectileDart::DartTouch);	// |-- Mirv: Account for GCC strictness
		SetThink(&CFFProjectileDart::BubbleThink);	// |-- Mirv: Account for GCC strictness

		// Next think(ie. how bubbly it'll be) 
		SetNextThink(gpGlobals->curtime + 0.1f);
		
		// Make sure we're updated if we're underwater
		UpdateWaterState();

		BaseClass::Spawn();
	}

#endif

//----------------------------------------------------------------------------
// Purpose: Precache the rocket model
//----------------------------------------------------------------------------
void CFFProjectileDart::Precache() 
{
	PrecacheModel(DART_MODEL);

	PrecacheScriptSound("Dart.HitBody");
	PrecacheScriptSound("Dart.HitWorld");

	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Touch function for a dart
//----------------------------------------------------------------------------
void CFFProjectileDart::DartTouch(CBaseEntity *pOther) 
{
	if (!pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS) || !g_pGameRules->ShouldCollide(GetCollisionGroup(), pOther->GetCollisionGroup())) 
		return;

	if (pOther->m_takedamage != DAMAGE_NO) 
	{
		trace_t	tr, tr2;
		tr = BaseClass::GetTouchTrace();
		Vector	vecNormalizedVel = GetAbsVelocity();

#ifdef GAME_DLL
		ClearMultiDamage();
		VectorNormalize(vecNormalizedVel);

		CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), m_flDamage, DMG_BULLET | DMG_NEVERGIB);
		CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 0.7f);
		dmgInfo.SetDamagePosition(tr.endpos);
		dmgInfo.SetDamageForce(Vector(0, 0, 0)); // nerf the force this applies.. TODO: FIX
		pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);
		
		
		ApplyMultiDamage();

		/* apply the tranq'ed flag here */
		if (pOther->IsPlayer()) 
		{
			CFFPlayer *pPlayer = ToFFPlayer(pOther);

			// make the player walk slow
			pPlayer->AddSpeedEffect(SE_TRANQ, 15.0, 0.4f, SEM_BOOLEAN|SEM_HEALABLE);

			// send them the status icon
			DevMsg("[Tranq Debug] Sending status icon\n");
			CSingleUserRecipientFilter user((CBasePlayer *) pPlayer);
			user.MakeReliable();
			UserMessageBegin(user, "StatusIconUpdate");
				WRITE_BYTE(FF_ICON_TRANQ);
				WRITE_FLOAT(15.0);
			MessageEnd();
		}
#endif

		//Adrian: keep going through the glass.
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS) 
			 return;

		SetAbsVelocity(Vector(0, 0, 0));

		// play body "thwack" sound
		EmitSound("Dart.HitBody");

		Vector vForward;

		AngleVectors(GetAbsAngles(), &vForward);
		VectorNormalize(vForward);

		UTIL_TraceLine(GetAbsOrigin(), 	GetAbsOrigin() + vForward * 128, MASK_OPAQUE, pOther, COLLISION_GROUP_NONE, &tr2);

		if (tr2.fraction != 1.0f) 
		{
			if (tr2.m_pEnt == NULL || (tr2.m_pEnt && tr2.m_pEnt->GetMoveType() == MOVETYPE_NONE)) 
			{
				CEffectData	data;

				data.m_vOrigin = tr2.endpos;
				data.m_vNormal = vForward;
				data.m_nEntIndex = tr2.fraction != 1.0f;
			
				DispatchEffect("DartImpact", data);
			}
		}
		
		SetTouch(NULL);
		SetThink(NULL);

		Vector vecDir = GetAbsVelocity();
		float speed = VectorNormalize(vecDir);

		// Spark if we hit an entity which wasn't human(ie. player) 
		if (!pOther->IsPlayer() && UTIL_PointContents(GetAbsOrigin()) != CONTENTS_WATER && speed > 500) 
		{
            g_pEffects->Sparks(GetAbsOrigin());
		}

		Remove();
	}
	else
	{
		trace_t	tr;
		tr = BaseClass::GetTouchTrace();

		// See if we struck the world
		if (pOther->GetMoveType() == MOVETYPE_NONE && ! (tr.surface.flags & SURF_SKY)) 
		{
			EmitSound("Dart.HitWorld");

			// if what we hit is static architecture, can stay around for a while.
			Vector vecDir = GetAbsVelocity();
			float speed = VectorNormalize(vecDir);

			SetThink(&CFFProjectileDart::SUB_Remove);
			SetNextThink(gpGlobals->curtime + 2.0f);
				
			//FIXME: We actually want to stick(with hierarchy) to what we've hit
			SetMoveType(MOVETYPE_NONE);
			
			Vector vForward;

			AngleVectors(GetAbsAngles(), &vForward);
			VectorNormalize(vForward);

			CEffectData	data;

			data.m_vOrigin = tr.endpos;
			data.m_vNormal = vForward;
			data.m_nEntIndex = 0;
			
			DispatchEffect("DartImpact", data);
				
			UTIL_ImpactTrace(&tr, DMG_BULLET);

			AddEffects(EF_NODRAW);
			SetTouch(NULL);
			SetThink(&CFFProjectileDart::SUB_Remove);		// |-- Mirv: Account for GCC strictness
			SetNextThink(gpGlobals->curtime + 2.0f);

			// Shoot some sparks
			if (UTIL_PointContents(GetAbsOrigin()) != CONTENTS_WATER && speed > 500) 
			{
				g_pEffects->Sparks(GetAbsOrigin());
			}
		}
		else
		{
			// Put a mark unless we've hit the sky
			if ((tr.surface.flags & SURF_SKY) == false) 
			{
				UTIL_ImpactTrace(&tr, DMG_BULLET);
			}

			Remove();
		}
	}
}

//----------------------------------------------------------------------------
// Purpose: Make a trail of bubbles
//----------------------------------------------------------------------------
void CFFProjectileDart::BubbleThink() 
{
	QAngle angNewAngles;

	VectorAngles(GetAbsVelocity(), angNewAngles);
	SetAbsAngles(angNewAngles);

	SetNextThink(gpGlobals->curtime + 0.1f);

	if (GetWaterLevel() == 0) 
		return;

#ifdef GAME_DLL
	UTIL_BubbleTrail(GetAbsOrigin() - GetAbsVelocity() * 0.1f, GetAbsOrigin(), 5);
#endif
}

//----------------------------------------------------------------------------
// Purpose: Create a new dart
//----------------------------------------------------------------------------
CFFProjectileDart *CFFProjectileDart::CreateDart(const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iSpeed) 
{
	CFFProjectileDart *pDart = (CFFProjectileDart *) CreateEntityByName("dart");

	UTIL_SetOrigin(pDart, vecOrigin);
	pDart->SetAbsAngles(angAngles);
	pDart->Spawn();
	pDart->SetOwnerEntity(pentOwner);

	Vector vecForward;
	AngleVectors(angAngles, &vecForward);

	// Set the speed and the initial transmitted velocity
	pDart->SetAbsVelocity(vecForward * iSpeed);

#ifdef GAME_DLL
	pDart->SetupInitialTransmittedVelocity(vecForward * iSpeed);
#endif

	pDart->m_flDamage = iDamage;

	return pDart;
}
