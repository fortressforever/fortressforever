#include "cbase.h"
#include "ff_hud_buildstate_base.h"

#include "c_ff_player.h" //required to cast base player

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar hud_buildstate_x( "hud_buildstate_x", "640", FCVAR_ARCHIVE, "Panel's X position on 640 480 Resolution", true, 0, true, 640);
static ConVar hud_buildstate_y( "hud_buildstate_y", "190", FCVAR_ARCHIVE, "Panel's Y Position on 640 480 Resolution", true, 0, true, 480);
static ConVar hud_buildstate_align_horiz( "hud_buildstate_align_horiz", "2", FCVAR_ARCHIVE, "Panel's alignment to the specified position (0=left, 1=center, 2=right", true, 0, true, 2);
static ConVar hud_buildstate_align_vert( "hud_buildstate_align_vert", "2", FCVAR_ARCHIVE, "Panel's alignment to the specified position (0=top, 1=middle, 2=bottom", true, 0, true, 2);
static ConVar hud_buildstate_columns( "hud_buildstate_columns", "1", FCVAR_ARCHIVE, "Number of quantity bar columns", true, 1, true, 6);

static ConVar hud_buildstate_spacing_x( "hud_buildstate_spacing_x", "5", FCVAR_ARCHIVE, "Horizontal spacing between bars", true, 0, true, 40);
static ConVar hud_buildstate_spacing_y( "hud_buildstate_spacing_y", "5", FCVAR_ARCHIVE, "Vertical spacing between bars", true, 0, true, 40);

static ConVar hud_buildstate_pos_headerText_x( "hud_buildstate_pos_headerText_x", "30", FCVAR_ARCHIVE, "Header text X from top left", true, 0, true, 640);
static ConVar hud_buildstate_pos_headerText_y( "hud_buildstate_pos_headerText_y", "15", FCVAR_ARCHIVE, "Header text Y from top left", true, 0, true, 640);

static ConVar hud_buildstate_pos_headericon_x( "hud_buildstate_pos_headericon_x", "10", FCVAR_ARCHIVE, "Header icon X from top left", true, 0, true, 640);
static ConVar hud_buildstate_pos_headericon_y( "hud_buildstate_pos_headericon_y", "10", FCVAR_ARCHIVE, "Header icon Y from top left", true, 0, true, 640);

static ConVar hud_buildstate_pos_bars_x( "hud_buildstate_pos_bars_x", "35", FCVAR_ARCHIVE, "Quantity bars X offset from top left", true, 0, true, 640);
static ConVar hud_buildstate_pos_bars_y( "hud_buildstate_pos_bars_y", "15", FCVAR_ARCHIVE, "Quantity bars Y offset from top left", true, 0, true, 640);

static ConVar hud_buildstate_intensity_red( "hud_buildstate_intensity_red", "20", FCVAR_ARCHIVE, "When using colormodes stepped and faded this is the value at which the bar is red");
static ConVar hud_buildstate_intensity_orange( "hud_buildstate_intensity_orange", "50", FCVAR_ARCHIVE, "When using colormodes stepped and faded this is the value at which the bar is orange");
static ConVar hud_buildstate_intensity_yellow( "hud_buildstate_intensity_yellow", "70", FCVAR_ARCHIVE, "When using colormodes stepped and faded this is the value at which the bar is yellow");
static ConVar hud_buildstate_intensity_green( "hud_buildstate_intensity_green", "100", FCVAR_ARCHIVE, "When using colormodes stepped and faded this is the value at which the bar is green");

static ConVar hud_buildstate_show_Bar( "hud_buildstate_show_bar", "1", FCVAR_ARCHIVE, "Show Bar", true, 0, true, 1);
static ConVar hud_buildstate_show_BarBackground( "hud_buildstate_show_barBackground", "1", FCVAR_ARCHIVE, "Show Bar Background", true, 0, true, 1);
static ConVar hud_buildstate_show_BarBorder( "hud_buildstate_show_barBorder", "1", FCVAR_ARCHIVE, "Show Bar Boarder", true, 0, true, 1);
static ConVar hud_buildstate_show_Icon( "hud_buildstate_show_icon", "1", FCVAR_ARCHIVE, "Show Icon", true, 0, true, 1);
static ConVar hud_buildstate_show_Label( "hud_buildstate_show_label", "1", FCVAR_ARCHIVE, "Show label", true, 0, true, 1);
static ConVar hud_buildstate_show_Amount( "hud_buildstate_show_amount", "1", FCVAR_ARCHIVE, "Show amount", true, 0, true, 1);

static ConVar hud_buildstate_bar_width( "hud_buildstate_bar_width", "25", FCVAR_ARCHIVE, "Bar width on 640 480 Resolution", true, 1, false, 0);
static ConVar hud_buildstate_bar_height( "hud_buildstate_bar_height", "4", FCVAR_ARCHIVE, "Bar height on 640 480 Resolution", true, 1, false, 0);
static ConVar hud_buildstate_bar_borderWidth( "hud_buildstate_bar_borderWidth", "1", FCVAR_ARCHIVE, "Bar border width on 640 480 Resolution", true, 0, false, 0);

static ConVar hud_buildstate_color_bar_r( "hud_buildstate_color_bar_r", "255", FCVAR_ARCHIVE, "Bar color red component");
static ConVar hud_buildstate_color_bar_g( "hud_buildstate_color_bar_g", "255", FCVAR_ARCHIVE, "Bar color green component");
static ConVar hud_buildstate_color_bar_b( "hud_buildstate_color_bar_b", "255", FCVAR_ARCHIVE, "Bar color blue component");
static ConVar hud_buildstate_color_bar_a( "hud_buildstate_color_bar_a", "200", FCVAR_ARCHIVE, "Bar color alpha component");
static ConVar hud_buildstate_color_barBackground_r( "hud_buildstate_color_barBackground_r", "255", FCVAR_ARCHIVE, "Bar Background  background color red component");
static ConVar hud_buildstate_color_barBackground_g( "hud_buildstate_color_barBackground_g", "255", FCVAR_ARCHIVE, "Bar Background color green component");
static ConVar hud_buildstate_color_barBackground_b( "hud_buildstate_color_barBackground_b", "255", FCVAR_ARCHIVE, "Bar Background color blue component");
static ConVar hud_buildstate_color_barBackground_a( "hud_buildstate_color_barBackground_a", "96", FCVAR_ARCHIVE, "Bar Background color alpha component");
static ConVar hud_buildstate_color_barBorder_r( "hud_buildstate_color_barBorder_r", "255", FCVAR_ARCHIVE, "Bar Border color red component");
static ConVar hud_buildstate_color_barBorder_g( "hud_buildstate_color_barBorder_g", "255", FCVAR_ARCHIVE, "Bar Border color green component");
static ConVar hud_buildstate_color_barBorder_b( "hud_buildstate_color_barBorder_b", "255", FCVAR_ARCHIVE, "Bar Border color blue component");
static ConVar hud_buildstate_color_barBorder_a( "hud_buildstate_color_barBorder_a", "255", FCVAR_ARCHIVE, "Bar Border color alpha component");
static ConVar hud_buildstate_color_icon_r( "hud_buildstate_color_icon_r", "255", FCVAR_ARCHIVE, "Icon color red component");
static ConVar hud_buildstate_color_icon_g( "hud_buildstate_color_icon_g", "255", FCVAR_ARCHIVE, "Icon color green component");
static ConVar hud_buildstate_color_icon_b( "hud_buildstate_color_icon_b", "255", FCVAR_ARCHIVE, "Icon color blue component");
static ConVar hud_buildstate_color_icon_a( "hud_buildstate_color_icon_a", "255", FCVAR_ARCHIVE, "Icon color alpha component");
static ConVar hud_buildstate_color_label_r( "hud_buildstate_color_label_r", "255", FCVAR_ARCHIVE, "Label color red component");
static ConVar hud_buildstate_color_label_g( "hud_buildstate_color_label_g", "255", FCVAR_ARCHIVE, "Label color green component");
static ConVar hud_buildstate_color_label_b( "hud_buildstate_color_label_b", "255", FCVAR_ARCHIVE, "Label color blue component");
static ConVar hud_buildstate_color_label_a( "hud_buildstate_color_label_a", "255", FCVAR_ARCHIVE, "Label color alpha component");
static ConVar hud_buildstate_color_amount_r( "hud_buildstate_color_amount_r", "255", FCVAR_ARCHIVE, "Amount color red component");
static ConVar hud_buildstate_color_amount_g( "hud_buildstate_color_amount_g", "255", FCVAR_ARCHIVE, "Amount color green component");
static ConVar hud_buildstate_color_amount_b( "hud_buildstate_color_amount_b", "255", FCVAR_ARCHIVE, "Amount color blue component");
static ConVar hud_buildstate_color_amount_a( "hud_buildstate_color_amount_a", "255", FCVAR_ARCHIVE, "Amount color alpha component");

static ConVar hud_buildstate_shadow_icon( "hud_buildstate_shadow_icon", "0", FCVAR_ARCHIVE, "Icon Shadow (0 Off, 1 On)");
static ConVar hud_buildstate_shadow_label( "hud_buildstate_shadow_label", "0", FCVAR_ARCHIVE, "Label Shadow (0 Off, 1 On)");
static ConVar hud_buildstate_shadow_amount( "hud_buildstate_shadow_amount", "0", FCVAR_ARCHIVE, "Amount Shadow (0 Off, 1 On)");

static ConVar hud_buildstate_colorMode_bar( "hud_buildstate_colorMode_bar", "2", FCVAR_ARCHIVE, "Bar color mode (0=Custom, 1=Stepped Intensity, 2=Faded Intensity, 3=Team Coloured)");
static ConVar hud_buildstate_colorMode_barBackground( "hud_buildstate_colorMode_barBackground", "2", FCVAR_ARCHIVE, "Bar Background color mode (0=Custom, 1=Stepped Intensity, 2=Faded Intensity, 3=Team Coloured)");
static ConVar hud_buildstate_colorMode_barBorder( "hud_buildstate_colorMode_barBorder", "2", FCVAR_ARCHIVE, "Bar Border color mode (0=Custom, 1=Stepped Intensity, 2=Faded Intensity, 3=Team Coloured)");
static ConVar hud_buildstate_colorMode_icon( "hud_buildstate_colorMode_icon", "2", FCVAR_ARCHIVE, "Icon color mode (0=Custom, 1=Stepped Intensity, 2=Faded Intensity, 3=Team Coloured)");
static ConVar hud_buildstate_colorMode_label( "hud_buildstate_colorMode_label", "0", FCVAR_ARCHIVE, "Label color mode (0=Custom, 1=Stepped Intensity, 2=Faded Intensity, 3=Team Coloured)");
static ConVar hud_buildstate_colorMode_amount( "hud_buildstate_colorMode_amount", "0", FCVAR_ARCHIVE, "Amount color mode (0=Custom, 1=Stepped Intensity, 2=Faded Intensity, 3=Team Coloured)");

static ConVar hud_buildstate_offset_bar_x( "hud_buildstate_offset_bar_x", "0", FCVAR_ARCHIVE, "Bar offset x");
static ConVar hud_buildstate_offset_bar_y( "hud_buildstate_offset_bar_y", "0", FCVAR_ARCHIVE, "Bar offset y");
static ConVar hud_buildstate_offset_icon_x( "hud_buildstate_offset_icon_x", "-6", FCVAR_ARCHIVE, "Icon offset x");
static ConVar hud_buildstate_offset_icon_y( "hud_buildstate_offset_icon_y", "0", FCVAR_ARCHIVE, "Icon offset y");
static ConVar hud_buildstate_offset_label_x( "hud_buildstate_offset_label_x", "-12", FCVAR_ARCHIVE, "label offset x");
static ConVar hud_buildstate_offset_label_y( "hud_buildstate_offset_label_y", "0", FCVAR_ARCHIVE, "label offset y");
static ConVar hud_buildstate_offset_amount_x( "hud_buildstate_offset_amount_x", "30", FCVAR_ARCHIVE, "Amount offset x");
static ConVar hud_buildstate_offset_amount_y( "hud_buildstate_offset_amount_y", "0", FCVAR_ARCHIVE, "Amount offset y");

void CHudBuildStateBase::CheckCvars(bool updateBarPositions)
{
	if(!m_bChildOverride)
	{
		if(m_iX != hud_buildstate_x.GetInt() || m_iY != hud_buildstate_y.GetInt())
		{
			m_iX = hud_buildstate_x.GetInt();
			m_iY = hud_buildstate_y.GetInt();
		}

		if(m_iHorizontalAlign != hud_buildstate_align_horiz.GetInt())
			m_iHorizontalAlign = hud_buildstate_align_horiz.GetInt();

		if(m_iVerticalAlign != hud_buildstate_align_vert.GetInt())
			m_iVerticalAlign = hud_buildstate_align_vert.GetInt();

		if(m_qb_iColumns != hud_buildstate_columns.GetInt())
		{
			m_qb_iColumns = hud_buildstate_columns.GetInt();
			updateBarPositions = true;
		}	
	}
		
	if(m_qb_iSpacingX != hud_buildstate_spacing_x.GetInt() || m_qb_iSpacingY != hud_buildstate_spacing_y.GetInt())
	{
		m_qb_iSpacingX = hud_buildstate_spacing_x.GetInt();
		m_qb_iSpacingY = hud_buildstate_spacing_y.GetInt();
		updateBarPositions = true;
	}

	if(m_iHeaderTextX != hud_buildstate_pos_headerText_x.GetInt() || m_iHeaderTextY != hud_buildstate_pos_headerText_y.GetInt())
	{
		m_iHeaderTextX = hud_buildstate_pos_headerText_x.GetInt();
		m_iHeaderTextY = hud_buildstate_pos_headerText_y.GetInt();
		updateBarPositions = true;
	}

	if(m_iHeaderIconX != hud_buildstate_pos_headericon_x.GetInt() || m_iHeaderIconY != hud_buildstate_pos_headericon_y.GetInt())
	{
		m_iHeaderIconX = hud_buildstate_pos_headericon_x.GetInt();
		m_iHeaderIconY = hud_buildstate_pos_headericon_y.GetInt();
		updateBarPositions = true;
	}

	if(m_qb_iPositionX != hud_buildstate_pos_bars_x.GetInt() || m_qb_iPositionY != hud_buildstate_pos_bars_y.GetInt())
	{
		m_qb_iPositionX = hud_buildstate_pos_bars_x.GetInt();
		m_qb_iPositionY = hud_buildstate_pos_bars_y.GetInt();
		updateBarPositions = true;
	}

	if(m_qb_iIntensity_red != hud_buildstate_intensity_red.GetInt() || m_qb_iIntensity_orange != hud_buildstate_intensity_orange.GetInt() ||
		m_qb_iIntensity_yellow != hud_buildstate_intensity_yellow.GetInt() || m_qb_iIntensity_green != hud_buildstate_intensity_green.GetInt())
	{
		m_qb_iIntensity_red = hud_buildstate_intensity_red.GetInt();
		m_qb_iIntensity_orange = hud_buildstate_intensity_orange.GetInt();
		m_qb_iIntensity_yellow = hud_buildstate_intensity_yellow.GetInt();
		m_qb_iIntensity_green = hud_buildstate_intensity_green.GetInt();
		OnIntensityValuesChanged();
	}
	if(m_qb_bShowBar != hud_buildstate_show_Bar.GetBool())
	{
		m_qb_bShowBar = hud_buildstate_show_Bar.GetBool();
		OnShowBarChanged();
		updateBarPositions = true;
	}
	if(m_qb_bShowBarBackground != hud_buildstate_show_BarBackground.GetBool())
	{
		m_qb_bShowBarBackground = hud_buildstate_show_BarBackground.GetBool();
		OnShowBarBackgroundChanged();
		updateBarPositions = true;
	}
	if(m_qb_bShowBarBorder != hud_buildstate_show_BarBorder.GetBool())
	{
		m_qb_bShowBarBorder = hud_buildstate_show_BarBorder.GetBool();
		OnShowBarBorderChanged();
		updateBarPositions = true;
	}
	if(m_qb_bShowIcon != hud_buildstate_show_Icon.GetBool())
	{
		m_qb_bShowIcon = hud_buildstate_show_Icon.GetBool();
		OnShowIconChanged();
		updateBarPositions = true;
	}
	if(m_qb_bShowLabel != hud_buildstate_show_Label.GetBool())
	{
		m_qb_bShowLabel = hud_buildstate_show_Label.GetBool();
		OnShowLabelChanged();
		updateBarPositions = true;
	}
	if(m_qb_bShowAmount != hud_buildstate_show_Amount.GetBool())
	{
		m_qb_bShowAmount = hud_buildstate_show_Amount.GetBool();
		OnShowAmountChanged();
		updateBarPositions = true;
	}

	if(m_qb_iBarWidth != hud_buildstate_bar_width.GetInt() || m_qb_iBarHeight != hud_buildstate_bar_height.GetInt())
	{
		m_qb_iBarWidth = hud_buildstate_bar_width.GetInt();
		m_qb_iBarHeight = hud_buildstate_bar_height.GetInt();
		OnBarSizeChanged();
		updateBarPositions = true;
	}

	if(m_qb_iBarBorderWidth != hud_buildstate_bar_borderWidth.GetInt())
	{
		m_qb_iBarBorderWidth = hud_buildstate_bar_borderWidth.GetInt();
		OnBarBorderWidthChanged();
		updateBarPositions = true;
	}

	if(m_qb_iColorBar_r != hud_buildstate_color_bar_r.GetInt() || m_qb_iColorBar_g != hud_buildstate_color_bar_g.GetInt() ||
		m_qb_iColorBar_b != hud_buildstate_color_bar_b.GetInt() || m_qb_iColorBar_a != hud_buildstate_color_bar_a.GetInt())
	{
		m_qb_iColorBar_r = hud_buildstate_color_bar_r.GetInt();
		m_qb_iColorBar_g = hud_buildstate_color_bar_g.GetInt();
		m_qb_iColorBar_b = hud_buildstate_color_bar_b.GetInt();
		m_qb_iColorBar_a = hud_buildstate_color_bar_a.GetInt();
		OnColorBarChanged();
	}

	if(m_qb_iColorBarBackground_r != hud_buildstate_color_barBackground_r.GetInt() || m_qb_iColorBarBackground_g != hud_buildstate_color_barBackground_g.GetInt() ||
		m_qb_iColorBarBackground_b != hud_buildstate_color_barBackground_b.GetInt() || m_qb_iColorBarBackground_a != hud_buildstate_color_barBackground_a.GetInt())
	{
		m_qb_iColorBarBackground_r = hud_buildstate_color_barBackground_r.GetInt();
		m_qb_iColorBarBackground_g = hud_buildstate_color_barBackground_g.GetInt();
		m_qb_iColorBarBackground_b = hud_buildstate_color_barBackground_b.GetInt();
		m_qb_iColorBarBackground_a = hud_buildstate_color_barBackground_a.GetInt();
		OnColorBarBackgroundChanged();
	}

	if(m_qb_iColorBarBorder_r != hud_buildstate_color_barBorder_r.GetInt() || m_qb_iColorBarBorder_g != hud_buildstate_color_barBorder_g.GetInt() ||
		m_qb_iColorBarBorder_b != hud_buildstate_color_barBorder_b.GetInt() || m_qb_iColorBarBorder_a != hud_buildstate_color_barBorder_a.GetInt())
	{
		m_qb_iColorBarBorder_r = hud_buildstate_color_barBorder_r.GetInt();
		m_qb_iColorBarBorder_g = hud_buildstate_color_barBorder_g.GetInt();
		m_qb_iColorBarBorder_b = hud_buildstate_color_barBorder_b.GetInt();
		m_qb_iColorBarBorder_a = hud_buildstate_color_barBorder_a.GetInt();
		OnColorBarBorderChanged();
	}

	if(m_qb_iColorIcon_r != hud_buildstate_color_icon_r.GetInt() || m_qb_iColorIcon_g != hud_buildstate_color_icon_g.GetInt() ||
		m_qb_iColorIcon_b != hud_buildstate_color_icon_b.GetInt() || m_qb_iColorIcon_a != hud_buildstate_color_icon_a.GetInt())
	{
		m_qb_iColorIcon_r = hud_buildstate_color_icon_r.GetInt();
		m_qb_iColorIcon_g = hud_buildstate_color_icon_g.GetInt();
		m_qb_iColorIcon_b = hud_buildstate_color_icon_b.GetInt();
		m_qb_iColorIcon_a = hud_buildstate_color_icon_a.GetInt();
		OnColorIconChanged();
	}

	if(m_qb_iColorLabel_r != hud_buildstate_color_label_r.GetInt() || m_qb_iColorLabel_g != hud_buildstate_color_label_g.GetInt() ||
		m_qb_iColorLabel_b != hud_buildstate_color_label_b.GetInt() || m_qb_iColorLabel_a != hud_buildstate_color_label_a.GetInt())
	{
		m_qb_iColorLabel_r = hud_buildstate_color_label_r.GetInt();
		m_qb_iColorLabel_g = hud_buildstate_color_label_g.GetInt();
		m_qb_iColorLabel_b = hud_buildstate_color_label_b.GetInt();
		m_qb_iColorLabel_a = hud_buildstate_color_label_a.GetInt();
		OnColorLabelChanged();
	}
	
	if(m_qb_iColorAmount_r != hud_buildstate_color_amount_r.GetInt() || m_qb_iColorAmount_g != hud_buildstate_color_amount_g.GetInt() ||
		m_qb_iColorAmount_b != hud_buildstate_color_amount_b.GetInt() || m_qb_iColorAmount_a != hud_buildstate_color_amount_a.GetInt())
	{
		m_qb_iColorAmount_r = hud_buildstate_color_amount_r.GetInt();
		m_qb_iColorAmount_g = hud_buildstate_color_amount_g.GetInt();
		m_qb_iColorAmount_b = hud_buildstate_color_amount_b.GetInt();
		m_qb_iColorAmount_a = hud_buildstate_color_amount_a.GetInt();
		OnColorAmountChanged();
	}

	if(m_qb_bShadowIcon != hud_buildstate_shadow_icon.GetBool())
	{
		m_qb_bShadowIcon = hud_buildstate_shadow_icon.GetBool();
		OnIconShadowChanged();
	}
	if(m_qb_bShadowLabel != hud_buildstate_shadow_label.GetBool())
	{
		m_qb_bShadowLabel = hud_buildstate_shadow_label.GetBool();
		OnLabelShadowChanged();
	}
	if(m_qb_bShadowAmount != hud_buildstate_shadow_amount.GetBool())
	{
		m_qb_bShadowAmount = hud_buildstate_shadow_amount.GetBool();
		OnAmountShadowChanged();
	}

	if(m_qb_iColorModeBar != hud_buildstate_colorMode_bar.GetInt())
	{
		m_qb_iColorModeBar = hud_buildstate_colorMode_bar.GetInt();
		OnColorModeBarChanged();
	}
	if(m_qb_iColorModeBarBackground != hud_buildstate_colorMode_barBackground.GetInt())
	{
		m_qb_iColorModeBarBackground = hud_buildstate_colorMode_barBackground.GetInt();
		OnColorModeBarBackgroundChanged();
	}
	if(m_qb_iColorModeBarBorder != hud_buildstate_colorMode_barBorder.GetInt())
	{
		m_qb_iColorModeBarBorder = hud_buildstate_colorMode_barBorder.GetInt();
		OnColorModeBarBorderChanged();
	}
	if(m_qb_iColorModeIcon != hud_buildstate_colorMode_icon.GetInt())
	{
		m_qb_iColorModeIcon = hud_buildstate_colorMode_icon.GetInt();
		OnColorModeIconChanged();
	}
	if(m_qb_iColorModeLabel != hud_buildstate_colorMode_label.GetInt())
	{
		m_qb_iColorModeLabel = hud_buildstate_colorMode_label.GetInt();
		OnColorModeLabelChanged();
	}
	if(m_qb_iColorModeAmount != hud_buildstate_colorMode_amount.GetInt())
	{
		m_qb_iColorModeAmount = hud_buildstate_colorMode_amount.GetInt();
		OnColorModeAmountChanged();
	}

	if(m_qb_iOffsetIconX != hud_buildstate_offset_icon_x.GetInt() || m_qb_iOffsetIconY != hud_buildstate_offset_icon_y.GetInt())
	{
		m_qb_iOffsetIconX = hud_buildstate_offset_icon_x.GetInt();
		m_qb_iOffsetIconY = hud_buildstate_offset_icon_y.GetInt();
		OnIconOffsetChanged();
		updateBarPositions = true;
	}
	if(m_qb_iOffsetLabelX != hud_buildstate_offset_label_x.GetInt() || m_qb_iOffsetLabelY != hud_buildstate_offset_label_y.GetInt())
	{
		m_qb_iOffsetLabelX = hud_buildstate_offset_label_x.GetInt();
		m_qb_iOffsetLabelY = hud_buildstate_offset_label_y.GetInt();
		OnLabelOffsetChanged();
		updateBarPositions = true;
	}
	if(m_qb_iOffsetAmountX != hud_buildstate_offset_amount_x.GetInt() || m_qb_iOffsetAmountY != hud_buildstate_offset_amount_y.GetInt())
	{
		m_qb_iOffsetAmountX = hud_buildstate_offset_amount_x.GetInt();
		m_qb_iOffsetAmountY = hud_buildstate_offset_amount_y.GetInt();
		OnAmountOffsetChanged();
		updateBarPositions = true;
	}

	if(updateBarPositions)
		UpdateQBPositions();
}

void CHudBuildStateBase::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	m_hfText = pScheme->GetFont( "QuantityPanel", IsProportional() );
	
	SetPaintBackgroundType(2);
	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);
	SetPaintEnabled(false);

	UpdateQBPositions();
	BaseClass::ApplySchemeSettings( pScheme );
}

