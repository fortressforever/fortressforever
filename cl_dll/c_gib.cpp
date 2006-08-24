//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "vcollide_parse.h"
#include "c_gib.h"
#include "debugoverlay_shared.h"
#include "iefx.h"
#include "decals.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

int	C_Gib::m_iBloodDecal = -1;

//NOTENOTE: This is not yet coupled with the server-side implementation of CGib
//			This is only a client-side version of gibs at the moment

C_Gib::C_Gib()
{
	bool	m_bDecal = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_Gib::~C_Gib( void )
{
	VPhysicsDestroyObject();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pszModelName - 
//			vecOrigin - 
//			vecForceDir - 
//			vecAngularImp - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
C_Gib *C_Gib::CreateClientsideGib( const char *pszModelName, Vector vecOrigin, Vector vecForceDir, AngularImpulse vecAngularImp, float flLifetime )
{
	C_Gib *pGib = new C_Gib;

	if ( pGib == NULL )
		return NULL;

	if ( pGib->InitializeGib( pszModelName, vecOrigin, vecForceDir, vecAngularImp, flLifetime ) == false )
		return NULL;

	return pGib;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pszModelName - 
//			vecOrigin - 
//			vecForceDir - 
//			vecAngularImp - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_Gib::InitializeGib( const char *pszModelName, Vector vecOrigin, Vector vecForceDir, AngularImpulse vecAngularImp, float flLifetime )
{
	if ( InitializeAsClientEntity( pszModelName, RENDER_GROUP_OPAQUE_ENTITY ) == false )
	{
		Release();
		return false;
	}

	SetAbsOrigin( vecOrigin );
	SetCollisionGroup( COLLISION_GROUP_DEBRIS );

	solid_t tmpSolid;
	PhysModelParseSolid( tmpSolid, this, GetModelIndex() );
	
	m_pPhysicsObject = VPhysicsInitNormal( SOLID_VPHYSICS, 0, false, &tmpSolid );
	
	if ( m_pPhysicsObject )
	{
		float flForce = m_pPhysicsObject->GetMass();
		vecForceDir *= flForce;	

		m_pPhysicsObject->ApplyForceOffset( vecForceDir, GetAbsOrigin() );
		m_pPhysicsObject->SetCallbackFlags( m_pPhysicsObject->GetCallbackFlags() | CALLBACK_GLOBAL_TOUCH | CALLBACK_GLOBAL_TOUCH_STATIC );
	}
	else
	{
		// failed to create a physics object
		//Release();
		//return false;

		SetSolid(SOLID_BBOX);
		AddSolidFlags(FSOLID_NOT_STANDABLE);
		SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
	}

	SetNextClientThink( gpGlobals->curtime + flLifetime );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Gib::ClientThink( void )
{
	SetRenderMode( kRenderTransAlpha );
	m_nRenderFX		= kRenderFxFadeFast;

	if ( m_clrRender->a == 0 )
	{
#ifdef HL2_CLIENT_DLL
		s_AntlionGibManager.RemoveGib( this );
#endif
		Release();
		return;
	}

	SetNextClientThink( gpGlobals->curtime + 1.0f );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void C_Gib::StartTouch( C_BaseEntity *pOther )
{
	// Limit the amount of times we can bounce
	if ( m_flTouchDelta < gpGlobals->curtime )
	{
		HitSurface( pOther );
		m_flTouchDelta = gpGlobals->curtime + 0.1f;
	}

	BaseClass::StartTouch( pOther );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void C_Gib::HitSurface( C_BaseEntity *pOther )
{
	// Already done a decal
	if (m_bDecal)
		return;

	if (m_iBloodDecal == -1)
	{
		m_iBloodDecal = decalsystem->GetDecalIndexForName("Blood");
	}

	if (m_iBloodDecal >= 0 )
	{
		effects->DecalShoot(m_iBloodDecal, pOther->entindex(), pOther->GetModel(), pOther->GetAbsOrigin(), pOther->GetAbsAngles(), GetAbsOrigin(), 0, 0);
	}

	m_bDecal = true;
}
