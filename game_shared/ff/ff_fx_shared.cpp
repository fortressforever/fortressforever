//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ff_fx_shared.h"
#include "ff_weapon_base.h"

#ifdef CLIENT_DLL

#include "fx_impact.h"
#include "c_ff_player.h"

	// this is a cheap ripoff from CBaseCombatWeapon::WeaponSound():
	void FX_WeaponSound(
		int iPlayerIndex,
		WeaponSound_t sound_type,
		const Vector &vOrigin,
		CFFWeaponInfo *pWeaponInfo )
	{

		// If we have some sounds from the weapon classname.txt file, play a random one of them
		const char *shootsound = pWeaponInfo->aShootSounds[ sound_type ]; 
		if ( !shootsound || !shootsound[0] )
			return;

		CBroadcastRecipientFilter filter; // this is client side only
		if ( !te->CanPredict() )
			return;
				
		CBaseEntity::EmitSound( filter, iPlayerIndex, shootsound, &vOrigin ); 
	}

	class CGroupedSound
	{
	public:
		string_t m_SoundName;
		Vector m_vPos;
	};

	CUtlVector<CGroupedSound> g_GroupedSounds;

	
	// Called by the ImpactSound function.
	void ShotgunImpactSoundGroup( const char *pSoundName, const Vector &vEndPos )
	{
		// Don't play the sound if it's too close to another impact sound.
		for ( int i=0; i < g_GroupedSounds.Count(); i++ )
		{
			CGroupedSound *pSound = &g_GroupedSounds[i];

			if ( vEndPos.DistToSqr( pSound->m_vPos ) < 300*300 )
			{
				if ( Q_stricmp( pSound->m_SoundName, pSoundName ) == 0 )
					return;
			}
		}

		// Ok, play the sound and add it to the list.
		CLocalPlayerFilter filter;
		C_BaseEntity::EmitSound( filter, NULL, pSoundName, &vEndPos );

		i = g_GroupedSounds.AddToTail();
		g_GroupedSounds[i].m_SoundName = pSoundName;
		g_GroupedSounds[i].m_vPos = vEndPos;
	}


	void StartGroupingSounds()
	{
		Assert( g_GroupedSounds.Count() == 0 );
		SetImpactSoundRoute( ShotgunImpactSoundGroup );
	}


	void EndGroupingSounds()
	{
		g_GroupedSounds.Purge();
		SetImpactSoundRoute( NULL );
	}

#else

	#include "ff_player.h"
	#include "te_firebullets.h"
	
	// Server doesn't play sounds anyway.
	void StartGroupingSounds() {}
	void EndGroupingSounds() {}
	void FX_WeaponSound ( int iPlayerIndex,
		WeaponSound_t sound_type,
		const Vector &vOrigin,
		CFFWeaponInfo *pWeaponInfo ) {};

#endif



// This runs on both the client and the server.
// On the server, it only does the damage calculations.
// On the client, it does all the effects.
void FX_FireBullets( 
	int	iPlayerIndex,
	const Vector &vOrigin,
	const QAngle &vAngles,
	int	iWeaponID,
	int	iMode,
	int iSeed,
	float flSpread,
	float flSniperRifleCharge	// added 9/20/2005 by Mulchman
	)
{
	bool bDoEffects = true;

#ifdef CLIENT_DLL
	C_FFPlayer *pPlayer = ToFFPlayer( ClientEntityList().GetBaseEntity( iPlayerIndex ) );
#else
	CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex( iPlayerIndex) );
#endif

	const char * weaponAlias =	WeaponIDToAlias( iWeaponID );

	if ( !weaponAlias )
	{
		DevMsg("FX_FireBullets: weapon alias for ID %i not found\n", iWeaponID );
		return;
	}

	char wpnName[128];
	Q_snprintf( wpnName, sizeof( wpnName ), "ff_weapon_%s", weaponAlias );
	WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( wpnName );

	if ( hWpnInfo == GetInvalidWeaponInfoHandle() )
	{
		DevMsg("FX_FireBullets: LookupWeaponInfoSlot failed for weapon %s\n", wpnName );
		return;
	}

	CFFWeaponInfo *pWeaponInfo = static_cast< CFFWeaponInfo* >( GetFileWeaponInfoFromHandle( hWpnInfo ) );

#ifdef CLIENT_DLL
	// Do the firing animation event.
	if ( pPlayer && !pPlayer->IsDormant() )
	{
		if ( iMode == Primary_Mode )
			pPlayer->m_PlayerAnimState->DoAnimationEvent( PLAYERANIMEVENT_FIRE_GUN_PRIMARY );
		else
			pPlayer->m_PlayerAnimState->DoAnimationEvent( PLAYERANIMEVENT_FIRE_GUN_SECONDARY );
	}
#else
	// if this is server code, send the effect over to client as temp entity
	// Dispatch one message for all the bullet impacts and sounds.
	TE_FireBullets( 
		iPlayerIndex,
		vOrigin, 
		vAngles, 
		iWeaponID,
		iMode,
		iSeed,
		flSpread
		);

	bDoEffects = false; // no effects on server
#endif

	iSeed++;

	float	flDamage = (pWeaponInfo->m_iBullets ? (float) pWeaponInfo->m_iDamage / pWeaponInfo->m_iBullets : pWeaponInfo->m_iDamage);	// |-- Mirv: Split damage for each shot
	int		iAmmoType = pWeaponInfo->iAmmoType;

	WeaponSound_t sound_type = SINGLE;

	if ( bDoEffects)
	{
		FX_WeaponSound( iPlayerIndex, sound_type, vOrigin, pWeaponInfo );
	}


	// Fire bullets, calculate impacts & effects

	if ( !pPlayer )
		return;
	
	StartGroupingSounds();

	for ( int iBullet=0; iBullet < pWeaponInfo->m_iBullets; iBullet++ )
	{
		RandomSeed( iSeed );	// init random system with this seed

		// Get circular gaussian spread.
		float x, y;
		x = RandomFloat( -0.5, 0.5 ) + RandomFloat( -0.5, 0.5 );
		y = RandomFloat( -0.5, 0.5 ) + RandomFloat( -0.5, 0.5 );
	
		iSeed++; // use new seed for next bullet

		pPlayer->FireBullet(
			vOrigin,
			vAngles,
			flSpread,
			flDamage,	// |-- Mirv: Float
			iAmmoType,
			pPlayer,
			bDoEffects,
			x,y,
			flSniperRifleCharge ); // added by Mulchman 9/20/2005
	}

	EndGroupingSounds();
}

