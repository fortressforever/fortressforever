#include "cbase.h"
#include "ff_quantitypanel.h"

//#include "c_ff_player.h" //required to cast base player

#include "iclientmode.h" //to set panel parent as the cliends viewport

class CHudBuildStateBase : public vgui::FFQuantityPanel
{
	DECLARE_CLASS_SIMPLE( CHudBuildStateBase, vgui::FFQuantityPanel );

public:
	CHudBuildStateBase(vgui::Panel* parent, const char *pElementName) : vgui::FFQuantityPanel(parent, pElementName)
	{
		SetParent(g_pClientMode->GetViewport());

		m_flScale = 1.0f;
	}

	~CHudBuildStateBase( void ) {}

	void CheckCvars(bool updateBarPositions = false);
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

protected:
	
	vgui::HFont m_hfText;
};