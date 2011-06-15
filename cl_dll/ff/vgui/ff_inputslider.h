/********************************************************************
	created:	2006/09/22
	created:	22:9:2006   20:04
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_button.h
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
	file base:	ff_inputslider
	file ext:	h
	author:		Adam "Elmo" Willden
	
	purpose:	This just keeps a slider hooked up with a text element.

	notes: 
	
	Used to reside in ff_options but I'm separting the classes into their own files
	Used to be named CInputSlider - why have FFButton and not CFFInputSlider...
	or CButton and CInputSlider. Unless I'm missing the point here.

*********************************************************************/


#ifndef FF_INPUTSLIDER_H
#define FF_INPUTSLIDER_H

#include "vgui_controls/Slider.h"
#include "vgui_controls/TextEntry.h"

namespace vgui
{
	class CFFInputSlider : public Slider
	{
		DECLARE_CLASS_SIMPLE(CFFInputSlider, Slider)

	public:

		//-----------------------------------------------------------------------------
		// Purpose: Link this slider in with its input box
		//-----------------------------------------------------------------------------
		CFFInputSlider(Panel *parent, char const *panelName, char const *inputName) : BaseClass(parent, panelName)
		{
			m_pInputBox = new TextEntry(parent, inputName);
			m_pInputBox->SetAllowNumericInputOnly(true);
			//m_pInputBox->SetEditable(false);
			m_pInputBox->AddActionSignalTarget(this);
			//m_pInputBox->SendNewLine(true);

			AddActionSignalTarget(parent);
		}

		//-----------------------------------------------------------------------------
		// Purpose: Transfer the value onto the input box
		//-----------------------------------------------------------------------------
		virtual void SetValue(int value, bool bTriggerChangeMessage = true)
		{
			m_pInputBox->SetText(VarArgs("%d", value));
			BaseClass::SetValue(value, bTriggerChangeMessage);
		}
		virtual void SetPos(int x, int y)
		{
			int iWide, iTall;
			GetSize(iWide, iTall);
			m_pInputBox->SetPos(x + iWide, y);
			BaseClass::SetPos(x,y);
		}

		//-----------------------------------------------------------------------------
		// Purpose: Keep the input box up to date
		//-----------------------------------------------------------------------------
		virtual void SetEnabled(bool state)
		{
			m_pInputBox->SetEnabled(state);
			BaseClass::SetEnabled(state);
		}

		//-----------------------------------------------------------------------------
		// Purpose: Keep the input box up to date
		//-----------------------------------------------------------------------------
		virtual void SetVisible(bool state)
		{
			m_pInputBox->SetVisible(state);
			BaseClass::SetVisible(state);
		}

	private:

		//-----------------------------------------------------------------------------
		// Purpose: Allow the input box to change this value
		//-----------------------------------------------------------------------------
		virtual void UpdateFromInput(int iValue, bool bTriggerChangeMessage = true)
		{
			BaseClass::SetValue(iValue, bTriggerChangeMessage);
		}

		//-----------------------------------------------------------------------------
		// Purpose: 
		//-----------------------------------------------------------------------------
		int GetInputValue()
		{
			if (m_pInputBox->GetTextLength() == 0)
				return -1;

			char szValue[5];
			m_pInputBox->GetText(szValue, 4);

			if (!szValue[0])
				return -1;

			int iValue = atoi(szValue);

			// Since text is disabled on this box (and hopefully Valve haven't messed that
			// up, the next checks aren't need

			// atoi returned zero so make sure that the box is 1 character long and that
			// character is 0, otherwise it could just be some text.
			//if (iValue == 0 && (szValue[0] != '0' || m_pInputBox->GetTextLength() != 1))
			//	return -1;

			// Make sure that this number is as long as the string
			//

			return iValue;
		}

		//-----------------------------------------------------------------------------
		// Purpose: Catch the text box being changed and update the slider
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnTextChanged, "TextChanged", data)
		{
			// Apparently this is a good check
			if (!m_pInputBox->HasFocus())
				return;

			int iValue = GetInputValue();

			int iMin, iMax;
			GetRange(iMin, iMax);

			iValue = clamp(iValue, iMin, iMax);

			UpdateFromInput(iValue, true);
		}

		//-----------------------------------------------------------------------------
		// Purpose: Don't let the box be left on invalid values
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnKillFocus, "TextKillFocus", data)
		{
			int iValue = GetInputValue();

			int iMin, iMax;
			GetRange(iMin, iMax);

			iValue = clamp(iValue, iMin, iMax);

			m_pInputBox->SetText(VarArgs("%d", iValue));
		}

		TextEntry *m_pInputBox;
	};
}
#endif