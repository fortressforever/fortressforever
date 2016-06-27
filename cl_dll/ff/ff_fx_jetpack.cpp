#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "particles_simple.h"
#include "c_te_effect_dispatch.h"
#include "cliententitylist.h"
#include "iefx.h"

#include "ff_fx_jetpack.h"

#define JETPACK_FLAME_EFFECT_MATERIAL "effects/flame"

//ConVar jetpack_particles	( "ffdev_particles_per_jetpack", "100", FCVAR_CHEAT, "The number of particles in each jetpack." );

// re-use the flamethrower dlight cvar for jetpack
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

//========================================================================
// Client effect precache table
//========================================================================
CLIENTEFFECT_REGISTER_BEGIN( PrecacheJetpackEmitter )
CLIENTEFFECT_MATERIAL( JETPACK_FLAME_EFFECT_MATERIAL )
CLIENTEFFECT_REGISTER_END()

//========================================================================
// Static material handles
//========================================================================
PMaterialHandle CJetpackEmitter::m_hFlameMaterial = INVALID_MATERIAL_HANDLE;

//========================================================================
// CJetpackEmitter constructor
//========================================================================
CJetpackEmitter::CJetpackEmitter( const char *pDebugName ) : CSimpleEmitter( pDebugName )
{
	m_pDebugName = pDebugName;
	m_flNextParticle = 0.0f;
}

//========================================================================
// CJetpackEmitter destructor
//========================================================================
CJetpackEmitter::~CJetpackEmitter( void )
{
}

//========================================================================
// CJetpackEmitter::Create
// ----------------------
// Purpose: Creates a new instance of a CJetpackEmitter object
//========================================================================
CSmartPtr< CJetpackEmitter > CJetpackEmitter::Create( const char *pDebugName, CBaseEntity *pOwner )
{
	CJetpackEmitter *pRet = new CJetpackEmitter( pDebugName );

	if( !pRet )
		return NULL;

	pRet->SetDynamicallyAllocated();
	pRet->SetOwnerEntity(pOwner);

	if( m_hFlameMaterial == INVALID_MATERIAL_HANDLE )
		m_hFlameMaterial = pRet->GetPMaterial( JETPACK_FLAME_EFFECT_MATERIAL );

	return pRet;
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

//========================================================================
// CJetpackEmitter::RenderParticles
// ----------
// Purpose: Renders all the particles in the system
//========================================================================
void CJetpackEmitter::RenderParticles( CParticleRenderIterator *pIterator )
{
	const JetpackParticle *pParticle = ( const JetpackParticle * )pIterator->GetFirst();
	while( pParticle )
	{
		// Render.
		Vector tPos;
		TransformParticle(ParticleMgr()->GetModelView(), pParticle->m_Pos, tPos);
		float sortKey = tPos.z;

		// Normal alpha
		float alpha = 0.3f; // ffdev_flame_alpha.GetFloat(); // 0.3f; // 0.95f; // 180.0f;

		// Fade out everything in its last moments
		if (/*pParticle->m_Type != SMOKE &&*/ pParticle->m_Dietime - pParticle->m_Lifetime < /*ffdev_flame_fadeout_time.GetFloat()*/ 0.2f ) 
		{
			alpha *= ((pParticle->m_Dietime - pParticle->m_Lifetime) / /*ffdev_flame_fadeout_time.GetFloat()*/ 0.2f );

			// also fade the dynamic light
			if ( pParticle->m_pDLight && gpGlobals->curtime < pParticle->m_fDLightDieTime )
				pParticle->m_pDLight->color.exponent = 5.0f/*ffdev_flame_dlight_color_e.GetFloat()*/ * ((pParticle->m_Dietime - pParticle->m_Lifetime) / 0.2f/*ffdev_flame_fadeout_time.GetFloat()*/);
		}

		// Fade in smoke after a delay
		if (pParticle->m_Type == SMOKE && pParticle->m_Lifetime < /*ffdev_flame_fadeout_time.GetFloat()*/ 0.2f ) 
		{
			alpha *= pParticle->m_Lifetime / 0.2f /*ffdev_flame_fadeout_time.GetFloat()*/;
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

		pParticle = ( const JetpackParticle * )pIterator->GetNext( sortKey );
	}
}

//========================================================================
// CJetpackEmitter::SimulateParticles
// ----------
// Purpose: Simulates all the particles in the system
//========================================================================
void CJetpackEmitter::SimulateParticles( CParticleSimulateIterator *pIterator )
{
	float timeDelta = pIterator->GetTimeDelta();

	JetpackParticle *pParticle = ( JetpackParticle * )pIterator->GetFirst();
	while( pParticle )
	{
		pParticle->m_Lifetime += timeDelta;

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

			// move the dynamic light along with the particle
			if ( pParticle->m_pDLight )
			{
				if ( gpGlobals->curtime >= pParticle->m_fDLightDieTime )
					pParticle->m_pDLight = NULL;
				else
					pParticle->m_pDLight->origin = pParticle->m_Pos;
			}
		}

		pParticle = ( JetpackParticle * )pIterator->GetNext();
	}
}

//========================================================================
// CRingEmitter::AddJetpackParticle
// ----------
// Purpose: Add a new particle to the system
//========================================================================
JetpackParticle *CJetpackEmitter::AddJetpackParticle( const Vector& vecStart, const Vector& vecForward )
{
	// Passing 128 as a third argument because these flame particles are bigger than MAX_PARTICLE_SIZE (96)
	JetpackParticle *pParticle = ( JetpackParticle * )AddParticle( sizeof( JetpackParticle ), m_hFlameMaterial, vecStart, 128 );

	if (!pParticle)
		return NULL;
	
	m_SpreadSpeed	= 100; // ffdev_flame_spreadspeed.GetInt(); // 100;
	m_Speed			= 800; // ffdev_flame_speed.GetInt(); // 1200;
	m_StartSize		= 2; // ffdev_flame_startsize.GetInt(); // 3;
	m_EndSize		= 192; // ffdev_flame_endsize.GetInt(); // 192;
	float flDLightScale = cl_ffdlight_flamethrower.GetFloat();

	pParticle->m_Type			= FLAME_JET;
	pParticle->m_Appearance		= TEX_FLAME_NORMAL;

	pParticle->m_Pos			= vecStart;
	pParticle->m_Origin			= pParticle->m_Pos;

	Vector	vecRight, vecUp;
	VectorVectors(vecForward, vecRight, vecUp);
	pParticle->m_Velocity		= FRand(-m_SpreadSpeed, m_SpreadSpeed) * vecRight +
								  FRand(-m_SpreadSpeed, m_SpreadSpeed) * vecUp +
								  m_Speed * vecForward;

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

	if (gpGlobals->curtime - 0.2f/*ffdev_flame_dlight_rate.GetFloat()*/ > m_flLastParticleDLightTime )
	{
		pParticle->m_fDLightDieTime = gpGlobals->curtime + pParticle->m_Dietime;
		pParticle->m_fDLightStartRadius = random->RandomFloat(80.0f/*ffdev_flame_dlight_startradius_min.GetFloat()*/, 100.0f/*ffdev_flame_dlight_startradius_max.GetFloat()*/) * flDLightScale;
		pParticle->m_fDLightEndRadius = random->RandomFloat(130.0f/*ffdev_flame_dlight_endradius_min.GetFloat()*/, 150.0f/*ffdev_flame_dlight_endradius_max.GetFloat()*/) * flDLightScale;

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
			pParticle->m_pDLight->color.r = 255;//ffdev_flame_dlight_color_r.GetInt();
			pParticle->m_pDLight->color.g = 144;//ffdev_flame_dlight_color_g.GetInt();
			pParticle->m_pDLight->color.b = 64;//ffdev_flame_dlight_color_b.GetInt();
			pParticle->m_pDLight->color.exponent = 5;//ffdev_flame_dlight_color_e.GetInt();
			pParticle->m_pDLight->style = 6;//ffdev_flame_dlight_style.GetInt();

			m_flLastParticleDLightTime = gpGlobals->curtime;
		}
	}
	else
	{
		pParticle->m_fDLightDieTime = gpGlobals->curtime;
		pParticle->m_pDLight = NULL;
	}

	return pParticle;
}

void CJetpackEmitter::ApplyDrag( Vector *F, Vector vecVelocity, float flScale, float flTargetVel )
{
	if( vecVelocity.IsLengthLessThan( flTargetVel ) )
		return;

	Vector vecDir = -vecVelocity;
	vecVelocity.NormalizeInPlace();

	float flMag = vecVelocity.Length() * flScale;
	*F += ( vecDir * flMag );
}

void CJetpackEmitter::Update( float flTimeDelta )
{
	if (!GetOwnerEntity())
		return;

	if( gpGlobals->curtime > m_flDieTime )
		return;

	if( gpGlobals->curtime < m_flNextParticle )
		return;

	m_flNextParticle = gpGlobals->curtime + 0.05f;

	// Default positions and angles
	Vector vecStart = GetOwnerEntity()->GetAbsOrigin();
	Vector vecForward;
	AngleVectors( GetOwnerEntity()->EyeAngles(), &vecForward );
	vecForward.z = 0.0f;
	VectorNormalizeFast( vecForward );
	vecForward *= -0.25f;
	vecForward.z = -1.0f;
	VectorNormalizeFast( vecForward );

	if( UTIL_PointContents(vecStart) & (CONTENTS_WATER|CONTENTS_SLIME) )
	{
		return;
	}

	Vector vecFacingDir; 
	AngleVectors(GetOwnerEntity()->EyeAngles(), &vecFacingDir);
	Vector	vecRight, vecUp;
	VectorVectors(vecForward, vecRight, vecUp);
	vecRight.z = 0;
	
	float originRandOffset = 32.0f;
	for (int i=0; i<10; i++)
	{
		AddJetpackParticle( vecStart - vecFacingDir * 8.0f + vecRight * 8.0f + FRand(-originRandOffset, originRandOffset) * Vector(0.0f, 0.0f, 1.0f), vecForward );
	}
	for (int i=0; i<10; i++)
	{
		AddJetpackParticle( vecStart - vecFacingDir * 8.0f - vecRight * 8.0f + FRand(-originRandOffset, originRandOffset) * Vector(0.0f, 0.0f, 1.0f), vecForward );
	}
}
