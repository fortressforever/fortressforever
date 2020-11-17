/********************************************************************
	created:	2010/08
	filename: 	cl_dll\ff\ff_hud_quantitypanel.h
	file path:	cl_dll\ff
	file base:	ff_hud_quantitypanel
	file ext:	h
	author:		Elmo
	
	purpose:	Customisable Quanitity Panel for the HUD
*********************************************************************/

#ifndef FF_QUANTITYPANEL_H
#define FF_QUANTITYPANEL_H

#include "cbase.h"
#include <vgui_controls/Panel.h>
#include "keyValues.h"

#include "ff_quantityitem.h"

//this is defined in the panel style preset .h too, keep it in sync
#define QUANTITYPANELTEXTSIZES 15
#define QUANTITYPANELICONSIZES 20

namespace vgui
{
	class FFQuantityPanel : public Panel
	{
	DECLARE_CLASS_SIMPLE( FFQuantityPanel, Panel );
	public:

		FFQuantityPanel( Panel *parent, const char *panelName );

		enum Corners {
			CORNERS_ROUND=2,
			CORNERS_SQUARE=0
		};

		void SetHeaderText( wchar_t *newHeaderText, bool bRecalculatePaintOffset = false );
		void SetHeaderIconChar( char *newHeaderIconChar, bool bRecalculatePaintOffset = false );
		void SetText( wchar_t *newText, bool bRecalculatePaintOffset = false );

		void SetUseToggleText( bool bUseToggletext );
		void SetToggleTextVisible( bool bIsVisible );

		FFQuantityItem* AddItem( const char *pElementName );
		void HideItem( FFQuantityItem* qBar );
		void ShowItem( FFQuantityItem* qBar );
		void DisableItem( FFQuantityItem* qBar );
		void EnableItem( FFQuantityItem* qBar );

		void SetPreviewMode(bool bInPreview);
		//TODO void FlashColor(Color colorFlash);

	protected:

		void AddPanelToHudOptions( const char* szSelfName, const char* szSelfText, const char* szParentName, const char* szParentText );
		void AddBooleanOption(KeyValues* kvMessage, const char *pszName, const char *pszText, const bool defaultValue = false);
		void AddComboOption(KeyValues* kvMessage, const char *pszName, const char *pszText, KeyValues* kvOptions, const int defaultValueId = -1);
		virtual KeyValues* AddPanelSpecificOptions(KeyValues *kvPanelSpecificOptions);
		virtual KeyValues* AddItemStyleList(KeyValues *kvItemStyleList);

		virtual void Paint( void );
		virtual void OnTick( void );

		virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
		virtual KeyValues* GetDefaultStyleData();

		virtual MESSAGE_FUNC_PARAMS( OnStyleDataRecieved, "SetStyleData", kvStyleData );
		virtual MESSAGE_FUNC_PARAMS( OnPresetPreviewDataRecieved, "SetPresetPreviewData", kvPresetPreviewData );

	private:

		char m_szSelfName[128];
		char m_szSelfText[128];
		char m_szParentName[128];
		char m_szParentText[128];
		bool m_bAddToHud;
		bool m_bAddToHudSent;
		
		bool m_bPreviewMode;

		float m_flScale;
		float m_flScaleX;
		float m_flScaleY;
		
		int m_iOffsetX;
		int m_iOffsetY;

		int m_iPanelMargin;

		vgui::HFont m_hfHeaderIconFamily[QUANTITYPANELICONSIZES * 3];
		vgui::HFont m_hfHeaderTextFamily[QUANTITYPANELTEXTSIZES * 3];
		vgui::HFont m_hfTextFamily[QUANTITYPANELTEXTSIZES * 3];

		vgui::HFont m_hfHeaderIcon;
		vgui::HFont m_hfHeaderText;
		vgui::HFont m_hfText;

		bool m_bHeaderTextFontShadow;
		bool m_bHeaderIconFontShadow;
		bool m_bTextFontShadow;

		bool m_bShowHeaderText;
		bool m_bShowHeaderIcon;
		bool m_bShowText;
		bool m_bShowPanel;

		bool m_bUseToggleText;
		bool m_bToggleTextVisible;

		int m_iHeaderTextSize;
		int m_iHeaderIconSize;
		int m_iTextSize;
		
		int m_iTeamColourAlpha;
		Color m_clrTeam;
		Color m_clrPanel;

		int m_iPositionalHashCode;
		Dar<FFQuantityItem*> m_DarQuantityItems;

		bool m_bCheckUpdates;

		int m_iX;
		int m_iY;
		int m_iWidth;
		int m_iHeight;
		int m_iHorizontalAlign;
		int m_iVerticalAlign;

		wchar_t m_wszHeaderText[50];
		wchar_t m_wszHeaderIcon[2];
		wchar_t m_wszText[50];
		
		Color m_clrHeaderText;
		Color m_clrHeaderIcon;
		Color m_clrText;

		Color m_clrPanelCustom;
		Color m_clrHeaderTextCustom;
		Color m_clrHeaderIconCustom;
		Color m_clrTextCustom;

		int m_iPanelColorMode;
		int m_iHeaderTextColorMode;
		int m_iHeaderIconColorMode;
		int m_iTextColorMode;

		int m_iHeaderTextWidth;
		int m_iHeaderTextHeight;
		int m_iHeaderIconWidth;
		int m_iHeaderIconHeight;
		int m_iTextWidth;
		int m_iTextHeight;

		int m_iHeaderTextAnchorPosition;
		int m_iHeaderIconAnchorPosition;
		int m_iTextAnchorPosition;

		int m_iHeaderTextAlignHoriz;
		int m_iHeaderIconAlignHoriz;
		int m_iTextAlignHoriz;

		int m_iHeaderTextAlignVert;
		int m_iHeaderIconAlignVert;
		int m_iTextAlignVert;

		int m_iHeaderTextAnchorPositionX;
		int m_iHeaderIconAnchorPositionX;
		int m_iTextAnchorPositionX;
		
		int m_iHeaderTextAnchorPositionY;
		int m_iHeaderIconAnchorPositionY;
		int m_iTextAnchorPositionY;

		int m_iHeaderTextAlignmentOffsetX;
		int m_iHeaderIconAlignmentOffsetX;
		int m_iTextAlignmentOffsetX;
		
		int m_iHeaderTextAlignmentOffsetY;
		int m_iHeaderIconAlignmentOffsetY;
		int m_iTextAlignmentOffsetY;

		int m_iHeaderTextPositionOffsetX;
		int m_iHeaderIconPositionOffsetX;
		int m_iTextPositionOffsetX;
		
		int m_iHeaderTextPositionOffsetY;
		int m_iHeaderIconPositionOffsetY;
		int m_iTextPositionOffsetY;
		
		int m_iItemMarginHorizontal;
		int m_iItemMarginVertical;
		int m_iItemColumns;
		int m_iItemsWidth;
		int m_iItemsHeight;
		
		int m_iItemsPositionX;
		int m_iItemsPositionY;
		int m_iHeaderTextPositionX;
		int m_iHeaderTextPositionY;
		int m_iHeaderIconPositionX;
		int m_iHeaderIconPositionY;
		int m_iTextPositionX;
		int m_iTextPositionY;

		bool SetHeaderTextShadow( bool bUseShadow);
		bool SetHeaderIconShadow( bool bUseShadow);
		bool SetTextShadow( bool bUseShadow);

		bool SetHeaderTextSize( int iSize );
		bool SetHeaderIconSize( int iSize );
		bool SetTextSize( int iSize );

		void RecalculateHeaderIconFont();
		void RecalculateHeaderTextFont();
		void RecalculateTextFont();
		vgui::HFont GetFont(vgui::HFont* hfFamily, int iSize, bool bUseModifier);

		bool SetHeaderTextVisible( bool bIsVisible );
		bool SetHeaderIconVisible( bool bIsVisible );
		bool SetTextVisible( bool bIsVisible );

		bool SetHeaderTextAnchorPosition( int iAnchorPositionHeaderText );
		bool SetHeaderIconAnchorPosition( int iAnchorPositionHeaderIcon );
		bool SetTextAnchorPosition( int iAnchorPositionText );

		bool SetHeaderTextAlignment( int iAlignHHeaderText, int iAlignVHeaderText );
		bool SetHeaderIconAlignment( int iAlignHHeaderIcon, int iAlignVHeaderIcon );
		bool SetTextAlignment( int iAlignHText, int iAlignVText );

		bool SetHeaderTextPositionOffset( int iOffsetXHeaderText, int iOffsetYHeaderText );
		bool SetHeaderIconPositionOffset( int iOffsetXHeaderIcon, int iOffsetYHeaderIcon );
		bool SetTextPositionOffset( int iOffsetXText, int iOffsetYText );

		bool SetPanelColorMode( int iColorMode );
		bool SetHeaderIconColorMode( int iColorMode );
		bool SetHeaderTextColorMode( int iColorMode );
		bool SetTextColorMode( int iColorMode );

		bool SetCustomPanelColor( int iRed, int iGreen, int iBlue, int iAlpha );
		bool SetCustomHeaderIconColor( int iRed, int iGreen, int iBlue, int iAlpha );
		bool SetCustomHeaderTextColor( int iRed, int iGreen, int iBlue, int iAlpha );
		bool SetCustomTextColor( int iRed, int iGreen, int iBlue, int iAlpha );

		bool SetTeamColor( Color teamColor );
		Color GetTeamColor();

		void ApplyStyleData( KeyValues *kvStyleData, bool useDefaults = true );
		
		int GetInt(const char *keyName, KeyValues *kvStyleData, KeyValues *kvDefaultStyleData, int iDefaultValue = -1);
		
		void RecalculateColor( Color &clr, int iColorMode, Color &clrCustom );
		Color GetRgbColor( int iColorMode, Color &clrCustom );
		void SetColor( Color &clr, Color &clrRgb, Color &clrAlpha );

		void CalculateTextAnchorPosition( int &outX, int &outY, int iAnchorPosition );
		void CalculateTextAlignmentOffset( int &outX, int &outY, int &iWide, int &iTall, int iAlignHoriz, int iAlignVert, HFont hfFont, wchar_t* wszString );
		
		void RecalculateItemPositions( );
		void RecalculateHeaderIconPosition( );
		void RecalculateHeaderTextPosition( );
		void RecalculateTextPosition( );
		void RecalculatePaintOffset( );

		void DrawText(wchar_t* wszText, HFont font, Color color, int iXPosition, int iYPosition);

		MESSAGE_FUNC_PARAMS( OnDefaultStyleDataRequested, "GetStyleData", data);	
	};
}

#endif