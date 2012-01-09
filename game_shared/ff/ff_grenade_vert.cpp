/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_grenade_vert.cpp
/// @author Ryan "squeek" Liptak
/// @date September 2009
/// @brief The vert grenade
///
/// REVISIONS
/// ---------
/// August 25, 2009		squeek: File first created

//========================================================================
// Required includes
//========================================================================
#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"
#include "beam_flags.h"
#include "Sprite.h"
#include "model_types.h"
#include "effect_dispatch_data.h"
#include "IEffects.h"

#ifdef GAME_DLL
	#include "ff_entity_system.h"
	#include "te_effect_dispatch.h"
#else
	#include "c_te_effect_dispatch.h"
#endif

#define VERT_MODEL "models/grenades/nailgren/nailgren.mdl"
#define VERT_SOUND "Vert.Explode"
#define VERT_EFFECT "FF_Vert"

ConVar ffdev_vert_radius("ffdev_vert_radius", "270", FCVAR_REPLICATED /* | FCVAR_CHEAT */, "Radius of vert grenade");
#define VERT_RADIUS ffdev_vert_radius.GetFloat()

ConVar ffdev_vert_vertical_velocity_min("ffdev_vert_vertical_velocity_min", "400", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
ConVar ffdev_vert_vertical_velocity_max("ffdev_vert_vertical_velocity_max", "800", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
#define VERT_MIN_VERTICAL_VELOCITY ffdev_vert_vertical_velocity_min.GetFloat()
#define VERT_MAX_VERTICAL_VELOCITY ffdev_vert_vertical_velocity_max.GetFloat()

ConVar ffdev_vert_lateral_velocity_scale_min("ffdev_vert_lateral_velocity_scale_min", "0", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
ConVar ffdev_vert_lateral_velocity_scale_max("ffdev_vert_lateral_velocity_scale_max", ".20", FCVAR_REPLICATED /* | FCVAR_CHEAT */);
#define VERT_MIN_LATERAL_VELOCITY_SCALE ffdev_vert_lateral_velocity_scale_min.GetFloat()
#define VERT_MAX_LATERAL_VELOCITY_SCALE ffdev_vert_lateral_velocity_scale_max.GetFloat()

#ifdef CLIENT_DLL
	int g_iVertRingTexture = -1;
#endif

#ifdef CLIENT_DLL
	#define CFFGrenadeVert C_FFGrenadeVert
#endif

//=============================================================================
// CFFGrenadeVert
//=============================================================================

class CFFGrenadeVert : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeVert, CFFGrenadeBase) 
	DECLARE_NETWORKCLASS(); 

	CNetworkVector(m_vInitialVelocity);

	virtual void Precache();
	virtual const char *GetBounceSound() { return "NailGrenade.Bounce"; }
	
	virtual float GetGrenadeDamage()		{ return 0.0f; }
	virtual float GetGrenadeRadius()		{ return VERT_RADIUS; }

	virtual Class_T Classify( void ) { return CLASS_GREN_VERT; } 

	virtual color32 GetColour() { color32 col = { 128, 225, 255, GREN_ALPHA_DEFAULT }; return col; }

#ifdef CLIENT_DLL
	CFFGrenadeVert() {}
	CFFGrenadeVert(const CFFGrenadeVert&) {}
#else
	DECLARE_DATADESC(); // Since we're adding new thinks etc
	virtual void Spawn();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);

#endif
};

#ifdef GAME_DLL
	BEGIN_DATADESC(CFFGrenadeVert) 
	END_DATADESC() 
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeVert, DT_FFGrenadeVert)

BEGIN_NETWORK_TABLE(CFFGrenadeVert, DT_FFGrenadeVert)
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(ff_grenade_vert, CFFGrenadeVert);
PRECACHE_WEAPON_REGISTER(ff_grenade_vert);

//-----------------------------------------------------------------------------
// Purpose: Various precache things
//-----------------------------------------------------------------------------
void CFFGrenadeVert::Precache() 
{
	PrecacheModel(VERT_MODEL);
	PrecacheScriptSound(VERT_SOUND);

#ifdef CLIENT_DLL
	g_iVertRingTexture = PrecacheModel("sprites/lgtning.vmt");
#endif

	BaseClass::Precache();
}

#ifdef GAME_DLL

	//-----------------------------------------------------------------------------
	// Purpose: Various spawny flag things
	//-----------------------------------------------------------------------------
	void CFFGrenadeVert::Spawn() 
	{
		SetModel(VERT_MODEL);
		BaseClass::Spawn();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Instead of exploding, change to 
	//-----------------------------------------------------------------------------
	void CFFGrenadeVert::Explode(trace_t *pTrace, int bitsDamageType)
	{
		// Don't explode if in no gren area
		if( !FFScriptRunPredicates( this, "onexplode", true ) )
		{
			UTIL_Remove( this );
			return;
		}
		
		CEffectData data;
		data.m_vOrigin = GetAbsOrigin();
		data.m_flRadius = GetGrenadeRadius();

		DispatchEffect(VERT_EFFECT, data);

		EmitSound(VERT_SOUND);

		CBaseEntity *pEntity = NULL;

		for( CEntitySphereQuery sphere( GetAbsOrigin(), GetGrenadeRadius() ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			if (!pEntity || !pEntity->IsPlayer())
				continue;

			CFFPlayer *pPlayer = ToFFPlayer(pEntity);

			if( !pPlayer->IsAlive() || pPlayer->IsObserver() )
				continue;

			// Some useful things to know
			Vector vecDisplacement = pPlayer->GetLegacyAbsOrigin() - GetAbsOrigin();
			float flDistance = vecDisplacement.Length();
			Vector vecDir = vecDisplacement / flDistance;
			
			// caes: make hh concs not push other players
			if ( pEntity == GetThrower() && m_fIsHandheld )
				continue;

			// People who are building shouldn't be pushed around by anything
			if (pPlayer->IsStaticBuilding())
				continue;

			Vector vecResult;

			float verticalVelocity = SimpleSplineRemapVal(flDistance, 0.0f, VERT_RADIUS, VERT_MAX_VERTICAL_VELOCITY, VERT_MIN_VERTICAL_VELOCITY);
			float lateralVelocityScale = SimpleSplineRemapVal(flDistance, 0.0f, VERT_RADIUS, VERT_MIN_LATERAL_VELOCITY_SCALE, VERT_MAX_LATERAL_VELOCITY_SCALE);

			Vector playerVelocity = pPlayer->GetAbsVelocity();

			//pPlayer->SetAbsVelocity(vecDisplacement);
			vecResult = Vector( playerVelocity.x * lateralVelocityScale, playerVelocity.y * lateralVelocityScale, verticalVelocity );
			
			pPlayer->SetAbsVelocity(vecResult);
		}

		UTIL_Remove(this);
	}

#endif
