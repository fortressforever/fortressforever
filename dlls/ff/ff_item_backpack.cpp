/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file f:\Program Files\Steam\SteamApps\SourceMods\FortressForeverCode\dlls\ff\ff_item_backpack.cpp
/// @author Shawn Smith (L0ki)
/// @date Jun. 26, 2005
/// @brief Backpack class
/// 
/// Implements the backpack entity
/// 
/// Revisions
/// ---------
/// Jun. 26, 2005	L0ki: Initial Creation
/// Jan. 03, 2006	Mirv: A lot redone: does what its meant to do, doesn't crash linux srv.

#include "cbase.h"
#include "ff_item_backpack.h"
#include "tier0/memdbgon.h"

#define ITEM_PICKUP_BOX_BLOAT		24

BEGIN_DATADESC( CFFItemBackpack )

	// weapon ammo types
/*	DEFINE_KEYFIELD( m_iAmmoCounts[1], FIELD_INTEGER, AMMO_BULLETS ),
	DEFINE_KEYFIELD( m_iAmmoCounts[2], FIELD_INTEGER, AMMO_SHELLS ),
	DEFINE_KEYFIELD( m_iAmmoCounts[3], FIELD_INTEGER, AMMO_NAILS ),
	DEFINE_KEYFIELD( m_iAmmoCounts[4], FIELD_INTEGER, AMMO_ROCKETS ),
	DEFINE_KEYFIELD( m_iAmmoCounts[5], FIELD_INTEGER, AMMO_GRENADES ),
	DEFINE_KEYFIELD( m_iAmmoCounts[6], FIELD_INTEGER, AMMO_CELLS ),
	DEFINE_KEYFIELD( m_iAmmoCounts[7], FIELD_INTEGER, AMMO_DETPACK ),

	//grenades
	DEFINE_KEYFIELD( m_iGren1,	FIELD_INTEGER, "no_primary_grenades" ),
	DEFINE_KEYFIELD( m_iGren2,	FIELD_INTEGER, "no_secondary_grenades" ),

	//health and armor
	DEFINE_KEYFIELD( m_iArmor,	FIELD_INTEGER, "armorvalue" ),
	DEFINE_KEYFIELD( m_iHealth,	FIELD_INTEGER, "healthvalue" ),*/

	DEFINE_ENTITYFUNC( RestockTouch ),

END_DATADESC();

LINK_ENTITY_TO_CLASS( ff_item_backpack, CFFItemBackpack );
PRECACHE_REGISTER( ff_item_backpack );

CFFItemBackpack::CFFItemBackpack()
{
	m_spawnflags = 0;

	for (int i = 0; i < MAX_AMMO_SLOTS; i++)
		m_iAmmoCounts[i] = 0;

	m_iGren1	= 0;
	m_iGren2	= 0;

	m_iArmor	= 0;
	m_iHealth	= 0;

	m_flSpawnTime = gpGlobals->curtime;

	m_iDetpackIndex = GetAmmoDef()->Index( AMMO_DETPACK );
}

void CFFItemBackpack::Precache()
{
	PrecacheModel(BACKPACK_MODEL);
}

void CFFItemBackpack::Spawn()
{
	Precache();
	
	// Bug #0000131: Ammo, health and armor packs stop rockets
	// Projectiles won't collide with COLLISION_GROUP_WEAPON
	// We don't want to set as not-solid because we need to trace it for sniper rifle dot
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE|FSOLID_TRIGGER);
	SetCollisionGroup(COLLISION_GROUP_WEAPON);

	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );

	SetModel( BACKPACK_MODEL );
	
	CollisionProp()->UseTriggerBounds(true, ITEM_PICKUP_BOX_BLOAT);

	SetNextThink( gpGlobals->curtime + 30.0f );

	SetTouch(&CFFItemBackpack::RestockTouch);
	SetThink(&CFFItemBackpack::SUB_Remove);
}

int CFFItemBackpack::TakeEmp()
{
	int ammo = 0;

	for (int i = 1; i < MAX_AMMO_SLOTS; i++)
		ammo += m_iAmmoCounts[i];

	ammo += m_iGren1;
	ammo += m_iGren2;

	UTIL_Remove(this);

	return ammo;
}

void CFFItemBackpack::RestockTouch( CBaseEntity *pPlayer )
{
	if (gpGlobals->curtime - m_flSpawnTime < 1.0f)
		return;

	CFFPlayer *pFFPlayer = ToFFPlayer(pPlayer);

	if (pFFPlayer && pFFPlayer->IsAlive())
	{
		// COMMENTED OUT BY MULCHMAN BECAUSE ITS NOT USED AND THROWING A COMPILLE WARNING
		//CAmmoDef *pAmmoDef = GetAmmoDef();

		// TODO: only pickup ammo the class can carry
		// TODO: for the ammo the class can carry, only pickup up to its max carry

		int ammotaken = 0;

		for (int i = 1; i < MAX_AMMO_SLOTS; i++)
		{
			// Don't take detpack ammo if a player's detpack is active/alive
			if( pFFPlayer->m_hDetpack.Get() && ( m_iDetpackIndex == i ) )
				continue;

			ammotaken += pFFPlayer->GiveAmmo(m_iAmmoCounts[i], i);
		}

		ammotaken += pFFPlayer->AddPrimaryGrenades(m_iGren1);
		ammotaken += pFFPlayer->AddSecondaryGrenades(m_iGren2);

		// Only TakeHealth if we have hp to give, otherwise we'll heal their illnesses
		if (m_iHealth)
			ammotaken += pFFPlayer->TakeHealth(m_iHealth, DMG_GENERIC);

		if (ammotaken)
			UTIL_Remove(this);
	}
}

bool CFFItemBackpack::CreateItemVPhysicsObject()
{
	SetMoveType(MOVETYPE_NONE);

	// Bug #0000131: Ammo, health and armor packs stop rockets
	// Projectiles won't collide with COLLISION_GROUP_WEAPON
	// We don't want to set as not-solid because we need to trace it for sniper rifle dot
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE|FSOLID_TRIGGER);
	SetCollisionGroup(COLLISION_GROUP_WEAPON);

	// If it's not physical, drop it to the floor
	if (UTIL_DropToFloor(this, MASK_SOLID) == 0)
	{
		Warning( "xxxx Item %s fell out of level at %f,%f,%f\n", GetClassname(), GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z);
		UTIL_Remove( this );
		return false;
	}

	// make it respond to touches
	//SetCollisionGroup(COLLISION_GROUP_WEAPON);
	SetCollisionGroup(COLLISION_GROUP_NONE);
	CollisionProp()->UseTriggerBounds(true, ITEM_PICKUP_BOX_BLOAT);
	SetTouch(&CFFItemBackpack::RestockTouch);

	return true;
}

void CFFItemBackpack::SetSpawnFlags( int flags )
{
	m_spawnflags = flags;
}

void CFFItemBackpack::SetAmmoCount( int iIndex, int iNewCount )
{
	if( iIndex < 1 || iIndex >= MAX_AMMO_SLOTS )
	{
		AssertMsg(0, "CFFItemBackpack::SetAmmoCount: Invalid index\n");
		return;
	}

	m_iAmmoCounts[ iIndex ] = iNewCount;
}

void CFFItemBackpack::SetGren1( int iNewCount )
{
	m_iGren1 = iNewCount;
}

void CFFItemBackpack::SetGren2( int iNewCount )
{
	m_iGren2 = iNewCount;
}

void CFFItemBackpack::SetArmor( int iNewArmor )
{
	m_iArmor = iNewArmor;
}

void CFFItemBackpack::SetHealth( int iNewHealth )
{
	m_iHealth = iNewHealth;
}

int CFFItemBackpack::GetAmmoCount( int iIndex )
{
	if( iIndex < 0 || iIndex >= MAX_AMMO_SLOTS )
	{
		AssertMsg(0, "CFFItemBackpack::GetAmmoCount: Invalid index\n");
		return 0;
	}

	return m_iAmmoCounts[ iIndex ];
}

int CFFItemBackpack::GetGren1()
{
	return m_iGren1;
}

int CFFItemBackpack::GetGren2()
{
	return m_iGren2;
}

int CFFItemBackpack::GetArmor()
{
	return m_iArmor;
}

int CFFItemBackpack::GetHealth()
{
	return m_iHealth;
}
