//----------------------------------------------------------------------------
#include <fstream.h>
#include <vcl.h>
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <gl\gl.h>      // Header file for the OpenGL32 library
#include <gl\glu.h>     // Header file for the GLu32 library
#include <gl\glaux.h>   // Header file for the GLaux library
#pragma hdrstop
#define Rz  6380.       //������ ����� � ��

const int MAP_SIZE_X = 1200;        // ������ ����� ����� Terrain.RAW
const int MAP_SIZE_Y = 1200;

const int STEP_SIZE  = 1;            // ��� ����������� ����� ������

int STEP_GRID = 256;  // ��� ����������� ����� ������

int		windowwidth;		// Holds The Current Window (Screen) Width
int		windowheight;		// Holds The Current Window (Screen) Height


long	frame = 0;			// Counter To Be Used In Calculating FPS
long	timebase = 0;		// Used In Calculating FPS
float	fps = 0.0f;			// Holds The Current FPS (Frames Per Second)
bool	culling = false;	// Indicates If Frustum Culling Is On Or Not

float	frustum[6][4];		// Holds The Current Frustum Plane Equations


//----------------------------------------------------------------------------
#pragma argsused
//----------------------------------------------------------------------------
HGLRC hRC = NULL;
HDC hDC = NULL;
HWND hWnd = NULL;
HINSTANCE hInstance = NULL;

GLuint		base;			// Base Display List For The Font Set

bool keys[256];
bool active = true;
bool fullscreen = true;


bool        isClicked  = false;										// NEW: Clicking The Mouse?
bool        isRClicked = false;										// NEW: Clicking The Right Mouse Button?
bool        isDragging = false;					                    // NEW: Dragging The Mouse?

GLUquadricObj *quadratic;
//---------------------------------------------------------------------------
BYTE g_HeightMap[MAP_SIZE_Y * MAP_SIZE_X];	// ����� ������� � ��������

int mouseX, mouseY;
GLfloat xrot = -62.0;	        // X ��������� ���� ��������
GLfloat yrot = 0.3;		        // Y
GLfloat zrot = 47.1;	        	  // Z
GLuint texture[ 1 ];      	     // ������ � ������ ��������(���������)
double points[ MAP_SIZE_Y][ MAP_SIZE_X][ 3 ];  // ������ ������� ����� ��������

double xa=0, ya=0, za=0;
double hmova=0, wmova=0;
double ScaleMap = -45.5;               // ������� �����
double ScaleHeight = 0.0235;           // ������� ������
double hMov = 24.5f, wMov = 24.5f;

double Lat, Lon, region;

//----------------------------------------------------------------------------
LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
//----------------------------------------------------------------------------
GLvoid BuildFont(GLvoid)								// Build Our Bitmap Font
{
	HFONT	font;										// Windows Font ID

	base = glGenLists(96);								// Storage For 96 Characters
	font = CreateFont(	-12,							// Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_NORMAL,						// Font Weight
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						ANSI_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						ANTIALIASED_QUALITY,			// Output Quality
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						"Courier New");					// Font Name
	SelectObject(hDC, font);							// Selects The Font We Created
	wglUseFontBitmaps(hDC, 32, 96, base);				// Builds 96 Characters Starting At Character 32
}

GLvoid KillFont(GLvoid)									// Delete The Font
{
 	glDeleteLists(base, 96);							// Delete All 96 Characters
}

GLvoid glPrint(float x, float y, const char *fmt, ...)	// Custom GL "Print" Routine
{
	char		text[256];								// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing
	va_start(ap, fmt);									// Parses The String For Variables
    vsprintf(text, fmt, ap);							// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text
	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(base - 32);								// Sets The Base Character to 32
	glRasterPos2f( x, y );								// Move To Drawing Position
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
}
//----------------------------------------------------------------------------
AUX_RGBImageRec *LoadBMP(char *Filename)
{
	FILE *File = NULL;
	if (!Filename)
		return NULL;
	File = fopen(Filename,"r");
	if (File)
	{
		fclose(File);
		return auxDIBImageLoad(Filename);
	}
	return NULL;
}
//----------------------------------------------------------------------------
int LoadGLTextures(char *Filename)
{
	int Status = false;
	AUX_RGBImageRec *TextureImage[1];
	memset(TextureImage,0,sizeof(void *)*1);
	if (TextureImage[0] = LoadBMP(Filename))
	{
		Status = true;
		glGenTextures(1, &texture[0]);
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	}
	if (TextureImage[0])
	{
		if (TextureImage[0]->data)
			free(TextureImage[0]->data);
		free(TextureImage[0]);
	}
	return Status;
}
//----------------------------------------------------------------------------
GLvoid ReSizeGLScene(GLsizei width, GLsizei height)
{
   if (height == 0)
      height = 1;

   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// ������ �����������
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height, 1.0f,1000.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	windowwidth = width;					   			// Save Width In Global
	windowheight = height;								// Save Height In Global
}
//----------------------------------------------------------------------------
// �������� ����� ������ �� terrain.raw
void LoadRawFile(LPSTR strName, int nSize, BYTE *pHeightMap)
{
	FILE *pFile = NULL;
	pFile = fopen( strName, "rb" );
	if ( pFile == NULL )
	{
		MessageBox(NULL, "��� ����� cashmap.raw - ����� ����� � ����������� EarthDATA\\CASHE\\", "Error", MB_OK);
		return;
	}
	fread( pHeightMap, 1, nSize, pFile );
	int result = ferror( pFile );
	if (result)
	{
		MessageBox(NULL, "�� ���� ��������� ���� cashmap.raw - ����� ����� � ����������� EarthDATA\\CASHE\\", "Error", MB_OK);
	}
	fclose(pFile);
}
//----------------------------------------------------------------------------
int InitGL(GLvoid)      // ������������� ���������� OpenGL
{
   BuildFont();
   String FileName = ExtractFilePath(Application->ExeName);
   FileName += "EarthDATA\\CASHE\\cashmap.bmp";
   if (!LoadGLTextures(FileName.c_str()))
		return false;
	glEnable(GL_TEXTURE_2D);                // ������������� �������
	glShadeModel(GL_SMOOTH);                // �������� ��� ����������������
	glClearColor(1.0f, 1.0f, 1.0f, 0.5f);   // ���� �������
	glClearDepth(1.0f);                     // ������
	glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);
	//glDepthFunc(GL_LESS);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);      // ��������� �����������

   //glPolygonMode(GL_BACK, GL_LINE);	   // ���������� �������
   glPolygonMode(GL_FRONT, GL_FILL);	// ���������� �����
   glFrontFace(GL_CW);

   glEnable(GL_CULL_FACE);  // ���������� ���������� �� ������� ������
   glCullFace (GL_BACK);    // ����� ��������� �������

   quadratic = gluNewQuadric();			        // Create a pointer to the quadric object ( NEW )
	gluQuadricNormals(quadratic, GLU_SMOOTH);       	// Create smooth normals ( NEW )
	gluQuadricTexture(quadratic, GL_TRUE);	        	// Create texture coords ( NEW )

   FileName = ExtractFilePath(Application->ExeName);
   FileName += "EarthDATA\\CASHE\\cashmap.raw";
   LoadRawFile(FileName.c_str(), MAP_SIZE_Y * MAP_SIZE_X, g_HeightMap);
   //  Load Coord
   FileName = ExtractFilePath(Application->ExeName);
   FileName += "EarthDATA\\CASHE\\coord.tmp";
   ifstream file(FileName.c_str());
   if(file)
   {
      file >> Lat;
      file >> Lon;
      file >> region;
   }
   file.close();
   //--
       //����
       GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
       GLfloat mat_shininess[] = {50.0};
       GLfloat light_position[] = {24.5, 50.5, 50.0, 1.0};
       GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};

       glClearColor(1.0, 1.0, 1.0, 1.0);
       glShadeModel(GL_FLAT);
      //glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
       //glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
       glMaterialfv(GL_BACK, GL_SPECULAR, mat_specular);
       //glMaterialfv(GL_BACK, GL_SHININESS, mat_shininess);
       glLightfv(GL_LIGHT0, GL_POSITION, light_position);
       glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
       glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);

       glEnable(GL_LIGHTING);
       glEnable(GL_LIGHT0);
       glEnable(GL_DEPTH_TEST);

       // �����
       GLint fogMode;
       glEnable(GL_FOG);
       {
         GLfloat fogColor[4] = {1.0, 1.0, 1.0, 1.0};
         fogMode = GL_EXP2;
         glFogi(GL_FOG_MODE, fogMode);
         glFogfv(GL_FOG_COLOR, fogColor);
         glFogf(GL_FOG_DENSITY, 0.008);
         glHint(GL_FOG_HINT, GL_NICEST);
         glFogf(GL_FOG_START, 3.0);
         glFogf(GL_FOG_END, 5.0);
       }
   //--
	return true;
}
//----------------------------------------------------------------------------
int Height(BYTE *pHeightMap, int Y, int X)
{
   int y = Y % MAP_SIZE_Y;
	int x = MAP_SIZE_X-X % MAP_SIZE_X;
	if(!pHeightMap)
      return 0;
   BYTE temp;
  temp = pHeightMap[(x * MAP_SIZE_Y) + y];
   //if(temp == (byte)"�"){temp = (int)0;} // �������� ��� ������ � �������� raw �� �������������� ������

	return temp;
}
//----------------------------------------------------------------------------

void ortho( void )										// Changes Projection To Orthgonal View For Text
{
	glDisable(GL_DEPTH_TEST);							// Disable Depth Testing (All Text On Top!)
	glDisable(GL_LIGHTING);								// Disable Lighting
	glMatrixMode(GL_PROJECTION);						// Save The Current PROJECTION Matrix
	glPushMatrix();
	glLoadIdentity();
	glOrtho( 0, windowwidth, windowheight, 0, -1, 1 );	// Enter Orthographic Mode
	glMatrixMode(GL_MODELVIEW);							// Save The MODELVIEW Matrix
	glPushMatrix();
	glLoadIdentity();
}

void perspective( void )								// Changes Projection To Perspective View
{
	glMatrixMode( GL_PROJECTION );						// Restore The PROJECTION Matrix
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );						// Restore The MODELVIEW Matrix
	glPopMatrix();
	glEnable( GL_DEPTH_TEST );							// Enable Depth Testing
	glEnable(GL_LIGHTING);								// Enable Lighting
}

void ExtractFrustum()									// Extracts The Current View Frustum Plane Equations
{
	float	proj[16];									// For Grabbing The PROJECTION Matrix
	float	modl[16];									// For Grabbing The MODELVIEW Matrix
	float	clip[16];									// Result Of Concatenating PROJECTION and MODELVIEW
	float	t;											// Temporary Work Variable

	glGetFloatv( GL_PROJECTION_MATRIX, proj );			// Grab The Current PROJECTION Matrix
	glGetFloatv( GL_MODELVIEW_MATRIX, modl );			// Grab The Current MODELVIEW Matrix

	// Concatenate (Multiply) The Two Matricies
	clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
	clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
	clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
	clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

	clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
	clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
	clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
	clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

	clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
	clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
	clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
	clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

	clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
	clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
	clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
	clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];


	// Extract the RIGHT clipping plane
	frustum[0][0] = clip[ 3] - clip[ 0];
	frustum[0][1] = clip[ 7] - clip[ 4];
	frustum[0][2] = clip[11] - clip[ 8];
	frustum[0][3] = clip[15] - clip[12];

	// Normalize it
	t = (float) sqrt( frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1] + frustum[0][2] * frustum[0][2] );
	frustum[0][0] /= t;
	frustum[0][1] /= t;
	frustum[0][2] /= t;
	frustum[0][3] /= t;


	// Extract the LEFT clipping plane
	frustum[1][0] = clip[ 3] + clip[ 0];
	frustum[1][1] = clip[ 7] + clip[ 4];
	frustum[1][2] = clip[11] + clip[ 8];
	frustum[1][3] = clip[15] + clip[12];

	// Normalize it
	t = (float) sqrt( frustum[1][0] * frustum[1][0] + frustum[1][1] * frustum[1][1] + frustum[1][2] * frustum[1][2] );
	frustum[1][0] /= t;
	frustum[1][1] /= t;
	frustum[1][2] /= t;
	frustum[1][3] /= t;


	// Extract the BOTTOM clipping plane
	frustum[2][0] = clip[ 3] + clip[ 1];
	frustum[2][1] = clip[ 7] + clip[ 5];
	frustum[2][2] = clip[11] + clip[ 9];
	frustum[2][3] = clip[15] + clip[13];

	// Normalize it
	t = (float) sqrt( frustum[2][0] * frustum[2][0] + frustum[2][1] * frustum[2][1] + frustum[2][2] * frustum[2][2] );
	frustum[2][0] /= t;
	frustum[2][1] /= t;
	frustum[2][2] /= t;
	frustum[2][3] /= t;


	// Extract the TOP clipping plane
	frustum[3][0] = clip[ 3] - clip[ 1];
	frustum[3][1] = clip[ 7] - clip[ 5];
	frustum[3][2] = clip[11] - clip[ 9];
	frustum[3][3] = clip[15] - clip[13];

	// Normalize it
	t = (float) sqrt( frustum[3][0] * frustum[3][0] + frustum[3][1] * frustum[3][1] + frustum[3][2] * frustum[3][2] );
	frustum[3][0] /= t;
	frustum[3][1] /= t;
	frustum[3][2] /= t;
	frustum[3][3] /= t;


	// Extract the FAR clipping plane
	frustum[4][0] = clip[ 3] - clip[ 2];
	frustum[4][1] = clip[ 7] - clip[ 6];
	frustum[4][2] = clip[11] - clip[10];
	frustum[4][3] = clip[15] - clip[14];

	// Normalize it
	t = (float) sqrt( frustum[4][0] * frustum[4][0] + frustum[4][1] * frustum[4][1] + frustum[4][2] * frustum[4][2] );
	frustum[4][0] /= t;
	frustum[4][1] /= t;
	frustum[4][2] /= t;
	frustum[4][3] /= t;


	// Extract the NEAR clipping plane.  This is last on purpose (see pointinfrustum() for reason)
	frustum[5][0] = clip[ 3] + clip[ 2];
	frustum[5][1] = clip[ 7] + clip[ 6];
	frustum[5][2] = clip[11] + clip[10];
	frustum[5][3] = clip[15] + clip[14];

	// Normalize it
	t = (float) sqrt( frustum[5][0] * frustum[5][0] + frustum[5][1] * frustum[5][1] + frustum[5][2] * frustum[5][2] );
	frustum[5][0] /= t;
	frustum[5][1] /= t;
	frustum[5][2] /= t;
	frustum[5][3] /= t;
}

// Test If A Point Is In The Frustum.
bool PointInFrustum( float x, float y, float z )
{
	int p;

	for( p = 0; p < 6; p++ )
		if( frustum[p][0] * x + frustum[p][1] * y + frustum[p][2] * z + frustum[p][3] <= 0 )
			return false;
	return true;
}

// Test If A Sphere Is In The Frustum
bool SphereInFrustum( float x, float y, float z, float radius )
{
	int p;

	for( p = 0; p < 6; p++ )
		if( frustum[p][0] * x + frustum[p][1] * y + frustum[p][2] * z + frustum[p][3] <= -radius )
			return false;
	return true;
}

// Test If A Cube Is In The Frustum
bool CubeInFrustum( float x, float y, float z, float size )
{
	int p;

	for( p = 0; p < 6; p++ )
	{
		if( frustum[p][0] * (x - size) + frustum[p][1] * (y - size) + frustum[p][2] * (z - size) + frustum[p][3] > 0 )
			continue;
		if( frustum[p][0] * (x + size) + frustum[p][1] * (y - size) + frustum[p][2] * (z - size) + frustum[p][3] > 0 )
			continue;
		if( frustum[p][0] * (x - size) + frustum[p][1] * (y + size) + frustum[p][2] * (z - size) + frustum[p][3] > 0 )
			continue;
		if( frustum[p][0] * (x + size) + frustum[p][1] * (y + size) + frustum[p][2] * (z - size) + frustum[p][3] > 0 )
			continue;
		if( frustum[p][0] * (x - size) + frustum[p][1] * (y - size) + frustum[p][2] * (z + size) + frustum[p][3] > 0 )
			continue;
		if( frustum[p][0] * (x + size) + frustum[p][1] * (y - size) + frustum[p][2] * (z + size) + frustum[p][3] > 0 )
			continue;
		if( frustum[p][0] * (x - size) + frustum[p][1] * (y + size) + frustum[p][2] * (z + size) + frustum[p][3] > 0 )
			continue;
		if( frustum[p][0] * (x + size) + frustum[p][1] * (y + size) + frustum[p][2] * (z + size) + frustum[p][3] > 0 )
			continue;
		return false;
	}
	return true;
}

//----------------------------------------------------------------------------
int DrawGLScene(GLvoid)
{
   int i;
	long time;											// Used In Calculating FPS
	bool visible;										// Used To Test If An Object Was Culled

	frame++;											// Increment Frame Counter
	time = GetTickCount();								// Get Timer Value
	if( time - timebase >= 1000 )						// Has A Second Passed Since Last FPS Update?
	{
		fps = frame * 1000.0f / (time - timebase);		// Calculate New FPS
	 	timebase = time;								// Save Current Tick Count
		frame = 0;										// Start Counting Frames From Zero Again
	}

   double x, y, z;
	double double_x, double_y, double_xb, double_yb;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //CLS
	glLoadIdentity();                     // ����� ������

   glTranslatef(0.0f,0.0f,ScaleMap);     // �����������

	glRotatef(xrot,1.0f,0.0f,0.0f);		// �������� X
	glRotatef(yrot,0.0f,1.0f,0.0f);		// Y
	glRotatef(zrot,0.0f,0.0f,1.0f);		// Z

   ExtractFrustum();									// Extract The Frustum Plane Equations

   glBegin(GL_QUADS);
	for( x = 1; x  < (STEP_GRID-STEP_SIZE-1); x++ )
   {
		for( y = 1; y < (STEP_GRID-STEP_SIZE-1); y++ )
		{
         visible = PointInFrustum(points[(int)x][(int)y][0], points[(int)x][(int)y][1], points[(int)x][(int)y][2]);
         if( visible )
		   {
            double_x = (double)x/(STEP_GRID-STEP_SIZE)  ;
		   	double_y = (double)y/(STEP_GRID-STEP_SIZE);
	   		double_xb = (double)(x+1.0)/(STEP_GRID-STEP_SIZE);
   			double_yb = (double)(y+1.0)/(STEP_GRID-STEP_SIZE);

            float colorindex;
            colorindex = 0.7 + points[(int)x][(int)y][2] - 0.010*STEP_GRID
               *cos((MAP_SIZE_Y*(double)(x)*region)/(50*Rz*(STEP_GRID-STEP_SIZE-1)) - M_PI_4)
               *cos((MAP_SIZE_X*(double)(y)*region)/(50*Rz*(STEP_GRID-STEP_SIZE-1)) - M_PI_4);
            //colorindex = 0.7;
            glColor3f(colorindex,colorindex,colorindex);    
            /*if(points[(int)x][(int)y][0] > 10 || points[(int)x][(int)y][1] > 10)
            {
               colorindex = 0.7 + points[(int)x][(int)y][2]
            }   */
            glTexCoord2f(double_x, double_y);	// ����� ������
            glVertex3f( points[(int)x][(int)y][0], points[(int)x][(int)y][1],points[(int)x][(int)y][2]);

   			glTexCoord2f( double_x, double_yb );	// ����� �������
	   		glVertex3f(points[(int)x][(int)y+1][0], points[(int)x][(int)y+1][1],points[(int)x][(int)y+1][2]);

   			glTexCoord2f( double_xb, double_yb );	// ������ �������
	   		glVertex3f( points[(int)x+1][(int)y+1][0], points[(int)x+1][(int)y+1][1],points[(int)x+1][(int)y+1][2]);

   			glTexCoord2f( double_xb, double_y );	// ������ ������
	   		glVertex3f( points[(int)x+1][(int)y][0], points[(int)x+1][(int)y][1], points[(int)x+1][(int)y][2]);
         }
		}
   }
   glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[0]);		// ����� ��������
	int X = 0, Y = 0;
  	for ( X = 1; X < (STEP_GRID-STEP_SIZE); X = X + STEP_SIZE)
		for ( Y = 1; Y < (STEP_GRID-STEP_SIZE); Y = Y + STEP_SIZE)
		{
			// ��������� �������� �� ����� �����
            x = Y;
            y = X;
            z = 1 + Height(g_HeightMap,
               MAP_SIZE_Y*(double)(y)/(STEP_GRID-STEP_SIZE),
               MAP_SIZE_X*(double)(x)/(STEP_GRID-STEP_SIZE));

            double r, tetta, fi;
            //r = pow(((double)x*(double)x + (double)y*(double)y + (double)z*(double)z), 0.5);
           /* r = sqrt((double)x*(double)x + (double)y*(double)y + (double)z*(double)z);
            tetta = -1;
            fi = -100;
            if( z > r )
            {
               z = r;
            }
            tetta = acos(z / r);
            double atanYX;
            if(fabs(x) < 1e-6)
            {
               atanYX = y>=0 ? M_PI_2: -M_PI_2;
            }
            else atanYX = atan(y / x);        //  atan2(y,x)
            {
               fi = fabs(atanYX) < 1e-6 ? 2*M_PI : 1/atanYX;
            }    */

            //if( visible )
		      {
               Lat;
               double a,b;
               a = sin(Lat + region * (double)Y /  (double(STEP_GRID-STEP_SIZE)* 240.0 * 9));
               a = fabs(a);
               b = fabs(((double)X - (double)X * a )/2.0);

               //points[X][Y][0] = double((X/5.0f)-wMov);
               points[X][Y][0] = double(a*(X) - wMov - a*double(STEP_GRID-STEP_SIZE)/2);
               //points[X][Y][1] = double((Y/5.0f)-hMov);
	   		   points[X][Y][1] = double((Y)-hMov);

               points[X][Y][2] = double((z * ScaleHeight)
               +0.010*STEP_GRID
               *cos((MAP_SIZE_Y*(double)(Y)*region)/(50*Rz*(STEP_GRID-STEP_SIZE-1)) - M_PI_4)
               *cos((MAP_SIZE_X*(double)(X)*region)/(50*Rz*(STEP_GRID-STEP_SIZE-1)) - M_PI_4));

               //if (z > 32) points[X][Y][2] = double(10*cos(M_PI_2*y/(60*Rz/region) - M_PI_4)*cos(M_PI_2*x/(60*Rz/region) - M_PI_4));
            }
      }

   xrot+=xa;
   yrot+=ya;
   zrot+=za;
   hMov += hmova;
   wMov += wmova;
   
   glBegin(GL_TRIANGLES);
   glColor3f( 1.0f, 0.5f, 1.0f );
   glVertex3f(points[(int)(STEP_GRID * (double)mouseX/(double)windowwidth)][(int)(STEP_GRID * (1-(double)mouseY/(double)windowheight))][0] - 1,
              points[(int)(STEP_GRID * (double)mouseX/(double)windowwidth)][(int)(STEP_GRID * (1-(double)mouseY/(double)windowheight))][1] - 1,
              points[(int)(STEP_GRID * (double)mouseX/(double)windowwidth)][(int)(STEP_GRID * (1-(double)mouseY/(double)windowheight))][2] + 1);
   glVertex3f(points[(int)(STEP_GRID * (double)mouseX/(double)windowwidth)][(int)(STEP_GRID * (1-(double)mouseY/(double)windowheight))][0] + 1,
              points[(int)(STEP_GRID * (double)mouseX/(double)windowwidth)][(int)(STEP_GRID * (1-(double)mouseY/(double)windowheight))][1] + 1,
              points[(int)(STEP_GRID * (double)mouseX/(double)windowwidth)][(int)(STEP_GRID * (1-(double)mouseY/(double)windowheight))][2] + 1);
   glVertex3f(points[(int)(STEP_GRID * (double)mouseX/(double)windowwidth)][(int)(STEP_GRID * (1-(double)mouseY/(double)windowheight))][0] +1,
              points[(int)(STEP_GRID * (double)mouseX/(double)windowwidth)][(int)(STEP_GRID * (1-(double)mouseY/(double)windowheight))][1] -1,
              points[(int)(STEP_GRID * (double)mouseX/(double)windowwidth)][(int)(STEP_GRID * (1-(double)mouseY/(double)windowheight))][2] + 1);
/*   glVertex3f(points[(int)(STEP_GRID * (double)mouseX/(double)windowwidth)][(int)(STEP_GRID * (1-(double)mouseY/(double)windowheight))][0] - 1,
              points[(int)(STEP_GRID * (double)mouseX/(double)windowwidth)][(int)(STEP_GRID * (1-(double)mouseY/(double)windowheight))][1] + 1,
              points[(int)(STEP_GRID * (double)mouseX/(double)windowwidth)][(int)(STEP_GRID * (1-(double)mouseY/(double)windowheight))][2]);
   glVertex3f(points[(int)(STEP_GRID * (double)mouseX/(double)windowwidth)][(int)(STEP_GRID * (1-(double)mouseY/(double)windowheight))][0],
              points[(int)(STEP_GRID * (double)mouseX/(double)windowwidth)][(int)(STEP_GRID * (1-(double)mouseY/(double)windowheight))][1],
              points[(int)(STEP_GRID * (double)mouseX/(double)windowwidth)][(int)(STEP_GRID * (1-(double)mouseY/(double)windowheight))][2] + 1);      */
   //gluDisk(quadratic,0.5f,10.5f,32,32);
	glEnd();
   glDisable(GL_TEXTURE_2D);
   ortho();													// Enter Orthographic Mode
	glColor3f( 0.0f, 0.5f, 1.0f );								// Set Color To White
	glPrint( 1, 10, "FPS    : %0.1f", fps );					// Display Current FPS
   glPrint( 1, 20, "Lat    : %0.1f", (Lat - (10) * (double)mouseY/(double)windowheight));
   glPrint( 1, 30, "Lon    : %0.1f", (Lon + (10) * (double)mouseX/(double)windowwidth));
   glPrint( 1, 40, "Height : %0.1f", points[(int)(STEP_GRID * (double)mouseX/(double)windowwidth)][(int)(STEP_GRID * (1-(double)mouseY/(double)windowheight))][2] - 0.010*STEP_GRID
               *cos((MAP_SIZE_Y*(double)(Y)*region)/(50*Rz*(STEP_GRID-STEP_SIZE-1)) - M_PI_4)
               *cos((MAP_SIZE_X*(double)(X)*region)/(50*Rz*(STEP_GRID-STEP_SIZE-1)) - M_PI_4));
   glPrint( 1, 50, "X      : %0.01f", (double)mouseX/(double)windowwidth);
   glPrint( 1, 60, "Y      : %0.01f", (double)mouseY/(double)windowheight);
   glPrint( 1, 70, "xrot    : %0.1f", (double)xrot);
   glPrint( 1, 80, "yrot    : %0.1f", (double)yrot);
   glPrint( 1, 90, "yrot    : %0.1f", (double)zrot);
   glPrint( 1, 100, "ScaleMap: %0.1f", (double)ScaleMap);
   glPrint( 1, 110, "hMov    : %0.1f", (double)hMov);
   glPrint( 1, 120, "wMov    : %0.1f", (double)wMov);
   glPrint( 1, 130, "StepGrid: %0.1f", (double)STEP_GRID);
   glPrint( 1, 140, "StepSize: %0.1f", (double)STEP_SIZE);
   glColor3f( 1.0f, 1.0f, 1.0f );
   perspective();
   glEnable(GL_TEXTURE_2D);
	return true;
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/*	This Code Creates Our OpenGL Window.  Parameters Are:
 *	title			- Title To Appear At The Top Of The Window
 *	width			- Width Of The GL Window Or Fullscreen Mode
 *	height			- Height Of The GL Window Or Fullscreen Mode
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)*/

GLvoid KillGLWindow(GLvoid)
{
   gluDeleteQuadric(quadratic);	// Delete Quadratic - Free Resources
   
	if (fullscreen)
	{
		ChangeDisplaySettings(NULL,0);
		ShowCursor(true);
	}
	if (hRC)
	{
		if (!wglMakeCurrent(NULL,NULL))
		{
			MessageBox(NULL,"Release of DC and RC failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))             // Are we able to delete the RC?
		{
			MessageBox(NULL,"Release rendering context failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;             // Set RC to NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))        // Are we able to release the DC
	{
		MessageBox(NULL,"Release device context failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC = NULL;             // Set DC to NULL
	}

	if (hWnd && !DestroyWindow(hWnd))       // Are we able to destroy the window?
	{
		MessageBox(NULL,"Could not release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;            // Set hWnd to NULL
	}

	if (!UnregisterClass("OpenGL",hInstance))       // Are we able to unregister class
	{
		MessageBox(NULL,"Could not unregister class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;       // Set hInstance to NULL
	}
}

BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;		// Holds the results after searching for a match
	WNDCLASS	wc;		        // Windows class structure
	DWORD		dwExStyle;              // Window extended style
	DWORD		dwStyle;                // Window style
	RECT		WindowRect;             // Grabs rctangle upper left / lower right values
	WindowRect.left = (long)0;              // Set left value to 0
	WindowRect.right = (long)width;		// Set right value to requested width
	WindowRect.top = (long)0;               // Set top value to 0
	WindowRect.bottom = (long)height;       // Set bottom value to requested height

	fullscreen = fullscreenflag;              // Set the global fullscreen flag

	hInstance               = GetModuleHandle(NULL);		// Grab an instance for our window
	wc.style                = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;   // Redraw on size, and own DC for window
	wc.lpfnWndProc          = (WNDPROC) WndProc;			// WndProc handles messages
	wc.cbClsExtra           = 0;					// No extra window data
	wc.cbWndExtra           = 0;					// No extra window data
	wc.hInstance            = hInstance;				// Set the Instance
	wc.hIcon                = LoadIcon(NULL, IDI_WINLOGO);		// Load the default icon
	wc.hCursor              = LoadCursor(NULL, IDC_ARROW);		// Load the arrow pointer
	wc.hbrBackground        = NULL;					// No background required for GL
	wc.lpszMenuName		= NULL;					// We don't want a menu
	wc.lpszClassName	= "OpenGL";				// Set the class name

	if (!RegisterClass(&wc))					// Attempt to register the window class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);

		return false;   // Return FALSE
	}

	if (fullscreen)         // Attempt fullscreen mode?
	{
		DEVMODE dmScreenSettings;                                     // Device mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	        // Makes sure memory's cleared
		dmScreenSettings.dmSize       = sizeof(dmScreenSettings);     // Size of the devmode structure
		dmScreenSettings.dmPelsWidth	= width;                        // Selected screen width
		dmScreenSettings.dmPelsHeight	= height;                       // Selected screen height
		dmScreenSettings.dmBitsPerPel	= bits;	                       // Selected bits per pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try to set selected mode and get results. NOTE: CDS_FULLSCREEN gets rid of start bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If the mode fails, offer two options. Quit or use windowed mode.
			if (MessageBox(NULL,"The requested fullscreen mode is not supported by\nyour video card. Use windowed mode instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen = false;       // Windowed mode selected. Fullscreen = FALSE
			}
			else
			{
				// Pop up a message box letting user know the program is closing.
				MessageBox(NULL,"Program will now close.","ERROR",MB_OK|MB_ICONSTOP);
				return false;           // Return FALSE
			}
		}
	}

	if (fullscreen)                         // Are We Still In Fullscreen Mode?
	{
		dwExStyle = WS_EX_APPWINDOW;    // Window extended style
		dwStyle = WS_POPUP;		// Windows style
		ShowCursor(true);		// Hide mouse pointer
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;           // Window extended style
		dwStyle=WS_OVERLAPPEDWINDOW;                            // Windows style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);     // Adjust window to true requested size

	// Create the window
	if (!(hWnd = CreateWindowEx(dwExStyle,          // Extended Style For The Window
                "OpenGL",				// Class name
		title,					// Window title
		dwStyle |				// Defined window style
		WS_CLIPSIBLINGS |			// Required window style
		WS_CLIPCHILDREN,			// Required window style
		0, 0,					// Window position
		WindowRect.right-WindowRect.left,	// Calculate window width
		WindowRect.bottom-WindowRect.top,	// Calculate window height
		NULL,					// No parent window
		NULL,					// No menu
		hInstance,				// Instance
		NULL)))					// Dont pass anything to WM_CREATE
	{
		KillGLWindow();                         // Reset the display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;                           // Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd =             // pfd tells windows how we want things to be
	{
		sizeof(PIXELFORMATDESCRIPTOR),          // Size of this pixel format descriptor
		1,					// Version number
		PFD_DRAW_TO_WINDOW |			// Format must support window
		PFD_SUPPORT_OPENGL |			// Format must support OpenGL
		PFD_DOUBLEBUFFER,			// Must support double buffering
		PFD_TYPE_RGBA,				// Request an RGBA format
		bits,					// Select our color depth
		0, 0, 0, 0, 0, 0,			// Color bits ignored
		0,					// No alpha buffer
		0,					// Shift bit ignored
		0,					// No accumulation buffer
		0, 0, 0, 0,				// Accumulation bits ignored
		32,					// 16Bit Z-Buffer (Depth buffer)
		0,					// No stencil buffer
		0,					// No auxiliary buffer
		PFD_MAIN_PLANE,				// Main drawing layer
		0,					// Reserved
		0, 0, 0					// Layer masks ignored
	};

	if (!(hDC = GetDC(hWnd)))         // Did we get a device context?
	{
		KillGLWindow();         // Reset the display
		MessageBox(NULL,"Can't create a GL device context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;           // Return FALSE
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC,&pfd)))	// Did windows find a matching pixel format?
	{
		KillGLWindow();         // Reset the display
		MessageBox(NULL,"Can't find a suitable pixelformat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;           // Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))       // Are we able to set the pixel format?
	{
		KillGLWindow();         // Reset the display
		MessageBox(NULL,"Can't set the pixelformat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;           // Return FALSE
	}

	if (!(hRC = wglCreateContext(hDC)))               // Are we able to get a rendering context?
	{
		KillGLWindow();         // Reset the display
		MessageBox(NULL,"Can't create a GL rendering context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;           // Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))    // Try to activate the rendering context
	{
		KillGLWindow();         // Reset the display
		MessageBox(NULL,"Can't activate the GL rendering context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;           // Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);       // Show the window
	SetForegroundWindow(hWnd);      // Slightly higher priority
	SetFocus(hWnd);                 // Sets keyboard focus to the window
	ReSizeGLScene(width, height);   // Set up our perspective GL screen

	if (!InitGL())                  // Initialize our newly created GL window
	{
		KillGLWindow();         // Reset the display
		MessageBox(NULL,"Initialization failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;           // Return FALSE
	}
	return true;                    // Success
}

LRESULT CALLBACK WndProc(HWND hWnd,     // Handle for this window
                        UINT uMsg,      // Message for this window
			WPARAM wParam,  // Additional message information
			LPARAM lParam)  // Additional message information
{
   double dmouseX = mouseX;
   double dmouseY = mouseY;
	switch (uMsg)                           // Check for windows messages
	{
        case WM_MOUSEMOVE:
            mouseX = (GLfloat)LOWORD(lParam);
            mouseY = (GLfloat)HIWORD(lParam);
            dmouseX -= mouseX;
            dmouseY -= mouseY;
            isClicked   = (LOWORD(wParam) & MK_LBUTTON) ? true : false;
            isRClicked  = (LOWORD(wParam) & MK_RBUTTON) ? true : false;
            if(isClicked)
            {
               yrot -= (double)dmouseX * 0.05f;
               xrot += (double)dmouseY * 0.05f;
            }
            if(isRClicked)
            {
               ScaleMap += (double)dmouseY * 0.05f;
               zrot -= (double)dmouseX * 0.05f;
            }
            break;
        case WM_LBUTTONUP:
            isClicked   = false;
            break;
        case WM_RBUTTONUP:
            isRClicked  = false;
            break;
        case WM_LBUTTONDOWN:
            isClicked   = true;
            break;
        case WM_RBUTTONDOWN:
            isRClicked  = true;
            break;

		case WM_ACTIVATE:               // Watch for window activate message
		{
			if (!HIWORD(wParam))    // Check minimization state
			{
				active = true;  // Program is active
			}
			else
			{
				active = false; // Program is no longer active
			}

			return 0;               // Return to the message loop
		}

		case WM_SYSCOMMAND:             // Intercept system commands
		{
			switch (wParam)         // Check system calls
			{
				case SC_SCREENSAVE:     // Screensaver trying to start?
				case SC_MONITORPOWER:	// Monitor trying to enter powersave?
				return 0;       // Prevent from happening
			}
			break;                  // Exit
		}

		case WM_CLOSE:                  // Did we receive a close message?
		{
			PostQuitMessage(0);     // Send a quit message
			return 0;               // Jump back
		}

		case WM_KEYDOWN:                // Is a key being held down?
		{
			keys[wParam] = true;    // If so, mark it as TRUE
//����������---------------------------------------------
         int a;
         a = wParam;
         if (wParam ==82)
            hmova += 0.02;
         if (wParam ==69)
            hmova -= 0.02;
         if (wParam ==86)
            wmova +=0.1;
         if (wParam ==67)
            wmova -=0.1;

	      if (wParam == 38)
            xrot+=0.5f;
         if (wParam == 40)
            xrot-=0.5f;
         if (wParam == 39)
            yrot+=0.5f;
         if (wParam == 37)
	         yrot-=0.5f;
         if (wParam == 34)
	         zrot+=0.5f;
         if (wParam == 46)
	         zrot-=0.5f;
         if (wParam == 107)
         {
            ScaleMap+=0.5f;
         }
         if (wParam == 109)
         {
            ScaleMap-=0.5f;
         }
         if (wParam == 36)
            ScaleHeight += 0.002;
         if (wParam == 35)
            ScaleHeight -= 0.002;
         if (wParam ==104)
            hMov -= 5;
         if (wParam ==98)
            hMov += 5;
         if (wParam ==102)
            wMov -= 5;
         if (wParam ==100)
            wMov += 5;

         if (wParam == 81)
            xa+=0.1;
         if (wParam == 87)
            xa-=0.1;
         if (wParam == 65)
            ya+=0.1;
         if (wParam == 83)
            ya-=0.1;
         if (wParam == 90)
            za+=0.1;
         if (wParam == 88)
            za-=0.1;
         if (wParam == 222)
         {
            STEP_GRID += 50;
            hMov += 25/10;
            wMov += 25/10;
            ScaleMap -= 0.5f;

            if (STEP_GRID > (int)(2*region))
            {
               STEP_GRID = (int)(2*region);
            }
            if (STEP_GRID > (int)(2*region))
            {
               STEP_GRID = (int)(2*region);
            }

         }
         if (wParam == 191)
         {
            STEP_GRID -= 50;
            hMov -= 25/10;
            wMov -= 25/10;
            ScaleMap += 0.5f;
            if (STEP_GRID < 64) STEP_GRID = 64;
         }
         if (wParam == 32)
         {
            xa=0, ya=0, za=0;
            hmova = wmova = 0;
         }
         if (wParam == 71)
         {
            glPolygonMode(GL_FRONT, GL_LINE);
         }
         if (wParam == 70)
         {
            glPolygonMode(GL_FRONT, GL_FILL);
         }

//----------------------------------------------------------------
			return 0;               // Jump back
		}

		case WM_KEYUP:                  // Has a key been released?
		{
			keys[wParam] = false;   // If so, mark it as FALSE
			return 0;               // Jump back
		}

		case WM_SIZE:                   // Resize the OpenGL window
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));  // LoWord = Width, HiWord = Height
			return 0;               // Jump back
		}
	}

	// Pass all unhandled messages to DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}


//---------------------------------------------------------------------------
#include <fstream.h>
#include <math.h>
#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
#include "matrc.h"
#include "keypoint.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
   : TForm(Owner)
{
   DoubleBuffered=true;

   StringGrid1->Cells[0][0] = "������";
   StringGrid1->Cells[0][1] = "�������";
   StringGrid1->Cells[0][2] = "��� ����� [�������]";
   StringGrid1->Cells[1][2] = 1.0;
   StringGrid1->Cells[0][3] = "������� [����/������]";
   StringGrid1->Cells[1][3] = 240.0;
   StringGrid1->Cells[0][4] = "����������";
   StringGrid1->Cells[1][4] = "EarthDATA/CASHE/";
   StringGrid1->Options = StringGrid1->Options << goEditing;
   AnsiString nPath = ExtractFilePath(Application->ExeName) + "EarthDATA\\CASHE\\coord.tmp";
   ifstream file(nPath.c_str());
   if(file)
   {
      double Lat, Lon, region, Scal;
      file >> Lat;
      file >> Lon;
      file >> region;
      file >> Scal;
      Edit1->Text = Lat;
      Edit2->Text = Lon;
      Edit3->Text = region;
      StringGrid1->Cells[1][3] = Scal;
   }
   file.close();
}
//-----------------------------------------------------------------------------
BYTE* ReadRAWpoint(String FileName, long int StLat, long int StLon, long int m, long int n, long int LatP, long int LonP)
{
   BYTE buf[1024];
   ifstream file(FileName.c_str(), ios::binary);
   long int location = 0;
   long int position;
   long int j,k;
   j = m - LatP + StLat*120;
   k = LonP - StLon*120;

   position = location + ((j*n) + k) * 1;
   file.seekg(position);
   file.read(buf, 1);
   file.close();
   return buf;
};
//-----------------------------------------------------------------------------
unsigned char* ReadBMPpoint(String FileName, double StLat, double StLon, int m, int n, double LatP, double LonP)
{
   unsigned char buf[1024];
   ifstream file(FileName.c_str(), ios::binary);
   long int location = 54;
   long int position;
   long int j,k;
   j = (LatP * 120 - StLat * 120);
   k = (LonP * 120 - StLon * 120);

   position = location + ((j*n) + k) * 3;
   file.seekg(position);
   file.read(buf, 3);
   file.close();
   return buf;
};

//-----------------------------------------------------------------------------
//class CashControl();
/*virtual*/  CashControl :: CreateCasheBMP(double Lat, double Lon, double region)
{
   String FileName = ExtractFilePath(Application->ExeName) + "EarthDATA\\BMPcolor\\a10g_24.bmp";
   ifstream file_a(FileName.c_str(), ios::binary);
   if(!file_a)
   {
      Application->MessageBoxA(FileName.c_str(), "������ ������ �����:");
      return 0;
   }
   AnsiString nPath = ExtractFilePath(Application->ExeName) + "EarthDATA\\CASHE\\cashmap.bmp";
   ofstream file2(nPath.c_str());
   if(!file2)
   {
      Application->MessageBoxA("��� ���� �� ����� ���� �����������", "������");
      return 0;
   }
   unsigned char *buf = new unsigned char[1024];
   double R = 2*region;
//********************************
   file_a.read(buf, 54);
   long int location = ((long int)(buf[10]))+((long int)(buf[11]) << 8)+((long int)(buf[12]) << 16)+((long int) (buf[13]) << 24);
   int n = ((long int)(buf[18])) + ((long int)(buf[19]) << 8) + ((long int)(buf[20]) << 16) + ((long int)(buf[21]) << 24);
   int m = ((long int)(buf[22]))+((long int)(buf[23]) << 8)+((long int)(buf[24]) << 16)+((long int) (buf[25]) << 24);
   long int bytes_bitmap_data = ((long int)(buf[34]))+((long int)(buf[35]) << 8)+((long int)(buf[36]) << 16)+((long int) (buf[37]) << 24);
   file_a.ignore((location - 54));
   long int j = (m - 1), k = 0, i = 0;

   buf[18] = (long int)R;
   buf[19] = ((long int)R)>>8;
   buf[20] = ((long int)R)>>16;
   buf[21] = ((long int)R)>>24;

   buf[22] = (long int)R;
   buf[23] = ((long int)R)>>8;
   buf[24] = ((long int)R)>>16;
   buf[25] = ((long int)R)>>24;
   file2.write(buf, 54);
//********************************

   double StartLat, StartLon;
   Lat = Lat - region/60.0;
   double SLon = Lon;
   double SLat = Lat;
   for(int j1=0; j1<(R); j1++)
   {
      {
         Lat = Lat + 1.0/120.0;
         Lon = SLon;
      }
      for(int k1 = 0; k1 < R; k1++)
      {
         if(k1>(10e-9))
         {
            Lon = Lon + 1.0/120.0;
         }

         if(Lat > 50)
         {
            StartLat = 50.0;
            if(Lon > 90)      //D
            {
               StartLon = 90.0;
               buf = ReadBMPpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\BMPcolor\\d10g_24.bmp", StartLat, StartLon, 4800, 10800, Lat, Lon);
               file2.write(buf, 3);
            }
            else if(Lon > 0)  //C
            {
               StartLon = 0.0;
               buf = ReadBMPpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\BMPcolor\\c10g_24.bmp", StartLat, StartLon, 4800, 10800, Lat, Lon);
               file2.write(buf, 3);
            }
            else if(Lon > -90)   //B
            {
               StartLon = -90.0;
               buf = ReadBMPpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\BMPcolor\\b10g_24.bmp", StartLat, StartLon, 4800, 10800, Lat, Lon);
               file2.write(buf, 3);
            }

            else                    //A
            {
               StartLon = -180.0;
               buf = ReadBMPpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\BMPcolor\\a10g_24.bmp", StartLat, StartLon, 4800, 10800, Lat, Lon);
               file2.write(buf, 3);
            }

         }
         else if(Lat > 0)
         {
            StartLat = 0.0;
            if(Lon > 90)               //H
            {
               StartLon = 90.0;
               buf = ReadBMPpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\BMPcolor\\h10g_24.bmp", StartLat, StartLon, 6000, 10800, Lat, Lon);
               file2.write(buf, 3);
            }
            else if(Lon > 0)  //G
            {
               StartLon = 0.0;
               buf = ReadBMPpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\BMPcolor\\g10g_24.bmp", StartLat, StartLon, 6000, 10800, Lat, Lon);
               file2.write(buf, 3);
            }
            else if(Lon > -90)   //F
            {
               StartLon = -90.0;
               buf = ReadBMPpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\BMPcolor\\f10g_24.bmp", StartLat, StartLon,6000, 10800, Lat, Lon);
               file2.write(buf, 3);
            }

            else      //E
            {
               StartLon = -180.0;
               buf = ReadBMPpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\BMPcolor\\e10g_24.bmp", StartLat, StartLon, 6000, 10800, Lat, Lon);
               file2.write(buf, 3);
            }
         }
         else if(Lat > -50)
         {
            StartLat = -50.0;
            if (Lon > 90)              //L
            {
               StartLon = 90.0;
               buf = ReadBMPpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\BMPcolor\\l10g_24.bmp", StartLat, StartLon, 6000, 10800, Lat, Lon);
               file2.write(buf, 3);
            }

            else if(Lon > 0)  //K
            {
               StartLon = 0.0;
               buf = ReadBMPpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\BMPcolor\\k10g_24.bmp", StartLat, StartLon, 6000, 10800, Lat, Lon);
               file2.write(buf, 3);
            }
            else if(Lon > -90)   //J
            {
               StartLon = -90.0;
               buf = ReadBMPpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\BMPcolor\\j10g_24.bmp", StartLat, StartLon, 6000, 10800, Lat, Lon);
               file2.write(buf, 3);
            }
            else      //I
            {
               StartLon = -180.0;
               buf = ReadBMPpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\BMPcolor\\i10g_24.bmp", StartLat, StartLon, 6000, 10800, Lat, Lon);
               file2.write(buf, 3);
            }
         }
         else
         {
            StartLat = -90.0;
            if(Lon > 90)               //P
            {
               StartLon = 90.0;
               buf = ReadBMPpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\BMPcolor\\p10g_24.bmp", StartLat, StartLon, 4800, 10800, Lat, Lon);
               file2.write(buf, 3);
            }

            else if(Lon > 0)  //O
            {
               StartLon = 0.0;
               buf = ReadBMPpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\BMPcolor\\o10g_24.bmp", StartLat, StartLon, 4800, 10800, Lat, Lon);
               file2.write(buf, 3);
            }
            else if(Lon > -90)   //N
            {
               StartLon = -90.0;
               buf = ReadBMPpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\BMPcolor\\n10g_24.bmp", StartLat, StartLon, 4800, 10800, Lat, Lon);
               file2.write(buf, 3);
            }
            else      //M
            {
               StartLon = -180.0;
               buf = ReadBMPpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\BMPcolor\\m10g_24.bmp", StartLat, StartLon, 4800, 10800, Lat, Lon);
               file2.write(buf, 3);
            }
         }
      }
   }
   delete buf;
   file2.close();
   return true;
};
//---------------------------------------------------------------------------
/*virtual*/ CashControl :: CreateCasheTerrain(double Lat, double Lon, double region)
{
 AnsiString nPath = ExtractFilePath(Application->ExeName) + "EarthDATA\\CASHE\\cashmap.raw";
 ofstream file2(nPath.c_str());
 if(!file2)
 {
    Application->MessageBoxA("��� ���� �� ����� ���� �����������", "������");
    return 0;
 }
 else
 {
   unsigned char	*buf = new unsigned char[1024];
   double StartLat, StartLon;
   double SLat = Lat;
   double SLon = Lon;
   int imax = 2*region;
   long int LatT, LonT;
   LatT = Lat * 120;
   LonT = Lon * 120;
   long int poscashe = 0;

   for(long int j1=0; j1 < imax; j1++)
   {
      Lat = Lat - 1.0/120.0;
      Lon = SLon;
      LatT --;
      if(LatT == 6000)
      {
         int a;
         a=0;
         Lat = 50.0;
      }
      else{}
      LonT = (int)(SLon * 120.0);

      for(int k1 = 0; k1 < imax; k1++)
      {
         LonT++;
         Lon = Lon + 1.0/120.0;
         if(Lat > 50)
         {
            StartLat = 50.0;
            if(Lon > 90)      //D
            {
               StartLon = 90.0;
               buf = ReadRAWpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\Raw\\d10g_8bit.raw", StartLat, StartLon, 4800, 10800, LatT, LonT);
               file2.seekp(poscashe);
               file2.write(buf, 1);
               poscashe ++;
            }
            else if(Lon > 0)  //C
            {
               StartLon = 0.0;
               buf = ReadRAWpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\Raw\\c10g_8bit.raw", StartLat, StartLon, 4800, 10800, LatT, LonT);
               file2.seekp(poscashe);
               file2.write(buf, 1);
               poscashe ++;
            }
            else if(Lon > -90)   //B
            {
               StartLon = -90.0;
               buf = ReadRAWpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\Raw\\b10g_8bit.raw", StartLat, StartLon, 4800, 10800, LatT, LonT);
               file2.seekp(poscashe);
               file2.write(buf, 1);
               poscashe ++;
            }
            else                    //A
            {
               StartLon = -180.0;
               buf = ReadRAWpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\Raw\\a10g_8bit.raw", StartLat, StartLon, 4800, 10800, LatT, LonT);
               file2.seekp(poscashe);
               file2.write(buf, 1);
               poscashe ++;
            }
         }
         else if(Lat > 0)
         {
            StartLat = 0.0;
            if(Lon > 90)               //H
            {
               StartLon = 90.0;
               buf = ReadRAWpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\Raw\\h10g_8bit.raw", StartLat, StartLon, 6000, 10800, LatT, LonT);
               file2.seekp(poscashe);
               file2.write(buf, 1);
               poscashe ++;
            }
            else if(Lon > 0)  //G
            {
               StartLon = 0.0;
               buf = ReadRAWpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\Raw\\g10g_8bit.raw", StartLat, StartLon, 6000, 10800, LatT, LonT);
               file2.seekp(poscashe);
               file2.write(buf, 1);
               poscashe ++;
            }
            else if(Lon > -90)   //F
            {
               StartLon = -90.0;
               buf = ReadRAWpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\Raw\\f10g_8bit.raw", StartLat, StartLon,6000, 10800, LatT, LonT);
               file2.seekp(poscashe);
               file2.write(buf, 1);
               poscashe ++;
            }

            else      //E
            {
               StartLon = -180.0;
               buf = ReadRAWpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\Raw\\e10g_8bit.raw", StartLat, StartLon, 6000, 10800, LatT, LonT);
               file2.seekp(poscashe);
               file2.write(buf, 1);
               poscashe ++;
            }
         }
         else if(Lat > -50)
         {
            StartLat = -50.0;
            if (Lon > 90)              //L
            {
               StartLon = 90.0;
               buf = ReadRAWpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\Raw\\l10g_8bit.raw", StartLat, StartLon, 6000, 10800, LatT, LonT);
               file2.seekp(poscashe);
               file2.write(buf, 1);
               poscashe ++;
            }

            else if(Lon > 0)  //K
            {
               StartLon = 0.0;
               buf = ReadRAWpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\Raw\\k10g_8bit.raw", StartLat, StartLon, 6000, 10800, LatT, LonT);
               file2.seekp(poscashe);
               file2.write(buf, 1);
               poscashe ++;
            }
            else if(Lon > -90)   //J
            {
               StartLon = -90.0;
               buf = ReadRAWpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\Raw\\j10g_8bit.raw", StartLat, StartLon, 6000, 10800, LatT, LonT);
               file2.seekp(poscashe);
               file2.write(buf, 1);
               poscashe ++;
            }
            else      //I
            {
               StartLon = -180.0;
               buf = ReadRAWpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\Raw\\i10g_8bit.raw", StartLat, StartLon, 6000, 10800, LatT, LonT);
               file2.seekp(poscashe);
               file2.write(buf, 1);
               poscashe ++;
            }
         }
         else
         {
            StartLat = -90.0;
            if(Lon > 90)               //P
            {
               StartLon = 90.0;
               buf = ReadRAWpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\Raw\\p10g_8bit.raw", StartLat, StartLon, 4800, 10800, LatT, LonT);
               file2.seekp(poscashe);
               file2.write(buf, 1);
               poscashe ++;
            }

            else if(Lon > 0)  //O
            {
               StartLon = 0.0;
               buf = ReadRAWpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\Raw\\o10g_8bit.raw", StartLat, StartLon, 4800, 10800, LatT, LonT);
               file2.seekp(poscashe);
               file2.write(buf, 1);
               poscashe ++;
            }
            else if(Lon > -90)   //N
            {
               StartLon = -90.0;
               buf = ReadRAWpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\Raw\\n10g_8bit.raw", StartLat, StartLon, 4800, 10800, LatT, LonT);
               file2.seekp(poscashe);
               file2.write(buf, 1);
               poscashe ++;
            }
            else      //M
            {
               StartLon = -180.0;
               buf = ReadRAWpoint(ExtractFilePath(Application->ExeName) + "EarthDATA\\Raw\\m10g_8bit.raw", StartLat, StartLon, 4800, 10800, LatT, LonT);
               file2.seekp(poscashe);
               file2.write(buf, 1);
               poscashe ++;
            }
         }
      }
   }
   delete buf;
   file2.close();
   return true;
 }
};
//---------------------------------------------------------------------------
//double Lat, Lon, region;
CashControl A;
void __fastcall TForm1::Button1Click(TObject *Sender)
{
 fEdit = true;
 if(Application->MessageBoxA("��������, ��� ����� ������, � ������ ��� ��������� ������ �������� ����, ��������� �� �����. ��� ������ ��������� �����. ����������?", "�������� ������ �������", MB_YESNO)==IDYES)
 {
    Form1->WindowState = wsMinimized;
    region = Edit3->Text.ToDouble();
    Lat = Edit1->Text.ToDouble();
    Lon = Edit2->Text.ToDouble();

    String FileName;

    Lat =  Lat - ((int)(Lat*12000)%100/100.0)/120.0;
    Lon =  Lon - ((int)(Lon*12000)%100/100.0)/120.0;

    Edit1->Text = Lat;
    Edit2->Text = Lon;
    Edit3->Text = region;
    if(CheckBox1->Checked){A.CreateCasheBMP(Lat, Lon, region);}
    Edit1->Text = Lat;
    Edit2->Text = Lon;
    Edit3->Text = region;
    if(CheckBox2->Checked){A.CreateCasheTerrain(Lat, Lon, region);}
    Form1->WindowState = wsMaximized;
    Application->MessageBoxA("��������� ���������", "���������");
    String nPach = ExtractFilePath(Application->ExeName);
    nPach += "EarthDATA\\CASHE\\coord.tmp";
    ofstream file(nPach.c_str());
    if(file)
    {
       Lat = Edit1->Text.ToDouble();
       Lon = Edit2->Text.ToDouble();
       region = Edit3->Text.ToDouble();
       double Scal = StringGrid1->Cells[1][3].ToDouble();
       file << Lat << " ";
       file << Lon << " ";
       file << region << " ";
       file << Scal << " ";
    }
    file.close();
 }
 else
 {}
};
//---------------------------------------------------------------------------

void __fastcall TForm1::Image1MouseMove(TObject *Sender, TShiftState Shift,
      int X, int Y)
{
   if(!Shift.Empty())
   {
      x1 += (float)(Y - Ya);
      y1 += (float)(X - Xa);
      if (x1 < (- picture->Width+Image1->Height)){ x1 = (- picture->Width+Image1->Height);}
      if (y1 < (- picture->Height+Image1->Width)){ y1 = (- picture->Height+Image1->Width);}
      if (x1 > 0) {x1 = 0;}
      if (y1 > 0) {y1 = 0;}
      if(abs(Y-Ya) > 1 || abs(X-Xa) > 1)
      {
         Image1->Canvas->Draw(y1, x1, picture);
      }
   }
   Ya=Y;
   Xa=X;
   if((StringGrid1->Cells[1][3] != "") && (Edit1->Text != "") && (Edit2->Text != ""))
   {
      StringGrid1->Cells[1][0] = -Ya*StringGrid1->Cells[1][2].ToDouble()/StringGrid1->Cells[1][3].ToDouble() + Edit1->Text.ToDouble();
      StringGrid1->Cells[1][1] = Xa*StringGrid1->Cells[1][2].ToDouble()/StringGrid1->Cells[1][3].ToDouble() + Edit2->Text.ToDouble();
   }
}
//---------------------------------------------------------------------------
 
void __fastcall TForm1::Button2Click(TObject *Sender)
{
   MSG msg;                // Windows message structure
	bool done = false;      // Bool variable to exit loop
   fullscreen = false;

	// Create our OpenGL window
	if (!CreateGLWindow("3D Map",800,600,16,fullscreen))
	{
      Application->Destroying();
		//return 0;               // Quit if window was not created
   }
	while(!done)            // Loop that runs while done = FALSE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is there a message waiting?
		{
			if (msg.message == WM_QUIT)             // Have we received a quit message?
			{
				done = true;                    // If so done = TRUE
			}
			else                                    // If not, deal with window messages
			{
				TranslateMessage(&msg);         // Translate the message
				DispatchMessage(&msg);          // Dispatch the message
			}
		}
		else            // If there are no messages
		{
			// Draw the scene.  Watch for ESC key and quit messages from DrawGLScene()
			if (active)                             // Program active?
			{
				if (keys[VK_ESCAPE])            // Was ESC pressed?
				{
					done = true;            // ESC signalled a quit
				}
				else                            // Not time to quit, Update screen
				{
					DrawGLScene();          // Draw the scene
					SwapBuffers(hDC);       // Swap buffers (Double buffering)
				}
			}

			if (keys[VK_F1])                        // Is F1 being pressed?
			{
				keys[VK_F1] = false;            // If so make key FALSE
				KillGLWindow();                 // Kill our current window
				fullscreen =! fullscreen;       // Toggle fullscreen / windowed mode
				// Recreate our OpenGL window
				if (!CreateGLWindow("3D Map",800,600,16,fullscreen))
				{
               Application->Destroying();
					//return 0;               // Quit if window was not created
				}
			}
		}
	}

	// Shutdown
	KillGLWindow();         // Kill the window
	//return (msg.wParam);    // Exit the program


}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit1KeyPress(TObject *Sender, char &Key)
{
   fEdit = true;
  	Set <char, '0', '9'> Dig;
	Dig  << '0' << '1' << '2' << '3' << '4' << '5' << '6' <<'7'<<'8'<< '9';
   int a;
   a = Key;
   if(Key == 8 || Key == 0x2c || Key == 45)
		;
	else if(Key == 0x2e)
		Key = 0x2c;
	else if ( !Dig.Contains(Key) )
	{
		Key = 0;
		Beep();
	}
   if(Edit1->Text == ""){Edit1->Text = "0";}
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Edit2KeyPress(TObject *Sender, char &Key)
{
   fEdit = true;
  	Set <char, '0', '9'> Dig;
	Dig << '0' << '1' << '2' << '3' << '4' << '5' << '6' <<'7'<<'8'<< '9';
   if(Key == 8 || Key == 0x2c || Key == 45)
		;
	else if(Key == 0x2e)
		Key = 0x2c;
	else if ( !Dig.Contains(Key) )
	{
		Key = 0;
		Beep();
	}

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit3KeyPress(TObject *Sender, char &Key)
{
   fEdit = true;
  	Set <char, '0', '9'> Dig;
	Dig << '0' << '1' << '2' << '3' << '4' << '5' << '6' <<'7'<<'8'<< '9';
   if(Key == 8 || Key == 0x2c)
		;
	else if(Key == 0x2e)
		Key = 0x2c;
	else if ( !Dig.Contains(Key) )
	{
		Key = 0;
		Beep();
	}

}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormCanResize(TObject *Sender, int &NewWidth,
      int &NewHeight, bool &Resize)
{
   Image1->Stretch = false;
   Image1->Width = (Form1->Width - 400);
   Image1->Height = (Form1->Height - 164);
   Image1->Destroying();
   AnsiString nPath = ExtractFilePath(Application->ExeName);
   nPath += "EarthDATA\\CASHE\\cashmap.bmp";
   ifstream test(nPath.c_str());
   if(test)
   {
      picture->LoadFromFile(nPath);
      Image1->Canvas->Draw(0, 0, picture);
   }
   test.close();
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Button6Click(TObject *Sender)
{
   AnsiString nPath = ExtractFilePath(Application->ExeName);
      nPath += "EarthDATA\\CASHE\\coord.tmp";
   ofstream file(nPath.c_str());
   if(file)
   {
      Lat = Edit1->Text.ToDouble();
      Lon = Edit2->Text.ToDouble();
      region = Edit3->Text.ToDouble();
      double Scal = StringGrid1->Cells[1][3].ToDouble();
      file << Lat << " ";
      file << Lon << " ";
      file << region << " ";
      file << Scal << " ";
   }
   file.close();

   if (Form1->SaveDialog1->Execute())
   {
      fEdit = false;
      int x=0;
      int D;
      TMemoryStream * T = new TMemoryStream;
      TSearchRec sr;
      AnsiString nPath = ExtractFilePath(Application->ExeName);
      nPath += "EarthDATA\\CASHE\\";
      String nPath2 = ExtractFilePath(SaveDialog1->FileName);

      bool fRecord = true;
      if (FindFirst(nPath2 + "\\cashmap.bmp", faAnyFile, sr) == 0)
      {
         fRecord = false;
         String text;
         text = "���� cashmap.bmp ��� ���� �" + nPath2 + ". ��������?","����������";
         if(Application->MessageBoxA(text.c_str(), "������ �����", MB_YESNO)==IDYES)
         {
            fRecord == true;
         }
      }
      if(fRecord == true)
      {
         D = FindFirst(nPath + "\\cashmap.bmp", faAnyFile, sr);
         while (!D)
         {
            if (sr.Name != "." && sr.Name != ".." )
            {
               T->LoadFromFile(nPath + "\\" + sr.Name);
               T->SaveToFile(nPath2 + "\\" + sr.Name);
            }
            x++;
            D=FindNext(sr);
         }
      }
      if (FindFirst(nPath2 + "\\cashmap.raw", faAnyFile, sr) == 0)
      {
         fRecord = false;
         String text;
         text = "���� cashmap.raw ��� ���� �" + nPath2 + ". ��������?","����������";
         if(Application->MessageBoxA(text.c_str(), "������ �����", MB_YESNO)==IDYES)
         {
            fRecord == true;
         }
      }
      if(fRecord == true)
      {
         D = FindFirst(nPath + "\\cashmap.raw", faAnyFile, sr);
         while (!D)
         {
            if (sr.Name != "." && sr.Name != ".." )
            {
               T->LoadFromFile(nPath + "\\" + sr.Name);
               T->SaveToFile(nPath2 + "\\" + sr.Name);
            }
            x++;
            D=FindNext(sr);
         }
      }
      if (FindFirst(nPath2 + "\\coord.tmp", faAnyFile, sr) == 0)
      {
         fRecord = false;
         String text;
         text = "���� coord.tmp ��� ���� �" + nPath2 + ". ��������?","����������";
         if(Application->MessageBoxA(text.c_str(), "������ �����", MB_YESNO)==IDYES)
         {
            fRecord == true;
         }
      }
      if(fRecord == true)
      {
         D = FindFirst(nPath + "\\coord.tmp", faAnyFile, sr);
         while (!D)
         {
            if (sr.Name != "." && sr.Name != ".." )
            {
               T->LoadFromFile(nPath + "\\" + sr.Name);
               T->SaveToFile(nPath2 + "\\" + sr.Name);
            }
            x++;
            D=FindNext(sr);
         }
      }
      FindClose(sr);
      delete T;
      Image1->Stretch = false;
      Image1->Width = (Form1->Width - 400);
      Image1->Height = (Form1->Height - 164);
      Image1->Destroying();
      picture->LoadFromFile(ExtractFilePath(nPath) + "cashmap.bmp");
      Image1->Canvas->Draw(0, 0, picture);

   }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button7Click(TObject *Sender)
{
   fEdit = true;
   //---------------
   if (Form1->OpenDialog1->Execute())
   {
      picture->FreeImage();
      
      int x=0;
      int D;
      TMemoryStream * T = new TMemoryStream;
      TSearchRec sr;
      AnsiString nPath = ExtractFilePath(Application->ExeName);
      nPath += "EarthDATA\\CASHE\\";
      String nPath2 = ExtractFilePath(OpenDialog1->FileName);
      D = FindFirst(nPath2 + "\\cashmap.bmp", faAnyFile, sr);
      while (!D)
      {
         if (sr.Name != "." && sr.Name != ".." )
         {
            T->LoadFromFile(nPath2 + "\\" + sr.Name);
            T->SaveToFile(nPath + "\\" + sr.Name);
         }
         x++;
         D=FindNext(sr);
      }
      picture->LoadFromFile(ExtractFilePath(nPath) + "cashmap.bmp");
      D = FindFirst(nPath2 + "\\cashmap.raw", faAnyFile, sr);
      while (!D)
      {
         if (sr.Name != "." && sr.Name != ".." )
         {
            T->LoadFromFile(nPath2 + "\\" + sr.Name);
            T->SaveToFile(nPath + "\\" + sr.Name);
         }
         x++;
         D=FindNext(sr);
      }
      D = FindFirst(nPath2 + "\\coord.tmp", faAnyFile, sr);
      while (!D)
      {
         if (sr.Name != "." && sr.Name != ".." )
         {
            T->LoadFromFile(nPath2 + "\\" + sr.Name);
            T->SaveToFile(nPath + "\\" + sr.Name);
         }
         x++;
         D=FindNext(sr);
      }
      FindClose(sr);
      delete T;
   }
   else{}
   AnsiString nPath = ExtractFilePath(Application->ExeName);
      nPath += "EarthDATA\\CASHE\\coord.tmp" ;
   ifstream file(nPath.c_str());
   if(file)
   {
      double Lat, Lon, region, Scal;
      file >> Lat;
      file >> Lon;
      file >> region;
      file >> Scal;
      Edit1->Text = Lat;
      Edit2->Text = Lon;
      Edit3->Text = region;
      StringGrid1->Cells[1][3] = Scal;
   }
   file.close();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button11Click(TObject *Sender)
{
   TRect TheRect;
   double StepScale;
   StepScale = StringGrid1->Cells[1][2].ToDouble()*StringGrid1->Cells[1][3].ToDouble();
   int dLat, dLon;
   String Coord;
   dLat = (Edit1->Text.ToDouble()-(int)Edit1->Text.ToDouble())*StringGrid1->Cells[1][3].ToDouble()/StringGrid1->Cells[1][2].ToDouble();
   dLon = (Edit2->Text.ToDouble()-(int)Edit2->Text.ToDouble())*StringGrid1->Cells[1][3].ToDouble()/StringGrid1->Cells[1][2].ToDouble();
   for (int i=0; i<25; i++)
   {
      Image1->Canvas->Pen->Color=clGreen;
      Image1->Canvas->MoveTo(y1 + i*StepScale - dLon, 0);
      Image1->Canvas->LineTo(y1 + i*StepScale - dLon, Image1->Height);
      Image1->Canvas->MoveTo(0, x1 + i*StepScale + dLat);
      Image1->Canvas->LineTo(Image1->Width, x1 + i*StepScale + dLat);
      for(int i2=0; i2<25; i2++)
      {
         Coord = int(-(i2-1) + (x1 - dLat)*StringGrid1->Cells[1][2].ToDouble()/StringGrid1->Cells[1][3].ToDouble() + Edit1->Text.ToDouble());
         Coord += "; ";
         Coord += int((i-1)+(y1 - dLon)*StringGrid1->Cells[1][2].ToDouble()/StringGrid1->Cells[1][3].ToDouble() + Edit2->Text.ToDouble());
         TheRect = Rect(y1 + (i-1)*StepScale - dLon + 10, x1 + (i2-1)*StepScale + dLat + 10,y1 + (i-1)*StepScale - dLon + 60, x1 + (i2-1)*StepScale + dLat + 25);
         Image1->Canvas->TextRect(TheRect, y1 + (i-1)*StepScale - dLon + 10, x1 + (i2-1)*StepScale + dLat +10, Coord);
      }
   }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button8Click(TObject *Sender)
{
   USEFORM("draw.cpp", OpenGL_Form);
   Application->Destroying();
   try
   {
       Application->Initialize();
       Application->CreateForm(__classid(TOpenGL_Form), &OpenGL_Form);
       Application->Run();
   }
   catch (Exception &exception)
   {
       Application->ShowException(&exception);
   }
   catch (...)
   {
       try
       {
          throw Exception("");
       }
       catch (Exception &exception)
       {
          Application->ShowException(&exception);
       }
   }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::StringGrid1KeyPress(TObject *Sender, char &Key)
{
   fEdit = true;
  	Set <char, '0', '9'> Dig;
	Dig << '0' << '1' << '2' << '3' << '4' << '5' << '6' <<'7'<<'8'<< '9';

   if(Key == 8 || Key == 0x2c || Key == 13) //����� ��� �������
		;
	else if(Key == 0x2e) //�����
		Key = 0x2c;
	else if ( !Dig.Contains(Key) )
	{
		Key = 0;
		Beep();
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button6Exit(TObject *Sender)
{
   AnsiString nPath = ExtractFilePath(Application->ExeName);
      nPath += "EarthDATA\\CASHE\\coord.tmp";
   ofstream file(nPath.c_str());
   if(file)
   {
      Lat = Edit1->Text.ToDouble();
      Lon = Edit2->Text.ToDouble();
      region = Edit3->Text.ToDouble();
      double Scal = StringGrid1->Cells[1][3].ToDouble();
      file << Lat << " ";
      file << Lon << " ";
      file << region << " ";
      file << Scal << " ";
   }
   file.close();

   if(fEdit){Form1->Button6Click(Owner);}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
   AnsiString nPath = ExtractFilePath(Application->ExeName);
      nPath += "EarthDATA\\CASHE\\coord.tmp";
   ofstream file(nPath.c_str());
   if(file)
   {
      Lat = Edit1->Text.ToDouble();
      Lon = Edit2->Text.ToDouble();
      region = Edit3->Text.ToDouble();
      double Scal = StringGrid1->Cells[1][3].ToDouble();
      file << Lat << " ";
      file << Lon << " ";
      file << region << " ";
      file << Scal << " ";
   }
   file.close();

   if(fEdit){Form1->Button6Click(Owner);}
}






















