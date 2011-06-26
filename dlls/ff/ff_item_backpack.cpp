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
//
//	9/13/2006, Mulchman
//		Cleaned up a tad

#include "cbase.h"
#include "IEffects.h"
#include "ff_item_backpack.h"
#include "ff_gamerules.h"
#include "tier0/memdbgon.h"

#define ITEM_PICKUP_BOX_BLOAT		24

BEGIN_DATADESC( CFFItemBackpack )
	DEFINE_ENTITYFUNC( RestockTouch ),
	DEFINE_THINKFUNC( PreDespawnThink ),
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
}

void CFFItemBackpack::Precache()
{
	PrecacheModel(BACKPACK_MODEL);
	PrecacheScriptSound( "Backpack.touch" );
}

void CFFItemBackpack::Spawn()
{
	Precache();
	
	// Bug #0000131: Ammo, health and armor packs stop rockets
	// We don't want to set as not-solid because we need to trace it for sniper rifle dot
	// Jigggles: But with SetSolid(SOLID_BBOX), backpacks block doors/eles -- I think the sniper dot passing through backpacks isn't a big deal
	SetSolid(SOLID_NONE);
	AddSolidFlags(FSOLID_NOT_STANDABLE|FSOLID_TRIGGER);
	SetCollisionGroup(COLLISION_GROUP_TRIGGERONLY);

	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );

	SetModel( BACKPACK_MODEL );
	
	CollisionProp()->UseTriggerBounds(true, ITEM_PICKUP_BOX_BLOAT);

	SetNextThink( gpGlobals->curtime + 15.0f );

	SetTouch(&CFFItemBackpack::RestockTouch);
	SetThink(&CFFItemBackpack::PreDespawnThink);
}

void CFFItemBackpack::PreDespawnThink()
{
	Vector vecUp(0, 0, 1.0f);
	g_pEffects->Sparks(GetAbsOrigin(), 5, 1, &vecUp);

	SetNextThink( gpGlobals->curtime + 5.0f );
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

	return int ( ammo / 2) ;
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
			ammotaken += pFFPlayer->GiveAmmo(m_iAmmoCounts[i], i);

		ammotaken += pFFPlayer->AddPrimaryGrenades(m_iGren1);
		ammotaken += pFFPlayer->AddSecondaryGrenades(m_iGren2);

		// Only TakeHealth if we have hp to give, otherwise we'll heal their illnesses
		if (m_iHealth)
			ammotaken += pFFPlayer->TakeHealth(m_iHealth, DMG_GENERIC);

		// Lets let people take packs all the time so engies can't make us fodder!
		//if (ammotaken)
		EmitSound("Backpack.Touch");
		UTIL_Remove(this);
	}
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

///////////////////////////////
// HEALTH DROPS
// HEALTH DROPS
///////////////////////////////

#define HEALTHDROP_MODEL	"models/items/healthkit.mdl"
#define HEALTHDROP_SOUND	"HealthVial.Touch"
#define HEALTHDROP_LIFE		5.0f

class CFFItemHealthDrop : public CBaseAnimating
{
public:
	DECLARE_CLASS(CFFItemHealthDrop, CBaseAnimating);
	DECLARE_DATADESC();

	CFFItemHealthDrop()
	{
	}

	void Spawn()
	{
		Precache();
	
		// These are the same as the backpack.
		SetSolid(SOLID_NONE);
		AddSolidFlags(FSOLID_NOT_STANDABLE|FSOLID_TRIGGER);
		SetCollisionGroup(COLLISION_GROUP_TRIGGERONLY);

		SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);

		SetModel(HEALTHDROP_MODEL);
		SetSize(Vector(-10, -15, -0.1f), Vector(10, 15, 0.1f));
	
		CollisionProp()->UseTriggerBounds(true, ITEM_PICKUP_BOX_BLOAT);

		SetNextThink(gpGlobals->curtime + HEALTHDROP_LIFE);

		SetTouch(&CFFItemHealthDrop::RestockTouch);
		SetThink(&CFFItemHealthDrop::SUB_Remove);

		m_flSpawnTime = gpGlobals->curtime;
	}

	void Precache()
	{
		PrecacheModel(HEALTHDROP_MODEL);
		PrecacheScriptSound(HEALTHDROP_SOUND);
	}

	void RestockTouch(CBaseEntity *pEntity)
	{
		// This is disabled while medics cannot pick up their own medkits
		//if (gpGlobals->curtime - m_flSpawnTime < 1.0f)
		//	return;

		if (!pEntity->IsPlayer())
			return;

		CFFPlayer *pFFPlayer = ToFFPlayer(pEntity);
		CFFPlayer *pFFOwner = dynamic_cast <CFFPlayer *> (GetOwnerEntity());

		// Make sure a valid live owner and player
		if (!pFFOwner || !pFFPlayer || !pFFPlayer->IsAlive())
			return;

		// We can't pick up our own packs
		if (pFFPlayer == pFFOwner)
			return;

		// Enemies can't pick up this health
		if (FFGameRules()->IsTeam1AlliedToTeam2(pFFPlayer->GetTeamNumber(), pFFOwner->GetTeamNumber()) == GR_NOTTEAMMATE)
			return;

		bool bCured = pFFPlayer->Cure(pFFOwner);

		// Remove if we heal any health or if we cure any afflictions
		if ( ( pFFPlayer->GetHealth() < pFFPlayer->GetMaxHealth() && pFFPlayer->Heal(ToFFPlayer(GetOwnerEntity()), 15.0f, false) ) || bCured )
		{
			EmitSound(HEALTHDROP_SOUND);
			UTIL_Remove(this);	
		}
	}


	// Bug #0000656: Throwing discards at miniturret will freak it out
	virtual bool IsPlayer( void ) { return false; }
	virtual bool BlocksLOS( void ) { return false; }
	virtual bool IsAlive( void ) { return false; }

	virtual Class_T Classify( void ) { return CLASS_BACKPACK; }
	virtual bool CanClipOwnerEntity( void ) const { return true; }

private:
	float m_flSpawnTime;
};

BEGIN_DATADESC(CFFItemHealthDrop)
	DEFINE_ENTITYFUNC(RestockTouch),
END_DATADESC();

LINK_ENTITY_TO_CLASS(ff_item_healthdrop, CFFItemHealthDrop);
PRECACHE_REGISTER(ff_item_healthdrop);
