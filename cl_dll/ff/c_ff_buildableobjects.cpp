// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildableobjects.cpp
// @author Patrick O'Leary (Mulchman)
// @date 06/08/2005
// @brief Client side BuildableObject classes:
//			Dispenser, Detpack, Sentry Gun, & Man Cannon
//
// REVISIONS
// ---------
//	12/28/2005, Mulchman: 
//		ClientSide Detpack first created
//
//	12/28/2005, Mulchman: 
//		ClientSide Dispenser first created
//
//	01/05/2005, Mulchman: 
//		ClientSide SentryGun first created
//
//	05/11/2005, Mulchman
//		Added the CreateClientSide stuff so that we spawn
//		a dispenser only on the client - mainly for the
//		dispenser weapon slot of the engineer
//
//	05/12/2005, Mulchman
//		Added the CreateClientSide stuff so that we spawn
//		a detpack only on the client - mainly for the
//		detpack weapon slot of the demoman
//
//	05/12/2005, Mulchman:
//		Added the CreateClientSide stuff so that we spawn
//		a sentrygun only on the client - mainly for the
//		sentrygun weapon slot of the engineer
//
//	06/08/2005, Mulchman:
//		Now derives from C_AI_BaseNPC for additional
//		(and easy) functionality
//
// 06/08/2005, Mulchman: 
//		This file First created - a container for all
//		ClientSide buildable object code
//
//	03/17/2006, Mulchman:
//		Removing aim sphere
//
//	05/10/2006, Mulchman:
//		Add radio tags to dispenser
//
//	12/6/2007, Mulchman:
//		Added man cannon stuff

#include "cbase.h"
#include "c_playerresource.h"
#include "ff_buildableobjects_shared.h"
#include "c_ff_timers.h"
#include "ff_gamerules.h"

#include "materialsystem/IMaterialSystem.h"
#include "materialsystem/IMesh.h"
#include "ClientEffectPrecacheSystem.h"

// For DrawSprite
#include "beamdraw.h"

// Defines
#define FF_BUILD_ERROR_NOROOM	"sprites/ff_build_noroom"
#define FF_BUILD_ERROR_TOOSTEEP	"sprites/ff_build_toosteep"
#define FF_BUILD_ERROR_TOOFAR	"sprites/ff_build_toofar"
#define FF_BUILD_ERROR_OFFGROUND	"sprites/ff_build_offground"
#define FF_BUILD_ERROR_MOVEABLE		"sprites/ff_build_moveable"
#define FF_BUILD_ERROR_NEEDAMMO		"sprites/ff_build_needammo"
#define FF_BUILD_ERROR_ALREADYBUILTSG	"sprites/ff_build_alreadybuiltsg"
#define FF_BUILD_ERROR_ALREADYBUILTDISP	"sprites/ff_build_alreadybuiltdisp"
#define FF_BUILD_ERROR_ALREADYBUILTMANCANNON	"sprites/ff_build_alreadybuiltmancannon"


// Define all the sprites to precache
CLIENTEFFECT_REGISTER_BEGIN( PrecacheBuildErrorNoRoom )
CLIENTEFFECT_MATERIAL( FF_BUILD_ERROR_NOROOM )
CLIENTEFFECT_REGISTER_END()

CLIENTEFFECT_REGISTER_BEGIN( PrecacheBuildErrorTooSteep )
CLIENTEFFECT_MATERIAL( FF_BUILD_ERROR_TOOSTEEP )
CLIENTEFFECT_REGISTER_END()

CLIENTEFFECT_REGISTER_BEGIN( PrecacheBuildErrorTooFar )
CLIENTEFFECT_MATERIAL( FF_BUILD_ERROR_TOOFAR )
CLIENTEFFECT_REGISTER_END()

CLIENTEFFECT_REGISTER_BEGIN( PrecacheBuildErrorOffGround )
CLIENTEFFECT_MATERIAL( FF_BUILD_ERROR_OFFGROUND )
CLIENTEFFECT_REGISTER_END()

CLIENTEFFECT_REGISTER_BEGIN( PrecacheBuildErrorMoveable )
CLIENTEFFECT_MATERIAL( FF_BUILD_ERROR_MOVEABLE )
CLIENTEFFECT_REGISTER_END()

//////////////////////////////////////////////////////////////////////////
// For ghost buildables.
//ConVar ffdev_pulsebuildable("ffdev_pulsebuildable", "0", FCVAR_FF_FFDEV_REPLICATED, "Buildable ghost slow pulse");
#define FFDEV_PULSEBUILDABLE false
//ConVar ffdev_buildabledrawonerror("ffdev_buildabledrawonerror", "1", FCVAR_FF_FFDEV_REPLICATED, "Draw the buildable when it can't be built");
#define FFDEV_BUILDABLEDRAWONERROR true
const RenderFx_t g_BuildableRenderFx = kRenderFxPulseSlowWide;
//////////////////////////////////////////////////////////////////////////

//extern ConVar ffdev_mancannon_combatcooldown;
#define MANCANNON_COMBATCOOLDOWN 3.0f

//=============================================================================
//
//	class C_FFBuildableObject
//
//=============================================================================

#if defined( CFFBuildableObject )
	#undef CFFBuildableObject
#endif

IMPLEMENT_CLIENTCLASS_DT( C_FFBuildableObject, DT_FFBuildableObject, CFFBuildableObject )
	RecvPropEHandle( RECVINFO( m_hOwner ) ),
	RecvPropInt( RECVINFO( m_iHealth ) ),
	RecvPropInt( RECVINFO( m_iMaxHealth ) ),
	RecvPropBool( RECVINFO( m_bBuilt ) ),
	RecvPropFloat( RECVINFO( m_flSabotageTime ) ),
	RecvPropInt( RECVINFO( m_iSaboteurTeamNumber ) ),
END_RECV_TABLE( )

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_FFBuildableObject::C_FFBuildableObject( void )
{
	// Initialize
	m_bClientSideOnly = false;
	m_flSabotageTime = 0.0f;
	m_iSaboteurTeamNumber = TEAM_UNASSIGNED;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
C_FFBuildableObject::~C_FFBuildableObject( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFBuildableObject::OnDataChanged( DataUpdateType_t updateType )
{
	// NOTE: We MUST call the base classes' implementation of this function
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: To smooth the build helpers
//-----------------------------------------------------------------------------
void C_FFBuildableObject::ClientThink( void )
{
	// This is to "smooth" the build helper models
	if( m_bClientSideOnly )
	{
		C_FFPlayer *pPlayer = ToFFPlayer( m_hOwner.Get() );
		if( !pPlayer )
			return;

		float flBuildDist = 0.0f;

		switch( Classify() )
		{
			case CLASS_DISPENSER:
				flBuildDist = FF_BUILD_DISP_BUILD_DIST;
			break;

			case CLASS_SENTRYGUN:
				flBuildDist = FF_BUILD_SG_BUILD_DIST;
			break;

			case CLASS_DETPACK:
				flBuildDist = FF_BUILD_DET_BUILD_DIST;
			break;

			case CLASS_MANCANNON:
				flBuildDist = FF_BUILD_MC_BUILD_DIST;
			break;

			default: return; break;
		}

		Vector vecForward;
		pPlayer->EyeVectors( &vecForward );
		vecForward.z = 0.0f;
		VectorNormalize( vecForward );

		// Need to save off the z value before setting new origin
		Vector vecOrigin = GetAbsOrigin();

		// Compute a new origin in front of the player
		Vector vecNewOrigin = pPlayer->GetAbsOrigin() + ( vecForward * ( flBuildDist + 16.0f ) );
		vecNewOrigin.z = vecOrigin.z;

		SetAbsOrigin( vecNewOrigin );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Two pass so that the player icons can be drawn
//-----------------------------------------------------------------------------
RenderGroup_t C_FFBuildableObject::GetRenderGroup()
{
	if ( m_flSabotageTime > 0.0f )
		return RENDER_GROUP_TWOPASS;

	return BaseClass::GetRenderGroup();
}

//-----------------------------------------------------------------------------
// Purpose: Using this to draw any "can't build" type glyphs
//-----------------------------------------------------------------------------
int C_FFBuildableObject::DrawModel( int flags )
{
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	// render a spy icon during the transparency pass
	if ( flags & STUDIO_TRANSPARENCY && pPlayer && !pPlayer->IsObserver() && m_flSabotageTime > 0.0f )
	{
		if( FFGameRules()->IsTeam1AlliedToTeam2( pPlayer->GetTeamNumber(), m_iSaboteurTeamNumber ) == GR_TEAMMATE )
		{
			// Thanks mirv!
			IMaterial *pMaterial = materials->FindMaterial( "sprites/ff_sprite_spy", TEXTURE_GROUP_CLIENT_EFFECTS );
			if( pMaterial )
			{
				materials->Bind( pMaterial );

				// The color is based on the saboteur's team
				Color clr = Color( 255, 255, 255, 255 );

				if( g_PR )
				{
					float flSabotageTime = clamp( m_flSabotageTime - gpGlobals->curtime, 0, FF_BUILD_SABOTAGE_TIMEOUT );
					int iAlpha = 64 + (191 * (flSabotageTime / FF_BUILD_SABOTAGE_TIMEOUT) );
					clr.SetColor( g_PR->GetTeamColor( m_iSaboteurTeamNumber ).r(), g_PR->GetTeamColor( m_iSaboteurTeamNumber ).g(), g_PR->GetTeamColor( m_iSaboteurTeamNumber ).b(), iAlpha );
				}

				color32 c = { clr.r(), clr.g(), clr.b(), clr.a() };
				DrawSprite( Vector( GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z + 64.0f ), 15.0f, 15.0f, c );
			}
		}
	}

	if( m_bClientSideOnly )
	{
		// Draw our glyphs

		// See if there's even an error
		if( m_hBuildError != BUILD_ALLOWED )
		{
			float flOffset = 0.0f;
			
			// Get an offset for drawing (relative to GetAbsOrigin)
			const int iEntityClass = Classify();
			switch( iEntityClass )
			{
				case CLASS_DISPENSER: flOffset = 32.0f; break;
				case CLASS_SENTRYGUN: flOffset = 32.0f; break;
				case CLASS_DETPACK: flOffset = 0.0f; break;
				case CLASS_MANCANNON: flOffset = 0.0f; break;
				default: return BaseClass::DrawModel( flags ); break;
			}

			// Find out which error we're showing
			const char *pszMaterial = NULL;
			switch( m_hBuildError )
			{
				case BUILD_NOROOM: pszMaterial = FF_BUILD_ERROR_NOROOM; break;
				case BUILD_TOOSTEEP: pszMaterial = FF_BUILD_ERROR_TOOSTEEP; break;
				case BUILD_TOOFAR: pszMaterial = FF_BUILD_ERROR_TOOFAR; break;
				case BUILD_PLAYEROFFGROUND: pszMaterial = FF_BUILD_ERROR_OFFGROUND; break;
				case BUILD_MOVEABLE: pszMaterial = FF_BUILD_ERROR_MOVEABLE; break;
				case BUILD_NEEDAMMO: pszMaterial = FF_BUILD_ERROR_NEEDAMMO; break;
				case BUILD_ALREADYBUILT:
				{				
					if(iEntityClass == CLASS_DISPENSER)
						pszMaterial = FF_BUILD_ERROR_ALREADYBUILTDISP; 
					else if(iEntityClass == CLASS_SENTRYGUN)
						pszMaterial = FF_BUILD_ERROR_ALREADYBUILTSG;
					else if( iEntityClass == CLASS_MANCANNON )
						pszMaterial = FF_BUILD_ERROR_ALREADYBUILTMANCANNON;
				}
				break;
			}

			// If a valid material...
			if( pszMaterial )
			{
				// Draw!
				IMaterial *pMaterial = materials->FindMaterial( pszMaterial, TEXTURE_GROUP_OTHER );
				if( pMaterial )
				{
					materials->Bind( pMaterial );
					color32 c = { 255, 255, 255, 255 };
					DrawSprite( Vector( GetAbsOrigin().x, GetAbsOrigin().y, EyePosition().z + flOffset ), 15.0f, 15.0f, c );
				}
			}

			// This just looks bad, even if it is a cvar. 
			// We're already drawing the sprite noculled.
			// UNDONE:
			// Finally, there was a build error, so don't actually draw the real model!
			//if(!ffdev_buildabledrawonerror.GetBool())
			return 0;
		}
	}
		
	
	return BaseClass::DrawModel( flags );
}

//=============================================================================
//
//	class C_FFDetpack
//
//=============================================================================

#if defined( CFFDetpack )
	#undef CFFDetpack
#endif

IMPLEMENT_CLIENTCLASS_DT( C_FFDetpack, DT_FFDetpack, CFFDetpack )
END_RECV_TABLE( )


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_FFDetpack::C_FFDetpack( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
C_FFDetpack::~C_FFDetpack( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFDetpack::OnDataChanged( DataUpdateType_t updateType )
{
	// NOTE: We MUST call the base classes' implementation of this function
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Creates a client side entity using the detpack model
//-----------------------------------------------------------------------------
C_FFDetpack *C_FFDetpack::CreateClientSideDetpack( const Vector& vecOrigin, const QAngle& vecAngles )
{
	C_FFDetpack *pDetpack = new C_FFDetpack;

	if( !pDetpack )
		return NULL;

	if( !pDetpack->InitializeAsClientEntity( FF_DETPACK_MODEL, RENDER_GROUP_TRANSLUCENT_ENTITY ) )
	{
		pDetpack->Release( );

		return NULL;
	}

	pDetpack->SetAbsOrigin( vecOrigin );
	pDetpack->SetLocalAngles( vecAngles );
	pDetpack->SetCollisionGroup( COLLISION_GROUP_DEBRIS );
	pDetpack->SetRenderMode( kRenderTransAlpha );
	pDetpack->SetRenderColorA( ( byte )110 );

	if(FFDEV_PULSEBUILDABLE)
		pDetpack->m_nRenderFX = g_BuildableRenderFx;

	//kRenderTransAlphaAdd
	//kRender

	// Since this is client side only, give it an owner just in case
	// someone accesses the m_hOwner.Get() and wants to return something
	// that isn't NULL!
	pDetpack->m_hOwner = ( C_BaseEntity * )C_BasePlayer::GetLocalPlayer();
	pDetpack->SetClientSideOnly( true );
	pDetpack->SetNextClientThink( CLIENT_THINK_ALWAYS );

	return pDetpack;
}

//=============================================================================
//
//	class C_FFDispenser
//
//=============================================================================

#if defined( CFFDispenser )
	#undef CFFDispenser
#endif

IMPLEMENT_CLIENTCLASS_DT( C_FFDispenser, DT_FFDispenser, CFFDispenser )
	RecvPropInt( RECVINFO( m_iAmmoPercent ) ),
	RecvPropInt( RECVINFO( m_iCells ) ),
	RecvPropInt( RECVINFO( m_iShells ) ),
	RecvPropInt( RECVINFO( m_iNails ) ),
	RecvPropInt( RECVINFO( m_iRockets ) ),
	RecvPropInt( RECVINFO( m_iArmor ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFDispenser::OnDataChanged( DataUpdateType_t updateType )
{
	// NOTE: We MUST call the base classes' implementation of this function
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Creates a client side entity using the dispenser model
//-----------------------------------------------------------------------------
C_FFDispenser *C_FFDispenser::CreateClientSideDispenser( const Vector& vecOrigin, const QAngle& vecAngles )
{
	C_FFDispenser *pDispenser = new C_FFDispenser;

	if( !pDispenser )
		return NULL;

	if( !pDispenser->InitializeAsClientEntity( FF_DISPENSER_MODEL, RENDER_GROUP_TRANSLUCENT_ENTITY ) )
	{
		pDispenser->Release( );

		return NULL;
	}

	pDispenser->SetAbsOrigin( vecOrigin );
	pDispenser->SetLocalAngles( vecAngles );
	pDispenser->SetCollisionGroup( COLLISION_GROUP_DEBRIS );
	pDispenser->SetRenderMode( kRenderTransAlpha );
	pDispenser->SetRenderColorA( ( byte )110 );
	
	if(FFDEV_PULSEBUILDABLE)
		pDispenser->m_nRenderFX = g_BuildableRenderFx;

	// Since this is client side only, give it an owner just in case
	// someone accesses the m_hOwner.Get() and wants to return something
	// that isn't NULL!
	pDispenser->m_hOwner = ( C_BaseEntity * )C_BasePlayer::GetLocalPlayer();
	pDispenser->m_nSkin = clamp(CBasePlayer::GetLocalPlayer()->GetTeamNumber() + 1 - TEAM_BLUE, 0, 4); // dispenser skin 0 is neutral
	pDispenser->SetClientSideOnly( true );
	pDispenser->SetNextClientThink( CLIENT_THINK_ALWAYS );

	return pDispenser;
}

//=============================================================================
//
//	class C_FFSentryGun
//
//=============================================================================

#if defined( CFFSentryGun )
	#undef CFFSentryGun
#endif

IMPLEMENT_CLIENTCLASS_DT( C_FFSentryGun, DT_FFSentryGun, CFFSentryGun )
	RecvPropInt( RECVINFO( m_iAmmoPercent ) ),
	//RecvPropFloat( RECVINFO( m_flRange ) ),
	RecvPropInt( RECVINFO( m_iLevel ) ),
	RecvPropInt( RECVINFO( m_iShells ) ),
	RecvPropInt( RECVINFO( m_iRockets ) ),
	RecvPropInt( RECVINFO( m_iMaxShells ) ),
	RecvPropInt( RECVINFO( m_iMaxRockets ) ),
	RecvPropInt( RECVINFO( m_iUpgradeProgress ) ),
END_RECV_TABLE( )

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_FFSentryGun::C_FFSentryGun( void )
{
	m_iLocalHallucinationIndex = -1;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
C_FFSentryGun::~C_FFSentryGun( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFSentryGun::OnDataChanged( DataUpdateType_t updateType )
{
	// NOTE: We MUST call the base classes' implementation of this function
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_FFSentryGun::CanUpgrade()
{
	return IsBuilt() && GetLevel() < 3 && NeedsHealth() == 0;
}

bool C_FFSentryGun::CanBeUpgradedBy(C_FFPlayer *pPlayer) 
{
	if (!CanUpgrade())
		return false;

	return pPlayer->GetAmmoCount(AMMO_CELLS) >= FF_BUILDCOST_UPGRADE_SENTRYGUN;
}

void C_FFSentryGun::Upgrade()
{
}

//-----------------------------------------------------------------------------
// Purpose: Creates a client side entity using the sentrygun model
//-----------------------------------------------------------------------------
C_FFSentryGun *C_FFSentryGun::CreateClientSideSentryGun( const Vector& vecOrigin, const QAngle& vecAngles )
{
	C_FFSentryGun *pSentryGun = new C_FFSentryGun;

	if( !pSentryGun )
		return NULL;

	if( !pSentryGun->InitializeAsClientEntity( FF_SENTRYGUN_MODEL, RENDER_GROUP_TRANSLUCENT_ENTITY ) )
	{
		pSentryGun->Release( );

		return NULL;
	}

	pSentryGun->SetAbsOrigin( vecOrigin );
	pSentryGun->SetLocalAngles( vecAngles );
	pSentryGun->SetCollisionGroup( COLLISION_GROUP_DEBRIS );
	pSentryGun->SetRenderMode( kRenderTransAlpha );
	pSentryGun->SetRenderColorA( ( byte )110 );
	
	if(FFDEV_PULSEBUILDABLE)
		pSentryGun->m_nRenderFX = g_BuildableRenderFx;
	
	// Since this is client side only, give it an owner just in case
	// someone accesses the m_hOwner.Get() and wants to return something
	// that isn't NULL!
	pSentryGun->m_hOwner = ( C_BaseEntity * )C_BasePlayer::GetLocalPlayer();

	// Mirv: Show up as the correct skin
	pSentryGun->m_nSkin = clamp(CBasePlayer::GetLocalPlayer()->GetTeamNumber() - TEAM_BLUE, 0, 3);
	pSentryGun->SetClientSideOnly( true );
	pSentryGun->SetNextClientThink( CLIENT_THINK_ALWAYS );

	return pSentryGun;
}

//-------------------------------------------------------------------------
// Purpose: Sentryguns will sometimes appear as the wrong model when
//			the local player is hallucinating
//-------------------------------------------------------------------------
int C_FFSentryGun::DrawModel(int flags)
{
	int nRet = BaseClass::DrawModel(flags);

	if (m_iLevel <= 0)
		return nRet;

	C_FFPlayer *pLocalPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());

	// No hallucinations here. But check first if we have to reset
	if (!pLocalPlayer || !pLocalPlayer->m_iHallucinationIndex)
	{
		if (m_iLocalHallucinationIndex >= 0)
		{
			switch (m_iLevel)
			{
			case 1:
				SetModel(FF_SENTRYGUN_MODEL);
				break;
			case 2:
				SetModel(FF_SENTRYGUN_MODEL_LVL2);
				break;
			case 3:
				SetModel(FF_SENTRYGUN_MODEL_LVL3);
				break;
			}

			m_iLocalHallucinationIndex = -1;
		}

		return nRet;
	}

	int nNewLevel = entindex() + pLocalPlayer->m_iHallucinationIndex;

	nNewLevel = nNewLevel % 3; // ouch

	if (m_iLocalHallucinationIndex == nNewLevel)
		return nRet;

	switch (nNewLevel)
	{
	case 0:
		SetModel(FF_SENTRYGUN_MODEL);
		break;
	case 1:
		SetModel(FF_SENTRYGUN_MODEL_LVL2);
		break;
	case 2:
		SetModel(FF_SENTRYGUN_MODEL_LVL3);
		break;
	}

	m_iLocalHallucinationIndex = nNewLevel;

	return nRet;
}

//=============================================================================
//
//	class C_FFManCannon
//
//=============================================================================

#if defined( CFFManCannon )
	#undef CFFManCannon
#endif

IMPLEMENT_CLIENTCLASS_DT( C_FFManCannon, DT_FFManCannon, CFFManCannon )
	RecvPropFloat( RECVINFO( m_flLastDamage ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFManCannon::OnDataChanged( DataUpdateType_t updateType )
{
	// NOTE: We MUST call the base classes' implementation of this function
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Creates a client side entity using the man cannon model
//-----------------------------------------------------------------------------
C_FFManCannon *C_FFManCannon::CreateClientSideManCannon( const Vector& vecOrigin, const QAngle& vecAngles )
{
	C_FFManCannon *pManCannon = new C_FFManCannon;

	if( !pManCannon )
		return NULL;

	if( !pManCannon->InitializeAsClientEntity( FF_MANCANNON_MODEL, RENDER_GROUP_TRANSLUCENT_ENTITY ) )
	{
		pManCannon->Release();

		return NULL;
	}

	pManCannon->SetAbsOrigin( vecOrigin );
	pManCannon->SetLocalAngles( vecAngles );
	pManCannon->SetCollisionGroup( COLLISION_GROUP_DEBRIS );
	pManCannon->SetRenderMode( kRenderTransAlpha );
	pManCannon->SetRenderColorA( ( byte )110 );
	
	if( FFDEV_PULSEBUILDABLE )
		pManCannon->m_nRenderFX = g_BuildableRenderFx;

	// Since this is client side only, give it an owner just in case
	// someone accesses the m_hOwner.Get() and wants to return something
	// that isn't NULL!
	pManCannon->m_hOwner = (C_BaseEntity *)C_BasePlayer::GetLocalPlayer();
	//Team Coloring -GreenMushy
	// slightly modified by Dexter to use the member just set.. :)	
	pManCannon->m_nSkin = ( pManCannon->m_hOwner->GetTeamNumber() - 1 );
	pManCannon->SetClientSideOnly( true );
	pManCannon->SetNextClientThink( CLIENT_THINK_ALWAYS );

	return pManCannon;
}

//-------------------------------------------------------------------------
// Purpose: Sentryguns will sometimes appear as the wrong model when
//			the local player is hallucinating
//-------------------------------------------------------------------------
int C_FFManCannon::DrawModel(int flags)
{
	int nRet = BaseClass::DrawModel(flags);
	
	if( gpGlobals->curtime < m_flLastDamage + MANCANNON_COMBATCOOLDOWN )
	{
		// Thanks mirv!
		IMaterial *pMaterial = materials->FindMaterial( "sprites/ff_sprite_combat", TEXTURE_GROUP_CLIENT_EFFECTS );
		if( pMaterial )
		{
			materials->Bind( pMaterial );

			// The color is based on the owner's team
			Color clr = Color( 255, 255, 255, 255 );

			if( g_PR )
			{
				int teamnumber = GetTeamNumber();
				float flCombatTime = clamp( gpGlobals->curtime - m_flLastDamage, 0, MANCANNON_COMBATCOOLDOWN );
				int iAlpha = (255 * ( 1.0f - (flCombatTime / MANCANNON_COMBATCOOLDOWN) ) );
				clr.SetColor( g_PR->GetTeamColor( teamnumber ).r(), g_PR->GetTeamColor( teamnumber ).g(), g_PR->GetTeamColor( teamnumber ).b(), iAlpha );
			}

			color32 c = { clr.r(), clr.g(), clr.b(), clr.a() };
			DrawSprite( Vector( GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z + 48.0f ), 32.0f, 32.0f, c );
		}
	}

	return nRet;
}