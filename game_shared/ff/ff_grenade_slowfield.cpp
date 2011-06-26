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
#include "beam_shared.h"


#ifdef GAME_DLL
	#include "ff_entity_system.h"
	#include "te_effect_dispatch.h"
	#include "ai_basenpc.h"
#else
	#include "c_te_effect_dispatch.h"
#endif

#define SLOWFIELDGRENADE_MODEL			"models/grenades/gas/gas.mdl"

ConVar ffdev_slowfield_vmt("ffdev_slowfield_vmt", "sprites/ff_slowfieldoutline1.vmt", FCVAR_FF_FFDEV_REPLICATED, "Sprite texture");
#define SLOWFIELDGRENADE_GLOW_SPRITE ffdev_slowfield_vmt.GetString()

//#define SLOWFIELDGRENADE_GLOW_SPRITE	"sprites/ff_slowfieldoutline1.vmt"
#define SLOWFIELDGRENADE_SOUND			"Slowfield.Explode"
#define SLOWFIELDGRENADE_LOOP			"Slowfield.SlowLoop"
#define SLOWFIELDGRENADE_BEAM_LOOP		"Slowfield.LaserLoop"
#define SLOWFIELD_EFFECT				"FF_SlowFieldEffect" // "ConcussionExplosion"
#define CONCBITS_EFFECT					"FF_ConcBitsEffect"
#define FLASH_EFFECT					"FF_FlashEffect"
#define RING_EFFECT						"FF_RingEffect"

#ifdef CLIENT_DLL

ConVar slowfield_glow_r("ffdev_slowfield_glow_r", "255", FCVAR_FF_FFDEV_CLIENT, "Slowfield glow red(0-255) ");
ConVar slowfield_glow_g("ffdev_slowfield_glow_g", "255", FCVAR_FF_FFDEV_CLIENT, "Slowfield glow green(0-255) ");
ConVar slowfield_glow_b("ffdev_slowfield_glow_b", "200", FCVAR_FF_FFDEV_CLIENT, "Slowfield glow blue(0-255) ");
ConVar slowfield_glow_a("ffdev_slowfield_glow_a", "0.8", FCVAR_FF_FFDEV_CLIENT, "Slowfield glow alpha(0-1) ");
ConVar slowfield_glow_a_min("ffdev_slowfield_glow_a_min", "180", FCVAR_FF_FFDEV_CLIENT, "Slowfield glow alpha min(0-255) ");
ConVar slowfield_glow_a_max("ffdev_slowfield_glow_a_max", "200", FCVAR_FF_FFDEV_CLIENT, "Slowfield glow alpha max(0-255) ");
ConVar slowfield_glow_size("ffdev_slowfield_glow_size", "0.3", FCVAR_FF_FFDEV_CLIENT, "Slowfield glow size(0.0-10.0");

#define SLOWFIELD_GLOW_R slowfield_glow_r.GetFloat()
#define SLOWFIELD_GLOW_G slowfield_glow_g.GetFloat()
#define SLOWFIELD_GLOW_B slowfield_glow_b.GetFloat()
#define SLOWFIELD_GLOW_A slowfield_glow_a.GetFloat()
#define SLOWFIELD_GLOW_A_MIN slowfield_glow_a_min.GetFloat()
#define SLOWFIELD_GLOW_A_MAX slowfield_glow_a_max.GetFloat()
#define SLOWFIELD_GLOW_SIZE slowfield_glow_size.GetFloat()

#endif

#define GRENADE_BEAM_SPRITE	"sprites/plasma.spr"

ConVar ffdev_slowfield_beam_widthstart("ffdev_slowfield_beam_widthstart", "4", FCVAR_FF_FFDEV_REPLICATED, "Width at the start of the slowfield grenade beam");
#define SLOWFIELD_BEAM_WIDTHSTART ffdev_slowfield_beam_widthstart.GetFloat()

ConVar ffdev_slowfield_beam_widthend("ffdev_slowfield_beam_widthend", "4", FCVAR_FF_FFDEV_REPLICATED, "Width at the end of the slowfield grenade beam");
#define SLOWFIELD_BEAM_WIDTHEND ffdev_slowfield_beam_widthend.GetFloat()

ConVar ffdev_slowfield_beam_noise("ffdev_slowfield_beam_noise", "0.5", FCVAR_FF_FFDEV_REPLICATED, "Noise of the slowfield grenade beam");
#define SLOWFIELD_BEAM_NOISE ffdev_slowfield_beam_noise.GetFloat()

ConVar ffdev_slowfield_shrinktime_stack("ffdev_slowfield_shrinktime_stack", "1", FCVAR_FF_FFDEV_REPLICATED, "If stacked (1) then the slowfiled inner radius shrinks followed by outer");
#define SLOWFIELD_SHRINKTIME_STACK ffdev_slowfield_shrinktime_stack.GetBool()

ConVar ffdev_slowfield_shrinktime_inner("ffdev_slowfield_shrinktime_inner", "0.15", FCVAR_FF_FFDEV_REPLICATED, "Time it takes the slowfiled inner radius to shrink when the slowfield expires");
#define SLOWFIELD_SHRINKTIME_INNER ffdev_slowfield_shrinktime_inner.GetFloat()

ConVar ffdev_slowfield_shrinktime_outer("ffdev_slowfield_shrinktime_outer", "0.25", FCVAR_FF_FFDEV_REPLICATED, "Time it takes the slowfiled outer radius to shrink when the slowfield expires");
#define SLOWFIELD_SHRINKTIME_OUTER ffdev_slowfield_shrinktime_outer.GetFloat()

ConVar ffdev_slowfield_radius_outer("ffdev_slowfield_radius_outer", "200", FCVAR_FF_FFDEV_REPLICATED, "Outer radius of slowfield grenade (scales from no effect to full effect at inner radius)");
#define SLOWFIELD_RADIUS_OUTER ffdev_slowfield_radius_outer.GetFloat()

ConVar ffdev_slowfield_radius_inner("ffdev_slowfield_radius_inner", "176", FCVAR_FF_FFDEV_REPLICATED, "Inner radius of slowfield grenade (where slowfield has full effect)");
#define SLOWFIELD_RADIUS_INNER ffdev_slowfield_radius_inner.GetFloat()

ConVar ffdev_slowfield_radius_power("ffdev_slowfield_radius_power", "3", FCVAR_FF_FFDEV_REPLICATED, "Power by which to raise the pecentage that a player is between inner & outer radius of the slowfield grenade (so it ramps up and feels less like a brick wall)");
#define SLOWFIELD_RADIUS_POWER ffdev_slowfield_radius_power.GetFloat()

ConVar ffdev_slowfield_duration("ffdev_slowfield_duration", "10", FCVAR_FF_FFDEV_REPLICATED, "Duration of slowfield grenade");
#define SLOWFIELD_DURATION ffdev_slowfield_duration.GetFloat()

ConVar ffdev_slowfield_multiplier("ffdev_slowfield_multiplier", "28", FCVAR_FF_FFDEV_REPLICATED, "Multiplier to compensate for the effect of the ffdev_slowfield_power cvar");
#define SLOWFIELD_MULTIPLIER ffdev_slowfield_multiplier.GetFloat()

ConVar ffdev_slowfield_power("ffdev_slowfield_power", "0.35", FCVAR_FF_FFDEV_REPLICATED, "Lower Power = slow fast players more", true, 0.0f, true, 1.0f);
#define SLOWFIELD_POWER ffdev_slowfield_power.GetFloat()

ConVar ffdev_slowfield_friendlyignore("ffdev_slowfield_friendlyignore", "1", FCVAR_FF_FFDEV_REPLICATED, "When set to 1 and friendly fire is off, the grenade does not affect teammates");
#define SLOWFIELD_FRIENDLYIGNORE ffdev_slowfield_friendlyignore.GetBool()

ConVar ffdev_slowfield_selfignore("ffdev_slowfield_selfignore", "0", FCVAR_FF_FFDEV_REPLICATED, "When set to 1, the grenade does not affect the thrower");
#define SLOWFIELD_SELFIGNORE ffdev_slowfield_selfignore.GetBool()

ConVar ffdev_slowfield_friendlyscale("ffdev_slowfield_friendlyscale", "1", FCVAR_FF_FFDEV_REPLICATED, "When friendly fire is on, modifies the slow amount for teammates", true, 0.0f, true, 1.0f);
#define SLOWFIELD_FRIENDLYSCALE ffdev_slowfield_friendlyscale.GetFloat()

ConVar ffdev_slowfield_selfscale("ffdev_slowfield_selfscale", "1", FCVAR_FF_FFDEV_REPLICATED, "When selfignore is 0, modifies the slow amount for the thrower", true, 0.0f, true, 1.0f);
#define SLOWFIELD_SELFSCALE ffdev_slowfield_selfscale.GetFloat()

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
	virtual float GetGrenadeRadius()		{ return SLOWFIELD_RADIUS_OUTER; }
	virtual float GetShakeAmplitude()		{ return 0.0f; }

	virtual Class_T Classify( void ) { return CLASS_GREN_SLOWFIELD; } 

	virtual color32 GetColour() { color32 col = { 255, 225, 255, GREN_ALPHA_DEFAULT }; return col; }

	virtual void StopLoopingSounds( void );
	virtual void UpdateOnRemove( void );

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
	bool m_bBeamLoopPlaying;
	float	m_flLastThinkTime;

	int m_iSequence;
	Activity m_Activity;

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
	PrecacheScriptSound(SLOWFIELDGRENADE_LOOP);
	PrecacheScriptSound(SLOWFIELDGRENADE_BEAM_LOOP);

	BaseClass::Precache();
}

void CFFGrenadeSlowfield::StopLoopingSounds()
{
#ifdef GAME_DLL
	StopSound(SLOWFIELDGRENADE_LOOP);
	if(m_bBeamLoopPlaying)
	{
		m_bBeamLoopPlaying = false;
		StopSound(SLOWFIELDGRENADE_BEAM_LOOP);
	}
#endif
}

void CFFGrenadeSlowfield::UpdateOnRemove()
{
#ifdef GAME_DLL
	// loop through all players
	for(int i = 1 ; i <= gpGlobals->maxClients; i++)
	{
		CFFPlayer* pPlayer = ToFFPlayer(UTIL_EntityByIndex(i));

		if( !pPlayer || pPlayer->IsObserver() )
			continue;

		if (pPlayer->GetActiveSlowfield() == this)
		{
			pPlayer->SetLaggedMovementValue(1.0f);
			pPlayer->SetActiveSlowfield( NULL );
			
			// remove status icon
			CSingleUserRecipientFilter user( ( CBasePlayer * )pPlayer );
			user.MakeReliable();

			UserMessageBegin( user, "StatusIconUpdate" );
				WRITE_BYTE( FF_STATUSICON_SLOWMOTION );
				WRITE_FLOAT( 0.0f );
			MessageEnd();
		}
	}
#endif
	BaseClass::UpdateOnRemove();
}

#ifdef GAME_DLL

	extern Activity ACT_GAS_IDLE;
	extern Activity ACT_GAS_DEPLOY;
	extern Activity ACT_GAS_DEPLOY_IDLE;

	//-----------------------------------------------------------------------------
	// Purpose: Various spawny flag things
	//-----------------------------------------------------------------------------
	void CFFGrenadeSlowfield::Spawn() 
	{
		SetModel(SLOWFIELDGRENADE_MODEL);
		BaseClass::Spawn();
		
		ADD_CUSTOM_ACTIVITY( CFFGrenadeSlowfield, ACT_GAS_IDLE );
		ADD_CUSTOM_ACTIVITY( CFFGrenadeSlowfield, ACT_GAS_DEPLOY );
		ADD_CUSTOM_ACTIVITY( CFFGrenadeSlowfield, ACT_GAS_DEPLOY_IDLE );

		m_Activity = ( Activity )ACT_GAS_IDLE;
		m_iSequence = SelectWeightedSequence( m_Activity );
		m_bBeamLoopPlaying = false;
		SetSequence( m_iSequence );		
	}

	//-----------------------------------------------------------------------------
	// Purpose: Instead of exploding, change to 
	//-----------------------------------------------------------------------------
	void CFFGrenadeSlowfield::Explode(trace_t *pTrace, int bitsDamageType)
	{
		CFFPlayer *pSlower = ToFFPlayer( GetOwnerEntity() );
		
		if (!pSlower)
			return;

		EmitSound(SLOWFIELDGRENADE_SOUND);
		
		CEffectData data;
		data.m_vOrigin = GetAbsOrigin();
		data.m_flScale = 1.0f;
		data.m_flRadius = GetGrenadeRadius();

		// using m_nColor as team num
		data.m_nColor = pSlower->GetTeamNumber();
		
		DispatchEffect(SLOWFIELD_EFFECT, data);
		
		/*
		// add the sprite
		//m_hGlowSprite = CSprite::SpriteCreate(CONCUSSIONGRENADE_GLOW_SPRITE, GetAbsOrigin(), false);
		m_hGlowSprite = CFFGrenadeSlowfieldGlow::Create(GetAbsOrigin(), this);
		m_hGlowSprite->SetAttachment(this, LookupAttachment("glowsprite"));
		m_hGlowSprite->SetTransparency(kRenderTransAdd, 255, 255, 255, 128, kRenderFxNone);
		m_hGlowSprite->SetBrightness(255, 0.2f);
		m_hGlowSprite->SetScale(1.0f, 0.2f);
		*/

		// Clumsy, will do for now
		if (GetMoveType() == MOVETYPE_FLY)
		{
			BaseClass::Explode(pTrace, bitsDamageType);
			return;
		}

		SetDetonateTimerLength(SLOWFIELD_DURATION);
		m_bIsOn = true;

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
		// If we're done deploying, deploy idle
		if( m_Activity == ACT_GAS_DEPLOY )
		{
			m_Activity = ACT_GAS_DEPLOY_IDLE;
			m_iSequence = SelectWeightedSequence( m_Activity );
			SetSequence( m_iSequence );
		}
		
		// If we were idling, deploy
		if( m_Activity == ACT_GAS_IDLE )
		{
			m_Activity = ACT_GAS_DEPLOY;
			m_iSequence = SelectWeightedSequence( m_Activity );
			SetSequence( m_iSequence );
		}

		// Blow up if we've reached the end of our fuse
		if (gpGlobals->curtime > m_flDetonateTime) 
		{
			UTIL_Remove(this);
			return;
		}

		// emit looping sound
		EmitSound(SLOWFIELDGRENADE_LOOP);

		float flRisingheight = 0;

		float flOuterStartShrinkTime = m_flDetonateTime - SLOWFIELD_SHRINKTIME_OUTER;
		float flOuterRadiusShrink = 1.0f;
		if( gpGlobals->curtime >= flOuterStartShrinkTime )
			flOuterRadiusShrink = 1 - ( gpGlobals->curtime - flOuterStartShrinkTime ) / ( m_flDetonateTime - flOuterStartShrinkTime );

		bool bShrinkStack = SLOWFIELD_SHRINKTIME_STACK;
	
		float flInnerStartShrinkTime = ( bShrinkStack ? m_flDetonateTime - ( SLOWFIELD_SHRINKTIME_OUTER + SLOWFIELD_SHRINKTIME_INNER ) : m_flDetonateTime - SLOWFIELD_SHRINKTIME_INNER );
		float flInnerRadiusShrink = 1.0f;

		if( bShrinkStack && gpGlobals->curtime >= flOuterStartShrinkTime )
			flInnerRadiusShrink = 0.0f;
		else if( bShrinkStack && gpGlobals->curtime >= flInnerStartShrinkTime )
			flInnerRadiusShrink = 1 - ( gpGlobals->curtime - flInnerStartShrinkTime ) / ( flOuterStartShrinkTime - flInnerStartShrinkTime );
		else if( gpGlobals->curtime >= flInnerStartShrinkTime )
			flInnerRadiusShrink = 1 - ( gpGlobals->curtime - flInnerStartShrinkTime ) / ( m_flDetonateTime - flInnerStartShrinkTime );

		float flInnerRadius = SLOWFIELD_RADIUS_INNER * flInnerRadiusShrink;
		float flOuterRadius = SLOWFIELD_RADIUS_OUTER * flOuterRadiusShrink;

		// Lasts for 3 seconds, rise for 0.3, but only if not handheld
		//if (gpGlobals->curtime > m_flDetonateTime - 0.3 && !m_fIsHandheld)
		//	flRisingheight = 80;

		SetAbsVelocity(Vector(0, 0, flRisingheight + 20 * sin(DEG2RAD(GetAbsAngles().y))));
		SetAbsAngles(GetAbsAngles() + QAngle(0, 15, 0));

		Vector vecOrigin = GetAbsOrigin();

		bool bHitPlayer = false;

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

			Vector vecDisplacement = pPlayer->GetLegacyAbsOrigin() - vecOrigin;
			float flDistance = vecDisplacement.Length();

			// inside the radius of the gren
			if (flDistance < GetGrenadeRadius())
			{
 				if( SLOWFIELD_FRIENDLYIGNORE && !g_pGameRules->FCanTakeDamage( pPlayer, GetOwnerEntity() ) )
					continue;
				
				if( SLOWFIELD_SELFIGNORE && pPlayer == pSlower )
					continue;

				float flFriendlyScale = 1.0f;

				// Check if is a teammate and scale accordingly
				if (pPlayer != pSlower && g_pGameRules->PlayerRelationship(pPlayer, pSlower) == GR_TEAMMATE)
					flFriendlyScale = SLOWFIELD_FRIENDLYSCALE;
				else if (pPlayer == pSlower)
					flFriendlyScale = SLOWFIELD_SELFSCALE;

				float flDistanceMult = 1.0f;
				//if we're scaling between outer and inner radius (linear!!)
				//don't allow divide by zero or for inner/outer to be reversed
				if(flDistance > (flInnerRadius * flInnerRadiusShrink) && ( flOuterRadius - flInnerRadius ) > 0.0f)
				{
					flDistanceMult = clamp(1.0f - ( flDistance - flInnerRadius ) / ( flOuterRadius - flInnerRadius ), 0.0f, 1.0f);
				}

				float flSpeed = pPlayer->GetAbsVelocity().Length();
				float flSpeedReduction = flSpeed - ( pow( flSpeed, SLOWFIELD_POWER ) * SLOWFIELD_MULTIPLIER );
				flSpeedReduction *= pow( flDistanceMult, SLOWFIELD_RADIUS_POWER );
				flSpeedReduction *= flFriendlyScale;

				float flLaggedMovement = 1.0f;
				if(flSpeed > 0.0f)
				//no divide by zero
				{
					flLaggedMovement = clamp( (flSpeed - flSpeedReduction), 1.0f, flSpeed ) / flSpeed;
				}

				// only change players active slowfield if they will be going slower
				if (pPlayer->GetActiveSlowfield() != this && pPlayer->GetLaggedMovementValue() > flLaggedMovement || pPlayer->GetActiveSlowfield() == NULL)
				{
					pPlayer->SetLaggedMovementValue(flLaggedMovement);
					pPlayer->SetActiveSlowfield( this );

					// add status icon
					CSingleUserRecipientFilter user( ( CBasePlayer * )pPlayer );
					user.MakeReliable();

					UserMessageBegin( user, "StatusIconUpdate" );
						WRITE_BYTE( FF_STATUSICON_SLOWMOTION );
						WRITE_FLOAT( -1.0f );
					MessageEnd();
				}
				// else just give them an updated laggedmovement value
				else if (pPlayer->GetActiveSlowfield() == this)
				{
					pPlayer->SetLaggedMovementValue(flLaggedMovement);
				}		

				CFFPlayer *pGrenOwner = ToFFPlayer( this->GetOwnerEntity() );

				bHitPlayer = true;
	
				CBeam *pBeam = CBeam::BeamCreate( GRENADE_BEAM_SPRITE, 1 );
				pBeam->SetWidth( SLOWFIELD_BEAM_WIDTHSTART );
				pBeam->SetEndWidth( SLOWFIELD_BEAM_WIDTHEND );
				pBeam->LiveForTime(gpGlobals->interval_per_tick);
				pBeam->SetNoise( SLOWFIELD_BEAM_NOISE );
				pBeam->SetBrightness( (1 - flLaggedMovement) * 128 + 128 );
				if(pGrenOwner->GetTeamNumber() == TEAM_RED)
					pBeam->SetColor( 255, 64, 64 );
				else if(pGrenOwner->GetTeamNumber() == TEAM_BLUE)
					pBeam->SetColor( 64, 128, 255 );
				else if(pGrenOwner->GetTeamNumber() == TEAM_GREEN)
					pBeam->SetColor( 153, 255, 153 );
				else if(pGrenOwner->GetTeamNumber() == TEAM_YELLOW)
					pBeam->SetColor( 255, 178, 0 );
				else // just in case
					pBeam->SetColor( 204, 204, 204 );
				pBeam->PointsInit( vecOrigin, pPlayer->GetLegacyAbsOrigin() );
			}
			// outside the radius of the gren
			else if (pPlayer->GetActiveSlowfield() == this)
			{
				pPlayer->SetLaggedMovementValue( 1.0f );
				pPlayer->SetActiveSlowfield( NULL );
				
				// remove status icon
				CSingleUserRecipientFilter user( ( CBasePlayer * )pPlayer );
				user.MakeReliable();

				UserMessageBegin( user, "StatusIconUpdate" );
					WRITE_BYTE( FF_STATUSICON_SLOWMOTION );
					WRITE_FLOAT( 0.0f );
				MessageEnd();
			}
		}


		if(!bHitPlayer && m_bBeamLoopPlaying)
		{
			m_bBeamLoopPlaying = false;
			StopSound( SLOWFIELDGRENADE_BEAM_LOOP );
		}
		else if(bHitPlayer && !m_bBeamLoopPlaying)
		{
			m_bBeamLoopPlaying = true;
			EmitSound( SLOWFIELDGRENADE_BEAM_LOOP );
		}

		// Animate
		StudioFrameAdvance();

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
#ifdef _DEBUG
	Assert(slowgren != 0);
#endif

	if (!slowgren)
		return 0;

	/* I (Elmo) presume that this commented code is a TODO/TO-DO
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
		SLOWFIELD_RADIUS_INNER / 64 * SLOWFIELD_GLOW_SIZE);			// sprite scale

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
