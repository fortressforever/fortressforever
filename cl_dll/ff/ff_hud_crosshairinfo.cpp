//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_buildable_messages.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 02/03/2006
//	@brief client side Hud crosshair info
//
//	REVISIONS
//	---------
//	02/03/2006, Mulchman: 
//		First created

#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "view.h"

using namespace vgui;

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>

//#include "debugoverlay_shared.h"

#include "c_ff_player.h"
#include "c_ff_buildableobjects.h"
#include <igameresources.h>
#include "c_ff_team.h"
#include "ff_gamerules.h"
#include "ff_utils.h"

static ConVar hud_centerid( "hud_centerid", "0" );

//=============================================================================
//
//	class CHudCrosshairInfo
//
//=============================================================================
class CHudCrosshairInfo : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CHudCrosshairInfo, vgui::Panel );

public:
	CHudCrosshairInfo( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudCrosshairInfo" )
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport() );

		// Hide when player is dead
		SetHiddenBits( HIDEHUD_PLAYERDEAD );

		vgui::ivgui()->AddTickSignal( GetVPanel() );

		m_flDuration = 0.2f;
		m_flDrawDuration = 2.0f;
	}

	~CHudCrosshairInfo( void ) {}

	void Init( void );
	void VidInit( void );
	void OnTick( void );
	void Paint( void );

protected:
	float		m_flStartTime;
	float		m_flDuration;
	wchar_t		m_pText[ 256 ];	// Unicode text buffer
	float		m_flDrawTime;
	float		m_flDrawDuration;

	// For center printing
	float		m_flXOffset;
	float		m_flYOffset;

private:

	// Stuff we need to know
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );

	CPanelAnimationVarAliasType( float, text1_xpos, "text1_xpos", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, text1_ypos, "text1_ypos", "20", "proportional_float" );
};

DECLARE_HUDELEMENT( CHudCrosshairInfo );

void CHudCrosshairInfo::Init( void )
{
	m_pText[ 0 ] = '\0';
}

void CHudCrosshairInfo::VidInit( void )
{	
	SetPaintBackgroundEnabled( false );
	m_flStartTime = -99;		// |-- Mirv: Fix messages reappearing next map
	m_flDrawTime = -99;

	// Make the panel as big as the screen
	SetPos( 0, 0 );
	SetWide( scheme()->GetProportionalScaledValue( 640 ) );
	SetTall( scheme()->GetProportionalScaledValue( 480 ) );
}

void CHudCrosshairInfo::OnTick( void )
{
	if( !engine->IsInGame() )
		return;

	// Check for crosshair info every x seconds
	if( m_flStartTime < gpGlobals->curtime )
	{
		// Store off when to trace next
		m_flStartTime = gpGlobals->curtime + m_flDuration;

		// Get a player pointer
		C_FFPlayer *pPlayer = ToFFPlayer( C_BasePlayer::GetLocalPlayer() );
		if( pPlayer )
		{
			// Get our forward vector
			Vector vecForward;
			pPlayer->EyeVectors( &vecForward );

			VectorNormalize( vecForward );

			// Get eye position
			Vector vecOrigin = pPlayer->EyePosition();

			//debugoverlay->AddLineOverlay( vecOrigin + ( vecForward * 64.f ), vecOrigin + ( vecForward * 1024.f ), 0, 0, 255, false, 3.0f );
			//debugoverlay->AddLineOverlay( vecOrigin + ( vecForward * 64.f ), vecOrigin + ( vecForward * 64.f ) + Vector( 0, 0, 8 ), 255, 0, 0, false, 3.0f );
			//debugoverlay->AddLineOverlay( vecOrigin + ( vecForward * 64.f ), vecOrigin + ( vecForward * 64.f ) + Vector( 0, 0, -8 ), 255, 0, 0, false, 3.0f );

			trace_t tr;
			UTIL_TraceLine( vecOrigin, vecOrigin + ( vecForward * 1024.f ), MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_PLAYER, &tr );

			// If we hit something...
			if( tr.DidHit() )
			{
				// Some defaults...
				bool bBuildable = false;
				C_FFPlayer *pHitPlayer = NULL;

				// If we hit a player
				if( tr.m_pEnt->IsPlayer() )
				{					
					if( tr.m_pEnt->IsAlive() )
						pHitPlayer = ToFFPlayer( tr.m_pEnt );
				}
				// If we hit a sentrygun
				else if( tr.m_pEnt->Classify() == CLASS_SENTRYGUN )
				{
					C_FFSentryGun *pSentryGun = ( C_FFSentryGun * )tr.m_pEnt;
					if( !pSentryGun->m_bBuilt )
						return;					

					if( pSentryGun->IsAlive() )
					{
						bBuildable = true;
						pHitPlayer = ToFFPlayer( pSentryGun->m_hOwner.Get() );
					}
				}
				// If we hit a dispenser
				else if( tr.m_pEnt->Classify() == CLASS_DISPENSER )
				{
					C_FFDispenser *pDispenser = ( C_FFDispenser * )tr.m_pEnt;
					if( !pDispenser->m_bBuilt )
						return;

					if( pDispenser->IsAlive() )
					{
						bBuildable = true;
						pHitPlayer = ToFFPlayer( pDispenser->m_hOwner.Get() );
					}					
				}

				// If the players/objects aren't "alive" pHitPlayer will still be NULL
				// and we'll bail here...

				// If we got a player/owner
				if( pHitPlayer )
				{
					// Get at the game resources
					IGameResources *pGR = GameResources();
					if( !pGR )
					{
						Warning( "[Crosshair Info] Failed to get game resources!\n" );
						return;
					}

					// Are we a medic?
					bool bWeMedic = ( pPlayer->GetClassSlot() == 5 );
					// Are we an engineer?
					bool bWeEngy = ( pPlayer->GetClassSlot() == 9 );
					// Are we looking at a spy?
					bool bTheySpy = ( pHitPlayer->GetClassSlot() == 8 );
					
					// For the player/owner name
					char szName[ MAX_PLAYER_NAME_LENGTH ];
					// For the class
					char szClass[ MAX_PLAYER_NAME_LENGTH ];
					
					// Get their real name now (deal w/ non teammate/ally spies later)
					Q_strcpy( szName, pGR->GetPlayerName( pHitPlayer->index ) );

					if( bBuildable )
					{
						if( tr.m_pEnt->Classify() == CLASS_SENTRYGUN )
							Q_strcpy( szClass, "#FF_PLAYER_SENTRYGUN" );
						else
							Q_strcpy( szClass, "#FF_PLAYER_DISPENSER" );
					}
					else
						Q_strcpy( szClass, Class_IntToResourceString( pHitPlayer->GetClassSlot() ) );

					int iHealth = -1, iArmor = -1, iTeam = pHitPlayer->GetTeamNumber();

					if( FFGameRules()->PlayerRelationship( pPlayer, pHitPlayer ) == GR_TEAMMATE )
					{
						// We're looking at a teammate/ally

						if( bBuildable )
						{
							if( bWeEngy )
							{
								C_FFBuildableObject *pBuildable = ( C_FFBuildableObject * )tr.m_pEnt;
								iHealth = ( ( float )pBuildable->GetHealth() / pBuildable->GetMaxHealth() ) * 100;
								iArmor = 999; // fake value for now
								// TODO: Get ammo -> iArmor will be ammo % for buildables
							}
						}
						else
						{
							iHealth = ( ( float )pHitPlayer->GetHealth() / pHitPlayer->GetMaxHealth() ) * 100;
							iArmor = ( ( float )pHitPlayer->GetArmor() / pHitPlayer->GetMaxArmor() ) * 100;							
						}
					}
					else
					{
						// We're looking at a non teammate/ally
						// Only thing we care about is if we area medic or we're looking
						// at a spy because otherwise we've done everything above

						if( !bBuildable )
						{
							if( bWeMedic )
							{
								// We can see their health/armor as a medic
								if( !bTheySpy )
								{
									// Get real health/armor
									iHealth = ( ( float )pHitPlayer->GetHealth() / pHitPlayer->GetMaxHealth() ) * 100;
									iArmor = ( ( float )pHitPlayer->GetArmor() / pHitPlayer->GetMaxArmor() ) * 100;
								}
								else
								{
									// We can see their health but mirv said we should make up a health/armor value									
									iHealth = random->RandomInt( 1, 100 );
									iArmor = random->RandomInt( 1, 100 );
								}
							}

							if( bTheySpy )
							{
								// Let's get the class by checking to see what model
								// the spy is using (ie. the model will be whatever they
								// are disguised as)

								int iClassSlot = 0;

								// Get the model name (something like models/player/spy.mdl)
								const char *pModelName = modelinfo->GetModelName( pHitPlayer->GetModel() );
								// DevMsg( "[Crosshair Info] Spy's model: %s\n", pModelName );

								// Now strip off everything but the last part
								int ch1 = '/', ch2 = '.';
								char *pBeg = Q_strrchr( pModelName, ch1 );
								if( !pBeg )
								{
									ch1 = '\\';
									pBeg = Q_strrchr( pModelName, ch1 );
								}
								char *pEnd = Q_strrchr( pModelName, ch2 );

								if( pBeg && pEnd )
								{	
									int iBeg = ( int )( pBeg - pModelName + 1 );
									int iEnd = ( int )( pEnd - pModelName + 1 );

									Q_strncpy( szClass, &pModelName[ iBeg ], iEnd - iBeg );
									//szClass[ 0 ] = toupper( szClass[ 0 ] );
									iClassSlot = Class_StringToInt( szClass );

									// Lame way to do this, ha
									Q_strcpy( szClass, Class_IntToResourceString( iClassSlot ) );
								}
								else
								{
									Warning( "[Crosshair Info] Incorrect model name format!\n" );
									return;
								}								

								// Get the "team" by whatever skin the spy is using								
								iTeam = pHitPlayer->m_nSkin + 2; // (skin 0 = blue,
								// gotta get the value up to FF_TEAM_BLUE)

								// Change name (if we can) to someone on the team m_nSkin
								// that is playing as the class we are disguised as

								// Gonna generate an array of people on the team we're disguised as
								// in case we have to randomly pick a name later
								int iPlayers[ 128 ], iCount = 0;

								bool bDone = false;
								for( int i = 1; ( i < gpGlobals->maxClients ) && ( !bDone ); i++ )
								{
									// Skip this spy - kind of useless if it tells us
									// our real name, eh?
									if( i == pHitPlayer->index )
										continue;

									if( pGR->IsConnected( i ) )
									{
										// If the guy's on the team we're disguised as...
										if( pGR->GetTeam( i ) == iTeam )
										{
											// Store off the player index since we found
											// someone on the team we're disguised as
											iPlayers[ iCount++ ] = i;

											// If the guy's playing as the class we're disguised as...
											if( pGR->GetClass( i ) == iClassSlot )
											{
												// We're stealing this guys name
												Q_strcpy( szName, pGR->GetPlayerName( i ) ) ;
												bDone = true; // bail
											}
										}
									}
								}

								// We iterated around and found no one on the team we're disguised as
								// playing as the class we're disguised as so just pick a guy from
								// the team we're disguised as
								if( !bDone )
								{
									// So we got an array of indexes to players of whom we can steal
									// their name, so randomly steal one
									Q_strcpy( szName, pGR->GetPlayerName( iPlayers[ random->RandomInt( 0, iCount - 1 ) ] ) );
								}
							}
						}					
					}

					// NOW! Remember team is 1 higher than the actual team
					// If health/armor are -1 then we don't show it

					// Convert to unicode & localize stuff

					const char *pszOldName = szName;
					int iBufSize = ( int )strlen( pszOldName ) * 2;
					char *pszNewName = ( char * )_alloca( iBufSize );

					UTIL_MakeSafeName( pszOldName, pszNewName, iBufSize );

					wchar_t wszName[ 256 ];
					vgui::localize()->ConvertANSIToUnicode( pszNewName, wszName, sizeof( wszName ) );

					wchar_t wszClass[ 256 ];					
					wchar_t *pszTemp = vgui::localize()->Find( szClass );
					if( pszTemp )
						wcscpy( wszClass, pszTemp );
					else
					{
						wcscpy( wszClass, L"CLASS" );	// TODO: fix to show English version of class name :/
					}

					if( ( iHealth != -1 ) && ( iArmor != -1 ) )
					{
						char szHealth[ 5 ], szArmor[ 5 ];
						Q_snprintf( szHealth, 5, "%i", iHealth );
						Q_snprintf( szArmor, 5, "%i", iArmor );

						wchar_t wszHealth[ 10 ], wszArmor[ 10 ];

                        vgui::localize()->ConvertANSIToUnicode( szHealth, wszHealth, sizeof( wszHealth ) );
						vgui::localize()->ConvertANSIToUnicode( szArmor, wszArmor, sizeof( wszArmor ) );

						_snwprintf( m_pText, 255, L"(%s) %s - H: %s, A: %s", wszClass, wszName, wszHealth, wszArmor );
					}
					else
						_snwprintf( m_pText, 255, L"(%s) %s", wszClass, wszName );

					if( hud_centerid.GetInt() )
					{
						// Get the screen width/height
						int iScreenWide, iScreenTall;
						surface()->GetScreenSize( iScreenWide, iScreenTall );

						// "map" screen res to 640/480
						float iXScale = 640.0f / iScreenWide;
						float iYScale = 480.0f / iScreenTall;

						int iWide = UTIL_ComputeStringWidth( m_hTextFont, m_pText );
						int iTall = surface()->GetFontTall( m_hTextFont );

						// Adjust values to get below the crosshair and offset correctly
						m_flXOffset = ( float )( iScreenWide / 2 ) - ( iWide / 2 );
						m_flYOffset = ( float )( iScreenTall / 2 ) + ( iTall / 2 ) + 75; // 100 to get it below the crosshair and not right on it

						// Scale by "map" scale values
						m_flXOffset *= iXScale;
						m_flYOffset *= iYScale;

						// Scale to screen co-ords
						m_flXOffset = scheme()->GetProportionalScaledValue( m_flXOffset );
						m_flYOffset = scheme()->GetProportionalScaledValue( m_flYOffset );
					}

					// Start drawing
					m_flDrawTime = gpGlobals->curtime;
				}
			}
		}
	}
}

void CHudCrosshairInfo::Paint( void )
{
	if( ( m_flDrawTime + m_flDrawDuration ) > gpGlobals->curtime )
	{
		surface()->DrawSetTextFont( m_hTextFont );
		surface()->DrawSetTextColor( GetFgColor() );

		if( hud_centerid.GetInt() )
			surface()->DrawSetTextPos( m_flXOffset, m_flYOffset );
		else
			surface()->DrawSetTextPos( text1_xpos, text1_ypos );

		for( wchar_t *wch = m_pText; *wch != 0; wch++ )
			surface()->DrawUnicodeChar( *wch );
	}
}
