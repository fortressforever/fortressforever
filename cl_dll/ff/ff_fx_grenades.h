/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_fx_grenades.h
/// @author Shawn Smith (L0ki)
/// @date Apr. 30, 2005
/// @brief declarations for grenade FX
///
/// Declarations for all of the grenade effect functions
/// 
/// Revisions
/// ---------
/// Apr. 30, 2005	L0ki: Initial Creation

#ifndef FF_FX_GRENADES_H
#define FF_FX_GRENADES_H

#include "particles_simple.h"
#include "c_pixel_visibility.h"

void FF_FX_ConcussionExplosion	( Vector &origin );
void FF_FX_NapalmBurst			( Vector &origin );
void FF_FX_GasCloud				( Vector &origin );
void FF_FX_EmpExplosion			( Vector &origin );

#endif//FF_FX_GRENADES_H