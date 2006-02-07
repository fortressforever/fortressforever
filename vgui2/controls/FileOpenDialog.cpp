//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of vgui generic open file dialog
//
// $NoKeywords: $
//=============================================================================//
#define PROTECTED_THINGS_DISABLE

#include "winlite.h"
#undef GetCurrentDirectory
#include "filesystem.h"

#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <KeyValues.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui/IInput.h>

#include <vgui_controls/FileOpenDialog.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ListViewPanel.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/MenuItem.h>
#include <vgui_controls/Tooltip.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

#ifndef _WIN32
#error "This class is WIN32 specific, please port me"

#else

static bool __cdecl ListViewFileSortFunc(KeyValues *kv1, KeyValues *kv2)
{
	bool dir1 = kv1->GetInt("directory") == 1;
	bool dir2 = kv2->GetInt("directory") == 1;

	// if they're both not directories of files, return if dir1 is a directory (before files)
	if (dir1 != dir2)
	{
		return dir1;
	}

	const char *string1 = kv1->GetString("text");
	const char *string2 = kv2->GetString("text");
	return stricmp(string1, string2) < 0;
}

namespace vgui
{
class FileCompletionMenu : public Menu
{
public:
	FileCompletionMenu(Panel *parent, const char *panelName) : Menu(parent, panelName)
	{
	}

	// override it so it doesn't request focus
	virtual void SetVisible(bool state)
	{
		Panel::SetVisible(state);
	}

};

class FileCompletionEdit : public TextEntry
{
	DECLARE_CLASS_SIMPLE( FileCompletionEdit, TextEntry );
public:
	FileCompletionEdit(Panel *parent) : TextEntry(parent, NULL)
	{
		m_pDropDown = new FileCompletionMenu(this, NULL);
		m_pDropDown->AddActionSignalTarget(this);
	}

    ~FileCompletionEdit()
	{
		delete m_pDropDown;
	}

    int AddItem(const char *itemText, KeyValues *userData)
	{
		// when the menu item is selected it will send the custom message "SetText"
		return m_pDropDown->AddMenuItem(itemText, new KeyValues("SetText", "text", itemText), this, userData);
	}
	int AddItem(const wchar_t *itemText, KeyValues *userData)
	{
		// add the element to the menu
		// when the menu item is selected it will send the custom message "SetText"
		KeyValues *kv = new KeyValues("SetText");
		kv->SetWString("text", itemText);

		// get an ansi version for the menuitem name
		char ansi[128];
		localize()->ConvertUnicodeToANSI(itemText, ansi, sizeof(ansi));
		return m_pDropDown->AddMenuItem(ansi, kv, this, userData);
	}

	void DeleteAllItems()
	{
		m_pDropDown->DeleteAllItems();
	}

	int GetItemCount()
	{
		return m_pDropDown->GetItemCount();
	}

	int GetItemIDFromRow(int row)		// valid from [0, GetItemCount)
	{
		return m_pDropDown->GetMenuID(row);
	}

	int GetRowFromItemID(int itemID)
	{
		int i;
		for (i=0;i<GetItemCount();i++)
		{
			if (m_pDropDown->GetMenuID(i) == itemID)
				return i;
		}
		return -1;
	}
	
	virtual void PerformLayout()
	{
		BaseClass::PerformLayout();
	
		// move the menu to the correct place below the button
		int wide, tall;;
		GetSize(wide, tall);

		int x = 0;
		int y = tall;
		LocalToScreen(x, y);

		m_pDropDown->SetPos(x, y + 1);

		// reset the width of the drop down menu to be the width of this edit box
		m_pDropDown->SetFixedWidth(GetWide());
	}

	void OnSetText(const wchar_t *newtext)
	{
		// see if the combobox text has changed, and if so, post a message detailing the new text
		wchar_t wbuf[255];
		GetText(wbuf, 254);
		
		if ( wcscmp(wbuf, newtext) )
		{
			// text has changed
			SetText(newtext);
			// fire off that things have changed
			PostActionSignal(new KeyValues("TextChanged", "text", newtext));
			Repaint();
		}
	}

	virtual void OnKillFocus()
	{
		HideMenu();
		BaseClass::OnKillFocus();
	}

	void HideMenu(void)
	{
		// hide the menu
		m_pDropDown->SetVisible(false);
	}
	
	void ShowMenu(void)
	{
		// reset the dropdown's position
		m_pDropDown->InvalidateLayout();

		// make sure we're at the top of the draw order (and therefore our children as well)
		// this important to make sure the menu will be drawn in the foreground
		MoveToFront();

		// reset the drop down
		m_pDropDown->ClearCurrentlyHighlightedItem();

		// limit it to only 6
		if (m_pDropDown->GetItemCount() > 6)
		{
			m_pDropDown->SetNumberOfVisibleItems(6);
		}
		else
		{
			m_pDropDown->SetNumberOfVisibleItems(m_pDropDown->GetItemCount());
		}
		// show the menu
		m_pDropDown->SetVisible(true);

		Repaint();
	}

	virtual void OnKeyCodeTyped(KeyCode code)
	{
		if ( code == KEY_DOWN )
		{
			if (m_pDropDown->GetItemCount() > 0)
			{
				int menuID = m_pDropDown->GetCurrentlyHighlightedItem();
				int row = -1;
				if ( menuID == -1 )
				{
					row = m_pDropDown->GetItemCount() - 1;
				}
				else
				{
					row = GetRowFromItemID(menuID);
				}
				row++;
				if (row == m_pDropDown->GetItemCount())
				{
					row = 0;
				}
				menuID = GetItemIDFromRow(row);
				m_pDropDown->SetCurrentlyHighlightedItem(menuID);
				return;
			}
		}
		else if ( code == KEY_UP )
		{
			if (m_pDropDown->GetItemCount() > 0)
			{
				int menuID = m_pDropDown->GetCurrentlyHighlightedItem();
				int row = -1;
				if ( menuID == -1 )
				{
					row = 0;
				}
				else
				{
					row = GetRowFromItemID(menuID);
				}
				row--;
				if ( row < 0 )
				{
					row = m_pDropDown->GetItemCount() - 1;
				}
				menuID = GetItemIDFromRow(row);
				m_pDropDown->SetCurrentlyHighlightedItem(menuID);
				return;
			}
		}
		else if ( code == KEY_ESCAPE )
		{
			if ( m_pDropDown->IsVisible() )
			{
				HideMenu();
				return;
			}
		}
		BaseClass::OnKeyCodeTyped(code);
		return;
	}

	MESSAGE_FUNC_INT( OnMenuItemHighlight, "MenuItemHighlight", itemID )
	{
		char wbuf[80];
		if ( m_pDropDown->IsValidMenuID(itemID) )
		{
			m_pDropDown->GetMenuItem(itemID)->GetText(wbuf, 80);
		}
		else
		{
			wbuf[0] = 0;
		}
		SetText(wbuf);
		RequestFocus();
		GotoTextEnd();
	}

private:
	FileCompletionMenu *m_pDropDown;
};

} // namespace vgui

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
FileOpenDialog::FileOpenDialog(Panel *parent, const char *title, bool bOpenOnly) : Frame(parent, "FileOpenDialog")
{
	m_bOpenOnly = bOpenOnly;
	SetTitle(title, true);
	SetMinimizeButtonVisible(false);
	Q_strncpy(m_szLastPath, "c:\\", sizeof( m_szLastPath ) );

	// Get the list of available drives and put them in a menu here.
	// Start with the directory we are in.
	m_pFullPathEdit = new ComboBox(this, "FullPathEdit", 6, false);
	m_pFullPathEdit->GetTooltip()->SetTooltipFormatToSingleLine();

	// list panel
	m_pFileList = new ListViewPanel(this, "FileList");
	m_pFileList->SetSortFunc(ListViewFileSortFunc);

	// file name edit box
	m_pFileNameEdit = new FileCompletionEdit(this); 
	m_pFileNameEdit->AddActionSignalTarget(this);

	m_pFileTypeCombo = new ComboBox(this, "FileTypeCombo", 6, false);

	m_pOpenButton = new Button(this, "OpenButton", "#FileOpenDialog_Open");
	m_pCancelButton = new Button(this, "CancelButton", "#FileOpenDialog_Cancel");
	m_pFolderUpButton = new Button(this, "FolderUpButton", "");
	Label *lookIn  = new Label(this, "LookInLabel", "#FileOpenDialog_Look_in");
	Label *fileName = new Label(this, "FileNameLabel", "#FileOpenDialog_File_name");
	Label *fileType = new Label(this, "FileTypeLabel", "#FileOpenDialog_File_type");
	m_pFolderIcon = new ImagePanel(NULL, "FolderIcon");

	// set up the control's initial positions
	SetSize(426, 260);

	m_pFullPathEdit->SetBounds(67, 32, 310, 24);
	m_pFolderUpButton->SetBounds(392, 32, 24, 24);
	m_pFileList->SetBounds(10, 60, 406, 130);
	m_pFileNameEdit->SetBounds(84, 194, 238, 24);
	m_pFileTypeCombo->SetBounds(84, 224, 238, 24);
	m_pOpenButton->SetBounds(336, 194, 74, 24);
	m_pCancelButton->SetBounds(336, 224, 74, 24);
	lookIn->SetBounds(10, 32, 55, 24);
	fileName->SetBounds(10, 194, 72, 24);
	fileType->SetBounds(10, 224, 72, 24);

	// set autolayout parameters
	m_pFullPathEdit->SetAutoResize(Panel::AUTORESIZE_RIGHT);
	m_pFileNameEdit->SetAutoResize(Panel::AUTORESIZE_RIGHT);
	m_pFileTypeCombo->SetAutoResize(Panel::AUTORESIZE_RIGHT);
	m_pFileList->SetAutoResize(Panel::AUTORESIZE_DOWNANDRIGHT);

	m_pFolderUpButton->SetPinCorner(Panel::PIN_TOPRIGHT);
	m_pFileNameEdit->SetPinCorner(Panel::PIN_BOTTOMLEFT);
	m_pFileTypeCombo->SetPinCorner(Panel::PIN_BOTTOMLEFT);
	m_pOpenButton->SetPinCorner(Panel::PIN_BOTTOMRIGHT);
	m_pCancelButton->SetPinCorner(Panel::PIN_BOTTOMRIGHT);
	fileName->SetPinCorner(Panel::PIN_BOTTOMLEFT);
	fileType->SetPinCorner(Panel::PIN_BOTTOMLEFT);

	// label settings
	lookIn->SetContentAlignment(Label::a_west);
	fileName->SetContentAlignment(Label::a_west);
	fileType->SetContentAlignment(Label::a_west);

	lookIn->SetAssociatedControl(m_pFullPathEdit);
	fileName->SetAssociatedControl(m_pFileNameEdit);
	fileType->SetAssociatedControl(m_pFileTypeCombo);

	// set tab positions
	GetFocusNavGroup().SetDefaultButton(m_pOpenButton);

	m_pFileNameEdit->SetTabPosition(1);
	m_pFileTypeCombo->SetTabPosition(2);
	m_pOpenButton->SetTabPosition(3);
	m_pCancelButton->SetTabPosition(4);
	m_pFullPathEdit->SetTabPosition(5);
	m_pFileList->SetTabPosition(6);

	m_pOpenButton->SetCommand("Open");
	m_pCancelButton->SetCommand("Close");
	m_pFolderUpButton->SetCommand("FolderUp");

	SetSize(426, 384);
	
	// Set our starting path to the current directory
	char localPath[255];
	filesystem()->GetCurrentDirectory( localPath , 255 );
	SetStartDirectory(localPath);

	PopulateDriveList();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
FileOpenDialog::~FileOpenDialog()
{
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void FileOpenDialog::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	m_pFolderIcon->SetImage(scheme()->GetImage("resource/icon_folder", false));
	m_pFolderUpButton->AddImage(scheme()->GetImage("resource/icon_folderup", false), -3);

	ImageList *imageList = new ImageList(false);
	imageList->AddImage(scheme()->GetImage("resource/icon_file", false));
	imageList->AddImage(scheme()->GetImage("resource/icon_folder", false));
	imageList->AddImage(scheme()->GetImage("resource/icon_folder_selected", false));

	m_pFileList->SetImageList(imageList, true);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void FileOpenDialog::PopulateDriveList()
{
	char fullpath[MAX_PATH * 4];
	char subDirPath[MAX_PATH * 4];
	GetCurrentDirectory(fullpath, sizeof(fullpath) - MAX_PATH);
	Q_strncpy(subDirPath, fullpath, sizeof( subDirPath ) );

	m_pFullPathEdit->DeleteAllItems();

	// populate the drive list
	char buf[512];
	int len = system()->GetAvailableDrives(buf, 512);
	char *pBuf = buf;
	for (int i=0; i < len / 4; i++)
	{
		m_pFullPathEdit->AddItem(pBuf, NULL);

		// is this our drive - add all subdirectories
		if (!_strnicmp(pBuf, fullpath, 2))
		{
			int indent = 0;
			char *pData = fullpath;
			while (*pData)
			{
				if (*pData == '\\')
				{
					if (indent > 0)
					{
						memset(subDirPath, ' ', indent);
						memcpy(subDirPath+indent, fullpath, pData-fullpath);
						subDirPath[indent+pData-fullpath] = 0;

						m_pFullPathEdit->AddItem(subDirPath, NULL);
					}
					indent += 2;
				}
				pData++;
			}
		}
		pBuf += 4;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Delete self on close
//-----------------------------------------------------------------------------
void FileOpenDialog::OnClose()
{
	m_pFileNameEdit->SetText("");
	m_pFileNameEdit->HideMenu();

	BaseClass::OnClose();
}

//-----------------------------------------------------------------------------
// Purpose: Handle for button commands
//-----------------------------------------------------------------------------
void FileOpenDialog::OnCommand(const char *command)
{
	if (!stricmp(command, "Open"))
	{
		OnOpen();
	}
	else if (!stricmp(command, "FolderUp"))
	{
		MoveUpFolder();
		OnOpen();
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set the starting directory of the file search.
//-----------------------------------------------------------------------------
void FileOpenDialog::SetStartDirectory(const char *dir)
{
	m_pFullPathEdit->SetText(dir);

	// ensure it's validity
	ValidatePath();
	PopulateDriveList();
}

//-----------------------------------------------------------------------------
// Purpose: Add filters for the drop down combo box
//-----------------------------------------------------------------------------
void FileOpenDialog::AddFilter(const char *filter, const char *filterName, bool bActive)
{
	KeyValues *kv = new KeyValues("item");
	kv->SetString("filter", filter);
	int itemID = m_pFileTypeCombo->AddItem(filterName, kv);
	if ( bActive )
	{
		m_pFileTypeCombo->ActivateItem(itemID);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Activate the dialog
//-----------------------------------------------------------------------------
void FileOpenDialog::DoModal(bool isSaveAsDialog)
{
	// move to the middle of the screen
	// get the screen size
	int swide, stall, wide, tall;
	surface()->GetScreenSize(swide, stall);

	// get our dialog size
	GetSize(wide, tall);

	// put the dialog in the middle of the screen
	SetPos((swide - wide) / 2, (stall - tall) / 2);

	SetVisible( true );
	SetEnabled( true );
	MoveToFront();

	m_pFileNameEdit->RequestFocus();

	PopulateFileList();
	InvalidateLayout();
	RequestFocus();
}

//-----------------------------------------------------------------------------
// Purpose: Gets the directory this is currently in
//-----------------------------------------------------------------------------
void FileOpenDialog::GetCurrentDirectory(char *buf, int bufSize)
{
	// get the text from the text entry
	m_pFullPathEdit->GetText(buf, bufSize);
}

//-----------------------------------------------------------------------------
// Purpose: Get the last selected file name
//-----------------------------------------------------------------------------
void FileOpenDialog::GetSelectedFileName(char *buf, int bufSize)
{
	m_pFileNameEdit->GetText(buf, bufSize);
}

//-----------------------------------------------------------------------------
// Purpose: Move the directory structure up
//-----------------------------------------------------------------------------
void FileOpenDialog::MoveUpFolder()
{
	char fullpath[MAX_PATH * 4];
	GetCurrentDirectory(fullpath, sizeof(fullpath) - MAX_PATH);

	// strip it back
	char *pos = strrchr(fullpath, '\\');
	if (pos)
	{
		*pos = 0;

		if (!pos[1])
		{
			pos = strrchr(fullpath, '\\');
			if (pos)
			{
				*pos = 0;
			}
		}
	}

	// append a trailing slash
	Q_strncat(fullpath, "\\", sizeof( fullpath ), COPY_ALL_CHARACTERS );

	SetStartDirectory(fullpath);
	PopulateFileList();
	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: Validate that the current path is valid
//-----------------------------------------------------------------------------
void FileOpenDialog::ValidatePath()
{
	char fullpath[MAX_PATH * 4];
	GetCurrentDirectory(fullpath, sizeof(fullpath) - MAX_PATH);

	// ensure to add '\' to end of path
	char *pos = strrchr(fullpath, '\\');
	if ( ! ( pos && !pos[1] && pos[0] == '\\' ) )
	{
		Q_strncat(fullpath, "\\", sizeof( fullpath ), COPY_ALL_CHARACTERS );
	}

	// see if the path can be opened
	Q_strncat(fullpath, "*", sizeof( fullpath ), COPY_ALL_CHARACTERS );
	char *pData = fullpath;
	while (*pData == ' ')
	{
		pData++;
	}

	WIN32_FIND_DATA findData;
	HANDLE findHandle = ::FindFirstFile(pData, &findData);
	if (findHandle != INVALID_HANDLE_VALUE)
	{
		// directory is valid, remove * and store
		char *pos = strrchr(pData, '*');
		if (pos)
		{
			*pos = 0;
		}
		Q_strncpy(m_szLastPath, pData, sizeof(m_szLastPath));
	}
	else
	{
		// failed to load file, use the previously successful path
		Q_strcpy(pData, m_szLastPath);
	}

	m_pFullPathEdit->SetText(pData);
	m_pFullPathEdit->GetTooltip()->SetText(pData);
}

//-----------------------------------------------------------------------------
// Purpose: Fill the filelist with the names of all the files in the current directory
//-----------------------------------------------------------------------------
#define MAX_FILTER_LENGTH 255
void FileOpenDialog::PopulateFileList()
{
	// clear the current list
	m_pFileList->DeleteAllItems();
	
	// get the current directory
	char currentDir[MAX_PATH * 4];
	char dir[MAX_PATH * 4];
	char filterList[MAX_FILTER_LENGTH+1];
	GetCurrentDirectory(currentDir, sizeof(dir));

	KeyValues *combokv = m_pFileTypeCombo->GetActiveItemUserData();
	if (combokv)
	{
		Q_strncpy(filterList, combokv->GetString("filter", "*"), MAX_FILTER_LENGTH);
	}
	else
	{
		// add wildcard for search
		Q_strncpy(filterList, "*\0", MAX_FILTER_LENGTH);
	}

	char *filterPtr = filterList;
	KeyValues *kv = new KeyValues("item");
	WIN32_FIND_DATA findData;
	HANDLE findHandle = NULL;

	while ((filterPtr != NULL) && (*filterPtr != 0))
	{
		// parse the next filter in the list.
		char curFilter[MAX_FILTER_LENGTH];
		curFilter[0] = 0;
		int i = 0;
		while ((filterPtr != NULL) && ((*filterPtr == ',') || (*filterPtr == ';') || (*filterPtr <= ' ')))
		{
			++filterPtr;
		}
		while ((filterPtr != NULL) && (*filterPtr != ',') && (*filterPtr != ';') && (*filterPtr > ' '))
		{
			curFilter[i++] = *(filterPtr++);
		}
		curFilter[i] = 0;

		if (curFilter[0] == 0)
		{
			break;
		}

		Q_snprintf(dir, MAX_PATH*4, "%s%s", currentDir, curFilter);

		// open the directory and walk it, loading files
		findHandle = ::FindFirstFile(dir, &findData);
		while (findHandle != INVALID_HANDLE_VALUE)
		{
			if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				// add the file to the list
				kv->SetString("text", findData.cFileName);
				kv->SetInt("image", 1);
				kv->SetInt("imageSelected", 0);
				kv->SetInt("directory", 0);
				m_pFileList->AddItem(kv, false, false);
			}

			if (!::FindNextFile(findHandle, &findData))
				break;
		}
		::FindClose(findHandle);
	}

	// find all the directories
	GetCurrentDirectory(dir, sizeof(dir));
	Q_strncat(dir, "*", sizeof( dir ), COPY_ALL_CHARACTERS);
	findHandle = ::FindFirstFile(dir, &findData);
	while (findHandle != INVALID_HANDLE_VALUE)
	{
		if (findData.cFileName[0] != '.' && findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			kv->SetString("text", findData.cFileName);
			kv->SetInt("image", 2);
			kv->SetInt("imageSelected", 3);
			kv->SetInt("directory", 1);
			m_pFileList->AddItem(kv, false, false);
		}

		if (!::FindNextFile(findHandle, &findData))
			break;
	}
	::FindClose(findHandle);

	kv->deleteThis();
	m_pFileList->SortList();
}

//-----------------------------------------------------------------------------
// Purpose: Handle the open button being pressed
//			checks on what has changed and acts accordingly
//-----------------------------------------------------------------------------
void FileOpenDialog::OnOpen()
{
	ValidatePath();

	// construct a file path
	char filename[MAX_PATH];
	GetSelectedFileName(filename, sizeof(filename));
	char fullpath[MAX_PATH * 4];
	GetCurrentDirectory(fullpath, sizeof(fullpath) - MAX_PATH);
	strcat(fullpath, filename);

	if (!stricmp(filename, ".."))
	{
		MoveUpFolder();
		
		// clear the name text
		m_pFileNameEdit->SetText("");
		return;
	}

	// get the details of the filename so we know what to do with it
	WIN32_FIND_DATA findData;
	HANDLE findHandle = ::FindFirstFile(fullpath, &findData);
	if (findHandle != INVALID_HANDLE_VALUE)
	{
		// file found
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// it's a directory; change to the specified directory
			strcat(fullpath, "\\");
			SetStartDirectory(fullpath);

			// clear the name text
			m_pFileNameEdit->SetText("");
			m_pFileNameEdit->HideMenu();
		}
		else
		{
			// open the file!
			PostActionSignal(new KeyValues("FileSelected", "fullpath", fullpath));
			PostMessage(this, new KeyValues("Close"));
			return;
		}
	}
	else
	{
		// file not found
		if (!m_bOpenOnly && filename[0])
		{
			// open the file!
			PostActionSignal(new KeyValues("FileSelected", "fullpath", fullpath));
			PostMessage(this, new KeyValues("Close"));
			return;
		}
	}

	PopulateDriveList();
	PopulateFileList();
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: using the file edit box as a prefix, create a menu of all possible files 
//-----------------------------------------------------------------------------
void FileOpenDialog::PopulateFileNameCompletion()
{
	char buf[80];
	m_pFileNameEdit->GetText(buf, 80);
	wchar_t wbuf[80];
	m_pFileNameEdit->GetText(wbuf, 80);
	int bufLen = wcslen(wbuf);

	// delete all items before we check if there's even a string
	m_pFileNameEdit->DeleteAllItems();

	// no string at all - don't show even bother showing it
	if (bufLen == 0)
	{
		m_pFileNameEdit->HideMenu();
		return;
	}

	// what files use current string as a prefix?
	int nCount = m_pFileList->GetItemCount();
	int i;
	for ( i = 0 ; i < nCount ; i++ )
	{
		KeyValues *kv = m_pFileList->GetItem(m_pFileList->GetItemIDFromPos(i));
		const wchar_t *wszString = kv->GetWString("text");
		if ( !_wcsnicmp(wbuf, wszString, bufLen) )
		{
			m_pFileNameEdit->AddItem(wszString, NULL);
		}
	}

	// if there are any items - show the menu
	if ( m_pFileNameEdit->GetItemCount() > 0 )
	{
		m_pFileNameEdit->ShowMenu();
	}
	else
	{
		m_pFileNameEdit->HideMenu();
	}

	m_pFileNameEdit->InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Handle an item in the list being selected
//-----------------------------------------------------------------------------
void FileOpenDialog::OnItemSelected()
{
	// make sure only one item is selected
	if (m_pFileList->GetSelectedItemsCount() != 1)
	{
		m_pFileNameEdit->SetText("");
	}
	else
	{
		// put the file name into the text edit box
		KeyValues *data = m_pFileList->GetItem(m_pFileList->GetSelectedItem(0));
		m_pFileNameEdit->SetText(data->GetString("text"));
	}

	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Handle an item in the Drive combo box being selected
//-----------------------------------------------------------------------------
void FileOpenDialog::OnTextChanged(KeyValues *kv)
{
	Panel *pPanel = (Panel *) kv->GetPtr("panel", NULL);

	// first check which control had its text changed!
	if (pPanel == m_pFullPathEdit)
	{
		m_pFileNameEdit->HideMenu();
		m_pFileNameEdit->SetText("");
		OnOpen();
	}
	else if (pPanel == m_pFileNameEdit)
	{
		PopulateFileNameCompletion();
	}
	else if (pPanel == m_pFileTypeCombo)
	{
		m_pFileNameEdit->HideMenu();
		PopulateFileList();
	}
}
#endif // ifndef _WIN32
