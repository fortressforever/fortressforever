#ifndef FF_HUDBUILDSTATEDETPACK_H
#define FF_HUDBUILDSTATEDETPACK_H

#include "ff_quantitypanel.h"
#include "hudelement.h"

#include "hud_macros.h"

#include "iclientmode.h" //to set panel parent as the clients viewport
#include "c_ff_player.h" //required to cast base player

#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>

using namespace vgui;

class CHudBuildStateDetpack : public CHudElement, public FFQuantityPanel
{
private:
	DECLARE_CLASS_SIMPLE( CHudBuildStateDetpack, FFQuantityPanel );

	FFQuantityItem *m_qiDetpackTimeLeft;

	bool	m_bBuilt;
	bool	m_bBuilding;
	float	m_flDetonateTime;
	wchar_t* m_wszNotBuiltText;
	wchar_t* m_wszBuildingText;
public:
	CHudBuildStateDetpack(const char *pElementName);
	~CHudBuildStateDetpack();

	KeyValues* GetDefaultStyleData();

	void Init( void );
	void VidInit( void );
	void OnTick( void );
	void Paint( void );

	void	MsgFunc_DetpackMsg(bf_read &msg);
};

#endif