/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_grenade_concussion.cpp
/// @author Shawn Smith (L0ki)
/// @date Jan. 29, 2005
/// @brief concussion grenade class
///
/// Implementation of the CFFGrenadeConcussion class. This is the secondary grenade type for scout and medic.
/// 
/// Revisions
/// ---------
/// Jan. 29, 2005	L0ki: Initial Creation
/// Apr. 23, 2005	L0ki: removed header file, moved everything to a single cpp file

#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"
#include "beam_flags.h"
#include "Sprite.h"
#ifdef GAME_DLL
	#include "ff_player.h"
#endif

ConVar conc_push_mag("ffdev_conc_mag", "85.0",0, "Magnitude of the concussion push effect");
ConVar conc_push_base("ffdev_conc_base", "20.0", 0, "Base push of the conc");
//ConVar conc_radius("ffdev_conc_radius","280.0f",0,"Radius of grenade explosions");

#define CONCUSSIONGRENADE_MODEL "models/grenades/conc/conc.mdl"
#define CONCUSSIONGRENADE_GLOW_SPRITE "sprites/glow04.vmt"
#define CONCUSSION_SOUND "ConcussionGrenade.Explode"
#define CONCUSSION_EFFECT "ConcussionExplosion"

#ifdef CLIENT_DLL
	#define CFFGrenadeConcussion C_FFGrenadeConcussion
#endif

class CFFGrenadeConcussion : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeConcussion,CFFGrenadeBase)
	DECLARE_DATADESC()
	CNetworkVector(m_vInitialVelocity);

	virtual void Precache();
	virtual float GetGrenadeDamage() { return 0.0f; }
	virtual float GetGrenadeRadius() { return 280.0f; }

#ifdef CLIENT_DLL
	CFFGrenadeConcussion() {}
	CFFGrenadeConcussion( const CFFGrenadeConcussion& ) {}
	virtual void DoEffectIdle( void );
#else
	virtual void Spawn();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);
#endif

	CHandle<CSprite> m_hGlowSprite;

};

LINK_ENTITY_TO_CLASS( concussiongrenade, CFFGrenadeConcussion );
PRECACHE_WEAPON_REGISTER( concussiongrenade );

BEGIN_DATADESC( CFFGrenadeConcussion )
	DEFINE_FIELD( m_hGlowSprite, FIELD_EHANDLE ),
END_DATADESC()


#ifdef GAME_DLL
	void CFFGrenadeConcussion::Spawn( void )
	{
		SetModel( CONCUSSIONGRENADE_MODEL );

		// add the sprite
		m_hGlowSprite = CSprite::SpriteCreate( CONCUSSIONGRENADE_GLOW_SPRITE, GetAbsOrigin(), false );
		m_hGlowSprite->SetAttachment( this, LookupAttachment( "glowsprite" ) );
		m_hGlowSprite->SetTransparency( kRenderTransAdd, 255, 255, 255, 128, kRenderFxNone );
		m_hGlowSprite->SetBrightness( 255, 0.2f );
		m_hGlowSprite->SetScale( 1.0f, 0.2f );

		BaseClass::Spawn();
	}

	void CFFGrenadeConcussion::Explode(trace_t *pTrace, int bitsDamageType)
	{
		CFFGrenadeBase::PreExplode(pTrace, CONCUSSION_SOUND, CONCUSSION_EFFECT);

		// --> Mirv: Rewritten
		Vector vecDisplacement, vecForce;

		BEGIN_ENTITY_SPHERE_QUERY(GetAbsOrigin(), GetGrenadeRadius())
			if (pPlayer)
			{
				vecDisplacement = pPlayer->GetAbsOrigin() - GetAbsOrigin();

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
				CSingleUserRecipientFilter user( (CBasePlayer *)pPlayer );
				user.MakeReliable();

				UserMessageBegin(user, "StatusIconUpdate");
					WRITE_BYTE(FF_ICON_CONCUSSION);
					WRITE_FLOAT( pPlayer->GetClassSlot() == CLASS_MEDIC ? 7.5f : 15.0f );
				MessageEnd();

				VectorNormalize(vecDisplacement);
				
				QAngle angDirection;
				VectorAngles(vecDisplacement, angDirection);

				// only concuss if teamplay rules says the player could be damaged
				if( g_pGameRules->FPlayerCanTakeDamage( pPlayer, GetOwnerEntity() ) )
				{
					pPlayer->Concuss( ( pPlayer->GetClassSlot() == 5 ? 7.5f : 15.0f ), ( pPlayer == GetOwnerEntity() ? NULL : &angDirection ) );
				}
			}
		END_ENTITY_SPHERE_QUERY();
		// <-- Mirv: Rewritten

		CFFGrenadeBase::PostExplode();
	}
#endif

#ifndef GAME_DLL
void CFFGrenadeConcussion::DoEffectIdle( void )
{
	DevMsg("[concussion] idle\n");
	if (m_hGlowSprite)
	{
//02567                         m_hGlowSprites[i]->SetBrightness( random->RandomInt( 32, 48 ) );
//02568                         m_hGlowSprites[i]->SetScale( random->RandomFloat( 0.15, 0.2 ) * flScaleFactor );
		m_hGlowSprite->SetBrightness( random->RandomInt( 32, 48 ) );
		m_hGlowSprite->SetScale( random->RandomFloat( 2.5, 3.5) );
	}
}
#endif

//----------------------------------------------------------------------------
// Purpose: Precache the model ready for spawn
//----------------------------------------------------------------------------
void CFFGrenadeConcussion::Precache()
{
	DevMsg("[Grenade Debug] CFFGrenadeConcussion::Precache\n");
	PrecacheModel( CONCUSSIONGRENADE_MODEL );
	PrecacheModel( CONCUSSIONGRENADE_GLOW_SPRITE );
	PrecacheScriptSound( CONCUSSION_SOUND );
	BaseClass::Precache();
}
