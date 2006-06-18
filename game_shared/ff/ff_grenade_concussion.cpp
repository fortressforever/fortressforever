/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_grenade_concussion.cpp
/// @author Shawn Smith(L0ki)
/// @date Jan. 29, 2005
/// @brief concussion grenade class
///
/// Implementation of the CFFGrenadeConcussion class. This is the secondary grenade type for scout and medic.
/// 
/// Revisions
/// ---------
/// Jan. 29, 2005	L0ki: Initial Creation
/// Apr. 23, 2005	L0ki: removed header file, moved everything to a single cpp file
/// Feb. 11, 2006	Mirv: Added new concussion glow effect.

#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"
#include "beam_flags.h"
#include "Sprite.h"
#include "model_types.h"

#ifdef GAME_DLL
	#include "ff_player.h"
#endif

ConVar conc_push_mag("ffdev_conc_mag", "85.0", 0, "Magnitude of the concussion push effect");
ConVar conc_push_base("ffdev_conc_base", "20.0", 0, "Base push of the conc");

#ifdef CLIENT_DLL
	ConVar conc_glow_r("ffdev_conc_glow_r", "255", 0, "Conc glow red(0-255) ");
	ConVar conc_glow_g("ffdev_conc_glow_g", "255", 0, "Conc glow green(0-255) ");
	ConVar conc_glow_b("ffdev_conc_glow_b", "200", 0, "Conc glow blue(0-255) ");
	ConVar conc_glow_a("ffdev_conc_glow_a", "0.8", 0, "Conc glow alpha(0-1) ");
	ConVar conc_glow_size("ffdev_conc_glow_size", "1.0", 0, "Conc glow size(0.0-10.0");
#endif

//ConVar conc_radius("ffdev_conc_radius", "280.0f", 0, "Radius of grenade explosions");

#define CONCUSSIONGRENADE_MODEL "models/grenades/conc/conc.mdl"
#define CONCUSSIONGRENADE_GLOW_SPRITE "sprites/glow04_noz.vmt"
#define CONCUSSION_SOUND "ConcussionGrenade.Explode"
#define CONCUSSION_EFFECT "FF_ConcussionEffect" // "ConcussionExplosion"
#define CONCBITS_EFFECT "FF_ConcBitsEffect"
#define FLASH_EFFECT "FF_FlashEffect"
#define RING_EFFECT "FF_RingEffect"

#ifdef CLIENT_DLL
	#define CFFGrenadeConcussion C_FFGrenadeConcussion
	#define CFFGrenadeConcussionGlow C_FFGrenadeConcussionGlow
#endif

//=============================================================================
// CFFGrenadeConcussionGlow
//=============================================================================
class CFFGrenadeConcussionGlow : public CSprite
{
public:
	DECLARE_CLASS(CFFGrenadeConcussionGlow, CSprite);

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
	static CFFGrenadeConcussionGlow *Create(const Vector &origin, CBaseEntity *pOwner = NULL);
#endif
};

//=============================================================================
// CFFGrenadeConcussionGlow tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeConcussionGlow, DT_FFGrenadeConcussionGlow)

BEGIN_NETWORK_TABLE(CFFGrenadeConcussionGlow, DT_FFGrenadeConcussionGlow)
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(env_ffconcussionglow, CFFGrenadeConcussionGlow);

BEGIN_DATADESC(CFFGrenadeConcussionGlow)
END_DATADESC()

class CFFGrenadeConcussion : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeConcussion, CFFGrenadeBase)

	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC()

	CNetworkVector(m_vInitialVelocity);

	virtual void Precache();
	virtual float GetGrenadeDamage() { return 0.0f; }
	virtual float GetGrenadeRadius() { return 280.0f; }
	virtual const char *GetBounceSound() { return "ConcussionGrenade.Bounce"; }

#ifdef CLIENT_DLL
	CFFGrenadeConcussion() {}
	CFFGrenadeConcussion(const CFFGrenadeConcussion&) {}
	virtual void DoEffectIdle();
#else
	virtual void Spawn();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);
#endif

	CHandle<CFFGrenadeConcussionGlow> m_hGlowSprite;
};

BEGIN_DATADESC(CFFGrenadeConcussion)
END_DATADESC()

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeConcussion, DT_FFGrenadeConcussion)

BEGIN_NETWORK_TABLE(CFFGrenadeConcussion, DT_FFGrenadeConcussion)
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(concussiongrenade, CFFGrenadeConcussion);
PRECACHE_WEAPON_REGISTER(concussiongrenade);

#ifdef GAME_DLL
	void CFFGrenadeConcussion::Spawn()
	{
		SetModel(CONCUSSIONGRENADE_MODEL);

		// add the sprite
		//m_hGlowSprite = CSprite::SpriteCreate(CONCUSSIONGRENADE_GLOW_SPRITE, GetAbsOrigin(), false);
		m_hGlowSprite = CFFGrenadeConcussionGlow::Create(GetAbsOrigin(), this);
		m_hGlowSprite->SetAttachment(this, LookupAttachment("glowsprite"));
		m_hGlowSprite->SetTransparency(kRenderTransAdd, 255, 255, 255, 128, kRenderFxNone);
		m_hGlowSprite->SetBrightness(255, 0.2f);
		m_hGlowSprite->SetScale(1.0f, 0.2f);

		BaseClass::Spawn();
	}

	void CFFGrenadeConcussion::Explode(trace_t *pTrace, int bitsDamageType)
	{
		CFFGrenadeBase::PreExplode(pTrace, CONCUSSION_SOUND, CONCUSSION_EFFECT);
		//CFFGrenadeBase::PreExplode( pTrace, NULL, FLASH_EFFECT );
		CFFGrenadeBase::PreExplode( pTrace, NULL, CONCBITS_EFFECT );

		// If the grenade is in a no gren area don't deploy caltrops
		if( !IsInNoGren() )
		{
			// --> Mirv: Rewritten
			Vector vecDisplacement, vecForce;

			CBaseEntity *pEntity = NULL;
			for( CEntitySphereQuery sphere( GetAbsOrigin(), GetGrenadeRadius() ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
			{
				if( !pEntity )
					continue;

				if( !pEntity->IsPlayer() )
					continue;

				CFFPlayer *pPlayer = ToFFPlayer( pEntity );

				if( !pPlayer->IsAlive() )
					continue;

				// I don't like this macro
			//BEGIN_ENTITY_SPHERE_QUERY(GetAbsOrigin(), GetGrenadeRadius())
				//if (pPlayer)
				//{
				vecDisplacement = pPlayer->GetLegacyAbsOrigin() - GetAbsOrigin();

				// The conc effect is calculated differently if its handheld
				// These values are from TFC
				if (m_fIsHandheld)
				{
					VectorNormalize(vecDisplacement);
					Vector pvel = pPlayer->GetAbsVelocity();

					// These values are from TFC
					pPlayer->SetAbsVelocity(Vector(pvel.x * 2.74, pvel.y * 2.74, pvel.z * 4.12));
				}
				else
				{
					float length = vecDisplacement.Length();
					float multiplier = -0.0247f * length + 11.059f;

					DevMsg("%.2f %.2f\n", length, multiplier);

					vecDisplacement *= multiplier;
					vecDisplacement.z *= 1.5f;

					pPlayer->SetAbsVelocity(vecDisplacement);
				}

				// send the concussion icon to be displayed
				CSingleUserRecipientFilter user((CBasePlayer *) pPlayer);
				user.MakeReliable();

				UserMessageBegin(user, "StatusIconUpdate");
					WRITE_BYTE(FF_ICON_CONCUSSION);
					WRITE_FLOAT(pPlayer->GetClassSlot() == CLASS_MEDIC ? 7.5f : 15.0f);
				MessageEnd();

				VectorNormalize(vecDisplacement);
					
				QAngle angDirection;
				VectorAngles(vecDisplacement, angDirection);

				// only concuss if teamplay rules says the player could be damaged
				if (g_pGameRules->FPlayerCanTakeDamage(pPlayer, GetOwnerEntity()))
				{
					pPlayer->Concuss((pPlayer->GetClassSlot() == 5 ? 7.5f : 15.0f), (pPlayer == GetOwnerEntity() ? NULL : &angDirection));
				}
			}
			// I don't like this macro
			//END_ENTITY_SPHERE_QUERY();
			// <-- Mirv: Rewritten
		}

		CFFGrenadeBase::PostExplode();
	}
#endif

#ifndef GAME_DLL
void CFFGrenadeConcussion::DoEffectIdle()
{
	DevMsg("[concussion] idle\n");
	if (m_hGlowSprite)
	{
		m_hGlowSprite->SetBrightness(random->RandomInt(32, 48));
		m_hGlowSprite->SetScale(random->RandomFloat(2.5, 3.5));
	}
}
#endif

//----------------------------------------------------------------------------
// Purpose: Precache the model ready for spawn
//----------------------------------------------------------------------------
void CFFGrenadeConcussion::Precache()
{
	DevMsg("[Grenade Debug] CFFGrenadeConcussion::Precache\n");
	PrecacheModel(CONCUSSIONGRENADE_MODEL);
	PrecacheModel(CONCUSSIONGRENADE_GLOW_SPRITE);
	PrecacheScriptSound(CONCUSSION_SOUND);
	BaseClass::Precache();
}

//=============================================================================
// CFFGrenadeConcussionGlow implementation
//=============================================================================

#ifdef GAME_DLL

CFFGrenadeConcussionGlow *CFFGrenadeConcussionGlow::Create(const Vector &origin, CBaseEntity *pOwner)
{
	CFFGrenadeConcussionGlow *pConcGlow = (CFFGrenadeConcussionGlow *) CBaseEntity::Create("env_ffconcussionglow", origin, QAngle(0, 0, 0));

	if (pConcGlow == NULL)
		return NULL;

	pConcGlow->SetRenderMode(kRenderWorldGlow/*(RenderMode_t) 9*/);

	pConcGlow->SetMoveType(MOVETYPE_NONE);
	pConcGlow->AddSolidFlags(FSOLID_NOT_SOLID);
	pConcGlow->AddEffects(EF_NOSHADOW);
	UTIL_SetSize(pConcGlow, vec3_origin, vec3_origin);

	pConcGlow->SetOwnerEntity(pOwner);

	pConcGlow->AddEFlags(EFL_FORCE_CHECK_TRANSMIT);

	pConcGlow->SpriteInit(CONCUSSIONGRENADE_GLOW_SPRITE, origin);
	pConcGlow->SetName(AllocPooledString("TEST"));
	pConcGlow->SetTransparency(kRenderWorldGlow, 255, 255, 255, 255, kRenderFxNoDissipation);
	pConcGlow->SetScale(0.25f);
	pConcGlow->SetOwnerEntity(pOwner);
	pConcGlow->SetSimulatedEveryTick(true);

	return pConcGlow;
}

#else

int CFFGrenadeConcussionGlow::DrawModel(int flags)
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

	CFFGrenadeConcussion *conc = dynamic_cast<CFFGrenadeConcussion *> (GetOwnerEntity());

	if (!conc)
		return 0;

	// Because we're using a NOZ sprite, need to traceline to ensure this is really
	// visible. And we're using a NOZ sprite so it doesnt clip on stuff, okay!
	trace_t tr;
	UTIL_TraceLine(CBasePlayer::GetLocalPlayer()->EyePosition(), GetAbsOrigin(), MASK_SHOT, conc, COLLISION_GROUP_NONE, &tr);

	if (tr.fraction < 1.0f && tr.m_pEnt != conc)
		return 0;

	Vector vecForward, vecRight, vecUp, vecDir;
	conc->GetVectors(&vecForward, &vecRight, &vecUp);
	vecDir = conc->GetAbsOrigin() - CBasePlayer::GetLocalPlayer()->EyePosition();
	VectorNormalize(vecDir);

	float alpha = vecUp.Dot(vecDir);

	if (alpha < 0)
		alpha *= -1;
	
	alpha = 1.0f - alpha;
	alpha *= alpha;

	alpha = conc_glow_a.GetFloat() * (54.0f + 200.0f * alpha);

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
		/*m_clrRender->r */ conc_glow_r.GetInt(), 
		/*m_clrRender->g */ conc_glow_g.GetInt(), 
		/*m_clrRender->b */ conc_glow_b.GetInt(), 
		conc_glow_size.GetFloat());			// sprite scale

	return drawn;
}

//-----------------------------------------------------------------------------
// Purpose: Setup our sprite reference
//-----------------------------------------------------------------------------
void CFFGrenadeConcussionGlow::OnDataChanged(DataUpdateType_t updateType)
{
	if (updateType == DATA_UPDATE_CREATED)
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}
#endif
