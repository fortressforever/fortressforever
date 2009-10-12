/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_fx_conc_emitter.cpp
/// @author Paul Peloski (zero)
/// @date Feb 5, 2006
/// @brief conc effect emitter
///
/// Implementation of the conc effect emitter particle system
/// 
/// Revisions
/// ---------
/// Feb 5, 2006	zero: Initial Creation

#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "particles_simple.h"
#include "ff_fx_conc_emitter.h"
#include "materialsystem/imaterialvar.h"
#include "c_te_effect_dispatch.h"
#include "beam_flags.h"
#include "fx_explosion.h"
#include "tempent.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"

#define CONC_EFFECT_MATERIAL "sprites/concrefract"
extern int g_iConcRingTexture;

ConVar conc_on			 ("ffdev_conc_on", "1", FCVAR_CHEAT, "Turn the conc effect on or off - 1 or 0." );
ConVar conc_scale		 ("ffdev_conc_scale", "512.0", FCVAR_CHEAT, "How big the conc effect gets.");
//ConVar conc_refract		 ("ffdev_conc_refract", "0.2", FCVAR_CHEAT, "Refraction amount for conc effect.");
//ConVar conc_blur		 ("ffdev_conc_blur", "0", FCVAR_CHEAT, "Blur amount for conc effect.");
ConVar conc_speed		 ("ffdev_conc_speed", "0.5", FCVAR_CHEAT, "Duration of the conc effect.");
ConVar conc_ripples		 ("ffdev_conc_ripples", "1", FCVAR_CHEAT, "How many ripples the conc effect has.");
ConVar conc_ripple_period("ffdev_conc_ripple_period", "0.05", FCVAR_CHEAT, "Time between ripples.");

ConVar conc_blur("ffdev_conc_blur", "0.5", FCVAR_CHEAT);
ConVar conc_refract("ffdev_conc_refract", "0.5", FCVAR_CHEAT);
ConVar conc_grow("ffdev_conc_grow", "0.25", FCVAR_CHEAT);
ConVar conc_shrink("ffdev_conc_shrink", "0.35", FCVAR_CHEAT);

//==============================
// conc ring effect vars
//==============================
//ConVar ffdev_conc_effect_num_rings("ffdev_conc_effect_num_rings", "1");
ConVar ffdev_conc_effect_framerate("ffdev_conc_effect_framerate", "1");
ConVar ffdev_conc_effect_width("ffdev_conc_effect_width", "50");
ConVar ffdev_conc_effect_width2("ffdev_conc_effect_width2", "0");
ConVar ffdev_conc_effect_width3("ffdev_conc_effect_width3", "10");
ConVar ffdev_conc_effect_spread("ffdev_conc_effect_spread", "0");
ConVar ffdev_conc_effect_amplitude("ffdev_conc_effect_amplitude", "0");
ConVar ffdev_conc_effect_lifetime("ffdev_conc_effect_lifetime", ".3");
ConVar ffdev_conc_effect_r("ffdev_conc_effect_r", "255");
ConVar ffdev_conc_effect_g("ffdev_conc_effect_g", "255");
ConVar ffdev_conc_effect_b("ffdev_conc_effect_b", "225");
ConVar ffdev_conc_effect_a("ffdev_conc_effect_a", "178");
ConVar ffdev_conc_effect_radius("ffdev_conc_effect_radius", "600");
ConVar ffdev_conc_effect_radius2("ffdev_conc_effect_radius2", "520");

#define CONC_FRAMERATE		ffdev_conc_effect_framerate.GetFloat()
#define CONC_WIDTH			ffdev_conc_effect_width.GetFloat()
#define CONC_WIDTH2			ffdev_conc_effect_width2.GetFloat()
#define CONC_WIDTH3			ffdev_conc_effect_width3.GetFloat()
#define CONC_SPREAD			ffdev_conc_effect_spread.GetFloat()
#define CONC_AMPLITUDE		ffdev_conc_effect_amplitude.GetFloat()
#define CONC_LIFETIME		ffdev_conc_effect_lifetime.GetFloat()
#define CONC_R				ffdev_conc_effect_r.GetFloat()
#define CONC_G				ffdev_conc_effect_g.GetFloat()
#define CONC_B				ffdev_conc_effect_b.GetFloat()
#define CONC_A				ffdev_conc_effect_a.GetFloat()
#define CONC_RADIUS			ffdev_conc_effect_radius.GetFloat()
#define CONC_RADIUS2			ffdev_conc_effect_radius2.GetFloat()


//========================================================================
// Ragdoll stuff
//========================================================================
class CRagdollConcEnumerator : public IPartitionEnumerator
{
	DECLARE_CLASS_GAMEROOT( CRagdollConcEnumerator, IPartitionEnumerator );
public:
	//Forced constructor
	CRagdollConcEnumerator( Vector origin);
	~CRagdollConcEnumerator();

	//Actual work code
	virtual IterationRetval_t EnumElement( IHandleEntity *pHandleEntity );

public:
	//Data members
	CUtlVector<C_BaseEntity*> m_Entities;
	Vector	m_vecOrigin;
};

// Enumator class for ragdolls being affected by explosive forces
CRagdollConcEnumerator::CRagdollConcEnumerator( Vector origin)
{
	m_vecOrigin		= origin;
}

// Actual work code
IterationRetval_t CRagdollConcEnumerator::EnumElement( IHandleEntity *pHandleEntity )
{
	C_BaseEntity *pEnt = ClientEntityList().GetBaseEntityFromHandle( pHandleEntity->GetRefEHandle() );
	
	if ( pEnt == NULL )
		return ITERATION_CONTINUE;

	C_BaseAnimating *pModel = static_cast< C_BaseAnimating * >( pEnt );

	// If the ragdoll was created on this tick, then the forces were already applied on the server
	if ( pModel == NULL || WasRagdollCreatedOnCurrentTick( pEnt ) )
		return ITERATION_CONTINUE;
	
	m_Entities.AddToTail( pEnt );

	return ITERATION_CONTINUE;
}

CRagdollConcEnumerator::~CRagdollConcEnumerator()
{
	for (int i = 0; i < m_Entities.Count(); i++ )
	{
		C_BaseEntity *pEnt = m_Entities[i];
		//C_BaseAnimating *pModel = static_cast< C_BaseAnimating * >( pEnt );

		Vector	position = pEnt->CollisionProp()->GetCollisionOrigin();
	}
}


//========================================================================
// Client effect precache table
//========================================================================
CLIENTEFFECT_REGISTER_BEGIN(PrecacheConcEmitter)
	CLIENTEFFECT_MATERIAL(CONC_EFFECT_MATERIAL)
CLIENTEFFECT_REGISTER_END()

//========================================================================
// Static material handles
//========================================================================
PMaterialHandle CConcEmitter::m_hMaterial = INVALID_MATERIAL_HANDLE;

//========================================================================
// CConcEmitter constructor
//========================================================================
CConcEmitter::CConcEmitter(const char *pDebugName) : CSimpleEmitter(pDebugName)
{
	m_pDebugName = pDebugName;
}

//========================================================================
// CConcEmitter destructor
//========================================================================
CConcEmitter::~CConcEmitter()
{
}

//========================================================================
// CConcEmitter::Create
// ----------------------
// Purpose: Creates a new instance of a CConcEmitter object
//========================================================================
CSmartPtr<CConcEmitter> CConcEmitter::Create(const char *pDebugName)
{
	CConcEmitter *pRet = new CConcEmitter(pDebugName);

	pRet->SetDynamicallyAllocated();

	if (m_hMaterial == INVALID_MATERIAL_HANDLE)
		m_hMaterial = pRet->GetPMaterial(CONC_EFFECT_MATERIAL);

	return pRet;
}

//========================================================================
// CConcEmitter::RenderParticles
// ----------
// Purpose: Renders all the particles in the system
//========================================================================
void CConcEmitter::RenderParticles(CParticleRenderIterator *pIterator)
{
	if( conc_on.GetInt() == 0 )
		return;

	const	ConcParticle *pParticle = (const ConcParticle *) pIterator->GetFirst();
	
	float	flLife, flDeath;

	bool	bFound;

	while (pParticle)
	{
		Vector	tPos;

		TransformParticle(ParticleMgr()->GetModelView(), pParticle->m_Pos, tPos);
		float sortKey = (int) tPos.z;

		flLife = pParticle->m_flLifetime - pParticle->m_flOffset;
		flDeath = pParticle->m_flDieTime - pParticle->m_flOffset;

		if (flLife > 0.0f)
		{

			IMaterial *pMat = pIterator->GetParticleDraw()->m_pSubTexture->m_pMaterial;

		    if (pMat)
			{
				IMaterialVar *pVar = pMat->FindVar("$refractamount", &bFound, true);

				if (pVar)
					pVar->SetFloatValue(-pParticle->m_flRefract + SimpleSplineRemapVal(flLife, 0.0f, flDeath, 0.001f, pParticle->m_flRefract));

				pVar = pMat->FindVar("$bluramount", &bFound, true);

				if (pVar)
					pVar->SetFloatValue(conc_blur.GetFloat());
			}

			float flColor = 1.0f;// RemapVal(flLife, 0.0f, flDeath, 1.0f, 0.0f);

			Vector vColor = Vector(flColor, flColor, flColor);

			// Render it
			RenderParticle_ColorSizeAngle(
				pIterator->GetParticleDraw(), 
				tPos, 
				vColor, 
				flColor, 																	// Alpha
				SimpleSplineRemapVal(flLife, 0.0f, flDeath, 0, conc_scale.GetFloat()), 		// Size
				pParticle->m_flRoll
				);
		}

		pParticle = (const ConcParticle *) pIterator->GetNext(sortKey);
	}

}

//========================================================================
// CConcEmitter::AddConcParticle
// ----------
// Purpose: Add a new particle to the system
//========================================================================
ConcParticle * CConcEmitter::AddConcParticle()
{
	ConcParticle *pRet = (ConcParticle *) AddParticle(sizeof(ConcParticle), m_hMaterial, GetSortOrigin());

	if (pRet)
	{
		pRet->m_Pos = GetSortOrigin();
		pRet->m_vecVelocity.Init();
		pRet->m_flRoll = 0;
		pRet->m_flRollDelta = 0;
		pRet->m_flLifetime = 0;
		pRet->m_flDieTime = 0;
		pRet->m_uchColor[0] = pRet->m_uchColor[1] = pRet->m_uchColor[2] = 0;
		pRet->m_uchStartAlpha = pRet->m_uchEndAlpha = 255;
		pRet->m_uchStartSize = 0;
		pRet->m_iFlags = 0;
		pRet->m_flOffset = 0;
	}

	return pRet;
}

class C_ConcEffect : public C_BaseAnimating
{
	typedef C_BaseAnimating BaseClass;
public:

	static C_ConcEffect	*CreateClientsideEffect(const char *pszModelName, Vector vecOrigin);

	bool	InitializeEffect( const char *pszModelName, Vector vecOrigin );
	void	ClientThink( void );

protected:

	IMaterial	*m_pMaterial;
	float		m_flStart;
};

//-----------------------------------------------------------------------------
// Purpose: Create the conc effect
//-----------------------------------------------------------------------------
C_ConcEffect *C_ConcEffect::CreateClientsideEffect(const char *pszModelName, Vector vecOrigin)
{
	C_ConcEffect *pEffect = new C_ConcEffect;

	if (pEffect == NULL)
		return NULL;

	if (pEffect->InitializeEffect(pszModelName, vecOrigin) == false)
		return NULL;

	pEffect->m_pMaterial = materials->FindMaterial(CONC_EFFECT_MATERIAL, TEXTURE_GROUP_OTHER);
	pEffect->m_flStart = gpGlobals->curtime;

	return pEffect;
}

//-----------------------------------------------------------------------------
// Purpose: Initialise all the conc stuff
//-----------------------------------------------------------------------------
bool C_ConcEffect::InitializeEffect( const char *pszModelName, Vector vecOrigin )
{
	if (InitializeAsClientEntity(pszModelName, RENDER_GROUP_OPAQUE_ENTITY) == false)
	{
		Release();
		return false;
	}

	SetAbsOrigin(vecOrigin);

	SetNextClientThink(CLIENT_THINK_ALWAYS);

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Adjust the material proxies for the conc as time goes on
//-----------------------------------------------------------------------------
void C_ConcEffect::ClientThink( void )
{
	C_BasePlayer *pPlayer = CBasePlayer::GetLocalPlayer();

	// We need to keep the correct part of the shader oriented towards the player
	// The bit we want is on the top, so rotate around x axis by 90
	Vector vecDir = GetAbsOrigin() - pPlayer->EyePosition();

	QAngle angFace;
	VectorAngles(vecDir, angFace);
	angFace.x += 90;

	SetAbsAngles(angFace);

	float flLife = gpGlobals->curtime - m_flStart;
	float flStrength = 0.0f;

	// These are temp until the values are decided
	float flMidTime = conc_grow.GetFloat();
	float flEndTime = conc_grow.GetFloat() + conc_shrink.GetFloat();

	if (flLife <= /*0.4f*/ flMidTime)
	{
		flStrength = SimpleSplineRemapVal(flLife, 0.0f, /*0.4f*/ flMidTime, 0.0f, 1.0f);
	}
	else if(flLife <= /*1.2f*/ flEndTime)
	{
		flStrength = SimpleSplineRemapVal(flLife, /*0.4f*/ flMidTime, /*1.2f*/ flEndTime, 1.0f, 0.0f);
	}
	// Bit of time to settle before releasing
	else if (flLife > /*1.5f*/ flEndTime + 0.2f)
	{
		Release();
		return;
	}

	flStrength = clamp(flStrength, 0.0f, 1.0f);

	bool bFound;
	IMaterialVar *pVar = m_pMaterial->FindVar("$refractamount", &bFound, true);

	if (pVar)
		pVar->SetFloatValue(flStrength * conc_refract.GetFloat());

	pVar = m_pMaterial->FindVar("$bluramount", &bFound, true);

	if (pVar)
		pVar->SetFloatValue(flStrength * conc_blur.GetFloat());
}

//-----------------------------------------------------------------------------
// Purpose: Callback function for the concussion effect
//-----------------------------------------------------------------------------
void FF_FX_ConcussionEffect_Callback(const CEffectData &data)
{
	/*
	voogru: not enough time for this shit, spent 3-4 hours fucking with this and ragdolls are being a fucking pain in the ass, 
	I'm able to affect them, but not the way I want them to behave.

	CRagdollConcEnumerator ragdollEnum(data.m_vOrigin);
	partition->EnumerateElementsInSphere( PARTITION_CLIENT_RESPONSIVE_EDICTS, data.m_vOrigin, data.m_flRadius, false, &ragdollEnum );
	*/
#define TE_EXPLFLAG_NODLIGHTS	0x2	// do not render dynamic lights
#define TE_EXPLFLAG_NOSOUND		0x4	// do not play client explosion sound

	// If underwater, just do a bunch of gas
	if (UTIL_PointContents(data.m_vOrigin) & CONTENTS_WATER)
	{
		WaterExplosionEffect().Create(data.m_vOrigin, 180.0f, 10.0f, TE_EXPLFLAG_NODLIGHTS|TE_EXPLFLAG_NOSOUND);
		//return;
	}

	CBroadcastRecipientFilter filter;
		te->BeamRingPoint(filter,
			0,						// delay
			data.m_vOrigin,			// origin
			1.0f,					// start radius
			CONC_RADIUS,			// end radius
			g_iConcRingTexture,		// texture index
			0,						// halo index
			0,						// start frame
			CONC_FRAMERATE,			// frame rate
			CONC_LIFETIME,			// life
			CONC_WIDTH,				// width
			CONC_SPREAD,			// spread (10x end width)
			CONC_AMPLITUDE,			// amplitude
			CONC_R,					// r
			CONC_G,					// g
			CONC_B,					// b
			CONC_A,					// a
			0,						// speed
			FBEAM_FADEOUT | FBEAM_SINENOISE);	// flags

		te->BeamRingPoint(filter,
			0,						// delay
			( data.m_vOrigin + Vector( 0.0f, 0.0f, 32.0f ) ),			// origin
			1.0f,					// start radius
			CONC_RADIUS2,			// end radius
			g_iConcRingTexture,		// texture index
			0,						// halo index
			0,						// start frame
			CONC_FRAMERATE,			// frame rate
			CONC_LIFETIME,			// life
			CONC_WIDTH2,				// width
			CONC_SPREAD,			// spread (10x end width)
			CONC_AMPLITUDE,			// amplitude
			CONC_R,					// r
			CONC_G,					// g
			CONC_B,					// b
			CONC_A,					// a
			0,						// speed
			FBEAM_FADEOUT | FBEAM_SINENOISE);	// flags

		te->BeamRingPoint(filter,
			0,						// delay
			( data.m_vOrigin + Vector( 0.0f, 0.0f, -32.0f ) ),			// origin
			1.0f,					// start radius
			CONC_RADIUS2,			// end radius
			g_iConcRingTexture,		// texture index
			0,						// halo index
			0,						// start frame
			CONC_FRAMERATE,			// frame rate
			CONC_LIFETIME,			// life
			CONC_WIDTH2,				// width
			CONC_SPREAD,			// spread (10x end width)
			CONC_AMPLITUDE,			// amplitude
			CONC_R,					// r
			CONC_G,					// g
			CONC_B,					// b
			CONC_A,					// a
			0,						// speed
			FBEAM_FADEOUT | FBEAM_SINENOISE);	// flags


		// Outer bounding rings
		te->BeamRingPoint(filter,
			0,						// delay
			data.m_vOrigin,			// origin
			CONC_RADIUS - 1,					// start radius
			CONC_RADIUS,			// end radius
			g_iConcRingTexture,		// texture index
			0,						// halo index
			0,						// start frame
			CONC_FRAMERATE,			// frame rate
			CONC_LIFETIME,			// life
			CONC_WIDTH3,				// width
			CONC_SPREAD,			// spread (10x end width)
			CONC_AMPLITUDE,			// amplitude
			CONC_R,					// r
			CONC_G,					// g
			CONC_B,					// b
			CONC_A,					// a
			0,						// speed
			FBEAM_FADEOUT | FBEAM_SINENOISE);	// flags

/*
	// Okay so apparently dx7 is not so good for the 3d conc effect
	// So instead we use the flat one for those systems
	if (g_pMaterialSystemHardwareConfig->GetDXSupportLevel() > 70)
	{
		C_ConcEffect::CreateClientsideEffect("models/grenades/conc/conceffect.mdl", data.m_vOrigin );
	}
	else
	{
		CSmartPtr<CConcEmitter> concEffect = CConcEmitter::Create("ConcussionEffect");

		float offset = 0;

		for (int i = 0; i < conc_ripples.GetInt(); i++)
		{
			ConcParticle *c = concEffect->AddConcParticle();

			if (c)
			{
				c->m_flDieTime = conc_speed.GetFloat();
				c->m_Pos = data.m_vOrigin;
				c->m_flRefract = conc_refract.GetFloat();
				c->m_flOffset = offset;

				offset += conc_ripple_period.GetFloat();
			}
		}
	}
*/

}

DECLARE_CLIENT_EFFECT("FF_ConcussionEffect", FF_FX_ConcussionEffect_Callback);
