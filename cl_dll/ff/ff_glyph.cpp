//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_glyph.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 09/19/2005
//	@brief Some common glyph code stuff
//
//	REVISIONS
//	---------
//	09/19/2005, Mulchman: 
//		First created. Making glyph code
//		shared between scout radar and
//		radio tagging.

#include "cbase.h"

using namespace vgui;

#include <vgui_controls/Panel.h>
//#include <vgui_controls/Frame.h>
//#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
//#include <vgui/ILocalize.h>

#include "ff_esp_shared.h"
#include "ff_glyph.h"

// Create instances of our externed guys
Glyph_s	g_ClassGlyphs[ FF_RADIOTAG_NUMGLYPHS ];
Glyph_s	g_RadioTowerGlyphs[ 16 ];
bool	g_bGlyphsCached = false;

void CacheGlyphs( void )
{
	if( !g_bGlyphsCached )
	{
		for( int i = 0; i < FF_RADIOTAG_NUMGLYPHS; i++ )
		{
			g_ClassGlyphs[ i ].m_pTexture = new CHudTexture( );
			
			// Jiggles: For the distance glyphs
			g_ClassGlyphs[ i ].m_pDistTexture = new CHudTexture( );  
			g_ClassGlyphs[ i ].m_pDistTexture->textureId = vgui::surface( )->CreateNewTextureID( );

			g_ClassGlyphs[ i ].m_pTexture->textureId = vgui::surface( )->CreateNewTextureID( );			

			string_t szTemp;
			string_t szDistTemp;
			switch( i )
			{
				case 9: szTemp = "glyphs/radiotag_glyph_civilian";
						szDistTemp = "glyphs/radiotag_distant_civilian";
						break;
				case 0: szTemp = "glyphs/radiotag_glyph_scout";
						szDistTemp = "glyphs/radiotag_distant_scout";
						break;
				case 1: szTemp = "glyphs/radiotag_glyph_sniper";
						szDistTemp = "glyphs/radiotag_distant_sniper";
						break;
				case 2: szTemp = "glyphs/radiotag_glyph_soldier";
						szDistTemp = "glyphs/radiotag_distant_soldier";
						break;
				case 3: szTemp = "glyphs/radiotag_glyph_demoman";
						szDistTemp = "glyphs/radiotag_distant_demoman";
						break;
				case 4: szTemp = "glyphs/radiotag_glyph_medic";
						szDistTemp = "glyphs/radiotag_distant_medic";
						break;
				case 5: szTemp = "glyphs/radiotag_glyph_hwguy";
						szDistTemp = "glyphs/radiotag_distant_hwguy";
						break;
				case 6: szTemp = "glyphs/radiotag_glyph_pyro";
						szDistTemp = "glyphs/radiotag_distant_pyro";
						break;
				case 7: szTemp = "glyphs/radiotag_glyph_spy";
						szDistTemp = "glyphs/radiotag_distant_spy";
						break;
				case 8: szTemp = "glyphs/radiotag_glyph_engineer";
						szDistTemp = "glyphs/radiotag_distant_engineer";
						break;
				default: szTemp = "glyphs/radiotag_glyph_civilian";
						 szDistTemp = "glyphs/radiotag_distant_civilian";
						break;
			}

			g_ClassGlyphs[ i ].m_szMaterial = szTemp;
			g_ClassGlyphs[ i ].m_szDistMaterial = szDistTemp;

			PrecacheMaterial( g_ClassGlyphs[ i ].m_szMaterial );
			PrecacheMaterial( g_ClassGlyphs[ i ].m_szDistMaterial );
		}

		for( int i = 0; i < 16; i++ )
		{
			g_RadioTowerGlyphs[ i ].m_pTexture = new CHudTexture( );
			g_RadioTowerGlyphs[ i ].m_pTexture->textureId = vgui::surface( )->CreateNewTextureID( );

			switch( i )
			{
				// LAME WAY TO DO IT :P :P :P
				case 0: g_RadioTowerGlyphs[ i ].m_szMaterial = "glyphs/radar01"; break;
				case 1: g_RadioTowerGlyphs[ i ].m_szMaterial = "glyphs/radar02"; break;
				case 2: g_RadioTowerGlyphs[ i ].m_szMaterial = "glyphs/radar03"; break;
				case 3: g_RadioTowerGlyphs[ i ].m_szMaterial = "glyphs/radar04"; break;
				case 4: g_RadioTowerGlyphs[ i ].m_szMaterial = "glyphs/radar05"; break;
				case 5: g_RadioTowerGlyphs[ i ].m_szMaterial = "glyphs/radar06"; break;
				case 6: g_RadioTowerGlyphs[ i ].m_szMaterial = "glyphs/radar07"; break;
				case 7: g_RadioTowerGlyphs[ i ].m_szMaterial = "glyphs/radar08"; break;
				case 8: g_RadioTowerGlyphs[ i ].m_szMaterial = "glyphs/radar09"; break;
				case 9: g_RadioTowerGlyphs[ i ].m_szMaterial = "glyphs/radar10"; break;
				case 10: g_RadioTowerGlyphs[ i ].m_szMaterial = "glyphs/radar11"; break;
				case 11: g_RadioTowerGlyphs[ i ].m_szMaterial = "glyphs/radar12"; break;
				case 12: g_RadioTowerGlyphs[ i ].m_szMaterial = "glyphs/radar13"; break;
				case 13: g_RadioTowerGlyphs[ i ].m_szMaterial = "glyphs/radar14"; break;
				case 14: g_RadioTowerGlyphs[ i ].m_szMaterial = "glyphs/radar15"; break;
				case 15: g_RadioTowerGlyphs[ i ].m_szMaterial = "glyphs/radar16"; break;
			}
		}

		g_bGlyphsCached = true;
	}
}
