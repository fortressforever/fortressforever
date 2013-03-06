// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======

#ifndef FF_BUILDABLEOBJECT_H
#define FF_BUILDABLEOBJECT_H

#ifdef _WIN32
#pragma once
#endif

//#include "cbase.h"
//#include "ai_basenpc.h"
//#include "server_class.h"
//#include "ff_player.h"
//#include "soundent.h"
//#include "ff_buildableobjects_shared.h"

//#include <igameevents.h>
//#include <igamesystem.h>
//class CFFPlayer;

//inline void Build_ReturnDetpack( CFFPlayer *pPlayer )
//{
//	pPlayer->GiveAmmo( 1, "AMMO_DETPACK" );
//}

/*
class CFFBuildableObject : public CAI_BaseNPC
{
public:
	DECLARE_CLASS( CFFBuildableObject, CAI_BaseNPC )
	DECLARE_SERVERCLASS()

public:
	CFFBuildableObject( void );
	~CFFBuildableObject( void );

	virtual void Spawn( void ); 
	void GoLive( void );
	virtual void Precache( void );
	virtual bool BlocksLOS( void ) { return false; }

	virtual int	BloodColor( void ) { return BLOOD_COLOR_MECH; } // |-- Mirv: Don't bleed

	bool IsBuilt() { return m_bBuilt; } // |-- Mirv: Moved here from CFFSentry

	void Detonate( void );
	void RemoveQuietly( void );
	virtual void Cancel( void ) { RemoveQuietly(); }
	bool CheckForOwner( void )
	{
		if( !m_hOwner.Get() )
		{
			RemoveQuietly();
			return false;
		}
		
		return true;
	}
	//void EyeVectors( )
	
	// NOTE: Super class handles touch function
	// void OnObjectTouch( CBaseEntity *pOther );
	void OnObjectThink( void );

	int OnTakeDamage( const CTakeDamageInfo &info );

	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual bool IsAlive( void ) { return true; }
	virtual bool IsPlayer( void ) const { return false; }

	bool ShouldSavePhysics( void ) { return false; }

	// Mirv: Store in advance the ground position
	virtual void SetGroundAngles(const QAngle &ang) { m_angGroundAngles = ang; }
	virtual void SetGroundOrigin(const Vector &vec) { m_vecGroundOrigin = vec; }

private:
	// NOTE: Don't call the CFFBuildableObject::Create function
	static CFFBuildableObject *Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = NULL );

public:
	// So weapons (like the railgun) don't effect building
	virtual int VPhysicsTakeDamage( const CTakeDamageInfo &info );

protected:
	void Explode( void );
	void SpawnGib( const char *szGibModel, bool bFlame = true, bool bDieGroundTouch = false );
	void DoExplosion( void );

	virtual void SendStatsToBot() {};
protected:

	// Mirv: Store in advance the ground position
	QAngle m_angGroundAngles;
	Vector m_vecGroundOrigin;

	// Pointer to array of char *'s of model names
	const char **m_ppszModels;
	// Pointer to array of char *'s of gib model names
	const char **m_ppszGibModels;
	// Pointer to array of char *'s of sounds
	const char **m_ppszSounds;

	// For the explosion function

	// Explosion magnitude (int)
	int		m_iExplosionMagnitude;
	// Explosion magnitude (float)
	float	m_flExplosionMagnitude;
	// Explosion radius (float -> 3.5*magnitude)
	float	m_flExplosionRadius;
	// Explosion radius (int -> 3.5*magnitude)
	int		m_iExplosionRadius;
	// Explosion force
	float	m_flExplosionForce;
	// Explosion damage (for radius damage - same as flExplosion force)
	float	m_flExplosionDamage;
	// Explosion duration (duration of screen shaking)
	float	m_flExplosionDuration;
	// Explosion fireball scale
	int		m_iExplosionFireballScale;

	// Time (+ gpGlobals->curtime) that we will think (update network vars)
	float	m_flThinkTime;// = 0.2f;

	// Shockwave texture
	int		m_iShockwaveExplosionTexture;
	// Draw shockwaves
	bool	m_bShockWave;

	// Object is live and in color (not being built)
	//bool	m_bBuilt;
	CNetworkVar( bool, m_bBuilt );
	// Object takes damage once it is built
	bool	m_bTakesDamage;

	// Object has sounds associated with it
	bool	m_bHasSounds;

	// Whether or not the model is translucent
	// while building
	bool	m_bTranslucent;

	// If true we should be using physics
	bool	m_bUsePhysics;

public:	
	// Player who owns us
	//CBaseEntity *m_hOwner;
	CNetworkHandle( CBaseEntity, m_hOwner );

};
*/

#endif // FF_BUILDABLEOBJECT_H
