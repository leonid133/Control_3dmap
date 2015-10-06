//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "draw.h"

//---------------------------------------------------------------------------
#pragma resource "*.dfm"
TOpenGL_Form *OpenGL_Form;
//---------------------------------------------------------------------------
__fastcall TOpenGL_Form::TOpenGL_Form(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TOpenGL_Form::Timer1Timer(TObject *Sender)
{
	Timer1->Enabled = false;
    DrawScene();
    Yield();
    Timer1->Enabled = true;
}

BOOL bSetupPixelFormat(HDC hdc)
{
    PIXELFORMATDESCRIPTOR pfd, *ppfd;
    int pixelformat;

    ppfd = &pfd;

    ppfd->nSize = sizeof(PIXELFORMATDESCRIPTOR);
    ppfd->nVersion = 1;
    ppfd->dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
                        PFD_DOUBLEBUFFER;
    ppfd->dwLayerMask = PFD_MAIN_PLANE;
    ppfd->iPixelType = PFD_TYPE_COLORINDEX;
    ppfd->cColorBits = 8;
    ppfd->cDepthBits = 16;

    ppfd->cAccumBits = 0;
    ppfd->cStencilBits = 0;

    if ( (pixelformat = ChoosePixelFormat(hdc, ppfd)) == 0 )
    {
        MessageBox(NULL, "ChoosePixelFormat failed", "Error", MB_OK);
        return FALSE;
    }

    if (SetPixelFormat(hdc, pixelformat, ppfd) == FALSE)
    {
        MessageBox(NULL, "SetPixelFormat failed", "Error", MB_OK);
        return FALSE;
    }

    return TRUE;

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

int TOpenGL_Form::LoadGLTextures()
{
	int Status = false;
	AUX_RGBImageRec *TextureImage[2];
	memset(TextureImage,0,sizeof(void *)*1);
	if ((TextureImage[0] = LoadBMP("Earth/Data/Map.bmp")) && (TextureImage[1] = LoadBMP("Earth/Data/Map2.bmp")))
	{
		Status = true;
		glGenTextures(1, &texName);
		glBindTexture(GL_TEXTURE_2D, texName);
		glTexImage2D(GL_TEXTURE_2D, 1, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
      glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[1]->sizeX, TextureImage[1]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[1]->data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	}
 	if (TextureImage[0])
	{
		if (TextureImage[0]->data)
			free(TextureImage[0]->data);
		free(TextureImage[0]);
	}

	return Status;
}

void	TOpenGL_Form::DrawScene()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
 
    glPushMatrix();
 
    latitude += latinc;
    longitude += longinc;
    twist += twistinc;

    polarView( radius, twist, latitude, longitude );
    
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindTexture(GL_TEXTURE_2D, texName);

    glCallList(GLOBE);
    //Освещение
    if(flagLight == true)
    {
       glEnable(GL_LIGHTING);
       glEnable(GL_LIGHT0);
       glEnable(GL_DEPTH_TEST);
    }
    else
    {
       glDisable(GL_LIGHTING);
       glDisable(GL_LIGHT0);
       glDisable(GL_DEPTH_TEST);
    }
    //Туман
    if(flagFog == true)
    {
       glEnable(GL_FOG);
    }
    else
    {
       glDisable(GL_FOG);
    }
    //сеточка
    if(flagCells == true)
    {
       glDisable(GL_POLYGON_OFFSET_FILL);
       glColor3f(1.0, 1.0, 1.0);
       glDisable(GL_LIGHTING);
       glDisable(GL_LIGHT0);
       //glDisable(GL_DEPTH_TEST);
       glDisable(GL_DEPTH_TEST);

       glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
       glCallList(GLOBE);
    }
    glPopMatrix();
    SwapBuffers(ghDC);
}

GLvoid TOpenGL_Form::resize( GLsizei width, GLsizei height )
{
    GLfloat aspect;

    glViewport( 0, 0, width, height );

    aspect = (GLfloat) width / height;

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    gluPerspective(1.0, aspect, 1.0, 1000.0);

    glMatrixMode( GL_MODELVIEW );
}

GLvoid TOpenGL_Form::createObjects()
{
  GLUquadricObj *quadObj;
    glNewList(GLOBE, GL_COMPILE);
        quadObj = gluNewQuadric ();
        gluQuadricTexture(quadObj, GL_TRUE);
        gluQuadricDrawStyle (quadObj, GLU_FILL);
        gluSphere (quadObj, 1.0, 180, 180);
    glEndList();
}

GLvoid TOpenGL_Form::initializeGL(GLsizei width, GLsizei height)
{

    if(!LoadGLTextures())Application->MessageBoxA("Data\map.bmp","Error load", true);

    GLfloat     maxObjectSize, aspect;
    GLdouble    near_plane;
    glShadeModel(GL_FLAT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);


    glEnable(GL_TEXTURE_2D);
    glEnable(GL_SMOOTH);
    glDepthFunc(GL_LEQUAL);
	 glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glPolygonMode(GL_FRONT, GL_TRIANGLE_STRIP);	


    glMatrixMode( GL_PROJECTION );
    aspect = (GLfloat) width / height;

    glMatrixMode( GL_MODELVIEW );

    near_plane = 150.0;
    maxObjectSize = 4.0F;
    radius = near_plane + maxObjectSize/2.0;

    latitude = 35.0F;
    longitude = 0.0F;
    twist = 6.0F;
    latinc = 0.0F;
    longinc = 0.0F;
    twistinc = 0.0F;

    //Свет
       GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
       GLfloat mat_shininess[] = {50.0};
       GLfloat light_position[] = {100.0, 100.0, 100.0, 1.0};
       GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};

       glClearColor(0.0, 0.0, 0.0, 1.0);
       glShadeModel(GL_SMOOTH);
       glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
       glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
       glLightfv(GL_LIGHT0, GL_POSITION, light_position);
       glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
       glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);

       glEnable(GL_LIGHTING);
       glEnable(GL_LIGHT0);
       glEnable(GL_DEPTH_TEST);
       flagLight = true;

    // Туман
       GLint fogMode;
       glEnable(GL_FOG);
       {
         GLfloat fogColor[4] = {1.0, 1.0, 1.0, 0.1};
         fogMode = GL_EXP;
         glFogi(GL_FOG_MODE, fogMode);
         glFogfv(GL_FOG_COLOR, fogColor);
         glFogf(GL_FOG_DENSITY, 0.003);
         glHint(GL_FOG_HINT, GL_NICEST);
         glFogf(GL_FOG_START, 1.0);
         glFogf(GL_FOG_END, 5.0);
       }
    createObjects();
}

void TOpenGL_Form::polarView(GLdouble radius, GLdouble twist, GLdouble latitude,
           			  GLdouble longitude)
{
    glTranslated(0.0, 0.0, -radius);
    glRotated(-twist, 0.0, 1.0, 0.0);
    glRotated(-latitude, 1.0, 0.0, 0.0);
    glRotated(longitude, 0.0, 0.0, 1.0);
    
}


//---------------------------------------------------------------------------
void __fastcall TOpenGL_Form::FormCreate(TObject *Sender)
{
        ghDC = GetDC(Handle);
        if (!bSetupPixelFormat(ghDC))
            Close();                                      
        ghRC = wglCreateContext(ghDC);
        wglMakeCurrent(ghDC, ghRC);
        initializeGL(ClientRect.Right, ClientRect.Bottom);
}
//---------------------------------------------------------------------------
void __fastcall TOpenGL_Form::FormResize(TObject *Sender)
{
	resize(ClientRect.Right, ClientRect.Bottom);
}
//---------------------------------------------------------------------------
void __fastcall TOpenGL_Form::FormClose(TObject *Sender, TCloseAction &Action)
{
	    Timer1->Enabled = false;
        if (ghRC)
            wglDeleteContext(ghRC);
        if	(ghDC)
        	ReleaseDC(Handle, ghDC);
}
//---------------------------------------------------------------------------
void __fastcall TOpenGL_Form::FormKeyDown(TObject *Sender, WORD &Key,
	TShiftState Shift)
{
   int aa;
   aa = Key;

   if(Key==107)
      radius-=1.0;
   if(Key==109)
      radius+=1.0;
   if(Key==37)
      latinc += 0.01F;
   if(Key==39)
      latinc -= 0.01F;
   if(Key==38)
      longinc += 0.01F;
   if(Key==40)
      longinc -= 0.01F;

   //space
   if(Key==32)
   {
      latinc = 0.0F;
      longinc = 0.0F;
      twistinc = 0.0F;
   }

   //c
   if(Key==67)
      flagCells = !flagCells;

   //f
   if(Key==70)
      flagFog = !flagFog;

   //l
   if(Key==76)
      flagLight = !flagLight;

   //Esc
   if(Key==27)
	   Close();
}

//---------------------------------------------------------------------------

void __fastcall TOpenGL_Form::FormMouseWheelUp(TObject *Sender,
      TShiftState Shift, TPoint &MousePos, bool &Handled)
{
   radius-=1.0;
}
//---------------------------------------------------------------------------

void __fastcall TOpenGL_Form::FormMouseWheelDown(TObject *Sender,
      TShiftState Shift, TPoint &MousePos, bool &Handled)
{
   radius+=1.0;
}
//---------------------------------------------------------------------------


void __fastcall TOpenGL_Form::FormMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
    if(!Shift.Empty())
    {
      latitude -= (float)(Y - Yaccsel)*0.1;
      longitude += (float)(X - Xaccsel)*0.1;
    }
    Xaccsel = X;
    Yaccsel = Y;
}
//---------------------------------------------------------------------------

