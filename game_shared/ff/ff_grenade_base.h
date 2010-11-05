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
	extern ConVar gren_radius;
	extern ConVar gren_grav;
	extern ConVar gren_fric;
	extern ConVar gren_elas;
	extern ConVar gren_fric_conc;
	extern ConVar gren_elas_conc;
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
	
	virtual float GetGrenadeGravity()		{ return gren_grav.GetFloat(); }
	virtual float GetGrenadeFriction()		{ return gren_fric.GetFloat(); }
	virtual float GetGrenadeElasticity()	{ return gren_elas.GetFloat(); }
	virtual float GetGrenadeDamage()		{ return 180.0f; }
	virtual float GetGrenadeRadius()		{ return GetGrenadeDamage() * 1.5f; }
	virtual float GetShakeAmplitude()		{ return 2.5f; }

	bool m_fIsHandheld;
#endif

#ifdef CLIENT_DLL
	CFFGrenadeBase()
	{
		m_flModelSize = 0.0f;
	}
#endif

	virtual void	Precache();
	
	virtual Class_T		Classify( void ) { return CLASS_GREN; }
	Class_T	GetGrenId() { return Classify(); }
	virtual color32 CFFGrenadeBase::GetColour();

#ifdef GAME_DLL

	DECLARE_DATADESC();

	virtual void	Spawn();
	virtual void	CreateTrail();

	virtual void	GrenadeThink();
	virtual void	Detonate();
	virtual void	Explode(trace_t *pTrace, int bitsDamageType);

	void WaterCheck( void );
	void SetDetonateTimerLength(float timer);

	// BaseClass projectile was making grens use its takeemp which just removed
	// them from the level
	// Bug #0000527: Emps makes other grenades disappear instead of detonating them.
	int TakeEmp( void ) { return 0; }

protected:

	//Custom collision to allow for constant elasticity on hit surfaces
	virtual void ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity );

	CNetworkVarForDerived(float, m_flDetonateTime);

	bool	m_bHitwater;
	float	m_flHitwaterTimer;

#else

private:

	float	m_flModelSize;
	
	// This is only needed client-side so far
	virtual const unsigned char	*GetEncryptionKey() { return NULL; }
	GRENADE_FILE_INFO_HANDLE	m_hGrenadeFileInfo;

public:
	CFFGrenadeInfo const &GetFFGrenadeData() const;

	int		DrawModel(int flags);

#endif

private:
	CHandle<CSpriteTrail>	m_pTrail;
};

#endif //FF_GRENADE_BASE_H
