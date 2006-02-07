//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Declaration of FileOpenDialog class, a generic open/save as file dialog
//
// $NoKeywords: $
//=============================================================================//

#ifndef FILEOPENDIALOG_H
#define FILEOPENDIALOG_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>

namespace vgui
{

class FileCompletionEdit;		// local

//-----------------------------------------------------------------------------
// Purpose: generic open/save as file dialog
//-----------------------------------------------------------------------------
class FileOpenDialog : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( FileOpenDialog, Frame );

public:
	FileOpenDialog(Panel *parent, const char *title, bool bOpenOnly);
	~FileOpenDialog();

	// Set the directory the file search starts in
	void SetStartDirectory(const char *dir);

	// Add filters for the drop down combo box
	void AddFilter(const char *filter, const char *filterName, bool bActive);

	// Activate the dialog
	void DoModal(bool isSaveAsDialog);

	// Get the directory this is currently in
	void GetCurrentDirectory(char *buf, int bufSize);

	// Get the last selected file name
	void GetSelectedFileName(char *buf, int bufSize);


	/*
		messages sent:
			"FileSelected"
				"fullpath"

	*/

protected:
	virtual void ApplySchemeSettings(IScheme *pScheme);
	virtual void OnClose();
	virtual void OnCommand(const char *command);

	// handles the open button being pressed
	// checks on what has changed and acts accordingly
	virtual void OnOpen();

	// moves the directory structure up
	virtual void MoveUpFolder();

	// validates that the current path is valid
	virtual void ValidatePath();

	// handles an item in the list being selected
	MESSAGE_FUNC( OnItemSelected, "ItemSelected" );
	MESSAGE_FUNC( OnListViewItemSelected, "ListViewItemSelected" )
	{
		OnItemSelected();
	}

	// fills the filelist with the names of all the files in the current directory
	virtual void PopulateFileList();

	virtual void PopulateDriveList();

	virtual void PopulateFileNameCompletion();

	// changes directories in response to selecting drives from the combo box
	MESSAGE_FUNC_PARAMS( OnTextChanged, "TextChanged", kv );

private:
	vgui::ComboBox 		*m_pFullPathEdit;
	vgui::ListViewPanel *m_pFileList;
	
	FileCompletionEdit 	*m_pFileNameEdit;

	vgui::ComboBox 		*m_pFileTypeCombo;
	vgui::Button 		*m_pOpenButton;
	vgui::Button 		*m_pCancelButton;
	vgui::Button 		*m_pFolderUpButton;
	vgui::ImagePanel 	*m_pFolderIcon;

	char m_szLastPath[1024];
	bool m_bOpenOnly;
};

} // namespace vgui

#endif // FILEOPENDIALOG_H
