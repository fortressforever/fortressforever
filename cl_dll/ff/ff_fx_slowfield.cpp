/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_fx_slowfield_emitter.cpp
/// @author Paul Peloski (zero)
/// @date Feb 5, 2006
/// @brief slowfield effect emitter
///
/// Implementation of the slowfield effect emitter particle system
/// 
/// Revisions
/// ---------
/// Feb 5, 2006	zero: Initial Creation

#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "particles_simple.h"
#include "materialsystem/imaterialvar.h"
#include "c_te_effect_dispatch.h"
#include "beam_flags.h"
#include "fx_explosion.h"
#include "tempent.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"

#define FF_SLOWFIELD_MATERIAL "effects/slowfield"
#define FF_SLOWFIELD_MATERIAL_BLUE "effects/slowfield_blue"
#define FF_SLOWFIELD_MATERIAL_RED "effects/slowfield_red"
#define FF_SLOWFIELD_MATERIAL_YELLOW "effects/slowfield_yellow"
#define FF_SLOWFIELD_MATERIAL_GREEN "effects/slowfield_green"
#define FF_SLOWFIELD_TEXTURE_GROUP TEXTURE_GROUP_CLIENT_EFFECTS

extern ConVar ffdev_slowfield_duration;

CLIENTEFFECT_REGISTER_BEGIN(PrecacheSlowfieldEmitter)
	CLIENTEFFECT_MATERIAL(FF_SLOWFIELD_MATERIAL)
	CLIENTEFFECT_MATERIAL(FF_SLOWFIELD_MATERIAL_BLUE)
	CLIENTEFFECT_MATERIAL(FF_SLOWFIELD_MATERIAL_RED)
	CLIENTEFFECT_MATERIAL(FF_SLOWFIELD_MATERIAL_YELLOW)
	CLIENTEFFECT_MATERIAL(FF_SLOWFIELD_MATERIAL_GREEN)
CLIENTEFFECT_REGISTER_END()

class C_SlowfieldEffect : public C_BaseAnimating
{
	typedef C_BaseAnimating BaseClass;
public:

	static C_SlowfieldEffect	*CreateClientsideEffect(const char *pszModelName, Vector vecOrigin, int iTeam);

	bool	InitializeEffect( const char *pszModelName, Vector vecOrigin );
	void	ClientThink( void );
	virtual int	DrawModel( int flags );

protected:

	IMaterial	*m_pMaterial;
	float		m_flStart;
	int			m_iTeam;
};

//-----------------------------------------------------------------------------
// Purpose: Create the slowfield effect
//-----------------------------------------------------------------------------
C_SlowfieldEffect *C_SlowfieldEffect::CreateClientsideEffect(const char *pszModelName, Vector vecOrigin, int iTeam)
{
	C_SlowfieldEffect *pEffect = new C_SlowfieldEffect;

	if (pEffect == NULL)
		return NULL;

	if (pEffect->InitializeEffect(pszModelName, vecOrigin) == false)
		return NULL;

	pEffect->m_flStart = gpGlobals->curtime;
	pEffect->m_iTeam = iTeam;

	return pEffect;
}

//-----------------------------------------------------------------------------
// Purpose: Initialise all the slowfield stuff
//-----------------------------------------------------------------------------
bool C_SlowfieldEffect::InitializeEffect( const char *pszModelName, Vector vecOrigin )
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

int C_SlowfieldEffect::DrawModel( int flags )
{
	switch(m_iTeam)
	{
	case TEAM_BLUE:
		FindOverrideMaterial(FF_SLOWFIELD_MATERIAL_BLUE, FF_SLOWFIELD_TEXTURE_GROUP);
		break;
	case TEAM_RED:
		FindOverrideMaterial(FF_SLOWFIELD_MATERIAL_RED, FF_SLOWFIELD_TEXTURE_GROUP);
		break;
	case TEAM_YELLOW:
		FindOverrideMaterial(FF_SLOWFIELD_MATERIAL_YELLOW, FF_SLOWFIELD_TEXTURE_GROUP);
		break;
	case TEAM_GREEN:
		FindOverrideMaterial(FF_SLOWFIELD_MATERIAL_GREEN, FF_SLOWFIELD_TEXTURE_GROUP);
		break;
	default:
		FindOverrideMaterial(FF_SLOWFIELD_MATERIAL, FF_SLOWFIELD_TEXTURE_GROUP);
		break;
	}

	return BaseClass::DrawModel( flags );
}

//-----------------------------------------------------------------------------
// Purpose: Adjust the material proxies for the conc as time goes on
//-----------------------------------------------------------------------------
void C_SlowfieldEffect::ClientThink( void )
{
	if (gpGlobals->curtime - m_flStart >= ffdev_slowfield_duration.GetFloat())
	{
		Release();
		return;
	}
	
	C_BasePlayer *pPlayer = CBasePlayer::GetLocalPlayer();

	// We need to keep the correct part of the shader oriented towards the player
	// The bit we want is on the top, so rotate around x axis by 90
	Vector vecDir = GetAbsOrigin() - pPlayer->EyePosition();

	QAngle angFace;
	VectorAngles(vecDir, angFace);
	angFace.x += 90;

	SetAbsAngles(angFace);

	BaseClass::ClientThink();
}

//-----------------------------------------------------------------------------
// Purpose: Callback function for the slowfield effect
//-----------------------------------------------------------------------------
void FF_FX_SlowfieldEffect_Callback(const CEffectData &data)
{
	C_SlowfieldEffect::CreateClientsideEffect("models/grenades/conc/conceffect.mdl", data.m_vOrigin, (int)data.m_nColor );
	/*
	// Okay so apparently dx7 is not so good for the 3d slowfield effect
	// So instead we use the flat one for those systems
	if (g_pMaterialSystemHardwareConfig->GetDXSupportLevel() > 70)
	{
		C_SlowfieldEffect::CreateClientsideEffect("models/grenades/conc/conceffect.mdl", data.m_vOrigin );
	}
	*/
}

DECLARE_CLIENT_EFFECT("FF_SlowfieldEffect", FF_FX_SlowfieldEffect_Callback);
