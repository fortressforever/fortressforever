// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_weapon_sniperrifle.h
// @author Patrick O'Leary (Mulchman)
// @date May 2nd, 2007
// @brief The FF sniperrifle code .h file
//
// REVISIONS
// ---------
// May 02 2007:	Mulchman
//				Just adding the .h file

#ifndef FF_WEAPON_SNIPERRIFLE_H
#define FF_WEAPON_SNIPERRIFLE_H

#include "cbase.h"

#include "ff_weapon_base.h"
#include "ff_fx_shared.h"
#include "in_buttons.h"
#include "Sprite.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponSniperRifle C_FFWeaponSniperRifle
	#define CFFWeaponLaserDot C_FFWeaponLaserDot
	#include "c_ff_player.h"
	#include "view.h"

	#include "iviewrender_beams.h"
	#include "beam_shared.h"
	#include "beamdraw.h"
	#include "fx.h"
	#include "fx_sparks.h"
	#include "fx_line.h"
	#include "model_types.h"
	#include "ff_utils.h"
#else
	#include "ff_player.h"
#endif

#define SNIPER_BEAM		"effects/bluelaser1.vmt"
#define SNIPER_HALO		"sprites/muzzleflash1.vmt"
#define SNIPER_DOT		"sprites/sniperdot_red.vmt"

//=============================================================================
// CFFWeaponLaserDot
//=============================================================================

class CFFWeaponLaserDot : public CSprite
{
public:
	DECLARE_CLASS(CFFWeaponLaserDot, CSprite);

	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

#ifdef CLIENT_DLL
	CFFWeaponLaserDot();
	virtual void GetRenderBounds(Vector& mins, Vector& maxs);
#endif

	static	CFFWeaponLaserDot *Create(const Vector &origin, CBaseEntity *pOwner = NULL);

	void	SetLaserPosition(const Vector &origin);

	bool	IsOn() const		{ return m_bIsOn; }

	void	TurnOn() 		{ m_bIsOn = true; }
	void	TurnOff() 		{ m_bIsOn = false; }
	void	Toggle() 		{ m_bIsOn = !m_bIsOn; }

	int		ObjectCaps() { return (BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }

#ifdef CLIENT_DLL

	virtual bool			IsTransparent() { return true; }
	virtual RenderGroup_t	GetRenderGroup() { return RENDER_GROUP_TRANSLUCENT_ENTITY; }
	virtual int				DrawModel(int flags);
	virtual void			OnDataChanged(DataUpdateType_t updateType);
	virtual bool			ShouldDraw() { return (IsEffectActive(EF_NODRAW) ==false); }

	IMaterial	*m_pMaterial;
#endif

	CNetworkVar(float, m_flStartTime);

protected:
	bool				m_bIsOn;

};

// ============================================================================
// CFFWeaponSniperRifle
//=============================================================================

class CFFWeaponSniperRifle : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponSniperRifle, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CFFWeaponSniperRifle();
	~CFFWeaponSniperRifle();

	virtual void Precache();
	virtual void PrimaryAttack();
	virtual void Spawn();
	virtual void Fire();

	void ToggleZoom();

	virtual bool Deploy();
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);

	virtual void ItemPostFrame();
	virtual void ItemBusyFrame();

#ifdef CLIENT_DLL
	virtual float GetFOV();
#endif

	void UpdateLaserPosition();

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_SNIPERRIFLE; }

	virtual float GetRecoilMultiplier( void );

	float GetFireStartTime( void ) const		{ return m_flFireStartTime; }
	bool IsInFire( void ) const					{ return m_bInFire; }

private:
	bool m_bZoomed;
	bool m_bInFire;

	CFFWeaponSniperRifle(const CFFWeaponSniperRifle &);

	void CheckZoomToggle();
	void CheckFire();

#ifdef GAME_DLL
	CHandle<CFFWeaponLaserDot>	m_hLaserDot;
#endif

	float m_flFireStartTime;

#ifdef CLIENT_DLL
	float m_flZoomTime;
	float m_flNextZoomTime;
	int m_iUnchargedShots;		// Counts the number of consecutive uncharged shots (for hint code)
#endif
};

#endif // FF_WEAPON_SNIPERRIFLE_H
