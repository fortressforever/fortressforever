//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file c_ff_materialproxies.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 09/23/2005
//	@brief Fortress Forever Material Proxies
//
//	REVISIONS
//	---------
//	09/23/2005,	Mulchman: 
//		First created
//
//	08/10/2006, Mulchman:
//		Removed some unneeded stuff. I don't think a
//		couple of these are used anymore, though.
//
// 04/11/2007, Mulchman:
//		Adding team score mat proxies

#include "cbase.h"
#include "c_ff_materialproxies.h"
#include <KeyValues.h>
#include "materialsystem/IMaterialVar.h"
#include "materialsystem/IMaterial.h"
#include "IClientRenderable.h"
#include "ff_shareddefs.h"

#include "ff_buildableobjects_shared.h"
#include "c_ff_team.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//ConVar ffdev_spy_mincloakness( "ffdev_spy_mincloakness", "0.05" );
#define SPY_MINCLOAKNESS 1.0 //ffdev_spy_mincloakness.GetFloat()
//ConVar ffdev_spy_maxrefractval( "ffdev_spy_maxrefractval", "0.5" );
#define SPY_MAXREFRACTVAL 1.0 //ffdev_spy_maxrefractval.GetFloat()

//=============================================================================
//
//	class C_TeamColorMaterialProxy
//
//=============================================================================

// Strings we need to find for this material proxy to get loaded correctly
// Note the basclass one is a STUB and doesn't do anything.
// The order is important - the $ value we will be changing needs to
// come first followed by the strings to look for to get the team
// coloring values from.
const char *g_ppszTeamColorStrings[ ] =
{
	NULL
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_TeamColorMaterialProxy::C_TeamColorMaterialProxy( void )
{
	// Point this to STUB (super class will override this)
	m_ppszStrings = g_ppszTeamColorStrings;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
C_TeamColorMaterialProxy::~C_TeamColorMaterialProxy( void )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool C_TeamColorMaterialProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	int iCount = 0;	
	while( m_ppszStrings[ iCount ] != NULL )
	{
		bool bFound = true;

		if( iCount == 0 )
		{
			m_pValue = pMaterial->FindVar( m_ppszStrings[ iCount ], &bFound );

			if( !bFound )
				return false;
		}
		else
		{
			IMaterialVar *pMatVar = pMaterial->FindVar( m_ppszStrings[ iCount ], &bFound );

			if( !bFound )
				return false;

			Vector vecVals;
			pMatVar->GetVecValue( vecVals.Base(), pMatVar->VectorSize() );

			m_vecTeamColorVals[ iCount - 1 ] = vecVals;
		}

		iCount++;
	}

	DevMsg( "[Team Color] Material Proxy succesful!\n" );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TeamColorMaterialProxy::OnBind( void *pC_BaseEntity )
{
	// Get the entity this material is on
	C_BaseEntity *pEntity = ( ( IClientRenderable * )pC_BaseEntity )->GetIClientUnknown()->GetBaseEntity();
	if( pEntity )
	{
		// Now we need to figure out what team this entity is on.
		// For players, it is easy.
		// For buildable objects, it's a little complicated.
		// For other objects, beware - there's no code yet. :P

		int iTeam = -1;

		if( pEntity->IsPlayer() )
		{
			// TODO: Test on players...

			// Adjust to get the actual team
			// 1 = blue
			// 2 = red
			// 3 = yellow
			// 4 = green
			CFFPlayer *pFFPlayer = ToFFPlayer( pEntity );
			if ( pFFPlayer )
				iTeam = pFFPlayer->GetTeamNumber() - 1;
			else
				iTeam = -1;
		}
		else
		{
			// Get non-player's team

			// Get the class (actual c++ class name)
			//const char *pszClassname = pEntity->GetClassname();

			if( ( pEntity->Classify() == CLASS_DETPACK ) || ( pEntity->Classify() == CLASS_DISPENSER ) || ( pEntity->Classify() == CLASS_SENTRYGUN ) )
			{
				C_FFBuildableObject *pBuildable = dynamic_cast< C_FFBuildableObject * >( pEntity );
				if( pBuildable )
				{
					CFFPlayer *pBuildableOwner = pBuildable->GetOwnerPlayer();
					if ( pBuildableOwner )
						iTeam = pBuildableOwner->GetTeamNumber() - 1;
					else
						iTeam = -1;
				}
			}
			else if( pEntity->Classify() == CLASS_NONE )
			{
				//Warning( "[Team Color Proxy] (Classify() == CLASS_NONE)\n" );
				iTeam = -1;
			}
			else
			{
				//Warning( "[Team Color Proxy] No Class_T entry for classname: %s\n", pEntity->GetClassname() );
				iTeam = -1;
			}
		}

		// Adjust for array indexing
		iTeam--;

		// Do the coloring
		if(( iTeam >= 0 ) && ( iTeam <= 3 ))
			m_pValue->SetVecValue( m_vecTeamColorVals[ iTeam ].x, m_vecTeamColorVals[ iTeam ].y, m_vecTeamColorVals[ iTeam ].z );
		else
			m_pValue->SetVecValue( 1, 1, 1 );	// all white
	}
}

// Don't use this - just here to copy/paste from
//EXPOSE_INTERFACE( C_TeamColorMaterialProxy, IMaterialProxy, "TeamColor" IMATERIAL_PROXY_INTERFACE_VERSION )

//=============================================================================
//
//	class C_Color_TeamColorMaterialProxy
//
//=============================================================================
const char *g_ppszColor_TeamColorStrings[ ] =
{
	"$color",
	"$TeamColorBlue",
	"$TeamColorRed",
	"$TeamColorYellow",
	"$TeamColorGreen",
	NULL
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_Color_TeamColorMaterialProxy::C_Color_TeamColorMaterialProxy( void )
{
	// Overwrite
	m_ppszStrings = g_ppszColor_TeamColorStrings;
}

EXPOSE_INTERFACE( C_Color_TeamColorMaterialProxy, IMaterialProxy, "Color_TeamColor" IMATERIAL_PROXY_INTERFACE_VERSION )

//=============================================================================
//
//	class C_Refract_TeamColorMaterialProxy
//
//=============================================================================
const char *g_ppszRefract_TeamColorStrings[ ] =
{
	"$refracttint",
	"$TeamColorBlue",
	"$TeamColorRed",
	"$TeamColorYellow",
	"$TeamColorGreen",
	NULL
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_Refract_TeamColorMaterialProxy::C_Refract_TeamColorMaterialProxy( void )
{
	// Overwrite
	m_ppszStrings = g_ppszRefract_TeamColorStrings;
}

EXPOSE_INTERFACE( C_Refract_TeamColorMaterialProxy, IMaterialProxy, "Refract_TeamColor" IMATERIAL_PROXY_INTERFACE_VERSION )

//=============================================================================
//
//	class C_FFPlayerVelocityMaterialProxy
//
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_FFPlayerVelocityMaterialProxy::C_FFPlayerVelocityMaterialProxy( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
C_FFPlayerVelocityMaterialProxy::~C_FFPlayerVelocityMaterialProxy( void )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool C_FFPlayerVelocityMaterialProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	if( !CResultProxy::Init( pMaterial, pKeyValues ) )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_FFPlayerVelocityMaterialProxy::OnBind( void *pC_BaseEntity )
{
	if( !pC_BaseEntity )
		return;

	//C_BaseEntity* pEntity = ( C_BaseEntity * )pC_BaseEntity;
	C_BaseEntity *pEntity = ( ( IClientRenderable * )pC_BaseEntity )->GetIClientUnknown()->GetBaseEntity();
	if( !pEntity )
		return;

	if( !pEntity->IsPlayer() )
		return;

	C_FFPlayer *pPlayer = ToFFPlayer( pEntity );
	if( !pPlayer )
		return;

	Assert( m_pResult );

	//float flSpeed = pPlayer->GetCloakSpeed();
	float flSpeed = pPlayer->GetLocalVelocity().Length();

	float flVal = clamp( flSpeed / ffdev_spy_maxcloakspeed.GetFloat(), SPY_MINCLOAKNESS, SPY_MAXREFRACTVAL );

	// Player Velocity
	SetFloatResult( flVal );

	//Warning( "[Player Velocity Proxy] %s - %f\n", pPlayer->GetPlayerName(), flVal );
}

EXPOSE_INTERFACE( C_FFPlayerVelocityMaterialProxy, IMaterialProxy, "FF_PlayerVelocityProxy" IMATERIAL_PROXY_INTERFACE_VERSION );

//=============================================================================
//
//	class C_FFLocalPlayerVelocityMaterialProxy
//
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_FFLocalPlayerVelocityMaterialProxy::C_FFLocalPlayerVelocityMaterialProxy( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
C_FFLocalPlayerVelocityMaterialProxy::~C_FFLocalPlayerVelocityMaterialProxy( void )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool C_FFLocalPlayerVelocityMaterialProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	if( !CResultProxy::Init( pMaterial, pKeyValues ) )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_FFLocalPlayerVelocityMaterialProxy::OnBind( void *pC_BaseEntity )
{
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return;

	Assert( m_pResult );

	float flSpeed = pPlayer->GetLocalVelocity().Length();
	float flVal = clamp( flSpeed / ffdev_spy_maxcloakspeed.GetFloat(), SPY_MINCLOAKNESS, SPY_MAXREFRACTVAL );

	// Player Velocity
	SetFloatResult( flVal );
}

EXPOSE_INTERFACE( C_FFLocalPlayerVelocityMaterialProxy, IMaterialProxy, "FF_LocalPlayerVelocityProxy" IMATERIAL_PROXY_INTERFACE_VERSION );

//=============================================================================
//
//	class C_FFWeaponVelocityMaterialProxy
//
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_FFWeaponVelocityMaterialProxy::C_FFWeaponVelocityMaterialProxy( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
C_FFWeaponVelocityMaterialProxy::~C_FFWeaponVelocityMaterialProxy( void )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool C_FFWeaponVelocityMaterialProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	if( !CResultProxy::Init( pMaterial, pKeyValues ) )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_FFWeaponVelocityMaterialProxy::OnBind( void *pC_BaseEntity )
{
	if( !pC_BaseEntity )
		return;

	C_BaseEntity *pEntity = ( ( IClientRenderable * )pC_BaseEntity )->GetIClientUnknown()->GetBaseEntity();
	if( !pEntity )
		return;

	C_FFWeaponBase *pWeapon = dynamic_cast< C_FFWeaponBase * >( pEntity );
	if( !pWeapon )
		return;

	C_FFPlayer *pWeaponOwner = pWeapon->GetPlayerOwner();
	if( !pWeaponOwner )
		return;

	Assert( m_pResult );

	float flSpeed = pWeaponOwner->GetLocalVelocity().Length();
	float flVal = clamp( flSpeed / ffdev_spy_maxcloakspeed.GetFloat(), SPY_MINCLOAKNESS, SPY_MAXREFRACTVAL );

	// Weapon Velocity
	SetFloatResult( flVal );

	//Warning( "[Weapon Velocity Proxy] %s - %f (%f)\n", pWeaponOwner->GetPlayerName(), pWeaponOwner->GetLocalVelocity().Length() );
}

EXPOSE_INTERFACE( C_FFWeaponVelocityMaterialProxy, IMaterialProxy, "FF_WeaponVelocityProxy" IMATERIAL_PROXY_INTERFACE_VERSION )

//=============================================================================
//
//	class C_FFSpyCloakMaterialProxy
//
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_FFSpyCloakMaterialProxy::C_FFSpyCloakMaterialProxy( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
C_FFSpyCloakMaterialProxy::~C_FFSpyCloakMaterialProxy( void )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool C_FFSpyCloakMaterialProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	if( !CResultProxy::Init( pMaterial, pKeyValues ) )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_FFSpyCloakMaterialProxy::OnBind( void *pC_BaseEntity )
{
	if( !pC_BaseEntity )
		return;

	C_BaseEntity *pEntity = ( ( IClientRenderable * )pC_BaseEntity )->GetIClientUnknown()->GetBaseEntity();
	if( !pEntity )
		return;

	C_FFPlayer *pPlayer = NULL;
	
	// Player
	if( pEntity->IsPlayer() )
	{
		pPlayer = ToFFPlayer( pEntity );
	}
	// Something else
	else
	{
		// Viewmodel
		C_BaseViewModel *pViewModel = dynamic_cast< C_BaseViewModel * >( pEntity );
		if( pViewModel )
		{
			if( pViewModel->IsViewModel() )
			{
				pPlayer = C_FFPlayer::GetLocalFFPlayerOrObserverTarget();
			}
		}

		// Weapon
		C_FFWeaponBase *pWeapon = dynamic_cast< C_FFWeaponBase * >( pEntity );
		if( pWeapon && !pPlayer )
		{
			pPlayer = pWeapon->GetPlayerOwner();
		}
	}	

	// No valid player, quit
	if( !pPlayer )
		return;

	Assert( m_pResult );

	float flSpeed = pPlayer->GetLocalVelocity().Length();
	float flVal = clamp( flSpeed / ffdev_spy_maxcloakspeed.GetFloat(), SPY_MINCLOAKNESS, SPY_MAXREFRACTVAL );

	// Update the value in the material proxy
	SetFloatResult( flVal );

	//Warning( "[Spy Cloak Proxy] %s - %f (%f)\n", pPlayer->GetPlayerName(), flSpeed );
}

EXPOSE_INTERFACE( C_FFSpyCloakMaterialProxy, IMaterialProxy, "FF_SpyCloakProxy" IMATERIAL_PROXY_INTERFACE_VERSION )

//=============================================================================
//
//	class C_FFTeamScore_MaterialProxy
//
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_FFTeamScore_MaterialProxy::C_FFTeamScore_MaterialProxy( void )
{
	m_iTeam = FF_TEAM_UNASSIGNED;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool C_FFTeamScore_MaterialProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	if( !CResultProxy::Init( pMaterial, pKeyValues ) )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_FFTeamScore_MaterialProxy::OnBind( void *pC_BaseEntity )
{
	C_FFTeam *pTeam = GetGlobalFFTeam( m_iTeam );
	if( !pTeam )
		return;

	// Update the value in the material proxy
	SetFloatResult( (float)pTeam->Get_Score() );
}

EXPOSE_INTERFACE( C_FFTeamScore_Blue_MaterialProxy, IMaterialProxy, "FF_TeamScore_Blue_Proxy" IMATERIAL_PROXY_INTERFACE_VERSION )
EXPOSE_INTERFACE( C_FFTeamScore_Red_MaterialProxy, IMaterialProxy, "FF_TeamScore_Red_Proxy" IMATERIAL_PROXY_INTERFACE_VERSION )
EXPOSE_INTERFACE( C_FFTeamScore_Yellow_MaterialProxy, IMaterialProxy, "FF_TeamScore_Yellow_Proxy" IMATERIAL_PROXY_INTERFACE_VERSION )
EXPOSE_INTERFACE( C_FFTeamScore_Green_MaterialProxy, IMaterialProxy, "FF_TeamScore_Green_Proxy" IMATERIAL_PROXY_INTERFACE_VERSION )
