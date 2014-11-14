# [Fortress Forever](http://www.fortress-forever.com)

A Team Fortress mod on the Source Engine (Source SDK 2006)

### Compiling

Fortress Forever must be compiled using Visual C++ 2005. The following instructions are an updated version of Microsoft's old [Using Visual C++ 2005 Express Edition with the Microsoft Platform SDK](https://web.archive.org/web/20070210205738/http://msdn.microsoft.com/vstudio/express/visualc/usingpsdk/) guide

1. **Install Visual C++ 2005**
  * Download and install [Visual Studio C++ 2005 Express Edition](http://download.microsoft.com/download/8/3/a/83aad8f9-38ba-4503-b3cd-ba28c360c27b/ENU/vcsetup.exe) (or the full version if you have it)
  * Download and install the [Visual Studio 2005 Express Editions Service Pack 1](http://www.microsoft.com/en-us/download/details.aspx?id=804)
  * Download and install the [Visual Studio 2005 Service Pack 1 Update for Windows Vista](http://www.microsoft.com/en-us/download/details.aspx?id=7524)
2. **Install the Microsoft Platform SDK**
  * Download and install the [Windows Server 2003 SP1 Platform SDK](http://www.microsoft.com/en-us/download/details.aspx?id=6510) 
    * You'll probably want the amd64 version; only get the x86 version if you are running a 32 bit version of Windows
    * You only need to install *Microsoft Windows Core SDK*
3. **Configure Visual C++ 2005**
  * Open Visual Studio C++ 2005
  * Update *VC++ Directories* in the *Projects and Solutions* section of the *Tools* -> *Options* dialog box. 
    * Add the paths to the appropriate subsection:
      * Executable files: `C:\Program Files\Microsoft Platform SDK\Bin`
      * Include files: `C:\Program Files\Microsoft Platform SDK\Include`
      * Library files: `C:\Program Files\Microsoft Platform SDK\Lib`
    * **Note:** Alternatively, you can update the Visual C++ Directories by modifying the `VCProjectEngine.dll.express.config` file located in the `\vc\vcpackages` subdirectory of the Visual C++ Express install location. Please make sure that you also delete the file `vccomponents.dat` located in the `%USERPROFILE%\Local Settings\Application Data\Microsoft\VCExpress\8.0` directory if it exists before restarting Visual C++ Express Edition. 
  * Update the `corewin_express.vsprops` file found in `C:\Program Files (x86)\Microsoft Visual Studio 8\VC\VCProjectDefaults`.
    * Edit `corewin_express.vsprops` in the text editor of your choice.
      * Change the string that reads: 
      `AdditionalDependencies="kernel32.lib"`
      to
      `AdditionalDependencies="kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib"`
    * **Note:** You might need to change the file permissions to be able to edit the corewin_express.vsprops file. To do so, right click `corewin_express.vsprops` and select *Properties*. Open the *Security* tab and click the *Edit...* button. Select the *Users (computername\User)* group and check *Write* in the *Allow* column, then click *OK* twice
  * Restart Visual C++ 2005
4. **Build Fortress Forever**
  * Open `Game_Scratch-2005.sln` and run *Build Solution*

#### Addendum: Registering VC++ 2005 Express

The Microsoft registration servers for VC++ 2005 Express edition are no longer online, so the only way to register your copy is to edit the registry. Simply save the following text as `register_vc.reg` and run it:
```
Windows Registry Editor Version 5.00

[HKEY_CURRENT_USER\Software\Microsoft\VCExpress\8.0\Registration]
"Params"="487A8D4D0000000001000000010000009F6A4D0000000000"
```