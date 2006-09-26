/********************************************************************
	created:	2006/09/22
	created:	22:9:2006   17:24
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_modelpanel.h
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
	file base:	ff_modelpanel
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	ModelPanel - display a static model
				PlayerModelPanel - display an animated player model
*********************************************************************/

#ifndef FF_MODELPANEL_H
#define FF_MODELPANEL_H

#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/Panel.h>
#include "color.h"

namespace vgui
{
	//=============================================================================
	// Static models
	//=============================================================================
	class ModelPanel : public Panel
	{
	public:
		DECLARE_CLASS_SIMPLE(ModelPanel, Panel);

		ModelPanel(Panel *pParent, const char *pName);
		virtual ~ModelPanel();

		virtual void Paint();
		virtual void SetModel(const char *pszPlayerModelName);
		virtual void SetupPositioningAndLighting(Vector &vecOrigin);
		virtual void DoRendering(Vector vecOrigin);
		virtual void DrawModels();
		virtual void Reset();

	public:
		
		char m_szModelName[128];
		//C_BaseAnimatingOverlay *m_pModel;

		CHandle<C_BaseAnimatingOverlay> m_hModel;
	};

	//=============================================================================
	// Animated player models
	//=============================================================================
	class PlayerModelPanel : public ModelPanel
	{
	public:
		DECLARE_CLASS_SIMPLE(PlayerModelPanel, ModelPanel);

		PlayerModelPanel(Panel *pParent, const char *pName);
		virtual ~PlayerModelPanel();

		virtual void Paint();
		virtual void DrawModels();

		void SetClass(const char *pszClassname);
		void SetAnimations(const char *pszUpper, const char *pszLower);
		void SetWeaponModel(const char *pszWeaponModelName);
		void Reset();

	private:

		//C_BaseAnimatingOverlay *m_pWeaponModel;
		CHandle<C_BaseAnimating> m_hWeaponModel;

		char m_szWeaponModelName[128];
		char m_szLowerAnim[128];
		char m_szUpperAnim[128];
	};
}

#endif