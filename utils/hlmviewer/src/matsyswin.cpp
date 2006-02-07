//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
/*

  glapp.c - Simple OpenGL shell
  
	There are several options allowed on the command line.  They are:
	-height : what window/screen height do you want to use?
	-width  : what window/screen width do you want to use?
	-bpp    : what color depth do you want to use?
	-window : create a rendering window rather than full-screen
	-fov    : use a field of view other than 90 degrees
*/


//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           MatSysWindow.cpp
// last modified:  May 04 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
// version:        1.2
//
// email:          mete@swissquake.ch
// web:            http://www.swissquake.ch/chumbalum-soft/
//
#include <mx/mx.h>
#include <mx/mxMessageBox.h>
#include <mx/mxTga.h>
#include <mx/mxPcx.h>
#include <mx/mxBmp.h>
#include <mx/mxMatSysWindow.h>
// #include "gl.h"
// #include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "MatSysWin.h"
#include "MDLViewer.h"
#include "StudioModel.h"
#include "ControlPanel.h"
#include "ViewerSettings.h"
#include "cmdlib.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialproxyfactory.h"
#include "FileSystem.h"
#include "materialsystem/IMesh.h"
#include "materialsystem/IMaterialSystemHardwareConfig.h"
#include "materialsystem/ITexture.h"
#include "materialsystem/MaterialSystem_Config.h"
#include "tier0/dbg.h"
#include "istudiorender.h"
#include "vstdlib/icommandline.h"
#include "vmatrix.h"
#include "studio_render.h"
#include "vstdlib/cvar.h"

extern char g_appTitle[];
extern bool g_bInError;
extern int g_dxlevel;

// FIXME: move all this to mxMatSysWin

class DummyMaterialProxyFactory : public IMaterialProxyFactory
{
public:
	virtual IMaterialProxy *CreateProxy( const char *proxyName )	{return NULL;}
	virtual void DeleteProxy( IMaterialProxy *pProxy )				{}
};
DummyMaterialProxyFactory	g_DummyMaterialProxyFactory;


static void ReleaseMaterialSystemObjects()
{
	StudioModel::ReleaseStudioModel();
}

static void RestoreMaterialSystemObjects( int nChangeFlags )
{
	StudioModel::RestoreStudioModel();
}

void InitMaterialSystemConfig(MaterialSystem_Config_t *pConfig)
{
	if ( g_viewerSettings.enableNormalMapping )
	{
		pConfig->m_Flags &= ~MATSYS_VIDCFG_FLAGS_DISABLE_BUMPMAP;
	}
	else
	{
		pConfig->m_Flags |= MATSYS_VIDCFG_FLAGS_DISABLE_BUMPMAP;
	}
	if ( g_viewerSettings.enableSpecular)
	{
		pConfig->m_Flags &= ~MATSYS_VIDCFG_FLAGS_DISABLE_SPECULAR;
	}
	else
	{
		pConfig->m_Flags |= MATSYS_VIDCFG_FLAGS_DISABLE_SPECULAR;
	}
	if ( g_viewerSettings.enableParallaxMapping )
	{
		pConfig->m_Flags |= MATSYS_VIDCFG_FLAGS_ENABLE_PARALLAX_MAPPING;
	}
	else
	{
		pConfig->m_Flags &= ~MATSYS_VIDCFG_FLAGS_ENABLE_PARALLAX_MAPPING;
	}
}

MatSysWindow *g_MatSysWindow = 0;
IMaterialSystem *g_pMaterialSystem;
IMaterialSystemHardwareConfig *g_pMaterialSystemHardwareConfig;

Vector g_vright( 50, 50, 0 );		// needs to be set to viewer's right in order for chrome to work

IMaterial *g_materialBackground = NULL;
IMaterial *g_materialWireframe = NULL;
IMaterial *g_materialFlatshaded = NULL;
IMaterial *g_materialSmoothshaded = NULL;
IMaterial *g_materialBones = NULL;
IMaterial *g_materialLines = NULL;
IMaterial *g_materialFloor = NULL;
IMaterial *g_materialVertexColor = NULL;
IMaterial *g_materialShadow = NULL;


// hack - this should go somewhere else.
CreateInterfaceFn g_MaterialSystemClientFactory = NULL;
CreateInterfaceFn g_MaterialSystemFactory = NULL;

MatSysWindow::MatSysWindow (mxWindow *parent, int x, int y, int w, int h, const char *label, int style)
: mxMatSysWindow (parent, x, y, w, h, label, style)
{
	m_pCubemapTexture = NULL;
	m_hWnd = (HWND)getHandle();

	// Load the material system DLL and get its interface.
	m_hMaterialSystemInst = g_pFullFileSystem->LoadModule( "MaterialSystem.dll" );
	if( !m_hMaterialSystemInst )
	{
		return; // Sys_Error( "Can't load MaterialSystem.dll\n" );
	}

	g_MaterialSystemFactory = Sys_GetFactory( m_hMaterialSystemInst );
	if ( g_MaterialSystemFactory )
	{
		g_pMaterialSystem = (IMaterialSystem *)g_MaterialSystemFactory( MATERIAL_SYSTEM_INTERFACE_VERSION, NULL );
		if ( !g_pMaterialSystem )
		{
			return; // Sys_Error( "Could not get the material system interface from materialsystem.dll" );
		}
	}
	else
	{
		return; // Sys_Error( "Could not find factory interface in library MaterialSystem.dll" );
	}

	const char *pShaderDLL = CommandLine()->ParmValue("-shaderdll");
	if(!pShaderDLL)
	{
		pShaderDLL = "shaderapidx9.dll";
	}

	if(!( g_MaterialSystemClientFactory = g_pMaterialSystem->Init(pShaderDLL, 
		&g_DummyMaterialProxyFactory, CmdLib_GetFileSystemFactory(), VStdLib_GetICVarFactory() )) )
		return; // Sys_Error("IMaterialSystem::Init failed");


	g_pMaterialSystemHardwareConfig = (IMaterialSystemHardwareConfig*)
		g_MaterialSystemClientFactory( MATERIALSYSTEM_HARDWARECONFIG_INTERFACE_VERSION, 0 );
	if ( !g_pMaterialSystemHardwareConfig )
		return;

	MaterialSystem_Config_t config;
	config = g_pMaterialSystem->GetCurrentConfigForVideoCard();
	InitMaterialSystemConfig(&config);
	if ( g_dxlevel != 0 )
	{
		config.dxSupportLevel = g_dxlevel;
	}
	
//	config.m_VideoMode.m_Width = config.m_VideoMode.m_Height = 0;
	config.SetFlag( MATSYS_VIDCFG_FLAGS_WINDOWED, true );
	config.SetFlag( MATSYS_VIDCFG_FLAGS_RESIZING, true );

	if (!g_pMaterialSystem->SetMode( ( void * )m_hWnd, config ) )
	{
		return;
	}
	
	g_pMaterialSystem->OverrideConfig( config, false );

	g_pMaterialSystem->AddReleaseFunc( ReleaseMaterialSystemObjects );
	g_pMaterialSystem->AddRestoreFunc( RestoreMaterialSystemObjects );

	m_pCubemapTexture = g_pMaterialSystem->FindTexture( "hlmv/cubemap", NULL, true );
	m_pCubemapTexture->IncrementReferenceCount();
	g_pMaterialSystem->BindLocalCubemap( m_pCubemapTexture );

	g_materialBackground	= g_pMaterialSystem->FindMaterial("hlmv/background", TEXTURE_GROUP_OTHER, true);
	g_materialWireframe		= g_pMaterialSystem->FindMaterial("debug/debugmrmwireframe", TEXTURE_GROUP_OTHER, true);
	g_materialFlatshaded	= g_pMaterialSystem->FindMaterial("debug/debugdrawflatpolygons", TEXTURE_GROUP_OTHER, true);
	g_materialSmoothshaded	= g_pMaterialSystem->FindMaterial("debug/debugmrmfullbright2", TEXTURE_GROUP_OTHER, true);
	g_materialBones			= g_pMaterialSystem->FindMaterial("debug/debugskeleton", TEXTURE_GROUP_OTHER, true);
	g_materialLines			= g_pMaterialSystem->FindMaterial("debug/debugwireframevertexcolor", TEXTURE_GROUP_OTHER, true);
	g_materialFloor			= g_pMaterialSystem->FindMaterial("hlmv/floor", TEXTURE_GROUP_OTHER, true);
	g_materialVertexColor   = g_pMaterialSystem->FindMaterial("debug/debugvertexcolor", TEXTURE_GROUP_OTHER, true);
	g_materialShadow		= g_pMaterialSystem->FindMaterial("hlmv/shadow", TEXTURE_GROUP_OTHER, true);
	if (!parent)
		setVisible (true);
	else
		mx::setIdleWindow (this);
}



MatSysWindow::~MatSysWindow ()
{
	if (m_pCubemapTexture)
		m_pCubemapTexture->DecrementReferenceCount();
	mx::setIdleWindow (0);
}



int
MatSysWindow::handleEvent (mxEvent *event)
{
	static float oldrx = 0, oldry = 0, oldtz = 50, oldtx = 0, oldty = 0;
	static float oldlrx = 0, oldlry = 0;
	static int oldx, oldy;

	switch (event->event)
	{

	case mxEvent::Idle:
	{
		static double prev;
		double curr = (double) mx::getTickCount () / 1000.0;
		double dt = (curr - prev);

		// clamp to 100fps
		if (dt >= 0.0 && dt < 0.01)
		{
			Sleep( 10 - dt * 1000.0 );
			return 1;
		}

		if ( prev != 0.0 )
		{
	//		dt = 0.001;

			g_pStudioModel->AdvanceFrame ( dt * g_viewerSettings.speedScale );
			g_ControlPanel->updateFrameSlider( );
			g_ControlPanel->updateGroundSpeed( );
		}
		prev = curr;

		if (!g_viewerSettings.pause)
			redraw ();

		g_ControlPanel->updateTransitionAmount();

		return 1;
	}
	break;

	case mxEvent::MouseUp:
	{
		g_viewerSettings.mousedown = false;
	}
	break;

	case mxEvent::MouseDown:
	{
		g_viewerSettings.mousedown = true;

		oldrx = g_pStudioModel->m_angles[0];
		oldry = g_pStudioModel->m_angles[1];
		oldtx = g_pStudioModel->m_origin[0];
		oldty = g_pStudioModel->m_origin[1];
		oldtz = g_pStudioModel->m_origin[2];
		oldx = event->x;
		oldy = event->y;
		oldlrx = g_viewerSettings.lightrot[1];
		oldlry = g_viewerSettings.lightrot[0];
		g_viewerSettings.pause = false;

		float r = 1.0/3.0 * min( w(), h() );

		float d = sqrt( ( float )( (event->x - w()/2) * (event->x - w()/2) + (event->y - h()/2) * (event->y - h()/2) ) );

		if (d < r)
			g_viewerSettings.rotating = false;
		else
			g_viewerSettings.rotating = true;

		return 1;
	}
	break;

	case mxEvent::MouseDrag:
	{
		if (event->buttons & mxEvent::MouseLeftButton)
		{
			if (event->modifiers & mxEvent::KeyShift)
			{
				g_pStudioModel->m_origin[1] = oldty - (float) (event->x - oldx);
				g_pStudioModel->m_origin[2] = oldtz + (float) (event->y - oldy);
			}
			else if (event->modifiers & mxEvent::KeyCtrl)
			{
				float ry = (float) (event->y - oldy);
				float rx = (float) (event->x - oldx);
				oldx = event->x;
				oldy = event->y;

				QAngle movement = QAngle( ry, rx, 0 );

				matrix3x4_t tmp1, tmp2, tmp3;
				AngleMatrix( g_viewerSettings.lightrot, tmp1 );
				AngleMatrix( movement, tmp2 );
				ConcatTransforms( tmp2, tmp1, tmp3 );
				MatrixAngles( tmp3, g_viewerSettings.lightrot );

				// g_viewerSettings.lightrot[0] = oldlrx + (float) (event->y - oldy);
				// g_viewerSettings.lightrot[1] = oldlry + (float) (event->x - oldx);
			}
			else
			{
				if (!g_viewerSettings.rotating)
				{
					float ry = (float) (event->y - oldy);
					float rx = (float) (event->x - oldx);
					oldx = event->x;
					oldy = event->y;

					QAngle movement;
					matrix3x4_t tmp1, tmp2, tmp3;

					movement = QAngle( 0, rx, 0 );
					AngleMatrix( g_pStudioModel->m_angles, tmp1 );
					AngleMatrix( movement, tmp2 );
					ConcatTransforms( tmp1, tmp2, tmp3 );
					MatrixAngles( tmp3, g_pStudioModel->m_angles );

					movement = QAngle( ry, 0, 0 );
					AngleMatrix( g_pStudioModel->m_angles, tmp1 );
					AngleMatrix( movement, tmp2 );
					ConcatTransforms( tmp2, tmp1, tmp3 );
					MatrixAngles( tmp3, g_pStudioModel->m_angles );
				}
				else
				{
					float ang1 = (180 / 3.1415) * atan2( oldx - w()/2.0, oldy - h()/2.0 );
					float ang2 = (180 / 3.1415) * atan2( event->x - w()/2.0, event->y - h()/2.0 );
					oldx = event->x;
					oldy = event->y;

					QAngle movement = QAngle( 0, 0, ang2 - ang1 );

					matrix3x4_t tmp1, tmp2, tmp3;
					AngleMatrix( g_pStudioModel->m_angles, tmp1 );
					AngleMatrix( movement, tmp2 );
					ConcatTransforms( tmp2, tmp1, tmp3 );
					MatrixAngles( tmp3, g_pStudioModel->m_angles );
				}
			}
		}
		else if (event->buttons & mxEvent::MouseRightButton)
		{
			g_pStudioModel->m_origin[0] = oldtx + (float) (event->y - oldy);
		}
		redraw ();

		return 1;
	}
	break;

	case mxEvent::KeyDown:
	{
		switch (event->key)
		{
		case 116: // F5
		{
			g_MDLViewer->Refresh();
			break;
		}
		case 32:
		{
			int iSeq = g_pStudioModel->GetSequence ();
			if (iSeq == g_pStudioModel->SetSequence (iSeq + 1))
			{
				g_pStudioModel->SetSequence (0);
			}
		}
		break;

		case 27:
			if (!getParent ()) // fullscreen mode ?
				mx::quit ();
			break;

		case 'g':
			g_viewerSettings.showGround = !g_viewerSettings.showGround;
			break;

		case 'h':
			g_viewerSettings.showHitBoxes = !g_viewerSettings.showHitBoxes;
			break;

		case 'o':
			g_viewerSettings.showBones = !g_viewerSettings.showBones;
			break;

		case 'b':
			g_viewerSettings.showBackground = !g_viewerSettings.showBackground;
			break;

		case 'm':
			g_viewerSettings.showMovement = !g_viewerSettings.showMovement;
			break;

		case '1':
		case '2':
		case '3':
		case '4':
			g_viewerSettings.renderMode = event->key - '1';
			break;

		case '-':
			g_viewerSettings.speedScale -= 0.1f;
			if (g_viewerSettings.speedScale < 0.0f)
				g_viewerSettings.speedScale = 0.0f;
			break;

		case '+':
			g_viewerSettings.speedScale += 0.1f;
			if (g_viewerSettings.speedScale > 5.0f)
				g_viewerSettings.speedScale = 5.0f;
			break;
		}
	}
	break;

	} // switch (event->event)

	return 1;
}



void
drawFloor ()
{
	/*
	glBegin (GL_TRIANGLE_STRIP);
	glTexCoord2f (0.0f, 0.0f);
	glVertex3f (-100.0f, 100.0f, 0.0f);

	glTexCoord2f (0.0f, 1.0f);
	glVertex3f (-100.0f, -100.0f, 0.0f);

	glTexCoord2f (1.0f, 0.0f);
	glVertex3f (100.0f, 100.0f, 0.0f);

	glTexCoord2f (1.0f, 1.0f);
	glVertex3f (100.0f, -100.0f, 0.0f);

	glEnd ();
	*/
}


void DrawBackground()
{
	if (!g_viewerSettings.showBackground)
		return;

	g_pMaterialSystem->Bind(g_materialBackground);
	g_pMaterialSystem->MatrixMode(MATERIAL_MODEL);
	g_pMaterialSystem->PushMatrix();
	g_pMaterialSystem->LoadIdentity();
	g_pMaterialSystem->MatrixMode(MATERIAL_VIEW);
	g_pMaterialSystem->PushMatrix();
	g_pMaterialSystem->LoadIdentity();
	{
		IMesh* pMesh = g_pMaterialSystem->GetDynamicMesh();
		CMeshBuilder meshBuilder;
		meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

		float dist=-15000.0f;
		float tMin=0, tMax=1;
		
		meshBuilder.Position3f(-dist, dist, dist);
		meshBuilder.TexCoord2f( 0, tMin,tMax );
		meshBuilder.Color4ub( 255, 255, 255, 255 );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( dist, dist, dist);
		meshBuilder.TexCoord2f( 0, tMax,tMax );
		meshBuilder.Color4ub( 255, 255, 255, 255 );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( dist,-dist, dist);
		meshBuilder.TexCoord2f( 0, tMax,tMin );
		meshBuilder.Color4ub( 255, 255, 255, 255 );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f(-dist,-dist, dist);
		meshBuilder.TexCoord2f( 0, tMin,tMin );
		meshBuilder.Color4ub( 255, 255, 255, 255 );
		meshBuilder.AdvanceVertex();

		meshBuilder.End();
		pMesh->Draw();
	}
}

void DrawHelpers()
{
	if (g_viewerSettings.mousedown)
	{
		g_pMaterialSystem->Bind( g_materialBones );

		g_pMaterialSystem->MatrixMode(MATERIAL_MODEL);
		g_pMaterialSystem->LoadIdentity();
		g_pMaterialSystem->MatrixMode(MATERIAL_VIEW);
		g_pMaterialSystem->LoadIdentity();

		IMesh* pMesh = g_pMaterialSystem->GetDynamicMesh();

		CMeshBuilder meshBuilder;
		meshBuilder.Begin( pMesh, MATERIAL_LINES, 1 );

		if (g_viewerSettings.rotating)
			meshBuilder.Color3ub( 255, 255, 0 );
		else
			meshBuilder.Color3ub(   0, 255, 0 );

		for (int i = 0; i < 360; i += 5)
		{
			float a = i * (3.151492653/180.0f);

			if (g_viewerSettings.rotating)
				meshBuilder.Color3ub( 255, 255, 0 );
			else
				meshBuilder.Color3ub(   0, 255, 0 );

			meshBuilder.Position3f( sin( a ), cos( a ), -3.0f );
			meshBuilder.AdvanceVertex();
		}
		meshBuilder.End();
		pMesh->Draw();
	}
}


void DrawGroundPlane()
{
	if (!g_viewerSettings.showGround)
		return;

	g_pMaterialSystem->Bind(g_materialFloor);
	g_pMaterialSystem->MatrixMode(MATERIAL_MODEL);
	g_pMaterialSystem->PushMatrix();;
	g_pMaterialSystem->LoadIdentity();
	g_pMaterialSystem->MatrixMode(MATERIAL_VIEW);
	g_pMaterialSystem->PushMatrix();;
	g_pMaterialSystem->LoadIdentity();

	g_pMaterialSystem->MatrixMode( MATERIAL_VIEW );
	g_pMaterialSystem->LoadIdentity( );

	g_pMaterialSystem->Rotate( -90,  1, 0, 0 );	    // put Z going up
	g_pMaterialSystem->Rotate( -90,  0, 0, 1 );

    g_pMaterialSystem->Translate( -g_pStudioModel->m_origin[0],  -g_pStudioModel->m_origin[1],  -g_pStudioModel->m_origin[2] );

	g_pMaterialSystem->Rotate( g_pStudioModel->m_angles[1],  0, 0, 1 );
    g_pMaterialSystem->Rotate( g_pStudioModel->m_angles[0],  0, 1, 0 );
    g_pMaterialSystem->Rotate( g_pStudioModel->m_angles[2],  1, 0, 0 );

	static Vector tMap( 0, 0, 0 );
	static Vector dxMap( 1, 0, 0 );
	static Vector dyMap( 0, 1, 0 );

	Vector deltaPos;
	QAngle deltaAngles;

	g_pStudioModel->GetMovement( g_pStudioModel->m_prevGroundCycles, deltaPos, deltaAngles );

	IMesh* pMesh = g_pMaterialSystem->GetDynamicMesh();
	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

	float scale = 10.0;
	float dist=-100.0f;

	float dpdd = scale / dist;

	tMap.x = tMap.x + dxMap.x * deltaPos.x * dpdd + dxMap.y * deltaPos.y * dpdd;	
	tMap.y = tMap.y + dyMap.x * deltaPos.x * dpdd + dyMap.y * deltaPos.y * dpdd;

	while (tMap.x < 0.0) tMap.x +=  1.0;
	while (tMap.x > 1.0) tMap.x += -1.0;
	while (tMap.y < 0.0) tMap.y +=  1.0;
	while (tMap.y > 1.0) tMap.y += -1.0;

	VectorYawRotate( dxMap, -deltaAngles.y, dxMap );
	VectorYawRotate( dyMap, -deltaAngles.y, dyMap );

	// ARRGHHH, this is right but I don't know what I've done
	meshBuilder.Position3f( -dist, dist, 0 );
	meshBuilder.TexCoord2f( 0, tMap.x + (-dxMap.x - dyMap.x) * scale, tMap.y + (dxMap.y + dyMap.y) * scale );
	meshBuilder.Color4ub( 128, 128, 128, 255 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f( dist, dist, 0 );
	meshBuilder.TexCoord2f( 0, tMap.x + (dxMap.x - dyMap.x) * scale, tMap.y + (-dxMap.y + dyMap.y) * scale );
	meshBuilder.Color4ub( 128, 128, 128, 255 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f( dist, -dist, 0 );
	meshBuilder.TexCoord2f( 0, tMap.x + (dxMap.x + dyMap.x) * scale, tMap.y + (-dxMap.y - dyMap.y) * scale );
	meshBuilder.Color4ub( 128, 128, 128, 255 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f( -dist, -dist, 0 );
	meshBuilder.TexCoord2f( 0, tMap.x - (dxMap.x - dyMap.x) * scale, tMap.y - (-dxMap.y + dyMap.y) * scale );
	meshBuilder.Color4ub( 128, 128, 128, 255 );
	meshBuilder.AdvanceVertex();

	// draw underside
	meshBuilder.Position3f( -dist, dist, 0 );
	meshBuilder.TexCoord2f( 0, tMap.x + (-dxMap.x - dyMap.x) * scale, tMap.y + (dxMap.y + dyMap.y) * scale );
	meshBuilder.Color4ub( 128, 128, 128, 128 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f( -dist, -dist, 0 );
	meshBuilder.TexCoord2f( 0, tMap.x - (dxMap.x - dyMap.x) * scale, tMap.y - (-dxMap.y + dyMap.y) * scale );
	meshBuilder.Color4ub( 128, 128, 128, 128 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f( dist, -dist, 0 );
	meshBuilder.TexCoord2f( 0, tMap.x + (dxMap.x + dyMap.x) * scale, tMap.y + (-dxMap.y - dyMap.y) * scale );
	meshBuilder.Color4ub( 128, 128, 128, 128 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f( dist, dist, 0 );
	meshBuilder.TexCoord2f( 0, tMap.x + (dxMap.x - dyMap.x) * scale, tMap.y + (-dxMap.y + dyMap.y) * scale );
	meshBuilder.Color4ub( 128, 128, 128, 128 );
	meshBuilder.AdvanceVertex();

	
	
	meshBuilder.End();
	pMesh->Draw();

	g_pMaterialSystem->MatrixMode(MATERIAL_MODEL);
	g_pMaterialSystem->PopMatrix();
	g_pMaterialSystem->MatrixMode(MATERIAL_VIEW);
	g_pMaterialSystem->PopMatrix();
}




void DrawMovementBoxes()
{
	if (!g_viewerSettings.showMovement)
		return;

	g_pMaterialSystem->Bind(g_materialFloor);
	g_pMaterialSystem->MatrixMode(MATERIAL_MODEL);
	g_pMaterialSystem->PushMatrix();
	g_pMaterialSystem->LoadIdentity();
	g_pMaterialSystem->MatrixMode(MATERIAL_VIEW);
	g_pMaterialSystem->PushMatrix();
	g_pMaterialSystem->LoadIdentity();

	g_pMaterialSystem->MatrixMode( MATERIAL_VIEW );
	g_pMaterialSystem->LoadIdentity( );

	g_pMaterialSystem->Rotate( -90,  1, 0, 0 );	    // put Z going up
	g_pMaterialSystem->Rotate( -90,  0, 0, 1 );

    g_pMaterialSystem->Translate( -g_pStudioModel->m_origin[0],  -g_pStudioModel->m_origin[1],  -g_pStudioModel->m_origin[2] );

	g_pMaterialSystem->Rotate( g_pStudioModel->m_angles[1],  0, 0, 1 );
    g_pMaterialSystem->Rotate( g_pStudioModel->m_angles[0],  0, 1, 0 );
    g_pMaterialSystem->Rotate( g_pStudioModel->m_angles[2],  1, 0, 0 );

	static matrix3x4_t mStart( 1, 0, 0, 0 ,  0, 1, 0, 0 ,  0, 0, 1, 0 );
	matrix3x4_t mTemp;
	static float prevframes[5];

	Vector deltaPos;
	QAngle deltaAngles;

	g_pStudioModel->GetMovement( prevframes, deltaPos, deltaAngles );

	AngleMatrix( deltaAngles, deltaPos, mTemp );
	MatrixInvert( mTemp, mTemp );
	ConcatTransforms( mTemp, mStart, mStart );

	Vector bboxMin, bboxMax;
	g_pStudioModel->ExtractBbox( bboxMin, bboxMax  );

	static float prevCycle = 0.0;

	if (fabs( g_pStudioModel->GetFrame( 0 ) - prevCycle) > 0.5)
	{
		SetIdentityMatrix( mStart );
	}
	prevCycle = g_pStudioModel->GetFrame( 0 );

	// starting position
	{
		float color[] = { 0.7, 1, 0, 0.5 };
		float wirecolor[] = { 1, 1, 0, 1.0 };
		g_pStudioModel->drawTransparentBox( bboxMin, bboxMax, mStart, color, wirecolor );
	}

	// current position
	{
		float color[] = { 1, 0.7, 0, 0.5 };
		float wirecolor[] = { 1, 0, 0, 1.0 };
		SetIdentityMatrix( mTemp );
		g_pStudioModel->drawTransparentBox( bboxMin, bboxMax, mTemp, color, wirecolor );
	}

	g_pMaterialSystem->MatrixMode(MATERIAL_MODEL);
	g_pMaterialSystem->PopMatrix();
	g_pMaterialSystem->MatrixMode(MATERIAL_VIEW);
	g_pMaterialSystem->PopMatrix();
}


void
MatSysWindow::draw ()
{
	if ( g_bInError || !g_pStudioModel->GetStudioRender() )
		return;

	g_pMaterialSystem->BeginFrame();
	g_pStudioModel->GetStudioRender()->BeginFrame();

	g_pMaterialSystem->ClearColor3ub(g_viewerSettings.bgColor[0] * 255, g_viewerSettings.bgColor[1] * 255, g_viewerSettings.bgColor[2] * 255);
	// g_pMaterialSystem->ClearColor3ub(0, 0, 0 );
	g_pMaterialSystem->ClearBuffers(true, true);

	g_pMaterialSystem->Viewport( 0, 0, w(), h() );

	g_pMaterialSystem->MatrixMode( MATERIAL_PROJECTION );
	g_pMaterialSystem->LoadIdentity( );
	g_pMaterialSystem->PerspectiveX(g_viewerSettings.fov, (float)w() / (float)h(), 1.0f, 20000.0f);
	
	DrawBackground();
	DrawGroundPlane();
	DrawMovementBoxes();
	DrawHelpers();

	g_pMaterialSystem->MatrixMode( MATERIAL_VIEW );
	g_pMaterialSystem->LoadIdentity( );
	// FIXME: why is this needed?  Doesn't SetView() override this?
	g_pMaterialSystem->Rotate( -90,  1, 0, 0 );	    // put Z going up
	g_pMaterialSystem->Rotate( -90,  0, 0, 1 );

	int polycount = g_pStudioModel->DrawModel ();

	g_pStudioModel->GetStudioRender()->EndFrame();

	g_ControlPanel->setModelInfo();

	int lod;
	float metric;
	metric = g_pStudioModel->GetLodMetric();
	lod = g_pStudioModel->GetLodUsed();
	g_ControlPanel->setLOD( lod, true, false );
	g_ControlPanel->setLODMetric( metric );

	g_ControlPanel->setPolycount( polycount );
	g_ControlPanel->setTransparent( g_pStudioModel->m_bIsTransparent );

	g_ControlPanel->updatePoseParameters( );
	
	// draw what ever else is loaded
	int i;
	for (i = 0; i < 4; i++)
	{
		if (g_pStudioExtraModel[i] != NULL)
		{
			g_pStudioModel->GetStudioRender()->BeginFrame();
			g_pStudioExtraModel[i]->DrawModel( true );
			g_pStudioModel->GetStudioRender()->EndFrame();
		}
	}

	g_pStudioModel->IncrementFramecounter();

    g_pMaterialSystem->SwapBuffers();
	
	g_pMaterialSystem->EndFrame();
}



/*
int
MatSysWindow::loadTexture (const char *filename, int name)
{
	if (!filename || !strlen (filename))
	{
		if (d_textureNames[name])
		{
			glDeleteTextures (1, (const GLuint *) &d_textureNames[name]);
			d_textureNames[name] = 0;

			if (name == 0)
				strcpy (g_viewerSettings.backgroundTexFile, "");
			else
				strcpy (g_viewerSettings.groundTexFile, "");
		}

		return 0;
	}

	mxImage *image = 0;

	char ext[16];
	strcpy (ext, mx_getextension (filename));

	if (!mx_strcasecmp (ext, ".tga"))
		image = mxTgaRead (filename);
	else if (!mx_strcasecmp (ext, ".pcx"))
		image = mxPcxRead (filename);
	else if (!mx_strcasecmp (ext, ".bmp"))
		image = mxBmpRead (filename);

	if (image)
	{
		if (name == 0)
			strcpy (g_viewerSettings.backgroundTexFile, filename);
		else
			strcpy (g_viewerSettings.groundTexFile, filename);

		d_textureNames[name] = name + 1;

		if (image->bpp == 8)
		{
			mstudiotexture_t texture;
			texture.width = image->width;
			texture.height = image->height;

			g_pStudioModel->UploadTexture (&texture, (byte *) image->data, (byte *) image->palette, name + 1);
		}
		else
		{
			glBindTexture (GL_TEXTURE_2D, d_textureNames[name]);
			glTexImage2D (GL_TEXTURE_2D, 0, 3, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		delete image;

		return name + 1;
	}

	return 0;
}
*/


void
MatSysWindow::dumpViewport (const char *filename)
{
	redraw ();
	int w = w2 ();
	int h = h2 ();

	mxImage *image = new mxImage ();
	if (image->create (w, h, 24))
	{
#if 0
		glReadBuffer (GL_FRONT);
		glReadPixels (0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, image->data);
#else
		HDC hdc = GetDC ((HWND) getHandle ());
		byte *data = (byte *) image->data;
		int i = 0;
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				COLORREF cref = GetPixel (hdc, x, y);
				data[i++] = (byte) ((cref >> 0)& 0xff);
				data[i++] = (byte) ((cref >> 8) & 0xff);
				data[i++] = (byte) ((cref >> 16) & 0xff);
			}
		}
		ReleaseDC ((HWND) getHandle (), hdc);
#endif
		if (!mxTgaWrite (filename, image))
			mxMessageBox (this, "Error writing screenshot.", g_appTitle, MX_MB_OK | MX_MB_ERROR);

		delete image;
	}
}

