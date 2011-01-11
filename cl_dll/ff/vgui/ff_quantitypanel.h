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

//this is defined in the custom hud options pages too, keep it in sync
#define QUANTITYPANELFONTSIZES 10
#define QUANTITYPANELICONSIZES 15

namespace vgui
{
	class FFQuantityPanel : public Panel
	{
	DECLARE_CLASS_SIMPLE( FFQuantityPanel, Panel );
	public:
		FFQuantityPanel( Panel *parent, const char *panelName );

		enum ColorMode {
			COLOR_MODE_CUSTOM=0,
			COLOR_MODE_STEPPED,
			COLOR_MODE_FADED,
			COLOR_MODE_TEAMCOLORED
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

		virtual void Paint( void );
		virtual void OnTick( void );
		virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

		void SetHeaderText( wchar_t *newHeaderText, bool bUpdateQBPositions = true );
		void SetHeaderIconChar( char *newHeaderIconChar, bool bUpdateQBPositions = true );
		void SetText( wchar_t *newText, bool bUpdateQBPositions = true );

		void SetHeaderTextShadow( bool bHasShadow);
		void SetHeaderIconShadow( bool bHasShadow);
		void SetTextShadow( bool bHasShadow);

		void SetHeaderTextSize( int iSize, bool bUpdateQBPositions = true );
		void SetHeaderIconSize( int iSize, bool bUpdateQBPositions = true );
		void SetTextSize( int iSize, bool bUpdateQBPositions = true );

		void SetHeaderTextVisible( bool bIsVisible, bool bUpdateQBPositions = true );
		void SetHeaderIconVisible( bool bIsVisible, bool bUpdateQBPositions = true );
		void SetTextVisible( bool bIsVisible );

		void SetUseToggleText( bool bUseToggletext );
		void SetToggleTextVisible( bool bIsVisible );

		void SetHeaderTextPosition( int iPositionX, int iPositionY, bool bUpdateQBPositions = true );
		void SetHeaderIconPosition( int iPositionX, int iPositionY, bool bUpdateQBPositions = true );
		void SetTextPosition( int iPositionX, int iPositionY, bool bUpdateQBPositions = true );

		void SetHeaderTextColor( Color newHeaderTextColor );
		void SetHeaderIconColor( Color newHeaderIconColor );
		void SetTextColor( Color newTextColor );

		void SetBarsVisible( bool bIsVisible, bool bUpdateQBPositions = true );

		void SetTeamColor( Color teamColor );
		Color GetTeamColor();

		virtual KeyValues* GetDefaultStyleData() = 0;
	
		void UpdateQBPositions();

	private:
		MESSAGE_FUNC_PARAMS( OnChildDimentionsChanged, "ChildDimentionsChanged", data );
		MESSAGE_FUNC_PARAMS( OnStyleDataRecieved, "StyleData", data);

	protected:
		void AddPanelToHudOptions( const char* szSelfName, const char* szSelfText, const char* szParentName, const char* szParentText );
		FFQuantityBar* AddChild( const char *pElementName );

		bool m_bDraw;

		char m_szSelfName[128];
		char m_szSelfText[128];
		char m_szParentName[128];
		char m_szParentText[128];
		bool m_bAddToHud;
		bool m_bAddToHudSent;

		float m_flScale;
		float m_flScaleX;
		float m_flScaleY;

		vgui::HFont m_hfText[QUANTITYPANELFONTSIZES * 3];
		vgui::HFont m_hfHeaderText[QUANTITYPANELFONTSIZES * 3];
		vgui::HFont m_hfHeaderIcon[QUANTITYPANELICONSIZES * 3];
		
		bool m_bHeaderTextShadow;
		bool m_bHeaderIconShadow;
		bool m_bTextShadow;

		bool m_bShowHeaderText;
		bool m_bShowHeaderIcon;
		bool m_bShowText;

		bool m_bUseToggleText;
		bool m_bToggleTextVisible;

		int m_iHeaderTextSize;
		int m_iHeaderIconSize;
		int m_iTextSize;
		
		int m_iTeamColourAlpha;
		bool m_bCustomBackroundColor;
		Color m_ColorTeam;
		Color m_ColorBackground;
		Color m_ColorBackgroundCustom;

		Dar<FFQuantityBar*> m_DarQuantityBars;

		bool m_bCheckUpdates;
		float m_flCheckUpdateFlagTime;

		int m_iX;
		int m_iY;
		int m_iWidth;
		int m_iHeight;
		int m_iHorizontalAlign;
		int m_iVerticalAlign;

		int m_iHeaderTextX;
		int m_iHeaderTextY;
		int m_iHeaderIconX;
		int m_iHeaderIconY;
		int m_iTextX;
		int m_iTextY;

		wchar_t m_wszHeaderText[50];
		wchar_t m_wszHeaderIcon[2];
		wchar_t m_wszText[50];
		
		Color m_ColorHeaderText;
		Color m_ColorHeaderIcon;
		Color m_ColorText;

		int m_iHeaderTextWidth;
		int m_iHeaderTextHeight;
		int m_iHeaderIconWidth;
		int m_iHeaderIconHeight;
		int m_iTextWidth;
		int m_iTextHeight;

		int m_iItemMarginHorizontal;
		int m_iItemMarginVertical;
		int m_iItemPositionX;
		int m_iItemPositionY;
		int m_iItemColumns;

		bool m_bShowPanel;
	};
}

#endif