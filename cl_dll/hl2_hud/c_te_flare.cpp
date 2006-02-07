//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Flare effects
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "ClientEffectPrecacheSystem.h"
#include "particles_simple.h"
#include "iefx.h"
#include "dlight.h"
#include "view.h"
#include "fx.h"
#include "clientsideeffects.h"
#include "c_pixel_visibility.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//Precahce the effects
CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectFlares )
CLIENTEFFECT_MATERIAL( "effects/redflare" )
CLIENTEFFECT_MATERIAL( "effects/yellowflare" )
CLIENTEFFECT_MATERIAL( "effects/yellowflare_noz" )
CLIENTEFFECT_REGISTER_END()

class C_Flare : public C_BaseCombatCharacter, CSimpleEmitter
{
public:
	DECLARE_CLASS( C_Flare, C_BaseCombatCharacter );
	DECLARE_CLIENTCLASS();

	C_Flare();

	void	OnDataChanged( DataUpdateType_t updateType );
	void	Update( float timeDelta );
	void	NotifyDestroyParticle( Particle* pParticle );
	void	NotifyShouldTransmit( ShouldTransmitState_t state );
	void	RestoreResources( void );

	float	m_flTimeBurnOut;
	float	m_flScale;
	bool	m_bLight;
	bool	m_bSmoke;
	pixelvis_handle_t m_queryHandle;


private:
	C_Flare( const C_Flare & );

	SimpleParticle	*m_pParticle[2];
};

IMPLEMENT_CLIENTCLASS_DT( C_Flare, DT_Flare, CFlare )
	RecvPropFloat( RECVINFO( m_flTimeBurnOut ) ),
	RecvPropFloat( RECVINFO( m_flScale ) ),
	RecvPropInt( RECVINFO( m_bLight ) ),
	RecvPropInt( RECVINFO( m_bSmoke ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
C_Flare::C_Flare() : CSimpleEmitter( "C_Flare" )
{
	m_pParticle[0]	= NULL;
	m_pParticle[1]	= NULL;
	m_flTimeBurnOut	= 0.0f;

	m_bLight		= true;
	m_bSmoke		= true;

	SetDynamicallyAllocated( false );
	m_queryHandle = 0;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : state - 
//-----------------------------------------------------------------------------
void C_Flare::NotifyShouldTransmit( ShouldTransmitState_t state )
{
	if ( state == SHOULDTRANSMIT_END )
	{
		AddEffects( EF_NODRAW );
	}
	else if ( state == SHOULDTRANSMIT_START )
	{
		RemoveEffects( EF_NODRAW );
	}

	BaseClass::NotifyShouldTransmit( state );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_Flare::OnDataChanged( DataUpdateType_t updateType )
{
	if ( updateType == DATA_UPDATE_CREATED )
	{
		SetSortOrigin( GetAbsOrigin() );
	}

	BaseClass::OnDataChanged( updateType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Flare::RestoreResources( void )
{
	if ( m_pParticle[0] == NULL )
	{
		m_pParticle[0] = (SimpleParticle *) AddParticle( sizeof( SimpleParticle ), GetPMaterial( "effects/redflare" ), GetAbsOrigin() );
		
		if ( m_pParticle[0] != NULL )
		{
			m_pParticle[0]->m_uchColor[0] = m_pParticle[0]->m_uchColor[1] = m_pParticle[0]->m_uchColor[2] = 0;
			m_pParticle[0]->m_flRoll		= random->RandomInt( 0, 360 );
			m_pParticle[0]->m_flRollDelta	= random->RandomFloat( 1.0f, 4.0f );
			m_pParticle[0]->m_flLifetime	= 0.0f;
			m_pParticle[0]->m_flDieTime		= 10.0f;
		}
		else
		{
			Assert(0);
		}
	}

	if ( m_pParticle[1] == NULL )
	{
		m_pParticle[1] = (SimpleParticle *) AddParticle( sizeof( SimpleParticle ), GetPMaterial( "effects/yellowflare_noz" ), GetAbsOrigin() );
		
		if ( m_pParticle[1] != NULL )
		{
			m_pParticle[1]->m_uchColor[0] = m_pParticle[1]->m_uchColor[1] = m_pParticle[1]->m_uchColor[2] = 0;
			m_pParticle[1]->m_flRoll		= random->RandomInt( 0, 360 );
			m_pParticle[1]->m_flRollDelta	= random->RandomFloat( 1.0f, 4.0f );
			m_pParticle[1]->m_flLifetime	= 0.0f;
			m_pParticle[1]->m_flDieTime		= 10.0f;
		}
		else
		{
			Assert(0);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pParticle - 
//-----------------------------------------------------------------------------
void C_Flare::NotifyDestroyParticle( Particle* pParticle )
{
	if ( pParticle == m_pParticle[0] )
	{
		m_pParticle[0] = NULL;
	}

	if ( pParticle == m_pParticle[1] )
	{
		m_pParticle[1] = NULL;
	}

	CSimpleEmitter::NotifyDestroyParticle( pParticle );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : timeDelta - 
//-----------------------------------------------------------------------------
void C_Flare::Update( float timeDelta )
{
	if ( !IsVisible() )
		return;

	CSimpleEmitter::Update( timeDelta );

	//Make sure our stored resources are up to date
	RestoreResources();

	//Don't do this if the console is down
	if ( timeDelta <= 0.0f )
		return;

	//Check for LOS
	pixelvis_queryparams_t params;
	params.Init(GetAbsOrigin());
	
	float visible = PixelVisibility_FractionVisible( params, &m_queryHandle );

	float	fColor;
	float	baseScale = m_flScale;

	//Account for fading out
	if ( ( m_flTimeBurnOut != -1.0f ) && ( ( m_flTimeBurnOut - gpGlobals->curtime ) <= 10.0f ) )
	{
		baseScale *= ( ( m_flTimeBurnOut - gpGlobals->curtime ) / 10.0f );
	}

	bool bVisible = (baseScale < 0.01f || visible == 0.0f) ? false : true;
	//Clamp the scale if vanished
	if ( !bVisible )
	{
		if ( m_pParticle[0] != NULL )
		{	
			m_pParticle[0]->m_flDieTime		= gpGlobals->curtime;
			m_pParticle[0]->m_uchStartSize	= m_pParticle[0]->m_uchEndSize = 0;
			m_pParticle[0]->m_uchColor[0]	= 0;
			m_pParticle[0]->m_uchColor[1]	= 0;
			m_pParticle[0]->m_uchColor[2]	= 0;
		}

		if ( m_pParticle[1] != NULL )
		{	
			m_pParticle[1]->m_flDieTime		= gpGlobals->curtime;
			m_pParticle[1]->m_uchStartSize	= m_pParticle[1]->m_uchEndSize = 0;
			m_pParticle[1]->m_uchColor[0]	= 0;
			m_pParticle[1]->m_uchColor[1]	= 0;
			m_pParticle[1]->m_uchColor[2]	= 0;
		}
	}

	if ( baseScale < 0.01f )
		return;
	//
	// Dynamic light
	//

	if ( m_bLight )
	{
		dlight_t *dl= effects->CL_AllocDlight( index );

		dl->origin	= GetAbsOrigin();
		dl->color.r = 255;
		dl->color.g = dl->color.b = random->RandomInt( 32, 64 );
		dl->radius	= baseScale * random->RandomFloat( 110.0f, 128.0f );
		dl->die		= gpGlobals->curtime;
	}

	//
	// Smoke
	//

	if ( m_bSmoke )
	{
		Vector	smokeOrg = GetAbsOrigin();

		Vector	flareScreenDir = ( smokeOrg - MainViewOrigin() );
		VectorNormalize( flareScreenDir );

		smokeOrg = smokeOrg + ( flareScreenDir * 2.0f );
		smokeOrg[2] += baseScale * 4.0f;

		SimpleParticle *sParticle = (SimpleParticle *) AddParticle( sizeof( SimpleParticle ), GetPMaterial( "particle/particle_noisesphere" ), smokeOrg );
			
		if ( sParticle == NULL )
			return;

		sParticle->m_flLifetime		= 0.0f;
		sParticle->m_flDieTime		= 1.0f;
		
		sParticle->m_vecVelocity	= Vector( random->RandomFloat( -16.0f, 16.0f ), random->RandomFloat( -16.0f, 16.0f ), random->RandomFloat( 8.0f, 16.0f ) + 32.0f );

		fColor = random->RandomInt( 64, 128 );

		sParticle->m_uchColor[0]	= fColor+64;
		sParticle->m_uchColor[1]	= fColor;
		sParticle->m_uchColor[2]	= fColor;
		sParticle->m_uchStartAlpha	= random->RandomInt( 16, 32 );
		sParticle->m_uchEndAlpha	= 0;
		sParticle->m_uchStartSize	= random->RandomInt( 2, 4 );
		sParticle->m_uchEndSize		= sParticle->m_uchStartSize * 6.0f;
		sParticle->m_flRoll			= random->RandomInt( 0, 360 );
		sParticle->m_flRollDelta	= random->RandomFloat( -2.0f, 2.0f );
	}

	if ( !bVisible )
		return;

	//
	// Outer glow
	//

	Vector	offset;
	
	//Cause the base of the effect to shake
	offset.Random( -0.5f * baseScale, 0.5f * baseScale );
	offset += GetAbsOrigin();

	if ( m_pParticle[0] != NULL )
	{
		m_pParticle[0]->m_Pos			= offset;
		m_pParticle[0]->m_flLifetime	= 0.0f;
		m_pParticle[0]->m_flDieTime		= 2.0f;
		
		m_pParticle[0]->m_vecVelocity.Init();

		fColor = random->RandomInt( 100.0f, 128.0f ) * visible;

		m_pParticle[0]->m_uchColor[0]	= fColor;
		m_pParticle[0]->m_uchColor[1]	= fColor;
		m_pParticle[0]->m_uchColor[2]	= fColor;
		m_pParticle[0]->m_uchStartAlpha	= fColor;
		m_pParticle[0]->m_uchEndAlpha	= fColor;
		m_pParticle[0]->m_uchStartSize	= baseScale * (float) random->RandomInt( 32, 48 );
		m_pParticle[0]->m_uchEndSize	= m_pParticle[0]->m_uchStartSize;
		m_pParticle[0]->m_flRollDelta	= 0.0f;
		
		if ( random->RandomInt( 0, 4 ) == 3 )
		{
			m_pParticle[0]->m_flRoll	+= random->RandomInt( 2, 8 );
		}
	}

	//
	// Inner core
	//

	//Cause the base of the effect to shake
	offset.Random( -1.0f * baseScale, 1.0f * baseScale );
	offset += GetAbsOrigin();

	if ( m_pParticle[1] != NULL )
	{
		m_pParticle[1]->m_Pos			= offset;
		m_pParticle[1]->m_flLifetime	= 0.0f;
		m_pParticle[1]->m_flDieTime		= 2.0f;
		
		m_pParticle[1]->m_vecVelocity.Init();

		fColor = 255 * visible;

		m_pParticle[1]->m_uchColor[0]	= fColor;
		m_pParticle[1]->m_uchColor[1]	= fColor;
		m_pParticle[1]->m_uchColor[2]	= fColor;
		m_pParticle[1]->m_uchStartAlpha	= fColor;
		m_pParticle[1]->m_uchEndAlpha	= fColor;
		m_pParticle[1]->m_uchStartSize	= baseScale * (float) random->RandomInt( 2, 4 );
		m_pParticle[1]->m_uchEndSize	= m_pParticle[0]->m_uchStartSize;
		m_pParticle[1]->m_flRoll		= random->RandomInt( 0, 360 );
	}
}
