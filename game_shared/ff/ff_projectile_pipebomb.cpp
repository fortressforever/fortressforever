/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_pipebomb.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 24, 2004
/// @brief The FF pipebomb projectile code.
///
/// REVISIONS
/// ---------
/// Dec 24, 2004 Mirv: First created


#include "cbase.h"
#include "ff_projectile_pipebomb.h"

#define PIPEBOMB_MODEL "models/projectiles/pipe/w_pipe.mdl"

//=============================================================================
// CFFProjectilePipebomb tables
//=============================================================================

#ifdef GAME_DLL
BEGIN_DATADESC(CFFProjectilePipebomb) 
END_DATADESC() 
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(FFProjectilePipebomb, DT_FFProjectilePipebomb) 

BEGIN_NETWORK_TABLE(CFFProjectilePipebomb, DT_FFProjectilePipebomb) 
END_NETWORK_TABLE() 

LINK_ENTITY_TO_CLASS(pipebomb, CFFProjectilePipebomb);
PRECACHE_WEAPON_REGISTER(pipebomb);

//=============================================================================
// CFFProjectilePipebomb implementation
//=============================================================================

#ifdef GAME_DLL
	//----------------------------------------------------------------------------
	// Purpose: Detonate the pipebomb(pOther = optional triggerer) 
	//----------------------------------------------------------------------------
	void CFFProjectilePipebomb::DetonatePipe(bool force, CBaseEntity *pOther) 
	{	
		// This is currently live
		if (!force && gpGlobals->curtime < m_flSpawnTime + PIPEBOMB_TIME_TILL_LIVE) 
			return;

		// Transfer ownership before exploding 
		//	eg. if an engineer dets these instead with emp
		if (pOther) 
			SetOwnerEntity(pOther);

		// Detonate!
		SetThink(&CFFProjectilePipebomb::Detonate);
		SetNextThink(gpGlobals->curtime);
	}
#endif

//----------------------------------------------------------------------------
// Purpose: Spawn like a normal grenade but replace skin
//----------------------------------------------------------------------------
void CFFProjectilePipebomb::Spawn() 
{
	BaseClass::Spawn();
	m_nSkin = 0;			// Green skin(#1) 

#ifdef CLIENT_DLL
	// Rebo you are quite mean with this tomfoolery!!!!!!!!!!
	player_info_t pinfo;
	engine->GetPlayerInfo(C_BasePlayer::GetLocalPlayer()->entindex(), &pinfo);

	fAltSkin = ! (pinfo.friendsID & 1);
#else

	// Start the armed light
	m_pArmedSprite = CSprite::SpriteCreate("sprites/redglow1.vmt", GetLocalOrigin(), false);

	int	nAttachment = LookupAttachment("glowsprite");

	if (m_pArmedSprite != NULL) 
	{
		m_pArmedSprite->FollowEntity(this);
		m_pArmedSprite->SetAttachment(this, nAttachment);
		m_pArmedSprite->SetTransparency(kRenderGlow, 255, 255, 255, 255, kRenderFxNoDissipation);
		m_pArmedSprite->SetScale(0.05f);
		m_pArmedSprite->SetGlowProxySize(4.0f);

		m_pArmedSprite->SetThink(&CBaseEntity::SUB_Remove);
		m_pArmedSprite->SetNextThink(gpGlobals->curtime + PIPEBOMB_TIME_TILL_LIVE);
	}
#endif	
}

#ifdef CLIENT_DLL
//----------------------------------------------------------------------------
// Purpose: Draw model with different skin
//----------------------------------------------------------------------------
int CFFProjectilePipebomb::DrawModel(int flags) 
{
	if (fAltSkin) 
		m_nSkin = 2;		// Yellow skin(#3) 

	return BaseClass::DrawModel(flags);
}
#endif

//----------------------------------------------------------------------------
// Purpose: Precache the pipebomb model
//----------------------------------------------------------------------------
void CFFProjectilePipebomb::Precache() 
{
	PrecacheModel(PIPEBOMB_MODEL);
	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Destroy all pipes belonging to a player
//----------------------------------------------------------------------------
void CFFProjectilePipebomb::DestroyAllPipes(CBaseEntity *pOwner, bool force) 
{
#ifdef GAME_DLL
	// Detonate all the pipes belonging to us
	CFFProjectilePipebomb *pPipe = NULL; 

	// Detonate any pipes belonging to us
	while ((pPipe = (CFFProjectilePipebomb *) gEntList.FindEntityByClassname(pPipe, "pipebomb")) != NULL) 
	{
		if (pPipe->GetOwnerEntity() == pOwner) 
			pPipe->DetonatePipe(force);
	}
#endif
}

//----------------------------------------------------------------------------
// Purpose: Create a new pipebomb
//----------------------------------------------------------------------------
CFFProjectilePipebomb * CFFProjectilePipebomb::CreatePipebomb(const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iSpeed) 
{
	CFFProjectilePipebomb *pPipebomb = (CFFProjectilePipebomb *) CreateEntityByName("pipebomb");

	UTIL_SetOrigin(pPipebomb, vecOrigin);
	pPipebomb->SetAbsAngles(angAngles);
	pPipebomb->Spawn();
	pPipebomb->SetOwnerEntity(pentOwner);

	Vector vecForward;
	AngleVectors(angAngles, &vecForward);

	// Set the speed and the initial transmitted velocity
	pPipebomb->SetAbsVelocity(vecForward * iSpeed);

#ifdef GAME_DLL
	pPipebomb->SetupInitialTransmittedVelocity(vecForward * iSpeed);
	
	pPipebomb->SetDetonateTimerLength(120);

	pPipebomb->SetElasticity(GetGrenadeElasticity());
#endif

	pPipebomb->m_flDamage = iDamage;
	pPipebomb->m_DmgRadius = pPipebomb->m_flDamage * 3.5f;

	pPipebomb->m_bIsLive = false;

	pPipebomb->SetThrower(pentOwner); 

	pPipebomb->SetGravity(GetGrenadeGravity());
	pPipebomb->SetFriction(GetGrenadeFriction());

	pPipebomb->SetLocalAngularVelocity(RandomAngle(-400, 400));

#ifdef GAME_DLL
	CFFProjectilePipebomb *pPipe = NULL, *pOldestPipe = NULL;
	int i = 0;

	// Make sure there aren't already too many pipes
	while ((pPipe = (CFFProjectilePipebomb *) gEntList.FindEntityByClassname(pPipe, "pipebomb")) != NULL) 
	{
		if (pPipe->GetOwnerEntity() == pPipebomb->GetOwnerEntity()) 
		{
			i++;

			if (!pOldestPipe) 
				pOldestPipe = pPipe;

			if (pPipe->m_flSpawnTime < pOldestPipe->m_flSpawnTime) 
				pOldestPipe = pPipe;
		}
	}

	// Too many pipes
	if (i > 8) 
		pOldestPipe->DetonatePipe();
#endif

	return pPipebomb; 
}
