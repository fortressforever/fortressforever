#include <vgui_controls/PropertyPage.h>

#ifndef FF_OPTIONSPAGE_H
#define FF_OPTIONSPAGE_H

class CFFOptionsPage : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CFFOptionsPage, vgui::PropertyPage);

public:
	virtual void	Apply() = 0;
	virtual void	Load() = 0;
	virtual void	Reset() = 0;

	CFFOptionsPage(Panel *parent, char const *panelName) : BaseClass(parent, panelName) {}
};
#endif