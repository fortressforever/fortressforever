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
#define FF_SLOWFIELD_TEXTURE_GROUP TEXTURE_GROUP_CLIENT_EFFECTS

extern ConVar ffdev_slowfield_duration;

CLIENTEFFECT_REGISTER_BEGIN(PrecacheSlowfieldEmitter)
	CLIENTEFFECT_MATERIAL(FF_SLOWFIELD_MATERIAL)
CLIENTEFFECT_REGISTER_END()

class C_SlowfieldEffect : public C_BaseAnimating
{
	typedef C_BaseAnimating BaseClass;
public:

	static C_SlowfieldEffect	*CreateClientsideEffect(const char *pszModelName, Vector vecOrigin);

	bool	InitializeEffect( const char *pszModelName, Vector vecOrigin );
	void	ClientThink( void );
	virtual int	DrawModel( int flags );

protected:

	IMaterial	*m_pMaterial;
	float		m_flStart;
};

//-----------------------------------------------------------------------------
// Purpose: Create the slowfield effect
//-----------------------------------------------------------------------------
C_SlowfieldEffect *C_SlowfieldEffect::CreateClientsideEffect(const char *pszModelName, Vector vecOrigin)
{
	C_SlowfieldEffect *pEffect = new C_SlowfieldEffect;

	if (pEffect == NULL)
		return NULL;

	if (pEffect->InitializeEffect(pszModelName, vecOrigin) == false)
		return NULL;

	pEffect->m_flStart = gpGlobals->curtime;

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
	FindOverrideMaterial(FF_SLOWFIELD_MATERIAL, FF_SLOWFIELD_TEXTURE_GROUP);

	return BaseClass::DrawModel( flags );
}

//-----------------------------------------------------------------------------
// Purpose: Adjust the material proxies for the conc as time goes on
//-----------------------------------------------------------------------------
void C_SlowfieldEffect::ClientThink( void )
{
	// Bit of time to settle before releasing
	if (gpGlobals->curtime - m_flStart >= ffdev_slowfield_duration.GetFloat() + 0.2f)
	{
		Release();
		return;
	}
	BaseClass::ClientThink();
}

//-----------------------------------------------------------------------------
// Purpose: Callback function for the slowfield effect
//-----------------------------------------------------------------------------
void FF_FX_SlowfieldEffect_Callback(const CEffectData &data)
{
	// Okay so apparently dx7 is not so good for the 3d slowfield effect
	// So instead we use the flat one for those systems
	if (g_pMaterialSystemHardwareConfig->GetDXSupportLevel() > 70)
	{
		C_SlowfieldEffect::CreateClientsideEffect("models/grenades/conc/conceffect.mdl", data.m_vOrigin );
	}
}

DECLARE_CLIENT_EFFECT("FF_SlowfieldEffect", FF_FX_SlowfieldEffect_Callback);
