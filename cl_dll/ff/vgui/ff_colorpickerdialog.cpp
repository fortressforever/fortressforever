#include "cbase.h"

#include "ff_colorpickerdialog.h"

#include "ff_shareddefs.h"

namespace vgui
{
	FFColorPickerDialog::FFColorPickerDialog( Panel *parent, const char* panelName, bool bColorModeIntensity ) : BaseClass(parent, panelName )
	{
		SetParent(parent);
		vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme");
		SetScheme( scheme );

		//Other useful options
		SetSizeable(false);
		SetMoveable(true);
		SetSize(300,200);

		// Centre this panel on the screen for consistency.
		int nWide = GetWide();
		int nTall = GetTall();

		SetPos((ScreenWidth() - nWide) / 2, (ScreenHeight() - nTall) / 2);
		SetVisible(false); //made visible on command later 

		m_pOKButton = new Button(this, "OKButton", "#GameUI_OK", this, "OK");
		m_pCancelButton = new Button(this, "CancelButton", "GameUI_Cancel", this, "Cancel");

		m_pRed = new CFFInputSlider(this, "Red", "RedInput");		
		m_pGreen = new CFFInputSlider(this, "Green", "GreenInput");		
		m_pBlue = new CFFInputSlider(this, "Blue", "BlueInput");		
		m_pAlpha = new CFFInputSlider(this, "Alpha", "AlphaInput");

		m_pRed->RemoveActionSignalTarget(this);
		m_pGreen->RemoveActionSignalTarget(this);
		m_pBlue->RemoveActionSignalTarget(this);
		m_pAlpha->RemoveActionSignalTarget(this);

		m_pRed->SetRange(0, 255);
		m_pGreen->SetRange(0, 255);
		m_pBlue->SetRange(0, 255);
		m_pAlpha->SetRange(0, 255);

		m_pRed->SetValue(255);
		m_pGreen->SetValue(255);
		m_pBlue->SetValue(255);
		m_pAlpha->SetValue(255);

		m_pColorMode = new ComboBox(this, "ColorModeCombo", 4, false);
		m_pColorMode->RemoveActionSignalTarget(this);

		KeyValues *kv = new KeyValues("Custom");
		kv->SetInt("Custom", 0);
		m_pColorMode->AddItem("#GameUI_Custom", kv);
		kv->deleteThis();
		if(bColorModeIntensity)
		{
			kv = new KeyValues("Stepped");
			kv->SetInt("Stepped", 1);
			m_pColorMode->AddItem("#GameUI_Stepped", kv);
			kv->deleteThis();
			kv = new KeyValues("Faded");
			kv->SetInt("Faded", 2);
			m_pColorMode->AddItem("#GameUI_Faded", kv);
			kv->deleteThis();
		}
		kv = new KeyValues("TeamColored");
		kv->SetInt("TeamColored", 3);
		m_pColorMode->AddItem("#GameUI_TeamColored", kv);
		kv->deleteThis();
		m_pColorMode->ActivateItemByRow(0);

		m_pColorBackground = new ImagePanel(this, "CrosshairBackground");
		m_pColorBackground->SetImage("crosshairbg");
		m_pColorBackground->SetZPos(-1);

		m_pColor = new Panel(this,"ColorPanel");

		m_iTeamColorPreview = TEAM_SPECTATOR;
		m_iRed = 255;
		m_iGreen = 255;
		m_iBlue = 255;
		m_iAlpha = 255;
		
		LoadControlSettings("resource/ui/FFColorPicker.res");
	}

	void FFColorPickerDialog::OnTick()
	{
		if(m_iTeamColorPreview == TEAM_GREEN)
		{
			m_iTeamColorPreview = TEAM_SPECTATOR;
		}
		else
		{
			++m_iTeamColorPreview;
		}
		
		RecalculateColorPreview();
	}

	int FFColorPickerDialog::GetRedComponentValue( )
	{
		return m_iRed;
	}
	int FFColorPickerDialog::GetGreenComponentValue( )
	{
		return m_iGreen;
	}
	int FFColorPickerDialog::GetBlueComponentValue( )
	{
		return m_iBlue;
	}
	int FFColorPickerDialog::GetAlphaComponentValue( )
	{
		return m_iAlpha;
	}

	void FFColorPickerDialog::SetRedComponentValue(int iRed)
	{
		m_pRed->RemoveActionSignalTarget(this);
		m_pRed->SetValue(iRed);
		m_pRed->AddActionSignalTarget(this);
		RecalculateColorPreview();
	}
	void FFColorPickerDialog::SetGreenComponentValue(int iGreen)
	{
		m_pGreen->RemoveActionSignalTarget(this);
		m_pGreen->SetValue(iGreen);
		m_pGreen->AddActionSignalTarget(this);
		RecalculateColorPreview();
	}
	void FFColorPickerDialog::SetBlueComponentValue(int iBlue)
	{
		m_pBlue->RemoveActionSignalTarget(this);
		m_pBlue->SetValue(iBlue);
		m_pBlue->AddActionSignalTarget(this);
		RecalculateColorPreview();
	}
	void FFColorPickerDialog::SetAlphaComponentValue(int iAlpha)
	{
		m_pAlpha->RemoveActionSignalTarget(this);
		m_pAlpha->SetValue(iAlpha);
		m_pAlpha->AddActionSignalTarget(this);
		RecalculateColorPreview();
	}
 
	void FFColorPickerDialog::SetValue( int iRed, int iGreen, int iBlue, int iAlpha )
	{
		m_iRed = iRed;
		m_iGreen = iGreen;
		m_iBlue = iBlue;
		m_iAlpha = iAlpha;

		m_pRed->RemoveActionSignalTarget(this);
		m_pGreen->RemoveActionSignalTarget(this);
		m_pBlue->RemoveActionSignalTarget(this);
		m_pAlpha->RemoveActionSignalTarget(this);

		m_pRed->SetValue(iRed, false);
		m_pGreen->SetValue(iGreen, false);
		m_pBlue->SetValue(iBlue, false);
		m_pAlpha->SetValue(iAlpha, false);

		m_pRed->AddActionSignalTarget(this);
		m_pGreen->AddActionSignalTarget(this);
		m_pBlue->AddActionSignalTarget(this);
		m_pAlpha->AddActionSignalTarget(this);

		RecalculateColorPreview();
	}
 
	void FFColorPickerDialog::RecalculateColorPreview()
	{
		KeyValues* colorMode = m_pColorMode->GetActiveItemUserData();
		switch(colorMode->GetInt(colorMode->GetName(),-1))
		{
			case 0:
				//custom
				m_pColor->SetBgColor( Color( m_pRed->GetValue(), m_pGreen->GetValue(), m_pBlue->GetValue(), m_pAlpha->GetValue() ) );
				break;
			case 3:
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

				m_pColor->SetBgColor( Color( teamColor.r(), teamColor.g(), teamColor.b(), m_pAlpha->GetValue() ) );
				break;
		}
	}

	 
	void FFColorPickerDialog::GetValue( int &iRed, int &iGreen, int &iBlue, int &iAlpha )
	{
		iRed = m_iRed;
		iGreen = m_iGreen;
		iBlue = m_iBlue;
		iAlpha = m_iAlpha;
	}

	void FFColorPickerDialog::SetVisible(bool visible)
	{
		if(visible)
		{
			m_pColorMode->AddActionSignalTarget(this);
			m_pRed->AddActionSignalTarget(this);
			m_pGreen->AddActionSignalTarget(this);
			m_pBlue->AddActionSignalTarget(this);
			m_pAlpha->AddActionSignalTarget(this);
		}
		RequestFocus();
		MoveToFront();
		BaseClass::SetVisible(visible);
	}

	void FFColorPickerDialog::OK()
	{
		SetVisible(false);
	}
	void FFColorPickerDialog::Cancel()
	{
		//revert back to the original values when ths dialog opened
		m_pRed->SetValue(m_iRed);
		m_pGreen->SetValue(m_iGreen);
		m_pBlue->SetValue(m_iBlue);
		m_pAlpha->SetValue(m_iAlpha);
		SetVisible(false);
	}

	void FFColorPickerDialog::OnButtonCommand(KeyValues *data)
	{
		const char *pszCommand = data->GetString("command");

		// Both Apply and OK save the options window stuff
		if (Q_strcmp(pszCommand, "OK") == 0)
		{
			m_iRed = m_pRed->GetValue();
			m_iGreen = m_pGreen->GetValue();
			m_iBlue = m_pBlue->GetValue();
			m_iAlpha = m_pAlpha->GetValue();
			OK();
		}
		else
		{
			Cancel();
		}
	}

	void FFColorPickerDialog::OnUpdateSliders(KeyValues *data)
	{
		if(data->GetPtr("panel") == m_pRed)
		{
			int iRed = m_pRed->GetValue();
			KeyValues* msg = new KeyValues("RedComponentChanged");
			msg->SetInt("Value", iRed);
			PostActionSignal ( msg );
		}
		else if(data->GetPtr("panel") == m_pGreen)
		{
			int iGreen = m_pGreen->GetValue();
			KeyValues* msg = new KeyValues("GreenComponentChanged");
			msg->SetInt("Value", iGreen);
			PostActionSignal ( msg );
		}
		else if(data->GetPtr("panel") == m_pBlue)
		{
			int iBlue = m_pBlue->GetValue();
			KeyValues* msg = new KeyValues("BlueComponentChanged");
			msg->SetInt("Value", iBlue);
			PostActionSignal ( msg );
		}
		else if(data->GetPtr("panel") == m_pAlpha)
		{
			int iAlpha = m_pAlpha->GetValue();
			KeyValues* msg = new KeyValues("AlphaComponentChanged");
			msg->SetInt("Value", iAlpha);
			PostActionSignal ( msg );
		}

		RecalculateColorPreview();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the comboboxs changing their selection
	//-----------------------------------------------------------------------------
	void FFColorPickerDialog::OnUpdateCombos(KeyValues *data)
	{
		if (data->GetPtr("panel") == m_pColorMode)
		{
			const char* m_szName = m_pColorMode->GetActiveItemUserData()->GetName();

			KeyValues* msg = new KeyValues("ColorModeChanged");
			msg->SetInt("Value", m_pColorMode->GetActiveItemUserData()->GetInt(m_szName, -1));
			PostActionSignal ( msg );

			if(Q_stricmp(m_szName, "Custom") == 0)
			{
				m_pRed->SetEnabled(true);
				m_pGreen->SetEnabled(true);
				m_pBlue->SetEnabled(true);
				ivgui()->RemoveTickSignal(GetVPanel());
			}
			else if(Q_stricmp(m_szName, "TeamColored") == 0)
			{
				m_pRed->SetEnabled(false);
				m_pGreen->SetEnabled(false);
				m_pBlue->SetEnabled(false);
				ivgui()->AddTickSignal(GetVPanel(), 1250);
			}
			else
			{
				m_pRed->SetEnabled(false);
				m_pGreen->SetEnabled(false);
				m_pBlue->SetEnabled(false);
				ivgui()->RemoveTickSignal(GetVPanel());
			}

			RecalculateColorPreview();
		}
	}
};