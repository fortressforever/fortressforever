/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file c_ff_env_flamejet.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date 20 March 2005
/// @brief Implements the client side of a flame jet particle system!
///
/// REVISIONS
/// ---------
/// Mar 20, 2005 Mirv: First logged

#include "cbase.h"
#include "particle_prototype.h"
#include "particle_util.h"
#include "baseparticleentity.h"
#include "ClientEffectPrecacheSystem.h"
#include "c_ff_player.h"
#include "c_ff_env_flamejet.h"
#include "iinput.h"
#include "iefx.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
// Debug ConVars
//=============================================================================

//static ConVar ffdev_flame_spreadspeed(	"ffdev_flame_spreadspeed", 	"100", 	0, 	"How fast the flames spread outwards");
//static ConVar ffdev_flame_speed(			"ffdev_flame_speed", 			"1200", 	0, 	"How fast the flames go forwards");
//static ConVar ffdev_flame_startsize(		"ffdev_flame_startsize", 		"2", 	0, 	"How big the flame starts(0-255) ");
//static ConVar ffdev_flame_endsize(		"ffdev_flame_endsize", 		"192", 	0, 	"How big the flame finishes(0-255) ");
//static ConVar ffdev_flame_rate(			"ffdev_flame_rate", 			"128", 	0, 	"Number of flame particles per second");
//static ConVar ffdev_flame_alpha(			"ffdev_flame_alpha", 			"0.3", 	0, 	"Alpha value of the flame(0 - 1.0) ");

//static ConVar ffdev_flame_startblue(		"ffdev_flame_startblue", "0.05",		0, "How long the flame stays blue for");

//static ConVar ffdev_flame_fadeout_time(	"ffdev_flame_fadeout_time", 	"0.2", 		0, 	"How long before end of life will flames fade out(in seconds) ");

//static ConVar ffdev_flame_dietime_min(		"ffdev_flame_dietime_min", 		/*"0.3"*/ "0.18", 	0, 	"Lifespan of the flames in seconds");
//static ConVar ffdev_flame_dietime_max(		"ffdev_flame_dietime_max", 		/*"0.4"*/ "0.24", 	0, 	"Lifespan of the flames in seconds");

// -> Defrag
//static ConVar ffdev_flame_eye_angle_bias( "ffdev_flame_eye_angle_bias", "0.5", 0, "Lerp factor for blending between eye & muzzle angles. 0.0 = Use muzzle angles as velocity. 1.0 = Use eye angles.", true, 0.0f, true, 1.0f );

ConVar ffdev_flame_dlight_color_r( "ffdev_flame_dlight_color_r", "255" );
ConVar ffdev_flame_dlight_color_g( "ffdev_flame_dlight_color_g", "144" );
ConVar ffdev_flame_dlight_color_b( "ffdev_flame_dlight_color_b", "64" );
ConVar ffdev_flame_dlight_color_e( "ffdev_flame_dlight_color_e", "5" );
ConVar ffdev_flame_dlight_startradius_min( "ffdev_flame_dlight_startradius_min", "80" );
ConVar ffdev_flame_dlight_startradius_max( "ffdev_flame_dlight_startradius_max", "100" );
ConVar ffdev_flame_dlight_endradius_min( "ffdev_flame_dlight_endradius_min", "130" );
ConVar ffdev_flame_dlight_endradius_max( "ffdev_flame_dlight_endradius_max", "150" );
ConVar ffdev_flame_dlight_rate( "ffdev_flame_dlight_rate", "0.2", 0, "Attach a light to one of the flame particles every X seconds." );
ConVar ffdev_flame_dlight_style( "ffdev_flame_dlight_style", "6", 0, "0 through 12 (0 = normal, 1 = flicker, 5 = gentle pulse, 6 = other flicker)");

//=============================================================================
// Globals
//=============================================================================

// dlight scale
extern ConVar cl_ffdlight_flamethrower;

// Behaviour
enum
{
	SMOKE, 
	FLAME_JET, 
	FLAME_LICK
};

// Appearance
enum
{
	TEX_FLAME_NORMAL, 
	TEX_SMOKE, 
	TEX_FLAME_SMALL1, 
	TEX_FLAME_SMALL2, 
	TEX_FLAME_SPLASH
};

// Texture Coordinates		  minX		maxX		minY	maxY
float tex_coords[5][4] = {	{ 0, 		0.25f, 		0, 			0.5f }, 	   // normal flame
							{ 0.25f, 	0.5f, 		0, 			0.5f }, // smoke
							{ 0, 		0.25f, 		0.5f, 		1.0f }, // smaller flame
							{ 0.25f, 	0.5f, 		0.5f, 		1.0f }, // smaller flame
							{ 0.5f, 		0.75f, 		0, 			1.0f } };  // flame splash

//=============================================================================
// C_FFFlameJet tables
//=============================================================================

// Expose to the particle app
EXPOSE_PROTOTYPE_EFFECT(FlameJet, C_FFFlameJet);

IMPLEMENT_CLIENTCLASS_DT(C_FFFlameJet, DT_FFFlameJet, CFFFlameJet) 
	RecvPropInt(RECVINFO(m_bEmit), 0), 
END_RECV_TABLE() 

LINK_ENTITY_TO_CLASS(env_flamejet, C_FFFlameJet);

CLIENTEFFECT_REGISTER_BEGIN(PrecacheFlameJet) 
CLIENTEFFECT_MATERIAL("effects/flame") 
CLIENTEFFECT_REGISTER_END() 

//=============================================================================
// C_FFFlameJet implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
C_FFFlameJet::C_FFFlameJet() 
{
	m_pParticleMgr	= NULL;
	m_hMaterialFlame = m_hMaterialSmoke = INVALID_MATERIAL_HANDLE;
	
	m_SpreadSpeed	= 100; // ffdev_flame_spreadspeed.GetInt(); // 100;
	m_Speed			= 1200; // ffdev_flame_speed.GetInt(); // 1200;
	m_StartSize		= 2; // ffdev_flame_startsize.GetInt(); // 3;
	m_EndSize		= 192; // ffdev_flame_endsize.GetInt(); // 192;
	m_Rate			= 128; // ffdev_flame_rate.GetInt(); // 128; 

	m_bEmit			= true;

	m_fLastParticleDLightTime = 0;

	m_pDLight = NULL;

	m_ParticleEffect.SetAlwaysSimulate(false); // Don't simulate outside the PVS or frustum.
}

//----------------------------------------------------------------------------
// Purpose: Destructor
//----------------------------------------------------------------------------
C_FFFlameJet::~C_FFFlameJet() 
{
	Cleanup();
}

//----------------------------------------------------------------------------
// Purpose: Cleanup
//----------------------------------------------------------------------------
void C_FFFlameJet::Cleanup( void )
{
	if( m_pParticleMgr ) 
		m_pParticleMgr->RemoveEffect( &m_ParticleEffect );
}

//----------------------------------------------------------------------------
// Purpose: Called after a data update has occured
// Input  : bnewentity - 
//----------------------------------------------------------------------------
void C_FFFlameJet::OnDataChanged(DataUpdateType_t updateType) 
{
	BaseClass::OnDataChanged(updateType);

	if (updateType == DATA_UPDATE_CREATED) 
	{
		Start(ParticleMgr(), NULL);
	}

	m_ParticleEffect.SetParticleCullRadius(max(m_StartSize, m_EndSize));

	UpdateVisibility();
}

//----------------------------------------------------------------------------
// Purpose: 
//----------------------------------------------------------------------------
void C_FFFlameJet::SetDormant( bool bDormant )
{
	/*
	if( bDormant )
	{
		Cleanup();
	}
	else
	{
		Start( ParticleMgr(), NULL );
	}
	*/

	BaseClass::SetDormant( bDormant );
}

//-----------------------------------------------------------------------------
// Purpose: Starts the effect
// Input  : *pParticleMgr - 
//			*pArgs - 
//-----------------------------------------------------------------------------
void C_FFFlameJet::Start(CParticleMgr *pParticleMgr, IPrototypeArgAccess *pArgs) 
{
	pParticleMgr->AddEffect(&m_ParticleEffect, this);

	m_hMaterialFlame	= m_ParticleEffect.FindOrAddMaterial("effects/flame");
	
	m_ParticleSpawn.Init(m_Rate);
	m_pParticleMgr = pParticleMgr;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : **ppTable - 
//			**ppObj - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_FFFlameJet::GetPropEditInfo(RecvTable **ppTable, void **ppObj) 
{
	*ppTable = &REFERENCE_RECV_TABLE(DT_FFFlameJet);
	*ppObj = this;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : fTimeDelta - 
//-----------------------------------------------------------------------------
void C_FFFlameJet::Update(float fTimeDelta) 
{
	if (!m_pParticleMgr) 
	{
		assert(false);
		return;
	}

	CFFPlayer *pOwner = dynamic_cast<CFFPlayer *> (GetOwnerEntity());

	// A bunch of conditions that may stop the flamethrower
	if (!pOwner || !pOwner->GetActiveFFWeapon() || pOwner->GetActiveFFWeapon()->GetWeaponID() != FF_WEAPON_FLAMETHROWER)
	{
		// Don't disable it for now
		//m_bEmit = false;
		return;
	}

	// Default positions and angles
	Vector vecStart = pOwner->Weapon_ShootPosition();
	QAngle angAngles = pOwner->EyeAngles();
	C_BaseAnimating *pWeapon = NULL;

	if (pOwner->IsLocalPlayer() && !input->CAM_IsThirdPerson()) 
		pWeapon = pOwner->GetViewModel(0);
	else
		pWeapon = pOwner->GetActiveWeapon();

	if (pWeapon)
	{
		int iAttachment = pWeapon->LookupAttachment("1");

		// #0001537: Flamethrower does not indicate vertical aiming properly.	-> Defrag
		// Changed this so that the angles returned by the GetAttachment function are no longer used to set the flame velocity.
		// Instead, we now lerp between the eye & weapon muzzle angles to get the velocity direction vector
		QAngle angWeapon;
		pWeapon->GetAttachment(iAttachment, vecStart, angWeapon);
		
		// factor by which we bias towards eye angles
		const float fEyeAngleBias = 0.5f/*ffdev_flame_eye_angle_bias.GetFloat()*/;		
		
		// lerp between pitch of eye and weapon angles.  The bigger the eye angle bias, the more the flamethrower jet's
		// velocity heads out along the eye angles.  Set it to 1.0f to make it use the eye angles.  Set it to 0 to use model angles.
		// I reckon something along the lines of 0.75 to 0.8 would be good.
        float fBlendedPitch = angAngles.x * fEyeAngleBias + angWeapon.x * ( 1.0f - fEyeAngleBias );
		angAngles.x = fBlendedPitch;
	}

	// Removing the forward thing as when you're up against a wall you see no flames now
	Vector vecForward;
	AngleVectors( angAngles, &vecForward );
	VectorNormalizeFast( vecForward );

	// Check that this isn't going through a wall
	trace_t tr;
	UTIL_TraceLine(pOwner->EyePosition(), vecStart /*+ ( vecForward * 4.0f )*/, MASK_SOLID_BRUSHONLY | MASK_WATER, pOwner, COLLISION_GROUP_NONE, &tr);

	// Yes, going through a wall
	if (tr.fraction < 1.0f)
	{
		// Drag backwards
		vecStart = tr.endpos - vecForward * 2.0f;
		//return;
	}

	if( ( tr.contents & CONTENTS_WATER ) || ( tr.contents & CONTENTS_SLIME ) )
	{
		return;
	}

	Vector forward, right, up;
	AngleVectors(angAngles, &forward, &right, &up);

	// Set the bbox so the particle manager knows when to draw this flamejet.
	Vector vEndPoint = vecStart + forward * m_Speed;
	Vector vMin, vMax;
	VectorMin(vecStart, vEndPoint, vMin);
	VectorMax(vecStart, vEndPoint, vMax);
	m_ParticleEffect.SetBBox(vMin, vMax);

	// Temp

	m_SpreadSpeed	= 100; // ffdev_flame_spreadspeed.GetInt(); // 100;
	m_Speed			= 1200; // ffdev_flame_speed.GetInt(); // 1200;
	m_StartSize		= 2; // ffdev_flame_startsize.GetInt(); // 3;
	m_EndSize		= 192; // ffdev_flame_endsize.GetInt(); // 192;

	// dlight scale
	float flDLightScale = cl_ffdlight_flamethrower.GetFloat();

	if (m_bEmit && m_ParticleEffect.WasDrawnPrevFrame()) 
	{
		// update the existing muzzle light
		if (m_pDLight)
		{
			m_pDLight->origin = vecStart;
			m_pDLight->die = gpGlobals->curtime + (0.18 /*ffdev_flame_dietime_min.GetFloat()*/ * 0.5);

			// Make it flicker
			m_pDLight->radius = clamp(random->RandomFloat(m_pDLight->radius * 0.9f, m_pDLight->radius * 1.1f), 80.0f * flDLightScale, 100.0f * flDLightScale);
		}
		else
		{
			// create the muzzle light
			if (flDLightScale > 0.0f)
				m_pDLight = effects->CL_AllocDlight( 0 );
			else
				m_pDLight = NULL;

			if ( m_pDLight )
			{
				m_pDLight->origin = vecStart;
				m_pDLight->radius = random->RandomFloat(/*80.0f*/ffdev_flame_dlight_startradius_min.GetFloat(), /*100.0f*/ffdev_flame_dlight_startradius_max.GetFloat()) * flDLightScale;
				m_pDLight->die = gpGlobals->curtime + (0.18/*ffdev_flame_dietime_min.GetFloat()*/ * 0.5);
				m_pDLight->color.r = ffdev_flame_dlight_color_r.GetInt();//255;
				m_pDLight->color.g = ffdev_flame_dlight_color_g.GetInt();//144;
				m_pDLight->color.b = ffdev_flame_dlight_color_b.GetInt();//64;
				m_pDLight->color.exponent = ffdev_flame_dlight_color_e.GetInt();//5;
				m_pDLight->style = ffdev_flame_dlight_style.GetInt();//6;
			}
		}

		float tempDelta = fTimeDelta;
		while (m_ParticleSpawn.NextEvent(tempDelta)) 
		{
			// Make a new particle.
			// Passing 128 as a third argument because these flame particles are bigger than MAX_PARTICLE_SIZE (96)
			if (C_FFFlameJetParticle *pParticle = (C_FFFlameJetParticle *) m_ParticleEffect.AddParticle(sizeof(C_FFFlameJetParticle), m_hMaterialFlame, 128)) 
			{
				pParticle->m_Type			= FLAME_JET;
				pParticle->m_Appearance		= TEX_FLAME_NORMAL;

				pParticle->m_Pos			= vecStart;
				pParticle->m_Origin			= pParticle->m_Pos;

				pParticle->m_Velocity		= FRand(-m_SpreadSpeed, m_SpreadSpeed) * right +
											  FRand(-m_SpreadSpeed, m_SpreadSpeed) * up +
											  m_Speed * forward;

				// Move along to correct position
				pParticle->m_Origin	+= pParticle->m_Velocity * tempDelta;
				//pParticle->m_Origin = pOwner->Weapon_ShootPosition();
				
				pParticle->m_Lifetime		= 0;
				pParticle->m_Dietime		= random->RandomFloat(0.18/*ffdev_flame_dietime_min.GetFloat()*/, 0.24/*ffdev_flame_dietime_max.GetFloat()*/ );

				pParticle->m_uchStartSize	= m_StartSize;
				pParticle->m_uchEndSize		= m_EndSize;

				pParticle->m_flRoll			= random->RandomFloat(0, 360);
				pParticle->m_flRollDelta	= random->RandomFloat(-8.0f, 8.0f);

				pParticle->m_flRedness		= random->RandomFloat(0, 1.0f);

				trace_t tr;

				// How far can this particle travel
				UTIL_TraceLine(pParticle->m_Pos, pParticle->m_Pos + (pParticle->m_Velocity * pParticle->m_Dietime), MASK_SOLID | MASK_WATER, GetOwnerEntity(), COLLISION_GROUP_NONE, &tr);

				pParticle->m_Collisiontime = tr.fraction * pParticle->m_Dietime;
				pParticle->m_HitSurfaceNormal = tr.plane.normal;

				if (gpGlobals->curtime - /*0.2f*/ffdev_flame_dlight_rate.GetFloat() > m_fLastParticleDLightTime )
				{
					pParticle->m_fDLightDieTime = gpGlobals->curtime + pParticle->m_Dietime;
					pParticle->m_fDLightStartRadius = random->RandomFloat(ffdev_flame_dlight_startradius_min.GetFloat()/*80.0f*/, ffdev_flame_dlight_startradius_max.GetFloat()/*100.0f*/) * flDLightScale;
					pParticle->m_fDLightEndRadius = random->RandomFloat(ffdev_flame_dlight_endradius_min.GetFloat()/*130.0f*/, ffdev_flame_dlight_endradius_max.GetFloat()/*150.0f*/) * flDLightScale;

					// -------------------------------------
					// Dynamic light stuff
					// -------------------------------------
					if (flDLightScale > 0.0f)
						pParticle->m_pDLight = effects->CL_AllocDlight( 0 );
					else
						pParticle->m_pDLight = NULL;

					// don't want to start trying to access something that's not there
					if (pParticle->m_pDLight)
					{
						pParticle->m_pDLight->origin = pParticle->m_Origin;
						pParticle->m_pDLight->radius = pParticle->m_fDLightStartRadius;
						pParticle->m_pDLight->die = pParticle->m_fDLightDieTime;
						pParticle->m_pDLight->color.r = ffdev_flame_dlight_color_r.GetInt();//255;
						pParticle->m_pDLight->color.g = ffdev_flame_dlight_color_g.GetInt();//144;
						pParticle->m_pDLight->color.b = ffdev_flame_dlight_color_b.GetInt();//64;
						pParticle->m_pDLight->color.exponent = ffdev_flame_dlight_color_e.GetInt();//5;
						pParticle->m_pDLight->style = ffdev_flame_dlight_style.GetInt();//6;

						m_fLastParticleDLightTime = gpGlobals->curtime;
					}
				}
				else
				{
					pParticle->m_fDLightDieTime = gpGlobals->curtime;
					pParticle->m_pDLight = NULL;
				}
			}
		}
	}
	else
	{
		// time to kill the muzzle light
		if (m_pDLight)
		{
			m_pDLight->decay = m_pDLight->radius / (m_pDLight->die - gpGlobals->curtime);
			m_pDLight = NULL;
		}

		// reset this
		m_fLastParticleDLightTime = 0;
	}
}

//----------------------------------------------------------------------------
// Purpose: Allows us to use one material(for speed & sorting issues) 
//----------------------------------------------------------------------------
inline void RenderParticle_ColorSizeAngle(
	ParticleDraw * pDraw, 									
	const Vector &pos, 
	const Vector &color, 
	const float alpha, 
	const float size, 
	const float angle, 
	const int appearance) 
{
	// Don't render totally transparent particles.
	if (alpha < 0.001f) 
		return;

	CMeshBuilder *pBuilder = pDraw->GetMeshBuilder();
	if (!pBuilder) 
		return;

	unsigned char ubColor[4];
	ubColor[0] = (unsigned char) RoundFloatToInt(color.x * 254.9f);
	ubColor[1] = (unsigned char) RoundFloatToInt(color.y * 254.9f);
	ubColor[2] = (unsigned char) RoundFloatToInt(color.z * 254.9f);
	ubColor[3] = (unsigned char) RoundFloatToInt(alpha * 254.9f);

	float ca = (float) cos(angle);
	float sa = (float) sin(angle);

	pBuilder->Position3f(pos.x + (-ca + sa) * size, pos.y + (-sa - ca) * size, pos.z);
	pBuilder->Color4ubv(ubColor);
	pBuilder->TexCoord2f(0, 0, 1);
 	pBuilder->AdvanceVertex();

	pBuilder->Position3f(pos.x + (-ca - sa) * size, pos.y + (-sa + ca) * size, pos.z);
	pBuilder->Color4ubv(ubColor);
	pBuilder->TexCoord2f(0, 0, 0);
 	pBuilder->AdvanceVertex();

	pBuilder->Position3f(pos.x + (ca - sa) * size, pos.y + (sa + ca) * size, pos.z);
	pBuilder->Color4ubv(ubColor);
	pBuilder->TexCoord2f(0, 1, 0);
 	pBuilder->AdvanceVertex();

	pBuilder->Position3f(pos.x + (ca + sa) * size, pos.y + (sa - ca) * size, pos.z);
	pBuilder->Color4ubv(ubColor);
	pBuilder->TexCoord2f(0, 1, 1);
 	pBuilder->AdvanceVertex();

}

//----------------------------------------------------------------------------
// Purpose: Render all the particles
//----------------------------------------------------------------------------
void C_FFFlameJet::RenderParticles(CParticleRenderIterator *pIterator) 
{
	const C_FFFlameJetParticle *pParticle = (const C_FFFlameJetParticle *) pIterator->GetFirst();
	while (pParticle) 
	{
		// Render.
		Vector tPos;
		TransformParticle(m_pParticleMgr->GetModelView(), pParticle->m_Pos, tPos);
		float sortKey = tPos.z;

		// Normal alpha
		float alpha = 0.3f; // ffdev_flame_alpha.GetFloat(); // 0.3f; // 0.95f; // 180.0f;

		// Fade out everything in its last moments
		if (/*pParticle->m_Type != SMOKE &&*/ pParticle->m_Dietime - pParticle->m_Lifetime < /*ffdev_flame_fadeout_time.GetFloat()*/ 0.2f ) 
		{
			alpha *= ((pParticle->m_Dietime - pParticle->m_Lifetime) / /*ffdev_flame_fadeout_time.GetFloat()*/ 0.2f );

			// also fade the dynamic light
			if ( pParticle->m_pDLight && gpGlobals->curtime < pParticle->m_fDLightDieTime )
				pParticle->m_pDLight->color.exponent = /*5.0f*/ffdev_flame_dlight_color_e.GetFloat() * ((pParticle->m_Dietime - pParticle->m_Lifetime) / 0.2f/*ffdev_flame_fadeout_time.GetFloat()*/);
		}

		// Fade in smoke after a delay
		if (pParticle->m_Type == SMOKE && pParticle->m_Lifetime < /*ffdev_flame_fadeout_time.GetFloat()*/ 0.2f ) 
		{
			alpha *= pParticle->m_Lifetime / 0.2f /*ffdev_flame_fadeout_time.GetFloat()*/;
		}

		// Randomise the brightness
		float col = random->RandomFloat(0.3f, 0.5f);

		// Smoke should generally be darker
		if (pParticle->m_Type == SMOKE) 
		{
			col = random->RandomFloat(0.1f, 0.2f);
		}

		float r = 1.0f, g = 1.0f, b = 1.0f;

		if (pParticle->m_Lifetime < 0.05/*ffdev_flame_startblue.GetFloat()*/)
		{
			float reduction = (0.05/*ffdev_flame_startblue.GetFloat()*/ - pParticle->m_Lifetime) / 0.05/*ffdev_flame_startblue.GetFloat()*/;
			r -= reduction;
			g -= reduction;
		}
		else
		{
			b -= pParticle->m_flRedness;
			g -= pParticle->m_flRedness;
		}

		RenderParticle_ColorSizeAngle(
			pIterator->GetParticleDraw(), 
			tPos, 
			Vector(r, g, b), 
			alpha, 
			FLerp(pParticle->m_uchStartSize, pParticle->m_uchEndSize, pParticle->m_Lifetime), 
			pParticle->m_flRoll, 
			pParticle->m_Appearance);

		// update the dynamic light's radius
		if ( pParticle->m_pDLight && gpGlobals->curtime < pParticle->m_fDLightDieTime )
			pParticle->m_pDLight->radius = FLerp(pParticle->m_fDLightStartRadius, pParticle->m_fDLightEndRadius, pParticle->m_Lifetime / pParticle->m_Dietime);

		pParticle = (const C_FFFlameJetParticle *) pIterator->GetNext(sortKey);
	}
}

//----------------------------------------------------------------------------
// Purpose: Simulate the particles, this function is quite messy atm
//----------------------------------------------------------------------------
void C_FFFlameJet::SimulateParticles(CParticleSimulateIterator *pIterator) 
{
	C_FFFlameJetParticle *pParticle = (C_FFFlameJetParticle *) pIterator->GetFirst();

	while (pParticle) 
	{
		pParticle->m_Lifetime += pIterator->GetTimeDelta();

		// Should this particle die?
		if (pParticle->m_Lifetime > pParticle->m_Dietime) 
		{
			pIterator->RemoveParticle(pParticle);
		}
		// Do stuff to this particle
		else 
		{
			pParticle->m_flRoll += pParticle->m_flRollDelta * pIterator->GetTimeDelta();
			pParticle->m_Pos = pParticle->m_Pos + pParticle->m_Velocity * pIterator->GetTimeDelta();

			if (pParticle->m_Lifetime > pParticle->m_Collisiontime && pParticle->m_Type != FLAME_LICK) 
			{
				// Pull out of the surface
				pParticle->m_Pos = pParticle->m_Origin + (pParticle->m_Velocity * (pParticle->m_Collisiontime - 0.01f));
				pParticle->m_Type = FLAME_LICK;

				// Some crossproducts (really!) 
				Vector cp1 = CrossProduct(pParticle->m_Velocity, pParticle->m_HitSurfaceNormal);
				Vector cp2 = CrossProduct(pParticle->m_HitSurfaceNormal, cp1);

				// Change the velocity to be parallel to the surface it hit
				float normal_len = pParticle->m_HitSurfaceNormal.Length();

				// Save from the dreaded divide by zero
				if (!normal_len) 
					normal_len += 0.01f;

				pParticle->m_Velocity = cp2 / normal_len;

				// Now slow down the flames a bit
				pParticle->m_Velocity *= 0.8f;

				// Work out next point of collision
				trace_t tr;

				// Work our how far of the route left we can go
				UTIL_TraceLine(pParticle->m_Pos, pParticle->m_Pos + pParticle->m_Velocity * (pParticle->m_Dietime - pParticle->m_Lifetime), MASK_SOLID, GetOwnerEntity(), COLLISION_GROUP_NONE, &tr);

				pParticle->m_Collisiontime += tr.fraction * (pParticle->m_Dietime - pParticle->m_Lifetime);

				// UNDONE:
				// Wait wait, why are flames only allowed to bounce once?
				// - Somebody?

				// Well, we don't want loads of trace's being done for loads 
				// of particles, and we have to trace the same route on the 
				// server too, so limiting it is a pretty good idea really!
				// - Somebody else?

				// Except, it looks really bad when the flames disappear into walls.
				// I think we need some kind of compromise here.
				// - Jon
			}
		}

		// move the dynamic light along with the particle
		if ( pParticle->m_pDLight )
		{
			if ( gpGlobals->curtime >= pParticle->m_fDLightDieTime )
				pParticle->m_pDLight = NULL;
			else
                pParticle->m_pDLight->origin = pParticle->m_Pos;
		}

		pParticle = (C_FFFlameJetParticle *) pIterator->GetNext();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Turn the flame jet on or off
//-----------------------------------------------------------------------------
bool C_FFFlameJet::FlameEmit(bool bEmit)
{
	if ((m_bEmit != 0) != bEmit)
	{
		m_bEmit = bEmit;
		return true;
	}
	return false;
}