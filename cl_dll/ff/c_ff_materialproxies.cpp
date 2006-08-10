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

#include "cbase.h"
#include "c_ff_materialproxies.h"
#include <KeyValues.h>
#include "materialsystem/IMaterialVar.h"
#include "materialsystem/IMaterial.h"
#include "IClientRenderable.h"

//#include "c_ff_player.h"
#include "ff_buildableobjects_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

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
// Purpose: Deconstructor
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
			iTeam = ToFFPlayer( pEntity )->GetTeamNumber() - 1;
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
					iTeam = pBuildable->GetOwnerPlayer()->GetTeamNumber() - 1;
				}

				/*
				C_FFDetpack *pDetpack = ( C_FFDetpack * )pEntity;
				if( pDetpack )
				{	
					// Get the owner
					C_FFPlayer *pOwner = ToFFPlayer( pDetpack->m_hOwner.Get() );
					if( pOwner )
					{
						// Get team number!
						iTeam = pOwner->GetTeamNumber() - 1;
					}
				}
				*/
			}
			/*
			else if( pEntity->Classify() == CLASS_DISPENSER )
			{
				C_FFDispenser *pDispenser = ( C_FFDispenser * )pEntity;
				if( pDispenser )
				{	
					// Get the owner
					C_FFPlayer *pOwner = ToFFPlayer( pDispenser->m_hOwner.Get() );
					if( pOwner )
					{
						// Get team number!
						iTeam = pOwner->GetTeamNumber() - 1;
					}
				}
			}
			else if( pEntity->Classify() == CLASS_SENTRYGUN )
			{
				C_FFSentryGun *pSentryGun = ( C_FFSentryGun * )pEntity;
				if( pSentryGun )
				{	
					// Get the owner
					C_FFPlayer *pOwner = ToFFPlayer( pSentryGun->m_hOwner.Get() );
					if( pOwner )
					{
						// Get team number!
						iTeam = pOwner->GetTeamNumber() - 1;
					}
				}
			}
			*/
			else if( pEntity->Classify() == CLASS_NONE )
			{
				Warning( "[Team Color Proxy] (Classify() == CLASS_NONE)\n" );
				iTeam = -1;
			}
			else
			{
				Warning( "[Team Color Proxy] No Class_T entry for classname: %s\n", pEntity->GetClassname() );
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
//	class C_GlowMaterialProxy
//
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_GlowMaterialProxy::C_GlowMaterialProxy( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
C_GlowMaterialProxy::~C_GlowMaterialProxy( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_GlowMaterialProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	// Return false, was just playing with this for fun.
	return false;

	const char *pszValue = "$selfillumtint";
	
	bool bFound = true;

	m_pValue = pMaterial->FindVar( pszValue, &bFound );

	if( !bFound )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_GlowMaterialProxy::OnBind( void *pC_BaseEntity )
{
	// Don't do anything, was just playing with this
	return;

	// Get the entity this material is on
	C_BaseEntity *pEntity = ( ( IClientRenderable * )pC_BaseEntity )->GetIClientUnknown()->GetBaseEntity();
	if( pEntity )
	{
		if( pEntity->IsPlayer() )
		{
		}
		else if( ( pEntity->Classify() == CLASS_DISPENSER ) || ( pEntity->Classify() == CLASS_SENTRYGUN ) || ( pEntity->Classify() == CLASS_DETPACK ) )
		{
			C_FFBuildableObject *pBuildable = dynamic_cast< C_FFBuildableObject * >( pEntity );
			if( pBuildable )
			{
				int iTeam = pBuildable->GetOwnerPlayer()->GetTeamNumber();
				switch( iTeam )
				{
				case TEAM_BLUE:
					Warning( "[Glow Proxy] Blue Something!\n" );
					m_pValue->SetVecValue( 0.0f, 0.0f, 1.0f );
					//m_pValue->SetIntValue( 1 );
					break;

				case TEAM_RED:
					//m_pValue->SetVecValue( 1.0f, 0.0f, 0.0f );
					break;

				case TEAM_YELLOW:
					break;

				case TEAM_GREEN:
					break;
				}
			}
		}
		else
		{
			char szAssertMsg[ 256 ];
			Q_snprintf( szAssertMsg, sizeof( szAssertMsg ), "[Glow Proxy] Unknown entity (%s) trying to use glow material proxy!\n", pEntity->GetClassname() );
			AssertMsg( false, szAssertMsg );
		}
	}
}

EXPOSE_INTERFACE( C_GlowMaterialProxy, IMaterialProxy, "Glow" IMATERIAL_PROXY_INTERFACE_VERSION )
