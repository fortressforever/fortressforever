#include "cbase.h"
#include "ff_colorpicker.h"
#include "ff_quantityhelper.h"
#include "ff_shareddefs.h"
#include "keyvalues.h"

namespace vgui
{
	FFColorPicker::FFColorPicker(Panel *parent, const char *panelName, Panel *pActionSignalTarget, bool bColorModeIntensity) : BaseClass(parent, panelName)
	{
		m_pDialogButton = NULL;

		char inputSliderName[128];
		char inputSliderInputName[128];

		Q_strncpy( inputSliderName, panelName, 127 );
		Q_strncpy( inputSliderInputName, panelName, 127 );
		Q_strncat( inputSliderName, "Red", 127, COPY_ALL_CHARACTERS );
		Q_strncat( inputSliderInputName, "RedInput", 127, COPY_ALL_CHARACTERS );
		m_pRed = new CFFInputSlider(parent, inputSliderName, inputSliderInputName, this);
		m_pRed->SetRange(0, 255);
		m_pRed->SetValue(255);

		Q_strncpy( inputSliderName, panelName, 127 );
		Q_strncpy( inputSliderInputName, panelName, 127 );
		Q_strncat( inputSliderName, "Green", 127, COPY_ALL_CHARACTERS );
		Q_strncat( inputSliderInputName, "GreenInput", 127, COPY_ALL_CHARACTERS );
		m_pGreen = new CFFInputSlider(parent, inputSliderName, inputSliderInputName, this);
		m_pGreen->SetRange(0, 255);
		m_pGreen->SetValue(255);

		Q_strncpy( inputSliderName, panelName, 127 );
		Q_strncpy( inputSliderInputName, panelName, 127 );
		Q_strncat( inputSliderName, "Blue", 127, COPY_ALL_CHARACTERS );
		Q_strncat( inputSliderInputName, "BlueInput", 127, COPY_ALL_CHARACTERS );
		m_pBlue = new CFFInputSlider(parent, inputSliderName, inputSliderInputName, this);
		m_pBlue->SetRange(0, 255);
		m_pBlue->SetValue(255);

		Q_strncpy( inputSliderName, panelName, 127 );
		Q_strncpy( inputSliderInputName, panelName, 127 );
		Q_strncat( inputSliderName, "Alpha", 127, COPY_ALL_CHARACTERS );
		Q_strncat( inputSliderInputName, "AlphaInput", 127, COPY_ALL_CHARACTERS );
		m_pAlpha = new CFFInputSlider(parent, inputSliderName, inputSliderInputName, this);
		m_pAlpha->SetRange(0, 255);
		m_pAlpha->SetValue(255);
		
		char colorModeComboName[128];
		Q_strncpy( colorModeComboName, panelName, 127 );
		Q_strncat( colorModeComboName, "ColorMode", 127, COPY_ALL_CHARACTERS );
		m_pColorMode = new ComboBox(parent, colorModeComboName, 4, false);
		m_pColorMode->RemoveActionSignalTarget(parent);
		if(bColorModeIntensity)
		{
			KeyValues *kv = new KeyValues("Custom");
			kv->SetInt("Value", FFQuantityHelper::COLOR_MODE_CUSTOM);
			m_pColorMode->AddItem("#GameUI_Custom", kv);
			kv->deleteThis();
			kv = new KeyValues("Stepped");
			kv->SetInt("Value", FFQuantityHelper::COLOR_MODE_STEPPED);
			m_pColorMode->AddItem("#GameUI_Stepped", kv);
			kv->deleteThis();
			kv = new KeyValues("Faded");
			kv->SetInt("Value", FFQuantityHelper::COLOR_MODE_FADED);
			m_pColorMode->AddItem("#GameUI_Faded", kv);
			kv->deleteThis();
			kv = new KeyValues("TeamColored");
			kv->SetInt("Value", FFQuantityHelper::COLOR_MODE_TEAM);
			m_pColorMode->AddItem("#GameUI_TeamColored", kv);
			kv->deleteThis();
		}
		else
		{
			KeyValues *kv = new KeyValues("Custom");
			kv->SetInt("Value", FFQuantityHelper::COLOR_MODE_CUSTOM);
			m_pColorMode->AddItem("#GameUI_Custom", kv);
			kv->deleteThis();
			kv = new KeyValues("TeamColored");
			kv->SetInt("Value", FFQuantityHelper::COLOR_MODE_TEAM);
			m_pColorMode->AddItem("#GameUI_TeamColored", kv);
			kv->deleteThis();
		}
		m_pColorMode->ActivateItemByRow(0);
		m_pColorMode->AddActionSignalTarget(this);
		m_iColorMode = 0;

		char imagePanelName[128];
		Q_strncpy( imagePanelName, panelName, 127 );
		Q_strncat( imagePanelName, "Background", 127, COPY_ALL_CHARACTERS );
		m_pColorBackground = new ImagePanel(parent, imagePanelName);
		m_pColorBackground->SetImage("crosshairbg");
		m_pColorBackground->SetZPos(-1);

		m_iTeamColorPreview = TEAM_BLUE;

		if(pActionSignalTarget)
		{
			this->AddActionSignalTarget(pActionSignalTarget);
		}
	}

	FFColorPicker::FFColorPicker(Panel *parent, const char *panelName, const char *dialogButtonText, Panel *pActionSignalTarget, const char *pCmd, bool bColorModeIntensity) : BaseClass(parent, panelName)
	{
		m_iRed = 255;
		m_iGreen = 255;
		m_iBlue = 255;
		m_iAlpha = 255;

		char dialogButtonName[128];
		Q_strncpy( dialogButtonName, panelName, 127 );
		Q_strncat( dialogButtonName, "DialogButton", 127, COPY_ALL_CHARACTERS );
		m_pDialogButton = new Button(parent, dialogButtonName, dialogButtonText, pActionSignalTarget, pCmd);

		if(pActionSignalTarget)
		{
			this->AddActionSignalTarget(pActionSignalTarget);
		}
	}

	FFColorPicker::FFColorPicker(Panel *parent, const char *panelName, const wchar_t *dialogButtonText, Panel *pActionSignalTarget, const char *pCmd, bool bColorModeIntensity) : BaseClass(parent, panelName)
	{
		m_iRed = 255;
		m_iGreen = 255;
		m_iBlue = 255;
		m_iAlpha = 255;

		char dialogButtonName[128];
		Q_strncpy( dialogButtonName, panelName, 127 );
		Q_strncat( dialogButtonName, "DialogButton", 127, COPY_ALL_CHARACTERS );
		m_pDialogButton = new Button(parent, dialogButtonName, dialogButtonText, pActionSignalTarget, pCmd);

		m_pDialog = new FFColorPickerDialog(this, "FFColourPicker");
		m_pDialog->AddActionSignalTarget(this);

		if(pActionSignalTarget)
		{
			this->AddActionSignalTarget(pActionSignalTarget);
		}
	}

	
	void FFColorPicker::ApplySchemeSettings( vgui::IScheme *pScheme )
	{
		//static bool to only fire once on loading settings - else this seems to get called constantly?
		static bool bFirst = true;
		if(bFirst)
		{
			bFirst = false;
			SetBgColor( Color( m_iRed, m_iGreen, m_iBlue, m_iAlpha ) );
		}
	}
	
	void FFColorPicker::AddActionSignalTarget(Panel *messageTarget)
	{
		if(m_pDialogButton)
		{
			m_pDialogButton->AddActionSignalTarget(messageTarget);
		}
		BaseClass::AddActionSignalTarget(messageTarget);
	}

	void FFColorPicker::AddActionSignalTarget(VPANEL messageTarget)
	{
		if(m_pDialogButton)
		{
			m_pDialogButton->AddActionSignalTarget(messageTarget);
		}
		BaseClass::AddActionSignalTarget(messageTarget);
	}

	void FFColorPicker::SetRGBComponentsEnabled(bool bState)
	{
		m_pRed->SetEnabled(bState);
		m_pGreen->SetEnabled(bState);
		m_pBlue->SetEnabled(bState);
	}
	void FFColorPicker::RemoveActionSignalTarget(Panel *oldTarget)
	{
		if(m_pDialogButton)
		{
			m_pDialogButton->RemoveActionSignalTarget(oldTarget);
		}
		BaseClass::RemoveActionSignalTarget(oldTarget);
	}

	void FFColorPicker::SetColorPickerDialog(FFColorPickerDialog *newDialog)
	{
		if(m_pDialogButton)
		{
			//cant set color picker dialog if
			AssertMsg(m_pDialogButton, "Can't set color picker dialog if we're not a dialog based instance");
		}
		if(m_pDialog)
		{
			m_pDialog->DeletePanel();
		}
		m_pDialog = newDialog;
		m_pDialog->AddActionSignalTarget(this);
	}

	void FFColorPicker::OnDialogButtonClick(KeyValues *data)
	{
		m_pDialog->SetVisible(true);

		//m_pDialog->RemoveActionSignalTarget(this);	

		m_pDialog->AddActionSignalTarget(this);

		m_pDialog->SetValue(m_iRed, m_iGreen, m_iBlue, m_iAlpha);
	}

	void FFColorPicker::UpdateColor()
	{
		RecalculateColorPreview();
			
		KeyValues* msg = new KeyValues("ColorChanged");
		msg->SetPtr("panel", this);
		PostActionSignal ( msg );
	}

	int FFColorPicker::GetRedComponentValue( )
	{
		return m_iRed;
	}
	int FFColorPicker::GetGreenComponentValue( )
	{
		return m_iGreen;
	}
	int FFColorPicker::GetBlueComponentValue( )
	{
		return m_iBlue;
	}
	int FFColorPicker::GetAlphaComponentValue( )
	{
		return m_iAlpha;
	}

	int FFColorPicker::GetColorMode( )
	{
		return m_iColorMode;
	}
	void FFColorPicker::SetColorMode(int iColorMode)
	{
		if(!m_pDialogButton)
		{
			m_iColorMode = iColorMode;
			m_pColorMode->RemoveActionSignalTarget(this);
			if(m_pColorMode->GetItemCount() >= iColorMode)
			{
				m_pColorMode->ActivateItemByRow(iColorMode);
			}
			else
			{
				m_pColorMode->ActivateItemByRow(m_pColorMode->GetItemCount() - 1);
			}
			m_pColorMode->AddActionSignalTarget(this);
		}
	}

	void FFColorPicker::SetRedComponentValue(int iRed)
	{
		if(!m_pDialogButton)
		{
			m_pRed->RemoveActionSignalTarget(this);
			m_pRed->SetValue(iRed);
			m_pRed->AddActionSignalTarget(this);
		}
		m_iRed = iRed;
		SetBgColor( Color( m_iRed, m_iGreen, m_iBlue, m_iAlpha ) );
	}
	void FFColorPicker::SetGreenComponentValue(int iGreen)
	{
		if(!m_pDialogButton)
		{
			m_pGreen->RemoveActionSignalTarget(this);
			m_pGreen->SetValue(iGreen);
			m_pGreen->AddActionSignalTarget(this);
		}
		m_iGreen = iGreen;
		SetBgColor( Color( m_iRed, m_iGreen, m_iBlue, m_iAlpha ) );
	}
	void FFColorPicker::SetBlueComponentValue(int iBlue)
	{
		if(!m_pDialogButton)
		{
			m_pBlue->RemoveActionSignalTarget(this);
			m_pBlue->SetValue(iBlue);
			m_pBlue->AddActionSignalTarget(this);
		}
		m_iBlue = iBlue;
		SetBgColor( Color( m_iRed, m_iGreen, m_iBlue, m_iAlpha ) );
	}
	void FFColorPicker::SetAlphaComponentValue(int iAlpha)
	{
		if(!m_pDialogButton)
		{
			m_pAlpha->RemoveActionSignalTarget(this);
			m_pAlpha->SetValue(iAlpha);
			m_pAlpha->AddActionSignalTarget(this);
		}
		m_iAlpha = iAlpha;
		SetBgColor( Color( m_iRed, m_iGreen, m_iBlue, m_iAlpha ) );
	}

	void FFColorPicker::SetValue( int iRed, int iGreen, int iBlue, int iAlpha )
	{
		m_iRed = iRed;
		m_iGreen = iGreen;
		m_iBlue = iBlue;
		m_iAlpha = iAlpha;
		if(!m_pDialogButton)
		{
			m_pRed->RemoveActionSignalTarget(this);
			m_pRed->SetValue(iRed);
			m_pRed->AddActionSignalTarget(this);
			m_pGreen->RemoveActionSignalTarget(this);
			m_pGreen->SetValue(iGreen);
			m_pGreen->AddActionSignalTarget(this);
			m_pBlue->RemoveActionSignalTarget(this);
			m_pBlue->SetValue(iBlue);
			m_pBlue->AddActionSignalTarget(this);
			m_pAlpha->RemoveActionSignalTarget(this);
			m_pAlpha->SetValue(iAlpha);
			m_pAlpha->AddActionSignalTarget(this);
		}
		SetBgColor( Color( m_iRed, m_iGreen, m_iBlue, m_iAlpha ) );
	}

	void FFColorPicker::GetValue( int &iRed, int &iGreen, int &iBlue, int &iAlpha )
	{
		iRed = m_iRed;
		iGreen = m_iGreen;
		iBlue = m_iBlue;
		iAlpha = m_iAlpha;
	}

	//dialog callbacks
	void FFColorPicker::OnRedComponentChanged(KeyValues *data)
	{
		m_iRed = data->GetInt("Value");
		UpdateColor();
	}
	void FFColorPicker::OnGreenComponentChanged(KeyValues *data)
	{
		m_iGreen = data->GetInt("Value");
		UpdateColor();
	}
	void FFColorPicker::OnBlueComponentChanged(KeyValues *data)
	{
		m_iBlue = data->GetInt("Value");
		UpdateColor();
	}
	void FFColorPicker::OnAlphaComponentChanged(KeyValues *data)
	{
		m_iAlpha = data->GetInt("Value");
		UpdateColor();
	}
	void FFColorPicker::OnColorModeChanged(KeyValues *data)
	{
		m_iColorMode = data->GetInt("Value");
		UpdateColor();
	}

	//no-dialog callbacks	
	void FFColorPicker::OnUpdateSliders(KeyValues *data)
	{
		if(data->GetPtr("panel") == m_pRed)
		{
			m_iRed = m_pRed->GetValue();
			UpdateColor();
		}
		else if(data->GetPtr("panel") == m_pGreen)
		{
			m_iGreen = m_pGreen->GetValue();
			UpdateColor();
		}
		else if(data->GetPtr("panel") == m_pBlue)
		{
			m_iBlue = m_pBlue->GetValue();
			UpdateColor();
		}
		else if(data->GetPtr("panel") == m_pAlpha)
		{
			m_iAlpha = m_pAlpha->GetValue();
			UpdateColor();
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the comboboxs changing their selection
	//-----------------------------------------------------------------------------
	void FFColorPicker::OnUpdateCombos(KeyValues *data)
	{
		if (data->GetPtr("panel") == m_pColorMode)
		{
			KeyValues* kvSelectedColorMode = m_pColorMode->GetActiveItemUserData();
			const char* m_szName = kvSelectedColorMode->GetName();

			m_iColorMode = kvSelectedColorMode->GetInt("Value", 0);

			KeyValues* msg = new KeyValues("ColorModeChanged");
			msg->SetInt("Value", m_pColorMode->GetActiveItemUserData()->GetInt(m_szName, -1));
			PostActionSignal ( msg );

			if(Q_stricmp(m_szName, "Custom") == 0)
			{
				SetRGBComponentsEnabled(true);
				ivgui()->RemoveTickSignal(GetVPanel());
			}
			else if(Q_stricmp(m_szName, "TeamColored") == 0)
			{
				SetRGBComponentsEnabled(false);
				ivgui()->AddTickSignal(GetVPanel(), 1250);
			}
			else
			{
				SetRGBComponentsEnabled(false);
				ivgui()->RemoveTickSignal(GetVPanel());
			}

			RecalculateColorPreview();
		}
	}

	void FFColorPicker::OnTick()
	{
		if(m_iTeamColorPreview == TEAM_GREEN)
		{
			m_iTeamColorPreview = TEAM_BLUE;
		}
		else
		{
			++m_iTeamColorPreview;
		}
		
		RecalculateColorPreview();
	}

	void FFColorPicker::RecalculateColorPreview()
	{
		KeyValues* colorMode = m_pColorMode->GetActiveItemUserData();
		switch(colorMode->GetInt("Value",-1))
		{
			case FFQuantityHelper::COLOR_MODE_CUSTOM:
				//custom
				SetBgColor( Color( m_iRed, m_iGreen, m_iBlue, m_iAlpha ) );
				break;
			case FFQuantityHelper::COLOR_MODE_TEAM:
				//teamcolored
				Color teamColor;
				switch(m_iTeamColorPreview)
				{
					case TEAM_SPECTATOR:
					default:
						teamColor = TEAM_COLOR_SPECTATOR;
						break;
					case TEAM_BLUE:
						teamColor = TEAM_COLOR_BLUE;
						break;
					case TEAM_RED:
						teamColor = TEAM_COLOR_RED;
						break;
					case TEAM_YELLOW:
						teamColor = TEAM_COLOR_YELLOW;
						break;
					case TEAM_GREEN:
						teamColor = TEAM_COLOR_GREEN;
						break;
				}

				SetBgColor( Color( teamColor.r(), teamColor.g(), teamColor.b(), m_iAlpha ) );
				break;
		}
	}
};