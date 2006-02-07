//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "glos.h"
#include <gl/gl.h>
#include <gl/glaux.h>
#include <gl/glu.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "cmdlib.h"
#include "mathlib.h"
#include "vstdlib/strtools.h"
#include "physdll.h"
#include "phyfile.h"
#include "vphysics_interface.h"
#include "vstdlib/icommandline.h"

HDC		camdc;
HGLRC	baseRC;
HWND	camerawindow;
HANDLE	main_instance;

/*	YWB:  3/13/98
	You run the program like normal with any file.  If you want to read portals for the
	file type, you type:  glview -portal filename.gl0 (or whatever).  glview will then
	try to read in the .prt file filename.prt.

	The portals are shown as white lines superimposed over your image.  You can toggle the 
	view between showing portals or not by hitting the '2' key.  The '1' key toggles 
	world polygons.

	The 'b' key toggles blending modes.

	If you don't want to depth buffer the portals, hit 'p'.

    The command line parsing is inelegant but functional.

    I sped up the KB movement and turn speed, too.
 */

// Vars added by YWB
Vector g_Center;               // Center of all read points, so camera is in a sensible place
int g_nTotalPoints	   = 0;    // Total points read, for calculating center
int g_UseBlending      = 0;	   // Toggle to use blending mode or not
BOOL g_bReadPortals    = 0;	   // Did we read in a portal file?
BOOL g_bNoDepthPortals = 0;    // Do we zbuffer the lines of the portals?
int g_nPortalHighlight = -1;	// The leaf we're viewing
BOOL g_bShowList1      = 1;	   // Show regular polygons?
BOOL g_bShowList2      = 1;	   // Show portals?
BOOL g_bShowLines      = 0;    // Show outlines of faces
BOOL g_Active = TRUE;
BOOL g_Update = TRUE;
BOOL g_bDisp = FALSE;
IPhysicsCollision *physcollision = NULL;
// -----------
static int g_Keys[255];
void AppKeyDown( int key );
void AppKeyUp( int key );


BOOL ReadDisplacementFile( const char *filename );
void DrawDisplacementData( void );


/*
=================
Error

For abnormal program terminations
=================
*/
void Error (char *error, ...)
{
	va_list argptr;
	char	text[1024];

	va_start (argptr,error);
	vsprintf (text, error,argptr);
	va_end (argptr);

    MessageBox(NULL, text, "Error", 0 /* MB_OK */ );

	exit (1);
}

float	origin[3] = {32, 32, 48};
float	angles[3];
float	forward[3], right[3], vup[3], vpn[3], vright[3];
float	width = 640;
float	height = 480;

#define	SPEED_MOVE	320		// Units / second (run speed of HL)
#define	SPEED_TURN	90		// Degrees / second

#define	VK_COMMA		188
#define	VK_PERIOD		190


void KeyDown (int key)
{
	switch (key)
	{
	case VK_ESCAPE:
		g_Active = FALSE;
		break;

	case VK_F1:
		glEnable (GL_CULL_FACE);
		glCullFace (GL_FRONT);
		break;
	case 'B':
		g_UseBlending ^= 1;
		if (g_UseBlending)
			glEnable(GL_BLEND);// YWB TESTING
		else
			glDisable(GL_BLEND);
		break;

	case '1':
		g_bShowList1 ^= 1;
		break;
	case '2':
		g_bShowList2 ^= 1;
		break;
	case 'P':
		g_bNoDepthPortals ^= 1;
		break;
	case 'L':
		g_bShowLines ^= 1;
		break;
	}
	g_Update = TRUE;
}

static BOOL g_Capture = FALSE;

#define	MOUSE_SENSITIVITY			0.2f
#define MOUSE_SENSITIVITY_X			(MOUSE_SENSITIVITY*1)
#define MOUSE_SENSITIVITY_Y			(MOUSE_SENSITIVITY*1)

void Cam_MouseMoved( void )
{
	if ( g_Capture )
	{
		RECT rect;
		int centerx, centery;
		float deltax, deltay;
		POINT cursorPoint;

		GetWindowRect( camerawindow, &rect );
		
		if ( rect.top < 0)
			rect.top = 0;
		if ( rect.left < 0)
			rect.left = 0;

		centerx = ( rect.left + rect.right ) / 2;
		centery = ( rect.top + rect.bottom ) / 2;

		GetCursorPos( &cursorPoint );
		SetCursorPos( centerx, centery );

		deltax = (cursorPoint.x - centerx) * MOUSE_SENSITIVITY_X;
		deltay = (cursorPoint.y - centery) * MOUSE_SENSITIVITY_Y;

		angles[1] -= deltax;
		angles[0] -= deltay;

		g_Update = TRUE;
	}
}

int Test_Key( int key )
{
	int r = (g_Keys[ key ] != 0);

	g_Keys[ key ] &= 0x01; // clear out debounce bit

	if (r)
		g_Update = TRUE;

	return r;
}

// UNDONE: Probably should change the controls to match the game - but I don't know who relies on them
// as of now.
void Cam_Update( float frametime )
{
	if ( Test_Key( 'W' ) )
	{
		VectorMA (origin, SPEED_MOVE*frametime, vpn, origin);
	}
	if ( Test_Key( 'S' ) )
	{
		VectorMA (origin, -SPEED_MOVE*frametime, vpn, origin);
	}
	if ( Test_Key( 'A' ) )
	{
		VectorMA (origin, -SPEED_MOVE*frametime, vright, origin);
	}
	if ( Test_Key( 'D' ) )
	{
		VectorMA (origin, SPEED_MOVE*frametime, vright, origin);
	}

	if ( Test_Key( VK_UP ) )
	{
		VectorMA (origin, SPEED_MOVE*frametime, forward, origin);
	}
	if ( Test_Key( VK_DOWN ) )
	{
		VectorMA (origin, -SPEED_MOVE*frametime, forward, origin);
	}

	if ( Test_Key( VK_LEFT ) )
	{
		angles[1] += SPEED_TURN * frametime;
	}
	if ( Test_Key( VK_RIGHT ) )
	{
		angles[1] -= SPEED_TURN * frametime;
	}
	if ( Test_Key( 'F' ) )
	{
		origin[2] += SPEED_MOVE*frametime;
	}
	if ( Test_Key( 'C' ) )
	{
		origin[2] -= SPEED_MOVE*frametime;
	}
	if ( Test_Key( VK_INSERT ) )
	{
		angles[0] += SPEED_TURN * frametime;
		if (angles[0] > 85)
			angles[0] = 85;
	}
	if ( Test_Key( VK_DELETE ) )
	{
		angles[0] -= SPEED_TURN * frametime;
		if (angles[0] < -85)
			angles[0] = -85;
	}
	Cam_MouseMoved();
}

void Cam_BuildMatrix (void)
{
	float	xa, ya;
	float	matrix[4][4];
	int		i;

	xa = angles[0]/180*M_PI;
	ya = angles[1]/180*M_PI;

	// the movement matrix is kept 2d ?? do we want this?

    forward[0] = cos(ya);
    forward[1] = sin(ya);
    right[0] = forward[1];
    right[1] = -forward[0];

	glGetFloatv (GL_PROJECTION_MATRIX, &matrix[0][0]);

	for (i=0 ; i<3 ; i++)
	{
		vright[i] = matrix[i][0];
		vup[i] = matrix[i][1];
		vpn[i] = matrix[i][2];
	}

	VectorNormalize (vright);
	VectorNormalize (vup);
	VectorNormalize (vpn);
}

void Draw (void)
{
	float	screenaspect;
	float	yfov;

	//glClearColor (0.5, 0.5, 0.5, 0);
	glClearColor(0.0, 0.0, 0.0, 0);  // Black Clearing YWB
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//
	// set up viewpoint
	//
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity ();

    screenaspect = (float)width/height;
	yfov = 2*atan((float)height/width)*180/M_PI;
    gluPerspective (yfov,  screenaspect,  4,  8192);

    glRotatef (-90,  1, 0, 0);	    // put Z going up
    glRotatef (90,  0, 0, 1);	    // put Z going up
    glRotatef (angles[0],  0, 1, 0);
    glRotatef (-angles[1],  0, 0, 1);
    glTranslatef (-origin[0],  -origin[1],  -origin[2]);

	Cam_BuildMatrix ();

	//
	// set drawing parms
	//
	glShadeModel (GL_SMOOTH);

	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glFrontFace(GL_CW);  // YWB   Carmack goes backward
	glCullFace(GL_BACK); // Cull backfaces (qcsg used to spit out two sides, doesn't for -glview now)
	glEnable(GL_CULL_FACE); // Enable face culling, just in case...
	glDisable(GL_TEXTURE_2D);

	// Blending function if enabled..
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (g_UseBlending)
		glEnable(GL_BLEND);// YWB TESTING
	else
		glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	if( g_bDisp )
	{
		DrawDisplacementData();
	}
	else
	{
		//
		// draw the list
		//
		if (g_bShowList1)
			glCallList (1);
		
		if (g_bReadPortals)
		{
			if (g_bNoDepthPortals)
				glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE); // Disable face culling
			if (g_bShowList2)
				glCallList(2);
		};
		
		if (g_bShowLines)
			glCallList(3);
	}
}

void ReadPolyFileType(const char *name, int nList, BOOL drawLines)
{
	FILE	*f;
	int		i, j, numverts;
	float	v[8];
	int		c;
	int		r;
	float divisor;

	f = fopen (name, "r");
	if (!f)
		Error ("Couldn't open %s", name);

	if (g_bReadPortals)
		divisor = 2.0f;
	else 
		divisor = 1.0f;

	c = 0;
	glNewList (nList, GL_COMPILE);
	
	for (i = 0; i < 3; i++)  // Find the center point so we can put the viewer there by default
		g_Center[i] = 0.0f;

	if (drawLines)           // Slight hilite
		glLineWidth(1.5);

	while (1)
	{
		r = fscanf (f, "%i\n", &numverts);
		if (!r || r == EOF)
			break;
		
		if (drawLines || numverts == 2)
			glBegin(GL_LINE_LOOP);
		else
			glBegin (GL_POLYGON);

		for (i=0 ; i<numverts ; i++)
		{
			r = fscanf (f, "%f %f %f %f %f %f\n", &v[0], &v[1],
				&v[2], &v[3], &v[4], &v[5]);

			if (drawLines)  // YELLOW OUTLINES
				glColor4f(1.0, 1.0, 0.0, 0.5);
			else
			{
				if (g_bReadPortals)  // Gray scale it, leave portals blue
				{
					if (fabs(fabs(v[5]) - 1.0f) < 0.01)   // Is this a detail brush (color 0,0,1 blue)
					{
						glColor4f (v[3],v[4],v[5],0.5);   
					}	
					else                                  // Normal brush, gray scale it...
					{
						v[3] += v[4] + v[5];
						v[3]/= 3.0f;
						glColor4f (v[3]/divisor, v[3]/divisor, v[3]/divisor, 0.6);   
					}
				}
				else 
				{
					v[3] = pow( v[3], (float)(1.0 / 2.2) );
					v[4] = pow( v[4], (float)(1.0 / 2.2) );
					v[5] = pow( v[5], (float)(1.0 / 2.2) );

					glColor4f (v[3]/divisor, v[4]/divisor, 	v[5]/divisor, 0.6);   // divisor is one, bright colors
				};
			};
			glVertex3f (v[0], v[1], v[2]);

			for (j = 0; j < 3; j++)
			{
				g_Center[j] += v[j];
			}
	
			g_nTotalPoints++;
		}
		glEnd ();
		c++;
	}

	if (f)
		fclose(f);

	glEndList ();

	if (g_nTotalPoints > 0)  // Avoid division by zero
	{
		for (i = 0; i < 3; i++)
		{
			g_Center[i] = g_Center[i]/(float)g_nTotalPoints; // Calculate center...
			origin[i] = g_Center[i];
		}
	}
}

void ReadPHYFile(const char *name, int nList)
{
	FILE *fp = fopen (name, "rb");
	if (!fp)
		Error ("Couldn't open %s", name);

	phyheader_t header;
	
	fread( &header, sizeof(header), 1, fp );
	if ( header.size != sizeof(header) || header.solidCount <= 0 )
		return;

	int pos = ftell( fp );
	fseek( fp, 0, SEEK_END );
	int fileSize = ftell(fp) - pos;
	fseek( fp, pos, SEEK_SET );

	char *buf = (char *)_alloca( fileSize );
	fread( buf, fileSize, 1, fp );
	fclose( fp );

	vcollide_t collide;
	physcollision->VCollideLoad( &collide, header.solidCount, (const char *)buf, fileSize );

	int i;
	for (i = 0; i < 3; i++)  // Find the center point so we can put the viewer there by default
		g_Center[i] = 0.0f;

	glNewList (nList, GL_COMPILE);

	for ( i = 0; i < header.solidCount; i++ )
	{
		Vector *outVerts;
		int vertCount = physcollision->CreateDebugMesh( collide.solids[i], &outVerts );
		int triCount = vertCount / 3;
		int vert = 0;
		for ( int j = 0; j < triCount; j++ )
		{
			g_Center += outVerts[vert+0];
			g_Center += outVerts[vert+1];
			g_Center += outVerts[vert+2];
			g_nTotalPoints += 3;

			glBegin(GL_POLYGON);
			glColor3ub( 255, 0, 0 );
			glVertex3fv( outVerts[vert].Base() );
			vert++;
			glColor3ub( 0, 255, 0 );
			glVertex3fv( outVerts[vert].Base() );
			vert++;
			glColor3ub( 0, 0, 255 );
			glVertex3fv( outVerts[vert].Base() );
			vert++;
			glEnd();
		}
		physcollision->DestroyDebugMesh( vertCount, outVerts );
	}
	glEndList ();
	if (g_nTotalPoints > 0)  // Avoid division by zero
	{
		for (i = 0; i < 3; i++)
		{
			g_Center[i] = g_Center[i]/(float)g_nTotalPoints; // Calculate center...
			origin[i] = g_Center[i];
		}
	}
}

void ReadPolyFile (const char *name)
{
	char ext[4];
	Q_ExtractFileExtension( name, ext, 4 );

	if ( !Q_stricmp( ext, "phy" ) )
	{
		FileSystem_Init( name, 0, FS_INIT_COMPATIBILITY_MODE );
		CreateInterfaceFn physicsFactory = GetPhysicsFactory();
		physcollision = (IPhysicsCollision *)physicsFactory( VPHYSICS_COLLISION_INTERFACE_VERSION, NULL );
		if ( physcollision )
		{
			ReadPHYFile( name, 1 );
		}
	}
	else
	{
		// Read in polys...
		ReadPolyFileType(name, 1, false);

		// Make list 3 just the lines... so we can draw outlines
		ReadPolyFileType(name, 3, true);
	}
}

void ReadPortalFile (char *name)
{
	FILE	*f;
	int		i, j, numverts;
	float	v[8];
	int		c;
	int		r;

	// For Portal type reading...
	char szDummy[80];
	int nNumLeafs;
	int nNumPortals;
	int nDummy;

	f = fopen (name, "r");
	if (!f)
		Error ("Couldn't open %s", name);

	c = 0;
	
	glNewList (2, GL_COMPILE);

	// Read in header
	fscanf(f, "%79s\n", szDummy);
	fscanf(f, "%i\n", &nNumLeafs);
	fscanf(f, "%i\n", &nNumPortals);

	glLineWidth(1.5);

	while (1)
	{
		r = fscanf(f, "%i %i %i ", &numverts, &nDummy, &nDummy);
		if (!r || r == EOF)
			break;

		glBegin(GL_LINE_LOOP);
		for (i=0 ; i<numverts ; i++)
		{
			r = fscanf (f, "(%f %f %f )\n", &v[0], &v[1],
				&v[2]);
			if (!r || (r != 3) || r == EOF)
				break;

			if ( c == g_nPortalHighlight )
			{
				glColor4f (1.0, 0.0, 0.0, 1.0);   
			}
			else
			{
				glColor4f (1.0f, 1.0f, 1.0f, 1.0f);   // WHITE portals
			}
			glVertex3f (v[0], v[1], v[2]);
		}

		glEnd ();
		c++;
	}

	if (f)
		fclose(f);

	glEndList ();
}

#define MAX_DISP_COUNT	4096
static Vector dispPoints[MAX_DISP_COUNT];
static Vector dispNormals[MAX_DISP_COUNT];
static int dispPointCount = 0;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL ReadDisplacementFile( const char *filename )
{
	FILE	*pFile;
	int		fileCount;

	//
	// open the file
	//
	pFile = fopen( filename, "r" );
	if( !pFile )
		Error( "Couldn't open %s", filename );

	//
	// read data in file
	//
	while( 1 )
	{
		// overflow test
		if( dispPointCount >= MAX_DISP_COUNT )
			break;

		fileCount = fscanf( pFile, "%f %f %f %f %f %f",
			                &dispPoints[dispPointCount][0], &dispPoints[dispPointCount][1], &dispPoints[dispPointCount][2],
							&dispNormals[dispPointCount][0], &dispNormals[dispPointCount][1], &dispNormals[dispPointCount][2] );
		dispPointCount++;

		// end of file check
		if( !fileCount || ( fileCount == EOF ) )
			break;
	}

	fclose( pFile );

	return TRUE;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void DrawDisplacementData( void )
{
	int		i, j;
	int		width, halfCount;

	GLUquadricObj *pObject = gluNewQuadric();

	glEnable( GL_DEPTH_TEST );

	for( i = 0; i < dispPointCount; i++ )
	{
		// draw a sphere where the point is (in red)
		glColor3f( 1.0f, 0.0f, 0.0f );
		glPushMatrix();
		glTranslatef( dispPoints[i][0], dispPoints[i][1], dispPoints[i][2] );
		gluSphere( pObject, 5, 5, 5 );
		glPopMatrix();

		// draw the normal (in yellow)
		glColor3f( 1.0f, 1.0f, 0.0f );
		glBegin( GL_LINES );
		glVertex3f( dispPoints[i][0], dispPoints[i][1], dispPoints[i][2] );
		glVertex3f( dispPoints[i][0] + ( dispNormals[i][0] * 50.0f ), dispPoints[i][1] + ( dispNormals[i][1] * 50.0f ), dispPoints[i][2] + ( dispNormals[i][2] * 50.0f ) );
		glEnd();
	}

	halfCount = dispPointCount / 2;

	width = sqrt( (float)halfCount );

	glDisable( GL_CULL_FACE );

	glColor3f( 0.0f, 0.0f, 1.0f );
	for( i = 0; i < width - 1; i++ )
	{
		for( j = 0; j < width - 1; j++ )
		{
			glBegin( GL_POLYGON );
			glVertex3f( dispPoints[i*width+j][0], dispPoints[i*width+j][1], dispPoints[i*width+j][2] );
			glVertex3f( dispPoints[(i+1)*width+j][0], dispPoints[(i+1)*width+j][1], dispPoints[(i+1)*width+j][2] );
			glVertex3f( dispPoints[(i+1)*width+(j+1)][0], dispPoints[(i+1)*width+(j+1)][1], dispPoints[(i+1)*width+(j+1)][2] );
			glVertex3f( dispPoints[i*width+(j+1)][0], dispPoints[i*width+(j+1)][1], dispPoints[i*width+(j+1)][2] );
			glEnd();
		}
	}

#if 0
	for( i = 0; i < width - 1; i++ )
	{
		for( j = 0; j < width - 1; j++ )
		{
			glBegin( GL_POLYGON );
			glVertex3f( dispPoints[halfCount+(i*width+j)][0], dispPoints[halfCount+(i*width+j)][1], dispPoints[halfCount+(i*width+j)][2] );
			glVertex3f( dispPoints[halfCount+((i+1)*width+j)][0], dispPoints[halfCount+(i+1)*width+j][1], dispPoints[halfCount+((i+1)*width+j)][2] );
			glVertex3f( dispPoints[halfCount+((i+1)*width+(j+1))][0], dispPoints[halfCount+(i+1)*width+(j+1)][1], dispPoints[halfCount+((i+1)*width+(j+1))][2] );
			glVertex3f( dispPoints[halfCount+(i*width+(j+1))][0], dispPoints[halfCount+(i*width+(j+1))][1], dispPoints[halfCount+(i*width+(j+1))][2] );
			glEnd();
		}
	}
#endif

	glColor3f( 0.0f, 1.0f, 0.0f );
	for( i = 0; i < width - 1; i++ )
	{
		for( j = 0; j < width - 1; j++ )
		{
			glBegin( GL_POLYGON );
			glVertex3f( dispPoints[i*width+j][0] + ( dispNormals[i*width+j][0] * 150.0f ), 
				        dispPoints[i*width+j][1] + ( dispNormals[i*width+j][1] * 150.0f ), 
						dispPoints[i*width+j][2] + ( dispNormals[i*width+j][2] * 150.0f ) );

			glVertex3f( dispPoints[(i+1)*width+j][0] + ( dispNormals[(i+1)*width+j][0] * 150.0f ), 
				        dispPoints[(i+1)*width+j][1] + ( dispNormals[(i+1)*width+j][1] * 150.0f ), 
						dispPoints[(i+1)*width+j][2] + ( dispNormals[(i+1)*width+j][2] * 150.0f ) );

			glVertex3f( dispPoints[(i+1)*width+(j+1)][0] + ( dispNormals[(i+1)*width+(j+1)][0] * 150.0f ), 
				        dispPoints[(i+1)*width+(j+1)][1] + ( dispNormals[(i+1)*width+(j+1)][1] * 150.0f ), 
						dispPoints[(i+1)*width+(j+1)][2] + ( dispNormals[(i+1)*width+(j+1)][2] * 150.0f ) );

			glVertex3f( dispPoints[i*width+(j+1)][0] + ( dispNormals[i*width+(j+1)][0] * 150.0f ), 
				        dispPoints[i*width+(j+1)][1] + ( dispNormals[i*width+(j+1)][1] * 150.0f ), 
						dispPoints[i*width+(j+1)][2] + ( dispNormals[i*width+(j+1)][2] * 150.0f ) );
			glEnd();
		}
	}

	glDisable( GL_DEPTH_TEST );

	glColor3f( 0.0f, 0.0f, 1.0f );
	for( i = 0; i < width - 1; i++ )
	{
		for( j = 0; j < width - 1; j++ )
		{
			glBegin( GL_LINE_LOOP );
			glVertex3f( dispPoints[i*width+j][0] + ( dispNormals[i*width+j][0] * 150.0f ), 
				        dispPoints[i*width+j][1] + ( dispNormals[i*width+j][1] * 150.0f ), 
						dispPoints[i*width+j][2] + ( dispNormals[i*width+j][2] * 150.0f ) );

			glVertex3f( dispPoints[(i+1)*width+j][0] + ( dispNormals[(i+1)*width+j][0] * 150.0f ), 
				        dispPoints[(i+1)*width+j][1] + ( dispNormals[(i+1)*width+j][1] * 150.0f ), 
						dispPoints[(i+1)*width+j][2] + ( dispNormals[(i+1)*width+j][2] * 150.0f ) );

			glVertex3f( dispPoints[(i+1)*width+(j+1)][0] + ( dispNormals[(i+1)*width+(j+1)][0] * 150.0f ), 
				        dispPoints[(i+1)*width+(j+1)][1] + ( dispNormals[(i+1)*width+(j+1)][1] * 150.0f ), 
						dispPoints[(i+1)*width+(j+1)][2] + ( dispNormals[(i+1)*width+(j+1)][2] * 150.0f ) );

			glVertex3f( dispPoints[i*width+(j+1)][0] + ( dispNormals[i*width+(j+1)][0] * 150.0f ), 
				        dispPoints[i*width+(j+1)][1] + ( dispNormals[i*width+(j+1)][1] * 150.0f ), 
						dispPoints[i*width+(j+1)][2] + ( dispNormals[i*width+(j+1)][2] * 150.0f ) );
			glEnd();
		}
	}


	gluDeleteQuadric( pObject );
}


//=====================================================================

BOOL bSetupPixelFormat(HDC hDC)
{
    static PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
	1,				// version number
	PFD_DRAW_TO_WINDOW |		// support window
	  PFD_SUPPORT_OPENGL |		// support OpenGL
	  PFD_DOUBLEBUFFER,		// double buffered
	PFD_TYPE_RGBA,			// RGBA type
	24,				// 24-bit color depth
	0, 0, 0, 0, 0, 0,		// color bits ignored
	0,				// no alpha buffer
	0,				// shift bit ignored
	0,				// no accumulation buffer
	0, 0, 0, 0, 			// accum bits ignored
	32,				// 32-bit z-buffer	
	0,				// no stencil buffer
	0,				// no auxiliary buffer
	PFD_MAIN_PLANE,			// main layer
	0,				// reserved
	0, 0, 0				// layer masks ignored
    };

    int pixelformat = 0;

    if ( (pixelformat = ChoosePixelFormat(hDC, &pfd)) == 0 )
        Error ("ChoosePixelFormat failed");

    if (!SetPixelFormat(hDC, pixelformat, &pfd))
        Error ("SetPixelFormat failed");

    return TRUE;
}

/*
============
CameraWndProc
============
*/
LONG WINAPI WCam_WndProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    LONG    lRet = 1;
    RECT	rect;
	int		xPos, yPos, fwKeys;

    GetClientRect(hWnd, &rect);

    switch (uMsg)
    {
	case WM_CREATE:
		{
            camdc = GetDC(hWnd);
	    	bSetupPixelFormat(camdc);

            baseRC = wglCreateContext( camdc );
			if (!baseRC)
				Error ("wglCreateContext failed");
            if (!wglMakeCurrent( camdc, baseRC ))
				Error ("wglMakeCurrent failed");
			glCullFace(GL_FRONT);
			glEnable(GL_CULL_FACE);
		}
		break;
	case WM_PAINT:
        { 
		    PAINTSTRUCT	ps;

		    BeginPaint(hWnd, &ps);
            if (!wglMakeCurrent( camdc, baseRC ))
				Error ("wglMakeCurrent failed");
			Draw ();
			SwapBuffers(camdc);
		    EndPaint(hWnd, &ps);
        }
		break;
	
		case WM_KEYDOWN:
			KeyDown (wParam);
			AppKeyDown( wParam );
			break;
			
		case WM_KEYUP:
			AppKeyUp( wParam );
			break;

		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONDOWN:
			SetCapture (camerawindow);
			ShowCursor( FALSE );
			g_Capture = TRUE;
			break;

		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		case WM_LBUTTONUP:
			if (! (wParam & (MK_LBUTTON|MK_RBUTTON|MK_MBUTTON)))
			{
				g_Capture = FALSE;
				ReleaseCapture ();
				ShowCursor( TRUE );
			}
			break;

    	case WM_SIZE:
			InvalidateRect(camerawindow, NULL, false);
            break;
		case WM_NCCALCSIZE:// don't let windows copy pixels
			lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
			return WVR_REDRAW;
   	    case WM_CLOSE:
            /* call destroy window to cleanup and go away */
            DestroyWindow (hWnd);
        break;

   	    case WM_DESTROY:
        {
    	    HGLRC hRC;
    	    HDC	  hDC;

                /* release and free the device context and rendering context */
    	    hRC = wglGetCurrentContext();
    	    hDC = wglGetCurrentDC();

    	    wglMakeCurrent(NULL, NULL);

    	    if (hRC)
    	    	wglDeleteContext(hRC);
    	    if (hDC)
    	        ReleaseDC(hWnd, hDC);

                PostQuitMessage (0);
        }
        break;

    	default:
            /* pass all unhandled messages to DefWindowProc */
            lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
        break;
    }

    /* return 1 if handled message, 0 if not */
    return lRet;
}


/*
==============
WCam_Register
==============
*/
void WCam_Register (HINSTANCE hInstance)
{
    WNDCLASS   wc;

    /* Register the camera class */
	memset (&wc, 0, sizeof(wc));

    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)WCam_WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = "camera";

    if (!RegisterClass (&wc) )
        Error ("WCam_Register: failed");
}


void WCam_Create (HINSTANCE hInstance)
{
	// Center it
	int nScx, nScy;
	int w, h;
	int x, y;

	WCam_Register (hInstance);

	w = 640;
	h = 480;

	nScx = GetSystemMetrics(SM_CXSCREEN);
	nScy = GetSystemMetrics(SM_CYSCREEN);


	x = (nScx - w)/2;
	y = (nScy - h)/2;

	camerawindow = CreateWindow ("camera" ,
		"Camera View",
		WS_OVERLAPPED |
		WS_CAPTION |
		WS_SYSMENU |
		WS_THICKFRAME |
		WS_MAXIMIZEBOX |
		WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN,

		x,
		y,
		w,
		h,	// size

		NULL,	// parent window
		0,		// no menu
		hInstance,
		0);
	if (!camerawindow)
		Error ("Couldn't create camerawindow");

    ShowWindow (camerawindow, SW_SHOWDEFAULT);
}


void AppKeyDown( int key )
{
	key &= 0xFF;

	g_Keys[key] = 0x03; // add debounce bit
}

void AppKeyUp( int key )
{
	key &= 0xFF;

	g_Keys[key] &= 0x02;
}

void AppRender( void )
{
	static double lastTime = 0;
	double time = timeGetTime() * 0.001f;
	double frametime = time - lastTime;

	// clamp too large frames (like first frame)
	if ( frametime > 0.2 )
		frametime = 0.2;
	lastTime = time;

    if (!wglMakeCurrent( camdc, baseRC ))
		Error ("wglMakeCurrent failed");

	Cam_Update( frametime );

	if (g_Update)
	{
		Draw ();
		SwapBuffers(camdc);
		g_Update = FALSE;
	}
	else
	{
		Sleep( 1.0 );
	}
}


/*
==================
WinMain

==================
*/
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance
					,LPSTR lpCmdLine, int nCmdShow)
{
	CommandLine()->CreateCmdLine( lpCmdLine );

	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f );
    MSG        msg;

	if (!lpCmdLine || !lpCmdLine[0])
		Error ("No file specified");

	main_instance = hInstance;

	WCam_Create (hInstance);

	// Last argument is the file name
	const char *pFileName = CommandLine()->GetParm( CommandLine()->ParmCount() - 1 );

	if ( CommandLine()->CheckParm( "-portal") )
	{
		g_bReadPortals = 1;
		g_nPortalHighlight = CommandLine()->ParmValue( "-portalhighlight", -1 );
	}

	if( CommandLine()->CheckParm( "-disp") )
	{
		ReadDisplacementFile( pFileName );
		g_bDisp = TRUE;
	}

	// Any chunk of original left is the filename.
	if (pFileName && pFileName[0] && !g_bDisp )
	{
		ReadPolyFile( pFileName );
	}

	if (g_bReadPortals)
	{
		// Copy file again and this time look for the . from .gl? so we can concatenate .prt
		// and open the portal file.
		char szTempCmd[MAX_PATH];
		strcpy(szTempCmd, pFileName);
		char *pTmp = szTempCmd;
		while (pTmp && *pTmp && *pTmp != '.')
		{
			pTmp++;
		}

		*pTmp = '\0';
		strcat(szTempCmd, ".prt");

		ReadPortalFile(szTempCmd);
	};

    /* main window message loop */
	while (g_Active)
	{
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
            TranslateMessage (&msg);
            DispatchMessage (&msg);
		}
		AppRender();
	}

    /* return success of application */
    return TRUE;
}

