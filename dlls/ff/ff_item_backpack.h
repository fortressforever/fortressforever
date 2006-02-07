/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file f:\Program Files\Steam\SteamApps\SourceMods\FortressForeverCode\dlls\ff\ff_item_backpack.cpp
/// @author Shawn Smith (L0ki)
/// @date Jun. 26, 2005
/// @brief Backpack class
/// 
/// Declares the backpack entity
/// 
/// Revisions
/// ---------
/// Jun. 26, 2005	L0ki: Initial Creation

#ifndef FF_ITEM_BACKPACK_H
#define FF_ITEM_BACKPACK_H

#include "ammodef.h"
#include "items.h"
#include "ff_player.h"
#include "ff_weapon_base.h" //for ammo types

//#define BACKPACK_MODEL "models/items/item_item_crate.mdl"
#define BACKPACK_MODEL "models/items/backpack/backpack.mdl"
//#define BACKPACK_MODEL "models/items/healthkit.mdl"

class CFFItemBackpack : public CItem
{
public:
	DECLARE_CLASS( CFFItemBackpack, CItem );
	DECLARE_DATADESC();

	CFFItemBackpack( );

	void Spawn		( void );
	void Precache	( void );
	void RestockTouch	(CBaseEntity *);

	void SetSpawnFlags( int flags );
	bool CreateItemVPhysicsObject(void);

	void SetAmmoCount( int iIndex, int iNewCount );
	void SetGren1( int iNewCount );
	void SetGren2( int iNewCount );
	void SetArmor( int iNewArmor );
	void SetHealth( int iNewHealth );

	int GetAmmoCount( int iIndex );
	int GetGren1( void );
	int GetGren2( void );
	int GetArmor( void );
	int GetHealth( void );

	virtual int TakeEmp();

private:
	int m_iAmmoCounts[ MAX_AMMO_SLOTS ];
	int m_iGren1;
	int m_iGren2;
	int m_iArmor;
	int m_iHealth;

	float m_flSpawnTime;
};

#endif//FF_ITEM_BACKPACK_H