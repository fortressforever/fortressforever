/********************************************************************
	created:	2006/01/31
	created:	31:1:2006   0:42
	filename: 	f:\cvs\code\cl_dll\ff\ProxyAmmo.cpp
	file path:	f:\cvs\code\cl_dll\ff
	file base:	ProxyAmmo
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	Ammo count material proxy for weapons
*********************************************************************/

#include "cbase.h"
#include "FunctionProxy.h"
#include "c_ff_player.h"
#include "ff_weapon_base.h"
#include "materialsystem/IMaterialVar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Returns ammo in active weapon(from 0 to 200) 
//-----------------------------------------------------------------------------
class CProxyAmmo : public CResultProxy
{
public:
	virtual bool Init(IMaterial *pMaterial, KeyValues *pKeyValues);
	virtual void OnBind(void *pC_BaseEntity);

private:
	int		m_iModifier;
	int		m_iPreviousValue;

	float	m_flLastTick;
};

bool CProxyAmmo::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	CFloatInput TextureScrollRate;
	TextureScrollRate.Init(pMaterial, pKeyValues, "digitindex", -1.0f);

	m_iModifier = (int) TextureScrollRate.GetFloat();

	m_iPreviousValue = 0;

	return CResultProxy::Init(pMaterial, pKeyValues);
}

void CProxyAmmo::OnBind(void *pC_BaseEntity)
{
	if (!pC_BaseEntity)
		return;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayerOrObserverTarget();
	Assert(pPlayer);

	if (!pPlayer)
		return;

	C_FFWeaponBase *pWeapon = pPlayer->GetActiveFFWeapon();
	Assert(pWeapon);

	if (!pWeapon)
		return;

	Assert(m_pResult);

	int iAmmo;

	// Make sure we report the right ammo
	if (pWeapon->UsesClipsForAmmo1())
	{
		iAmmo = pWeapon->m_iClip1;
	}
	else
	{
		iAmmo = pPlayer->GetAmmoCount(pWeapon->GetPrimaryAmmoType());
	}

	// Ammo has gone up, use a linear change
	// Should probably take the frame rate into account here
	if (iAmmo > m_iPreviousValue)
	{
		float flDelta = gpGlobals->curtime - m_flLastTick;

		// Just go straight to the right number if it's been a while since the last
		// tick. This means that it's probably been holstered and should show the 
		// correct value when it comes up.
		if (flDelta >= 0.0f && flDelta < 1.0f)
		{
			iAmmo = m_iPreviousValue + 1;
		}
	}

	// Store the previous value of this before we screw it up
	m_iPreviousValue = iAmmo;

	// Make sure we show the correct digit
	if (m_iModifier >= 0)
	{
		int p = pow(10.0f, m_iModifier);
		iAmmo /= p;
		iAmmo -= 10 * (iAmmo / 10);
	}

	if (pPlayer && pPlayer->GetActiveFFWeapon())
	{
		SetFloatResult(iAmmo);
	}
	else
	{
		SetFloatResult(0);
	}

	m_flLastTick = gpGlobals->curtime;
}

EXPOSE_INTERFACE(CProxyAmmo, IMaterialProxy, "CurrentAmmo" IMATERIAL_PROXY_INTERFACE_VERSION);

//=============================================================================
//
// Class CFFRadioTagData
//
//=============================================================================
class CFFProxyNumPipes : public CResultProxy
{
public:
	virtual void OnBind( void *pC_BaseEntity );
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFProxyNumPipes::OnBind( void *pC_BaseEntity )
{
	if( !pC_BaseEntity )
		return;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayerOrObserverTarget();
	if( !pPlayer )
	{
		Assert( 0 );
		return;
	}

	SetFloatResult( pPlayer->GetPipebombCounter()->GetPipes() );
}

EXPOSE_INTERFACE( CFFProxyNumPipes, IMaterialProxy, "FF_NumPipes" IMATERIAL_PROXY_INTERFACE_VERSION );
