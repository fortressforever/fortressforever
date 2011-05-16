#ifndef FF_HUDBUILDSTATEMANCANNON_H
#define FF_HUDBUILDSTATEMANCANNON_H

#include "ff_quantitypanel.h"
#include "hudelement.h"

#include "hud_macros.h"

#include "iclientmode.h" //to set panel parent as the clients viewport
#include "c_ff_player.h" //required to cast base player

#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>

using namespace vgui;

class CHudBuildStateManCannon : public CHudElement, public FFQuantityPanel
{
private:
	DECLARE_CLASS_SIMPLE( CHudBuildStateManCannon, FFQuantityPanel );

	FFQuantityBar *m_qbManCannonHealth;

	bool	m_bBuilt;
	bool	m_bBuilding;
	wchar_t* m_wszNotBuiltText;
	wchar_t* m_wszBuildingText;
public:
	CHudBuildStateManCannon(const char *pElementName);
	~CHudBuildStateManCannon();

	KeyValues* GetDefaultStyleData();

	void Init( void );
	void VidInit( void );
	void OnTick( void );
	void Paint( void );

	void	MsgFunc_ManCannonMsg(bf_read &msg);
};

#endif