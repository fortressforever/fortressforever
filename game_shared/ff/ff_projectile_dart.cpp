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

//extern ConVar ffdev_nail_speed;
//extern ConVar ffdev_nail_bbox;
#define NAIL_SPEED 2000.0f
#define NAIL_BBOX 2.0f

#define DART_MODEL "models/projectiles/dart/w_dart.mdl"

//Time the tranq effect lasts for on enemy targets
ConVar ffdev_tranq_duration_enemy("ffdev_tranq_duration_enemy", "4.0", FCVAR_REPLICATED | FCVAR_NOTIFY );
#define TRANQ_DURATION_ENEMY	ffdev_tranq_duration_enemy.GetFloat()

//Time the tranq effect last on friendly targets
ConVar ffdev_tranq_duration_friendly("ffdev_tranq_duration_friendly", "2.0", FCVAR_REPLICATED | FCVAR_NOTIFY );
#define TRANQ_DURATION_FRIENDLY	ffdev_tranq_duration_friendly.GetFloat()

class CRecvProxyData;
extern void RecvProxy_LocalVelocityX(const CRecvProxyData *pData, void *pStruct, void *pOut);
extern void RecvProxy_LocalVelocityY(const CRecvProxyData *pData, void *pStruct, void *pOut);
extern void RecvProxy_LocalVelocityZ(const CRecvProxyData *pData, void *pStruct, void *pOut);

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

IMPLEMENT_NETWORKCLASS_ALIASED(FFProjectileDart, DT_FFProjectileDart)

BEGIN_NETWORK_TABLE(CFFProjectileDart, DT_FFProjectileDart)
#ifdef CLIENT_DLL
	RecvPropFloat		( RECVINFO(m_vecVelocity[0]), 0, RecvProxy_LocalVelocityX ),
	RecvPropFloat		( RECVINFO(m_vecVelocity[1]), 0, RecvProxy_LocalVelocityY ),
	RecvPropFloat		( RECVINFO(m_vecVelocity[2]), 0, RecvProxy_LocalVelocityZ ),
#else
	// Increased range for this because it goes at a mad speed
	SendPropExclude("DT_BaseGrenade", "m_vecVelocity[0]"),
	SendPropExclude("DT_BaseGrenade", "m_vecVelocity[1]"),
	SendPropExclude("DT_BaseGrenade", "m_vecVelocity[2]"),
	SendPropFloat( SENDINFO_VECTORELEM(m_vecVelocity, 0), 16, SPROP_ROUNDDOWN, -4096.0f, 4096.0f ),
	SendPropFloat( SENDINFO_VECTORELEM(m_vecVelocity, 1), 16, SPROP_ROUNDDOWN, -4096.0f, 4096.0f ),
	SendPropFloat( SENDINFO_VECTORELEM(m_vecVelocity, 2), 16, SPROP_ROUNDDOWN, -4096.0f, 4096.0f ),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(ff_projectile_dart, CFFProjectileDart);
PRECACHE_WEAPON_REGISTER(ff_projectile_dart);

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
		SetSize(-Vector(1.0f, 1.0f, 1.0f) * NAIL_BBOX, Vector(1.0f, 1.0f, 1.0f) * NAIL_BBOX);
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
		dmgInfo.SetDamagePosition(tr.startpos);
		dmgInfo.SetImpactPosition(tr.endpos);

		// Damage force is nerf'd against player
		if (pOther->IsPlayer())
		{
			dmgInfo.ScaleDamageForce(0.0f);
		}

		pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);
		
		
		ApplyMultiDamage();

		// Apply the tranq'ed flag here
		if (pOther->IsPlayer()) 
		{
			CFFPlayer *pPlayer = ToFFPlayer(pOther);

			// #0000695: you can tranq your teammates w/ mp_friendlyfire 0
			if (g_pGameRules->FCanTakeDamage(pPlayer, GetOwnerEntity()))
			{
				//Check the team relationship to allot the correct tranq durations
				//ENEMY is hit->
				if( ToFFPlayer(GetOwnerEntity())->GetTeamNumber() != pPlayer->GetTeamNumber() )
				{
					// make the player walk slow, but run it through lau first!
					float flDuration = TRANQ_DURATION_ENEMY;
					float flIconDuration = flDuration;
					float flSpeed = 0.3f;
					if( pPlayer->LuaRunEffect( LUA_EF_TRANQ, GetOwnerEntity(), &flDuration, &flIconDuration, &flSpeed ) )
					{
						pPlayer->AddSpeedEffect( SE_TRANQ, flDuration, flSpeed, SEM_BOOLEAN | SEM_HEALABLE, FF_STATUSICON_TRANQUILIZED, flIconDuration );
					}

					// And now.. an effect
					CSingleUserRecipientFilter user(pPlayer);
					user.MakeReliable();

					UserMessageBegin(user, "FFViewEffect");
					WRITE_BYTE(FF_VIEWEFFECT_TRANQUILIZED);
					WRITE_FLOAT(TRANQ_DURATION_ENEMY);
					MessageEnd();
				}
				//FRIENDLY is hit ->
				else
				{
					// make the player walk slow, but run it through lau first!
					float flDuration = TRANQ_DURATION_FRIENDLY;
					float flIconDuration = flDuration;
					float flSpeed = 0.3f;
					if( pPlayer->LuaRunEffect( LUA_EF_TRANQ, GetOwnerEntity(), &flDuration, &flIconDuration, &flSpeed ) )
					{
						pPlayer->AddSpeedEffect( SE_TRANQ, flDuration, flSpeed, SEM_BOOLEAN | SEM_HEALABLE, FF_STATUSICON_TRANQUILIZED, flIconDuration );
					}

					// And now.. an effect
					CSingleUserRecipientFilter user(pPlayer);
					user.MakeReliable();

					UserMessageBegin(user, "FFViewEffect");
					WRITE_BYTE(FF_VIEWEFFECT_TRANQUILIZED);
					WRITE_FLOAT(TRANQ_DURATION_FRIENDLY);
					MessageEnd();
				}
			}
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

#ifdef GAME_DLL
				data.m_nEntIndex = tr2.fraction != 1.0f;
#else
				data.m_hEntity = NULL; // Mirv: FIXME
#endif
			
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

#ifdef GAME_DLL
			data.m_nEntIndex = 0;
#else
			data.m_hEntity = NULL;	// Mirv: FIXME
#endif
			
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
CFFProjectileDart *CFFProjectileDart::CreateDart(const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iSpeed) 
{
	CFFProjectileDart *pDart = (CFFProjectileDart *) CreateEntityByName("ff_projectile_dart");

	UTIL_SetOrigin(pDart, vecOrigin);
	pDart->SetAbsAngles(angAngles);
	pDart->Spawn();
	pDart->SetOwnerEntity(pentOwner);
	pDart->m_iSourceClassname = (pSource ? pSource->m_iClassname : NULL_STRING);

	Vector vecForward;
	AngleVectors(angAngles, &vecForward);

	vecForward *= NAIL_SPEED;//ffdev_nail_speed.GetFloat();		// iSpeed;

	// Set the speed and the initial transmitted velocity
	pDart->SetAbsVelocity(vecForward);

#ifdef GAME_DLL
	pDart->SetupInitialTransmittedVelocity(vecForward);
#endif

	pDart->m_flDamage = iDamage;

	return pDart;
}
