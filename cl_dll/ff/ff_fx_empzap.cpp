/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file d:\ffsrc\cl_dll\ff_fx_empzap.cpp
/// @author Alan Smithee (ted_maul)
/// @date 2006/05/20
/// @brief EMP lightning effect
///
/// lightning zaps for the EMP explosion effect

#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "particles_simple.h"

#include "ff_fx_empzap.h"

#include "materialsystem/imaterialvar.h"
#include "c_te_effect_dispatch.h"
#include "mathlib.h"

void FF_FX_EmpZap_Callback(const CEffectData &data)
{
}

DECLARE_CLIENT_EFFECT("FF_EmpZap", FF_FX_EmpZap_Callback);