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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//==================================================
// CFlameJet [mirrored in ff_weapon_flamethrower.cpp]
//==================================================

class CFlameJet : public CBaseParticleEntity
{
public:
	DECLARE_CLASS(CFlameJet, CBaseParticleEntity);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	virtual void	Spawn();

public:
	CNetworkVar(int, m_bEmit);
};

//=============================================================================
// CFlameJet tables
//=============================================================================

IMPLEMENT_SERVERCLASS_ST(CFlameJet, DT_FlameJet) 
	SendPropInt(SENDINFO(m_bEmit), 1, SPROP_UNSIGNED), 	// Declare our boolean state variable
END_SEND_TABLE() 

LINK_ENTITY_TO_CLASS(env_flamejet, CFlameJet);

BEGIN_DATADESC(CFlameJet) 
	DEFINE_FIELD(m_bEmit, FIELD_BOOLEAN), 
END_DATADESC() 

//=============================================================================
// CWeaponFlamethrower implementation
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Called before spawning, after key values have been set.
//-----------------------------------------------------------------------------
void CFlameJet::Spawn() 
{
	m_bEmit = false;
}
