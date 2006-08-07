//====== Copyright � 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================
#ifdef _WIN32
#include "vgui/IVgui.h"
#include "vgui_controls/Controls.h"

#include "vgui_controls/AnimatingImagePanel.h"
#include "vgui_controls/BitmapImagePanel.h"
#include "vgui_controls/ExpandButton.h"
#include "vgui_controls/TreeViewListControl.h"
#include "vgui_controls/HTML.h"

using namespace vgui;

USING_BUILD_FACTORY( Button );
USING_BUILD_FACTORY( EditablePanel );
USING_BUILD_FACTORY( ImagePanel );
USING_BUILD_FACTORY( Label );
USING_BUILD_FACTORY( Panel );
USING_BUILD_FACTORY( ToggleButton );

#if !defined( _XBOX )
USING_BUILD_FACTORY( AnimatingImagePanel );
USING_BUILD_FACTORY( CBitmapImagePanel );
USING_BUILD_FACTORY( CheckButton );
USING_BUILD_FACTORY( ComboBox );
USING_BUILD_FACTORY( Divider );
USING_BUILD_FACTORY( ExpandButton );
USING_BUILD_FACTORY( GraphPanel );
//USING_BUILD_FACTORY_ALIAS( HTML, HTML_NoJavascript );
//USING_BUILD_FACTORY_ALIAS( HTML, HTML_Javascript );
USING_BUILD_FACTORY( ListPanel );
USING_BUILD_FACTORY( ListViewPanel );
USING_BUILD_FACTORY( Menu );
USING_BUILD_FACTORY( MenuBar );
USING_BUILD_FACTORY( MenuButton );
USING_BUILD_FACTORY( MenuItem );
USING_BUILD_FACTORY( MessageBox );
USING_BUILD_FACTORY( ProgressBar );
USING_BUILD_FACTORY( RadioButton );
USING_BUILD_FACTORY( RichText );
USING_BUILD_FACTORY_ALIAS( ScrollBar, ScrollBar_Vertical );
USING_BUILD_FACTORY_ALIAS( ScrollBar, ScrollBar_Horizontal );
USING_BUILD_FACTORY( ScrollBar );
USING_BUILD_FACTORY( TextEntry );
USING_BUILD_FACTORY( TreeView );
USING_BUILD_FACTORY( CTreeViewListControl );
USING_BUILD_FACTORY( URLLabel );
#endif

#endif // _WIN32

