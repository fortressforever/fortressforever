/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file ff_modelglyph.cpp
/// @author Kevin Hjelden (FryGuy)
/// @date 29 Dec 2005
/// @brief Entity for displaying glyphs above player's heads (saveme!)
/// 
/// Revisions
/// ---------
/// 29 Dec 2005: Initial Creation

#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"

#if defined( CLIENT_DLL )
	#define CFFModelGlyph C_FFModelGlyph
	#include "c_ff_player.h"
#else
	#include "ff_player.h"
#endif

#define GLYPH_MODEL "models/misc/saveme.mdl"
#define GLYPH_DELAY 10.0f

//=============================================================================
// CFFWeaponAssaultCannon
//=============================================================================

class CFFModelGlyph : public CBaseAnimating
{
public:
	DECLARE_CLASS( CFFModelGlyph, CBaseAnimating );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	#ifdef GAME_DLL
		DECLARE_DATADESC();
	#endif

	CFFModelGlyph();

	void Spawn		( void );
	void Precache	( void );
	void OnThink( void );
private:

};

//=============================================================================
// CFFModelGlyph tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED( FFModelGlyph, DT_FFModelGlyph )

BEGIN_NETWORK_TABLE( CFFModelGlyph, DT_FFModelGlyph )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CFFModelGlyph )
END_PREDICTION_DATA()

#ifdef GAME_DLL
BEGIN_DATADESC( CFFModelGlyph )
	DEFINE_THINKFUNC( OnThink ),
END_DATADESC();
#endif

LINK_ENTITY_TO_CLASS( ff_modelglyph, CFFModelGlyph );
PRECACHE_REGISTER( ff_modelglyph );

//=============================================================================
// CFFModelGlyph implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFModelGlyph::CFFModelGlyph()
{
}

//----------------------------------------------------------------------------
// Purpose: Precache
//----------------------------------------------------------------------------
void CFFModelGlyph::Precache( void )
{
	PrecacheModel( GLYPH_MODEL );
	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Spawn
//----------------------------------------------------------------------------
void CFFModelGlyph::Spawn( void )
{
	Precache();

	SetModel( GLYPH_MODEL );

#ifndef GAME_DLL
	if ( C_BasePlayer::GetLocalPlayer() == NULL )
	{
		//AssertMsg( 0, "GetLocalPlayer is null");
		return;
	}

	if ( GetOwnerEntity() == NULL )
	{
		//AssertMsg( 0, "GetOwnerEntity is null");
		return;
	}

	CFFPlayer *me = ToFFPlayer(( C_BaseEntity * )C_BasePlayer::GetLocalPlayer());
	CFFPlayer *owner = ToFFPlayer(GetOwnerEntity());

	// hide from player if not the right team
	if (me && owner)
	{
		DevMsg("Setting nodraw flag: %d\n", me->GetTeamNumber() != owner->GetTeamNumber());
		if (me->GetTeamNumber() != owner->GetTeamNumber())
		{
			AddEffects( EF_NODRAW );
		}
	}
#endif

	SetRenderMode(kRenderGlow);
	SetRenderColor(255, 255, 255);
	//SetTransparency( , 255, 255, 255, 255, kRenderFxFlickerFast );

	SetThink ( &CFFModelGlyph::OnThink );
	SetNextThink( gpGlobals->curtime + GLYPH_DELAY );
}

//----------------------------------------------------------------------------
// Purpose: Remove the glyph after its time has expired
//----------------------------------------------------------------------------
void CFFModelGlyph::OnThink( void )
{
	DevMsg("Removing glyph!\n");
	if ( GetOwnerEntity() == NULL )
		Warning( "Owner entity still null!\n" );
	Remove();
}
