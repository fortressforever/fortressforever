//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_crosshairinfo.cpp
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

//#include "c_ff_player.h"
#include "ff_buildableobjects_shared.h"
#include <igameresources.h>
#include "c_ff_team.h"
#include "ff_gamerules.h"
#include "ff_utils.h"
#include "ff_shareddefs.h"

static ConVar hud_centerid( "hud_centerid", "0", FCVAR_ARCHIVE );
#define CROSSHAIRTYPE_NORMAL 0
#define CROSSHAIRTYPE_DISPENSER 1
#define CROSSHAIRTYPE_SENTRYGUN 2
#define CROSSHAIRTYPE_DETPACK 3

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
	virtual	bool	ShouldDraw( void );

	void Reset( void )
	{ 
		if( m_flDrawTime != 0.0f )
			m_flDrawTime = 0.0f; 
	}

protected:
	float		m_flStartTime;
	float		m_flDuration;
	wchar_t		m_pText[ 256 ];	// Unicode text buffer
	float		m_flDrawTime;
	float		m_flDrawDuration;

	// For center printing
	float		m_flXOffset;
	float		m_flYOffset;
	// For color
	int			m_iTeam;
	int			m_iClass;

private:

	// Stuff we need to know
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "ChatFont" );
	CPanelAnimationVarAliasType( float, text1_xpos, "text1_xpos", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, text1_ypos, "text1_ypos", "20", "proportional_float" );
};

DECLARE_HUDELEMENT( CHudCrosshairInfo );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCrosshairInfo::Init( void )
{
	m_pText[ 0 ] = '\0';
}

//-----------------------------------------------------------------------------
// Purpose: Reset on map/vgui load
//-----------------------------------------------------------------------------
void CHudCrosshairInfo::VidInit( void )
{	
	SetPaintBackgroundEnabled( false );
	m_pText[ 0 ] = '\0';
	m_flStartTime = 0.0f;		// |-- Mirv: Fix messages reappearing next map
	m_flDrawTime = 0.0f;
	m_iTeam = 0;
	m_iClass = 0;

	// Make the panel as big as the screen
	SetPos( 0, 0 );
	SetWide( scheme()->GetProportionalScaledValue( 640 ) );
	SetTall( scheme()->GetProportionalScaledValue( 480 ) );
}

//-----------------------------------------------------------------------------
// Purpose: Trace out and touch someone
//-----------------------------------------------------------------------------
void CHudCrosshairInfo::OnTick( void )
{
	if( !engine->IsInGame() )
		return;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return;

	if( !pPlayer->IsAlive() )
		Reset();

	// Check for crosshair info every x seconds
	if( m_flStartTime < gpGlobals->curtime )
	{
		// Store off when to trace next
		m_flStartTime = gpGlobals->curtime + m_flDuration;

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
				if( !pSentryGun->IsBuilt() )
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
				if( !pDispenser->IsBuilt() )
					return;

				if( pDispenser->IsAlive() )
				{
					bBuildable = true;
					pHitPlayer = ToFFPlayer( pDispenser->m_hOwner.Get() );
				}					
			}
			// If we hit a man cannon
			else if( tr.m_pEnt->Classify() == CLASS_MANCANNON )
			{
				C_FFManCannon *pManCannon = (C_FFManCannon *)tr.m_pEnt;
				if( !pManCannon->IsBuilt() )
					return;

				if( pManCannon->IsAlive() )
				{
					bBuildable = true;
					pHitPlayer = ToFFPlayer( pManCannon->m_hOwner.Get() );
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
				//bool bWeMedic = ( pPlayer->GetClassSlot() == CLASS_MEDIC );
				// Are we an engineer?
				//bool bWeEngy = ( pPlayer->GetClassSlot() == 9 );
				// Are we looking at a spy?
				bool bTheySpy = ( pHitPlayer->GetClassSlot() == CLASS_SPY );
				
				// For the player/owner name
				char szName[ MAX_PLAYER_NAME_LENGTH ];
				// For the class
				char szClass[ MAX_PLAYER_NAME_LENGTH ];
					
				// Get their real name now (deal w/ non teammate/ally spies later)
				Q_strcpy( szName, pGR->GetPlayerName( pHitPlayer->index ) );

				if( bBuildable )
				{
					switch( tr.m_pEnt->Classify() )
					{
					case CLASS_SENTRYGUN:
						Q_strcpy( szClass, "#FF_PLAYER_SENTRYGUN" );
						break;
					case CLASS_DISPENSER:
						Q_strcpy( szClass, "#FF_PLAYER_DISPENSER" );
						break;
					case CLASS_MANCANNON:
						Q_strcpy( szClass, "#FF_PLAYER_MANCANNON" );
						break;
					}
				}
				else
				{
					// Get the players' class always
					Q_strcpy( szClass, Class_IntToResourceString( pGR->GetClass( pHitPlayer->index ) ) );
				}

				// Default
				int iHealth = -1, iArmor = -1, iCells = -1, iRockets = -1, iNails = -1, iShells = -1, iLevel = -1, iFuseTime = -1;
				int CROSSHAIRTYPE = CROSSHAIRTYPE_NORMAL;
				// Default
				m_iTeam = pHitPlayer->GetTeamNumber();
				m_iClass = pHitPlayer->GetTeamNumber();
				
				if ( (pPlayer == pHitPlayer) && (bBuildable) ) // looking at our own buildable
				{
					C_FFBuildableObject *pBuildable = ( C_FFBuildableObject * )tr.m_pEnt;
					iHealth = pBuildable->GetHealthPercent();

					if( pBuildable->Classify() == CLASS_DISPENSER )
					{
						CROSSHAIRTYPE = CROSSHAIRTYPE_DISPENSER;
						iRockets = ( ( C_FFDispenser * )pBuildable )->GetRockets();
						iShells = ( ( C_FFDispenser * )pBuildable )->GetShells();
						iCells = ( ( C_FFDispenser * )pBuildable )->GetCells();
						iNails = ( ( C_FFDispenser* )pBuildable )->GetNails();
						iArmor = ( ( C_FFDispenser * )pBuildable )->GetArmor();
					}
					else if( pBuildable->Classify() == CLASS_SENTRYGUN )
					{
						CROSSHAIRTYPE = CROSSHAIRTYPE_SENTRYGUN;
						iLevel = ( ( C_FFSentryGun * )pBuildable )->GetLevel();
						//iRockets = ( ( C_FFSentryGun * )pBuildable )->GetRocketsPercent();
						//iShells = ( ( C_FFSentryGun * )pBuildable )->GetShellsPercent();
						//iArmor = ( ( C_FFSentryGun * )pBuildable )->GetAmmoPercent();

						//if (iArmor >= 128) //VOOGRU: when the sg has no rockets it would show ammopercent+128.
						//	iArmor -= 128;
					}
					else if( pBuildable->Classify() == CLASS_DETPACK )
					{
						// Doesnt work atm - aftershock
						//CROSSHAIRTYPE = CROSSHAIRTYPE_DETPACK;
						//iFuseTime = ( ( C_FFDetpack * )pBuildable )->GetFuseTime();
						iArmor = -1;
					}
					else if( pBuildable->Classify() == CLASS_MANCANNON )
					{
						// TODO: Do whatever other stuff is doing w/ the crosshair
						iArmor = -1;
					}
					else
					{
						iArmor = -1;
					}
				}
				else if( FFGameRules()->PlayerRelationship( pPlayer, pHitPlayer ) == GR_TEAMMATE )
				{
					// We're looking at a teammate/ally

					if( bBuildable )
					{
						// Now on teammates/allies can see teammate/allies
						// buildable info according to:
						// Bug #0000463: Hud Crosshair Info - douched
						//if( bWeEngy )
						//{
							C_FFBuildableObject *pBuildable = (C_FFBuildableObject *)tr.m_pEnt;

							iHealth = pBuildable->GetHealthPercent();
								
							if( pBuildable->Classify() == CLASS_DISPENSER )
							{
								iArmor = ((C_FFDispenser *)pBuildable)->GetAmmoPercent();
							}
							else if( pBuildable->Classify() == CLASS_SENTRYGUN )
							{
								//iArmor = ((C_FFSentryGun *)pBuildable)->GetAmmoPercent();

								//if (iArmor >= 128) //VOOGRU: when the sg has no rockets it would show ammopercent+128.
								//	iArmor -= 128;
							}
							else if( pBuildable->Classify() == CLASS_MANCANNON )
							{
								// TODO: Maybe man cannon's have armor? or some other property we want to show?
								iArmor = -1;
							}
							else
							{
								iArmor = -1;
							}
						//}
					}
					else
					{						
						iHealth = pHitPlayer->GetHealthPercentage();
						iArmor = pHitPlayer->GetArmorPercentage();
					}
				}
				else
				{
					// We're looking at a non teammate/ally
					// Only thing we care about is if we are a medic or we're looking
					// at a spy because otherwise we've done everything above
	
					if( !bBuildable )
					{
						// We're looking at a player

						//if( bWeMedic ) //AfterShock: Testing every class having this ability
						//{
							// Grab the real health/armor of this player
							iHealth = pHitPlayer->GetHealthPercentage();
							iArmor = pHitPlayer->GetArmorPercentage();
						//}
							
						if( bTheySpy )
						{
							// We're looking at an enemy/non-allied spy

							if( pHitPlayer->IsDisguised() )
							{
								// The spy is disguised so we do some special stuff
								// to try and fake out the player - like show the class
								// we're disguised as and try to steal a name from a
								// a player on whatever team we are disguised as playing
								// as whatever class we are disguised as. If that fails
								// we use a name from the team we're disguised as. If that
								// fails we use the real name.

								// Get the disguised class
								/*int iClassSlot*/ m_iClass = pHitPlayer->GetDisguisedClass();
								Q_strcpy( szClass, Class_IntToResourceString( m_iClass ) );

								// Get the disguised team
								m_iTeam = pHitPlayer->GetDisguisedTeam();

								// If this spy is disguised as our team we need to show his
								// health/armor
								if( m_iTeam == pPlayer->GetTeamNumber() )
								{
									iHealth = pHitPlayer->GetHealthPercentage();
									iArmor = pHitPlayer->GetArmorPercentage();
								}

								// Or, if this spy is disguised as an ally of our team we
								// need to show his health/armor
								if( FFGameRules()->IsTeam1AlliedToTeam2( pPlayer->GetTeamNumber(), m_iTeam ) == GR_TEAMMATE )
								{
									iHealth = pHitPlayer->GetHealthPercentage();
									iArmor = pHitPlayer->GetArmorPercentage();
								}

								// TODO: Could be bugs with this spy tracking thing in that
								// a player who's name is being used as a spy ID drops
								// and that name is still being used because the spy hasn't
								// changed disguises...

								// Check to see if we've ID'd this spy before as the 
								// disguise he's currently disguised as
								if( pPlayer->m_hSpyTracking[ pHitPlayer->index ].SameGuy( m_iTeam, m_iClass ) )
									Q_strcpy( szName, pPlayer->m_hSpyTracking[ pHitPlayer->index ].m_szName );
								else
								{
									// Change name (if we can) to someone on the team iTeam
									// that is playing the class this guy is disguised as

									// Gonna generate an array of people on the team we're disguised as
									// in case we have to randomly pick a name later
									int iPlayers[ 128 ], iCount = 0;

									bool bDone = false;
									for( int i = 1; ( i < gpGlobals->maxClients ) && ( !bDone ); i++ )
									{
										// Skip this spy - kind of useless if it tells us
										// our real name, eh? Using our real name is a last resort
										if( i == pHitPlayer->index )
											continue;

										if( pGR->IsConnected( i ) )
										{
											// If the guy's on the team we're disguised as...
											if( pGR->GetTeam( i ) == m_iTeam )
											{
												// Store off the player index since we found
												// someone on the team we're disguised as
												iPlayers[ iCount++ ] = i;

												// If the guy's playing as the class we're disguised as...
												if( pGR->GetClass( i ) == m_iClass )
												{
													// We're stealing this guys name
													Q_strcpy( szName, pGR->GetPlayerName( i ) ) ;
													bDone = true; // bail
												}
											}
										}
									}

									// If no one was on the other team, add the real name
									// to the array of possible choices
									if( iCount == 0 )
										iPlayers[ iCount++ ] = pHitPlayer->index;
	
									// We iterated around and found no one on the team we're disguised as
									// playing as the class we're disguised as so just pick a guy from
									// the team we're disguised as (or use real name if iCount was 0)
									if( !bDone )
									{
										// So we got an array of indexes to players of whom we can steal
										// their name, so randomly steal one
										Q_strcpy( szName, pGR->GetPlayerName( iPlayers[ random->RandomInt( 0, iCount - 1 ) ] ) );
									}

									// Store off the spies name, class & team in case we ID him again
									// and he hasn't changed disguise
									pPlayer->m_hSpyTracking[ pHitPlayer->index ].Set( szName, m_iTeam, m_iClass );
								}
							}
							// Jiggles: Don't draw anything if we're looking at a cloaked enemy spy
							if( pHitPlayer->IsCloaked() )
								return;  
						}
					}					
				}

				// Set up local crosshair info struct
				pPlayer->m_hCrosshairInfo.Set( szName, m_iTeam, m_iClass );

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
				
				if (CROSSHAIRTYPE == CROSSHAIRTYPE_DISPENSER)
				{
					char szHealth[ 5 ], szArmor[ 5 ], szRockets[ 5 ], szShells[ 5 ], szCells[ 5 ], szNails[ 5 ];
					Q_snprintf( szHealth, 5, "%i%%", iHealth );
					Q_snprintf( szArmor, 5, "%i", iArmor );
					Q_snprintf( szRockets, 5, "%i", iRockets );
					Q_snprintf( szShells, 5, "%i", iShells );
					Q_snprintf( szCells, 5, "%i", iCells );
					Q_snprintf( szNails, 5, "%i", iNails );
					
					wchar_t wszHealth[ 10 ], wszArmor[ 10 ], wszRockets[ 10 ], wszCells[ 10 ], wszShells[ 10 ], wszNails[ 10 ];

                    vgui::localize()->ConvertANSIToUnicode( szHealth, wszHealth, sizeof( wszHealth ) );
					vgui::localize()->ConvertANSIToUnicode( szArmor, wszArmor, sizeof( wszArmor ) );
					vgui::localize()->ConvertANSIToUnicode( szRockets, wszRockets, sizeof( wszRockets ) );
					vgui::localize()->ConvertANSIToUnicode( szCells, wszCells, sizeof( wszCells ) );
					vgui::localize()->ConvertANSIToUnicode( szShells, wszShells, sizeof( wszShells ) );
					vgui::localize()->ConvertANSIToUnicode( szNails, wszNails, sizeof( wszNails ) );
					
					_snwprintf( m_pText, 255, L"Your Dispenser - Cells(%s) Rkts(%s) Nls(%s) Shls(%s) Armr(%s)", wszCells, wszRockets, wszNails, wszShells, wszArmor );
				}
				else if (CROSSHAIRTYPE == CROSSHAIRTYPE_SENTRYGUN)
				{
					char szHealth[ 5 ], /*szRockets[ 5 ], szShells[ 5 ],*/ szLevel[ 5 ]/*, szArmor[ 5 ]*/;
					Q_snprintf( szHealth, 5, "%i%%", iHealth );
					Q_snprintf( szLevel, 5, "%i", iLevel );
					//Q_snprintf( szRockets, 5, "%i%%", iRockets );
					//Q_snprintf( szShells, 5, "%i%%", iShells );
					//Q_snprintf( szArmor, 5, "%i%%", iArmor );

					
					wchar_t wszHealth[ 10 ], /*wszRockets[ 10 ], wszShells[ 10 ],*/ wszLevel[ 10 ]/*, wszArmor[ 10 ]*/;

                    vgui::localize()->ConvertANSIToUnicode( szHealth, wszHealth, sizeof( wszHealth ) );
					//vgui::localize()->ConvertANSIToUnicode( szRockets, wszRockets, sizeof( wszRockets ) );
					vgui::localize()->ConvertANSIToUnicode( szLevel, wszLevel, sizeof( wszLevel ) );
					//vgui::localize()->ConvertANSIToUnicode( szShells, wszShells, sizeof( wszShells ) );
					//vgui::localize()->ConvertANSIToUnicode( szArmor, wszArmor, sizeof( wszArmor ) );
					
					_snwprintf( m_pText, 255, L"Your Sentry Gun: Level %s - Health: %s", wszLevel, wszHealth );
				
				}
				else if (CROSSHAIRTYPE == CROSSHAIRTYPE_DETPACK)
				{
					char szFuseTime[ 5 ];
					Q_snprintf( szFuseTime, 5, "%i", iFuseTime );

					
					wchar_t wszFuseTime[ 10 ];

                    vgui::localize()->ConvertANSIToUnicode( szFuseTime, wszFuseTime, sizeof( wszFuseTime ) );
					
					_snwprintf( m_pText, 255, L"Your %s Second Detpack", wszFuseTime );
				}
				// else CROSSHAIRTYPE_NORMAL
				else if( ( iHealth != -1 ) && ( iArmor != -1 ) )
				{
					char szHealth[ 5 ], szArmor[ 5 ];
					Q_snprintf( szHealth, 5, "%i%%", iHealth );
					Q_snprintf( szArmor, 5, "%i%%", iArmor );
					
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
					m_flYOffset = ( float )( iScreenTall / 2 ) + ( iTall / 2 ) + 75; // 75 to get it below the crosshair and not right on it

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
			else
			{
				// Hit something but not a player/dispenser/sentrygun
				pPlayer->m_hCrosshairInfo.Set( "", 0, 0 );
			}
		}
		else
		{
			// Didn't hit anything!
			pPlayer->m_hCrosshairInfo.Set( "", 0, 0 );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: See if we should draw stuff or not
//-----------------------------------------------------------------------------
bool CHudCrosshairInfo::ShouldDraw( void )
{
	if( !engine->IsInGame() )
		return false;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return false;

	return pPlayer->IsAlive();
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff
//-----------------------------------------------------------------------------
void CHudCrosshairInfo::Paint( void )
{
	if( ( m_flDrawTime + m_flDrawDuration ) > gpGlobals->curtime )
	{

		// draw xhair info
		if( hud_centerid.GetInt() )
			surface()->DrawSetTextPos( m_flXOffset, m_flYOffset );
		else
			surface()->DrawSetTextPos( text1_xpos, text1_ypos );

		// Bug #0000686: defrag wants team colored hud_crosshair names
		surface()->DrawSetTextFont( m_hTextFont );
		Color cColor;
		SetColorByTeam( m_iTeam, cColor );		
		surface()->DrawSetTextColor( cColor.r(), cColor.g(), cColor.b(), 255 );

		for( wchar_t *wch = m_pText; *wch != 0; wch++ )
			surface()->DrawUnicodeChar( *wch );


		
	}
}
