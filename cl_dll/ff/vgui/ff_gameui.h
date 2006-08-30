/********************************************************************
	created:	2006/08/29
	created:	29:8:2006   18:46
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_gameui.h
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
	file base:	ff_gameui
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	Some macros to speed up gameui elements
				Probably handy because we'll probably add a separate
				game ui window for the stats too.
*********************************************************************/

#ifndef FF_GAMEUI_H
#define FF_GAMEUI_H

#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose: Interface for our new vgui panels
//-----------------------------------------------------------------------------
class IGameUI
{
public:
	virtual void Create(vgui::VPANEL parent) = 0;
	virtual void Destroy() = 0;
	virtual vgui::Panel *GetPanel() = 0;
};


//-----------------------------------------------------------------------------
// Purpose: Handy macros, these are a good idea
//-----------------------------------------------------------------------------
#define DECLARE_GAMEUI(className, panelClassName, globalPanel) \
class className : public IGameUI \
	{ \
	private: \
		panelClassName *myPanel; \
	public: \
		className() \
		{ \
			myPanel = NULL; \
		} \
		void Create(vgui::VPANEL parent) \
		{ \
			myPanel = new panelClassName(parent); \
		} \
		void Destroy() \
		{ \
			if (myPanel) \
			{ \
				myPanel->SetParent((vgui::Panel *) NULL); \
				delete myPanel; \
			} \
		} \
		vgui::Panel *GetPanel() \
		{ \
			return myPanel; \
		} \
	}; \
	extern IGameUI *globalPanel

#define DEFINE_GAMEUI(className, panelClassName, globalPanel) \
	static className g_##className##Panel; \
	IGameUI *globalPanel = (IGameUI *) &g_##className##Panel

#endif