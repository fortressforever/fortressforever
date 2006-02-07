/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file \Steam\SteamApps\SourceMods\FortressForeverCode\dlls\ff\ff_item_flag.h
/// @author Kevin Hjelden (FryGuy)
/// @date Jun. 29, 2005
/// @brief Flag Item (generic lua entity)
/// 
/// Implements a basic flag item for the lua scripting system to implement
/// 
/// Revisions
/// ---------
/// Jun. 29, 2005	FryGuy: Initial Creation

#ifndef FF_ITEM_FLAG_H
#define FF_ITEM_FLAG_H

#include "ammodef.h"
#include "items.h"
#include "ff_player.h"
#include "ff_weapon_base.h" //for ammo types

//#define FLAG_MODEL "models/items/backpack/backpack.mdl"
#define FLAG_MODEL "models/items/healthkit.mdl"

class CFFItemFlag : public CBaseAnimating
{
public:
	DECLARE_CLASS( CFFItemFlag, CBaseAnimating );
	DECLARE_SERVERCLASS();		// |-- Mirv: Added for client class
	DECLARE_DATADESC();

	CFFItemFlag( );

	void Spawn		( void );
	void Precache	( void );
	void OnTouch( CBaseEntity * );
	void OnPlayerDied( CFFPlayer * );
	void OnThink( void );
	void OnRespawn( void );

	void Pickup( CFFPlayer * );
	void Drop( float delay, float speed = 0.0f );
	void Respawn( float delay );
	CBaseEntity* Return ( void );

	void SetSpawnFlags( int flags );

private:

	bool CreateItemVPhysicsObject( void );

	Vector m_vStartOrigin;
	QAngle m_vStartAngles;

	CFFPlayer *m_pLastOwner;
	
	CNetworkVar(float, m_flThrowTime);
};

#endif//FF_ITEM_FLAG_H