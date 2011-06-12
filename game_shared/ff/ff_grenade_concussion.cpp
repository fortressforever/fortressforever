/********************************************************************
	created:	2006/08/14
	created:	14:8:2006   11:12
	filename: 	f:\ff-svn\code\trunk\game_shared\ff\ff_grenade_concussion.cpp
	file path:	f:\ff-svn\code\trunk\game_shared\ff
	file base:	ff_grenade_concussion
	file ext:	cpp
	author:		Various
	
	purpose:	
*********************************************************************/

#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"
#include "beam_flags.h"
#include "Sprite.h"
#include "model_types.h"
#include "IEffects.h"

#ifdef GAME_DLL
	#include "ff_player.h"
	#include "ff_entity_system.h"
	#include "te_effect_dispatch.h"
#endif

#ifdef CLIENT_DLL
#define CONC_GLOW_R 255
#define CONC_GLOW_G 255
#define CONC_GLOW_B 200
#define CONC_GLOW_A 0.8f
#define CONC_GLOW_SIZE 1.0f

/* AfterShock: cvars to #defines for the win!
	ConVar conc_glow_r("ffdev_conc_glow_r", "255", FCVAR_CHEAT, "Conc glow red(0-255) ");
	ConVar conc_glow_g("ffdev_conc_glow_g", "255", FCVAR_CHEAT, "Conc glow green(0-255) ");
	ConVar conc_glow_b("ffdev_conc_glow_b", "200", FCVAR_CHEAT, "Conc glow blue(0-255) ");
	ConVar conc_glow_a("ffdev_conc_glow_a", "0.8", FCVAR_CHEAT, "Conc glow alpha(0-1) ");
	ConVar conc_glow_size("ffdev_conc_glow_size", "1.0", FCVAR_CHEAT, "Conc glow size(0.0-10.0");
	*/
#endif

// #0001629: Request: Dev variables for HH conc strength |-- Defrag

//ConVar ffdev_mancannon_conc_speed( "ffdev_mancannon_conc_speed", "1700.0", FCVAR_FF_FFDEV_REPLICATED, "Max conc speed a player can attain after just using a jump pad." );
#define MAX_JUMPPAD_TO_CONC_SPEED 1700.0f // ffdev_mancannon_conc_speed.GetFloat()
//static ConVar ffdev_conc_lateral_power( "ffdev_conc_lateral_power", "2.74", FCVAR_FF_FFDEV, "Lateral movement boost value for hand-held concs", true, 0.0f, true, 2.74f );
#define FFDEV_CONC_LATERAL_POWER 2.74f //ffdev_conc_lateral_power.GetFloat() //2.74f
//static ConVar ffdev_conc_vertical_power( "ffdev_conc_vertical_power", "4.10", FCVAR_FF_FFDEV, "Vertical movement boost value for hand-held concs", true, 0.0f, true, 4.10f );
#define FFDEV_CONC_VERTICAL_POWER 4.10f // ffdev_conc_vertical_power.GetFloat() //4.10f
//static ConVar ffdev_conc_newbconc_uppush( "ffdev_conc_newbconc_uppush", "90", FCVAR_FF_FFDEV, "Vertical movement boost value for newb-didnt-jump hh concs" );
#define FFDEV_CONC_NEWBCONC_UPPUSH 90
//

//ConVar conc_radius("ffdev_conc_radius", "280.0f", FCVAR_FF_FFDEV_REPLICATED, "Radius of grenade explosions");
ConVar conc_ragdoll_push("conc_ragdoll_push","600", FCVAR_FF_FFDEV_REPLICATED,"How much to push ragdolls");

#define CONCUSSIONGRENADE_MODEL			"models/grenades/conc/conc.mdl"
#define CONCUSSIONGRENADE_GLOW_SPRITE	"sprites/glow04_noz.vmt"
#define CONCUSSION_SOUND				"ConcussionGrenade.Explode"
#define CONCUSSION_EFFECT				"FF_ConcussionEffect" // "ConcussionExplosion"
#define CONCUSSION_EFFECT_HANDHELD		"FF_ConcussionEffectHandheld" // "ConcussionExplosion"
#define CONCBITS_EFFECT					"FF_ConcBitsEffect"
#define FLASH_EFFECT					"FF_FlashEffect"
#define RING_EFFECT						"FF_RingEffect"

#ifdef CLIENT_DLL
	int g_iConcRingTexture = -1;
#endif

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
	virtual Class_T Classify( void ) { return CLASS_GREN_CONC; }

	virtual color32 GetColour() { color32 col = { 255, 255, 210, GREN_ALPHA_DEFAULT }; return col; }

	virtual void Explode(trace_t *pTrace, int bitsDamageType);

#ifdef CLIENT_DLL
	CFFGrenadeConcussion() {}
	CFFGrenadeConcussion(const CFFGrenadeConcussion&) {}
	virtual void DoEffectIdle();
#else
	virtual void Spawn();
	virtual float GetGrenadeFriction()		{ return gren_fric_conc.GetFloat(); }
	virtual float GetGrenadeElasticity()	{ return gren_elas_conc.GetFloat(); }
#endif

	CHandle<CFFGrenadeConcussionGlow> m_hGlowSprite;
};

BEGIN_DATADESC(CFFGrenadeConcussion)
END_DATADESC()

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeConcussion, DT_FFGrenadeConcussion)

BEGIN_NETWORK_TABLE(CFFGrenadeConcussion, DT_FFGrenadeConcussion)
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(ff_grenade_concussion, CFFGrenadeConcussion);
PRECACHE_WEAPON_REGISTER(ff_grenade_concussion);

#ifdef GAME_DLL

	//-----------------------------------------------------------------------------
	// Purpose: Set model. Add sprites (TODO: Remove sprites)
	//-----------------------------------------------------------------------------
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

#endif

	//-----------------------------------------------------------------------------
	// Purpose: Do a proper conc explosion
	//-----------------------------------------------------------------------------
	void CFFGrenadeConcussion::Explode(trace_t *pTrace, int bitsDamageType)
	{
		EmitSoundShared(CONCUSSION_SOUND);

#ifdef GAME_DLL
		CEffectData data;
		data.m_vOrigin = GetAbsOrigin();
		data.m_flScale = 1.0f;
		data.m_flRadius = GetGrenadeRadius();
		if ( m_fIsHandheld )
		{
			DispatchEffect(CONCUSSION_EFFECT_HANDHELD, data);
		}
		else
		{
			DispatchEffect(CONCUSSION_EFFECT, data);
		}

		g_pEffects->EnergySplash(GetAbsOrigin(), Vector(0, 0, 1.0f), true);
#endif

		//Need this so they make ragdolls go flying.
		Vector vecAbsOrigin = GetAbsOrigin();

		if( pTrace->fraction != 1.0 ) 
		{
			Vector vecNormal = pTrace->plane.normal;
			surfacedata_t *pdata = physprops->GetSurfaceData( pTrace->surface.surfaceProps );	
			CPASFilter filter( vecAbsOrigin );
			te->Explosion( filter, -1.0, // don't apply cl_interp delay
				&vecAbsOrigin, 
				0,
				0, 
				0, 
				TE_EXPLFLAG_NOSOUND | TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOFIREBALL | TE_EXPLFLAG_NOFIREBALLSMOKE, 
				GetGrenadeRadius(), 
				conc_ragdoll_push.GetInt(), 
				&vecNormal, 
				( char )pdata->game.material );
		}
		else
		{
			CPASFilter filter( vecAbsOrigin );
			te->Explosion( filter, -1.0, // don't apply cl_interp delay
				&vecAbsOrigin, 
				0,
				0, 
				0, 
				TE_EXPLFLAG_NOSOUND | TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOFIREBALL | TE_EXPLFLAG_NOFIREBALLSMOKE, 
				GetGrenadeRadius(), 
				conc_ragdoll_push.GetInt());
		}

		// nb. Do not move this 32 units above the ground!
		// That behaviour does not occur with conc grenades

		CBaseEntity *pEntity = NULL;

		for( CEntitySphereQuery sphere( GetAbsOrigin(), GetGrenadeRadius() ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			if (!pEntity || !pEntity->IsPlayer())
				continue;

			CFFPlayer *pPlayer = ToFFPlayer(pEntity);

			if( !pPlayer->IsAlive() || pPlayer->IsObserver() )
				continue;

			// caes: make hh concs not push other players
			// Shok: OR concuss at all!
			if ( pEntity != GetThrower() && m_fIsHandheld )
				continue;

			// Some useful things to know
			Vector vecDisplacement = pPlayer->GetLegacyAbsOrigin() - GetAbsOrigin();
			float flDistance = vecDisplacement.Length();
			Vector vecDir = vecDisplacement / flDistance;

#ifdef GAME_DLL
			// Concuss the player first
			if (g_pGameRules->FCanTakeDamage(pPlayer, GetOwnerEntity()))
			{
				QAngle angDirection;
				VectorAngles(vecDir, angDirection);

				float flDuration = 10.0f;
				float flIconDuration = flDuration;
				if( pPlayer->LuaRunEffect( LUA_EF_CONC, GetOwnerEntity(), &flDuration, &flIconDuration ) )
				{
					if( pPlayer == GetOwnerEntity() )
						pPlayer->Concuss( flDuration, flIconDuration, NULL, flDistance);
					else
						pPlayer->Concuss( flDuration, flIconDuration, &angDirection, flDistance);
				}					
			}
#endif

			// People who are building shouldn't be pushed around by anything
			if (pPlayer->IsStaticBuilding())
				continue;

			// TFC considers a displacement < 16units to be a hh
			// However in FF sometimes the distance can be more with a hh
			// But we don't want to lose the trait of a hh-like jump with a drop conc
			// So an extra flag here helps out.
			// Remember that m_fIsHandheld only affects the grenade owner
			Vector vecResult;
			if ((pEntity == GetThrower() && m_fIsHandheld) || (flDistance < 16.0f))
			{
				// These values are close (~within 0.01) of TFC
				// #0001629: Request: Dev variables for HH conc strength.  Default = 2.74f and 4.10f respectively ---> Defrag
				float fLateral = FFDEV_CONC_LATERAL_POWER;
				float fVertical = FFDEV_CONC_VERTICAL_POWER;

				Vector vecVelocity = pPlayer->GetAbsVelocity();
				Vector vecLatVelocity = vecVelocity * Vector(1.0f, 1.0f, 0.0f);
				float flHorizontalSpeed = vecLatVelocity.Length();
				// if you're on the ground (forgot to jump or ur a noob), give a little push but not much
				if ((pPlayer->GetFlags() & FL_ONGROUND) && (flHorizontalSpeed < 550))
				{
					// noob conc
					//pPlayer->SetAbsVelocity(Vector(vecVelocity.x * fLateral * 0.95, vecVelocity.y  * fLateral* 0.95, (vecVelocity.z + FFDEV_CONC_NEWBCONC_UPPUSH)* fVertical));
					vecResult = Vector(vecVelocity.x * fLateral * 0.95, vecVelocity.y  * fLateral* 0.95, (vecVelocity.z + FFDEV_CONC_NEWBCONC_UPPUSH)* fVertical);
					DevMsg("[HH Conc] on the ground & slow (%f) so newb conc\n", flHorizontalSpeed);
				}
				else
				{
					//pPlayer->SetAbsVelocity(Vector(vecVelocity.x * fLateral, vecVelocity.y * fLateral, vecVelocity.z * fVertical));
					vecResult = Vector(vecVelocity.x * fLateral, vecVelocity.y * fLateral, vecVelocity.z * fVertical);
					DevMsg("[HH Conc] Not on the ground or too fast (%f)\n", flHorizontalSpeed);
				}

				//DevMsg("[HH Conc] flDistance = %f\n",flDistance);
			}
			else
			{
				float verticalDistance = vecDisplacement.z;
					
				vecDisplacement.z = 0;
				float horizontalDistance = vecDisplacement.Length();

				// Normalise the lateral direction of this
				vecDisplacement /= horizontalDistance;

				// This is the equation I've calculated for drop concs
				// Is accurate to about ~0.001 in TFC so pretty sure this is the
				// damn thing they use.
				vecDisplacement *= (horizontalDistance * (8.4f - 0.015f * flDistance));
				vecDisplacement.z = (verticalDistance * (12.6f - 0.0225f * flDistance));

				//pPlayer->SetAbsVelocity(vecDisplacement);
				vecResult = vecDisplacement;
			}

			// Jiggles: players can easily get insane speeds by using a jump pad and then concing
			// AfterShock: This takes into account vertical speed too, limiting horizontal speed if you're going upwards aswell
			//             is this a bad thing? 
#ifdef GAME_DLL
			if ( pPlayer->m_flMancannonTime && gpGlobals->curtime < pPlayer->m_flMancannonTime + 5.2f )
			{
				if ( vecResult.Length() > MAX_JUMPPAD_TO_CONC_SPEED )
				{
					vecResult.NormalizeInPlace();
					vecResult *= MAX_JUMPPAD_TO_CONC_SPEED;
				}
			}
#endif
			pPlayer->SetAbsVelocity(vecResult);

			//AfterShock: If we ever want to play effects on whoever got hit, we can do it like this
			//g_pEffects->EnergySplash( pPlayer->GetLegacyAbsOrigin() , Vector(0, 0, 1.0f), true);
			//g_pEffects->Sparks(pPlayer->GetLegacyAbsOrigin());

		}

		// Now get rid of this
#ifdef GAME_DLL
		UTIL_Remove(this);
#endif
	}

#ifndef GAME_DLL
void CFFGrenadeConcussion::DoEffectIdle()
{
	//DevMsg("[concussion] idle\n");
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
	PrecacheModel(CONCUSSIONGRENADE_MODEL);
	PrecacheModel(CONCUSSIONGRENADE_GLOW_SPRITE);
	PrecacheModel("models/grenades/conc/conceffect.mdl");
	
#ifdef CLIENT_DLL
	g_iConcRingTexture = PrecacheModel("sprites/lgtning.vmt");
#endif

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

	alpha = CONC_GLOW_A * (54.0f + 200.0f * alpha);

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
		/*m_clrRender->r */ CONC_GLOW_R, 
		/*m_clrRender->g */ CONC_GLOW_G, 
		/*m_clrRender->b */ CONC_GLOW_B, 
		CONC_GLOW_SIZE + random->RandomFloat(-0.04f, 0.04f));			// sprite scale

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
