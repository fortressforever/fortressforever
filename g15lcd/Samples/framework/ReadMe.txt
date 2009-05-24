//************************************************************************
// 
// Framework Sample
//
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

This sample utilizes the LCDUI Framework classes to create an LCD
Applet. The following Framework classes are used:

CLCDOutput
	- LCD Output Class (sends data to the LCD device(s))

CLCDManager
	- Screen Class (manages a screen of LCD data)

CLCDText
	- LCD Text Class (wrapper for displaying text)

CLCDIcon
	- LCD Icon Class (wrapper for displaying icons)

CLCDBitmap
	- LCD Bitmap Class (wrapper for displaying bitmaps)


The purpose is to explain how to create a simple Applet that displays a
simple text string, has a dialog interface to set the properties, and
allows for button handling.

By default, a simple "Hello World" is displayed. The configuration screen
allows the text and font to be modified.

Each LCD button has a bitmap assigned to it, and pushing the respective
button causes that bitmap to be shown.

The sample also shows how to register a callback handler with the LCD
Control Panel (located in the Windows Control Panel as "Logitech G-series LCD").
Selecting the "LCDUI Framework Sample" and pushing the "Configure" button
causes the callback to invoked.
