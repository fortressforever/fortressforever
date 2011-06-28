//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "usermessages.h"
#include "shake.h"
#include "voice_gamemgr.h"

void RegisterUserMessages()
{
	//TODO: optimise (reduce) list of these with -1 size

	usermessages->Register( "Geiger", 1 );		// geiger info data
	usermessages->Register( "Train", 1 );		// train control data
	usermessages->Register( "HudText", -1 );	
	usermessages->Register( "SayText", -1 );	
	usermessages->Register( "TextMsg", -1 );
	usermessages->Register( "HudMsg", -1 );
	usermessages->Register( "ResetHUD", 0 );	// called every respawn
	usermessages->Register( "GameTitle", 0 );	// show game title
	usermessages->Register( "ItemPickup", -1 );	// for item history on screen
	usermessages->Register( "ShowMenu", -1 );	// show hud menu
	usermessages->Register( "Shake", 13 );		// shake view
	usermessages->Register( "Fade", 10 );	// fade HUD in/out
	usermessages->Register( "VGUIMenu", -1 );	// Show VGUI menu
	usermessages->Register( "CloseCaption", 7 ); // Show a caption (by string id number)(duration in 10th of a second)

	usermessages->Register( "SendAudio", -1 );	// play radion command

	usermessages->Register( "VoiceMask", VOICE_MAX_PLAYERS_DW*4 * 2 + 1 );
	usermessages->Register( "RequestState", 0 );

	//usermessages->Register( "BarTime", -1 );	// For the C4 progress bar.
	usermessages->Register( "Damage", -1 );		// for HUD damage indicators
	usermessages->Register( "Hit", -1 );		// for HUD/crosshair hit indicators
	usermessages->Register( "RadioText", -1 );		// for HUD damage indicators
	usermessages->Register( "HintText", -1 );	// Displays hint text display
	
	usermessages->Register( "ReloadEffect", 2 );			// a player reloading..
	usermessages->Register( "PlayerAnimEvent", -1 );	// jumping, firing, reload, etc.

	usermessages->Register( "AmmoDenied", 2 );
	//usermessages->Register( "UpdateRadar", -1 );

	// Used to send a sample HUD message
	usermessages->Register( "GameMessage", -1 );

	// ESP/radar stuff, & hints
	usermessages->Register( "RadarUpdate", -1 );
	usermessages->Register( "FF_HudHint", -1 );
	// Jiggles: Added for testing Hint Center
	usermessages->Register( "FF_SendHint", -1 );
	// End: Jiggles
	usermessages->Register( "RadioTagUpdate", -1 );
	usermessages->Register( "SetPlayerLocation", -1 );
	usermessages->Register( "SetPlayerLatestFortPoints", -1 );
	usermessages->Register( "SetPlayerTotalFortPoints", -1 );
	// addhealth/armor
	usermessages->Register( "PlayerAddHealth", -1 );
	usermessages->Register( "PlayerAddArmor", -1 );

	usermessages->Register("FF_HudLua", -1);

	// When an enemy is touching our dispenser (sent to owner of dispenser)
	usermessages->Register( "Dispenser_EnemiesUsing", -1 );
	// When an enemy is touching our dispenser (sent to enemy doing the touching)
	usermessages->Register( "Dispenser_TouchEnemy", -1 );
	// When our dispenser blows up (sent to owner of dispenser)
	usermessages->Register( "Dispenser_Destroyed", -1 );
	// When our sentrygun blows up (sent to owner of sg)
	usermessages->Register( "SentryGun_Destroyed", -1 );

	usermessages->Register("FF_BuildTimer", -1);
  
	// used to send a status icon
	usermessages->Register( "StatusIconUpdate", -1 );

	usermessages->Register("FFViewEffect", -1);

	// after player connects, the crc checksum of the server's level
	// scripts are sent to the client for validation
	usermessages->Register("FFScriptCRC", 4);

	usermessages->Register("DispenserMsg", -1);
	usermessages->Register("SentryMsg", -1);
	usermessages->Register("ManCannonMsg", -1);
	usermessages->Register("DetpackMsg", -1);
	usermessages->Register("PipeMsg", -1);
}

