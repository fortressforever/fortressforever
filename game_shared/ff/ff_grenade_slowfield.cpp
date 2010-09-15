/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_grenade_nail.cpp
/// @author Ryan "squeek" Liptak
/// @date September 2009
/// @brief The slow field grenade
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

#define SLOWFIELDGRENADE_MODEL			"models/grenades/conc/conc.mdl"
#define SLOWFIELDGRENADE_GLOW_SPRITE	"sprites/glow04_ring.vmt"
#define SLOWFIELDGRENADE_SOUND			"ConcussionGrenade.Explode"
#define SLOWFIELD_EFFECT				"FF_ConcussionEffect" // "ConcussionExplosion"
#define CONCBITS_EFFECT					"FF_ConcBitsEffect"
#define FLASH_EFFECT					"FF_FlashEffect"
#define RING_EFFECT						"FF_RingEffect"

#ifdef CLIENT_DLL

ConVar slowfield_glow_r("ffdev_slowfield_glow_r", "255", FCVAR_CHEAT, "Slowfield glow red(0-255) ");
ConVar slowfield_glow_g("ffdev_slowfield_glow_g", "255", FCVAR_CHEAT, "Slowfield glow green(0-255) ");
ConVar slowfield_glow_b("ffdev_slowfield_glow_b", "200", FCVAR_CHEAT, "Slowfield glow blue(0-255) ");
ConVar slowfield_glow_a("ffdev_slowfield_glow_a", "0.8", FCVAR_CHEAT, "Slowfield glow alpha(0-1) ");
ConVar slowfield_glow_a_min("ffdev_slowfield_glow_a_min", "180", FCVAR_CHEAT, "Slowfield glow alpha min(0-255) ");
ConVar slowfield_glow_a_max("ffdev_slowfield_glow_a_max", "200", FCVAR_CHEAT, "Slowfield glow alpha max(0-255) ");
ConVar slowfield_glow_size("ffdev_slowfield_glow_size", "1.0", FCVAR_CHEAT, "Slowfield glow size(0.0-10.0");

#define SLOWFIELD_GLOW_R slowfield_glow_r.GetFloat()
#define SLOWFIELD_GLOW_G slowfield_glow_g.GetFloat()
#define SLOWFIELD_GLOW_B slowfield_glow_b.GetFloat()
#define SLOWFIELD_GLOW_A slowfield_glow_a.GetFloat()
#define SLOWFIELD_GLOW_A_MIN slowfield_glow_a_min.GetFloat()
#define SLOWFIELD_GLOW_A_MAX slowfield_glow_a_max.GetFloat()
#define SLOWFIELD_GLOW_SIZE slowfield_glow_size.GetFloat()

#endif

ConVar ffdev_slowfield_radius("ffdev_slowfield_radius", "256", FCVAR_REPLICATED/* | FCVAR_CHEAT */, "Radius of slowfield grenade");
#define SLOWFIELD_RADIUS ffdev_slowfield_radius.GetFloat()

ConVar ffdev_slowfield_duration("ffdev_slowfield_duration", "6", FCVAR_REPLICATED/* | FCVAR_CHEAT */, "Duration of slowfield grenade");
#define SLOWFIELD_DURATION ffdev_slowfield_duration.GetFloat()

ConVar ffdev_slowfield_min_slow("ffdev_slowfield_min_slow", ".20", FCVAR_REPLICATED/* | FCVAR_CHEAT */, "Minimum slow motion percentage of slowfield grenade");
#define SLOWFIELD_MIN_SLOW ffdev_slowfield_min_slow.GetFloat()

ConVar ffdev_slowfield_friendlyignore("ffdev_slowfield_friendlyignore", "1", FCVAR_REPLICATED/* | FCVAR_CHEAT */, "When set to 1 and friendly fire is off, the grenade does not affect teammates");
#define SLOWFIELD_FRIENDLYIGNORE ffdev_slowfield_friendlyignore.GetBool()

ConVar ffdev_slowfield_selfignore("ffdev_slowfield_selfignore", "0", FCVAR_REPLICATED/* | FCVAR_CHEAT */, "When set to 1, the grenade does not affect the thrower");
#define SLOWFIELD_SELFIGNORE ffdev_slowfield_selfignore.GetBool()

ConVar ffdev_slowfield_friendlyscale("ffdev_slowfield_friendlyscale", ".35", FCVAR_REPLICATED/* | FCVAR_CHEAT */, "When friendly fire is on, modifies the slow amount for teammates");
#define SLOWFIELD_FRIENDLYSCALE ffdev_slowfield_friendlyscale.GetFloat()

ConVar ffdev_slowfield_selfscale("ffdev_slowfield_selfscale", ".35", FCVAR_REPLICATED/* | FCVAR_CHEAT */, "When selfignore is 0, modifies the slow amount for the thrower");
#define SLOWFIELD_SELFSCALE ffdev_slowfield_selfscale.GetFloat()

ConVar ffdev_slowfield_fastspeedmod_start("ffdev_slowfield_fastspeedmod_start", "800", FCVAR_REPLICATED/* | FCVAR_CHEAT */, "When the slowed person is above this speed, they get slowed more depending on how fast they are moving");
#define SLOWFIELD_FASTSPEEDMOD_START ffdev_slowfield_fastspeedmod_start.GetFloat()



#ifdef CLIENT_DLL
	#define CFFGrenadeSlowfield C_FFGrenadeSlowfield
	#define CFFGrenadeSlowfieldGlow C_FFGrenadeSlowfieldGlow
#endif

//=============================================================================
// CFFGrenadeSlowfieldGlow
//=============================================================================
class CFFGrenadeSlowfieldGlow : public CSprite
{
public:
	DECLARE_CLASS(CFFGrenadeSlowfieldGlow, CSprite);

	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

	int	ObjectCaps() { return (BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }

#ifdef CLIENT_DLL
	virtual bool			IsTransparent() { return true; }
	virtual RenderGroup_t	GetRenderGroup() { return RENDER_GROUP_TRANSLUCENT_ENTITY; }
	virtual int				DrawModel(int flags);
	virtual void			OnDataChanged(DataUpdateType_t updateType);
	virtual bool			ShouldDraw() { return (IsEffectActive(EF_NODRAW) ==false); }
#else
	static CFFGrenadeSlowfieldGlow *Create(const Vector &origin, CBaseEntity *pOwner = NULL);
#endif
};

//=============================================================================
// CFFGrenadeSlowfieldGlow tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeSlowfieldGlow, DT_FFGrenadeSlowfieldGlow)

BEGIN_NETWORK_TABLE(CFFGrenadeSlowfieldGlow, DT_FFGrenadeSlowfieldGlow)
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(env_ffslowfieldglow, CFFGrenadeSlowfieldGlow);

BEGIN_DATADESC(CFFGrenadeSlowfieldGlow)
END_DATADESC()

//=============================================================================
// CFFGrenadeSlowfield
//=============================================================================

class CFFGrenadeSlowfield : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeSlowfield, CFFGrenadeBase) 
	DECLARE_NETWORKCLASS(); 

	CNetworkVector(m_vInitialVelocity);

	virtual void Precache();
	virtual const char *GetBounceSound() { return "ConcussionGrenade.Bounce"; }
	
	virtual float GetGrenadeDamage()		{ return 0.0f; }
	virtual float GetGrenadeRadius()		{ return SLOWFIELD_RADIUS; }
	virtual float GetShakeAmplitude()		{ return 0.0f; }

	virtual Class_T Classify( void ) { return CLASS_GREN_SLOWFIELD; } 

	virtual color32 GetColour() { color32 col = { 255, 225, 255, GREN_ALPHA_DEFAULT }; return col; }

	CHandle<CFFGrenadeSlowfieldGlow> m_hGlowSprite;

#ifdef CLIENT_DLL
	CFFGrenadeSlowfield() {}
	CFFGrenadeSlowfield(const CFFGrenadeSlowfield&) {}
#else
	DECLARE_DATADESC(); // Since we're adding new thinks etc
	virtual void Spawn();
	virtual void SlowThink();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);

protected:

	float	m_flLastThinkTime;

#endif
};

#ifdef GAME_DLL
	BEGIN_DATADESC(CFFGrenadeSlowfield) 
		DEFINE_THINKFUNC(SlowThink), 
	END_DATADESC() 
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeSlowfield, DT_FFGrenadeSlowfield)

BEGIN_NETWORK_TABLE(CFFGrenadeSlowfield, DT_FFGrenadeSlowfield)
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(ff_grenade_slowfield, CFFGrenadeSlowfield);
PRECACHE_WEAPON_REGISTER(ff_grenade_slowfield);

//-----------------------------------------------------------------------------
// Purpose: Various precache things
//-----------------------------------------------------------------------------
void CFFGrenadeSlowfield::Precache() 
{
	PrecacheModel(SLOWFIELDGRENADE_MODEL);
	PrecacheModel("models/grenades/conc/conceffect.mdl");
	PrecacheModel(SLOWFIELDGRENADE_GLOW_SPRITE);
	PrecacheScriptSound(SLOWFIELDGRENADE_SOUND);

	BaseClass::Precache();
}

#ifdef GAME_DLL

	//-----------------------------------------------------------------------------
	// Purpose: Various spawny flag things
	//-----------------------------------------------------------------------------
	void CFFGrenadeSlowfield::Spawn() 
	{
		SetModel(SLOWFIELDGRENADE_MODEL);
		BaseClass::Spawn();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Instead of exploding, change to 
	//-----------------------------------------------------------------------------
	void CFFGrenadeSlowfield::Explode(trace_t *pTrace, int bitsDamageType)
	{
		EmitSound(SLOWFIELDGRENADE_SOUND);

		/*
		CEffectData data;
		data.m_vOrigin = GetAbsOrigin();
		data.m_flScale = 1.0f;
		data.m_flRadius = GetGrenadeRadius();
		
		DispatchEffect(SLOWFIELD_EFFECT, data);
		*/
		
		// add the sprite
		//m_hGlowSprite = CSprite::SpriteCreate(CONCUSSIONGRENADE_GLOW_SPRITE, GetAbsOrigin(), false);
		m_hGlowSprite = CFFGrenadeSlowfieldGlow::Create(GetAbsOrigin(), this);
		m_hGlowSprite->SetAttachment(this, LookupAttachment("glowsprite"));
		m_hGlowSprite->SetTransparency(kRenderTransAdd, 255, 255, 255, 128, kRenderFxNone);
		m_hGlowSprite->SetBrightness(255, 0.2f);
		m_hGlowSprite->SetScale(1.0f, 0.2f);

		// Clumsy, will do for now
		if (GetMoveType() == MOVETYPE_FLY)
		{
			BaseClass::Explode(pTrace, bitsDamageType);
			return;
		}

		SetDetonateTimerLength(SLOWFIELD_DURATION);

		// Should this maybe be noclip?
		SetMoveType(MOVETYPE_FLY);

		// Go into slow mode
		SetThink(&CFFGrenadeSlowfield::SlowThink);
		SetNextThink(gpGlobals->curtime);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Spin round emitting nails
	//-----------------------------------------------------------------------------
	void CFFGrenadeSlowfield::SlowThink() 
	{
		// Blow up if we've reached the end of our fuse
		if (gpGlobals->curtime > m_flDetonateTime) 
		{
			UTIL_Remove(this);

			
			CBaseEntity *pEntity = NULL;
			for( CEntitySphereQuery sphere( GetAbsOrigin(), GetGrenadeRadius() + 64.0f ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
			{
				if( !pEntity )
					continue;

				if( !pEntity->IsPlayer() )
					continue;

				CFFPlayer *pPlayer = ToFFPlayer( pEntity );
				//CFFPlayer *pSlower = ToFFPlayer( GetOwnerEntity() );

				if( !pPlayer || pPlayer->IsObserver() )
					continue;

				//if( !g_pGameRules->FCanTakeDamage( pPlayer, GetOwnerEntity() ) )
				//	continue;

				pPlayer->SetLaggedMovementValue(1.0f);
				pPlayer->SetActiveSlowfield( NULL );
			}

			return;
		}

		float flRisingheight = 0;

		// Lasts for 3 seconds, rise for 0.3, but only if not handheld
		//if (gpGlobals->curtime > m_flDetonateTime - 0.3 && !m_fIsHandheld)
		//	flRisingheight = 80;

		SetAbsVelocity(Vector(0, 0, flRisingheight + 20 * sin(DEG2RAD(GetAbsAngles().y))));
		SetAbsAngles(GetAbsAngles() + QAngle(0, 15, 0));

		Vector vecOrigin = GetAbsOrigin();

		CBaseEntity *pEntity = NULL;
		for( CEntitySphereQuery sphere( GetAbsOrigin(), GetGrenadeRadius() + 64.0f ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			if( !pEntity )
				continue;

			if( !pEntity->IsPlayer() )
				continue;

			CFFPlayer *pPlayer = ToFFPlayer( pEntity );
			CFFPlayer *pSlower = ToFFPlayer( GetOwnerEntity() );

			if( !pPlayer || pPlayer->IsObserver() || !pSlower)
				continue;

			if( SLOWFIELD_FRIENDLYIGNORE && !g_pGameRules->FCanTakeDamage( pPlayer, GetOwnerEntity() ) )
				continue;
			
			if( SLOWFIELD_SELFIGNORE && pPlayer == pSlower )
				continue;

			Vector vecDisplacement = pPlayer->GetLegacyAbsOrigin() - vecOrigin;
			float flDistance = vecDisplacement.Length();

			if (flDistance < GetGrenadeRadius())
			{
				float flFriendlyScale = 1.0f;

				// Check if is a teammate and scale accordingly
				if (pPlayer != pSlower && g_pGameRules->PlayerRelationship(pPlayer, pSlower) == GR_TEAMMATE)
					flFriendlyScale = SLOWFIELD_FRIENDLYSCALE;
				else if (pPlayer == pSlower)
					flFriendlyScale = SLOWFIELD_SELFSCALE;

				Vector vecVelocity = pPlayer->GetAbsVelocity();
				Vector vecLatVelocity = vecVelocity * Vector(1.0f, 1.0f, 0.0f);
				float flHorizontalSpeed = vecLatVelocity.Length();

				float flFastSpeedMod = 1 / max(1.0f, flHorizontalSpeed / SLOWFIELD_FASTSPEEDMOD_START);

				float flLaggedMovement = SimpleSplineRemapVal(flDistance, 0.0f, SLOWFIELD_RADIUS, min( 1.0f, (flFriendlyScale > 0) ? (SLOWFIELD_MIN_SLOW / flFriendlyScale * flFastSpeedMod) : (1.0f) ), 1.0f);
				
				// only change players active slowfield if they will be going slower
				if (pPlayer->GetActiveSlowfield() != this && pPlayer->GetLaggedMovementValue() > flLaggedMovement)
				{
					pPlayer->SetLaggedMovementValue(flLaggedMovement);
					pPlayer->SetActiveSlowfield( this );
				}
				// else just give them an updated laggedmovement value
				else if (pPlayer->GetActiveSlowfield() == this)
				{
					pPlayer->SetLaggedMovementValue(flLaggedMovement);
				}
			}
			else if (pPlayer->GetActiveSlowfield() == this)
			{
				pPlayer->SetLaggedMovementValue( 1.0f );
				pPlayer->SetActiveSlowfield( NULL );
			}
		}

		SetNextThink(gpGlobals->curtime);
		m_flLastThinkTime = gpGlobals->curtime;
	}




#endif

	
//=============================================================================
// CFFGrenadeSlowfieldGlow implementation
//=============================================================================

#ifdef GAME_DLL

CFFGrenadeSlowfieldGlow *CFFGrenadeSlowfieldGlow::Create(const Vector &origin, CBaseEntity *pOwner)
{
	CFFGrenadeSlowfieldGlow *pSlowGlow = (CFFGrenadeSlowfieldGlow *) CBaseEntity::Create("env_ffslowfieldglow", origin, QAngle(0, 0, 0));

	if (pSlowGlow == NULL)
		return NULL;

	pSlowGlow->SetRenderMode(kRenderWorldGlow/*(RenderMode_t) 9*/);

	pSlowGlow->SetMoveType(MOVETYPE_NONE);
	pSlowGlow->AddSolidFlags(FSOLID_NOT_SOLID);
	pSlowGlow->AddEffects(EF_NOSHADOW);
	UTIL_SetSize(pSlowGlow, vec3_origin, vec3_origin);

	pSlowGlow->SetOwnerEntity(pOwner);

	pSlowGlow->AddEFlags(EFL_FORCE_CHECK_TRANSMIT);

	pSlowGlow->SpriteInit(SLOWFIELDGRENADE_GLOW_SPRITE, origin);
	pSlowGlow->SetName(AllocPooledString("TEST"));
	pSlowGlow->SetTransparency(kRenderWorldGlow, 255, 255, 255, 255, kRenderFxNoDissipation);
	pSlowGlow->SetScale(0.25f);
	pSlowGlow->SetOwnerEntity(pOwner);
	pSlowGlow->SetSimulatedEveryTick(true);

	return pSlowGlow;
}

#else

int CFFGrenadeSlowfieldGlow::DrawModel(int flags)
{
	//See if we should draw
	if (m_bReadyToDraw == false)
		return 0;

	//Must be a sprite
	if (modelinfo->GetModelType(GetModel()) != mod_sprite)
	{
		assert(0);
		return 0;
	}

	CFFGrenadeSlowfield *slowgren = dynamic_cast<CFFGrenadeSlowfield *> (GetOwnerEntity());

	if (!slowgren)
		return 0;

	/*
	THIS SHOULD NOT STAY LIKE THIS, MAKES THE SPRITE VISIBLE TO ALL PLAYERS AT ALL TIMES

	// Because we're using a NOZ sprite, need to traceline to ensure this is really
	// visible. And we're using a NOZ sprite so it doesnt clip on stuff, okay!
	trace_t tr;
	UTIL_TraceLine(CBasePlayer::GetLocalPlayer()->EyePosition(), GetAbsOrigin(), MASK_VISIBLE, slowgren, COLLISION_GROUP_NONE, &tr);

	if (tr.fraction < 1.0f && tr.m_pEnt != slowgren)
		return 0;
	*/

	Vector vecForward, vecRight, vecUp, vecDir;
	slowgren->GetVectors(&vecForward, &vecRight, &vecUp);
	vecDir = slowgren->GetAbsOrigin() - CBasePlayer::GetLocalPlayer()->EyePosition();
	VectorNormalize(vecDir);

	float alpha = vecUp.Dot(vecDir);

	if (alpha < 0)
		alpha *= -1;
	
	alpha = 1.0f - alpha;
	alpha *= alpha;

	alpha = clamp(SLOWFIELD_GLOW_A * (54.0f + 200.0f * alpha), SLOWFIELD_GLOW_A_MIN, SLOWFIELD_GLOW_A_MAX );

	int drawn = DrawSprite(
		this, 
		GetModel(), 
		GetAbsOrigin(), 
		GetAbsAngles(), 
		m_flFrame, 				// sprite frame to render
		m_hAttachedToEntity, 	// attach to
		m_nAttachment, 			// attachment point
		GetRenderMode(), 		// rendermode
		m_nRenderFX, 
		 (int) alpha, 		// alpha
		/*m_clrRender->r */ SLOWFIELD_GLOW_R, 
		/*m_clrRender->g */ SLOWFIELD_GLOW_G, 
		/*m_clrRender->b */ SLOWFIELD_GLOW_B, 
		SLOWFIELD_RADIUS / 64 * SLOWFIELD_GLOW_SIZE + random->RandomFloat(-0.04f, 0.04f));			// sprite scale

	return drawn;
}

//-----------------------------------------------------------------------------
// Purpose: Setup our sprite reference
//-----------------------------------------------------------------------------
void CFFGrenadeSlowfieldGlow::OnDataChanged(DataUpdateType_t updateType)
{
	if (updateType == DATA_UPDATE_CREATED)
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}
#endif
