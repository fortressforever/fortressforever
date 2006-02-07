//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "in_buttons.h"
#include "takedamageinfo.h"
#include "weapon_ffbase.h"
#include "ammodef.h"


#if defined( CLIENT_DLL )

	#include "c_ff_player.h"

#else

	#include "ff_player.h"

#endif


// ----------------------------------------------------------------------------- //
// Global functions.
// ----------------------------------------------------------------------------- //

//--------------------------------------------------------------------------------------------------------
static const char * s_WeaponAliasInfo[] = 
{
	"none",		// WEAPON_NONE
	"mp5",		// WEAPON_MP5
	"shotgun",	// WEAPON_SHOTGUN
	"grenade",	// WEAPON_GRENADE
	NULL,		// WEAPON_NONE
};

//--------------------------------------------------------------------------------------------------------
//
// Given an alias, return the associated weapon ID
//
int AliasToWeaponID( const char *alias )
{
	if (alias)
	{
		for( int i=0; s_WeaponAliasInfo[i] != NULL; ++i )
			if (!Q_stricmp( s_WeaponAliasInfo[i], alias ))
				return i;
	}

	return WEAPON_NONE;
}

//--------------------------------------------------------------------------------------------------------
//
// Given a weapon ID, return its alias
//
const char *WeaponIDToAlias( int id )
{
	if ( (id >= WEAPON_MAX) || (id < 0) )
		return NULL;

	return s_WeaponAliasInfo[id];
}

// ----------------------------------------------------------------------------- //
// CWeaponFFBase tables.
// ----------------------------------------------------------------------------- //

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponFFBase, DT_WeaponFFBase )

BEGIN_NETWORK_TABLE( CWeaponFFBase, DT_WeaponFFBase )
#ifdef CLIENT_DLL
  
#else
	// world weapon models have no animations
  	SendPropExclude( "DT_AnimTimeMustBeFirst", "m_flAnimTime" ),
	SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponFFBase )
	DEFINE_PRED_FIELD( m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_NOERRORCHECK ),
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_ff_base, CWeaponFFBase );


#ifdef GAME_DLL

	BEGIN_DATADESC( CWeaponFFBase )

		// New weapon Think and Touch Functions go here..

	END_DATADESC()

#endif

// ----------------------------------------------------------------------------- //
// CWeaponCSBase implementation. 
// ----------------------------------------------------------------------------- //
CWeaponFFBase::CWeaponFFBase()
{
	SetPredictionEligible( true );

	AddSolidFlags( FSOLID_TRIGGER ); // Nothing collides with these but it gets touches.
}

const CFFWeaponInfo &CWeaponFFBase::GetFFWpnData() const
{
	const FileWeaponInfo_t *pWeaponInfo = &GetWpnData();
	const CFFWeaponInfo *pFFInfo;

	#ifdef _DEBUG
		pFFInfo = dynamic_cast< const CFFWeaponInfo* >( pWeaponInfo );
		Assert( pFFInfo );
	#else
		pFFInfo = static_cast< const CFFWeaponInfo* >( pWeaponInfo );
	#endif

	return *pFFInfo;
}

bool CWeaponFFBase::PlayEmptySound()
{
	CPASAttenuationFilter filter( this );
	filter.UsePredictionRules();

	EmitSound( filter, entindex(), "Default.ClipEmpty_Rifle" );
	
	return 0;
}

CFFPlayer* CWeaponFFBase::GetPlayerOwner() const
{
	return dynamic_cast< CFFPlayer* >( GetOwner() );
}

#ifdef GAME_DLL

void CWeaponFFBase::SendReloadEvents()
{
	CFFPlayer *pPlayer = dynamic_cast< CFFPlayer* >( GetOwner() );
	if ( !pPlayer )
		return;

	// Send a message to any clients that have this entity to play the reload.
	CPASFilter filter( pPlayer->GetAbsOrigin() );
	filter.RemoveRecipient( pPlayer );

	UserMessageBegin( filter, "ReloadEffect" );
	WRITE_SHORT( pPlayer->entindex() );
	MessageEnd();

	// Make the player play his reload animation.
	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD );
}

#endif
