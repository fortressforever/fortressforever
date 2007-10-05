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

#ifdef GAME_DLL
	#include "ff_utils.h"
	#include "soundent.h"	
#endif

extern short	g_sModelIndexFireball;		// (in combatweapon.cpp) holds the index for the fireball 
extern short	g_sModelIndexWExplosion;	// (in combatweapon.cpp) holds the index for the underwater explosion
extern short	g_sModelIndexSmoke;			// (in combatweapon.cpp) holds the index for the smoke cloud

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

LINK_ENTITY_TO_CLASS(ff_projectile_pl, CFFProjectilePipebomb);
PRECACHE_WEAPON_REGISTER(ff_projectile_pl);

//0001279: Need convar for pipe det delay
ConVar pipebomb_time_till_live("ffdev_pipedetdelay", "0.55", FCVAR_REPLICATED | FCVAR_CHEAT);

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
		//if (!force && gpGlobals->curtime < m_flSpawnTime + pipebomb_time_till_live.GetFloat()) 
		//	return;

		// Transfer ownership before exploding 
		//	eg. if an engineer dets these instead with emp
		if (pOther)
		{
			SetThrower((CBaseCombatCharacter *)pOther);
			SetOwnerEntity(pOther);
		}

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

	m_flSpawnTime = gpGlobals->curtime;

#ifdef CLIENT_DLL
	// Rebo you are quite mean with this tomfoolery!!!!!!!!!!
	player_info_t pinfo;

	if (CBasePlayer::GetLocalPlayer())
	{
		engine->GetPlayerInfo(C_BasePlayer::GetLocalPlayer()->entindex(), &pinfo);
		fAltSkin = ! (pinfo.friendsID & 1);
	}
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
		m_pArmedSprite->SetNextThink(gpGlobals->curtime + pipebomb_time_till_live.GetFloat());
	}
#endif	
}

// Added so that grenades aren't using projectiles explode code.
// Grenades might need to look in more places than just below
// them to see if scorch marks can be drawn.
void CFFProjectilePipebomb::Explode( trace_t *pTrace, int bitsDamageType )
{
#ifdef GAME_DLL
	SetModelName( NULL_STRING );//invisible
	AddSolidFlags( FSOLID_NOT_SOLID );

	m_takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if( pTrace->fraction != 1.0 )
		SetLocalOrigin( pTrace->endpos + ( pTrace->plane.normal * 32 ) );

	Vector vecAbsOrigin = GetAbsOrigin();
	int contents = UTIL_PointContents( vecAbsOrigin );

	if( pTrace->fraction != 1.0 ) 
	{
		Vector vecNormal = pTrace->plane.normal;
		surfacedata_t *pdata = physprops->GetSurfaceData( pTrace->surface.surfaceProps );	
		CPASFilter filter( vecAbsOrigin );
		te->Explosion( filter, -1.0, // don't apply cl_interp delay
			&vecAbsOrigin, 
			! ( contents & MASK_WATER ) ? g_sModelIndexFireball : g_sModelIndexWExplosion, 
			m_flDamage / 128, 
			25, 
			TE_EXPLFLAG_NONE, 
			m_DmgRadius, 
			m_flDamage, 
			&vecNormal, 
			( char )pdata->game.material );

		// Normal decals since trace hit something
		UTIL_DecalTrace( pTrace, "Scorch" );
	}
	else
	{
		CPASFilter filter( vecAbsOrigin );
		te->Explosion( filter, -1.0, // don't apply cl_interp delay
			&vecAbsOrigin, 
			! ( contents & MASK_WATER ) ? g_sModelIndexFireball : g_sModelIndexWExplosion, 
			m_flDamage / 128, 
			25, 
			TE_EXPLFLAG_NONE, 
			m_DmgRadius, 
			m_flDamage );

		// Trace hit nothing so do custom scorch mark finding
		FF_DecalTrace( this, FF_DECALTRACE_TRACE_DIST, "Scorch" );
	}

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );

	CBaseEntity *pThrower = GetThrower();
	// Use the grenade's position as the reported position
	Vector vecReported = pTrace->endpos;
	CTakeDamageInfo info( this, pThrower, GetBlastForce(), GetAbsOrigin(), m_flDamage, bitsDamageType, 0, &vecReported );
	RadiusDamage( info, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );

	EmitSound( "BaseGrenade.Explode" );

	SetThink( &CBaseGrenade::SUB_Remove );
	SetTouch( NULL );

	AddEffects( EF_NODRAW );
	SetAbsVelocity( vec3_origin );
	SetNextThink( gpGlobals->curtime );
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
// Purpose: Destroy all pipes belonging to a player
//----------------------------------------------------------------------------
void CFFProjectilePipebomb::DestroyAllPipes(CBaseEntity *pOwner, bool force) 
{
#ifdef GAME_DLL
	// Detonate all the pipes belonging to us
	CFFProjectilePipebomb *pPipe = NULL; 

	// Detonate any pipes belonging to us
	while ((pPipe = (CFFProjectilePipebomb *) gEntList.FindEntityByClassT(pPipe, CLASS_PIPEBOMB)) != NULL) 
	{
		if (pPipe->GetOwnerEntity() == pOwner) 
			pPipe->DetonatePipe(force);
	}
#endif
}

//----------------------------------------------------------------------------
// Purpose: Create a new pipebomb
//----------------------------------------------------------------------------
CFFProjectilePipebomb * CFFProjectilePipebomb::CreatePipebomb(const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iDamageRadius, const int iSpeed) 
{
	CFFProjectilePipebomb *pPipebomb = (CFFProjectilePipebomb *) CreateEntityByName("ff_projectile_pl");

	if( !pPipebomb )
		return NULL;

	UTIL_SetOrigin(pPipebomb, vecOrigin);
	pPipebomb->SetAbsAngles(angAngles);
	pPipebomb->Spawn();
	pPipebomb->SetOwnerEntity(pentOwner);
	pPipebomb->m_iSourceClassname = (pSource ? pSource->m_iClassname : NULL_STRING);

	Vector vecForward;
	AngleVectors(angAngles, &vecForward);

	// Set the speed and the initial transmitted velocity
	pPipebomb->SetAbsVelocity(vecForward * iSpeed);

#ifdef GAME_DLL
	pPipebomb->SetupInitialTransmittedVelocity(vecForward * iSpeed);
	
	pPipebomb->SetDetonateTimerLength(120);

	pPipebomb->SetElasticity(GetGrenadeElasticity());
#endif

	pPipebomb->SetDamage(iDamage);
	pPipebomb->SetDamageRadius(iDamageRadius);

	pPipebomb->m_bIsLive = false;

	pPipebomb->SetThrower(pentOwner); 

	pPipebomb->SetGravity(GetGrenadeGravity());
	pPipebomb->SetFriction(GetGrenadeFriction());

	pPipebomb->SetLocalAngularVelocity(RandomAngle(-400, 400));

#ifdef GAME_DLL
	CFFProjectilePipebomb *pPipe = NULL, *pOldestPipe = NULL;
	int i = 0;

	// Make sure there aren't already too many pipes
	while ((pPipe = (CFFProjectilePipebomb *) gEntList.FindEntityByClassT(pPipe, CLASS_PIPEBOMB)) != NULL) 
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
