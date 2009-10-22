/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_env_flamejet.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date 20 March 2005
/// @brief Implements the server side of a flame jet particle system entity!
///
/// REVISIONS
/// ---------
/// Mar 20, 2005 Mirv: First logged


#include "cbase.h"
#include "baseparticleentity.h"
#include "ff_env_flamejet.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
// CFlameJet tables
//=============================================================================

IMPLEMENT_SERVERCLASS_ST(CFFFlameJet, DT_FFFlameJet) 
	SendPropInt(SENDINFO(m_bEmit), 1, SPROP_UNSIGNED), 	// Declare our boolean state variable
END_SEND_TABLE() 

LINK_ENTITY_TO_CLASS(env_flamejet, CFFFlameJet);

BEGIN_DATADESC(CFFFlameJet) 
	DEFINE_FIELD(m_bEmit, FIELD_BOOLEAN), 
END_DATADESC() 

//=============================================================================
// CWeaponFlamethrower implementation
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Called before spawning, after key values have been set.
//-----------------------------------------------------------------------------
void CFFFlameJet::Spawn() 
{
	m_bEmit = false;
}

//----------------------------------------------------------------------------
// Purpose: 
//----------------------------------------------------------------------------
void CFFFlameJet::UpdateOnRemove( void )
{
	m_bEmit = false;

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: Turn the flame jet on or off
//			Returns true if value has changed
//-----------------------------------------------------------------------------
bool CFFFlameJet::FlameEmit(bool bEmit)
{
	if ((m_bEmit != 0) != bEmit)
	{
		m_bEmit = bEmit;
		return true;
	}
	return false;
}
