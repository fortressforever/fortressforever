Logitech EZ LCD Wrapper for PC
Copyright (C) 2005 Logitech Inc. All Rights Reserved


Introduction
--------------------------------------------------------------------------

This package is aimed at any PC games and user-created applets and 
contains the following components:

- Logitech EZ LCD Wrapper Source Files
- Documentation

It essentially makes it very easy for the developer to display simple 
strings, progress bars and icons. The wrapper is built on top of another 
layer, called LCDUI (also contained in the SDK package under 
samples/shared/lcdui). That layer offers more possibilites in case the 
wrapper turns out to be too restrictive. In particular it enables to 
choose any font and font size. If you want to use a different font and 
size from the ones proposed in the wrapper, see the wrapper source code 
and modify it as needed.

The environment for use of this package is as follows:
1. Visual C++ 6.0 to build and run demo
2. Drivers installed for G-series keyboards
3. Logitech LCD SDK


Disclaimer
--------------------------------------------------------------------------
This is work in progress. If you find anything wrong with either 
documentation or code, please let us know so we can improve on it.


Where to start
--------------------------------------------------------------------------

For a demo program to see what some forces do:
1. Install the Logitech LCD SDK
2. Click on "EZ_LCD_WRAPPER.dsp" (in samples/EZ_LCD_Wrapper)
3. Plug in a Logitech G-series keyboard with LCD (if using the Alpha LCD SDK
this is not necessary since the LCD will also appear as a window).
4. Compile and run

For implementing LCD support in your game/applet:
1. Install Logitech LCD SDK and include paths to include/lglcd.h and 
lib/x86/lglcd.lib in your project.
2. Include all source files contained in the package except 
EZ_LCD_Wrapper.cpp.
3. See EZ_LCD_Wrapper.cpp for a sample program using all of the wrapper's 
methods.


Where to find documentation
--------------------------------------------------------------------------

In "Doc" directory: EzLcd.cpp.html contains detailed info for all the 
available methods.


Notes
--------------------------------------------------------------------------
Complete list of EZ LCD Wrapper source files to include in your project:

In samples/EZ_LCD_Wrapper directory:
EZ_LCD.h, EZ_LCD.cpp

In samples/shared/lcdui directory: 
- all files

For questions/problems/suggestions email to:
cj@wingmanteam.com
roland@wingmanteam.com

