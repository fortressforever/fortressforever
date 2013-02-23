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

	FFQuantityItem *m_qiDispenserHealth;
	FFQuantityItem *m_qiDispenserAmmo;
	FFQuantityItem *m_qiCellCounter;

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