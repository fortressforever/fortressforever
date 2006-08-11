//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_boxes.h
//	@author Patrick O'Leary (Mulchman)
//	@date 8/6/2006
//	@brief Loads up information for our vector bg boxes
//
//	REVISIONS
//	---------
//	08/06/2006, Mulchman: 
//		First created

#ifndef FF_HUD_BOXES_H
#define FF_HUD_BOXES_H

#ifdef _WIN32
#pragma once
#endif

// Disable for now... need FC's fonts uploaded and stuff
#define FF_USE_HUD_BOX 1
#undef FF_USE_HUD_BOX

// Prototype
bool GetVectorBgBoxInfo( const char *pszBoxName, char *pszFontFile, char& cFontChar );
void DrawHudBox( const char *pszBoxName, int x, int y, Color cColor );

#endif // FF_HUD_BOXES_H
