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

	FFQuantityItem *m_qiSentryHealth;
	FFQuantityItem *m_qiSentryLevel;
	FFQuantityItem *m_qiCellCounter;

	bool	m_bBuilt;
	bool	m_bBuilding;
	wchar_t* m_wszNotBuiltText;
	wchar_t* m_wszBuildingText;
	int		m_iHideCells;
	int		m_iHideLevel;
	int		m_iShowPanel;

	enum displayOptions
	{
		NEVER = 0,
		ALWAYS,
		IF_BUILT
	};
public:
	CHudBuildStateSentry(const char *pElementName);
	~CHudBuildStateSentry();

	KeyValues* AddPanelSpecificOptions(KeyValues *kvOptions);
	KeyValues* GetDefaultStyleData();

	void Init( void );
	void VidInit( void );
	void OnTick( void );

	void OnStyleDataRecieved( KeyValues *kvStyleData );
};

#endif