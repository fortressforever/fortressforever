#include "cbase.h"

#include "ff_hud_buildstate_base.h"
#include "hudelement.h"

#include "hud_macros.h"

#include "iclientmode.h" //to set panel parent as the clients viewport
#include "c_ff_player.h" //required to cast base player

#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>

static ConVar hud_buildstate_sg_override( "hud_buildstate_sg_overrideSettings", "1", FCVAR_ARCHIVE, "Whether positioning, alignment and columns should override standard buildstate settings", true, 0, true, 1);
static ConVar hud_buildstate_sg_x( "hud_buildstate_sg_x", "640", FCVAR_ARCHIVE, "Panel's X position on 640 480 Resolution", true, 0, true, 640);
static ConVar hud_buildstate_sg_y( "hud_buildstate_sg_y", "0", FCVAR_ARCHIVE, "Panel's Y Position on 640 480 Resolution", true, 0, true, 480);
static ConVar hud_buildstate_sg_align_horiz( "hud_buildstate_sg_align_horiz", "2", FCVAR_ARCHIVE, "Panel's horizontal alignment to the specified position (0=left, 1=center, 2=right", true, 0, true, 2);
static ConVar hud_buildstate_sg_align_vert( "hud_buildstate_sg_align_vert", "0", FCVAR_ARCHIVE, "Panel's vertical alignment to the specified position (0=top, 1=middle, 2=bottom", true, 0, true, 2);
static ConVar hud_buildstate_sg_columns( "hud_buildstate_sg_columns", "1", FCVAR_ARCHIVE, "Number of quantity bar columns", true, 1, true, 6);

using namespace vgui;

class CHudBuildStateSentry : public CHudElement, public CHudBuildStateBase
{
private:
	DECLARE_CLASS_SIMPLE( CHudBuildStateSentry, CHudBuildStateBase );

	FFQuantityBar *m_qbSentryHealth;
	FFQuantityBar *m_qbSentryLevel;

	bool	m_bBuilt;
public:
	CHudBuildStateSentry(const char *pElementName);
	~CHudBuildStateSentry();

	void Init( void );
	void VidInit( void );
	void OnTick( void );
	void Paint( void );
	
	void	MsgFunc_SentryMsg(bf_read &msg);

protected:
	virtual void CheckCvars();
};

DECLARE_HUDELEMENT(CHudBuildStateSentry);
DECLARE_HUD_MESSAGE(CHudBuildStateSentry, SentryMsg);
