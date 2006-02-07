//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#define PROTECTED_THINGS_DISABLE

#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/DirectorySelectDialog.h>
#include <vgui_controls/TreeView.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/MessageBox.h>
#include <vgui/Cursor.h>
#include <KeyValues.h>
#include <vgui/IInput.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>

#include <direct.h>
#include <stdio.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Used to handle dynamically populating the tree view
//-----------------------------------------------------------------------------
class DirectoryTreeView : public TreeView
{
public:
	DirectoryTreeView(DirectorySelectDialog *parent, const char *name) : TreeView(parent, name)
	{
		m_pParent = parent;
	}

	virtual void GenerateChildrenOfNode(int itemIndex)
	{
		m_pParent->GenerateChildrenOfDirectoryNode(itemIndex);
	}

private:
	DirectorySelectDialog *m_pParent;
};


//-----------------------------------------------------------------------------
// Purpose: Used to prompt the user to create a directory
//-----------------------------------------------------------------------------
class CreateDirectoryDialog : public Frame
{
	DECLARE_CLASS_SIMPLE(CreateDirectoryDialog, Frame);

public:
	CreateDirectoryDialog(Panel *parent, const char *defaultCreateDirName) : BaseClass(parent, NULL)
	{
		SetSize(320, 100);
		SetSizeable(false);
		SetTitle("Choose directory name", false);
		MoveToCenterOfScreen();

		m_pOKButton = new Button(this, "OKButton", "#vgui_ok");
		m_pCancelButton = new Button(this, "OKButton", "#vgui_cancel");
		m_pNameEntry = new TextEntry(this, "NameEntry");

		m_pOKButton->SetCommand("OK");
		m_pCancelButton->SetCommand("Close");
		m_pNameEntry->SetText(defaultCreateDirName);
		m_pNameEntry->RequestFocus();
		m_pNameEntry->SelectAllText(true);
	
		// If some other window was hogging the input focus, then we have to hog it or else we'll never get input.
		m_PrevAppFocusPanel = vgui::input()->GetAppModalSurface();
		if ( m_PrevAppFocusPanel )
			vgui::input()->SetAppModalSurface( GetVPanel() );
	}

	~CreateDirectoryDialog()
	{
		if ( m_PrevAppFocusPanel )
			vgui::input()->SetAppModalSurface( m_PrevAppFocusPanel );
	}

	virtual void PerformLayout()
	{
		BaseClass::PerformLayout();

		m_pNameEntry->SetBounds(24, 32, GetWide() - 48, 24);
		m_pOKButton->SetBounds(GetWide() - 176, 64, 72, 24);
		m_pCancelButton->SetBounds(GetWide() - 94, 64, 72, 24);
	}

	virtual void OnCommand(const char *command)
	{
		if (!stricmp(command, "OK"))
		{
			PostActionSignal(new KeyValues("CreateDirectory", "dir", GetControlString("NameEntry")));
			Close();
		}
		else
		{
			BaseClass::OnCommand(command);
		}
	}

	virtual void OnClose()
	{
		BaseClass::OnClose();
		MarkForDeletion();
	}

private:
	vgui::Button *m_pOKButton;
	vgui::Button *m_pCancelButton;
	vgui::TextEntry *m_pNameEntry;
	vgui::VPANEL m_PrevAppFocusPanel;
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
DirectorySelectDialog::DirectorySelectDialog(vgui::Panel *parent, const char *title) : Frame(parent, NULL)
{
	SetTitle(title, true);
	SetSize(320, 360);
	SetMinimumSize(300, 240);
	m_szCurrentDir[0] = 0;
	m_szDefaultCreateDirName[0] = 0;

	m_pDirTree = new DirectoryTreeView(this, "DirTree");
	m_pDriveCombo = new ComboBox(this, "DriveCombo", 6, false);
	m_pCancelButton = new Button(this, "CancelButton", "#VGui_Cancel");
	m_pSelectButton = new Button(this, "SelectButton", "#VGui_Select");
	m_pCreateButton = new Button(this, "CreateButton", "#VGui_CreateFolder");
	m_pCancelButton->SetCommand("Cancel");
	m_pSelectButton->SetCommand("Select");
	m_pCreateButton->SetCommand("Create");
}

//-----------------------------------------------------------------------------
// Purpose: lays out controls
//-----------------------------------------------------------------------------
void DirectorySelectDialog::PerformLayout()
{
	BaseClass::PerformLayout();

	// lay out all the controls
	m_pDriveCombo->SetBounds(24, 30, GetWide() - 48, 24);
	m_pDirTree->SetBounds(24, 64, GetWide() - 48, GetTall() - 128);

	m_pCreateButton->SetBounds(24, GetTall() - 48, 104, 24);
	m_pSelectButton->SetBounds(GetWide() - 172, GetTall() - 48, 72, 24);
	m_pCancelButton->SetBounds(GetWide() - 96, GetTall() - 48, 72, 24);
}

//-----------------------------------------------------------------------------
// Purpose: lays out controls
//-----------------------------------------------------------------------------
void DirectorySelectDialog::ApplySchemeSettings(IScheme *pScheme)
{
	ImageList *imageList = new ImageList(false);
	imageList->AddImage(scheme()->GetImage("Resource/icon_folder", false));
	imageList->AddImage(scheme()->GetImage("Resource/icon_folder_selected", false));
	m_pDirTree->SetImageList(imageList, true);

	BaseClass::ApplySchemeSettings(pScheme);
}

//-----------------------------------------------------------------------------
// Purpose: sets where it should start searching
//-----------------------------------------------------------------------------
void DirectorySelectDialog::SetStartDirectory(const char *path)
{
	strncpy(m_szCurrentDir, path, sizeof(m_szCurrentDir));
	strncpy(m_szCurrentDrive, path, sizeof(m_szCurrentDrive));
	m_szCurrentDrive[sizeof(m_szCurrentDrive) - 1] = 0;
	char *firstSlash = strstr(m_szCurrentDrive, "\\");
	if (firstSlash)
	{
		firstSlash[1] = 0;
	}

	BuildDirTree();
	BuildDriveChoices();

	// update state of create directory button
	int selectedIndex = m_pDirTree->GetSelectedItem();
	if (m_pDirTree->IsItemIDValid(selectedIndex))
	{
		m_pCreateButton->SetEnabled(true);
	}
	else
	{
		m_pCreateButton->SetEnabled(false);
	}
}

//-----------------------------------------------------------------------------
// Purpose: sets what name should show up by default in the create directory dialog
//-----------------------------------------------------------------------------
void DirectorySelectDialog::SetDefaultCreateDirectoryName(const char *defaultCreateDirName)
{
	strncpy(m_szDefaultCreateDirName, defaultCreateDirName, sizeof(m_szDefaultCreateDirName));
	m_szDefaultCreateDirName[sizeof(m_szDefaultCreateDirName) - 1] = 0;
}

//-----------------------------------------------------------------------------
// Purpose: opens the dialog
//-----------------------------------------------------------------------------
void DirectorySelectDialog::DoModal()
{
	//!! need fix, combobox dropdown doesn't work if this is modal
	// input()->SetAppModalSurface(GetVPanel());
	BaseClass::Activate();
	MoveToCenterOfScreen();
}

//-----------------------------------------------------------------------------
// Purpose: Builds drive choices
//-----------------------------------------------------------------------------
void DirectorySelectDialog::BuildDriveChoices()
{
	m_pDriveCombo->DeleteAllItems();

	char drives[256] = { 0 };
	int len = system()->GetAvailableDrives(drives, sizeof(drives));
	char *pBuf = drives;
	KeyValues *kv = new KeyValues("drive");
	for (int i = 0; i < len / 4; i++)
	{
		kv->SetString("drive", pBuf);
		int itemID = m_pDriveCombo->AddItem(pBuf, kv);
		if (!stricmp(pBuf, m_szCurrentDrive))
		{
			m_pDriveCombo->ActivateItem(itemID);
		}

		pBuf += 4;
	}
	kv->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: Builds the base tree directory
//-----------------------------------------------------------------------------
void DirectorySelectDialog::BuildDirTree()
{
	// clear current tree
	m_pDirTree->RemoveAll();

	// add in a root
	int rootIndex = m_pDirTree->AddItem(new KeyValues("root", "Text", m_szCurrentDrive), -1);

	// build first level of the tree
	ExpandTreeNode(m_szCurrentDrive, rootIndex);
	
	// start the root expanded
	m_pDirTree->ExpandItem(rootIndex, true);
}

//-----------------------------------------------------------------------------
// Purpose: expands a path
//-----------------------------------------------------------------------------
void DirectorySelectDialog::ExpandTreeNode(const char *path, int parentNodeIndex)
{
	// set the small wait cursor
	surface()->SetCursor(dc_waitarrow);

	// get all the subfolders of the current drive
	char searchString[512];
	sprintf(searchString, "%s*.*", path);

	_finddata_t wfd;
	memset(&wfd, 0, sizeof(_finddata_t));
	long hResult = _findfirst(searchString, &wfd);
	if (hResult != -1)
	{
		do
		{
			if ((wfd.attrib & _A_SUBDIR) && wfd.name[0] != '.')
			{
				KeyValues *kv = new KeyValues("item");
				kv->SetString("Text", wfd.name);
				// set the folder image
				kv->SetInt("Image", 1);
				kv->SetInt("SelectedImage", 1);
				kv->SetInt("Expand", DoesDirectoryHaveSubdirectories(path, wfd.name));	
				m_pDirTree->AddItem(kv, parentNodeIndex);
			}
		} while (_findnext(hResult, &wfd) == 0);
		_findclose(hResult);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool DirectorySelectDialog::DoesDirectoryHaveSubdirectories(const char *path, const char *dir)
{
	char searchString[512];
	sprintf(searchString, "%s%s\\*.*", path, dir);

	_finddata_t wfd;
	memset(&wfd, 0, sizeof(_finddata_t));
	long hResult = _findfirst(searchString, &wfd);
	if (hResult != -1)
	{
		do
		{
			if ((wfd.attrib & _A_SUBDIR) && wfd.name[0] != '.')
			{
				_findclose(hResult);
				return true;
			}
		} while (_findnext(hResult, &wfd) == 0);
		_findclose(hResult);
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Generates the children for the specified node
//-----------------------------------------------------------------------------
void DirectorySelectDialog::GenerateChildrenOfDirectoryNode(int nodeIndex)
{
	// generate path
	char path[512];
	GenerateFullPathForNode(nodeIndex, path, sizeof(path));

	// expand out
	ExpandTreeNode(path, nodeIndex);
}

//-----------------------------------------------------------------------------
// Purpose: creates the full path for a node
//-----------------------------------------------------------------------------
void DirectorySelectDialog::GenerateFullPathForNode(int nodeIndex, char *path, int pathBufferSize)
{
	// get all the nodes
	CUtlLinkedList<int, int> nodes;
	nodes.AddToTail(nodeIndex);
	int parentIndex = nodeIndex;
	while (1)
	{
		parentIndex = m_pDirTree->GetItemParent(parentIndex);
		if (parentIndex == -1)
			break;
		nodes.AddToHead(parentIndex);
	}

	// walk the nodes, adding to the path
	path[0] = 0;
	bool bFirst = true;
	FOR_EACH_LL( nodes, i )
	{
		KeyValues *kv = m_pDirTree->GetItemData( nodes[i] );
		strcat(path, kv->GetString("Text"));

		if (!bFirst)
		{
			strcat(path, "\\");
		}
		bFirst = false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handles combo box changes
//-----------------------------------------------------------------------------
void DirectorySelectDialog::OnTextChanged()
{
	KeyValues *kv = m_pDriveCombo->GetActiveItemUserData();
	if (!kv)
		return;
	const char *newDrive = kv->GetString("drive");
	if (stricmp(newDrive, m_szCurrentDrive))
	{
		// drive changed, reset
		SetStartDirectory(newDrive);
	}
}

//-----------------------------------------------------------------------------
// Purpose: creates a directory
//-----------------------------------------------------------------------------
void DirectorySelectDialog::OnCreateDirectory(const char *dir)
{
	int selectedIndex = m_pDirTree->GetSelectedItem();
	if (m_pDirTree->IsItemIDValid(selectedIndex))
	{
		char fullPath[512];
		GenerateFullPathForNode(selectedIndex, fullPath, sizeof(fullPath));

		// create the new directory underneath
		strcat(fullPath, dir);
		if (_mkdir(fullPath) == 0)
		{
			// add new path to tree view
			KeyValues *kv = new KeyValues("item");
			kv->SetString("Text", dir);
			// set the folder image
			kv->SetInt("Image", 1);
			kv->SetInt("SelectedImage", 1);
			int itemID = m_pDirTree->AddItem(kv, selectedIndex);

			// select the item
			m_pDirTree->SetSelectedItem(itemID);
		}
		else
		{
			// print error message
			MessageBox *box = new MessageBox("#vgui_CreateDirectoryFail_Title", "#vgui_CreateDirectoryFail_Info");
			box->DoModal(this);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: dialog closes
//-----------------------------------------------------------------------------
void DirectorySelectDialog::OnClose()
{
	BaseClass::OnClose();
	MarkForDeletion();
}

//-----------------------------------------------------------------------------
// Purpose: handles button commands
//-----------------------------------------------------------------------------
void DirectorySelectDialog::OnCommand(const char *command)
{
	if (!stricmp(command, "Cancel"))
	{
		Close();
	}
	else if (!stricmp(command, "Select"))
	{
		// path selected
		int selectedIndex = m_pDirTree->GetSelectedItem();
		if (m_pDirTree->IsItemIDValid(selectedIndex))
		{
			char fullPath[512];
			GenerateFullPathForNode(selectedIndex, fullPath, sizeof(fullPath));
			PostActionSignal(new KeyValues("DirectorySelected", "dir", fullPath));
			Close();
		}
	}
	else if (!stricmp(command, "Create"))
	{
		int selectedIndex = m_pDirTree->GetSelectedItem();
		if (m_pDirTree->IsItemIDValid(selectedIndex))
		{
			CreateDirectoryDialog *dlg = new CreateDirectoryDialog(this, m_szDefaultCreateDirName);
			dlg->AddActionSignalTarget(this);
			dlg->Activate();
		}
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Update the text in the combo
//-----------------------------------------------------------------------------
void DirectorySelectDialog::OnTreeViewItemSelected()
{
	int selectedIndex = m_pDirTree->GetSelectedItem();
	if (!m_pDirTree->IsItemIDValid(selectedIndex))
	{
		m_pCreateButton->SetEnabled(false);
		return;
	}
	m_pCreateButton->SetEnabled(true);

	// build the string
	char fullPath[512];
	GenerateFullPathForNode(selectedIndex, fullPath, sizeof(fullPath));

	int itemID = m_pDriveCombo->GetActiveItem();
	m_pDriveCombo->UpdateItem(itemID, fullPath, NULL);
	m_pDriveCombo->SetText(fullPath);
}