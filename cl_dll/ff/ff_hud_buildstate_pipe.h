#ifndef FF_HUDBUILDSTATEPIPE_H
#define FF_HUDBUILDSTATEPIPE_H

#include "ff_quantitypanel.h"
#include "hudelement.h"

#include "hud_macros.h"

#include "iclientmode.h" //to set panel parent as the clients viewport
#include "c_ff_player.h" //required to cast base player

#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>

using namespace vgui;

class CHudBuildStatePipe : public CHudElement, public FFQuantityPanel
{
private:
	DECLARE_CLASS_SIMPLE( CHudBuildStatePipe, FFQuantityPanel );

	FFQuantityItem *m_qiPipeLaid;

	bool	m_bBuilt;
	wchar_t* m_wszNotBuiltText;
	int		m_iNumPipes;
public:
	CHudBuildStatePipe(const char *pElementName);
	~CHudBuildStatePipe();

	KeyValues* GetDefaultStyleData();

	void Init( void );
	void VidInit( void );
	void OnTick( void );
	void Paint( void );

	void	MsgFunc_PipeMsg(bf_read &msg);
};

#endif