#include "cbase.h"
#include "ff_quantitypanel.h"

//#include "c_ff_player.h" //required to cast base player

class CHudBuildStateBase : public vgui::FFQuantityPanel
{
	DECLARE_CLASS_SIMPLE( CHudBuildStateBase, vgui::FFQuantityPanel );

public:
	CHudBuildStateBase(vgui::Panel* parent, const char *pElementName) : vgui::FFQuantityPanel(parent, pElementName) {}

	~CHudBuildStateBase( void ) {}

	virtual void CheckCvars(bool updateBarPositions = false);
};