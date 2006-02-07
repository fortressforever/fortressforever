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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
// Debug ConVars
//=============================================================================

static ConVar flame_spreadspeed(	"ffdev_flame_spreadspeed", 	"50", 	0, 	"How fast the flames spread outwards");
static ConVar flame_speed(			"ffdev_flame_speed", 			"800", 	0, 	"How fast the flames go forwards");
static ConVar flame_startsize(		"ffdev_flame_startsize", 		"3", 	0, 	"How big the flame starts(0-255) ");
static ConVar flame_endsize(		"ffdev_flame_endsize", 		"128", 	0, 	"How big the flame finishes(0-255) ");
static ConVar flame_rate(			"ffdev_flame_rate", 			"128", 	0, 	"Number of flame particles per second");
static ConVar flame_alpha(			"ffdev_flame_alpha", 			"0.95", 	0, 	"Alpha value of the flame(0 - 1.0) ");

static ConVar flame_fadeout_time(	"ffdev_flame_fadeout_time", 	"0.2", 		0, 	"How long before end of life will flames fade out(in seconds) ");

static ConVar flame_chance_smallflame(		"ffdev_flame_chance_smallflame", 		"5", 	0, "Chance of a litle small flame in the main stream(higher integer = less chance) ");
static ConVar flame_chance_smoke(			"ffdev_flame_chance_smoke", 			"1", 	0, "Chance of some smoke(higher integer = less chance) ");
static ConVar flame_chance_bouncequick(		"ffdev_flame_chance_bouncequick", 		"3", 	0, "Chance of a litle quick moving bounce flame(higher integer = less chance) ");
static ConVar flame_chance_bounceflareup(	"ffdev_flame_chance_bounceflareup", 	"3", 	0, "Chance of a big flarey flame after bounce(higher integer = less chance) ");

static ConVar flame_position_forward(	"ffdev_flame_position_forward", 	"24", 	0, "Where the flames begin");
static ConVar flame_position_up(		"ffdev_flame_position_up", 		"-16", 	0, "Where the flames begin");
static ConVar flame_position_right(		"ffdev_flame_position_right", 		"8", 	0, "Where the flames begin");

static ConVar flame_length_min(		"ffdev_flame_length_min", 		"0.3", 	0, 	"Length of the flames in seconds");
static ConVar flame_length_max(		"ffdev_flame_length_max", 		"0.4", 	0, 	"Length of the flames in seconds");

//=============================================================================
// Globals
//=============================================================================

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
// C_FlameJet
//=============================================================================

class C_FlameJet : public C_BaseParticleEntity, public IPrototypeAppEffect
{
public:
	DECLARE_CLIENTCLASS();
	DECLARE_CLASS(C_FlameJet, C_BaseParticleEntity);

	C_FlameJet();
	~C_FlameJet();

	//=========================================================================
	// C_FlameJetParticle
	//=========================================================================

	class C_FlameJetParticle : public Particle
	{
	public:
		
		int				m_Type, m_Appearance;		// Type of particle
		Vector			m_Origin, m_Velocity;					
		float			m_flRoll, m_flRollDelta;	// Rotation
		float			m_Lifetime, m_Dietime;		// To keep track
		float			m_Collisiontime;			// When particle will next hit
													// something
		unsigned char	m_uchStartSize, m_uchEndSize;
		Vector			m_HitSurfaceNormal;			// Normal of collision surface
	};

	int IsEmissive() { return true; /*return (m_spawnflags & SF_EMISSIVE);*/ }

// C_BaseEntity
public:

	virtual void		OnDataChanged(DataUpdateType_t updateType);

// IPrototypeAppEffect
public:
	virtual void		Start(CParticleMgr *pParticleMgr, IPrototypeArgAccess *pArgs);
	virtual bool		GetPropEditInfo(RecvTable **ppTable, void **ppObj);


// IParticleEffect
public:
	virtual void		Update(float fTimeDelta);
	virtual void		RenderParticles(CParticleRenderIterator *pIterator);
	virtual void		SimulateParticles(CParticleSimulateIterator *pIterator);

public:

	int				m_bEmit;

	float			m_SpreadSpeed;
	float			m_Speed;
	float			m_StartSize;
	float			m_EndSize;
	float			m_Rate;
	float			m_Lifetime; 

private:

	CParticleMgr		*m_pParticleMgr;
	PMaterialHandle		m_hMaterialFlame, m_hMaterialSmoke;
	TimedEvent			m_ParticleSpawn;

private:
					C_FlameJet(const C_FlameJet &);
};


//=============================================================================
// C_FlameJet tables
//=============================================================================

// Expose to the particle app
EXPOSE_PROTOTYPE_EFFECT(FlameJet, C_FlameJet);

IMPLEMENT_CLIENTCLASS_DT(C_FlameJet, DT_FlameJet, CFlameJet) 
	RecvPropInt(RECVINFO(m_bEmit), 0), 
END_RECV_TABLE() 

CLIENTEFFECT_REGISTER_BEGIN(PrecacheFlameJet) 
CLIENTEFFECT_MATERIAL("effects/compositeflare_bothalpha") 
CLIENTEFFECT_REGISTER_END() 

//=============================================================================
// C_FlameJet implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
C_FlameJet::C_FlameJet() 
{
	m_pParticleMgr	= NULL;
	m_hMaterialFlame = m_hMaterialSmoke = INVALID_MATERIAL_HANDLE;
	
	m_SpreadSpeed	= flame_spreadspeed.GetInt();	// 50;
	m_Speed			= flame_speed.GetInt();			// 800;
	m_StartSize		= flame_startsize.GetInt();		// 3;
	m_EndSize		= flame_endsize.GetInt();		// 128;
	m_Rate			= flame_rate.GetInt();			// 128; 

	m_bEmit			= true;

	m_ParticleEffect.SetAlwaysSimulate(false); // Don't simulate outside the PVS or frustum.
}

//----------------------------------------------------------------------------
// Purpose: Destructor
//----------------------------------------------------------------------------
C_FlameJet::~C_FlameJet() 
{
	if (m_pParticleMgr) 
		m_pParticleMgr->RemoveEffect(&m_ParticleEffect);
}

//----------------------------------------------------------------------------
// Purpose: Called after a data update has occured
// Input  : bnewentity - 
//----------------------------------------------------------------------------
void C_FlameJet::OnDataChanged(DataUpdateType_t updateType) 
{
	C_BaseEntity::OnDataChanged(updateType);

	if (updateType == DATA_UPDATE_CREATED) 
	{
		Start(&g_ParticleMgr, NULL);
	}

	m_ParticleEffect.SetParticleCullRadius(max(m_StartSize, m_EndSize));
}

//-----------------------------------------------------------------------------
// Purpose: Starts the effect
// Input  : *pParticleMgr - 
//			*pArgs - 
//-----------------------------------------------------------------------------
void C_FlameJet::Start(CParticleMgr *pParticleMgr, IPrototypeArgAccess *pArgs) 
{
	pParticleMgr->AddEffect(&m_ParticleEffect, this);

	m_hMaterialFlame	= m_ParticleEffect.FindOrAddMaterial("effects/compositeflare_bothalpha");
	
	m_ParticleSpawn.Init(m_Rate);
	m_pParticleMgr = pParticleMgr;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : **ppTable - 
//			**ppObj - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_FlameJet::GetPropEditInfo(RecvTable **ppTable, void **ppObj) 
{
	*ppTable = &REFERENCE_RECV_TABLE(DT_FlameJet);
	*ppObj = this;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : fTimeDelta - 
//-----------------------------------------------------------------------------
void C_FlameJet::Update(float fTimeDelta) 
{
	if (!m_pParticleMgr) 
	{
		assert(false);
		return;
	}

	CFFPlayer *pOwner = dynamic_cast<CFFPlayer *> (GetOwnerEntity());

	// Flames without an owner is just weird.
	if (!pOwner) 
		return;

	// Default positions and angles
	Vector vecStart = pOwner->Weapon_ShootPosition();
	QAngle angAngles = pOwner->EyeAngles();

	if (pOwner->IsLocalPlayer()) 
	{
		// Use the V_ model attachment
		C_BaseViewModel *vm = pOwner->GetViewModel(0);

		if (vm) 
		{
			int iAttachment = vm->LookupAttachment("1");
			vm->GetAttachment(iAttachment, vecStart, angAngles);
		}
	}
	else
	{
		// Use the W_ model attachment
		C_BaseCombatWeapon *cr = pOwner->GetActiveWeapon();

		if (cr) 
		{
			int iAttachment = cr->LookupAttachment("1");
			cr->GetAttachment(iAttachment, vecStart, angAngles);
		}
	}

	Vector forward, right, up;
	AngleVectors(angAngles, &up, &right, &forward);

	// Set the bbox so the particle manager knows when to draw this flamejet.
	Vector vEndPoint = vecStart + forward * m_Speed;
	Vector vMin, vMax;
	VectorMin(vecStart, vEndPoint, vMin);
	VectorMax(vecStart, vEndPoint, vMax);
	m_ParticleEffect.SetBBox(vMin, vMax);

	// Temp
	m_SpreadSpeed	= flame_spreadspeed.GetInt();	// 50;
	m_Speed			= flame_speed.GetInt();			// 800;
	m_StartSize		= flame_startsize.GetInt();		// 3;
	m_EndSize		= flame_endsize.GetInt();		// 128;


	if (m_bEmit && m_ParticleEffect.WasDrawnPrevFrame()) 
	{
		float tempDelta = fTimeDelta;
		while (m_ParticleSpawn.NextEvent(tempDelta)) 
		{
			// Make a new particle.
			if (C_FlameJetParticle *pParticle = (C_FlameJetParticle *) m_ParticleEffect.AddParticle(sizeof(C_FlameJetParticle), m_hMaterialFlame)) 
			{
				pParticle->m_Type			= FLAME_JET;
				pParticle->m_Appearance		= (random->RandomInt(0, flame_chance_smallflame.GetInt() /*5 */) == 0 ? TEX_FLAME_SMALL1 : TEX_FLAME_NORMAL);

				pParticle->m_Pos			= vecStart;
				pParticle->m_Origin			= pParticle->m_Pos;
				
				pParticle->m_Velocity		= FRand(-m_SpreadSpeed, m_SpreadSpeed) * right +
											  FRand(-m_SpreadSpeed, m_SpreadSpeed) * up +
											  m_Speed * forward;
				
				pParticle->m_Lifetime		= 0;
				pParticle->m_Dietime		= random->RandomFloat(flame_length_min.GetFloat() /*0.3 */, flame_length_max.GetFloat() /*0.4 */);

				pParticle->m_uchStartSize	= m_StartSize;
				pParticle->m_uchEndSize		= m_EndSize;

				pParticle->m_flRoll			= random->RandomFloat(0, 360);
				pParticle->m_flRollDelta	= random->RandomFloat(-8.0f, 8.0f);

				trace_t tr;

				// How far can this particle travel
				UTIL_TraceLine(pParticle->m_Pos, pParticle->m_Pos + (pParticle->m_Velocity * pParticle->m_Dietime), 	MASK_SOLID, GetOwnerEntity(), COLLISION_GROUP_NONE, &tr);

				pParticle->m_Collisiontime = tr.fraction * pParticle->m_Dietime;
				pParticle->m_HitSurfaceNormal = tr.plane.normal;
			}

			// Sometimes, I want a bit of smoke
			if (random->RandomInt(0, flame_chance_smoke.GetInt() /*1 */) == 0) 
			{
				if (C_FlameJetParticle *pParticle = (C_FlameJetParticle *) m_ParticleEffect.AddParticle(sizeof(C_FlameJetParticle), /*m_hMaterialSmoke */ m_hMaterialFlame)) 
				{
					pParticle->m_Type			= SMOKE;
					pParticle->m_Appearance		= TEX_SMOKE;

					pParticle->m_Pos			= vecStart;
					pParticle->m_Origin			= pParticle->m_Pos;
					
					pParticle->m_Velocity		= FRand(-m_SpreadSpeed, m_SpreadSpeed) * right +
												  FRand(0 , 3 *m_SpreadSpeed) * up +
												  m_Speed * forward;
					
					pParticle->m_Lifetime		= 0;
					pParticle->m_Dietime		= random->RandomFloat(flame_length_min.GetFloat() /*0.3 */, flame_length_max.GetFloat() /*0.4 */);

					pParticle->m_uchStartSize	= m_StartSize;
					pParticle->m_uchEndSize		= m_EndSize;

					pParticle->m_flRoll			= random->RandomFloat(0, 360);
					pParticle->m_flRollDelta	= random->RandomFloat(-8.0f, 8.0f);

					trace_t tr;

					// How far can this particle travel
					UTIL_TraceLine(pParticle->m_Pos, pParticle->m_Pos + (pParticle->m_Velocity * pParticle->m_Dietime), 	MASK_SOLID, GetOwnerEntity(), COLLISION_GROUP_NONE, &tr);

					pParticle->m_Collisiontime = tr.fraction * pParticle->m_Dietime;
					pParticle->m_HitSurfaceNormal = tr.plane.normal;
				}
			}
		}
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
	pBuilder->TexCoord2f(0, tex_coords[appearance][0], tex_coords[appearance][3]);
 	pBuilder->AdvanceVertex();

	pBuilder->Position3f(pos.x + (-ca - sa) * size, pos.y + (-sa + ca) * size, pos.z);
	pBuilder->Color4ubv(ubColor);
	pBuilder->TexCoord2f(0, tex_coords[appearance][0], tex_coords[appearance][2]);
 	pBuilder->AdvanceVertex();

	pBuilder->Position3f(pos.x + (ca - sa) * size, pos.y + (sa + ca) * size, pos.z);
	pBuilder->Color4ubv(ubColor);
	pBuilder->TexCoord2f(0, tex_coords[appearance][1], tex_coords[appearance][2]);
 	pBuilder->AdvanceVertex();

	pBuilder->Position3f(pos.x + (ca + sa) * size, pos.y + (sa - ca) * size, pos.z);
	pBuilder->Color4ubv(ubColor);
	pBuilder->TexCoord2f(0, tex_coords[appearance][1], tex_coords[appearance][3]);
 	pBuilder->AdvanceVertex();

}

//----------------------------------------------------------------------------
// Purpose: Render all the particles
//----------------------------------------------------------------------------
void C_FlameJet::RenderParticles(CParticleRenderIterator *pIterator) 
{
	const C_FlameJetParticle *pParticle = (const C_FlameJetParticle *) pIterator->GetFirst();
	while (pParticle) 
	{
		// Render.
		Vector tPos;
		TransformParticle(m_pParticleMgr->GetModelView(), pParticle->m_Pos, tPos);
		float sortKey = tPos.z;

		// Normal alpha
		float alpha = flame_alpha.GetFloat(); //0.95f; // 180.0f

		// Fade out everything in its last moments
		if (/*pParticle->m_Type != SMOKE &&*/ pParticle->m_Dietime - pParticle->m_Lifetime < flame_fadeout_time.GetFloat() /*0.2f */) 
		{
			alpha *= ((pParticle->m_Dietime - pParticle->m_Lifetime) / flame_fadeout_time.GetFloat() /*0.2f */);
		}

		// Fade in smoke after a delay
		if (pParticle->m_Type == SMOKE && pParticle->m_Lifetime < flame_fadeout_time.GetFloat() /*0.2f */) 
		{
			alpha *= pParticle->m_Lifetime / /*0.2f */ flame_fadeout_time.GetFloat();
		}

		// Randomise the brightness
		float col = random->RandomFloat(0.3f, 0.5f);

		// Smoke should generally be darker
		if (pParticle->m_Type == SMOKE) 
		{
			col = random->RandomFloat(0.1f, 0.2f);
		}

		RenderParticle_ColorSizeAngle(
			pIterator->GetParticleDraw(), 
			tPos, 
			Vector(col, col, col), 
			alpha, 
			FLerp(pParticle->m_uchStartSize, pParticle->m_uchEndSize, pParticle->m_Lifetime), 
			pParticle->m_flRoll, 
			pParticle->m_Appearance);

		pParticle = (const C_FlameJetParticle *) pIterator->GetNext(sortKey);
	}
}

//----------------------------------------------------------------------------
// Purpose: Simulate the particles, this function is quite messy atm
//----------------------------------------------------------------------------
void C_FlameJet::SimulateParticles(CParticleSimulateIterator *pIterator) 
{
	C_FlameJetParticle *pParticle = (C_FlameJetParticle *) pIterator->GetFirst();

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

			if (pParticle->m_Lifetime > pParticle->m_Collisiontime) 
			{
				// This is a flamejet, so modify and now change to a flame lick
				if (pParticle->m_Type == FLAME_JET) 
				{
					// Pull out of the surface
					pParticle->m_Pos = pParticle->m_Origin + (pParticle->m_Velocity * (pParticle->m_Collisiontime - 0.01f));
					pParticle->m_Type = FLAME_LICK;

					// Some crossproducts(really!) 
					Vector cp1 = CrossProduct(pParticle->m_Velocity, pParticle->m_HitSurfaceNormal);
					Vector cp2 = CrossProduct(pParticle->m_HitSurfaceNormal, cp1);

					// Change the velocity to be parallel to the surface it hit
					float normal_len = pParticle->m_HitSurfaceNormal.Length();

					// Save from the dreaded divide by zero
					if (!normal_len) 
						normal_len += 0.01f;

					pParticle->m_Velocity = cp2 / normal_len;

					// Okay some flames get smaller and faster
					if (random->RandomInt(0, flame_chance_bouncequick.GetInt() /*3 */) == 0) 
					{
						pParticle->m_Velocity *= 2.0f;
						pParticle->m_uchEndSize /= 2;
						pParticle->m_uchStartSize /= 2;
					}
					// Okay, some turn into some silly shape
					else if (random->RandomInt(0, flame_chance_bounceflareup.GetInt() /*4 */) == 0) 
					{
						pParticle->m_Appearance = TEX_FLAME_SPLASH;
						pParticle->m_flRoll = 0;
						pParticle->m_flRollDelta = 0;
						pParticle->m_uchEndSize = 120;
					}

					// Work out next point of collision
					trace_t tr;

					// Work our how far of the route left we can go
					UTIL_TraceLine(pParticle->m_Pos, pParticle->m_Pos + pParticle->m_Velocity * (pParticle->m_Dietime - pParticle->m_Lifetime), MASK_SOLID, GetOwnerEntity(), COLLISION_GROUP_NONE, &tr);

					pParticle->m_Collisiontime += tr.fraction * (pParticle->m_Dietime - pParticle->m_Lifetime);

					// Wait wait, why are flames only allowed to bounce once?
					// Well, we don't want loads of trace's being done for loads 
					// of particles, and we have to trace the same route on the 
					// server too, so limiting it is a pretty good idea really!
				}
				// It's either smoke or a flame lick, so remove it
				else
				{
					pIterator->RemoveParticle(pParticle);
				}
			}
		}

		pParticle = (C_FlameJetParticle *) pIterator->GetNext();
	}
}
