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

#include "ff_quantitybar.h"

//this is defined in the panel style preset .h too, keep it in sync
#define QUANTITYPANELFONTSIZES 15
#define QUANTITYPANELICONSIZES 20

namespace vgui
{
	class FFQuantityPanel : public Panel
	{
	DECLARE_CLASS_SIMPLE( FFQuantityPanel, Panel );
	public:
		FFQuantityPanel( Panel *parent, const char *panelName );

		enum ItemColorMode {
			ITEM_COLOR_MODE_CUSTOM=0,
			ITEM_COLOR_MODE_STEPPED,
			ITEM_COLOR_MODE_FADED,
			ITEM_COLOR_MODE_TEAMCOLORED
		};

		enum ColorMode {
			COLOR_MODE_TEAMCOLORED=3,
			COLOR_MODE_CUSTOM=0
		};
		
		enum Position {
			ANCHORPOS_TOPLEFT=0,
			ANCHORPOS_TOPCENTER,
			ANCHORPOS_TOPRIGHT,
			ANCHORPOS_MIDDLELEFT,
			ANCHORPOS_MIDDLECENTER,
			ANCHORPOS_MIDDLERIGHT,
			ANCHORPOS_BOTTOMLEFT,
			ANCHORPOS_BOTTOMCENTER,
			ANCHORPOS_BOTTOMRIGHT
		};

		enum AlignmentHorizontal {
			ALIGN_LEFT=0,
			ALIGN_CENTER,
			ALIGN_RIGHT
		};

		enum AlignmentVertical {
			ALIGN_TOP=0,
			ALIGN_MIDDLE,
			ALIGN_BOTTOM
		};

		enum OrientationMode {
			ORIENTATION_HORIZONTAL=0,
			ORIENTATION_VERTICAL,
			ORIENTATION_HORIZONTAL_INVERTED,
			ORIENTATION_VERTICAL_INVERTED
		};

		void AddBooleanOption(KeyValues* kvMessage, const char *pszName, const char *pszText, const bool defaultValue = false);
		void AddComboOption(KeyValues* kvMessage, const char *pszName, const char *pszText, KeyValues* kvOptions, const int defaultValueId = -1);
		virtual KeyValues* AddPanelSpecificOptions(KeyValues *kvPanelSpecificOptions);
		virtual KeyValues* AddItemStyleList(KeyValues *kvItemStyleList);

		virtual void Paint( void );
		virtual void OnTick( void );

		virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
		virtual KeyValues* GetDefaultStyleData();

		void SetHeaderText( wchar_t *newHeaderText, bool bRecalculatePaintOffset = true );
		void SetHeaderIconChar( char *newHeaderIconChar, bool bRecalculatePaintOffset = true );
		void SetText( wchar_t *newText, bool bRecalculatePaintOffset = true );

		void SetHeaderTextShadow( bool bHasShadow);
		void SetHeaderIconShadow( bool bHasShadow);
		void SetTextShadow( bool bHasShadow);

		void SetHeaderTextSize( int iSize, bool bRecalculatePaintOffset = true );
		void SetHeaderIconSize( int iSize, bool bRecalculatePaintOffset = true );
		void SetTextSize( int iSize, bool bRecalculatePaintOffset = true );

		void SetHeaderTextVisible( bool bIsVisible, bool bRecalculatePaintOffset = true );
		void SetHeaderIconVisible( bool bIsVisible, bool bRecalculatePaintOffset = true );
		void SetTextVisible( bool bIsVisible, bool bRecalculatePaintOffset = true );

		void SetUseToggleText( bool bUseToggletext );
		void SetToggleTextVisible( bool bIsVisible );

		void SetHeaderTextAnchorPosition( int iAnchorPositionHeaderText, bool bRecalculatePaintOffset = true );
		void SetHeaderIconAnchorPosition( int iAnchorPositionHeaderIcon, bool bRecalculatePaintOffset = true );
		void SetTextAnchorPosition( int iAnchorPositionText, bool bRecalculatePaintOffset = true );

		void SetHeaderTextAlignment( int iAlignHHeaderText, int iAlignVHeaderText, bool bRecalculatePaintOffset = true );
		void SetHeaderIconAlignment( int iAlignHHeaderIcon, int iAlignVHeaderIcon, bool bRecalculatePaintOffset = true );
		void SetTextAlignment( int iAlignHText, int iAlignVText, bool bRecalculatePaintOffset = true );

		void SetHeaderTextPositionOffset( int iOffsetXHeaderText, int iOffsetYHeaderText, bool bRecalculatePaintOffset = true );
		void SetHeaderIconPositionOffset( int iOffsetXHeaderIcon, int iOffsetYHeaderIcon, bool bRecalculatePaintOffset = true );
		void SetTextPositionOffset( int iOffsetXText, int iOffsetYText, bool bRecalculatePaintOffset = true );

		void SetHeaderTextColor( Color newHeaderTextColor );
		void SetHeaderIconColor( Color newHeaderIconColor );
		void SetTextColor( Color newTextColor );

		void SetTeamColor( Color teamColor );
		Color GetTeamColor();

		void ApplyStyleData( KeyValues *kvStyleData, bool useDefaults = true );
	
		FFQuantityItem* AddItem( const char *pElementName );
		void HideItem( FFQuantityItem* qBar );
		void ShowItem( FFQuantityItem* qBar );
		void DisableItem( FFQuantityItem* qBar );
		void EnableItem( FFQuantityItem* qBar );

		void SetPreviewMode(bool bInPreview);
		//TODO void FlashColor(Color colorFlash);

	private:
		MESSAGE_FUNC_PARAMS( OnItemDimentionsChanged, "ItemDimentionsChanged", data );
		MESSAGE_FUNC_PARAMS( OnDefaultStyleDataRequested, "GetStyleData", data);

	protected:
		void CalculateTextAnchorPosition( int &outX, int &outY, int iAnchorPosition );
		void CalculateTextAlignmentOffset( int &outX, int &outY, int &iWide, int &iTall, int iAlignHoriz, int iAlignVert, HFont hfFont, wchar_t* wszString );
		
		void RecalculateItemPositions();
		void RecalculateHeaderIconPosition( bool bRecalculatePaintOffset = true );
		void RecalculateHeaderTextPosition( bool bRecalculatePaintOffset = true );
		void RecalculateTextPosition( bool bRecalculatePaintOffset = true );
		void RecalculatePaintOffset( void );

		//bool m_bVidInit;
		bool m_bPreviewMode;

		virtual MESSAGE_FUNC_PARAMS( OnStyleDataRecieved, "SetStyleData", kvStyleData );
		virtual MESSAGE_FUNC_PARAMS( OnPresetPreviewDataRecieved, "SetPresetPreviewData", kvPresetPreviewData );
		void AddPanelToHudOptions( const char* szSelfName, const char* szSelfText, const char* szParentName, const char* szParentText );
		
		char m_szSelfName[128];
		char m_szSelfText[128];
		char m_szParentName[128];
		char m_szParentText[128];
		bool m_bAddToHud;
		bool m_bAddToHudSent;

		float m_flScale;
		float m_flScaleX;
		float m_flScaleY;
		
		int m_iOffsetX;
		int m_iOffsetY;

		int m_iPanelMargin;

		vgui::HFont m_hfText[QUANTITYPANELFONTSIZES * 3];
		vgui::HFont m_hfHeaderText[QUANTITYPANELFONTSIZES * 3];
		vgui::HFont m_hfHeaderIcon[QUANTITYPANELICONSIZES * 3];

		int m_iColorModeHeaderText;
		int m_iColorModeHeaderIcon;
		int m_iColorModeText;

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
		bool m_bCustomBackroundColor;
		Color m_ColorTeam;
		Color m_ColorPanelBackground;
		Color m_ColorPanelBackgroundCustom;

		Dar<FFQuantityItem*> m_DarQuantityItems;

		bool m_bCheckUpdates;
		float m_flCheckUpdateFlagTime;

		int m_iX;
		int m_iY;
		int m_iWidth;
		int m_iHeight;
		int m_iHorizontalAlign;
		int m_iVerticalAlign;

		wchar_t m_wszHeaderText[50];
		wchar_t m_wszHeaderIcon[2];
		wchar_t m_wszText[50];
		
		Color m_clrHeaderTextColor;
		Color m_clrHeaderIconColor;
		Color m_clrTextColor;

		Color m_clrHeaderTextCustomColor;
		Color m_clrHeaderIconCustomColor;
		Color m_clrTextCustomColor;

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
	};
}

#endif