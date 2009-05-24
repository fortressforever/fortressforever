// EZ_LCD_WRAPPER.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include "EZ_LCD.h"

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE /*hPrevInstance*/,
                     LPSTR     /*lpCmdLine*/,
                     int       /*nCmdShow*/)
{
    // Create instance of EzLcd. Initialize friendly name. Initialize size to size of LCD.
 	CEzLcd* lcd = new CEzLcd(_T("EZ LCD Wrapper Test"), 160, 43);
	
	///////////////////////////////
	// Static and scrolling Text //
	///////////////////////////////
    HANDLE titleScrolling = lcd->AddText(LG_SCROLLING_TEXT, LG_BIG, DT_CENTER, 160);
	lcd->SetOrigin(titleScrolling, 0, 2);

 	HANDLE text1Static = lcd->AddText(LG_STATIC_TEXT, LG_MEDIUM, DT_LEFT, 80);
	lcd->SetOrigin(text1Static, 0, 18);

    // Visible test
	//lcd->SetVisible(text1Static, FALSE);

	HANDLE text2Static = lcd->AddText(LG_STATIC_TEXT, LG_SMALL, DT_RIGHT, 80);
	lcd->SetOrigin(text2Static, 80, 20);

	//////////
	// Icon //
	//////////
	HICON hIcon = static_cast<HICON>(LoadImage(hInstance, _T("DemoFiles\\icon_high_volume.ico"), IMAGE_ICON, 16, 16, LR_LOADFROMFILE));
	HANDLE icon = lcd->AddIcon(hIcon, 16, 16);
	lcd->SetOrigin(icon, 2, 32);

	//////////////////
	// Progress Bar //
	//////////////////
	HANDLE progressBar = lcd->AddProgressBar(LG_CURSOR);
    //HANDLE progressBar = lcd->AddProgressBar(LG_FILLED);
	lcd->SetProgressBarSize(progressBar, 120, 5);
	lcd->SetOrigin(progressBar, 20, 36);

    // temp variable to update progress bar cursor position
    float tempCounter = 0.0f;

    // main loop
	while(1)
    {
		if (lcd->IsConnected())
		{
            // text can be updated any time.
            lcd->SetText(titleScrolling, _T("This title needs scrolling"));
			lcd->SetText(text1Static, _T("Medium size"));
			lcd->SetText(text2Static, _T("Small size"));
			lcd->SetProgressBarPosition(progressBar, (++tempCounter/10));

            // Check if button 1 triggered, pressed, or released
            if (lcd->ButtonTriggered(LG_BUTTON_1))
            {
                TRACE(_T("Button 1 triggered\n"));
            }

            if (lcd->ButtonReleased(LG_BUTTON_1))
            {
                TRACE(_T("Button 1 released\n"));
            }

            if (lcd->ButtonIsPressed(LG_BUTTON_1))
            {
                TRACE(_T("Button 1 is pressed\n"));
            }
		}

		// Must run the following every loop.
		lcd->Update();
		
		Sleep(50);
	}

    delete lcd;

	return 0;
}
