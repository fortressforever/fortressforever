/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_fx_napalm_emitter.cpp
/// @author Shawn Smith (L0ki)
/// @date Apr. 30, 2005
/// @brief napalm burst emitter
///
/// Implementation of the napalm burst emitter particle system
/// 
/// Revisions
/// ---------
/// Apr. 30, 2005	L0ki: Initial Creation

#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "particles_simple.h"
#include "ff_fx_napalm_emitter.h"
#include "ff_grenade_base.h"

//========================================================================
// Static material handles
//========================================================================
PMaterialHandle CNapalmEmitter::m_hMaterial			= INVALID_MATERIAL_HANDLE;
PMaterialHandle CNapalmEmitter::m_hHeatwaveMaterial	= INVALID_MATERIAL_HANDLE;
PMaterialHandle CNapalmEmitter::m_hFlameMaterial	= INVALID_MATERIAL_HANDLE;

//========================================================================
// material strings
//========================================================================
#define NAPALM_PARTICLE_MATERIAL	"particle/fire"
#define HEATWAVE_MATERIAL			"sprites/heatwave"
#define NAPALM_FLAME_MATERIAL		"sprites/napalm_flame2"

//========================================================================
// texture coordinates for the flame
//							  minX  maxX   minY  maxY
//========================================================================
float flame_tex_coords[][4] =
{
	{ 0.0f,		0.125f,	0.0f, 0.5f},	//frame 0
	{ 0.125f,	0.25f,	0.0f, 0.5f},	//frame 1
	{ 0.25f,	0.375f,	0.0f, 0.5f},	//frame 2
	{ 0.375f,	0.5f,	0.0f, 0.5f},	//frame 3
	{ 0.5f,		0.625f,	0.0f, 0.5f},	//frame 4
	{ 0.625f,	0.75f,	0.0f, 0.5f},	//frame 5
	{ 0.75f,	0.875f,	0.0f, 0.5f},	//frame 6
	{ 0.875f,	1.0f,	0.0f, 0.5f},	//frame 7
	{ 0.0f,		0.125f,	0.5f, 1.0f},	//frame 8
	{ 0.125f,	0.25f,	0.5f, 1.0f},	//frame 9
	{ 0.25f,	0.375f,	0.5f, 1.0f},	//frame 10
	{ 0.375f,	0.5f,	0.5f, 1.0f},	//frame 11
	{ 0.5f,		0.625f,	0.5f, 1.0f},	//frame 12
	{ 0.625f,	0.75f,	0.5f, 1.0f}		//frame 13
};
#define NUM_FLAME_FRAMES	15
ConVar nap_burst_flame_framerate("ffdev_nap_burst_flame_framerate","7",0,"Framerate of the fire \"sprites\".");

//========================================================================
// Client effect precache table
//========================================================================
CLIENTEFFECT_REGISTER_BEGIN( PrecacheNapalmBurst )
	CLIENTEFFECT_MATERIAL( NAPALM_PARTICLE_MATERIAL )
	CLIENTEFFECT_MATERIAL( HEATWAVE_MATERIAL )
	CLIENTEFFECT_MATERIAL( NAPALM_FLAME_MATERIAL )
CLIENTEFFECT_REGISTER_END()

//========================================================================
// Development ConVars
//========================================================================
ConVar nap_burst_scale("ffdev_nap_burst_scale","10",0,"Napalm burst scale");
ConVar nap_burst_dietime("ffdev_nap_burst_dietime","3.0",0,"Napalm burst particle dietime");
ConVar nap_burst_flame_scale("ffdev_nap_burst_flame_scale","16.0",0,"Scale of the flame sprites");
ConVar nap_burst_flame_time("ffdev_nap_burst_flame_time","5.0",0,"BUrn time for flames");

//========================================================================
// CNapalmEmitter constructor
//========================================================================
CNapalmEmitter::CNapalmEmitter( const char *pDebugName ) : CParticleEffect( pDebugName )
{
	m_pDebugName = pDebugName;

	m_flNearClipMin	= 16.0f;
	m_flNearClipMax	= 64.0f;
	m_vGravity = Vector(0,0,0);
	m_flGravityMagnitude = 0;
}

//========================================================================
// CNapalmEmitter destructor
//========================================================================
CNapalmEmitter::~CNapalmEmitter()
{
}

//========================================================================
// CNapalmEmitter::Create
// ----------------------
// Purpose: Creates a new instance of a CNapalmEmitter object
//========================================================================
CSmartPtr<CNapalmEmitter> CNapalmEmitter::Create( const char *pDebugName )
{
	CNapalmEmitter *pRet = new CNapalmEmitter( pDebugName );
	pRet->SetDynamicallyAllocated( true );
	if(m_hMaterial == INVALID_MATERIAL_HANDLE)
		m_hMaterial = pRet->GetPMaterial(NAPALM_PARTICLE_MATERIAL);
	if(m_hHeatwaveMaterial == INVALID_MATERIAL_HANDLE)
		m_hHeatwaveMaterial = pRet->GetPMaterial(HEATWAVE_MATERIAL);
	if(m_hFlameMaterial == INVALID_MATERIAL_HANDLE)
		m_hFlameMaterial = pRet->GetPMaterial(NAPALM_FLAME_MATERIAL);
	return pRet;
}

//========================================================================
// AddNapalmParticle
// -----------------
// Purpose: Adds a new NapalmParticle to the system
//========================================================================
NapalmParticle*	CNapalmEmitter::AddNapalmParticle( const Vector &vOrigin )
{
	NapalmParticle *pRet = (NapalmParticle*)AddParticle( sizeof( NapalmParticle ), m_hMaterial, vOrigin );
	if ( pRet )
	{
		pRet->m_iType = eNapalmParticle;
		pRet->m_vVelocity.Init();
		pRet->m_flDieTime = nap_burst_dietime.GetFloat();
		pRet->m_flLifetime = 0;
		pRet->m_uchColor[0] = 255;
		pRet->m_uchColor[1] = 160;
		pRet->m_uchColor[2] = 0;
		pRet->m_bStartFire = true;
	}

	return pRet;
}

//========================================================================
// SetGravity
// ----------
// Purpose: Sets the gravity vector and magnitude for this system
//========================================================================
void CNapalmEmitter::SetGravity(const Vector &vGravity, float flGravityMagnitude)
{
	m_vGravity = vGravity;
	VectorNormalize(m_vGravity);
	m_flGravityMagnitude = flGravityMagnitude;
}

//========================================================================
// ApplyGravity
// --------------
// Purpose: Applies gravity to the specified particle's velocity
//========================================================================
void CNapalmEmitter::ApplyGravity( NapalmParticle *pParticle, float timeDelta )
{
	if(pParticle->m_iType == eNapalmFlame)
		return;
	// apply gravity to this particle
	Vector vGravity = (m_vGravity * (m_flGravityMagnitude * timeDelta));
	pParticle->m_vVelocity += vGravity;
}

//========================================================================
// SimulateParticles
// ----------
// Purpose: Handles adjusting particle properties as time progresses as
//			well as removing dead particles
//========================================================================
void CNapalmEmitter::SimulateParticles( CParticleSimulateIterator *pIterator )
{
	float timeDelta = pIterator->GetTimeDelta();

	NapalmParticle *pParticle = (NapalmParticle*)pIterator->GetFirst();
	while ( pParticle )
	{
		pParticle->m_flLifetime += timeDelta;
		if ( pParticle->m_flLifetime >= pParticle->m_flDieTime )
			pIterator->RemoveParticle( pParticle );
		else
		{
			if(pParticle->m_iType == eNapalmParticle)
			{
				ApplyGravity( pParticle, timeDelta );
				//if this particle has moved outside of the grenade explosion readius, make it drop straight down
				// yes, its lame, but it works
				Vector displacement = m_vSortOrigin - (pParticle->m_Pos + pParticle->m_vVelocity * timeDelta);
				if(displacement.Length() > 180.0f)
				{
					pParticle->m_vVelocity.x = 0;
					pParticle->m_vVelocity.y = 0;
				}
				pParticle->m_Pos += pParticle->m_vVelocity * timeDelta;

				trace_t tr;
				UTIL_TraceLine(
					pParticle->m_Pos,
					pParticle->m_Pos + (pParticle->m_vVelocity * 0.1),
					MASK_SOLID,
					NULL,
					COLLISION_GROUP_NONE,
					&tr
					);
				if(tr.fraction != 1.0)
				{
					//pParticle->m_Pos = tr.endpos - (pParticle->m_vVelocity * timeDelta);
					pParticle->m_vVelocity.x = 0;
					pParticle->m_vVelocity.y = 0;
					//pParticle->m_vVelocity.z = 0;
					if(pParticle->m_bStartFire)
					{
						pIterator->RemoveParticle(pParticle);
						StartFire(tr.endpos);
					}
				}
			}
		}

		pParticle = (NapalmParticle*)pIterator->GetNext();
	}
}

// Render a quad on the screen where you pass in color and size.
inline void RenderParticle_ColorSizeFrame(
									 ParticleDraw* pDraw,									
									 const Vector &pos,
									 const Vector &color,
									 const float alpha,
									 const float size,
									 const int frame
									 )
{
	// Don't render totally transparent particles.
	if( alpha < 0.001f )
		return;

	CMeshBuilder *pBuilder = pDraw->GetMeshBuilder();
	if( !pBuilder )
		return;

	unsigned char ubColor[4];
	ubColor[0] = (unsigned char)RoundFloatToInt( color.x * 254.9f );
	ubColor[1] = (unsigned char)RoundFloatToInt( color.y * 254.9f );
	ubColor[2] = (unsigned char)RoundFloatToInt( color.z * 254.9f );
	ubColor[3] = (unsigned char)RoundFloatToInt( alpha * 254.9f );

	// Add the 4 corner vertices.
	pBuilder->Position3f( pos.x-size, pos.y-size, pos.z );
	pBuilder->Color4ubv( ubColor );
	pBuilder->TexCoord2f( 0, flame_tex_coords[frame][0], flame_tex_coords[frame][3] );
	pBuilder->AdvanceVertex();

	pBuilder->Position3f( pos.x-size, pos.y+size, pos.z );
	pBuilder->Color4ubv( ubColor );
	pBuilder->TexCoord2f( 0, flame_tex_coords[frame][0], flame_tex_coords[frame][2] );
	pBuilder->AdvanceVertex();

	pBuilder->Position3f( pos.x+size, pos.y+size, pos.z );
	pBuilder->Color4ubv( ubColor );
	pBuilder->TexCoord2f( 0, flame_tex_coords[frame][1], flame_tex_coords[frame][2] );
	pBuilder->AdvanceVertex();

	pBuilder->Position3f( pos.x+size, pos.y-size, pos.z );
	pBuilder->Color4ubv( ubColor );
	pBuilder->TexCoord2f( 0, flame_tex_coords[frame][1], flame_tex_coords[frame][3] );
	pBuilder->AdvanceVertex();
}

//========================================================================
// RenderParticles
// ----------
// Purpose: Renders all the particles in the system
//========================================================================
void CNapalmEmitter::RenderParticles( CParticleRenderIterator *pIterator )
{
	const NapalmParticle *pParticle = (const NapalmParticle *)pIterator->GetFirst();
	while ( pParticle )
	{
		//Render
		Vector	tPos;

		TransformParticle( g_ParticleMgr.GetModelView(), pParticle->m_Pos, tPos );
		float sortKey = (int) tPos.z;

		//Render it
		if (pParticle->m_iType == eNapalmParticle)
		{
			Vector vColor = Vector(pParticle->m_uchColor[0] / 255.0f,
				pParticle->m_uchColor[1] / 255.0f,
				pParticle->m_uchColor[2] / 255.0f);

			RenderParticle_ColorSize(
				pIterator->GetParticleDraw(),
				tPos,
				vColor,
				192,
				nap_burst_scale.GetFloat()
				);
		}
		else if(pParticle->m_iType == eNapalmFlame)
		{
			Vector vColor = Vector(pParticle->m_uchColor[0] / 255.0f,
				pParticle->m_uchColor[1] / 255.0f,
				pParticle->m_uchColor[2] / 255.0f);

			int frame = Float2Int(pParticle->m_flLifetime * nap_burst_flame_framerate.GetFloat()) % NUM_FLAME_FRAMES;
			RenderParticle_ColorSizeFrame(
				pIterator->GetParticleDraw(),
				tPos,
				vColor,
				1.0f,
				nap_burst_flame_scale.GetFloat(),
				frame
				);
		}
		else if(pParticle->m_iType == eHeatwave)
		{
			Vector clr = Vector(1.0f,1.0f,1.0f);
			float sinLifetime = sin(pParticle->m_flLifetime * 3.14159f / pParticle->m_flDieTime);
			float scale = nap_burst_flame_scale.GetFloat();
			RenderParticle_ColorSizePerturbNormal(
				pIterator->GetParticleDraw(),
				tPos,
				clr,
				sinLifetime,// * (nap_burst_flame_alpha.GetFloat() / 255.0f),
				(pParticle->m_bReverseSize ? FLerp(scale*2.0f,scale,pParticle->m_flLifetime) : FLerp(scale,scale*2.0f,pParticle->m_flLifetime))
				);
		}

		pParticle = (const NapalmParticle *)pIterator->GetNext( sortKey );
	}
}

void CNapalmEmitter::StartFire(const Vector &pos)
{
	NapalmParticle *pFireParticle = (NapalmParticle*)AddParticle( sizeof( NapalmParticle ), m_hFlameMaterial,pos );
	if ( pFireParticle )
	{
		pFireParticle->m_iType = eNapalmFlame;
		pFireParticle->m_Pos = pos;
		pFireParticle->m_vVelocity.Init();
		pFireParticle->m_flLifetime = 0;
		pFireParticle->m_flDieTime = nap_burst_flame_time.GetFloat();;
		pFireParticle->m_uchColor[0] = 255;
		pFireParticle->m_uchColor[1] = 255;
		pFireParticle->m_uchColor[2] = 255;
		pFireParticle->m_bStartFire = false;
	}
	NapalmParticle *pHeatParticle = (NapalmParticle*)AddParticle( sizeof( NapalmParticle ), m_hHeatwaveMaterial, pos );
	if(pHeatParticle)
	{
		pHeatParticle->m_iType = eHeatwave;
		pHeatParticle->m_Pos = pos;
		pHeatParticle->m_vVelocity.Init();
		pHeatParticle->m_flLifetime = 0;
		pHeatParticle->m_flDieTime = nap_burst_flame_time.GetFloat();;
		pHeatParticle->m_uchColor[0] = 255;
		pHeatParticle->m_uchColor[1] = 255;
		pHeatParticle->m_uchColor[2] = 255;
		pHeatParticle->m_bStartFire = false;
	}
}