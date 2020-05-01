/********************************************************************
	created:	2006/08/14
	created:	14:8:2006   11:09
	filename: 	f:\ff-svn\code\trunk\game_shared\ff\ff_grenade_base.h
	file path:	f:\ff-svn\code\trunk\game_shared\ff
	file base:	ff_grenade_base
	file ext:	h
	author:		Various
	
	purpose:	
*********************************************************************/

#ifndef FF_GRENADE_BASE_H
#define FF_GRENADE_BASE_H

#include "ff_projectile_base.h"
#include "Sprite.h"
#include "SpriteTrail.h"

#define GREN_ALPHA_DEFAULT	200

#ifdef CLIENT_DLL
	#define CFFGrenadeBase C_FFGrenadeBase

	#include "ff_grenade_parse.h"
#endif

//========================================================================
// Development convers
//========================================================================
#ifdef GAME_DLL
/*
	extern ConVar gren_radius;
	extern ConVar gren_grav;
	extern ConVar gren_fric;
	extern ConVar gren_elas;
	extern ConVar gren_fric_conc;
	extern ConVar gren_elas_conc;
*/
	#define GREN_RADIUS 180.0f
	#define GREN_GRAV 0.8f
	#define GREN_FRIC 0.25f
	#define GREN_ELAS 0.4f
	#define GREN_FRIC_CONC 0.3f
	#define GREN_ELAS_CONC 0.7f

	#define FRAG_GREN_DAMAGE 100.0f //145.0f //Previous damage
	#define FRAG_GREN_RADIUS 175.0f //270.0f //Previous damage radius
#endif

//=============================================================================
// CFFGrenadeBase class declaration
//=============================================================================
class CFFGrenadeBase : public CFFProjectileBase
{
public:
	DECLARE_CLASS( CFFGrenadeBase, CFFProjectileBase );
	DECLARE_NETWORKCLASS(); 

#ifdef GAME_DLL
	
	virtual float GetGrenadeGravity()		{ return GREN_GRAV; }
	virtual float GetGrenadeFriction()		{ return GREN_FRIC; }
	virtual float GetGrenadeElasticity()	{ return GREN_ELAS; }
	virtual float GetGrenadeDamage()		{ return 180.0f; }
	virtual float GetGrenadeRadius()		{ return GetGrenadeDamage() * 1.5f; }
	virtual float GetShakeAmplitude()		{ return 2.5f; }

#endif

#ifdef CLIENT_DLL
	CFFGrenadeBase()
	{
		m_flModelSize = 0.0f;
	}
#endif

	virtual void	Precache();
	virtual void	Spawn();
	virtual void	Detonate();
	virtual void	Explode(trace_t *pTrace, int bitsDamageType);
	void SetDetonateTimerLength(float timer);
	
	virtual Class_T		Classify( void ) { return CLASS_GREN; }
	Class_T	GetGrenId() { return Classify(); }
	virtual color32 CFFGrenadeBase::GetColour();

	bool	m_fIsHandheld;
	CNetworkVar( bool, m_bIsOn );

#ifdef GAME_DLL

	DECLARE_DATADESC();

	virtual void	GrenadeThink();
	virtual void	CreateTrail();

	void WaterCheck( void );

	// BaseClass projectile was making grens use its takeemp which just removed
	// them from the level
	// Bug #0000527: Emps makes other grenades disappear instead of detonating them.
	int TakeEmp( void ) { return 0; }

protected:

	//Custom collision to allow for constant elasticity on hit surfaces
	virtual void ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity );

	CNetworkVar(float, m_flDetonateTime);

	bool	m_bHitwater;
	float	m_flHitwaterTimer;

#else

protected:

	float	m_flModelSize;
	float	m_flDetonateTime;

private:	
	// This is only needed client-side so far
	virtual const unsigned char	*GetEncryptionKey() { return NULL; }
	GRENADE_FILE_INFO_HANDLE	m_hGrenadeFileInfo;

public:
	CFFGrenadeInfo const &GetFFGrenadeData() const;

	int		DrawModel(int flags);
	virtual void ClientThink();

#endif

private:
	CHandle<CSpriteTrail>	m_pTrail;
};

#endif //FF_GRENADE_BASE_H
