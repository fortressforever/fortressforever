// simple.cpp
//
// Simple application that shows usage of the lgLcd library
// to connect to the Logitech LCD monitor, enumerate and
// open a device, display a bitmap and read soft buttons
//
// Part of the lgLcd SDK package
//
// Copyright 2005 Logitech Inc.
//

// first, we need to include the necessary header files
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>

// include the Logitech LCD SDK header
#include "../../include/lglcd.h"
// make sure we use the library
#pragma comment(lib, "lgLcd.lib")

// and include our sample bitmap
#include "samplebitmap.inl"


void HandleError(DWORD res, LPCTSTR msg)
{
    if(ERROR_SUCCESS != res)
    {
        LPTSTR lpMsgBuf;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            res,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL);
        _tprintf(_T("%s: error 0x%08x occurred:\n%s\n"), msg, res, lpMsgBuf);
        LocalFree(lpMsgBuf);
        _tprintf(_T("press enter to exit\n"));
        _gettchar();
        exit(1);
    }
}
int main(int argc, char* argv[])
{
    // uncomment the following line if you don't want to have
    // the simulator window - by default it is shown in this alpha release
    //lgLcdSimUseWindow(FALSE);

    DWORD res;

    //// initialize the library
    res = lgLcdInit();
    HandleError(res, _T("lgLcdInit"));
    
    //// connect to LCDMon
    // set up connection context
    lgLcdConnectContext connectContext;
    ZeroMemory(&connectContext, sizeof(connectContext));
    connectContext.appFriendlyName = _T("simple sample");
    connectContext.isAutostartable = TRUE;
    connectContext.isPersistent = TRUE;
    // we don't have a configuration screen
    connectContext.onConfigure.configCallback = NULL;
    connectContext.onConfigure.configContext = NULL;
    // the "connection" member will be returned upon return
    connectContext.connection = LGLCD_INVALID_CONNECTION;
    // and connect
    res = lgLcdConnect(&connectContext);
    HandleError(res, _T("lgLcdConnect"));

    // now we are connected (and have a connection handle returned),
    // let's enumerate an LCD (the first one, index = 0)
    lgLcdDeviceDesc deviceDescription;
    res = lgLcdEnumerate(connectContext.connection, 0, &deviceDescription);
    HandleError(res, _T("lgLcdEnumerate"));

    // at this point, we have an LCD
    _tprintf(_T("Found an LCD with %dx%d pixels, %d bits per pixel and %d soft buttons\n"),
        deviceDescription.Width, deviceDescription.Height, deviceDescription.Bpp,
        deviceDescription.NumSoftButtons);

    // open it
    lgLcdOpenContext openContext;
    ZeroMemory(&openContext, sizeof(openContext));
    openContext.connection = connectContext.connection;
    openContext.index = 0;
    // we have no softbutton notification callback
    openContext.onSoftbuttonsChanged.softbuttonsChangedCallback = NULL;
    openContext.onSoftbuttonsChanged.softbuttonsChangedContext = NULL;
    // the "device" member will be returned upon return
    openContext.device = LGLCD_INVALID_DEVICE;
    res = lgLcdOpen(&openContext);
    HandleError(res, _T("lgLcdEnumerate"));

    // coming back from lgLcdOpen, we have a device handle (in openContext.device)
    // which we will be using from now on until the program exits

    // load a bitmap onto the display (defined in samplebitmap.cpp)
    lgLcdBitmap160x43x1 bmp;
    bmp.hdr.Format = LGLCD_BMP_FORMAT_160x43x1;
    memcpy(&bmp.pixels, samplebitmap, sizeof(bmp.pixels));
    res = lgLcdUpdateBitmap(openContext.device, &bmp.hdr, LGLCD_SYNC_UPDATE(LGLCD_PRIORITY_NORMAL));
    HandleError(res, _T("lgLcdUpdateBitmap"));

    _tprintf(_T("bitmap updated successfully, press enter to continue\n"));
    _gettchar();

    // clear the display
    ZeroMemory(&bmp.pixels, sizeof(bmp.pixels));
    res = lgLcdUpdateBitmap(openContext.device, &bmp.hdr, LGLCD_SYNC_UPDATE(LGLCD_PRIORITY_NORMAL));
    HandleError(res, _T("lgLcdUpdateBitmap"));

    // let's close the device again
    res = lgLcdClose(openContext.device);
    HandleError(res, _T("lgLcdClose"));

    // and take down the connection
    res = lgLcdDisconnect(connectContext.connection);
    HandleError(res, _T("lgLcdDisconnect"));

    // and shut down the library
    res = lgLcdDeInit();
    HandleError(res, _T("lgLcdDeInit"));

    _tprintf(_T("sample completed successfully, press enter to exit\n"));
    _gettchar();
    return 0;
}
