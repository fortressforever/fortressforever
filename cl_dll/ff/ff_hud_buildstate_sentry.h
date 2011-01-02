#ifndef FF_HUDBUILDSTATESENTRY_H
#define FF_HUDBUILDSTATESENTRY_H

#include "ff_quantitypanel.h"
#include "hudelement.h"

#include "hud_macros.h"

#include "iclientmode.h" //to set panel parent as the clients viewport
#include "c_ff_player.h" //required to cast base player

#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>

using namespace vgui;

class CHudBuildStateSentry : public CHudElement, public FFQuantityPanel
{
private:
	DECLARE_CLASS_SIMPLE( CHudBuildStateSentry, FFQuantityPanel );

	FFQuantityBar *m_qbSentryHealth;
	FFQuantityBar *m_qbSentryLevel;

	bool	m_bBuilt;
public:
	CHudBuildStateSentry(const char *pElementName);
	~CHudBuildStateSentry();

	KeyValues* GetDefaultStyleData();

	void Init( void );
	void VidInit( void );
	void OnTick( void );
	void Paint( void );
	
	void	MsgFunc_SentryMsg(bf_read &msg);
};

#endif