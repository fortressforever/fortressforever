#ifndef FF_HUDBUILDSTATEDISPENSER_H
#define FF_HUDBUILDSTATEDISPENSER_H

#include "ff_quantitypanel.h"
#include "hudelement.h"

#include "hud_macros.h"

#include "iclientmode.h" //to set panel parent as the clients viewport
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>

using namespace vgui;

class CHudBuildStateDispenser : public CHudElement, public FFQuantityPanel
{
private:
	DECLARE_CLASS_SIMPLE( CHudBuildStateDispenser, FFQuantityPanel );

	FFQuantityBar *m_qbDispenserHealth;
	FFQuantityBar *m_qbDispenserAmmo;
	FFQuantityBar *m_qbCellCounter;

	bool	m_bBuilt;
	bool	m_bBuilding;
	int		m_iMaxCells;
	wchar_t* m_wszNotBuiltText;
	wchar_t* m_wszBuildingText;
public:
	CHudBuildStateDispenser(const char *pElementName);
	~CHudBuildStateDispenser();

	KeyValues* GetDefaultStyleData();

	void Init( void );
	void VidInit( void );
	void OnTick( void );
	void Paint( void );
	
	void	MsgFunc_DispenserMsg(bf_read &msg);
};

#endif