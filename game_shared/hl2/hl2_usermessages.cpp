//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "usermessages.h"
#include "shake.h"
#include "voice_gamemgr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void RegisterUserMessages( void )
{
	usermessages->Register( "Geiger", 1 );
	if ( !IsXbox() )
		usermessages->Register( "Train", 1 );
	usermessages->Register( "HudText", -1 );
	usermessages->Register( "SayText", -1 );
	usermessages->Register( "TextMsg", -1 );
	usermessages->Register( "HudMsg", -1 );
	usermessages->Register( "ResetHUD", 1);		// called every respawn
	usermessages->Register( "GameTitle", 0 );
	usermessages->Register( "ItemPickup", -1 );
	usermessages->Register( "ShowMenu", -1 );
	usermessages->Register( "Shake", 13 );
	usermessages->Register( "Fade", 10 );
	usermessages->Register( "VGUIMenu", -1 );	// Show VGUI menu
	usermessages->Register( "XBoxRumble", 3 );	// Send a rumble to XBox controller
	usermessages->Register( "Battery", 2 );
	usermessages->Register( "Damage", 18 );		// BUG: floats are sent for coords, no variable bitfields in hud & fixed size Msg
	usermessages->Register( "VoiceMask", VOICE_MAX_PLAYERS_DW*4 * 2 + 1 );
	usermessages->Register( "RequestState", 0 );
	usermessages->Register( "CloseCaption", ( !IsXbox() ) ? 7 : -1 ); // Show a caption (by string id number)(duration in 10th of a second)
	usermessages->Register( "HintText", -1 );	// Displays hint text display
	usermessages->Register( "SquadMemberDied", 0 );
	usermessages->Register( "AmmoDenied", 2 );
	usermessages->Register( "CreditsMsg", 1 );
	usermessages->Register( "LogoTimeMsg", 4 );
}