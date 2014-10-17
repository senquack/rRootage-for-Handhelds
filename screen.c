/*
 * $Id: screen.c,v 1.6 2003/08/10 03:21:28 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * OpenGL screen handler.
 *
 * @version $Revision: 1.6 $
 */
#include <stdio.h>
#include <stdlib.h>

//#include "SDL.h"
//#include "SDL_mixer.h"
//#include "SDL_image.h"


//senquack
//#include "minimal.h"

#include <math.h>
#include <string.h>

#include "genmcr.h"
#include "screen.h"
#include "rr.h"
#include "degutil.h"
#include "attractmanager.h"
#include "letterrender.h"
#include "boss_mtd.h"

////senquack - added for gp2x volume control:
#include "soundmanager.h"

#define FAR_PLANE 720
//senquack - fixed point version:
#define FAR_PLANE_X 47185920

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define LOWRES_SCREEN_WIDTH 320
#define LOWRES_SCREEN_HEIGHT 240
#define SHARE_LOC "rr_share/"

////senquack - added this so we can support a rotated, zoomed screen
//int screenRotated = 0;

//senquack - temporary:
typedef struct
{
   int count;
   GLfixed size;
} shape;

shape shapes[7][50];

static int screenWidth, screenHeight;

//senquack - for wiz OpenGLES
EGLDisplay glDisplay;
EGLConfig glConfig;
EGLContext glContext;
EGLSurface glSurface;
NativeWindowType hNativeWnd = 0;

EGLint attrib_list_fsaa[] = {
   EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
   EGL_BUFFER_SIZE, 0,
   EGL_DEPTH_SIZE, 16,
   EGL_SAMPLE_BUFFERS, 1,
   EGL_SAMPLES, 4,
   EGL_NONE
};

EGLint attrib_list[] = {
   EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
   EGL_BUFFER_SIZE, 0,
   EGL_DEPTH_SIZE, 16,
   EGL_NONE
};

//senquack - experiment 2/12 (none of this appears linked to problem)
// Reset viewport when the screen is resized.
//static void screenResized() {
//  glViewport(0, 0, screenWidth, screenHeight);
//  glMatrixMode(GL_PROJECTION);
//  glLoadIdentity();
//  gluPerspective(45.0f, (GLfloat)screenWidth/(GLfloat)screenHeight, 0.1f, FAR_PLANE);
//  glMatrixMode(GL_MODELVIEW);
//}
//static void screenResized() {
// //senquack - do we really need to call glViewport every single screen refresh!?
//  glViewport(0, 0, screenWidth, screenHeight);
//  glMatrixMode(GL_PROJECTION);
//  glLoadIdentity();
//  gluPerspective(45.0f, (GLfloat)screenWidth/(GLfloat)screenHeight, 0.1f, FAR_PLANE);
//  glMatrixMode(GL_MODELVIEW);
//}
//senquack - fixed point for wiz openGLES
//static void screenResized() {
// //senquack - do we really need to call glViewport every single screen refresh!?
//  glViewport(0, 0, screenWidth, screenHeight);
//  glMatrixMode(GL_PROJECTION);
//  glLoadIdentity();
////  gluPerspective(45.0f, (GLfloat)screenWidth/(GLfloat)screenHeight, 0.1f, FAR_PLANE);
// if (screenRotated)
// {
//    gluPerspective(45.0f, 240.0f/320.0f, 0.1f, FAR_PLANE);
// } else
// {
//    gluPerspective(45.0f, 320.0f/240.0f, 0.1f, FAR_PLANE);
// }
//  glMatrixMode(GL_MODELVIEW);
//}
static void
screenResized ()
{
   //senquack - do we really need to call glViewport every single screen refresh!?
//  glViewport(0, 0, screenWidth, screenHeight);
//  glViewport(320-80, 0, 240, 320);
//  glViewport(0, 0, 320, 240);

   //senquack - border on left, flashing border on right, drawboard on top:
//  glViewport(0, -80, 320 , 240);
   //senquack - slight borders on top and bottom (no drawboards), shifted to right 80
//  glViewport(-80, 0, 400, 320);
   //senquack - shifted to the right 160 pixels, slight borders on top and bottom (top larger than bottom):
//  glViewport(-80, 80, 420, 320);
   //senquack - bottom border (maybe 40 pixels), left border (maybe 80 pixels) ,(squished on games' x axis 40 pixels)
//  glViewport((int)(-80.0 * (400.0/320.0)), -80, 400, 320);
   //senquack - left border (80 pixels maybe), top drawboard, good perspective on games' x axis
//  glViewport(0,-80, 400, 320);
   //still has left border of 80 pixels maybe,  shifted left 80 pixels, bottom border of 40 pixels maybe
//  glViewport((int)(-80.0 * (400.0/320.0)), -160, 400, 320);
//  glViewport((int)(-80.0 * (400.0/320.0)), -80, 400, 320);
   //senquack - almost there (squished 40 pix on game's x axis)
//  glViewport((int)(-80.0 * (400.0/320.0)), 0, 400, 320);
   //senquack - almost there, slight border on top and bottom:
//  glViewport((int)(-80.0 * (400.0/320.0)), 0, 440, 320);
   //senquack - close: (shifted a bit to the top)
//  glViewport((int)(-90.0 * (400.0/320.0)), 0, 460, 320);
// if (screenRotated) {
   if (settings.rotated) {
//    glViewport((int)(-87.0 * (400.0/320.0)), 0, 460, 320);
      glViewport ((int) (-90.0 * (400.0 / 320.0)), 0, 463, 320);
   } else {
      glViewport (0, 0, 320, 240);
   }

   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
//  gluPerspective(45.0f, (GLfloat)screenWidth/(GLfloat)screenHeight, 0.1f, FAR_PLANE);
//    gluPerspective(45.0f, 320.0f/240.0f, 0.1f, FAR_PLANE);
   gluPerspectivex (2949120, 87381, 6554, FAR_PLANE_X);

   glMatrixMode (GL_MODELVIEW);

}

//senquack - support screen rotation:
void
resized (int width, int height)
{
   screenWidth = width;
   screenHeight = height;
   screenResized ();
}

////Added for gpu940
//void gluPerspective(GLfloat fovy, GLfloat ratio, GLfloat near, GLfloat far)
//{
// GLfloat top = near * tan(fovy * M_PI/360.0f);
// GLfloat bottom = -top;
// GLfloat right = top * ratio;
// GLfloat left = -right;
//
// glFrustum(left, right, bottom, top, near, far);
//} 
////senquack - speeding up for wiz:
//void gluPerspective(GLfloat fovy, GLfloat ratio, GLfloat near, GLfloat far)
//{
////  GLfloat top = near * tan(fovy * M_PI/360.0f);
// GLfloat top = near * tanf(fovy * 3.14159f/360.0f);
// GLfloat bottom = -top;
// GLfloat right = top * ratio;
// GLfloat left = -right;
//
// glFrustum(left, right, bottom, top, near, far);
//} 
//senquack - fixed point version
//void gluPerspectivex(GLfixed fovy, GLfixed ratio, GLfixed near, GLfixed far)
//{
////  GLfloat top = near * tan(fovy * M_PI/360.0f);
////  GLfloat top = near * tanf(fovy * 3.14159f/360.0f);
////  GLfloat bottom = -top;
////  GLfloat right = top * ratio;
////  GLfloat left = -right;
//
////  GLfixed top = FMUL(near,INT2FNUM(tantbl[FMUL(fovy,572)]));
// GLfloat top = near * tanf(fovy * 3.14159f/360.0f);
// GLfixed bottom = -top;
// GLfixed right = FMUL(top, ratio);
// GLfixed left = -right;
//    
////  glFrustum(left, right, bottom, top, near, far);
// glFrustumx(left, right, bottom, top, near, far);
//} 
void
gluPerspectivex (GLfixed fovy, GLfixed ratio, GLfixed near, GLfixed far)
{
// GLfloat top = near * tan(fovy * M_PI/360.0f);
// GLfloat top = near * tanf(fovy * 3.14159f/360.0f);
// GLfloat bottom = -top;
// GLfloat right = top * ratio;
// GLfloat left = -right;

// GLfixed top = FMUL(near,INT2FNUM(tantbl[FNUM2INT(FMUL(fovy,572))]));
// GLfloat top = near * tanf(fovy * 3.14159f/360.0f);
// GLfixed top = f2x(near * tanf(fovy * 3.14159f/360.0f));
   GLfixed top = FMUL (near, f2x (tanf (x2f (fovy) * 0.008726646f)));
   GLfixed bottom = -top;
   GLfixed right = FMUL (top, ratio);
   GLfixed left = -right;

// glFrustum(left, right, bottom, top, near, far);
   glFrustumx (left, right, bottom, top, near, far);
}

//senquack - speeding up for wiz:
//void gluPerspective(GLfloat fovy, GLfloat ratio, GLfloat near, GLfloat far)
//{
////  GLfloat top = near * tan(fovy * M_PI/360.0f);
////  GLfloat top = near * tanf(fovy * 3.14159f/360.0f);
// GLfloat top = near * tanf(fovy * 0.008726646f);
// GLfloat bottom = -top;
// GLfloat right = top * ratio;
// GLfloat left = -right;
//
// //senquack - for Wiz, glFrustumf seems to be broken:
////  glFrustumf(left, right, bottom, top, near, far);
// glFrustumx(f2x(left), f2x(right), f2x(bottom), f2x(top), f2x(near), f2x(far));
//} 

////senquack - added for Wiz (pulled from libglues)
//#define __glPi 3.14159265358979323846
//
////senquack - added for Wiz (pulled from libglues)
//static void __gluMakeIdentityf(GLfloat m[16])
//{
//    m[0+4*0] = 1; m[0+4*1] = 0; m[0+4*2] = 0; m[0+4*3] = 0;
//    m[1+4*0] = 0; m[1+4*1] = 1; m[1+4*2] = 0; m[1+4*3] = 0;
//    m[2+4*0] = 0; m[2+4*1] = 0; m[2+4*2] = 1; m[2+4*3] = 0;
//    m[3+4*0] = 0; m[3+4*1] = 0; m[3+4*2] = 0; m[3+4*3] = 1;
//}
//
////senquack - added for Wiz (pulled from libglues)
//static void gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar)
//{
//    GLfloat m[4][4];
//    GLfloat sine, cotangent, deltaZ;
//    GLfloat radians = fovy / 2 * __glPi / 180;
//
//    deltaZ = zFar - zNear;
//    sine = sin(radians);
//    if ((deltaZ == 0) || (sine == 0) || (aspect == 0))
//    {
//        return;
//    }
//    cotangent = cos(radians) / sine;
//
//    __gluMakeIdentityf(&m[0][0]);
//    m[0][0] = cotangent / aspect;
//    m[1][1] = cotangent;
//    m[2][2] = -(zFar + zNear) / deltaZ;
//    m[2][3] = -1;
//    m[3][2] = -2 * zNear * zFar / deltaZ;
//    m[3][3] = 0;
//    glMultMatrixf(&m[0][0]);
//}

//senquack - 2/12
// Init OpenGL.
//static void initGL() 
//{
//  printf("Opening gpu940\n"); 
//
//  //senquack
//  glOpen(DEPTH_BUFFER); //Added for gpu940
//
//  printf("Opened gpu940\n");
//  glViewport(0, 0, screenWidth, screenHeight);
//  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
//
//  glLineWidth(1);
//  glEnable(GL_LINE_SMOOTH);
//
//  //senquack - tried tweaking this to fix hang:
//  glEnable(GL_BLEND);
//  //glBlendFunc(GL_SRC_ALPHA, GL_ONE);
//  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
//
//  
//  //senquack
//  //Added for gpu940
//  glDepthMask(GL_TRUE);
//
//  glDisable(GL_LIGHTING);
//  glDisable(GL_CULL_FACE);
//  glDisable(GL_DEPTH_TEST);
//  glDisable(GL_TEXTURE_2D);
//  glDisable(GL_COLOR_MATERIAL);
//
//  resized(screenWidth, screenHeight);
//}
// Init OpenGL.
static void
initGL ()
{
   printf ("Opening gpu940\n");

   //senquack - added support for both rotated and landscape modes into libWIZGLES:
   if (!settings.rotated)
      OS_LandscapeMode ();

   //senquack - why do we need a depth buffer?
//  glOpen(DEPTH_BUFFER); //Added for gpu940
//  glOpen(0);
//
//  printf("Opened gpu940\n");

   //senquack - no longer needed after OpenGLES conversion
//    nanoGL_Init();

   // Create native window.
   printf ("VID_Init: Creating the window\n");
   hNativeWnd = OS_CreateWindow ();
   if (!hNativeWnd)
      printf ("VID_Init: OS_CreateWindow Failed\n");

   EGLint numConfigs;
   EGLint majorVersion;
   EGLint minorVersion;

   glDisplay = eglGetDisplay ((NativeDisplayType) 0);
   if (glDisplay == EGL_NO_DISPLAY) {
      printf ("GL No Display failed\n");
   }

   if (!eglInitialize (glDisplay, &majorVersion, &minorVersion)) {
      printf ("GL Init failed\n");
   }
   if (!eglChooseConfig (glDisplay, attrib_list, &glConfig, 1, &numConfigs)) {
      printf ("GL Config failed\n");
   }
   glContext = eglCreateContext (glDisplay, glConfig, NULL, NULL);
   if (glContext == 0) {
      printf ("GL Context failed\n");
   }
   glSurface = eglCreateWindowSurface (glDisplay, glConfig, hNativeWnd, NULL);
   if (glSurface == 0) {
      printf ("GL Surface failed\n");
   }
   printf ("EGL Init Completed\n");

   eglMakeCurrent (glDisplay, glSurface, glSurface, glContext);

   //senquack - trying to support rotated, zoomed screen
//  glViewport(0, 0, screenWidth, screenHeight);
//  glViewport(320-80, 0, 240, 320);

   //senquack - border on left, flashing border on right, drawboard on top:
//  glViewport(0, -80, 320 , 240);
   //senquack - slight borders on top and bottom (no drawboards), shifted to right 80
//  glViewport(-80, 0, 400, 320);
   //senquack - shifted to the right 160 pixels, slight borders on top and bottom (top larger than bottom):
//  glViewport(-80, 80, 400, 320);
   //senquack - bottom border (maybe 40 pixels), left border (maybe 80 pixels) ,(squished on games' x axis 40 pixels)
//  glViewport((int)(-80.0 * (400.0/320.0)), -80, 400, 320);
   //senquack - left border (80 pixels maybe), top drawboard
//  glViewport(0,-80, 400, 320);
   //still has left border of 80 pixels maybe,  shifted left 80 pixels, bottom border of 40 pixels maybe
//  glViewport((int)(-80.0 * (400.0/320.0)), -160, 400, 320);
//  glViewport((int)(-80.0 * (400.0/320.0)), -80, 400, 320);
   //senquack - almost there (squished 40 pix on game's x axis)
//  glViewport((int)(-80.0 * (400.0/320.0)), 0, 400, 320);
   //senquack - almost there, slight border on top and bottom:
//  glViewport((int)(-80.0 * (400.0/320.0)), 0, 440, 320);
   //senquack - close: (shifted a bit to the top)
//  glViewport((int)(-90.0 * (400.0/320.0)), 0, 460, 320);
//  glViewport((int)(-87.0 * (400.0/320.0)), 0, 460, 320);

// if (screenRotated) {
   if (settings.rotated) {
//    glViewport((int)(-87.0 * (400.0/320.0)), 0, 460, 320);
      glViewport ((int) (-90.0 * (400.0 / 320.0)), 0, 463, 320);
   } else {
      glViewport (0, 0, 320, 240);
   }

   //senquack - tried disabling:
   glClearColor (0.0f, 0.0f, 0.0f, 0.0f);

//  glLineWidth(1);
   // On Wiz, we need to set this differently:
   glLineWidthx (f2x (1.0));

//  glEnable(GL_LINE_SMOOTH);
   glEnable (GL_LINE_SMOOTH);

   //senquack - tried tweaking this to fix hang:
   glEnable (GL_BLEND);
   //glBlendFunc(GL_SRC_ALPHA, GL_ONE);
   glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


   //senquack
   //Added for gpu940
//  glDepthMask(GL_TRUE);

   glDisable (GL_LIGHTING);
   glDisable (GL_CULL_FACE);
   glDisable (GL_DEPTH_TEST);
   glDisable (GL_TEXTURE_2D);
   glDisable (GL_COLOR_MATERIAL);

   //senquack - for OpenGLES, we enable these once and leave them enabled:
   glEnableClientState (GL_VERTEX_ARRAY);
   glEnableClientState (GL_COLOR_ARRAY);
   glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   resized (screenWidth, screenHeight);

//  //senquack - temporary:
////  printf("sizeof(shapes) = %d\n", sizeof(shapes));
//  memset(shapes, sizeof(shapes),0);

}


//// Load bitmaps and convert to textures.
void
loadGLTexture (char *fileName, GLuint * texture)
{
   SDL_Surface *surface;
   int mode;                    //The bit-depth of the texture.
   char name[32];
   strcpy (name, SHARE_LOC);
   strcat (name, "images/");
   strcat (name, fileName);

   surface = IMG_Load (name);   //Changed by Albert... this will load any image (hopefully transparent PNGs)
   if (!surface) {
      fprintf (stderr, "Unable to load texture: %s\n", SDL_GetError ());
      SDL_Quit ();
      exit (1);
   }

   surface = conv_surf_gl (surface, surface->format->Amask
                           || (surface->flags & SDL_SRCCOLORKEY));

/*
	//Attempted hackery to make transparencies/color-keying work - Albert
	SDL_PixelFormat RGBAFormat;
	RGBAFormat.palette = 0; RGBAFormat.colorkey = 0; RGBAFormat.alpha = 0;
	RGBAFormat.BitsPerPixel = 32; RGBAFormat.BytesPerPixel = 4;
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	RGBAFormat.Rmask = 0xFF000000; RGBAFormat.Rshift = 0; RGBAFormat.Rloss = 0;
	RGBAFormat.Gmask = 0x00FF0000; RGBAFormat.Gshift = 8; RGBAFormat.Gloss = 0;
	RGBAFormat.Bmask = 0x0000FF00; RGBAFormat.Bshift = 16; RGBAFormat.Bloss = 0;
	RGBAFormat.Amask = 0x000000FF; RGBAFormat.Ashift = 24; RGBAFormat.Aloss = 0;
	#else
	RGBAFormat.Rmask = 0x000000FF; RGBAFormat.Rshift = 24; RGBAFormat.Rloss = 0;
	RGBAFormat.Gmask = 0x0000FF00; RGBAFormat.Gshift = 16; RGBAFormat.Gloss = 0;
	RGBAFormat.Bmask = 0x00FF0000; RGBAFormat.Bshift = 8; RGBAFormat.Bloss = 0;
	RGBAFormat.Amask = 0xFF000000; RGBAFormat.Ashift = 0; RGBAFormat.Aloss = 0;
	#endif
*/


   /* Create the target alpha surface with correct color component ordering */
/*
  SDL_Surface *alphaImage = SDL_CreateRGBSurface( SDL_SWSURFACE, surface->w,
 	surface->h, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN // OpenGL RGBA masks 
                               0x000000FF, 
                               0x0000FF00, 
                               0x00FF0000, 
                               0xFF000000
#else
                               0xFF000000,
                               0x00FF0000, 
                               0x0000FF00, 
                               0x000000FF
#endif
  );
  
  if (alphaImage == 0)
  	printf("ruh oh, alphaImage creation failed in loadGLTexture() (screen.c)\n");
*/

   // Set up so that colorkey pixels become transparent :
   /* Uint32 colorkey = SDL_MapRGBA(alphaImage->format, 255, 255, 0, 0); //R=255, G=255, B=0
      SDL_FillRect(alphaImage, 0, colorkey);

      colorkey = SDL_MapRGBA(surface->format, 255, 255, 0, 0 );
      SDL_SetColorKey(surface, SDL_SRCCOLORKEY, colorkey);


      SDL_Rect area;

      // Copy the surface into the GL texture image : 
      area.x = 0;
      area.y = 0; 
      area.w = surface->w;
      area.h = surface->h;
      SDL_BlitSurface(surface, &area, alphaImage, &area);
    */

/*
  for (int i = 0; i < conv->w * conv->h; i++)
  {
      
  }
*/

   //SDL_Surface *conv = SDL_ConvertSurface(surface, &RGBAFormat, SDL_SWSURFACE);

//http://osdl.sourceforge.net/OSDL/OSDL-0.3/src/doc/web/main/documentation/rendering/SDL-openGL-examples.html
//http://osdl.sourceforge.net/main/documentation/rendering/SDL-openGL.html --> Good explainations

   // work out what format to tell glTexImage2D to use...
   if (surface->format->BytesPerPixel == 3) {   // RGB 24bit
      mode = GL_RGB;
   } else if (surface->format->BytesPerPixel == 4) {    // RGBA 32bit
      mode = GL_RGBA;
   } else {
      printf ("loadGLTexture: Error: Weird RGB mode found in surface.\n");
      SDL_Quit ();
   }

   //Disabled for gpu940
   glGenTextures (1, texture);
   glBindTexture (GL_TEXTURE_2D, *texture);
   //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
   //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
   //Disabled for gpu940
   //gluBuild2DMipmaps(GL_TEXTURE_2D, 3, surface->w, surface->h, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);

   if (surface == NULL) {
      printf ("Error: surface NULL in loadGLTexture.\n");
      SDL_Quit ();
   }
   //Added by Albert (somehow Kenta Cho managed to get OpenGL to render SDL textures, looks like)
   glTexImage2D (GL_TEXTURE_2D, 0, mode, surface->w, surface->h, 0,
                 mode, GL_UNSIGNED_BYTE, surface->pixels);

   //Added by Albert
   if (surface)
      SDL_FreeSurface (surface);
   //if (alphaImage)
   //   SDL_FreeSurface(alphaImage);   
   //if (conv)
   //   SDL_FreeSurface(conv);                      
}


void
generateTexture (GLuint * texture)
{
   glGenTextures (1, texture);
}

void
deleteTexture (GLuint * texture)
{
   glDeleteTextures (1, texture);
}

static GLuint starTexture;
#define STAR_BMP "star.bmp"
static GLuint smokeTexture;
#define SMOKE_BMP "smoke.bmp"
static GLuint titleTexture;
#define TITLE_BMP "title.bmp"

int lowres = 0;
int windowMode = 0;
int brightness = DEFAULT_BRIGHTNESS;
//Uint8 *keys;
SDL_Joystick *stick = NULL;
int joystickMode = 1;


//senquack - support screen rotation:
//void initSDL() {
//  Uint32 videoFlags;
//
//  if ( lowres ) {
//    screenWidth  = LOWRES_SCREEN_WIDTH;
//    screenHeight = LOWRES_SCREEN_HEIGHT;
//  } else {
//    screenWidth  = SCREEN_WIDTH;
//    screenHeight = SCREEN_HEIGHT;
//  }
//
//////senquack - for Wiz Opengl, re-enable this:
////  /* Initialize SDL */
////  if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
////    fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
////    exit(1);
////  }
//  
// // if ( SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0 ) {
//  if ( SDL_Init(SDL_INIT_JOYSTICK) < 0 ) {
//    fprintf(stderr, "Unable to initialize SDL_JOYSTICK: %s\n", SDL_GetError());
//    joystickMode = 0;
//    exit(1);
//  }
//  
////Changed for gpu940:
//  /* Create an OpenGL screen */
//  //if ( windowMode ) {
//    
//    //videoFlags = SDL
//    //videoFlags = SDL_OPENGL | SDL_RESIZABLE;
//  //} else {
//    //videoFlags = SDL_OPENGL | SDL_FULLSCREEN;
//  //}
//
//  /*if ( SDL_SetVideoMode(screenWidth, screenHeight, 16, videoFlags) == NULL ) {
//    fprintf(stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError());
//    SDL_Quit();
//    exit(2);
//  }*/
//
////senquack - for Wiz Opengl, re-enable this:
////senquack - leaving OPENGL here causes segfault:
////    videoFlags = SDL_OPENGL | SDL_FULLSCREEN;
////    videoFlags = SDL_FULLSCREEN;
////  if ( SDL_SetVideoMode(screenWidth, screenHeight, 16, videoFlags) == NULL ) {
////    fprintf(stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError());
////    SDL_Quit();
////    exit(2);
////  }
//
////    /* Select first display */
////     int status;
////    status=SDL_SelectVideoDisplay(0);
////    if (status<0)
////    {
////        fprintf(stderr, "Can't attach to first display: %s\n", SDL_GetError());
////        exit(-1);
////    }
////
//////    window=SDL_CreateWindow("Nehe: SDL/OpenGL ES Tutorial, Lesson 02",
//////        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
//////        WINDOW_WIDTH, WINDOW_HEIGHT,
//////        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
////    window=SDL_CreateWindow("rRootage",
////        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
////        320, 240,
//////        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN );
////        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
//////        | SDL_WINDOW_FOREIGN);
////    if (window==0)
////    {
////        fprintf(stderr, "Can't create window: %s\n", SDL_GetError());
////        SDL_Quit();
////    }
////
////    glcontext=SDL_GL_CreateContext(window);
////    if (glcontext==NULL)
////    {
////        fprintf(stderr, "Can't create OpenGL ES context: %s\n", SDL_GetError());
////        SDL_Quit();
////    }
////
////    status=SDL_GL_MakeCurrent(window, glcontext);
////    if (status<0)
////    {
////        fprintf(stderr, "Can't set current OpenGL ES context: %s\n", SDL_GetError());
////        SDL_Quit();
////    }
////
////    /* Enable swap on VSYNC */
////    SDL_GL_SetSwapInterval(1);
//
//  if (joystickMode == 1) {
//    SDL_JoystickEventState(SDL_ENABLE);
//    stick = SDL_JoystickOpen(0);
//  }
//
//  /* Set the title bar in environments that support it */
//  SDL_WM_SetCaption(CAPTION, NULL);
//
////senquack - was left enabled for Wiz accidentally:
//  initGL();
//
////  loadGLTexture(STAR_BMP, &starTexture);
////  loadGLTexture(SMOKE_BMP, &smokeTexture);
////  loadGLTexture(TITLE_BMP, &titleTexture);
//
//  SDL_ShowCursor(SDL_DISABLE);
//  
//}
void
initSDL ()
{
   Uint32 videoFlags;

   //senquack
//  if ( lowres ) {
//    screenWidth  = LOWRES_SCREEN_WIDTH;
//    screenHeight = LOWRES_SCREEN_HEIGHT;
//  } else {
//    screenWidth  = SCREEN_WIDTH;
//    screenHeight = SCREEN_HEIGHT;
//  }
//  if (screenRotated) {
//   screenWidth = 240;
//   screenHeight = 320;
//  } else {
//   screenWidth = 320;
//   screenHeight = 240;
//  }
   screenWidth = 320;
   screenHeight = 240;

////senquack - for Wiz Opengl, re-enable this:
//  /* Initialize SDL */
//  if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
//    fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
//    exit(1);
//  }

   // if ( SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0 ) {
   if (SDL_Init (SDL_INIT_JOYSTICK) < 0) {
      fprintf (stderr, "Unable to initialize SDL_JOYSTICK: %s\n",
               SDL_GetError ());
      joystickMode = 0;
      exit (1);
   }
//Changed for gpu940:
   /* Create an OpenGL screen */
   //if ( windowMode ) {

   //videoFlags = SDL
   //videoFlags = SDL_OPENGL | SDL_RESIZABLE;
   //} else {
   //videoFlags = SDL_OPENGL | SDL_FULLSCREEN;
   //}

   /*if ( SDL_SetVideoMode(screenWidth, screenHeight, 16, videoFlags) == NULL ) {
      fprintf(stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError());
      SDL_Quit();
      exit(2);
      } */

//senquack - for Wiz Opengl, re-enable this:
//senquack - leaving OPENGL here causes segfault:
//    videoFlags = SDL_OPENGL | SDL_FULLSCREEN;
//    videoFlags = SDL_FULLSCREEN;
//  if ( SDL_SetVideoMode(screenWidth, screenHeight, 16, videoFlags) == NULL ) {
//    fprintf(stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError());
//    SDL_Quit();
//    exit(2);
// }

//    /* Select first display */
//    int status;
//    status=SDL_SelectVideoDisplay(0);
//    if (status<0)
//    {
//        fprintf(stderr, "Can't attach to first display: %s\n", SDL_GetError());
//        exit(-1);
//    }
//
////    window=SDL_CreateWindow("Nehe: SDL/OpenGL ES Tutorial, Lesson 02",
////        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
////        WINDOW_WIDTH, WINDOW_HEIGHT,
////        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
//    window=SDL_CreateWindow("rRootage",
//        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
//        320, 240,
////        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN );
//        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
////       | SDL_WINDOW_FOREIGN);
//    if (window==0)
//    {
//        fprintf(stderr, "Can't create window: %s\n", SDL_GetError());
//        SDL_Quit();
//    }
//
//    glcontext=SDL_GL_CreateContext(window);
//    if (glcontext==NULL)
//    {
//        fprintf(stderr, "Can't create OpenGL ES context: %s\n", SDL_GetError());
//        SDL_Quit();
//    }
//
//    status=SDL_GL_MakeCurrent(window, glcontext);
//    if (status<0)
//    {
//        fprintf(stderr, "Can't set current OpenGL ES context: %s\n", SDL_GetError());
//        SDL_Quit();
//    }
//
//    /* Enable swap on VSYNC */
//    SDL_GL_SetSwapInterval(1);

   if (joystickMode == 1) {
      SDL_JoystickEventState (SDL_ENABLE);
      stick = SDL_JoystickOpen (0);
   }

   /* Set the title bar in environments that support it */
   SDL_WM_SetCaption (CAPTION, NULL);

//senquack - was left enabled for Wiz accidentally:
   initGL ();

   loadGLTexture (STAR_BMP, &starTexture);
   loadGLTexture (SMOKE_BMP, &smokeTexture);
   loadGLTexture (TITLE_BMP, &titleTexture);

   SDL_ShowCursor (SDL_DISABLE);

}

void
closeSDL ()
{
// //senquack - temporary 
// int typectr, sizectr;
// for (typectr = 0; typectr < 7; typectr++)
// {
//    for(sizectr = 0; (sizectr < 50) && (shapes[typectr][sizectr].size != 0); sizectr++)
//    {
//       printf("type: %d   size: %f\n", typectr, x2f(shapes[typectr][sizectr].size));
//    }
// }

   //senquack
//  SDL_ShowCursor(SDL_ENABLE);
//  glClose();

//senquack - for wiz OpenGLES
   eglMakeCurrent (glDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglDestroySurface (glDisplay, glSurface);
   eglDestroyContext (glDisplay, glContext);
   eglTerminate (glDisplay);

   //senquack - no longer needed after OpenGLES conversion
// nanoGL_Destroy();    

//    /* clean up the window */
//    SDL_GL_DeleteContext(glcontext);
//    SDL_DestroyWindow(window);


//senquack - experiment here for Wiz:
   SDL_Quit ();
}

//senquack - interestingly enough, this is only used in gluLookat and never changes it seems:
float zoom = 15;
////senquack - fixed point version (only used in gluLookat interestingly enough)
GLfixed fzoom = INT2FNUM (15);

static int screenShakeCnt = 0;
static int screenShakeType = 0;

//senquack - experiment 2/12:
//static void setEyepos() {
//  float x, y;
//  glPushMatrix();
//  if ( screenShakeCnt > 0 ) {
//    switch ( screenShakeType ) {
//    case 0:
//      x = (float)randNS2(256)/5000.0f;
//      y = (float)randNS2(256)/5000.0f;
//      break;
//    default:
//      x = (float)randNS2(256)*screenShakeCnt/21000.0f;
//      y = (float)randNS2(256)*screenShakeCnt/21000.0f;
//      break;
//    }
//    gluLookAt(0, 0, zoom, x, y, 0, 0.0f, 1.0f, 0.0f); //changed for gpu940
//  } else {
//    gluLookAt(0, 0, zoom, 0, 0, 0, 0.0f, 1.0f, 0.0f); //changed for gpu940
//  }
//}
//senquack - converting to fixed point:
//static void setEyepos() {
//  float x, y;
//  glPushMatrix();
//  if ( screenShakeCnt > 0 ) {
//    switch ( screenShakeType ) {
//    case 0:
//      x = (float)randNS2(256)/5000.0f;
//      y = (float)randNS2(256)/5000.0f;
//      break;
//    default:
//      x = (float)randNS2(256)*screenShakeCnt/21000.0f;
//      y = (float)randNS2(256)*screenShakeCnt/21000.0f;
//      break;
//    }
//    gluLookAt(0, 0, zoom, x, y, 0, 0.0f, 1.0f, 0.0f); //changed for gpu940
//  } else {
//     gluLookAt(0, 0, zoom, 0, 0, 0, 0.0f, 1.0f, 0.0f); //changed for gpu940
//  }
//}
static void
setEyepos ()
{
   GLfixed x, y;
   glPushMatrix ();
   if (screenShakeCnt > 0) {
      switch (screenShakeType) {
      case 0:
//      x = (float)randNS2(256)/5000.0f;
//      y = (float)randNS2(256)/5000.0f;
         x = FDIV (INT2FNUM (randNS2 (256)), 327680000);
         y = FDIV (INT2FNUM (randNS2 (256)), 327680000);
         break;
      default:
//      x = (float)randNS2(256)*screenShakeCnt/21000.0f;
//      y = (float)randNS2(256)*screenShakeCnt/21000.0f;
         x = FDIV (INT2FNUM (randNS2 (256) * screenShakeCnt), 1376256000);
         y = FDIV (INT2FNUM (randNS2 (256) * screenShakeCnt), 1376256000);
         break;
      }
//    gluLookAt(0, 0, zoom, x, y, 0, 0.0f, 1.0f, 0.0f); //changed for gpu940
      gluLookAtx (0, 0, fzoom, x, y, 0, 0, INT2FNUM (1), 0);
   } else {
//     gluLookAt(0, 0, zoom, 0, 0, 0, 0.0f, 1.0f, 0.0f); //changed for gpu940
      gluLookAtx (0, 0, fzoom, 0, 0, 0, 0, INT2FNUM (1), 0);
   }
}

void
setScreenShake (int type, int cnt)
{
   screenShakeType = type;
   screenShakeCnt = cnt;
}

void
moveScreenShake ()
{
   if (screenShakeCnt > 0) {
      screenShakeCnt--;
   }
}

//senquack - experiment 2/12 (failed)
//void drawGLSceneStart() {
//  glClear(GL_COLOR_BUFFER_BIT);
//  setEyepos();
//}
void
drawGLSceneStart ()
{
   glClear (GL_COLOR_BUFFER_BIT);
//  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   setEyepos ();
}

void
drawGLSceneEnd ()
{
   glPopMatrix ();
}

void
swapGLScene ()
{
   //SDL_GL_SwapBuffers(); //Switched to glSwapBuffers() for gpu940


//  glSwapBuffers();

//senquack - for wiz OpenGLES
   eglSwapBuffers (glDisplay, glSurface);

   //Believe it or not, this does fail sometimes... - Albert
   //if (glSwapBuffers() != GL_TRUE)
   //  printf("glSwapBuffers failed!!\n");
}

// NOTE
//senquack - disabling these next 3-4 functions allowed program to run longer before hang:

// 2/11 - new efforts to convert all triangle fans to something else:
//  //senquack - tried tweaking this to fix hang:
//void drawBox(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
//      int r, int g, int b) {
////  glPushMatrix();
////  glTranslatef(x, y, 0);
////  glColor4i(r, g, b, 128);
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex3f(-width, -height,  0);
////  glVertex3f( width, -height,  0);
////  glVertex3f( width,  height,  0);
////  glVertex3f(-width,  height,  0);
////  glEnd();
////  glColor4i(r, g, b, 255);
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(-width, -height,  0);
////  glVertex3f( width, -height,  0);
////  glVertex3f( width,  height,  0);
////  glVertex3f(-width,  height,  0);
////  glEnd();
////  glPopMatrix();
//}
  //senquack - tried tweaking this to fix hang:
//void drawBox(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
//      int r, int g, int b) {
//  glPushMatrix();
//  glTranslatef(x, y, 0);
//  glColor4i(r, g, b, 128);
////  glColor4f(r, g, b, 0.5);
//
//  //senquack - added disabling of gl_blend to fix diagonal lines
////    glDisable(GL_BLEND);
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex3f(-width, -height,  0);
////  glVertex3f( width, -height,  0);
////  glVertex3f( width,  height,  0);
////  glVertex3f(-width,  height,  0);
////  glEnd();
//
////  glBegin(GL_TRIANGLES);
////  glVertex3f(-width, -height,  0);
////  glVertex3f( width, -height,  0);
////  glVertex3f( width,  height,  0);
////  glEnd();
////  glBegin(GL_TRIANGLES);
////  glVertex3f(-width, -height,  0);
////  glVertex3f( width,  height,  0);
////  glVertex3f(-width,  height,  0);
////  glEnd();
//  glBegin(GL_QUADS);
//  glVertex3f(-width, -height,0);
//  glVertex3f( width, -height,0);
//  glVertex3f( width,  height,0);
//  glVertex3f(-width,  height,0);
//  glEnd();
//
//  glColor4i(r, g, b, 255);
////  glColor4f(r, g, b, 1.0);
//
//  //senquack
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(-width, -height,  0);
////  glVertex3f( width, -height,  0);
////  glVertex3f( width,  height,  0);
////  glVertex3f(-width,  height,  0);
////  glEnd();
//  //senquack - 1st try:
////  glBegin(GL_QUADS);
////  glVertex3f(-width, -height,  0);
////  glVertex3f( width, -height,  0);
////  glVertex3f( width,  height,  0);
////  glVertex3f(-width,  height,  0);
////  glEnd();
//  //senquack - 2nd try:
//
////  glBegin(GL_LINES);
////  glVertex3f(-width, -height,  0);
////  glVertex3f( width, -height,  0);
////  glEnd();
////
////  glBegin(GL_LINES);
////  glVertex3f( width, -height,  0);
////  glVertex3f( width,  height,  0);
////  glEnd();
////
////  glBegin(GL_LINES);
////  glVertex3f( width,  height,  0);
////  glVertex3f(-width,  height,  0);
////  glEnd();
////
////  glBegin(GL_LINES);
////  glVertex3f(-width,  height,  0);
////  glVertex3f(-width, -height,  0);
////  glEnd();
//
//  //senquack - 3rd try:
////  glBegin(GL_LINES);
////  glVertex2f(-width, -height);
////  glVertex2f( width, -height);
////  glEnd();
////
////  glBegin(GL_LINES);
////  glVertex2f( width, -height);
////  glVertex2f( width,  height);
////  glEnd();
////
////  glBegin(GL_LINES);
////  glVertex2f( width,  height);
////  glVertex2f(-width,  height);
////  glEnd();
//
//  //senquack -if I just leave this one enabled it STILL causes the vertical problem
////  glBegin(GL_LINES);
////  glVertex2f(-width,  height);
////  glVertex2f(-width, -height);
////  glEnd();
//
////  glBegin(GL_LINE_LOOP);
////  glVertex2f(-width, -height);
////  glVertex2f( width, -height);
////  glVertex2f( width,  height);
////  glVertex2f(-width,  height);
////  glVertex2f(-width, -height);
////  glEnd();
//
//  glPopMatrix();
//}

static GLfixed boxvertices[2000 * 2];
static GLubyte boxcolors[2000 * 4];
static GLfixed *boxvertptr;
static GLubyte *boxcolptr;

//senquack - new function called once before a series of calls to drawShape (for openglES speedup)
void
prepareDrawBoxes (void)
{
   boxvertptr = &boxvertices[0];
   boxcolptr = &boxcolors[0];
}

void
finishDrawBoxes (void)
{
   glEnable (GL_BLEND);
   glVertexPointer (2, GL_FIXED, 0, boxvertices);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, boxcolors);
   int numboxvertices =
      ((unsigned int) boxcolptr - (unsigned int) (&boxcolors[0])) >> 2;
   //senquack - never seemed to go above 1500 vertices here (higher when using non-rotated mode)
// printf("printing boxes with %d vertices\n", numboxvertices);
   glDrawArrays (GL_TRIANGLES, 0, numboxvertices);
}

//void drawBoxx(GLfixed fx, GLfixed fy, GLfixed fwidth, GLfixed fheight,
//      int r, int g, int b) {
//  glPushMatrix();
//  glTranslatex(fx, fy, 0);
//
//    GLubyte colors[4*4]; 
// GLfixed vertices[4*2];
// colors[0] = colors[4] = colors[8] = colors[12] = r;
// colors[1] = colors[5] = colors[9] = colors[13] = g;
// colors[2] = colors[6] = colors[10] = colors[14] = b;
// colors[3] = colors[7] = colors[11] = colors[15] = 128;
//
// vertices[0] = -fwidth;  vertices[1] = -fheight;
// vertices[2] = fwidth;      vertices[3] = -fheight;
// vertices[4] = fwidth;      vertices[5] = fheight;
// vertices[6] = -fwidth;  vertices[7] = fheight;
//
////  glEnableClientState(GL_VERTEX_ARRAY);
// glVertexPointer(2, GL_FIXED, 0, vertices);
////  glEnableClientState(GL_COLOR_ARRAY);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//
//  glPopMatrix();
//}
void
drawBoxx (GLfixed fx, GLfixed fy, GLfixed fwidth, GLfixed fheight,
          int r, int g, int b)
{
   *boxcolptr++ = r;
   *boxcolptr++ = g;
   *boxcolptr++ = b;
   *boxcolptr++ = 128;
   *boxcolptr++ = r;
   *boxcolptr++ = g;
   *boxcolptr++ = b;
   *boxcolptr++ = 128;
   *boxcolptr++ = r;
   *boxcolptr++ = g;
   *boxcolptr++ = b;
   *boxcolptr++ = 128;
   *boxcolptr++ = r;
   *boxcolptr++ = g;
   *boxcolptr++ = b;
   *boxcolptr++ = 128;
   *boxcolptr++ = r;
   *boxcolptr++ = g;
   *boxcolptr++ = b;
   *boxcolptr++ = 128;
   *boxcolptr++ = r;
   *boxcolptr++ = g;
   *boxcolptr++ = b;
   *boxcolptr++ = 128;
   *boxvertptr++ = fx - fwidth;
   *boxvertptr++ = fy - fheight;
   *boxvertptr++ = fx - fwidth;
   *boxvertptr++ = fy + fheight;
   *boxvertptr++ = fx + fwidth;
   *boxvertptr++ = fy - fheight;
   *boxvertptr++ = fx - fwidth;
   *boxvertptr++ = fy + fheight;
   *boxvertptr++ = fx + fwidth;
   *boxvertptr++ = fy + fheight;
   *boxvertptr++ = fx + fwidth;
   *boxvertptr++ = fy - fheight;
}

//void drawLine(GLfloat x1, GLfloat y1, GLfloat z1,
//       GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a) {
//  glColor4i(r, g, b, a);
//  glBegin(GL_LINES);
//  glVertex3f(x1, y1, z1);
//  glVertex3f(x2, y2, z2);
//  glEnd();
//}
//senquack - tried tweaking this to fix hang:
//senquack - changing this back to 3D coordinates resulted in same screwing up of triangles elsewhere:
//void drawLine(GLfloat x1, GLfloat y1, GLfloat z1,
//       GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a) {
//  glColor4i(r, g, b,a);
//
// //senquack- forcing this did not help:
////  glColor4i(255, 255, 255, 255);
//
//  //glColor4i(r, g, b, a);
////  glBegin(GL_LINES);
////  glVertex2f(x1, y1);
////  glVertex2f(x2, y2);
////  glEnd();
//  glBegin(GL_LINE_LOOP);
//  glVertex3f(x1, y1, z1);
//  glVertex3f(x2, y2, z2);
//  glEnd();
//}
//senquack - nanoGL does not support line drawing, must call OpenGLES directly:
//void drawLine(GLfloat x1, GLfloat y1, GLfloat z1,
//       GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a) {
////  glColor4i(r, g, b,a);
////
////  //senquack- forcing this did not help:
//////  glColor4i(255, 255, 255, 255);
////
////  //glColor4i(r, g, b, a);
//////  glBegin(GL_LINES);
//////  glVertex2f(x1, y1);
//////  glVertex2f(x2, y2);
//////  glEnd();
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(x1, y1, z1);
////  glVertex3f(x2, y2, z2);
////  glEnd();
//
////code from net (2d:)
////const GLfloat line[] = {
////-0.5f, -0.5f, //point A
////0.5f, -0.5f, //point B
////};
////
////glColor4f(0.0f,1.0f,0.0f,1.0f); //line color
////glVertexPointer(2, GL_FLOAT, 0, line);
////glEnableClientState(GL_VERTEX_ARRAY);
////
////glDrawArrays(GL_LINES, 0, 2);
// GLfloat line[6];
// line[0] = x1; line[1] = y1; line[2] = z1;
// line[3] = x2; line[4] = y2; line[5] = z2;
// 
// glDisable(GL_TEXTURE_2D);
////  glColor4f((float)r / 255.0, (float)g / 255.0, (float)b / 255.0, (float)a / 255.0);
//// temporary experiment:
// glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
// glVertexPointer(3, GL_FLOAT, 0, line);
// glEnableClientState(GL_VERTEX_ARRAY);
// 
// glDrawArrays(GL_LINES, 0, 2);
// glDisableClientState(GL_VERTEX_ARRAY);
//}
//void drawLine(GLfloat x1, GLfloat y1, GLfloat z1,
//       GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a) {
////  glColor4i(r, g, b,a);
////
////  //senquack- forcing this did not help:
//////  glColor4i(255, 255, 255, 255);
////
////  //glColor4i(r, g, b, a);
//////  glBegin(GL_LINES);
//////  glVertex2f(x1, y1);
//////  glVertex2f(x2, y2);
//////  glEnd();
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(x1, y1, z1);
////  glVertex3f(x2, y2, z2);
////  glEnd();
//
////code from net (2d:)
////const GLfloat line[] = {
////-0.5f, -0.5f, //point A
////0.5f, -0.5f, //point B
////};
////
////glColor4f(0.0f,1.0f,0.0f,1.0f); //line color
////glVertexPointer(2, GL_FLOAT, 0, line);
////glEnableClientState(GL_VERTEX_ARRAY);
////
////glDrawArrays(GL_LINES, 0, 2);
// GLfloat line[6];
// line[0] = x1; line[1] = y1; line[2] = z1;
// line[3] = x2; line[4] = y2; line[5] = z2;
// 
// glEnable(GL_BLEND);
// glEnable(GL_LINE_SMOOTH);
//
// glLineWidth(2.0f);
//
// glDisable(GL_TEXTURE_2D);
////  glColor4f((float)r / 255.0, (float)g / 255.0, (float)b / 255.0, (float)a / 255.0);
//// temporary experiment:
// glColor4f_direct(1.0f, 1.0f, 1.0f, 1.0f);
// glVertexPointer(3, GL_FLOAT, 0, line);
// glEnableClientState(GL_VERTEX_ARRAY);
// 
// glDrawArrays(GL_LINES, 0, 2);
//
// glDisableClientState(GL_VERTEX_ARRAY);
//}
//void drawLine(GLfloat x1, GLfloat y1, GLfloat z1,
//       GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a) {
////  glColor4i(r, g, b,a);
////
////  //senquack- forcing this did not help:
//////  glColor4i(255, 255, 255, 255);
////
////  //glColor4i(r, g, b, a);
//////  glBegin(GL_LINES);
//////  glVertex2f(x1, y1);
//////  glVertex2f(x2, y2);
//////  glEnd();
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(x1, y1, z1);
////  glVertex3f(x2, y2, z2);
////  glEnd();
//
////code from net (2d:)
////const GLfloat line[] = {
////-0.5f, -0.5f, //point A
////0.5f, -0.5f, //point B
////};
////
////glColor4f(0.0f,1.0f,0.0f,1.0f); //line color
////glVertexPointer(2, GL_FLOAT, 0, line);
////glEnableClientState(GL_VERTEX_ARRAY);
////
////glDrawArrays(GL_LINES, 0, 2);
// GLfloat line[12];
// line[0] = x1; line[1] = y1; line[2] = z1;
// line[3] = x1+10.0; line[4] = y1+10.0; line[5] = z1+10.0;
// line[6] = x2; line[7] = y2; line[8] = z2;
// line[9] = x2+10.0; line[10] = y2+10.0; line[11] = z2+10.0;
// 
//
////  GLfloat colors[16] = {1.0};
//
// GLubyte colors[16] = {255, 255, 255, 255,
//                         255,255,255,255,
//                         255,255,255,255,
//                         255,255,255,255};
////senquack - note: this doesn't work, you must specify all 16 manually
////  GLubyte colors[16] = {255};
//
////  colors[0] = colors[3] = (float)r / 255.0; 
////  colors[1] = colors[4] = (float)g / 255.0; 
////  colors[2] = colors[5] = (float)b / 255.0;
////  colors[0] = colors[4] = 1.0;
////  colors[1] = colors[5] = 1.0;
////  colors[2] = colors[6] = 1.0;
////  colors[3] = colors[7] = 1.0;
// 
// glEnable(GL_BLEND);
// glEnable(GL_LINE_SMOOTH);
// glLineWidth(2.0f);
////  glLineWidth(1.0f);
//
// glDisable(GL_TEXTURE_2D);
////  glColor4f((float)r / 255.0, (float)g / 255.0, (float)b / 255.0, (float)a / 255.0);
//// temporary experiment:
////  glColor4f_direct(1.0f, 1.0f, 1.0f, 1.0f);
// glVertexPointer(3, GL_FLOAT, 0, line);
// glEnableClientState(GL_VERTEX_ARRAY);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
// glEnableClientState(GL_COLOR_ARRAY);
// 
// glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//
////  glDisableClientState(GL_COLOR_ARRAY);
////  glDisableClientState(GL_VERTEX_ARRAY);
//}
//void drawLine(GLfloat x1, GLfloat y1, GLfloat z1,
//       GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a) {
////  glColor4i(r, g, b,a);
////
////  //senquack- forcing this did not help:
//////  glColor4i(255, 255, 255, 255);
////
////  //glColor4i(r, g, b, a);
//////  glBegin(GL_LINES);
//////  glVertex2f(x1, y1);
//////  glVertex2f(x2, y2);
//////  glEnd();
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(x1, y1, z1);
////  glVertex3f(x2, y2, z2);
////  glEnd();
//
////code from net (2d:)
////const GLfloat line[] = {
////-0.5f, -0.5f, //point A
////0.5f, -0.5f, //point B
////};
////
////glColor4f(0.0f,1.0f,0.0f,1.0f); //line color
////glVertexPointer(2, GL_FLOAT, 0, line);
////glEnableClientState(GL_VERTEX_ARRAY);
////
////glDrawArrays(GL_LINES, 0, 2);
// GLfloat line[6];
// line[0] = x1; line[1] = y1; line[2] = z1;
// line[3] = x2; line[4] = y2; line[5] = z2;
//
////  GLfloat colors[8];
//////   colors[0] = colors[3] = (float)r / 255.0; 
//////   colors[1] = colors[4] = (float)g / 255.0; 
//////   colors[2] = colors[5] = (float)b / 255.0;
////  colors[0] = colors[4] = 1.0;
////  colors[1] = colors[5] = 1.0;
////  colors[2] = colors[6] = 1.0;
////  colors[3] = colors[7] = 1.0;
// GLuint colors[8];
////  colors[0] = colors[3] = (float)r / 255.0; 
////  colors[1] = colors[4] = (float)g / 255.0; 
////  colors[2] = colors[5] = (float)b / 255.0;
// colors[0] = colors[4] = 255;
// colors[1] = colors[5] = 255;
// colors[2] = colors[6] = 255;
// colors[3] = colors[7] = 255;
// 
////  glDisable(GL_BLEND);
////  glDisable(GL_LINE_SMOOTH);
////  glLineWidth(2.0f);
// glLineWidth(1.0f);
//
// glDisable(GL_TEXTURE_2D);
////  glColor4f((float)r / 255.0, (float)g / 255.0, (float)b / 255.0, (float)a / 255.0);
//// temporary experiment:
////  glColor4f_direct(1.0f, 1.0f, 1.0f, 1.0f);
// glEnableClientState(GL_VERTEX_ARRAY);
// glEnableClientState(GL_COLOR_ARRAY);
////  glVertexPointer(3, GL_FLOAT, 0, line);
// glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), line);
////  glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
// glColorPointer(4, GL_UNSIGNED_BYTE, 4 * sizeof(GLuint), colors);
// 
// glDrawArrays(GL_LINES, 0, 2);
//
////  glDisableClientState(GL_COLOR_ARRAY);
////  glDisableClientState(GL_VERTEX_ARRAY);
//}
//void drawLine(GLfloat x1, GLfloat y1, GLfloat z1,
//       GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a) {
////  glColor4i(r, g, b,a);
////
////  //senquack- forcing this did not help:
//////  glColor4i(255, 255, 255, 255);
////
////  //glColor4i(r, g, b, a);
//////  glBegin(GL_LINES);
//////  glVertex2f(x1, y1);
//////  glVertex2f(x2, y2);
//////  glEnd();
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(x1, y1, z1);
////  glVertex3f(x2, y2, z2);
////  glEnd();
//
////code from net (2d:)
////const GLfloat line[] = {
////-0.5f, -0.5f, //point A
////0.5f, -0.5f, //point B
////};
////
////glColor4f(0.0f,1.0f,0.0f,1.0f); //line color
////glVertexPointer(2, GL_FLOAT, 0, line);
////glEnableClientState(GL_VERTEX_ARRAY);
////
////glDrawArrays(GL_LINES, 0, 2);
////  GLfloat line[6];
////  line[0] = x1; line[1] = y1; line[2] = z1;
////  line[3] = x2; line[4] = y2; line[5] = z2;
// GLfixed line[6];
// line[0] = f2x(x1); line[1] = f2x(y1); line[2] = f2x(z1);
// line[3] = f2x(x2); line[4] = f2x(y2); line[5] = f2x(z2);
// 
//
////  GLfloat colors[16] = {1.0};
//
// GLubyte colors[8] = {255, 255, 255, 255,
//                         255,255,255,255};
////senquack - note: this doesn't work, you must specify all 16 manually
////  GLubyte colors[16] = {255};
//
////  colors[0] = colors[3] = (float)r / 255.0; 
////  colors[1] = colors[4] = (float)g / 255.0; 
////  colors[2] = colors[5] = (float)b / 255.0;
////  colors[0] = colors[4] = 1.0;
////  colors[1] = colors[5] = 1.0;
////  colors[2] = colors[6] = 1.0;
////  colors[3] = colors[7] = 1.0;
// 
////  glEnable(GL_BLEND);
// glDisable(GL_LINE_SMOOTH);
////  glLineWidth(2.0f);
////  glLineWidth(1.0f);
//
// glDisable(GL_TEXTURE_2D);
////  glColor4f((float)r / 255.0, (float)g / 255.0, (float)b / 255.0, (float)a / 255.0);
//// temporary experiment:
////  glColor4f_direct(1.0f, 1.0f, 1.0f, 1.0f);
//
//  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//
// glVertexPointer(3, GL_FIXED, 0, line);
// glEnableClientState(GL_VERTEX_ARRAY);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
// glEnableClientState(GL_COLOR_ARRAY);
// 
// glDrawArrays(GL_LINES, 0, 2);
//
////  glDisableClientState(GL_COLOR_ARRAY);
////  glDisableClientState(GL_VERTEX_ARRAY);
//}
void
drawLine (GLfloat x1, GLfloat y1, GLfloat z1,
          GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a)
{
//senquack - note: it appears Wiz's opengl supports line drawing with either fixed or float vertices, your choice:
   GLfloat line[6];
   line[0] = x1;
   line[1] = y1;
   line[2] = z1;
   line[3] = x2;
   line[4] = y2;
   line[5] = z2;
// GLfixed line[6];
// line[0] = f2x(x1); line[1] = f2x(y1); line[2] = f2x(z1);
// line[3] = f2x(x2); line[4] = f2x(y2); line[5] = f2x(z2);

   GLubyte colors[8] = { r, g, b, a, r, g, b, a };

// glVertexPointer(3, GL_FIXED, 0, line);
   glVertexPointer (3, GL_FLOAT, 0, line);
   glEnableClientState (GL_VERTEX_ARRAY);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
   glEnableClientState (GL_COLOR_ARRAY);

   glDrawArrays (GL_LINES, 0, 2);

// senquack - note - not necessary to turn these off here it seems:
// glDisableClientState(GL_COLOR_ARRAY);
// glDisableClientState(GL_VERTEX_ARRAY);
}

//senquack - complete fixed point conversion:
//void drawLinex(GLfixed x1, GLfixed y1, GLfixed z1,
//       GLfixed x2, GLfixed y2, GLfixed z2, int r, int g, int b, int a) {
////senquack - note: it appears Wiz's opengl supports line drawing with either fixed or float vertices, your choice:
// GLfixed line[6];
// line[0] = x1; line[1] = y1; line[2] = z1;
// line[3] = x2; line[4] = y2; line[5] = z2;
// 
// GLubyte colors[8] = {r, g, b, a, r, g, b, a};
//
// glVertexPointer(3, GL_FIXED, 0, line);
////  glEnableClientState(GL_VERTEX_ARRAY);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
////  glEnableClientState(GL_COLOR_ARRAY);
// 
// glDrawArrays(GL_LINES, 0, 2);
//}
void
drawLinex (GLfixed x1, GLfixed y1,
           GLfixed x2, GLfixed y2, int r, int g, int b, int a)
{
//senquack - note: it appears Wiz's opengl supports line drawing with either fixed or float vertices, your choice:
   GLfixed line[4];
   line[0] = x1;
   line[1] = y1;
   line[2] = x2;
   line[3] = y2;

   GLubyte colors[8] = { r, g, b, a, r, g, b, a };

   glVertexPointer (2, GL_FIXED, 0, line);
// glEnableClientState(GL_VERTEX_ARRAY);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
// glEnableClientState(GL_COLOR_ARRAY);

   glDrawArrays (GL_LINES, 0, 2);
}


//void drawLinePart(GLfloat x1, GLfloat y1, GLfloat z1,
//      GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a, int len) {
//  glColor4i(r, g, b,a);
//  glBegin(GL_LINES);
//  glVertex3f(x1, y1, z1);
//  glVertex3f(x1+(x2-x1)*len/256, y1+(y2-y1)*len/256, z1+(z2-z1)*len/256);
//  glEnd();
//}
//senquack - tried tweaking this to fix hang:
//void drawLinePart(GLfloat x1, GLfloat y1, GLfloat z1,
//      GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a, int len) {
//  glColor4i(r, g, b, a);
////  glColor4i(255, 255, 255, 255);
////  glBegin(GL_LINES);
////  glVertex2f(x1, y1);
////  glVertex2f(x1+(x2-x1)*len/256, y1+(y2-y1)*len/256);
////  glEnd();
//  glBegin(GL_LINE_LOOP);
//  glVertex3f(x1, y1, z1);
//  glVertex3f(x1+(x2-x1)*len/256, y1+(y2-y1)*len/256, z1+(z2-z1)*len/256);
//  glEnd();
//}
////senquack - nanoGL does not support line drawing, must call OpenGLES directly:
//void drawLinePart(GLfloat x1, GLfloat y1, GLfloat z1,
//      GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a, int len) {
//////  glColor4i(r, g, b, a);
////////  glColor4i(255, 255, 255, 255);
////////  glBegin(GL_LINES);
////////  glVertex2f(x1, y1);
////////  glVertex2f(x1+(x2-x1)*len/256, y1+(y2-y1)*len/256);
////////  glEnd();
//////  glBegin(GL_LINE_LOOP);
//////  glVertex3f(x1, y1, z1);
//////  glVertex3f(x1+(x2-x1)*len/256, y1+(y2-y1)*len/256, z1+(z2-z1)*len/256);
//////  glEnd();
////
////  GLfloat line[6];
////  line[0] = x1; line[1] = y1; line[2] = z1;
////  line[3] = x1+(x2-x1)*len/256; line[4] = y1+(y2-y1)*len/256; line[5] = z1+(z2-z1)*len/256;
////  
////  glDisable(GL_TEXTURE_2D);
////  glColor4f((float)r / 255.0, (float)g / 255.0, (float)b / 255.0, (float)a / 255.0);
////  glVertexPointer(3, GL_FLOAT, 0, line);
////  glEnableClientState(GL_VERTEX_ARRAY);
////  glDrawArrays(GL_LINES, 0, 2);
////  glDisableClientState(GL_VERTEX_ARRAY);
//}
//senquack - nanoGL does not support line drawing, must call OpenGLES directly:
//void drawLinePart(GLfloat x1, GLfloat y1, GLfloat z1,
//      GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a, int len) {
//
// GLfloat line[6];
// line[0] = x1; line[1] = y1; line[2] = z1;
// line[3] = x1+(x2-x1)*len/256; line[4] = y1+(y2-y1)*len/256; line[5] = z1+(z2-z1)*len/256;
// 
// GLubyte colors[8] = {r, g, b, a, r, g, b, a};
//
////  glVertexPointer(3, GL_FIXED, 0, line);
// glVertexPointer(3, GL_FLOAT, 0, line);
// glEnableClientState(GL_VERTEX_ARRAY);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
// glEnableClientState(GL_COLOR_ARRAY);
// glDrawArrays(GL_LINES, 0, 2);
//}
//void drawLinePart(GLfloat x1, GLfloat y1, GLfloat z1,
//      GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a, int len) {
//
// GLfloat line[6];
// line[0] = x1; line[1] = y1; line[2] = z1;
// line[3] = x1+(x2-x1)*len/256; line[4] = y1+(y2-y1)*len/256; line[5] = z1+(z2-z1)*len/256;
// 
// GLubyte colors[8] = {r, g, b, a, r, g, b, a};
//
// glVertexPointer(3, GL_FIXED, 0, line);
////  glVertexPointer(3, GL_FLOAT, 0, line);
// glEnableClientState(GL_VERTEX_ARRAY);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
// glEnableClientState(GL_COLOR_ARRAY);
// glDrawArrays(GL_LINES, 0, 2);
//}
void
drawLinePartx (GLfixed x1, GLfixed y1,
               GLfixed x2, GLfixed y2, int r, int g, int b, int a, int len)
{

   GLfixed line[4];
   line[0] = x1;
   line[1] = y1;
   line[2] = x1 + (((x2 - x1) * len) >> 8);
   line[3] = y1 + (((y2 - y1) * len) >> 8);

   GLubyte colors[8] = { r, g, b, a, r, g, b, a };

   glVertexPointer (2, GL_FIXED, 0, line);
// glEnableClientState(GL_VERTEX_ARRAY);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
// glEnableClientState(GL_COLOR_ARRAY);
   glDrawArrays (GL_LINES, 0, 2);
}

  //senquack - tried tweaking this to fix hang:
//senquack - try changing this to LINE_LOOP to fix drawing problems:
//void drawRollLineAbs(GLfloat x1, GLfloat y1, GLfloat z1,
//         GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a, int d1) {
//  glPushMatrix();
//  glRotatef((float)d1*360/1024, 0, 0, 1);
//  glColor4i(r, g, b, a);
//  glBegin(GL_LINES);
//  glVertex3f(x1, y1, z1);
//  glVertex3f(x2, y2, z2);
//  glEnd();
//  glPopMatrix();
//}
//void drawRollLineAbs(GLfloat x1, GLfloat y1, GLfloat z1,
//         GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a, int d1) {
////  glPushMatrix();
////  glRotatef((float)d1*360/1024, 0, 0, 1);
////   
////  glColor4i(r, g, b, a);
////  glBegin(GL_LINES);
////  glVertex3f(x1, y1, z1);
////  glVertex3f(x2, y2, z2);
////  glEnd();
////
////  glPopMatrix();
//}
//senquack - z axis necessary here:
//void drawRollLineAbs(GLfloat x1, GLfloat y1, GLfloat z1,
//         GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a, int d1) {
//  glPushMatrix();
//  glRotatef((float)d1*360/1024, 0, 0, 1);
//
// GLfloat line[6];
// line[0] = x1; line[1] = y1; line[2] = z1;
// line[3] = x2; line[4] = y2; line[5] = z2;
////  GLfixed line[6];
////  line[0] = f2x(x1); line[1] = f2x(y1); line[2] = f2x(z1);
////  line[3] = f2x(x2); line[4] = f2x(y2); line[5] = f2x(z2);
//
// 
// GLubyte colors[8] = {r, g, b, a, r, g, b, a};
//
////  glVertexPointer(3, GL_FIXED, 0, line);
// glVertexPointer(3, GL_FLOAT, 0, line);
// glEnableClientState(GL_VERTEX_ARRAY);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
// glEnableClientState(GL_COLOR_ARRAY);
// 
// glDrawArrays(GL_LINES, 0, 2);
//  glPopMatrix();
//}
void
drawRollLineAbsx (GLfixed x1, GLfixed y1, GLfixed z1,
                  GLfixed x2, GLfixed y2, GLfixed z2, int r, int g, int b,
                  int a, int d1)
{
   glPushMatrix ();
//  glRotatef((float)d1*360/1024, 0, 0, 1);
   glRotatex ((d1 * 360) << 6, 0, 0, INT2FNUM (1));

   GLfloat line[6];
   line[0] = x1;
   line[1] = y1;
   line[2] = z1;
   line[3] = x2;
   line[4] = y2;
   line[5] = z2;
// GLfixed line[6];
// line[0] = f2x(x1); line[1] = f2x(y1); line[2] = f2x(z1);
// line[3] = f2x(x2); line[4] = f2x(y2); line[5] = f2x(z2);


   GLubyte colors[8] = { r, g, b, a, r, g, b, a };

// glVertexPointer(3, GL_FIXED, 0, line);
   glVertexPointer (3, GL_FLOAT, 0, line);
   glEnableClientState (GL_VERTEX_ARRAY);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
   glEnableClientState (GL_COLOR_ARRAY);

   glDrawArrays (GL_LINES, 0, 2);
   glPopMatrix ();
}


//senquack - new - changing all TRIANGLE_FANs with only three vertices to TRIANGLES
  //senquack - tried tweaking this to fix hang:
//void drawTestPoly()
//{
//  static float x = 0.0f;
//  if (x > 32)
//     x = 0.0f;
//  x++;
//     
//  //printf("drawing poly\n"); fflush(stdout);
//  glBegin(GL_TRIANGLE_FAN);
//  glColor4i(255, 0, 0, 255);
//  glVertex3f(x, 5.0f, 0.0f);
//  glColor4i(0, 255, 0, 255);
//  glVertex3f(0.0f, -5.0f, 0.0f);
//  glColor4i(0, 0, 255, 255);
//  glVertex3f(-2.0f, 5.0f, 0.0f);
//  glEnd();
//  //printf("done drawing poly\n"); fflush(stdout);
//
//}
//void drawTestPoly()
//{
//  static float x = 0.0f;
////  if (x > 32)
////     x = 0.0f;
////  x++;
//     
//  //printf("drawing poly\n"); fflush(stdout);
//  //senquack:
////  glBegin(GL_TRIANGLE_FAN);
//  glBegin(GL_TRIANGLES);
//  glColor4i(255, 0, 0, 255);
//  glVertex3f(x, 5.0f, 0.0f);
//  glColor4i(0, 255, 0, 255);
//  glVertex3f(0.0f, -5.0f, 0.0f);
//  glColor4i(0, 0, 255, 255);
//  glVertex3f(-2.0f, 5.0f, 0.0f);
//  glEnd();
//  //printf("done drawing poly\n"); fflush(stdout);
//
//}

  //senquack - tried tweaking this to fix hang:
//void drawRollLine(GLfloat x, GLfloat y, GLfloat z, GLfloat width,
//      int r, int g, int b, int a, int d1, int d2) {
//  glPushMatrix();
//  glTranslatef(x, y, z);
//  glRotatef((float)d1*360/1024, 0, 0, 1);
//  glRotatef((float)d2*360/1024, 1, 0, 0);
//  glColor4i(r, g, b, a);
//  glBegin(GL_LINES);
//  glVertex3f(0, -width, 0);
//  glVertex3f(0,  width, 0);
//  glEnd();
//  glPopMatrix();
//}
//void drawRollLine(GLfloat x, GLfloat y, GLfloat z, GLfloat width,
//      int r, int g, int b, int a, int d1, int d2) {
////  glPushMatrix();
////  glTranslatef(x, y, z);
////  glRotatef((float)d1*360/1024, 0, 0, 1);
////  glRotatef((float)d2*360/1024, 1, 0, 0);
////  glColor4i(r, g, b, a);
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(0, -width, 0);
////  glVertex3f(0,  width, 0);
////  glEnd();
////  glPopMatrix();
//}
void
drawRollLine (GLfloat x, GLfloat y, GLfloat z, GLfloat width,
              int r, int g, int b, int a, int d1, int d2)
{
   glPushMatrix ();
   glTranslatef (x, y, z);
   glRotatef ((float) d1 * 360 / 1024, 0, 0, 1);
   glRotatef ((float) d2 * 360 / 1024, 1, 0, 0);

   GLfloat line[6];
   line[0] = 0;
   line[1] = -width;
   line[2] = 0;
   line[3] = 0;
   line[4] = width;
   line[5] = 0;

   GLubyte colors[8] = { r, g, b, a, r, g, b, a };

   glVertexPointer (3, GL_FLOAT, 0, line);
   glEnableClientState (GL_VERTEX_ARRAY);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
   glEnableClientState (GL_COLOR_ARRAY);

   glDrawArrays (GL_LINES, 0, 2);
   glPopMatrix ();
}

//senquack - fixed point version (only called from frag.c)
// note: we dropped the a parameter (always 255 it turns out)
GLfixed rolllinevertices[4];
GLubyte rolllinecolors[8];

//senquack - this allows us to avoid a few hundred unnecessary calls to these two functions:
void
prepareDrawRollLinex (void)
{
   glVertexPointer (2, GL_FIXED, 0, rolllinevertices);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, rolllinecolors);
}

inline void
drawRollLinex (GLfixed x, GLfixed y, GLfixed z, GLfixed width,
               int r, int g, int b, int d1, int d2)
{
   glPushMatrix ();
//  glTranslatef(x, y, z);
   glTranslatex (x, y, z);
//  glRotatef((float)d1*360/1024, 0, 0, 1);
//  glRotatef((float)d2*360/1024, 1, 0, 0);
//    glRotatex((d1*360)<<6, 0, 0, INT2FNUM(1));
//    glRotatex((d2*360)<<6, INT2FNUM(1), 0, 0);
   glRotatef ((float) ((d1 * 360) >> 10), 0, 0, 1);
   glRotatef ((float) ((d2 * 360) >> 10), 1, 0, 0);

// GLfixed line[4];
   rolllinevertices[0] = 0;
   rolllinevertices[1] = -width;
   rolllinevertices[2] = 0;
   rolllinevertices[3] = width;

// GLubyte colors[8];
   rolllinecolors[0] = rolllinecolors[4] = r;
   rolllinecolors[1] = rolllinecolors[5] = g;
   rolllinecolors[2] = rolllinecolors[6] = b;
   rolllinecolors[3] = rolllinecolors[7] = 255;

// glVertexPointer(2, GL_FIXED, 0, line);
////  glEnableClientState(GL_VERTEX_ARRAY);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
////  glEnableClientState(GL_COLOR_ARRAY);

   glDrawArrays (GL_LINES, 0, 2);
   glPopMatrix ();
}


////senquack
////drawSquare really is just for drawing the wings and it will be handled with VBOs now:
//static GLfloat wingvertices[3*8]; // array capable of holding up to 8 3D vertices
//static GLubyte wingcolors[4*8];      // array capable of holding up to 8 RGBA colors
//
////senquack - new function called once before a series of calls to drawShape (for openglES speedup)
//void prepareDrawWings(void)
//{
// glVertexPointer(2, GL_FIXED, 0, wingvertices);
// glEnableClientState(GL_VERTEX_ARRAY);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, wingcolors);
// glEnableClientState(GL_COLOR_ARRAY);
//}

// 2/11 - new efforts to convert all triangle fans to something else:
//senquack - further attempts 2/11/2010:
  //senquack - tried tweaking this to fix hang:
//void drawSquare(GLfloat x1, GLfloat y1, GLfloat z1,
//    GLfloat x2, GLfloat y2, GLfloat z2,
//    GLfloat x3, GLfloat y3, GLfloat z3,
//    GLfloat x4, GLfloat y4, GLfloat z4,
//    int r, int g, int b) {
//  glColor4i(r, g, b, 64);
//
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(x1, y1, z1);
//  glVertex3f(x2, y2, z2);
//  glVertex3f(x3, y3, z3);
//  glVertex3f(x4, y4, z4);
//  glEnd();
//}
//void drawSquare(GLfloat x1, GLfloat y1, GLfloat z1,
//    GLfloat x2, GLfloat y2, GLfloat z2,
//    GLfloat x3, GLfloat y3, GLfloat z3,
//    GLfloat x4, GLfloat y4, GLfloat z4,
//    int r, int g, int b) {
//  glColor4i(r, g, b, 64);
//
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(x1, y1, z1);
//  glVertex3f(x2, y2, z2);
//  glVertex3f(x3, y3, z3);
//  glVertex3f(x4, y4, z4);
//  glEnd();
//
////  // senquack 2/11 converting this to triangles 
////  glBegin(GL_TRIANGLES);
////  glVertex3f(x1, y1, z1);
////  glVertex3f(x2, y2, z2);
////  glVertex3f(x3, y3, z3);
////  glEnd();
////
////  glBegin(GL_TRIANGLES);
////  glVertex3f(x1, y1, z1);
////  glVertex3f(x3, y3, z3);
////  glVertex3f(x4, y4, z4);
////  glEnd();
//}
void
drawSquare (GLfloat x1, GLfloat y1, GLfloat z1,
            GLfloat x2, GLfloat y2, GLfloat z2,
            GLfloat x3, GLfloat y3, GLfloat z3,
            GLfloat x4, GLfloat y4, GLfloat z4, int r, int g, int b)
{
   GLubyte colors[4 * 4];
// GLfloat vertices[4*3];
   GLfixed vertices[4 * 3];
   colors[0] = colors[4] = colors[8] = colors[12] = r;
   colors[1] = colors[5] = colors[9] = colors[13] = g;
   colors[2] = colors[6] = colors[10] = colors[14] = b;
   colors[3] = colors[7] = colors[11] = colors[15] = 64;

// vertices[0] = x1; vertices[1] = y1;    vertices[2] = z1;
// vertices[3] = x2; vertices[4] = y2;    vertices[5] = z2;
// vertices[6] = x3; vertices[7] = y3;    vertices[8] = z3;
// vertices[9] = x4; vertices[10] = y4; vertices[11] = z4;
   vertices[0] = f2x (x1);
   vertices[1] = f2x (y1);
   vertices[2] = f2x (z1);
   vertices[3] = f2x (x2);
   vertices[4] = f2x (y2);
   vertices[5] = f2x (z2);
   vertices[6] = f2x (x3);
   vertices[7] = f2x (y3);
   vertices[8] = f2x (z3);
   vertices[9] = f2x (x4);
   vertices[10] = f2x (y4);
   vertices[11] = f2x (z4);

   glEnableClientState (GL_VERTEX_ARRAY);
// glVertexPointer(3, GL_FLOAT, 0, vertices);
   glVertexPointer (3, GL_FIXED, 0, vertices);
   glEnableClientState (GL_COLOR_ARRAY);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
   glDrawArrays (GL_TRIANGLE_FAN, 0, 4);
}

void
drawSquarex (GLfixed x1, GLfixed y1, GLfixed z1,
             GLfixed x2, GLfixed y2, GLfixed z2,
             GLfixed x3, GLfixed y3, GLfixed z3,
             GLfixed x4, GLfixed y4, GLfixed z4, int r, int g, int b)
{
   GLubyte colors[4 * 4];
   GLfixed vertices[4 * 3];
   colors[0] = colors[4] = colors[8] = colors[12] = r;
   colors[1] = colors[5] = colors[9] = colors[13] = g;
   colors[2] = colors[6] = colors[10] = colors[14] = b;
   colors[3] = colors[7] = colors[11] = colors[15] = 64;

   vertices[0] = x1;
   vertices[1] = y1;
   vertices[2] = z1;
   vertices[3] = x2;
   vertices[4] = y2;
   vertices[5] = z2;
   vertices[6] = x3;
   vertices[7] = y3;
   vertices[8] = z3;
   vertices[9] = x4;
   vertices[10] = y4;
   vertices[11] = z4;

// glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer (3, GL_FIXED, 0, vertices);
// glEnableClientState(GL_COLOR_ARRAY);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
   glDrawArrays (GL_TRIANGLE_FAN, 0, 4);
}

//senquack - this was already commented out:
//void drawStar(int f, GLfloat x, GLfloat y, GLfloat z, int r, int g, int b, float size) {
// /* glEnable(GL_TEXTURE_2D);
//  if ( f ) {
//    glBindTexture(GL_TEXTURE_2D, starTexture);
//  } else {
//    glBindTexture(GL_TEXTURE_2D, smokeTexture);
//  }
//  */
//  /*
//  //printf("drawing star...\n"); fflush(stdout);
//  glColor4i(r, g, b, 255);
//  glPushMatrix();
//  glTranslatef(x, y, z);
//  glRotatef(rand()%360, 0.0f, 0.0f, 1.0f);
//  glBegin(GL_TRIANGLE_FAN);
//  glTexCoord2f(0.0f, 1.0f);
//  glVertex3f(-size, -size,  0);
//  glTexCoord2f(1.0f, 1.0f);
//  glVertex3f( size, -size,  0);
//  glTexCoord2f(1.0f, 0.0f);
//  glVertex3f( size,  size,  0);
//  glTexCoord2f(0.0f, 0.0f);
//  glVertex3f(-size,  size,  0);
//  glEnd();
//  glPopMatrix();
//  glDisable(GL_TEXTURE_2D);
//  */
//}
void
drawStarx (int f, GLfixed fx, GLfixed fy, int r, int g, int b, GLfixed fsize)
{
   /* glEnable(GL_TEXTURE_2D);
      if ( f ) {
      glBindTexture(GL_TEXTURE_2D, starTexture);
      } else {
      glBindTexture(GL_TEXTURE_2D, smokeTexture);
      }
    */
   /*
      //printf("drawing star...\n"); fflush(stdout);
      glColor4i(r, g, b, 255);
      glPushMatrix();
      glTranslatef(x, y, z);
      glRotatef(rand()%360, 0.0f, 0.0f, 1.0f);
      glBegin(GL_TRIANGLE_FAN);
      glTexCoord2f(0.0f, 1.0f);
      glVertex3f(-size, -size,  0);
      glTexCoord2f(1.0f, 1.0f);
      glVertex3f( size, -size,  0);
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f( size,  size,  0);
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(-size,  size,  0);
      glEnd();
      glPopMatrix();
      glDisable(GL_TEXTURE_2D);
    */

   GLfixed vertices[4 * 2];
   GLubyte colors[4 * 4];
   GLfixed texvertices[4 * 2];
   colors[0] = colors[4] = colors[8] = colors[12] = r;
   colors[1] = colors[5] = colors[9] = colors[13] = g;
   colors[2] = colors[6] = colors[10] = colors[14] = b;
   colors[3] = colors[7] = colors[11] = colors[15] = 255;
   vertices[0] = fsize;
   vertices[1] = -fsize;
   vertices[2] = fsize;
   vertices[3] = fsize;
   vertices[4] = -fsize;
   vertices[5] = -fsize;
   vertices[6] = -fsize;
   vertices[7] = fsize;
   texvertices[0] = INT2FNUM (1);
   texvertices[1] = 0;
   texvertices[2] = INT2FNUM (1);
   texvertices[3] = INT2FNUM (1);
   texvertices[4] = 0;
   texvertices[5] = 0;
   texvertices[6] = 0;
   texvertices[7] = INT2FNUM (1);

   glBlendFunc (GL_ONE, GL_SRC_ALPHA);
//  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
// glBlendFunc(GL_ONE, GL_SRC_ALPHA);
//    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 
//    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
   glEnableClientState (GL_TEXTURE_COORD_ARRAY);

   glEnable (GL_BLEND);
   glEnable (GL_TEXTURE_2D);
   if (f) {
      glBindTexture (GL_TEXTURE_2D, starTexture);
   } else {
      glBindTexture (GL_TEXTURE_2D, smokeTexture);
   }
   glPushMatrix ();
   glTranslatex (fx, fy, 0);
// glRotatef((float)(rand()%360), 0.0f, 0.0f, 1.0f);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
   glVertexPointer (2, GL_FIXED, 0, vertices);
   glTexCoordPointer (2, GL_FIXED, 0, texvertices);
   glRotatex (INT2FNUM (rand () % 360), 0, 0, 1);
   glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
   glPopMatrix ();
   glDisable (GL_TEXTURE_2D);
   glDisableClientState (GL_TEXTURE_COORD_ARRAY);
   glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

#define LASER_ALPHA 100
#define LASER_LINE_ALPHA 50
#define LASER_LINE_ROLL_SPEED 17
#define LASER_LINE_UP_SPEED 16

//void drawLaser(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
//        int cc1, int cc2, int cc3, int cc4, int cnt, int type) {
//  int i, d;
//  float gx, gy;
//  glBegin(GL_TRIANGLE_FAN);
//  if ( type != 0 ) {
////    glColor4ub(cc1, cc1, cc1, LASER_ALPHA);
//    glColor4i(cc1, cc1, cc1, LASER_ALPHA);
//    glVertex3f(x-width, y, 0);
//  }
//  glColor4i(cc2, 255, cc2, LASER_ALPHA);
//  glVertex3f(x, y, 0);
//  glColor4i(cc4, 255, cc4, LASER_ALPHA);
//  glVertex3f(x, y+height, 0);
//  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
//  glVertex3f(x-width, y+height, 0);
//  glEnd();
//  glBegin(GL_TRIANGLE_FAN);
//  if ( type != 0 ) {
//    glColor4i(cc1, cc1, cc1, LASER_ALPHA);
//    glVertex3f(x+width, y, 0);
//  }
//  glColor4i(cc2, 255, cc2, LASER_ALPHA);
//  glVertex3f(x, y, 0);
//  glColor4i(cc4, 255, cc4, LASER_ALPHA);
//  glVertex3f(x, y+height, 0);
//  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
//  glVertex3f(x+width, y+height, 0);
//  glEnd();
//  if ( type == 2 ) return;
////  glColor4i(80, 240, 80, LASER_LINE_ALPHA);
////  glBegin(GL_LINES);
////  d = (cnt*LASER_LINE_ROLL_SPEED)&(512/4-1);
////  for ( i=0 ; i<4 ; i++, d+=(512/4) ) {
////    d &= 1023;
////    gx = x + width*sctbl[d+256]/256.0f;
////    if ( type == 1 ) {
////      glVertex3f(gx, y, 0);
////    } else {
////      glVertex3f(x, y, 0);
////    }
////    glVertex3f(gx, y+height, 0);
////  }
////  if ( type == 0 ) {
////    glEnd();
////    return;
////  }
////  gy = y + (height/4/LASER_LINE_UP_SPEED) * (cnt&(LASER_LINE_UP_SPEED-1));
////  for ( i=0 ; i<4 ; i++, gy+=height/4 ) {
////    glVertex3f(x-width, gy, 0);
////    glVertex3f(x+width, gy, 0);
////  }
////  glEnd();
//}
//senquack - experimenting with speed improvements
//void drawLaser(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
//        int cc1, int cc2, int cc3, int cc4, int cnt, int type) {
//  int i, d;
//  float gx, gy;
//  glBegin(GL_TRIANGLE_FAN);
//  if ( type != 0 ) {
//    glColor4i(cc1, cc1, cc1, LASER_ALPHA);
//    glVertex3f(x-width, y, 0);
//  }
//  glColor4i(cc2, 255, cc2, LASER_ALPHA);
//  glVertex3f(x, y, 0);
//  glColor4i(cc4, 255, cc4, LASER_ALPHA);
//  glVertex3f(x, y+height, 0);
//  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
//  glVertex3f(x-width, y+height, 0);
//  glEnd();
//  glBegin(GL_TRIANGLE_FAN);
//  if ( type != 0 ) {
//    glColor4i(cc1, cc1, cc1, LASER_ALPHA);
//    glVertex3f(x+width, y, 0);
//  }
//  glColor4i(cc2, 255, cc2, LASER_ALPHA);
//  glVertex3f(x, y, 0);
//  glColor4i(cc4, 255, cc4, LASER_ALPHA);
//  glVertex3f(x, y+height, 0);
//  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
//  glVertex3f(x+width, y+height, 0);
//  glEnd();
//  if ( type == 2 ) return;
//  glColor4i(80, 240, 80, LASER_LINE_ALPHA);
//  glBegin(GL_LINES);
//  d = (cnt*LASER_LINE_ROLL_SPEED)&(512/4-1);
//  for ( i=0 ; i<4 ; i++, d+=(512/4) ) {
//    d &= 1023;
//    gx = x + width*sctbl[d+256]/256.0f;
//    if ( type == 1 ) {
//      glVertex3f(gx, y, 0);
//    } else {
//      glVertex3f(x, y, 0);
//    }
//    glVertex3f(gx, y+height, 0);
//  }
//  if ( type == 0 ) {
//    glEnd();
//    return;
//  }
//  gy = y + (height/4/LASER_LINE_UP_SPEED) * (cnt&(LASER_LINE_UP_SPEED-1));
//  for ( i=0 ; i<4 ; i++, gy+=height/4 ) {
//    glVertex3f(x-width, gy, 0);
//    glVertex3f(x+width, gy, 0);
//  }
//  glEnd();
//}
////senquack - stripped-down triangle version:
//void drawLaser(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
//        int cc1, int cc2, int cc3, int cc4, int cnt, int type) {
//  int i, d;
//  float gx, gy;
//
//  //senquack - original left-side code, modifying to match right-side code that had to be changed below:
////  glBegin(GL_TRIANGLE_FAN);
////  if ( type != 0 ) {
////    glColor4i(cc1, cc1, cc1, LASER_ALPHA);
////    glVertex3f(x-width, y, 0);
////  }
////  glColor4i(cc2, 255, cc2, LASER_ALPHA);
////  glVertex3f(x, y, 0);
////  glColor4i(cc4, 255, cc4, LASER_ALPHA);
////  glVertex3f(x, y+height, 0);
////  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
////  glVertex3f(x-width, y+height, 0);
////  glEnd();
////senquack - new - changing all TRIANGLE_FANs with only three vertices to TRIANGLES
////  glBegin(GL_TRIANGLE_FAN);
////  glBegin(GL_TRIANGLES);
//////  if ( type != 0 ) {
//////    glColor4i(cc1, cc1, cc1, LASER_ALPHA);
//////    glVertex3f(x-width, y, 0);
//////  }
////  glColor4i(cc2, 255, cc2, LASER_ALPHA);
////  glVertex3f(x, y, 0);
////  glColor4i(cc4, 255, cc4, LASER_ALPHA);
////  glVertex3f(x, y+height, 0);
////  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
////  glVertex3f(x-width, y+height, 0);
////  glEnd();
////  glEnd(); // senquack - merging this triangle and the one below
//
//  //senquack - this is the code for the right side that has the jerky bottom-right corner:
////  glBegin(GL_TRIANGLE_FAN);
////  if ( type != 0 ) {
////    glColor4i(cc1, cc1, cc1, LASER_ALPHA);
////    glVertex3f(x+width, y, 0);
////  }
////  glColor4i(cc2, 255, cc2, LASER_ALPHA);
////  glVertex3f(x, y, 0);
////  glColor4i(cc4, 255, cc4, LASER_ALPHA);
////  glVertex3f(x, y+height, 0);
////  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
////  glVertex3f(x+width, y+height, 0);
////  glEnd();
////senquack - new - changing all TRIANGLE_FANs with only three vertices to TRIANGLES
////  glBegin(GL_TRIANGLE_FAN);
////  glBegin(GL_TRIANGLES); // senquack - merging this triangle and the one above
//
////  if ( type != 0 ) {
////    glColor4i(cc1, cc1, cc1, LASER_ALPHA);
////    glVertex3f(x+width, y, 0);
////  }
//  glBegin(GL_TRIANGLE_FAN);
//  glColor4i(cc2, 255, cc2, LASER_ALPHA);
//  glVertex2f(x, y);
//  glColor4i(cc4, 255, cc4, LASER_ALPHA);
//  glVertex2f(x, y+height);
//  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
//  glVertex2f(x-width, y+height);
//  glColor4i(cc2, 255, cc2, LASER_ALPHA);
//  glVertex2f(x, y);
//  glColor4i(cc4, 255, cc4, LASER_ALPHA);
//  glVertex2f(x, y+height);
//  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
//  glVertex2f(x+width, y+height);
//  glEnd();
//
//
//
////  glVertex3f(x, y, 0);
//////  glColor4i(cc4, 255, cc4, LASER_ALPHA);
////  glVertex3f(x, y+height, 0);
//////  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
////  glVertex3f(x+width, y+height, 0);
//
//  //senquack - simply disabling this code reduces a lot of the weird assymetric line drawing
//  //         and also still darned cool and clean.  Should provide a much-needed speedup too!
////  if ( type == 2 ) return;
////  glColor4i(80, 240, 80, LASER_LINE_ALPHA);
////  glBegin(GL_LINES);
////  d = (cnt*LASER_LINE_ROLL_SPEED)&(512/4-1);
////  for ( i=0 ; i<4 ; i++, d+=(512/4) ) {
////    d &= 1023;
////    gx = x + width*sctbl[d+256]/256.0f;
////    if ( type == 1 ) {
////      glVertex3f(gx, y, 0);
////    } else {
////      glVertex3f(x, y, 0);
////    }
////    glVertex3f(gx, y+height, 0);
////  }
////  if ( type == 0 ) {
////    glEnd();
////    return;
////  }
////  gy = y + (height/4/LASER_LINE_UP_SPEED) * (cnt&(LASER_LINE_UP_SPEED-1));
////  for ( i=0 ; i<4 ; i++, gy+=height/4 ) {
////    glVertex3f(x-width, gy, 0);
////    glVertex3f(x+width, gy, 0);
////  }
////  glEnd();
//}
//senquack - nicer, fuller, slower version:
//void drawLaser(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
//        int cc1, int cc2, int cc3, int cc4, int cnt, int type) {
//  int i, d;
//  float gx, gy;
//
//  glBegin(GL_TRIANGLE_FAN);
//  if ( type != 0 ) {
//    glColor4i(cc1, cc1, cc1, LASER_ALPHA);
//    glVertex2f(x-width, y);
//  }
//  glColor4i(cc2, 255, cc2, LASER_ALPHA);
//  glVertex2f(x, y);
//  glColor4i(cc4, 255, cc4, LASER_ALPHA);
//  glVertex2f(x, y+height);
//  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
//  glVertex2f(x-width, y+height);
//  glEnd();
//
//
//  glBegin(GL_TRIANGLE_FAN);
//  if ( type != 0 ) {
//    glColor4i(cc1, cc1, cc1, LASER_ALPHA);
//    glVertex2f(x+width, y);
//  }
//  glColor4i(cc2, 255, cc2, LASER_ALPHA);
//  glVertex2f(x, y);
//  glColor4i(cc4, 255, cc4, LASER_ALPHA);
//  glVertex2f(x, y+height);
//  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
//  glVertex2f(x+width, y+height);
//  glEnd();
//  //senquack - no need for these:
////  if ( type == 2 ) return;
////  glColor4i(80, 240, 80, LASER_LINE_ALPHA);
////  glBegin(GL_LINES);
////  d = (cnt*LASER_LINE_ROLL_SPEED)&(512/4-1);
////  for ( i=0 ; i<4 ; i++, d+=(512/4) ) {
////    d &= 1023;
////    gx = x + width*sctbl[d+256]/256.0f;
////    if ( type == 1 ) {
////      glVertex3f(gx, y, 0);
////    } else {
////      glVertex3f(x, y, 0);
////    }
////    glVertex3f(gx, y+height, 0);
////  }
////  if ( type == 0 ) {
////    glEnd();
////    return;
////  }
////  gy = y + (height/4/LASER_LINE_UP_SPEED) * (cnt&(LASER_LINE_UP_SPEED-1));
////  for ( i=0 ; i<4 ; i++, gy+=height/4 ) {
////    glVertex3f(x-width, gy, 0);
////    glVertex3f(x+width, gy, 0);
////  }
////  glEnd();
//}
//void drawLaser(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
//        int cc1, int cc2, int cc3, int cc4, int cnt, int type) {
//  int i, d;
//  float gx, gy;
//
//  //senquack - original left-side code, modifying to match right-side code that had to be changed below:
////  glBegin(GL_TRIANGLE_FAN);
////  if ( type != 0 ) {
////    glColor4i(cc1, cc1, cc1, LASER_ALPHA);
////    glVertex3f(x-width, y, 0);
////  }
////  glColor4i(cc2, 255, cc2, LASER_ALPHA);
////  glVertex3f(x, y, 0);
////  glColor4i(cc4, 255, cc4, LASER_ALPHA);
////  glVertex3f(x, y+height, 0);
////  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
////  glVertex3f(x-width, y+height, 0);
////  glEnd();
////senquack - new - changing all TRIANGLE_FANs with only three vertices to TRIANGLES
////  glBegin(GL_TRIANGLE_FAN);
////  glBegin(GL_TRIANGLES);
//////  if ( type != 0 ) {
//////    glColor4i(cc1, cc1, cc1, LASER_ALPHA);
//////    glVertex3f(x-width, y, 0);
//////  }
////  glColor4i(cc2, 255, cc2, LASER_ALPHA);
////  glVertex3f(x, y, 0);
////  glColor4i(cc4, 255, cc4, LASER_ALPHA);
////  glVertex3f(x, y+height, 0);
////  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
////  glVertex3f(x-width, y+height, 0);
////  glEnd();
////  glEnd(); // senquack - merging this triangle and the one below
//
//  //senquack - this is the code for the right side that has the jerky bottom-right corner:
////  glBegin(GL_TRIANGLE_FAN);
////  if ( type != 0 ) {
////    glColor4i(cc1, cc1, cc1, LASER_ALPHA);
////    glVertex3f(x+width, y, 0);
////  }
////  glColor4i(cc2, 255, cc2, LASER_ALPHA);
////  glVertex3f(x, y, 0);
////  glColor4i(cc4, 255, cc4, LASER_ALPHA);
////  glVertex3f(x, y+height, 0);
////  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
////  glVertex3f(x+width, y+height, 0);
////  glEnd();
////senquack - new - changing all TRIANGLE_FANs with only three vertices to TRIANGLES
////  glBegin(GL_TRIANGLE_FAN);
////  glBegin(GL_TRIANGLES); // senquack - merging this triangle and the one above
//
////  if ( type != 0 ) {
////    glColor4i(cc1, cc1, cc1, LASER_ALPHA);
////    glVertex3f(x+width, y, 0);
////  }
//  glBegin(GL_TRIANGLE_FAN);
//  glColor4i(cc2, 255, cc2, LASER_ALPHA);
//  glVertex2f(x, y);
//  glColor4i(cc4, 255, cc4, LASER_ALPHA);
//  glVertex2f(x, y+height);
//  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
//  glVertex2f(x-width, y+height);
//  glColor4i(cc2, 255, cc2, LASER_ALPHA);
//  glVertex2f(x, y);
//  glColor4i(cc4, 255, cc4, LASER_ALPHA);
//  glVertex2f(x, y+height);
//  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
//  glVertex2f(x+width, y+height);
//  glEnd();
//
//
//
////  glVertex3f(x, y, 0);
//////  glColor4i(cc4, 255, cc4, LASER_ALPHA);
////  glVertex3f(x, y+height, 0);
//////  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
////  glVertex3f(x+width, y+height, 0);
//
//  //senquack - simply disabling this code reduces a lot of the weird assymetric line drawing
//  //         and also still darned cool and clean.  Should provide a much-needed speedup too!
////  if ( type == 2 ) return;
////  glColor4i(80, 240, 80, LASER_LINE_ALPHA);
////  glBegin(GL_LINES);
////  d = (cnt*LASER_LINE_ROLL_SPEED)&(512/4-1);
////  for ( i=0 ; i<4 ; i++, d+=(512/4) ) {
////    d &= 1023;
////    gx = x + width*sctbl[d+256]/256.0f;
////    if ( type == 1 ) {
////      glVertex3f(gx, y, 0);
////    } else {
////      glVertex3f(x, y, 0);
////    }
////    glVertex3f(gx, y+height, 0);
////  }
////  if ( type == 0 ) {
////    glEnd();
////    return;
////  }
////  gy = y + (height/4/LASER_LINE_UP_SPEED) * (cnt&(LASER_LINE_UP_SPEED-1));
////  for ( i=0 ; i<4 ; i++, gy+=height/4 ) {
////    glVertex3f(x-width, gy, 0);
////    glVertex3f(x+width, gy, 0);
////  }
////  glEnd();
//}
////senquack - (fixed point openglES)
//void drawLaserx(GLfixed fx, GLfixed fy, GLfixed fwidth, GLfixed fheight,
//        int cc1, int cc2, int cc3, int cc4, int cnt, int type) {
//  int i, d;
////  float gx, gy;
////  GLfixed fgx, fgy; // only needed if we're drawing ugly, useless lines
//
////  glBegin(GL_TRIANGLE_FAN);
////  if ( type != 0 ) {
////    glColor4i(cc1, cc1, cc1, LASER_ALPHA);
////    glVertex3f(x-width, y, 0);
////  }
//
// //optimized openglES version requires an external array with separate init and finish functions:
// GLfixed lasertrivertices[4 * 2];
// GLubyte lasertricolors[4 * 4];
// int curidxlaser = 0;
//
// glEnableClientState(GL_VERTEX_ARRAY);
// glVertexPointer(2, GL_FIXED, 0, lasertrivertices);
// glEnableClientState(GL_COLOR_ARRAY);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, lasertricolors);
//
//
// if (type != 0)
// {
//    lasertricolors[(curidxlaser<<2)] = 
//       lasertricolors[(curidxlaser<<2)+1] = 
//       lasertricolors[(curidxlaser<<2)+2] = cc1;
//    lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
//    lasertrivertices[(curidxlaser<<1)] = fx-fwidth;
//    lasertrivertices[(curidxlaser<<1)+1] = fy;
//    curidxlaser++;
// }
//
////  glColor4i(cc2, 255, cc2, LASER_ALPHA);
////  glVertex3f(x, y, 0);
////  glColor4i(cc4, 255, cc4, LASER_ALPHA);
////  glVertex3f(x, y+height, 0);
////  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
////  glVertex3f(x-width, y+height, 0);
////  glEnd();
//    lasertricolors[(curidxlaser<<2)] = 
//       lasertricolors[(curidxlaser<<2)+2] = cc2;
//       lasertricolors[(curidxlaser<<2)+1] = 255;
//    lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
//    lasertrivertices[(curidxlaser<<1)] = fx;
//    lasertrivertices[(curidxlaser<<1)+1] = fy;
//    curidxlaser++;
//    lasertricolors[(curidxlaser<<2)] = 
//       lasertricolors[(curidxlaser<<2)+2] = cc4;
//       lasertricolors[(curidxlaser<<2)+1] = 255;
//    lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
//    lasertrivertices[(curidxlaser<<1)] = fx;
//    lasertrivertices[(curidxlaser<<1)+1] = fy + fheight;
//    curidxlaser++;
//    lasertricolors[(curidxlaser<<2)] = 
//       lasertricolors[(curidxlaser<<2)+1] = 
//       lasertricolors[(curidxlaser<<2)+2] = cc3;
//    lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
//    lasertrivertices[(curidxlaser<<1)] = fx-fwidth;
//    lasertrivertices[(curidxlaser<<1)+1] = fy + fheight;
//    if (curidxlaser == 2) 
//    {
//       glDrawArrays(GL_TRIANGLES, 0, 3);
//    } else {
//       glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//    }
//    curidxlaser = 0;
// 
//
////  glBegin(GL_TRIANGLE_FAN);
////  if ( type != 0 ) {
////    glColor4i(cc1, cc1, cc1, LASER_ALPHA);
////    glVertex3f(x+width, y, 0);
////  }
//
// if (type != 0)
// {
//    lasertricolors[(curidxlaser<<2)] = 
//       lasertricolors[(curidxlaser<<2)+1] = 
//       lasertricolors[(curidxlaser<<2)+2] = cc1;
//    lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
//    lasertrivertices[(curidxlaser<<1)] = fx+fwidth;
//    lasertrivertices[(curidxlaser<<1)+1] = fy;
//    curidxlaser++;
// }
//
////  glColor4i(cc2, 255, cc2, LASER_ALPHA);
////  glVertex3f(x, y, 0);
////  glColor4i(cc4, 255, cc4, LASER_ALPHA);
////  glVertex3f(x, y+height, 0);
////  glColor4i(cc3, cc3, cc3, LASER_ALPHA);
////  glVertex3f(x+width, y+height, 0);
////  glEnd();
//
//    lasertricolors[(curidxlaser<<2)] = 
//       lasertricolors[(curidxlaser<<2)+2] = cc2;
//       lasertricolors[(curidxlaser<<2)+1] = 255;
//    lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
//    lasertrivertices[(curidxlaser<<1)] = fx;
//    lasertrivertices[(curidxlaser<<1)+1] = fy;
//    curidxlaser++;
//    lasertricolors[(curidxlaser<<2)] = 
//       lasertricolors[(curidxlaser<<2)+2] = cc4;
//       lasertricolors[(curidxlaser<<2)+1] = 255;
//    lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
//    lasertrivertices[(curidxlaser<<1)] = fx;
//    lasertrivertices[(curidxlaser<<1)+1] = fy+fheight;
//    curidxlaser++;
//    lasertricolors[(curidxlaser<<2)] = 
//       lasertricolors[(curidxlaser<<2)+1] = 
//       lasertricolors[(curidxlaser<<2)+2] = cc3;
//    lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
//    lasertrivertices[(curidxlaser<<1)] = fx+fwidth;
//    lasertrivertices[(curidxlaser<<1)+1] = fy+fheight;
////     glDrawArrays(GL_TRIANGLE_FAN, 0, curidxlaser);
//    if (curidxlaser == 2) 
//    {
//       glDrawArrays(GL_TRIANGLES, 0, 3);
//    } else {
//       glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//    }
//
////  if ( type == 2 ) return;
////  glColor4i(80, 240, 80, LASER_LINE_ALPHA);
////  glBegin(GL_LINES);
////  d = (cnt*LASER_LINE_ROLL_SPEED)&(512/4-1);
////  for ( i=0 ; i<4 ; i++, d+=(512/4) ) {
////    d &= 1023;
////    gx = x + width*sctbl[d+256]/256.0f;
////    if ( type == 1 ) {
////      glVertex3f(gx, y, 0);
////    } else {
////      glVertex3f(x, y, 0);
////    }
////    glVertex3f(gx, y+height, 0);
////  }
////  if ( type == 0 ) {
////    glEnd();
////    return;
////  }
////  gy = y + (height/4/LASER_LINE_UP_SPEED) * (cnt&(LASER_LINE_UP_SPEED-1));
////  for ( i=0 ; i<4 ; i++, gy+=height/4 ) {
////    glVertex3f(x-width, gy, 0);
////    glVertex3f(x+width, gy, 0);
////  }
////  glEnd();
//
//}

//static GLfixed laservertices[2*380];    // only 354 max seem to be used but to be safe we will reserve 380
//static GLubyte lasercolors[4*380];      
static GLfixed laservertices[2 * 75];   // only 61 max seem to be used but to be safe we will reserve 75
static GLubyte lasercolors[4 * 75];
GLfixed *laservertptr;
GLubyte *lasercolptr;

//senquack - new function called once before a series of calls to drawlaser (for openglES speedup)
void
prepareDrawLaserx (void)
{
   laservertptr = &(laservertices[0]);
   lasercolptr = &(lasercolors[0]);
}

void
finishDrawLaserx (void)
{
   int numlaservertices =
      ((unsigned int) lasercolptr - (unsigned int) &(lasercolors[0])) >> 2;
   //debugging - with new triangle strip code, only 61 vertices total max (down from 300+)
// printf("Drawing laser with %d vertices\n", numlaservertices);
   glVertexPointer (2, GL_FIXED, 0, laservertices);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, lasercolors);
   glDrawArrays (GL_TRIANGLE_STRIP, 0, numlaservertices);
   laservertptr = &(laservertices[0]);
   lasercolptr = &(lasercolors[0]);
}

////senquack - (fixed point openglES) (only using one color to draw the laser)
//inline void drawLaserx(GLfixed fx, GLfixed fy, GLfixed fwidth, GLfixed fheight,
//        int cc1, int cnt, int type) {
//  int i, d;
////  float gx, gy;
////  GLfixed fgx, fgy; // only needed if we're drawing ugly, useless lines
//  
// if (type != 0)
// {
////     lasertricolors[(curidxlaser<<2)] = 
////        lasertricolors[(curidxlaser<<2)+2] = 0;
////        lasertricolors[(curidxlaser<<2)+1] = cc1;
////     lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
////     lasertrivertices[(curidxlaser<<1)] = fx-fwidth;
////     lasertrivertices[(curidxlaser<<1)+1] = fy;
////     curidxlaser++;
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx-fwidth; *laservertptr++ = fy;
//
////     lasertricolors[(curidxlaser<<2)] = 
////        lasertricolors[(curidxlaser<<2)+2] = 0;
////        lasertricolors[(curidxlaser<<2)+1] = cc1;
////     lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
////     lasertrivertices[(curidxlaser<<1)] = fx;
////     lasertrivertices[(curidxlaser<<1)+1] = fy;
////     curidxlaser++;
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx; *laservertptr++ = fy;
//    
////     lasertricolors[(curidxlaser<<2)] = 
////        lasertricolors[(curidxlaser<<2)+2] = 0;
////        lasertricolors[(curidxlaser<<2)+1] = cc1;
////     lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
////     lasertrivertices[(curidxlaser<<1)] = fx;
////     lasertrivertices[(curidxlaser<<1)+1] = fy + fheight;
////     curidxlaser++;
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx; *laservertptr++ = fy+fheight;
//
////     lasertricolors[(curidxlaser<<2)] = 
////        lasertricolors[(curidxlaser<<2)+2] = 0;
////        lasertricolors[(curidxlaser<<2)+1] = cc1;
////     lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
////     lasertrivertices[(curidxlaser<<1)] = fx;
////     lasertrivertices[(curidxlaser<<1)+1] = fy;
////     curidxlaser++;
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx; *laservertptr++ = fy;
//
////     lasertricolors[(curidxlaser<<2)] = 
////        lasertricolors[(curidxlaser<<2)+2] = 0;
////        lasertricolors[(curidxlaser<<2)+1] = cc1;
////     lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
////     lasertrivertices[(curidxlaser<<1)] = fx;
////     lasertrivertices[(curidxlaser<<1)+1] = fy + fheight;
////     curidxlaser++;
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx; *laservertptr++ = fy+fheight;
//
////     lasertricolors[(curidxlaser<<2)] = 
////        lasertricolors[(curidxlaser<<2)+2] = 0;
////        lasertricolors[(curidxlaser<<2)+1] = cc1;
////     lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
////     lasertrivertices[(curidxlaser<<1)] = fx-fwidth;
////     lasertrivertices[(curidxlaser<<1)+1] = fy + fheight;
////     curidxlaser++;
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx-fwidth; *laservertptr++ = fy+fheight;
// } else {
////     lasertricolors[(curidxlaser<<2)] = 
////        lasertricolors[(curidxlaser<<2)+2] = 0;
////        lasertricolors[(curidxlaser<<2)+1] = cc1;
////     lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
////     lasertrivertices[(curidxlaser<<1)] = fx;
////     lasertrivertices[(curidxlaser<<1)+1] = fy;
////     curidxlaser++;
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx; *laservertptr++ = fy;
//
////     lasertricolors[(curidxlaser<<2)] = 
////        lasertricolors[(curidxlaser<<2)+2] = 0;
////        lasertricolors[(curidxlaser<<2)+1] = cc1;
////     lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
////     lasertrivertices[(curidxlaser<<1)] = fx;
////     lasertrivertices[(curidxlaser<<1)+1] = fy + fheight;
////     curidxlaser++;
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx; *laservertptr++ = fy+fheight;
//
////     lasertricolors[(curidxlaser<<2)] = 
////        lasertricolors[(curidxlaser<<2)+2] = 0;
////        lasertricolors[(curidxlaser<<2)+1] = cc1;
////     lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
////     lasertrivertices[(curidxlaser<<1)] = fx-fwidth;
////     lasertrivertices[(curidxlaser<<1)+1] = fy + fheight;
////     curidxlaser++;
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx-fwidth; *laservertptr++ = fy+fheight;
// }
//    
////     if (curidxlaser == 2) 
////     {
////        glDrawArrays(GL_TRIANGLES, 0, 3);
////     } else {
////        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
////     }
////     curidxlaser = 0;
//
// if (type != 0)
// {
////     lasertricolors[(curidxlaser<<2)] = 
////        lasertricolors[(curidxlaser<<2)+2] = 0;
////        lasertricolors[(curidxlaser<<2)+1] = cc1;
////     lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
////     lasertrivertices[(curidxlaser<<1)] = fx+fwidth;
////     lasertrivertices[(curidxlaser<<1)+1] = fy;
////     curidxlaser++;
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx+fwidth; *laservertptr++ = fy;
//
////     lasertricolors[(curidxlaser<<2)] = 
////        lasertricolors[(curidxlaser<<2)+2] = 0;
////        lasertricolors[(curidxlaser<<2)+1] = cc1;
////     lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
////     lasertrivertices[(curidxlaser<<1)] = fx;
////     lasertrivertices[(curidxlaser<<1)+1] = fy;
////     curidxlaser++;
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx; *laservertptr++ = fy;
//
////     lasertricolors[(curidxlaser<<2)] = 
////        lasertricolors[(curidxlaser<<2)+2] = 0;
////        lasertricolors[(curidxlaser<<2)+1] = cc1;
////     lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
////     lasertrivertices[(curidxlaser<<1)] = fx;
////     lasertrivertices[(curidxlaser<<1)+1] = fy+fheight;
////     curidxlaser++;
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx; *laservertptr++ = fy+fheight;
//
// } else {
////     lasertricolors[(curidxlaser<<2)] = 
////        lasertricolors[(curidxlaser<<2)+2] = 0;
////        lasertricolors[(curidxlaser<<2)+1] = cc1;
////     lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
////     lasertrivertices[(curidxlaser<<1)] = fx;
////     lasertrivertices[(curidxlaser<<1)+1] = fy;
////     curidxlaser++;
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx; *laservertptr++ = fy;
//
////     lasertricolors[(curidxlaser<<2)] = 
////        lasertricolors[(curidxlaser<<2)+2] = 0;
////        lasertricolors[(curidxlaser<<2)+1] = cc1;
////     lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
////     lasertrivertices[(curidxlaser<<1)] = fx;
////     lasertrivertices[(curidxlaser<<1)+1] = fy+fheight;
////     curidxlaser++;
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx; *laservertptr++ = fy+fheight;
//
////     lasertricolors[(curidxlaser<<2)] = 
////        lasertricolors[(curidxlaser<<2)+2] = 0;
////        lasertricolors[(curidxlaser<<2)+1] = cc1;
////     lasertricolors[(curidxlaser<<2)+3] = LASER_ALPHA;
////     lasertrivertices[(curidxlaser<<1)] = fx+fwidth;
////     lasertrivertices[(curidxlaser<<1)+1] = fy+fheight;
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx+fwidth; *laservertptr++ = fy+fheight;
//
////     if (curidxlaser == 2) 
////     {
////        glDrawArrays(GL_TRIANGLES, 0, 3);
////     } else {
////        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
////     }
// }
//}
//inline void drawLaserx(GLfixed fx, GLfixed fy, GLfixed fwidth, GLfixed fheight,
//        int cc1, int cnt, int type) {
//  int i, d;
//  
// if (type != 0)
// {
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx-fwidth; *laservertptr++ = fy;
//
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx-fwidth; *laservertptr++ = fy+fheight;
//    
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx+fwidth; *laservertptr++ = fy;
//
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx-fwidth; *laservertptr++ = fy+fheight;
//
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx+fwidth; *laservertptr++ = fy+fheight;
//
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx+fwidth; *laservertptr++ = fy;
// } else {
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx; *laservertptr++ = fy;
//
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx-fwidth; *laservertptr++ = fy+fheight;
//
//    *lasercolptr++ = 0; *lasercolptr++ = cc1; *lasercolptr++ = 0; *lasercolptr++ = LASER_ALPHA;
//    *laservertptr++ = fx+fwidth; *laservertptr++ = fy+fheight;
// }
//}
//senquack - heavily optimized triangle strip version:
inline void
drawLaserx (GLfixed fx, GLfixed fy, GLfixed fwidth, GLfixed fheight,
            int cc1, int cnt, int type)
{
   if (type != 0) {
      *lasercolptr++ = 0;
      *lasercolptr++ = cc1;
      *lasercolptr++ = 0;
      *lasercolptr++ = LASER_ALPHA;
      *laservertptr++ = fx - fwidth;
      *laservertptr++ = fy + fheight;

      *lasercolptr++ = 0;
      *lasercolptr++ = cc1;
      *lasercolptr++ = 0;
      *lasercolptr++ = LASER_ALPHA;
      *laservertptr++ = fx + fwidth;
      *laservertptr++ = fy + fheight;

   } else {
      //senquack - fix for getting triangle strip working properly:
//    laservertptr = &(laservertices[0]);
//    lasercolptr = &(lasercolors[0]);
      finishDrawLaserx ();

      *lasercolptr++ = 0;
      *lasercolptr++ = cc1;
      *lasercolptr++ = 0;
      *lasercolptr++ = LASER_ALPHA;
      *laservertptr++ = fx;
      *laservertptr++ = fy;

      *lasercolptr++ = 0;
      *lasercolptr++ = cc1;
      *lasercolptr++ = 0;
      *lasercolptr++ = LASER_ALPHA;
      *laservertptr++ = fx - fwidth;
      *laservertptr++ = fy + fheight;

      *lasercolptr++ = 0;
      *lasercolptr++ = cc1;
      *lasercolptr++ = 0;
      *lasercolptr++ = LASER_ALPHA;
      *laservertptr++ = fx + fwidth;
      *laservertptr++ = fy + fheight;
   }
}

#define SHAPE_POINT_SIZE 0.05f
#define SHAPE_POINT_SIZE_X 3277 // fixed point version of above number for OpenGLES
#define SHAPE_BASE_COLOR_R 250
#define SHAPE_BASE_COLOR_G 240
#define SHAPE_BASE_COLOR_B 180

#define CORE_HEIGHT 0.2f
#define CORE_HEIGHT_X 13107     // fixed point version of above for openGLES
#define CORE_RING_SIZE 0.6f
#define CORE_RING_SIZE_X 39322  // fixed point version of above for openGLES

#define SHAPE_POINT_SIZE_L 0.07f
#define SHAPE_POINT_SIZE_L_X 4588   // fixed point version of above number for openGLES

//senquack - fixed point optimization
//static void drawRing(GLfloat x, GLfloat y, int d1, int d2, int r, int g, int b) {
//  int i, d;
//  float x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
//  glPushMatrix();
//  glTranslatef(x, y, 0);
//  glRotatef((float)d1*360/1024, 0, 0, 1);
//  glRotatef((float)d2*360/1024, 1, 0, 0);
//  glColor4i(r, g, b, 255);
//  x1 = x2 = 0;
//  y1 = y4 =  CORE_HEIGHT/2;
//  y2 = y3 = -CORE_HEIGHT/2;
//  z1 = z2 = CORE_RING_SIZE;
//  for ( i=0,d=0 ; i<8 ; i++ ) {
//    d+=(1024/8); d &= 1023;
//    x3 = x4 = sctbl[d+256]*CORE_RING_SIZE/256;
//    z3 = z4 = sctbl[d]    *CORE_RING_SIZE/256;
//    drawSquare(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, r, g, b);
//    x1 = x3; y1 = y3; z1 = z3;
//    x2 = x4; y2 = y4; z2 = z4;
//  }
//  glPopMatrix();
//}
//senquack - optimizing more:
//static void drawRingx(GLfixed x, GLfixed y, int d1, int d2, int r, int g, int b) {
//  int i, d;
////  float x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
//  GLfixed x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
//  glPushMatrix();
//  glTranslatex(x, y, 0);
////  glRotatef((float)d1*360/1024, 0, 0, 1);
////  glRotatef((float)d2*360/1024, 1, 0, 0);
//    glRotatex((d1*360)<<6, 0, 0, INT2FNUM(1));
//    glRotatex((d2*360)<<6, INT2FNUM(1), 0, 0);
//
////  glColor4i(r, g, b, 255);
//  x1 = x2 = 0;
////  y1 = y4 =  CORE_HEIGHT/2;
//  y1 = y4 =  CORE_HEIGHT_X>>1;
////  y2 = y3 = -CORE_HEIGHT/2;
//  y2 = y3 = -(CORE_HEIGHT_X>>1);
////  z1 = z2 = CORE_RING_SIZE;
//  z1 = z2 = CORE_RING_SIZE_X;
//  for ( i=0,d=0 ; i<8 ; i++ ) {
//    d+=(1024/8); d &= 1023;
////    x3 = x4 = sctbl[d+256]*CORE_RING_SIZE/256;
////    z3 = z4 = sctbl[d]    *CORE_RING_SIZE/256;
//    x3 = x4 = FMUL(INT2FNUM(sctbl[d+256]),CORE_RING_SIZE_X)>>8;
//    z3 = z4 = FMUL(INT2FNUM(sctbl[d]),CORE_RING_SIZE_X)>>8;
////    drawSquare(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, r, g, b);
//    drawSquarex(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, r, g, b);
//    x1 = x3; y1 = y3; z1 = z3;
//    x2 = x4; y2 = y4; z2 = z4;
//  }
//  glPopMatrix();
//}

//static GLfixed ringvertices[18 * 3];
//static GLubyte ringcolors[18 * 4];
//static GLfixed *ringvertptr;
//static GLubyte *ringcolptr;
//
////senquack - added this
//static void prepareDrawRingx(void)
//{
// ringvertptr = &(ringvertices[0]);
// ringcolptr = &(ringcolors[0]);
//}
//
////senquack - added this
//static void finishDrawRingx(void)
//{
// int numringvertices = ((unsigned int)ringcolptr - (unsigned int)&(ringcolors[0])) >> 2;
////  printf("Drawing ring with %d vertices\n", numringvertices);
// glVertexPointer(3, GL_FIXED, 0, ringvertices);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, ringcolors);
////  glDrawArrays(GL_TRIANGLE_FAN, 0, numringvertices);
// glDrawArrays(GL_TRIANGLE_STRIP, 0, numringvertices);
//}
//
//static void drawRingx(GLfixed x, GLfixed y, int d1, int d2, int r, int g, int b) {
// prepareDrawRingx();
//
//  int i, d;
////  float x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
//  GLfixed x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
//  glPushMatrix();
//  glTranslatex(x, y, 0);
////  glRotatef((float)d1*360/1024, 0, 0, 1);
////  glRotatef((float)d2*360/1024, 1, 0, 0);
////    glRotatex((d1*360)<<6, 0, 0, INT2FNUM(1));
////    glRotatex((d2*360)<<6, INT2FNUM(1), 0, 0);
//  glRotatef((float)((d1*360)>>10), 0, 0, 1);
//  glRotatef((float)((d2*360)>>10), 1, 0, 0);
//
////  glColor4i(r, g, b, 255);
//  x1 = x2 = 0;
////  y1 = y4 =  CORE_HEIGHT/2;
//  y1 = y4 =  CORE_HEIGHT_X>>1;
////  y2 = y3 = -CORE_HEIGHT/2;
//  y2 = y3 = -(CORE_HEIGHT_X>>1);
////  z1 = z2 = CORE_RING_SIZE;
//  z1 = z2 = CORE_RING_SIZE_X;
//
// *ringcolptr++ = r; *ringcolptr++ = g; *ringcolptr++ = b; *ringcolptr++ = 64;
// *ringcolptr++ = r; *ringcolptr++ = g; *ringcolptr++ = b; *ringcolptr++ = 64;
// *ringvertptr++ = x1; *ringvertptr++ = y1; *ringvertptr++ = z1;
// *ringvertptr++ = x2; *ringvertptr++ = y2; *ringvertptr++ = z2;
//  for ( i=0,d=0 ; i<8 ; i++ ) {
//    d+=(1024/8); d &= 1023;
////    x3 = x4 = sctbl[d+256]*CORE_RING_SIZE/256;
////    z3 = z4 = sctbl[d]    *CORE_RING_SIZE/256;
//    x3 = x4 = FMUL(INT2FNUM(sctbl[d+256]),CORE_RING_SIZE_X)>>8;
//    z3 = z4 = FMUL(INT2FNUM(sctbl[d]),CORE_RING_SIZE_X)>>8;
////    drawSquare(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, r, g, b);
////    drawSquarex(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, r, g, b);
//    *ringcolptr++ = r; *ringcolptr++ = g; *ringcolptr++ = b; *ringcolptr++ = 64;
//    *ringcolptr++ = r; *ringcolptr++ = g; *ringcolptr++ = b; *ringcolptr++ = 64;
//    *ringvertptr++ = x3; *ringvertptr++ = y3; *ringvertptr++ = z3;
//    *ringvertptr++ = x4; *ringvertptr++ = y4; *ringvertptr++ = z4;
////    x1 = x3; y1 = y3; z1 = z3;
////    x2 = x4; y2 = y4; z2 = z4;
//  }
//
// finishDrawRingx();
//  glPopMatrix();
//}
static GLubyte ringcolors[4 * 4];
static GLfixed ringvertices[4 * 3];
static GLubyte *ringcolptr;
static GLfixed *ringvertptr;

static void
prepareDrawRingx (void)
{
   glVertexPointer (3, GL_FIXED, 0, ringvertices);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, ringcolors);
}



static void
drawRingx (GLfixed x, GLfixed y, int d1, int d2, int r, int g, int b)
{
   int i, d;
//  float x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
   GLfixed x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;


   glPushMatrix ();
   glTranslatex (x, y, 0);
//  glRotatef((float)d1*360/1024, 0, 0, 1);
//  glRotatef((float)d2*360/1024, 1, 0, 0);
//    glRotatex((d1*360)<<6, 0, 0, INT2FNUM(1));
//    glRotatex((d2*360)<<6, INT2FNUM(1), 0, 0);
   glRotatef ((float) ((d1 * 360) >> 10), 0, 0, 1);
   glRotatef ((float) ((d2 * 360) >> 10), 1, 0, 0);

//  glColor4i(r, g, b, 255);
   x1 = x2 = 0;
//  y1 = y4 =  CORE_HEIGHT/2;
   y1 = y4 = CORE_HEIGHT_X >> 1;
//  y2 = y3 = -CORE_HEIGHT/2;
   y2 = y3 = -(CORE_HEIGHT_X >> 1);
//  z1 = z2 = CORE_RING_SIZE;
   z1 = z2 = CORE_RING_SIZE_X;
   for (i = 0, d = 0; i < 8; i++) {
      ringcolptr = &(ringcolors[0]);
      ringvertptr = &(ringvertices[0]);
      d += (1024 / 8);
      d &= 1023;
//    x3 = x4 = sctbl[d+256]*CORE_RING_SIZE/256;
//    z3 = z4 = sctbl[d]    *CORE_RING_SIZE/256;
      x3 = x4 = FMUL (INT2FNUM (sctbl[d + 256]), CORE_RING_SIZE_X) >> 8;
      z3 = z4 = FMUL (INT2FNUM (sctbl[d]), CORE_RING_SIZE_X) >> 8;
//    drawSquare(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, r, g, b);
//    drawSquarex(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, r, g, b);
      *ringcolptr++ = r;
      *ringcolptr++ = g;
      *ringcolptr++ = b;
      *ringcolptr++ = 64;
      *ringcolptr++ = r;
      *ringcolptr++ = g;
      *ringcolptr++ = b;
      *ringcolptr++ = 64;
      *ringcolptr++ = r;
      *ringcolptr++ = g;
      *ringcolptr++ = b;
      *ringcolptr++ = 64;
      *ringcolptr++ = r;
      *ringcolptr++ = g;
      *ringcolptr++ = b;
      *ringcolptr++ = 64;
      *ringvertptr++ = x1;
      *ringvertptr++ = y1;
      *ringvertptr++ = z1;
      *ringvertptr++ = x2;
      *ringvertptr++ = y2;
      *ringvertptr++ = z2;
      *ringvertptr++ = x3;
      *ringvertptr++ = y3;
      *ringvertptr++ = z3;
      *ringvertptr++ = x4;
      *ringvertptr++ = y4;
      *ringvertptr++ = z4;
      glDrawArrays (GL_TRIANGLE_FAN, 0, 4);
      x1 = x3;
      y1 = y3;
      z1 = z3;
      x2 = x4;
      y2 = y4;
      z2 = z4;

   }
   glPopMatrix ();
}

//senquack - 2/11 experiment:
//void drawCore(GLfloat x, GLfloat y, int cnt, int r, int g, int b) {
//  int i;
//  float cy;
//  glPushMatrix();
//  glTranslatef(x, y, 0);
//  glColor4i(r, g, b, 255);
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(-SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L,  0);
//  glVertex3f( SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L,  0);
//  glVertex3f( SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L,  0);
//  glVertex3f(-SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L,  0);
//  glEnd();
//  glPopMatrix();
//  cy = y - CORE_HEIGHT*2.5f;
//  for ( i=0 ; i<4 ; i++, cy+=CORE_HEIGHT ) {
//    drawRing(x, cy, (cnt*(4+i))&1023, (sctbl[(cnt*(5+i))&1023]/4)&1023, r, g, b);
//  }
//}
//void drawCore(GLfloat x, GLfloat y, int cnt, int r, int g, int b) {
//  int i;
//  float cy;
//  glPushMatrix();
//  glTranslatef(x, y, 0);
//  glColor4i(r, g, b, 255);
//
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex3f(-SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L,  0);
////  glVertex3f( SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L,  0);
////  glVertex3f( SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L,  0);
////  glVertex3f(-SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L,  0);
////  glEnd();
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex2f(-SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L);
//  glVertex2f( SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L);
//  glVertex2f( SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L);
//  glVertex2f(-SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L);
//  glEnd();
//
//  glPopMatrix();
//  cy = y - CORE_HEIGHT*2.5f;
//  for ( i=0 ; i<4 ; i++, cy+=CORE_HEIGHT ) {
//    drawRing(x, cy, (cnt*(4+i))&1023, (sctbl[(cnt*(5+i))&1023]/4)&1023, r, g, b);
//  }
//}
void
drawCorex (GLfixed fx, GLfixed fy, int cnt, int r, int g, int b)
{
   int i;
//  float cy;
   GLfixed fcy;
   glPushMatrix ();
   glTranslatex (fx, fy, 0);
//  glColor4i(r, g, b, 255);

   GLubyte colors[4 * 4];
   GLfixed vertices[4 * 2];
   colors[0] = colors[4] = colors[8] = colors[12] = r;
   colors[1] = colors[5] = colors[9] = colors[13] = g;
   colors[2] = colors[6] = colors[10] = colors[14] = b;
   colors[3] = colors[7] = colors[11] = colors[15] = 255;

   vertices[0] = -SHAPE_POINT_SIZE_L_X;
   vertices[1] = -SHAPE_POINT_SIZE_L_X;
   vertices[2] = SHAPE_POINT_SIZE_L_X;
   vertices[3] = -SHAPE_POINT_SIZE_L_X;
   vertices[4] = SHAPE_POINT_SIZE_L_X;
   vertices[5] = SHAPE_POINT_SIZE_L_X;
   vertices[6] = -SHAPE_POINT_SIZE_L_X;
   vertices[7] = SHAPE_POINT_SIZE_L_X;

// glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer (2, GL_FIXED, 0, vertices);
// glEnableClientState(GL_COLOR_ARRAY);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
   glDrawArrays (GL_TRIANGLE_FAN, 0, 4);

   glPopMatrix ();

//  cy = y - CORE_HEIGHT*2.5f;
   fcy = fy - FMUL (CORE_HEIGHT_X, 163840);

   prepareDrawRingx ();
//  for ( i=0 ; i<4 ; i++, cy+=CORE_HEIGHT ) {
   for (i = 0; i < 4; i++, fcy += CORE_HEIGHT_X) {
//    drawRing(x, cy, (cnt*(4+i))&1023, (sctbl[(cnt*(5+i))&1023]/4)&1023, r, g, b);
//    drawRingx(fx, fcy, (cnt*(4+i))&1023, (sctbl[(cnt*(5+i))&1023]/4)&1023, r, g, b);
      drawRingx (fx, fcy, (cnt * (4 + i)) & 1023,
                 (sctbl[(cnt * (5 + i)) & 1023] >> 2) & 1023, r, g, b);
   }
}

#define SHIP_DRUM_R 0.4f
#define SHIP_DRUM_R_X 26214     //fixed point version -senquack
#define SHIP_DRUM_WIDTH 0.05f
#define SHIP_DRUM_WIDTH_X 3277  //fixed point version -senquack
#define SHIP_DRUM_HEIGHT 0.35f
#define SHIP_DRUM_HEIGHT_X 22938    //fixed point version -senquack

//senquack - one of the causes of freezing is the drawing of the rotating "drum" in this function
//             and the way I found to fix it is changing GL_LINE_LOOP to GL_QUADS:
//void drawShipShape(GLfloat x, GLfloat y, float d, int inv) {
//  int i;
//  glPushMatrix();
//  glTranslatef(x, y, 0);
//  glColor4i(255, 100, 100, 255);
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(-SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L,  0);
//  glVertex3f( SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L,  0);
//  glVertex3f( SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L,  0);
//  glVertex3f(-SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L,  0);
//  glEnd();
//  if ( inv ) {
//    glPopMatrix();
//    return;
//  }
//  glRotatef(d, 0, 1, 0);
//    glColor4i(120, 220, 100, 150);
//    /*if ( mode == IKA_MODE ) {
//    glColor4i(180, 200, 160, 150);
//  } else {
//    glColor4i(120, 220, 100, 150);
//    }*/
//  for ( i=0 ; i<8 ; i++ ) {
//    glRotatef(45, 0, 1, 0);
//    glBegin(GL_LINE_LOOP);
//    glVertex3f(-SHIP_DRUM_WIDTH, -SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
//    glVertex3f( SHIP_DRUM_WIDTH, -SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
//    glVertex3f( SHIP_DRUM_WIDTH,  SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
//    glVertex3f(-SHIP_DRUM_WIDTH,  SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
//    glEnd();
//  }
//  glPopMatrix();
//}
//senquack - 2nd try:
//void drawShipShape(GLfloat x, GLfloat y, float d, int inv) {
//  int i;
//  glPushMatrix();
//  glTranslatef(x, y, 0);
//  glColor4i(255, 100, 100, 255);
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(-SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L,  0);
//  glVertex3f( SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L,  0);
//  glVertex3f( SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L,  0);
//  glVertex3f(-SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L,  0);
//  glEnd();
//  if ( inv ) {
//    glPopMatrix();
//    return;
//  }
//  glRotatef(d, 0, 1, 0);
//    glColor4i(120, 220, 100, 150);
//    /*if ( mode == IKA_MODE ) {
//    glColor4i(180, 200, 160, 150);
//  } else {
//    glColor4i(120, 220, 100, 150);
//    }*/
//  for ( i=0 ; i<8 ; i++ ) {
//    glRotatef(45, 0, 1, 0);
//  //senquack - freezing fix:
////    glBegin(GL_LINE_LOOP);
//    glBegin(GL_QUADS);
//    glVertex3f(-SHIP_DRUM_WIDTH, -SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
//    glVertex3f( SHIP_DRUM_WIDTH, -SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
//    glVertex3f( SHIP_DRUM_WIDTH,  SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
//    glVertex3f(-SHIP_DRUM_WIDTH,  SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
//    glEnd();
//  }
//  glPopMatrix();
//}
//senquack - 3rd try:
//void drawShipShape(GLfloat x, GLfloat y, float d, int inv) {
//  int i;
//  glPushMatrix();
//  glTranslatef(x, y, 0);
//  glColor4i(255, 100, 100, 255);
//
//  //senquack
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex3f(-SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L,  0);
////  glVertex3f( SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L,  0);
////  glVertex3f( SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L,  0);
////  glVertex3f(-SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L,  0);
////  glEnd();
////  glBegin(GL_TRIANGLES);
////  glVertex3f(-SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L,  0);
////  glVertex3f( SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L,  0);
////  glVertex3f( SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L,  0);
////  glEnd();
////
////  glBegin(GL_TRIANGLES);
////  glVertex3f(-SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L,  0);
////  glVertex3f( SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L,  0);
////  glVertex3f(-SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L,  0);
////  glEnd();
//  glBegin(GL_QUADS);
//  glVertex2f(-SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L);
//  glVertex2f( SHAPE_POINT_SIZE_L, -SHAPE_POINT_SIZE_L);
//  glVertex2f( SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L);
//  glVertex2f(-SHAPE_POINT_SIZE_L,  SHAPE_POINT_SIZE_L);
//  glEnd();
//
//  if ( inv ) {
//    glPopMatrix();
//    return;
//  }
//  glRotatef(d, 0, 1, 0);
//    glColor4i(120, 220, 100, 150);
//    /*if ( mode == IKA_MODE ) {
//    glColor4i(180, 200, 160, 150);
//  } else {
//    glColor4i(120, 220, 100, 150);
//    }*/
//  for ( i=0 ; i<8 ; i++ ) {
//    glRotatef(45, 0, 1, 0);
//  //senquack - freezing fix:
////    glBegin(GL_LINE_LOOP);
////    glBegin(GL_QUADS);
//
//  //senquack - disable for now
////    glBegin(GL_LINES);
////    glVertex3f(-SHIP_DRUM_WIDTH, -SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
////    glVertex3f( SHIP_DRUM_WIDTH, -SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
////  glEnd();
////
////  glBegin(GL_LINES);
////    glVertex3f( SHIP_DRUM_WIDTH, -SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
////    glVertex3f( SHIP_DRUM_WIDTH,  SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
////  glEnd();
////
////  glBegin(GL_LINES);
////    glVertex3f( SHIP_DRUM_WIDTH,  SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
////    glVertex3f(-SHIP_DRUM_WIDTH,  SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
////  glEnd();
////
////  glBegin(GL_LINES);
////    glVertex3f(-SHIP_DRUM_WIDTH,  SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
////    glVertex3f(-SHIP_DRUM_WIDTH, -SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
////    glEnd();
//    glBegin(GL_QUADS);
//    glVertex3f(-SHIP_DRUM_WIDTH, -SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
//    glVertex3f( SHIP_DRUM_WIDTH, -SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
//    glVertex3f( SHIP_DRUM_WIDTH,  SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
//    glVertex3f(-SHIP_DRUM_WIDTH,  SHIP_DRUM_HEIGHT, SHIP_DRUM_R);
//    glEnd();
//  }
//  glPopMatrix();
//}
//senquack - fixed point version of original:
//void drawShipShapex(GLfixed x, GLfixed y, GLfixed d, int inv) {
//  int i;
//  glPushMatrix();
//  glTranslatex(x, y, 0);
//
//    GLubyte colors[4*4]; 
// GLfixed vertices[4*3];
// colors[0] = colors[4] = colors[8] = colors[12] = 255;
// colors[1] = colors[5] = colors[9] = colors[13] = 
// colors[2] = colors[6] = colors[10] = colors[14] = 100;
// colors[3] = colors[7] = colors[11] = colors[15] = 255;
//
// vertices[0] = -SHAPE_POINT_SIZE_L_X;   vertices[1] = -SHAPE_POINT_SIZE_L_X;   vertices[2] = 0;
// vertices[3] = SHAPE_POINT_SIZE_L_X; vertices[4] = -SHAPE_POINT_SIZE_L_X;   vertices[5] = 0;
// vertices[6] = SHAPE_POINT_SIZE_L_X; vertices[7] = SHAPE_POINT_SIZE_L_X;    vertices[8] = 0;
// vertices[9] = -SHAPE_POINT_SIZE_L_X;   vertices[10] = SHAPE_POINT_SIZE_L_X;   vertices[11] = 0;
//
////  glEnableClientState(GL_VERTEX_ARRAY);
// glVertexPointer(3, GL_FIXED, 0, vertices);
////  glEnableClientState(GL_COLOR_ARRAY);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//
//  if ( inv ) {
//    glPopMatrix();
//    return;
//  }
//
// glRotatex(d, 0, INT2FNUM(1), 0);
//
////    glColor4i(120, 220, 100, 150);
//
////  for ( i=0 ; i<8 ; i++ ) {
//  for ( i=8 ; i>0 ; i-- ) {
// glRotatex(INT2FNUM(45), 0, INT2FNUM(1), 0);
// colors[0] = colors[4] = colors[8] = colors[12] = 120;
// colors[1] = colors[5] = colors[9] = colors[13] = 220;
// // shouldn't be necessary (already set):
////  colors[2] = colors[6] = colors[10] = colors[14] = 100;
// colors[3] = colors[7] = colors[11] = colors[15] = 150;
//
// vertices[0] = -SHIP_DRUM_WIDTH_X;   vertices[1] = -SHIP_DRUM_HEIGHT_X;  vertices[2] = SHIP_DRUM_R_X;
// vertices[3] = SHIP_DRUM_WIDTH_X; vertices[4] = -SHIP_DRUM_HEIGHT_X;  vertices[5] = SHIP_DRUM_R_X;
// vertices[6] = SHIP_DRUM_WIDTH_X; vertices[7] = SHIP_DRUM_HEIGHT_X;   vertices[8] = SHIP_DRUM_R_X;
// vertices[9] = -SHIP_DRUM_WIDTH_X;   vertices[10] = SHIP_DRUM_HEIGHT_X;  vertices[11] = SHIP_DRUM_R_X;
// glDrawArrays(GL_LINE_LOOP, 0, 4);
//  }
//  glPopMatrix();
//}
void
drawShipShapex (GLfixed x, GLfixed y, float d, int inv)
{
   int i;
   glPushMatrix ();
   glTranslatex (x, y, 0);

   GLubyte colors[4 * 4];
   GLfixed vertices[4 * 3];
   colors[0] = colors[4] = colors[8] = colors[12] = 255;
   colors[1] = colors[5] = colors[9] = colors[13] =
      colors[2] = colors[6] = colors[10] = colors[14] = 100;
   colors[3] = colors[7] = colors[11] = colors[15] = 255;

   vertices[0] = -SHAPE_POINT_SIZE_L_X;
   vertices[1] = -SHAPE_POINT_SIZE_L_X;
   vertices[2] = 0;
   vertices[3] = SHAPE_POINT_SIZE_L_X;
   vertices[4] = -SHAPE_POINT_SIZE_L_X;
   vertices[5] = 0;
   vertices[6] = SHAPE_POINT_SIZE_L_X;
   vertices[7] = SHAPE_POINT_SIZE_L_X;
   vertices[8] = 0;
   vertices[9] = -SHAPE_POINT_SIZE_L_X;
   vertices[10] = SHAPE_POINT_SIZE_L_X;
   vertices[11] = 0;

// glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer (3, GL_FIXED, 0, vertices);
// glEnableClientState(GL_COLOR_ARRAY);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
   glDrawArrays (GL_TRIANGLE_FAN, 0, 4);

   if (inv) {
      glPopMatrix ();
      return;
   }
// glRotatex(d, 0, INT2FNUM(1), 0);
   glRotatef (d, 0, 1, 0);

//    glColor4i(120, 220, 100, 150);

   //senquack - moved these outside the loop
   colors[0] = colors[4] = colors[8] = colors[12] = 120;
   colors[1] = colors[5] = colors[9] = colors[13] = 220;
   // shouldn't be necessary (already set):
// colors[2] = colors[6] = colors[10] = colors[14] = 100;
   colors[3] = colors[7] = colors[11] = colors[15] = 150;

   vertices[0] = -SHIP_DRUM_WIDTH_X;
   vertices[1] = -SHIP_DRUM_HEIGHT_X;
   vertices[2] = SHIP_DRUM_R_X;
   vertices[3] = SHIP_DRUM_WIDTH_X;
   vertices[4] = -SHIP_DRUM_HEIGHT_X;
   vertices[5] = SHIP_DRUM_R_X;
   vertices[6] = SHIP_DRUM_WIDTH_X;
   vertices[7] = SHIP_DRUM_HEIGHT_X;
   vertices[8] = SHIP_DRUM_R_X;
   vertices[9] = -SHIP_DRUM_WIDTH_X;
   vertices[10] = SHIP_DRUM_HEIGHT_X;
   vertices[11] = SHIP_DRUM_R_X;

//  for ( i=0 ; i<8 ; i++ ) {
   for (i = 8; i > 0; i--) {
// glRotatex(INT2FNUM(45), 0, INT2FNUM(1), 0);
      glRotatef (45, 0, 1, 0);
      glDrawArrays (GL_LINE_LOOP, 0, 4);
   }
   glPopMatrix ();
}

//senquack - converting to line strip for speed
//senquack - converted to fixed point
//void drawBomb(GLfloat x, GLfloat y, GLfloat width, int cnt) {
//  int i, d, od, c;
//  GLfloat x1, y1, x2, y2;
//  d = cnt*48; d &= 1023;
//  c = 4+(cnt>>3); if ( c > 16 ) c = 16;
//  od = 1024/c;
//  x1 = (sctbl[d]    *width)/256 + x;
//  y1 = (sctbl[d+256]*width)/256 + y;
//  for ( i=0 ; i<c ; i++ ) {
//    d += od; d &= 1023;
//    x2 = (sctbl[d]    *width)/256 + x;
//    y2 = (sctbl[d+256]*width)/256 + y;
//    drawLine(x1, y1, 0, x2, y2, 0, 255, 255, 255, 255);
//    x1 = x2; y1 = y2;
//  }
//}
//void drawBombx(GLfixed x, GLfixed y, GLfixed width, int cnt) {
//  int i, d, od, c;
////  GLfloat x1, y1, x2, y2;
//  GLfixed x1, y1, x2, y2;
//  d = cnt*48; d &= 1023;
//  c = 4+(cnt>>3); if ( c > 16 ) c = 16;
//  od = 1024/c;
////  x1 = (sctbl[d]    *width)/256 + x;
////  y1 = (sctbl[d+256]*width)/256 + y;
//  x1 = (FMUL(INT2FNUM(sctbl[d]), width)>>8) + x;
//  y1 = (FMUL(INT2FNUM(sctbl[d+256]),width)>>8) + y;
//  for ( i=0 ; i<c ; i++ ) {
//    d += od; d &= 1023;
////    x2 = (sctbl[d]    *width)/256 + x;
////    y2 = (sctbl[d+256]*width)/256 + y;
//    x2 = (FMUL(INT2FNUM(sctbl[d]),width)>>8) + x;
//    y2 = (FMUL(INT2FNUM(sctbl[d+256]),width)>>8) + y;
////    drawLine(x1, y1, 0, x2, y2, 0, 255, 255, 255, 255);
//    drawLinex(x1, y1, 0, x2, y2, 0, 255, 255, 255, 255);
//    x1 = x2; y1 = y2;
//  }
//}
//senquack - moving drawing directly into the function for speed
//void drawBombx(GLfixed x, GLfixed y, GLfixed width, int cnt) {
//  int i, d, od, c;
////  GLfloat x1, y1, x2, y2;
//  GLfixed x1, y1, x2, y2;
//  d = cnt*48; d &= 1023;
//  c = 4+(cnt>>3); if ( c > 16 ) c = 16;
//  od = 1024/c;
////  x1 = (sctbl[d]    *width)/256 + x;
////  y1 = (sctbl[d+256]*width)/256 + y;
//  x1 = (FMUL(INT2FNUM(sctbl[d]), width)>>8) + x;
//  y1 = (FMUL(INT2FNUM(sctbl[d+256]),width)>>8) + y;
//  for ( i=0 ; i<c ; i++ ) {
//    d += od; d &= 1023;
////    x2 = (sctbl[d]    *width)/256 + x;
////    y2 = (sctbl[d+256]*width)/256 + y;
//    x2 = (FMUL(INT2FNUM(sctbl[d]),width)>>8) + x;
//    y2 = (FMUL(INT2FNUM(sctbl[d+256]),width)>>8) + y;
////    drawLine(x1, y1, 0, x2, y2, 0, 255, 255, 255, 255);
////    drawLinex(x1, y1, 0, x2, y2, 0, 255, 255, 255, 255);
//    drawLinex(x1, y1, x2, y2, 255, 255, 255, 255);
//    x1 = x2; y1 = y2;
//  }
//}
void
drawBombx (GLfixed x, GLfixed y, GLfixed width, int cnt)
{
   int i, d, od, c;
//  GLfloat x1, y1, x2, y2;
   GLfixed x1, y1, x2, y2;
   GLfixed vertices[2 * 17];
   GLfixed *vertptr = &(vertices[0]);
   GLubyte colors[4 * 17];
   glVertexPointer (2, GL_FIXED, 0, vertices);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
   memset (&(colors[0]), 255, 4 * 17);

   d = cnt * 48;
   d &= 1023;
   c = 4 + (cnt >> 3);
   if (c > 16)
      c = 16;
   od = 1024 / c;
//  x1 = (sctbl[d]    *width)/256 + x;
//  y1 = (sctbl[d+256]*width)/256 + y;
   x1 = (FMUL (INT2FNUM (sctbl[d]), width) >> 8) + x;
   y1 = (FMUL (INT2FNUM (sctbl[d + 256]), width) >> 8) + y;
   *vertptr++ = x1;
   *vertptr++ = y1;
   for (i = 0; i < c; i++) {
      d += od;
      d &= 1023;
//    x2 = (sctbl[d]    *width)/256 + x;
//    y2 = (sctbl[d+256]*width)/256 + y;
      x2 = (FMUL (INT2FNUM (sctbl[d]), width) >> 8) + x;
      y2 = (FMUL (INT2FNUM (sctbl[d + 256]), width) >> 8) + y;
//    drawLine(x1, y1, 0, x2, y2, 0, 255, 255, 255, 255);
//    drawLinex(x1, y1, 0, x2, y2, 0, 255, 255, 255, 255);
//    drawLinex(x1, y1, x2, y2, 255, 255, 255, 255);
      *vertptr++ = x2;
      *vertptr++ = y2;
//    x1 = x2; y1 = y2;
   }
   glDrawArrays (GL_LINE_STRIP, 0, 1 + c);
}

//senquack - FURTHER NOTE: this "circle" is jittery and obviously buggy somehow
//senquack - NO IT STILL FREEZES WITHOUT THIS BUT IT TAKES A LOONG TIME
//senquack - this might be what is causing freezes in IKA mode:
//NOTE - still freezes but with this enabled the circle seems jerky and something is wrong
//void drawCircle(GLfloat x, GLfloat y, GLfloat width, int cnt,
//    int r1, int g1, int b1, int r2, int b2, int g2) {
//  int i, d;
//  GLfloat x1, y1, x2, y2;
//  if ( (cnt&1) == 0 ) {
//    glColor4i(r1, g1, b1, 64);
//  } else {
//    glColor4i(255, 255, 255, 64);
//  }
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(x, y, 0);
//  d = cnt*48; d &= 1023;
//  x1 = (sctbl[d]    *width)/256 + x;
//  y1 = (sctbl[d+256]*width)/256 + y;
//  glColor4i(r2, g2, b2, 150);
//  for ( i=0 ; i<16 ; i++ ) {
//    d += 64; d &= 1023;
//    x2 = (sctbl[d]    *width)/256 + x;
//    y2 = (sctbl[d+256]*width)/256 + y;
//    glVertex3f(x1, y1, 0);
//    glVertex3f(x2, y2, 0);
//    x1 = x2; y1 = y2;
//  }
//  glEnd();
//}
//senquack - converting to 2D below:
//void drawCircle(GLfloat x, GLfloat y, GLfloat width, int cnt,
//    int r1, int g1, int b1, int r2, int b2, int g2) {
//  int i, d;
//  GLfloat x1, y1, x2, y2;
//  if ( (cnt&1) == 0 ) {
//    glColor4i(r1, g1, b1, 64);
//  } else {
//    glColor4i(255, 255, 255, 64);
//  }
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(x, y, 0);
//  d = cnt*48; d &= 1023;
//  x1 = (sctbl[d]    *width)/256 + x;
//  y1 = (sctbl[d+256]*width)/256 + y;
//  glColor4i(r2, g2, b2, 150);
//  for ( i=0 ; i<16 ; i++ ) {
//    d += 64; d &= 1023;
//    x2 = (sctbl[d]    *width)/256 + x;
//    y2 = (sctbl[d+256]*width)/256 + y;
//    glVertex3f(x1, y1, 0);
//    glVertex3f(x2, y2, 0);
//    x1 = x2; y1 = y2;
//  }
//  glEnd();
//}
//void drawCircle(GLfloat x, GLfloat y, GLfloat width, int cnt,
//    int r1, int g1, int b1, int r2, int b2, int g2) {
//  int i, d;
//  GLfloat x1, y1, x2, y2;
//  if ( (cnt&1) == 0 ) {
//    glColor4i(r1, g1, b1, 64);
//  } else {
//    glColor4i(255, 255, 255, 64);
//  }
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex2f(x, y);
//  d = cnt*48; d &= 1023;
//  x1 = (sctbl[d]    *width)/256 + x;
//  y1 = (sctbl[d+256]*width)/256 + y;
//  glColor4i(r2, g2, b2, 150);
//  for ( i=0 ; i<16 ; i++ ) {
//    d += 64; d &= 1023;
//    x2 = (sctbl[d]    *width)/256 + x;
//    y2 = (sctbl[d+256]*width)/256 + y;
//    glVertex2f(x1, y1);
//    glVertex2f(x2, y2);
//    x1 = x2; y1 = y2;
//  }
//  glEnd();
//}
void
drawCirclex (GLfixed x, GLfixed y, GLfixed width, int cnt,
             int r1, int g1, int b1, int r2, int b2, int g2)
{
   int i, d;
//  GLfloat x1, y1, x2, y2;
   GLfixed x1, y1, x2, y2;

   GLubyte colors[33 * 4];
   GLfixed vertices[33 * 2];

   if ((cnt & 1) == 0) {
//    glColor4i(r1, g1, b1, 64);
      colors[0] = r1;
      colors[1] = g1;
      colors[2] = b1;
   } else {
//    glColor4i(255, 255, 255, 64);
      colors[0] = colors[1] = colors[2] = 64;
   }
   colors[3] = 64;

//  glBegin(GL_TRIANGLE_FAN);
//  glVertex2f(x, y);
   vertices[0] = x;
   vertices[1] = y;
   d = cnt * 48;
   d &= 1023;
//  x1 = (sctbl[d]    *width)/256 + x;
//  y1 = (sctbl[d+256]*width)/256 + y;
   x1 = (FMUL (INT2FNUM (sctbl[d]), width) >> 8) + x;
   y1 = (FMUL (INT2FNUM (sctbl[d + 256]), width) >> 8) + y;
//  glColor4i(r2, g2, b2, 150);
   for (i = 0; i < 16; i++) {
      colors[4 + (i << 3)] = colors[4 + (i << 3) + 4] = r2;
      colors[4 + (i << 3) + 1] = colors[4 + (i << 3) + 5] = g2;
      colors[4 + (i << 3) + 2] = colors[4 + (i << 3) + 6] = b2;
      colors[4 + (i << 3) + 3] = colors[4 + (i << 3) + 7] = 150;

      d += 64;
      d &= 1023;
//    x2 = (sctbl[d]    *width)/256 + x;
//    y2 = (sctbl[d+256]*width)/256 + y;
      x2 = (FMUL (INT2FNUM (sctbl[d]), width) >> 8) + x;
      y2 = (FMUL (INT2FNUM (sctbl[d + 256]), width) >> 8) + y;
//    glVertex2f(x1, y1);
//    glVertex2f(x2, y2);
      vertices[2 + (i << 2)] = x1;
      vertices[2 + (i << 2) + 1] = y1;
      vertices[2 + (i << 2) + 2] = x2;
      vertices[2 + (i << 2) + 3] = y2;
      x1 = x2;
      y1 = y2;
   }
//  glEnd();
// glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer (2, GL_FIXED, 0, vertices);
// glEnableClientState(GL_COLOR_ARRAY);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
   glDrawArrays (GL_TRIANGLE_FAN, 0, 33);
}

//senquack - new - found inconsistencies here.. TRIANGLE_FAN called with only three vertices.. changing 
//void drawShape(GLfloat x, GLfloat y, GLfloat size, int d, int cnt, int type,
//        int r, int g, int b) {
//  GLfloat sz, sz2;
//  glPushMatrix();
//  glTranslatef(x, y, 0);
//  glColor4i(r, g, b, 255);
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
//  glVertex3f( SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
//  glVertex3f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
//  glVertex3f(-SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
//  glEnd();
//  switch ( type ) {
//  case 0:
//    sz = size/2;
//    glRotatef((float)d*360/1024, 0, 0, 1);
//    glDisable(GL_BLEND);
//    glBegin(GL_LINE_LOOP);
//    glVertex3f(-sz, -sz,  0);
//    glVertex3f( sz, -sz,  0);
//    glVertex3f( 0, size,  0);
//    glEnd();
//    glEnable(GL_BLEND);
//    glColor4i(r, g, b, 150);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex3f(-sz, -sz,  0);
//    glVertex3f( sz, -sz,  0);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex3f( 0, size,  0);
//    glEnd();
//    break;
//  case 1:
//    sz = size/2;
//    glRotatef((float)((cnt*23)&1023)*360/1024, 0, 0, 1);
//    glDisable(GL_BLEND);
//    //glBegin(GL_LINE_LOOP); 
//    glBegin(GL_LINES);
//    glVertex3f(  0, -size,  0);
//    glVertex3f( sz,     0,  0);
//    glVertex3f(  0,  size,  0);
//    glVertex3f(-sz,     0,  0);
//    glEnd();
//    glEnable(GL_BLEND);
//    glColor4i(r, g, b, 180);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex3f(  0, -size,  0);
//    glVertex3f( sz,     0,  0);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex3f(  0,  size,  0);
//    glVertex3f(-sz,     0,  0);
//    glEnd();
//    break;
//  case 2:
//    sz = size/4; sz2 = size/3*2;
//    glRotatef((float)d*360/1024, 0, 0, 1);
//    glDisable(GL_BLEND);
//    //glBegin(GL_LINE_LOOP);
//    glBegin(GL_LINES);
//    glVertex3f(-sz, -sz2,  0);
//    glVertex3f( sz, -sz2,  0);
//    glVertex3f( sz,  sz2,  0);
//    glVertex3f(-sz,  sz2,  0);
//    glEnd();
//    glEnable(GL_BLEND);
//    glColor4i(r, g, b, 120);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex3f(-sz, -sz2,  0);
//    glVertex3f( sz, -sz2,  0);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex3f( sz, sz2,  0);
//    glVertex3f(-sz, sz2,  0);
//    glEnd();
//    break;
//  case 3:
//    sz = size/2;
//    glRotatef((float)((cnt*37)&1023)*360/1024, 0, 0, 1);
//    glDisable(GL_BLEND);
//    //glBegin(GL_LINE_LOOP);
//    glBegin(GL_LINES);
//    glVertex3f(-sz, -sz,  0);
//    glVertex3f( sz, -sz,  0);
//    glVertex3f( sz,  sz,  0);
//    glVertex3f(-sz,  sz,  0);
//    glEnd();
//    glEnable(GL_BLEND);
//    glColor4i(r, g, b, 180);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex3f(-sz, -sz,  0);
//    glVertex3f( sz, -sz,  0);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex3f( sz,  sz,  0);
//    glVertex3f(-sz,  sz,  0);
//    glEnd();
//    break;
//  case 4:
//    sz = size/2;
//    glRotatef((float)((cnt*53)&1023)*360/1024, 0, 0, 1);
//    glDisable(GL_BLEND);
//    //glBegin(GL_LINE_LOOP);
//    glBegin(GL_LINES);
//    glVertex3f(-sz/2, -sz,  0);
//    glVertex3f( sz/2, -sz,  0);
//    glVertex3f( sz,  -sz/2,  0);
//    glVertex3f( sz,   sz/2,  0);
//    glVertex3f( sz/2,  sz,  0);
//    glVertex3f(-sz/2,  sz,  0);
//    glVertex3f(-sz,   sz/2,  0);
//    glVertex3f(-sz,  -sz/2,  0);
//    glEnd();
//    glEnable(GL_BLEND);
//    glColor4i(r, g, b, 220);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex3f(-sz/2, -sz,  0);
//    glVertex3f( sz/2, -sz,  0);
//    glVertex3f( sz,  -sz/2,  0);
//    glVertex3f( sz,   sz/2,  0);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex3f( sz/2,  sz,  0);
//    glVertex3f(-sz/2,  sz,  0);
//    glVertex3f(-sz,   sz/2,  0);
//    glVertex3f(-sz,  -sz/2,  0);
//    glEnd();
//    break;
//  case 5:
//    sz = size*2/3; sz2 = size/5;
//    glRotatef((float)d*360/1024, 0, 0, 1);
//    glDisable(GL_BLEND);
//    glBegin(GL_LINE_STRIP);
//    glVertex3f(-sz, -sz+sz2,  0);
//    glVertex3f( 0, sz+sz2,  0);
//    glVertex3f( sz, -sz+sz2,  0);
//    glEnd();
//    glEnable(GL_BLEND);
//    glColor4i(r, g, b, 150);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex3f(-sz, -sz+sz2,  0);
//    glVertex3f( sz, -sz+sz2,  0);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex3f( 0, sz+sz2,  0);
//    glEnd();
//    break;
//  case 6:
//    sz = size/2;
//    glRotatef((float)((cnt*13)&1023)*360/1024, 0, 0, 1);
//    glDisable(GL_BLEND);
//    //glBegin(GL_LINE_LOOP);
//    glBegin(GL_LINES);
//    glVertex3f(-sz, -sz,  0);
//    glVertex3f(  0, -sz,  0);
//    glVertex3f( sz,   0,  0);
//    glVertex3f( sz,  sz,  0);
//    glVertex3f(  0,  sz,  0);
//    glVertex3f(-sz,   0,  0);
//    glEnd();
//    glEnable(GL_BLEND);
//    glColor4i(r, g, b, 210);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex3f(-sz, -sz,  0);
//    glVertex3f(  0, -sz,  0);
//    glVertex3f( sz,   0,  0);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex3f( sz,  sz,  0);
//    glVertex3f(  0,  sz,  0);
//    glVertex3f(-sz,   0,  0);
//    glEnd();
//    break;
//  }
//  glPopMatrix();
//}
//void drawShape(GLfloat x, GLfloat y, GLfloat size, int d, int cnt, int type,
//        int r, int g, int b) {
//  GLfloat sz, sz2;
//  glPushMatrix();
//  glTranslatef(x, y, 0);
//  glColor4i(r, g, b, 255);
//
//  //senquack
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
//  glVertex3f( SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
//  glVertex3f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
//  glVertex3f(-SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
//  glEnd();
//  //senquack - 2nd try:
////  glBegin(GL_TRIANGLES);
////  glVertex3f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
////  glVertex3f( SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
////  glVertex3f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
////  glEnd();
////
////  glBegin(GL_TRIANGLES);
////  glVertex3f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
////  glVertex3f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
////  glVertex3f(-SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
////  glEnd();
//  //senquack - 3rd try:
////  glBegin(GL_QUADS);
////  glVertex2f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE);
////  glVertex2f( SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE);
////  glVertex2f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE);
////  glVertex2f(-SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE);
////  glEnd();
//
//
//  switch ( type ) {
//  case 0:
//    sz = size/2;
//    glRotatef((float)d*360/1024, 0, 0, 1);
////    glDisable(GL_BLEND);
//
//  //senquack - no need for this:
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f( sz, -sz,  0);
////    glVertex3f( 0, size,  0);
////    glVertex3f(-sz, -sz,  0);
////    glEnd();
//
//    glEnable(GL_BLEND);
//    glColor4i(r, g, b, 150);
//  //senquack - here is a inconsistency, changing this line to just GL_TRIANGLES since we have only 3 vertices
////    glBegin(GL_TRIANGLE_FAN);
//  //senaquack - also converting to 2D:
////    glBegin(GL_TRIANGLES);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f( sz, -sz,  0);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f( 0, size,  0);
////    glEnd();
//    glBegin(GL_TRIANGLES);
//    glVertex2f(-sz, -sz);
//    glVertex2f( sz, -sz);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f( 0, size);
//    glEnd();
//    break;
//  case 1:
//    sz = size/2;
//    glRotatef((float)((cnt*23)&1023)*360/1024, 0, 0, 1);
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP); 
////    glVertex2f(  0, -size);
////    glVertex2f( sz,     0);
////    glVertex2f(  0,  size);
////    glVertex2f(-sz,     0);
////    glVertex2f(  0, -size);
////    glEnd();
////    glEnable(GL_BLEND);
//    glColor4i(r, g, b, 180);
//
// //senquack - converting to 2D
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(  0, -size,  0);
////    glVertex3f( sz,     0,  0);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f(  0,  size,  0);
////    glVertex3f(-sz,     0,  0);
////    glEnd();
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(  0, -size);
//    glVertex2f( sz,     0);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f(  0,  size);
//    glVertex2f(-sz,     0);
//    glEnd();
//
//    break;
//  case 2:
//    sz = size/4; sz2 = size/3*2;
//    glRotatef((float)d*360/1024, 0, 0, 1);
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex2f(-sz, -sz2);
////    glVertex2f( sz, -sz2);
////    glVertex2f( sz,  sz2);
////    glVertex2f(-sz,  sz2);
////    glVertex2f(-sz, -sz2);
////    glEnd();
////    glEnable(GL_BLEND);
//
//    glColor4i(r, g, b, 120);
//
//  //senquack - converting to 2D
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz, -sz2,  0);
////    glVertex3f( sz, -sz2,  0);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f( sz, sz2,  0);
////    glVertex3f(-sz, sz2,  0);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz, -sz2);
//    glVertex2f( sz, -sz2);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f( sz, sz2);
//    glVertex2f(-sz, sz2);
//    break;
//  case 3:
//    sz = size/2;
//    glRotatef((float)((cnt*37)&1023)*360/1024, 0, 0, 1);
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex2f(-sz, -sz);
////    glVertex2f( sz, -sz);
////    glVertex2f( sz,  sz);
////    glVertex2f(-sz,  sz);
////    glVertex2f(-sz, -sz);
////    glEnd();
////    glEnable(GL_BLEND);
//
//    glColor4i(r, g, b, 180);
//
//  //senquack - converting to 2D:
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f( sz, -sz,  0);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f( sz,  sz,  0);
////    glVertex3f(-sz,  sz,  0);
////    glEnd();
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz, -sz);
//    glVertex2f( sz, -sz);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f( sz,  sz);
//    glVertex2f(-sz,  sz);
//    glEnd();
//    break;
//  case 4:
//    sz = size/2;
//    glRotatef((float)((cnt*53)&1023)*360/1024, 0, 0, 1);
//
//  //senquack - no need for this
////    glBegin(GL_LINES);
////    glVertex3f(-sz/2, -sz,  0);
////    glVertex3f( sz/2, -sz,  0);
////    glVertex3f( sz,  -sz/2,  0);
////    glVertex3f( sz,   sz/2,  0);
////    glVertex3f( sz/2,  sz,  0);
////    glVertex3f(-sz/2,  sz,  0);
////    glVertex3f(-sz,   sz/2,  0);
////    glVertex3f(-sz,  -sz/2,  0);
//
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz/2, -sz,  0);
////    glVertex3f( sz/2, -sz,  0);
////    glVertex3f( sz,  -sz/2,  0);
////    glVertex3f( sz,   sz/2,  0);
////    glVertex3f( sz/2,  sz,  0);
////    glVertex3f(-sz/2,  sz,  0);
////    glVertex3f(-sz,   sz/2,  0);
////    glVertex3f(-sz,  -sz/2,  0);
////    glVertex3f(-sz/2, -sz,  0);
////   glEnd();
////    glEnable(GL_BLEND);
//
//
//    glColor4i(r, g, b, 220);
//
//  //senquack - converting to 2D
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz/2, -sz,  0);
////    glVertex3f( sz/2, -sz,  0);
////    glVertex3f( sz,  -sz/2,  0);
////    glVertex3f( sz,   sz/2,  0);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f( sz/2,  sz,  0);
////    glVertex3f(-sz/2,  sz,  0);
////    glVertex3f(-sz,   sz/2,  0);
////    glVertex3f(-sz,  -sz/2,  0);
////    glEnd();
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz/2, -sz);
//    glVertex2f( sz/2, -sz);
//    glVertex2f( sz,  -sz/2);
//    glVertex2f( sz,   sz/2);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f( sz/2,  sz);
//    glVertex2f(-sz/2,  sz);
//    glVertex2f(-sz,   sz/2);
//    glVertex2f(-sz,  -sz/2);
//    glEnd();
//
//
////    glBegin(GL_TRIANGLES);
////    glVertex3f(-sz/2, -sz,  0);
////    glVertex3f( sz/2, -sz,  0);
////    glVertex3f( sz,  -sz/2,  0);
////    glEnd();
////
////  glBegin(GL_TRIANGLES);
////    glVertex3f(-sz/2, -sz,  0);
////    glVertex3f( sz,  -sz/2,  0);
////    glVertex3f( sz,   sz/2,  0);
////  glEnd();
////
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////  glBegin(GL_TRIANGLES);
////    glVertex3f(-sz/2, -sz,  0);
////    glVertex3f( sz,   sz/2,  0);
////    glVertex3f( sz/2,  sz,  0);
////  glEnd();
////
////  glBegin(GL_TRIANGLES);
////    glVertex3f(-sz/2, -sz,  0);
////    glVertex3f( sz/2,  sz,  0);
////    glVertex3f(-sz/2,  sz,  0);
////  glEnd();
////
////  glBegin(GL_TRIANGLES);
////    glVertex3f(-sz/2, -sz,  0);
////    glVertex3f(-sz/2,  sz,  0);
////    glVertex3f(-sz,   sz/2,  0);
////  glEnd();
////
////  glBegin(GL_TRIANGLES);
////    glVertex3f(-sz/2, -sz,  0);
////    glVertex3f(-sz,   sz/2,  0);
////    glVertex3f(-sz,  -sz/2,  0);
////    glEnd();
//    break;
//  case 5:
//    sz = size*2/3; sz2 = size/5;
//    glRotatef((float)d*360/1024, 0, 0, 1);
//// 2/11 - new efforts to convert all triangle fans and line loops to something else:
//  //senquack
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_STRIP);
////    glVertex3f(-sz, -sz+sz2,  0);
////    glVertex3f( 0, sz+sz2,  0);
////    glVertex3f( sz, -sz+sz2,  0);
////    glEnd();
//
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex2f(-sz, -sz+sz2);
////    glVertex2f( 0, sz+sz2);
////    glVertex2f( sz, -sz+sz2);
////    glVertex2f(-sz, -sz+sz2);
////    glEnd();
//
////    glEnable(GL_BLEND);
//    glColor4i(r, g, b, 150);
//  //senquack - here is a inconsistency, changing this line to just GL_TRIANGLES since we have only 3 vertices
////    glBegin(GL_TRIANGLE_FAN);
//
//  //senquack - converting to 2D
////    glBegin(GL_TRIANGLES);
////    glVertex3f(-sz, -sz+sz2,  0);
////    glVertex3f( sz, -sz+sz2,  0);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f( 0, sz+sz2,  0);
////    glEnd();
//    glBegin(GL_TRIANGLES);
//    glVertex2f(-sz, -sz+sz2);
//    glVertex2f( sz, -sz+sz2);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f( 0, sz+sz2);
//    glEnd();
//    break;
//  case 6:
//    sz = size/2;
//    glRotatef((float)((cnt*13)&1023)*360/1024, 0, 0, 1);
//
//  //senquack - no need for this:
////    glDisable(GL_BLEND);
////
////    glBegin(GL_LINE_LOOP);
////    glVertex2f(-sz, -sz);
////    glVertex2f(  0, -sz);
////    glVertex2f( sz,   0);
////    glVertex2f( sz,  sz);
////    glVertex2f(  0,  sz);
////    glVertex2f(-sz,   0);
////    glEnd();
////
////    glEnable(GL_BLEND);
//
//    glColor4i(r, g, b, 210);
//
//  //senquack - converting to 2D
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f(  0, -sz,  0);
////    glVertex3f( sz,   0,  0);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f( sz,  sz,  0);
////    glVertex3f(  0,  sz,  0);
////    glVertex3f(-sz,   0,  0);
////    glEnd();
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz, -sz);
//    glVertex2f(  0, -sz);
//    glVertex2f( sz,   0);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f( sz,  sz);
//    glVertex2f(  0,  sz);
//    glVertex2f(-sz,   0);
//    glEnd();
//
////    glBegin(GL_TRIANGLES);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f(  0, -sz,  0);
////    glVertex3f( sz,   0,  0);
////  glEnd();
////
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////  glBegin(GL_TRIANGLES);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f( sz,   0,  0);
////    glVertex3f( sz,  sz,  0);
////  glEnd();
////
////  glBegin(GL_TRIANGLES);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f( sz,  sz,  0);
////    glVertex3f(  0,  sz,  0);
////  glEnd();
////
////  glBegin(GL_TRIANGLES);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f(  0,  sz,  0);
////    glVertex3f(-sz,   0,  0);
////    glEnd();
////    break;
//  }
//  glPopMatrix();
//}
//senquack - converting to VBOs and OpenGLES for Wiz:
//void drawShape(GLfloat x, GLfloat y, GLfloat size, int d, int cnt, int type,
//        int r, int g, int b) {
//  GLfloat sz, sz2;
//  glPushMatrix();
//  glTranslatef(x, y, 0);
////  glColor4i(r, g, b, 255);
//  glColor4i(r, g, b, 255);
//
//  //senquack - converting to 2D for speed:
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex3f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
////  glVertex3f( SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
////  glVertex3f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
////  glVertex3f(-SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
////  glEnd();
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex2f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE);
//  glVertex2f( SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE);
//  glVertex2f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE);
//  glVertex2f(-SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE);
//  glEnd();
//
//  switch ( type ) {
//  case 0:
//    sz = size/2;
//    glRotatef((float)d*360/1024, 0, 0, 1);
//
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f( sz, -sz,  0);
////    glVertex3f( 0, size,  0);
////    glEnd();
//    glEnable(GL_BLEND);
//
////    glColor4i(r, g, b, 150);
//    glColor4i(r, g, b, 150);
//
//  //senquack - converting to 2D for speed:
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f( sz, -sz,  0);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f( 0, size,  0);
////    glEnd();
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz, -sz);
//    glVertex2f( sz, -sz);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f( 0, size);
//    glEnd();
//    break;
//  case 1:
//    sz = size/2;
//    glRotatef((float)((cnt*23)&1023)*360/1024, 0, 0, 1);
//
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(  0, -size,  0);
////    glVertex3f( sz,     0,  0);
////    glVertex3f(  0,  size,  0);
////    glVertex3f(-sz,     0,  0);
////    glEnd();
//    glEnable(GL_BLEND);
//
////    glColor4i(r, g, b, 180);
//    glColor4i(r, g, b, 180);
//
//  //senquack - converting to 2D for speed:
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(  0, -size,  0);
////    glVertex3f( sz,     0,  0);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f(  0,  size,  0);
////    glVertex3f(-sz,     0,  0);
////    glEnd();
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(  0, -size);
//    glVertex2f( sz,     0);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f(  0,  size);
//    glVertex2f(-sz,     0);
//    glEnd();
//    break;
//  case 2:
//    sz = size/4; sz2 = size/3*2;
//    glRotatef((float)d*360/1024, 0, 0, 1);
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz, -sz2,  0);
////    glVertex3f( sz, -sz2,  0);
////    glVertex3f( sz,  sz2,  0);
////    glVertex3f(-sz,  sz2,  0);
////    glEnd();
//
//  //senquack - converting to 2D for speed:
////    glEnable(GL_BLEND);
//////    glColor4i(r, g, b, 120);
////    glColor4i(r, g, b, 120);
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz, -sz2,  0);
////    glVertex3f( sz, -sz2,  0);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f( sz, sz2,  0);
////    glVertex3f(-sz, sz2,  0);
////    glEnd();
//    glEnable(GL_BLEND);
////    glColor4i(r, g, b, 120);
//    glColor4i(r, g, b, 120);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz, -sz2);
//    glVertex2f( sz, -sz2);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f( sz, sz2);
//    glVertex2f(-sz, sz2);
//    glEnd();
//    break;
//  case 3:
//    sz = size/2;
//    glRotatef((float)((cnt*37)&1023)*360/1024, 0, 0, 1);
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f( sz, -sz,  0);
////    glVertex3f( sz,  sz,  0);
////    glVertex3f(-sz,  sz,  0);
////    glEnd();
//
//  //senquack - converting to 2D for speed:
////    glEnable(GL_BLEND);
//////    glColor4i(r, g, b, 180);
////    glColor4i(r, g, b, 180);
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f( sz, -sz,  0);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f( sz,  sz,  0);
////    glVertex3f(-sz,  sz,  0);
////    glEnd();
//    glEnable(GL_BLEND);
////    glColor4i(r, g, b, 180);
//    glColor4i(r, g, b, 180);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz, -sz);
//    glVertex2f( sz, -sz);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f( sz,  sz);
//    glVertex2f(-sz,  sz);
//    glEnd();
//    break;
//  case 4:
//    sz = size/2;
//    glRotatef((float)((cnt*53)&1023)*360/1024, 0, 0, 1);
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz/2, -sz,  0);
////    glVertex3f( sz/2, -sz,  0);
////    glVertex3f( sz,  -sz/2,  0);
////    glVertex3f( sz,   sz/2,  0);
////    glVertex3f( sz/2,  sz,  0);
////    glVertex3f(-sz/2,  sz,  0);
////    glVertex3f(-sz,   sz/2,  0);
////    glVertex3f(-sz,  -sz/2,  0);
////    glEnd();
//    glEnable(GL_BLEND);
////    glColor4i(r, g, b, 220);
//    glColor4i(r, g, b, 220);
//
//  //senquack - converting to 2D for speed:
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz/2, -sz,  0);
////    glVertex3f( sz/2, -sz,  0);
////    glVertex3f( sz,  -sz/2,  0);
////    glVertex3f( sz,   sz/2,  0);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f( sz/2,  sz,  0);
////    glVertex3f(-sz/2,  sz,  0);
////    glVertex3f(-sz,   sz/2,  0);
////    glVertex3f(-sz,  -sz/2,  0);
////    glEnd();
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz/2, -sz);
//    glVertex2f( sz/2, -sz);
//    glVertex2f( sz,  -sz/2);
//    glVertex2f( sz,   sz/2);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f( sz/2,  sz);
//    glVertex2f(-sz/2,  sz);
//    glVertex2f(-sz,   sz/2);
//    glVertex2f(-sz,  -sz/2);
//    glEnd();
//    break;
//  case 5:
//    sz = size*2/3; sz2 = size/5;
//    glRotatef((float)d*360/1024, 0, 0, 1);
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_STRIP);
////    glVertex3f(-sz, -sz+sz2,  0);
////    glVertex3f( 0, sz+sz2,  0);
////    glVertex3f( sz, -sz+sz2,  0);
////    glEnd();
//    glEnable(GL_BLEND);
////    glColor4i(r, g, b, 150);
//    glColor4i(r, g, b, 150);
//
//  //senquack - converting to 2D for speed:
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz, -sz+sz2);
//    glVertex2f( sz, -sz+sz2);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f( 0, sz+sz2);
//    glEnd();
//    break;
//  case 6:
//    sz = size/2;
//    glRotatef((float)((cnt*13)&1023)*360/1024, 0, 0, 1);
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f(  0, -sz,  0);
////    glVertex3f( sz,   0,  0);
////    glVertex3f( sz,  sz,  0);
////    glVertex3f(  0,  sz,  0);
////    glVertex3f(-sz,   0,  0);
////    glEnd();
//
//  //senquack - converting to 2D for speed:
////    glEnable(GL_BLEND);
//////    glColor4i(r, g, b, 210);
////    glColor4i(r, g, b, 210);
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f(  0, -sz,  0);
////    glVertex3f( sz,   0,  0);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f( sz,  sz,  0);
////    glVertex3f(  0,  sz,  0);
////    glVertex3f(-sz,   0,  0);
////    glEnd();
//    glEnable(GL_BLEND);
////    glColor4i(r, g, b, 210);
//    glColor4i(r, g, b, 210);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz, -sz);
//    glVertex2f(  0, -sz);
//    glVertex2f( sz,   0);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f( sz,  sz);
//    glVertex2f(  0,  sz);
//    glVertex2f(-sz,   0);
//    glEnd();
//    break;
//  }
//  glPopMatrix();
//}

static GLfixed shapevertices[2 * 8];    // array capable of holding up to 8 2D vertices
static GLubyte shapecolors[4 * 8];  // array capable of holding up to 8 RGBA colors

//static GLfixed shapeptvertices[1024 * 6];
//static GLubyte shapeptcolors[1024 * 4];
static GLfixed shapeptvertices[1024 * 2 * 6];
static GLubyte shapeptcolors[1024 * 4 * 6];
static GLfixed *shapeptvertptr = &shapeptvertices[0];
static GLubyte *shapeptcolptr = &shapeptcolors[0];

////senquack - experiment
//static int samectr = 0;
//static int diffctr = 0;
//static int   shouldrotate = 1;
//static int   lastd = 0;     // keeps track of the direction angle of the last shape so we can 
//                         //    avoid unnecessary calls to opengl rotation stuff if this shape is the same

//senquack - new function called once before a series of calls to drawShape (for openglES speedup)
void
prepareDrawShapes (void)
{
//  glPushMatrix();
   glEnable (GL_BLEND);
   glVertexPointer (2, GL_FIXED, 0, shapevertices);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, shapecolors);
   shapeptvertptr = &shapeptvertices[0];
   shapeptcolptr = &shapeptcolors[0];
// lastd = 0;
// shouldrotate = 1;
}

void
finishDrawShapes (void)
{
// glPopMatrix();

   // now, draw all the cores
   glVertexPointer (2, GL_FIXED, 0, shapeptvertices);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, shapeptcolors);
   glDrawArrays (GL_TRIANGLES, 0,
                 ((unsigned int) shapeptcolptr -
                  (unsigned int) (&shapeptcolors[0])) >> 2);

   //senquack - experiment:
// printf("samectr: %d   diffctr: %d\n", samectr, diffctr);
// samectr = 0; diffctr = 0;
}

//senquack - experimenting a bit for speedups
//senquack - conversion to fixed point
//void drawShapex(GLfixed fx, GLfixed fy, GLfixed size, int d, int cnt, int type,
//        int r, int g, int b) {
////  GLfloat sz, sz2;
//  GLfixed fsz, fsz2;  // fixed point versions of above 
//
//  //senquack - for speedup on Wiz
//    glDisable(GL_BLEND);
//
//  glPushMatrix();
//
////  glTranslatef(x, y, 0);
// glTranslatex(fx, fy, 0);
// 
////  glColor4i(r, g, b, 255);
//  //senquack - converting to 2D for speed:
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex3f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
////  glVertex3f( SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
////  glVertex3f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
////  glVertex3f(-SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
////  glEnd();
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex2f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE);
////  glVertex2f( SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE);
////  glVertex2f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE);
////  glVertex2f(-SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE);
////  glEnd();
//
// shapecolors[0] = shapecolors[4] = shapecolors[8] = shapecolors[12] = r;
// shapecolors[1] = shapecolors[5] = shapecolors[9] = shapecolors[13] = g;
// shapecolors[2] = shapecolors[6] = shapecolors[10] = shapecolors[14] = b;
// shapecolors[3] = shapecolors[7] = shapecolors[11] = shapecolors[15] = 255;
//
// shapevertices[0] = -SHAPE_POINT_SIZE_X;   shapevertices[1] = -SHAPE_POINT_SIZE_X;
// shapevertices[2] = SHAPE_POINT_SIZE_X;    shapevertices[3] = -SHAPE_POINT_SIZE_X;
// shapevertices[4] = SHAPE_POINT_SIZE_X;    shapevertices[5] = SHAPE_POINT_SIZE_X;
// shapevertices[6] = -SHAPE_POINT_SIZE_X;   shapevertices[7] = SHAPE_POINT_SIZE_X;
//
////  glEnableClientState(GL_VERTEX_ARRAY);
// //note: it seems this must be called every time drawarrays is called
////  glVertexPointer(2, GL_FIXED, 0, shapevertices);
////  glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
////  glEnableClientState(GL_COLOR_ARRAY);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//
//  switch ( type ) {
//  case 0:
//   //triangle shape
////    sz = size/2;
//    fsz = size>>1;
//
////    glRotatef((float)d*360/1024, 0, 0, 1);
//
//  //same thing:
////    glRotatex(INT2FNUM((d*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((d*360)<<6, 0, 0, INT2FNUM(1));
//
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f( sz, -sz,  0);
////    glVertex3f( 0, size,  0);
////    glEnd();
//
//  // for speedup on wiz:
////    glEnable(GL_BLEND);
//
//////    glColor4i(r, g, b, 150);
////    glColor4i(r, g, b, 150);
////
////   //senquack - converting to 2D for speed:
//////    glBegin(GL_TRIANGLE_FAN);
//////    glVertex3f(-sz, -sz,  0);
//////    glVertex3f( sz, -sz,  0);
////////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//////    glVertex3f( 0, size,  0);
//////    glEnd();
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex2f(-sz, -sz);
////    glVertex2f( sz, -sz);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex2f( 0, size);
////    glEnd();
//
//  //senquack - might be unnecessary since these are the same as the initial quad core that is drawn:
// shapecolors[0] = shapecolors[4] = r;
// shapecolors[1] = shapecolors[5] = g;
// shapecolors[2] = shapecolors[6] = b;
//
// shapecolors[3] = shapecolors[7] = shapecolors[11] = 150;
// shapecolors[8] = SHAPE_BASE_COLOR_R; shapecolors[9] = SHAPE_BASE_COLOR_G; shapecolors[10] = SHAPE_BASE_COLOR_B;
// shapevertices[0] = -fsz;   shapevertices[1] = -fsz;   
// shapevertices[2] = fsz;    shapevertices[3] = -fsz;
// shapevertices[4] = 0;      shapevertices[5] = size;
// glVertexPointer(2, GL_FIXED, 0, shapevertices);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
// glDrawArrays(GL_TRIANGLES, 0, 3);
//
//    break;
//  case 1:
//  // senquack - diamond shape
////    sz = size/2;
//    fsz = size>>1;
////    glRotatef((float)((cnt*23)&1023)*360/1024, 0, 0, 1);
//  //senquack - same thing:
////    glRotatex(INT2FNUM((((cnt*23)&1023)*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((((cnt*23)&1023)*360)<<6, 0, 0, INT2FNUM(1));
//
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(  0, -size,  0);
////    glVertex3f( sz,     0,  0);
////    glVertex3f(  0,  size,  0);
////    glVertex3f(-sz,     0,  0);
////    glEnd();
//
//  // for speedup on wiz:
////    glEnable(GL_BLEND);
//
//////    glColor4i(r, g, b, 180);
////    glColor4i(r, g, b, 180);
////
////   //senquack - converting to 2D for speed:
//////    glBegin(GL_TRIANGLE_FAN);
//////    glVertex3f(  0, -size,  0);
//////    glVertex3f( sz,     0,  0);
////////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//////    glVertex3f(  0,  size,  0);
//////    glVertex3f(-sz,     0,  0);
//////    glEnd();
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex2f(  0, -size);
////    glVertex2f( sz,     0);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex2f(  0,  size);
////    glVertex2f(-sz,     0);
////    glEnd();
//
//  //senquack - might be unnecessary since these are the same as the initial quad core that is drawn:
// shapecolors[0] = shapecolors[4] = r;
// shapecolors[1] = shapecolors[5] = g;
// shapecolors[2] = shapecolors[6] = b;
//
// shapecolors[3] = shapecolors[7] = 180;
// shapecolors[8] = shapecolors[12] = SHAPE_BASE_COLOR_R;
// shapecolors[9] = shapecolors[13] = SHAPE_BASE_COLOR_G;
// shapecolors[10] = shapecolors[14] = SHAPE_BASE_COLOR_B;
// shapecolors[11] = shapecolors[15] = 150;
// shapevertices[0] = 0;   shapevertices[1] = -size;  
// shapevertices[2] = fsz;    shapevertices[3] = 0;
// shapevertices[4] = 0;      shapevertices[5] = size;
// shapevertices[6] = -fsz;   shapevertices[7] = 0;
// glVertexPointer(2, GL_FIXED, 0, shapevertices);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//
//    break;
//  case 2:
//  //rectangular shape
////    sz =3 size/4; sz2 = size/3*2;
////    fsz = size>>2; fsz2 = FDIV(size,INT2FNUM(3)) << 1;
////    fsz = size>>2; fsz2 = f2x(x2f(size) / 3.0 * 2.0);
//
//    fsz = size>>2; fsz2 = FMUL(size,43691);   //43690 = 2/3 in fixed point
//
////   // TEMP DEBUGGING:
////   if (!blah)
////   {
////      fsz = f2x(x2f(size) / 4.0);
////      fsz2 = f2x(x2f(size) / 3.0 * 2.0);
////   } else {
////      fsz = size>>2; fsz2 = FMUL(size,43691);  //43690 = 2/3 in fixed point
////   }
////   blah = !blah;
//
////    glRotatef((float)d*360/1024, 0, 0, 1);
//  //same thing:
////    glRotatex(INT2FNUM((d*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((d*360)<<6, 0, 0, INT2FNUM(1));
//
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz, -sz2,  0);
////    glVertex3f( sz, -sz2,  0);
////    glVertex3f( sz,  sz2,  0);
////    glVertex3f(-sz,  sz2,  0);
////    glEnd();
//
//  //senquack - converting to 2D for speed:
////    glEnable(GL_BLEND);
//////    glColor4i(r, g, b, 120);
////    glColor4i(r, g, b, 120);
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz, -sz2,  0);
////    glVertex3f( sz, -sz2,  0);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f( sz, sz2,  0);
////    glVertex3f(-sz, sz2,  0);
////    glEnd();
//  
//  // for speedup on wiz:
////    glEnable(GL_BLEND);
//
//////    glColor4i(r, g, b, 120);
////    glColor4i(r, g, b, 120);
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex2f(-sz, -sz2);
////    glVertex2f( sz, -sz2);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex2f( sz, sz2);
////    glVertex2f(-sz, sz2);
////    glEnd();
//
//  //senquack - might be unnecessary since these are the same as the initial quad core that is drawn:
// shapecolors[0] = shapecolors[4] = r;
// shapecolors[1] = shapecolors[5] = g;
// shapecolors[2] = shapecolors[6] = b;
//
// shapecolors[3] = shapecolors[7] = 120;
// shapecolors[8] = shapecolors[12] = SHAPE_BASE_COLOR_R;
// shapecolors[9] = shapecolors[13] = SHAPE_BASE_COLOR_G;
// shapecolors[10] = shapecolors[14] = SHAPE_BASE_COLOR_B;
// shapecolors[11] = shapecolors[15] = 150;
// shapevertices[0] = -fsz;   shapevertices[1] = -fsz2;  
// shapevertices[2] = fsz;    shapevertices[3] = -fsz2;
// shapevertices[4] = fsz;    shapevertices[5] = fsz2;
// shapevertices[6] = -fsz;   shapevertices[7] = fsz2;
// glVertexPointer(2, GL_FIXED, 0, shapevertices);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//    break;
//
//  case 3:
//  //senquack - square shape
////    sz = size/2;
// fsz = size>>1;
//
//  //senquack
////    glRotatef((float)((cnt*37)&1023)*360/1024, 0, 0, 1);
//
//  //same thing:
////    glRotatex(INT2FNUM((((cnt*37)&1023)*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((((cnt*37)&1023)*360)<<6, 0, 0, INT2FNUM(1));
//
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f( sz, -sz,  0);
////    glVertex3f( sz,  sz,  0);
////    glVertex3f(-sz,  sz,  0);
////    glEnd();
//
//  //senquack - converting to 2D for speed:
//
//  // for speedup on wiz:
////    glEnable(GL_BLEND);
//
//////    glColor4i(r, g, b, 180);
////    glColor4i(r, g, b, 180);
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f( sz, -sz,  0);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f( sz,  sz,  0);
////    glVertex3f(-sz,  sz,  0);
////    glEnd();
//
////    glEnable(GL_BLEND);
//////    glColor4i(r, g, b, 180);
////    glColor4i(r, g, b, 180);
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex2f(-sz, -sz);
////    glVertex2f( sz, -sz);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex2f( sz,  sz);
////    glVertex2f(-sz,  sz);
////    glEnd();
//
//  //senquack - might be unnecessary since these are the same as the initial quad core that is drawn:
// shapecolors[0] = shapecolors[4] = r;
// shapecolors[1] = shapecolors[5] = g;
// shapecolors[2] = shapecolors[6] = b;
//
// shapecolors[3] = shapecolors[7] = 180;
// shapecolors[8] = shapecolors[12] = SHAPE_BASE_COLOR_R;
// shapecolors[9] = shapecolors[13] = SHAPE_BASE_COLOR_G;
// shapecolors[10] = shapecolors[14] = SHAPE_BASE_COLOR_B;
// shapecolors[11] = shapecolors[15] = 150;
// shapevertices[0] = -fsz;   shapevertices[1] = -fsz;   
// shapevertices[2] = fsz;    shapevertices[3] = -fsz;
// shapevertices[4] = fsz;    shapevertices[5] = fsz;
// shapevertices[6] = -fsz;   shapevertices[7] = fsz;
// glVertexPointer(2, GL_FIXED, 0, shapevertices);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//
//    break;
//  case 4:
//  // circular shape
////    sz = size/2;
//  fsz = size>>1;
// fsz2 = size>>2;   // added this because of simplicity later on
//
//  //senquack
////    glRotatef((float)((cnt*53)&1023)*360/1024, 0, 0, 1);
//  //same thing:
////    glRotatex(INT2FNUM((((cnt*53)&1023)*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((((cnt*53)&1023)*360)<<6, 0, 0, INT2FNUM(1));
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz/2, -sz,  0);
////    glVertex3f( sz/2, -sz,  0);
////    glVertex3f( sz,  -sz/2,  0);
////    glVertex3f( sz,   sz/2,  0);
////    glVertex3f( sz/2,  sz,  0);
////    glVertex3f(-sz/2,  sz,  0);
////    glVertex3f(-sz,   sz/2,  0);
////    glVertex3f(-sz,  -sz/2,  0);
////    glEnd();
//
//  // for speedup on wiz:
////    glEnable(GL_BLEND);
//
//////    glColor4i(r, g, b, 220);
////    glColor4i(r, g, b, 220);
////
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex2f(-sz/2, -sz);
////    glVertex2f( sz/2, -sz);
////    glVertex2f( sz,  -sz/2);
////    glVertex2f( sz,   sz/2);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex2f( sz/2,  sz);
////    glVertex2f(-sz/2,  sz);
////    glVertex2f(-sz,   sz/2);
////    glVertex2f(-sz,  -sz/2);
////    glEnd();
//
//  //senquack - these three lines shouldn't be necessary:
// shapecolors[0] = shapecolors[4] = shapecolors[8] = shapecolors[12] = r;
// shapecolors[1] = shapecolors[5] = shapecolors[9] = shapecolors[13] = g;
// shapecolors[2] = shapecolors[6] = shapecolors[10] = shapecolors[14] = b;
//
// shapecolors[3] = shapecolors[7] = shapecolors[11] = shapecolors[15] = 220;
// shapecolors[16] = shapecolors[20] = shapecolors[24] = shapecolors[28] = SHAPE_BASE_COLOR_R;
// shapecolors[17] = shapecolors[21] = shapecolors[25] = shapecolors[29] = SHAPE_BASE_COLOR_G;
// shapecolors[18] = shapecolors[22] = shapecolors[26] = shapecolors[30] = SHAPE_BASE_COLOR_B;
// shapecolors[19] = shapecolors[23] = shapecolors[27] = shapecolors[31] = 150;
// shapevertices[0] = -fsz2;  shapevertices[1] = -fsz;   
// shapevertices[2] = fsz2;   shapevertices[3] = -fsz;
// shapevertices[4] = fsz;    shapevertices[5] = -fsz2;
// shapevertices[6] = fsz;    shapevertices[7] = fsz2;
// shapevertices[8] = fsz2;   shapevertices[9] = fsz; 
// shapevertices[10] = -fsz2; shapevertices[11] = fsz;
// shapevertices[12] = -fsz;     shapevertices[13] = fsz2;
// shapevertices[14] = -fsz;     shapevertices[15] = -fsz2;
////  glVertexPointer(2, GL_FIXED, 0, shapevertices);
////  glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
//    break;
//
//  case 5:
//  // triangle shape
//  // tested working
////    sz = size*2/3; sz2 = size/5;
////   fsz = FDIV((size<<1), INT2FNUM(3));
//
//    fsz = FMUL(size,43691); //43690 = 2/3 in fixed point
////   fsz2 = FDIV(size, INT2FNUM(5));
//  fsz2 = FMUL(size,13107);  //13107 is 1/5 in fixed point
//
//  // TEMP DEBUGGING:
////   if (!blah)
////   {
////      fsz = f2x(x2f(size) * 2.0 / 3.0);
////      fsz2 = f2x(x2f(size) / 5.0);
////      printf("orig size\n");
////   } else {
////      fsz = FMUL(size,43691);   //43690 = 2/3 in fixed point
//////       fsz2 = FDIV(size, INT2FNUM(5));
////      fsz2 = FMUL(size,13107);  //13107 is 1/5 in fixed point
////      printf("alt size\n");
////   }
////   blah = !blah;
//
////    glRotatef((float)d*360/1024, 0, 0, 1);
//  //same thing:
////    glRotatex(INT2FNUM((d*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((d*360)<<6, 0, 0, INT2FNUM(1));
//
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_STRIP);
////    glVertex3f(-sz, -sz+sz2,  0);
////    glVertex3f( 0, sz+sz2,  0);
////    glVertex3f( sz, -sz+sz2,  0);
////    glEnd();
//
//  // for speedup on wiz:
////    glEnable(GL_BLEND);
//
//////    glColor4i(r, g, b, 150);
////    glColor4i(r, g, b, 150);
////
////   //senquack - converting to 2D for speed:
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex2f(-sz, -sz+sz2);
////    glVertex2f( sz, -sz+sz2);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex2f( 0, sz+sz2);
////    glEnd();
//
//  //senquack - might be unnecessary since these are the same as the initial quad core that is drawn:
// shapecolors[0] = shapecolors[4] = r;
// shapecolors[1] = shapecolors[5] = g;
// shapecolors[2] = shapecolors[6] = b;
//
// shapecolors[3] = shapecolors[7] = shapecolors[11] = 150;
// shapecolors[8] = SHAPE_BASE_COLOR_R; shapecolors[9] = SHAPE_BASE_COLOR_G; shapecolors[10] = SHAPE_BASE_COLOR_B;
// shapevertices[0] = -fsz;   shapevertices[1] = -fsz + fsz2;  
// shapevertices[2] = fsz;    shapevertices[3] = -fsz + fsz2;
// shapevertices[4] = 0;      shapevertices[5] = fsz + fsz2;
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
// glVertexPointer(2, GL_FIXED, 0, shapevertices);
// glDrawArrays(GL_TRIANGLES, 0, 3);
//
//    break;
//  case 6:
//  //senquack - hexagonal shape
////    sz = size/2;
//  fsz = size>>1;
//
//  //senquack
////    glRotatef((float)((cnt*13)&1023)*360/1024, 0, 0, 1);
////    glRotatex(INT2FNUM((((cnt*13)&1023)*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((((cnt*13)&1023)*360)<<6, 0, 0, INT2FNUM(1));
//
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f(  0, -sz,  0);
////    glVertex3f( sz,   0,  0);
////    glVertex3f( sz,  sz,  0);
////    glVertex3f(  0,  sz,  0);
////    glVertex3f(-sz,   0,  0);
////    glEnd();
//
//  //senquack - converting to 2D for speed:
//
//  // for speedup on wiz:
////    glEnable(GL_BLEND);
//
//////    glColor4i(r, g, b, 210);
////    glColor4i(r, g, b, 210);
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f(  0, -sz,  0);
////    glVertex3f( sz,   0,  0);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f( sz,  sz,  0);
////    glVertex3f(  0,  sz,  0);
////    glVertex3f(-sz,   0,  0);
////    glEnd();
//
////    glEnable(GL_BLEND);
//////    glColor4i(r, g, b, 210);
////    glColor4i(r, g, b, 210);
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex2f(-sz, -sz);
////    glVertex2f(  0, -sz);
////    glVertex2f( sz,   0);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex2f( sz,  sz);
////    glVertex2f(  0,  sz);
////    glVertex2f(-sz,   0);
////    glEnd();
//
// shapecolors[3] = shapecolors[7] = shapecolors[11] = 210;
// shapecolors[12] = shapecolors[16] = shapecolors[20] = SHAPE_BASE_COLOR_R;
// shapecolors[13] = shapecolors[17] = shapecolors[21] = SHAPE_BASE_COLOR_G;
// shapecolors[14] = shapecolors[18] = shapecolors[22] = SHAPE_BASE_COLOR_B;
// shapecolors[15] = shapecolors[19] = shapecolors[23] = 150;
// shapevertices[0] = -fsz;   shapevertices[1] = -fsz;   
// shapevertices[2] = 0;      shapevertices[3] = -fsz;
// shapevertices[4] = fsz;    shapevertices[5] = 0;
// shapevertices[6] = fsz;    shapevertices[7] = fsz;
// shapevertices[8] = 0;      shapevertices[9] = fsz;
// shapevertices[10] = -fsz;  shapevertices[11] = 0;
// glVertexPointer(2, GL_FIXED, 0, shapevertices);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
//
//    break;
//  }
//  glPopMatrix();
//}
//void drawShapex(GLfixed fx, GLfixed fy, GLfixed size, int d, int cnt, int type,
//        int r, int g, int b) {
////  GLfloat sz, sz2;
//  GLfixed fsz, fsz2;  // fixed point versions of above 
//
////  //senquack - temporary - make a list of all the various types and their sizes
////  int ctr, done;
////  for (ctr = 0, done = 0; (ctr < 50) && !done; ctr++)
////  {
////     if (shapes[type][ctr].count == 0) {
////        //found an unused slot to record our size
////        shapes[type][ctr].size = size;
////        shapes[type][ctr].count++;
////        done = 1;
////     } else {
////        if (shapes[type][ctr].size == size) {
////           shapes[type][ctr].count++;
////           done = 1;
////        }
////     }
////  }
//       
//  //senquack - for speedup on Wiz (no noticable speedup from this actually)
////    glDisable(GL_BLEND);
//    glEnable(GL_BLEND);
//
//  glPushMatrix();
//
////  glTranslatef(x, y, 0);
// glTranslatex(fx, fy, 0);
// 
////  glColor4i(r, g, b, 255);
//  //senquack - converting to 2D for speed:
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex3f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
////  glVertex3f( SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
////  glVertex3f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
////  glVertex3f(-SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
////  glEnd();
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex2f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE);
////  glVertex2f( SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE);
////  glVertex2f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE);
////  glVertex2f(-SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE);
////  glEnd();
//
// shapecolors[0] = shapecolors[4] = shapecolors[8] = shapecolors[12] = r;
// shapecolors[1] = shapecolors[5] = shapecolors[9] = shapecolors[13] = g;
// shapecolors[2] = shapecolors[6] = shapecolors[10] = shapecolors[14] = b;
// shapecolors[3] = shapecolors[7] = shapecolors[11] = shapecolors[15] = 255;
//
// shapevertices[0] = -SHAPE_POINT_SIZE_X;   shapevertices[1] = -SHAPE_POINT_SIZE_X;
// shapevertices[2] = SHAPE_POINT_SIZE_X;    shapevertices[3] = -SHAPE_POINT_SIZE_X;
// shapevertices[4] = SHAPE_POINT_SIZE_X;    shapevertices[5] = SHAPE_POINT_SIZE_X;
// shapevertices[6] = -SHAPE_POINT_SIZE_X;   shapevertices[7] = SHAPE_POINT_SIZE_X;
//
////  glEnableClientState(GL_VERTEX_ARRAY);
// //note: it seems this must be called every time drawarrays is called
////  glVertexPointer(2, GL_FIXED, 0, shapevertices);
////  glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
////  glEnableClientState(GL_COLOR_ARRAY);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//
//  switch ( type ) {
//  case 0:
//   //triangle shape
////    sz = size/2;
//    fsz = size>>1;
//
////    glRotatef((float)d*360/1024, 0, 0, 1);
//    glRotatef((d*360)>>10, 0, 0, 1);
//
//  //same thing:
////    glRotatex(INT2FNUM((d*360)>>10), 0, 0, INT2FNUM(1));
////    glRotatex((d*360)<<6, 0, 0, INT2FNUM(1));
//
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f( sz, -sz,  0);
////    glVertex3f( 0, size,  0);
////    glEnd();
//
//  // for speedup on wiz:
////    glEnable(GL_BLEND);
//
//////    glColor4i(r, g, b, 150);
////    glColor4i(r, g, b, 150);
////
////   //senquack - converting to 2D for speed:
//////    glBegin(GL_TRIANGLE_FAN);
//////    glVertex3f(-sz, -sz,  0);
//////    glVertex3f( sz, -sz,  0);
////////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//////    glVertex3f( 0, size,  0);
//////    glEnd();
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex2f(-sz, -sz);
////    glVertex2f( sz, -sz);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex2f( 0, size);
////    glEnd();
//
//  //senquack - might be unnecessary since these are the same as the initial quad core that is drawn:
////  shapecolors[0] = shapecolors[4] = r;
////  shapecolors[1] = shapecolors[5] = g;
////  shapecolors[2] = shapecolors[6] = b;
//
// shapecolors[3] = shapecolors[7] = shapecolors[11] = 150;
// shapecolors[8] = SHAPE_BASE_COLOR_R; shapecolors[9] = SHAPE_BASE_COLOR_G; shapecolors[10] = SHAPE_BASE_COLOR_B;
// shapevertices[0] = -fsz;   shapevertices[1] = -fsz;   
// shapevertices[2] = fsz;    shapevertices[3] = -fsz;
// shapevertices[4] = 0;      shapevertices[5] = size;
////  glVertexPointer(2, GL_FIXED, 0, shapevertices);
////  glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
// glDrawArrays(GL_TRIANGLES, 0, 3);
//
//    break;
//  case 1:
//  // senquack - diamond shape
////    sz = size/2;
//    fsz = size>>1;
////    glRotatef((float)((cnt*23)&1023)*360/1024, 0, 0, 1);
//  //senquack - same thing:
////    glRotatex(INT2FNUM((((cnt*23)&1023)*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((((cnt*23)&1023)*360)<<6, 0, 0, INT2FNUM(1));
//
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(  0, -size,  0);
////    glVertex3f( sz,     0,  0);
////    glVertex3f(  0,  size,  0);
////    glVertex3f(-sz,     0,  0);
////    glEnd();
//
//  // for speedup on wiz:
////    glEnable(GL_BLEND);
//
//////    glColor4i(r, g, b, 180);
////    glColor4i(r, g, b, 180);
////
////   //senquack - converting to 2D for speed:
//////    glBegin(GL_TRIANGLE_FAN);
//////    glVertex3f(  0, -size,  0);
//////    glVertex3f( sz,     0,  0);
////////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//////    glVertex3f(  0,  size,  0);
//////    glVertex3f(-sz,     0,  0);
//////    glEnd();
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex2f(  0, -size);
////    glVertex2f( sz,     0);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex2f(  0,  size);
////    glVertex2f(-sz,     0);
////    glEnd();
//
//  //senquack - might be unnecessary since these are the same as the initial quad core that is drawn:
////  shapecolors[0] = shapecolors[4] = r;
////  shapecolors[1] = shapecolors[5] = g;
////  shapecolors[2] = shapecolors[6] = b;
//
// shapecolors[3] = shapecolors[7] = 180;
// shapecolors[8] = shapecolors[12] = SHAPE_BASE_COLOR_R;
// shapecolors[9] = shapecolors[13] = SHAPE_BASE_COLOR_G;
// shapecolors[10] = shapecolors[14] = SHAPE_BASE_COLOR_B;
// shapecolors[11] = shapecolors[15] = 150;
// shapevertices[0] = 0;   shapevertices[1] = -size;  
// shapevertices[2] = fsz;    shapevertices[3] = 0;
// shapevertices[4] = 0;      shapevertices[5] = size;
// shapevertices[6] = -fsz;   shapevertices[7] = 0;
////  glVertexPointer(2, GL_FIXED, 0, shapevertices);
////  glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//
//    break;
//  case 2:
//  //rectangular shape
////    sz =3 size/4; sz2 = size/3*2;
////    fsz = size>>2; fsz2 = FDIV(size,INT2FNUM(3)) << 1;
////    fsz = size>>2; fsz2 = f2x(x2f(size) / 3.0 * 2.0);
//
//    fsz = size>>2; fsz2 = FMUL(size,43691);   //43690 = 2/3 in fixed point
//
////   // TEMP DEBUGGING:
////   if (!blah)
////   {
////      fsz = f2x(x2f(size) / 4.0);
////      fsz2 = f2x(x2f(size) / 3.0 * 2.0);
////   } else {
////      fsz = size>>2; fsz2 = FMUL(size,43691);  //43690 = 2/3 in fixed point
////   }
////   blah = !blah;
//
////    glRotatef((float)d*360/1024, 0, 0, 1);
//  //same thing:
////    glRotatex(INT2FNUM((d*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((d*360)<<6, 0, 0, INT2FNUM(1));
//
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz, -sz2,  0);
////    glVertex3f( sz, -sz2,  0);
////    glVertex3f( sz,  sz2,  0);
////    glVertex3f(-sz,  sz2,  0);
////    glEnd();
//
//  //senquack - converting to 2D for speed:
////    glEnable(GL_BLEND);
//////    glColor4i(r, g, b, 120);
////    glColor4i(r, g, b, 120);
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz, -sz2,  0);
////    glVertex3f( sz, -sz2,  0);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f( sz, sz2,  0);
////    glVertex3f(-sz, sz2,  0);
////    glEnd();
//  
//  // for speedup on wiz:
////    glEnable(GL_BLEND);
//
//////    glColor4i(r, g, b, 120);
////    glColor4i(r, g, b, 120);
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex2f(-sz, -sz2);
////    glVertex2f( sz, -sz2);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex2f( sz, sz2);
////    glVertex2f(-sz, sz2);
////    glEnd();
//
//  //senquack - might be unnecessary since these are the same as the initial quad core that is drawn:
////  shapecolors[0] = shapecolors[4] = r;
////  shapecolors[1] = shapecolors[5] = g;
////  shapecolors[2] = shapecolors[6] = b;
//
// shapecolors[3] = shapecolors[7] = 120;
// shapecolors[8] = shapecolors[12] = SHAPE_BASE_COLOR_R;
// shapecolors[9] = shapecolors[13] = SHAPE_BASE_COLOR_G;
// shapecolors[10] = shapecolors[14] = SHAPE_BASE_COLOR_B;
// shapecolors[11] = shapecolors[15] = 150;
// shapevertices[0] = -fsz;   shapevertices[1] = -fsz2;  
// shapevertices[2] = fsz;    shapevertices[3] = -fsz2;
// shapevertices[4] = fsz;    shapevertices[5] = fsz2;
// shapevertices[6] = -fsz;   shapevertices[7] = fsz2;
////  glVertexPointer(2, GL_FIXED, 0, shapevertices);
////  glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//    break;
//
//  case 3:
//  //senquack - square shape
////    sz = size/2;
// fsz = size>>1;
//
//  //senquack
////    glRotatef((float)((cnt*37)&1023)*360/1024, 0, 0, 1);
//
//  //same thing:
////    glRotatex(INT2FNUM((((cnt*37)&1023)*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((((cnt*37)&1023)*360)<<6, 0, 0, INT2FNUM(1));
//
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f( sz, -sz,  0);
////    glVertex3f( sz,  sz,  0);
////    glVertex3f(-sz,  sz,  0);
////    glEnd();
//
//  //senquack - converting to 2D for speed:
//
//  // for speedup on wiz:
////    glEnable(GL_BLEND);
//
//////    glColor4i(r, g, b, 180);
////    glColor4i(r, g, b, 180);
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f( sz, -sz,  0);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f( sz,  sz,  0);
////    glVertex3f(-sz,  sz,  0);
////    glEnd();
//
////    glEnable(GL_BLEND);
//////    glColor4i(r, g, b, 180);
////    glColor4i(r, g, b, 180);
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex2f(-sz, -sz);
////    glVertex2f( sz, -sz);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex2f( sz,  sz);
////    glVertex2f(-sz,  sz);
////    glEnd();
//
//  //senquack - might be unnecessary since these are the same as the initial quad core that is drawn:
////  shapecolors[0] = shapecolors[4] = r;
////  shapecolors[1] = shapecolors[5] = g;
////  shapecolors[2] = shapecolors[6] = b;
//
// shapecolors[3] = shapecolors[7] = 180;
// shapecolors[8] = shapecolors[12] = SHAPE_BASE_COLOR_R;
// shapecolors[9] = shapecolors[13] = SHAPE_BASE_COLOR_G;
// shapecolors[10] = shapecolors[14] = SHAPE_BASE_COLOR_B;
// shapecolors[11] = shapecolors[15] = 150;
// shapevertices[0] = -fsz;   shapevertices[1] = -fsz;   
// shapevertices[2] = fsz;    shapevertices[3] = -fsz;
// shapevertices[4] = fsz;    shapevertices[5] = fsz;
// shapevertices[6] = -fsz;   shapevertices[7] = fsz;
////  glVertexPointer(2, GL_FIXED, 0, shapevertices);
////  glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//
//    break;
//  case 4:
//  // circular shape
////    sz = size/2;
//  fsz = size>>1;
// fsz2 = size>>2;   // added this because of simplicity later on
//
//  //senquack
////    glRotatef((float)((cnt*53)&1023)*360/1024, 0, 0, 1);
//  //same thing:
////    glRotatex(INT2FNUM((((cnt*53)&1023)*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((((cnt*53)&1023)*360)<<6, 0, 0, INT2FNUM(1));
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz/2, -sz,  0);
////    glVertex3f( sz/2, -sz,  0);
////    glVertex3f( sz,  -sz/2,  0);
////    glVertex3f( sz,   sz/2,  0);
////    glVertex3f( sz/2,  sz,  0);
////    glVertex3f(-sz/2,  sz,  0);
////    glVertex3f(-sz,   sz/2,  0);
////    glVertex3f(-sz,  -sz/2,  0);
////    glEnd();
//
//  // for speedup on wiz:
////    glEnable(GL_BLEND);
//
//////    glColor4i(r, g, b, 220);
////    glColor4i(r, g, b, 220);
////
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex2f(-sz/2, -sz);
////    glVertex2f( sz/2, -sz);
////    glVertex2f( sz,  -sz/2);
////    glVertex2f( sz,   sz/2);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex2f( sz/2,  sz);
////    glVertex2f(-sz/2,  sz);
////    glVertex2f(-sz,   sz/2);
////    glVertex2f(-sz,  -sz/2);
////    glEnd();
//
//  //senquack - these three lines shouldn't be necessary:
////  shapecolors[0] = shapecolors[4] = shapecolors[8] = shapecolors[12] = r;
////  shapecolors[1] = shapecolors[5] = shapecolors[9] = shapecolors[13] = g;
////  shapecolors[2] = shapecolors[6] = shapecolors[10] = shapecolors[14] = b;
//
// shapecolors[3] = shapecolors[7] = shapecolors[11] = shapecolors[15] = 220;
// shapecolors[16] = shapecolors[20] = shapecolors[24] = shapecolors[28] = SHAPE_BASE_COLOR_R;
// shapecolors[17] = shapecolors[21] = shapecolors[25] = shapecolors[29] = SHAPE_BASE_COLOR_G;
// shapecolors[18] = shapecolors[22] = shapecolors[26] = shapecolors[30] = SHAPE_BASE_COLOR_B;
// shapecolors[19] = shapecolors[23] = shapecolors[27] = shapecolors[31] = 150;
// shapevertices[0] = -fsz2;  shapevertices[1] = -fsz;   
// shapevertices[2] = fsz2;   shapevertices[3] = -fsz;
// shapevertices[4] = fsz;    shapevertices[5] = -fsz2;
// shapevertices[6] = fsz;    shapevertices[7] = fsz2;
// shapevertices[8] = fsz2;   shapevertices[9] = fsz; 
// shapevertices[10] = -fsz2; shapevertices[11] = fsz;
// shapevertices[12] = -fsz;     shapevertices[13] = fsz2;
// shapevertices[14] = -fsz;     shapevertices[15] = -fsz2;
////  glVertexPointer(2, GL_FIXED, 0, shapevertices);
////  glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
//    break;
//
//  case 5:
//  // triangle shape
//  // tested working
////    sz = size*2/3; sz2 = size/5;
////   fsz = FDIV((size<<1), INT2FNUM(3));
//
//    fsz = FMUL(size,43691); //43690 = 2/3 in fixed point
////   fsz2 = FDIV(size, INT2FNUM(5));
//  fsz2 = FMUL(size,13107);  //13107 is 1/5 in fixed point
//
//  // TEMP DEBUGGING:
////   if (!blah)
////   {
////      fsz = f2x(x2f(size) * 2.0 / 3.0);
////      fsz2 = f2x(x2f(size) / 5.0);
////      printf("orig size\n");
////   } else {
////      fsz = FMUL(size,43691);   //43690 = 2/3 in fixed point
//////       fsz2 = FDIV(size, INT2FNUM(5));
////      fsz2 = FMUL(size,13107);  //13107 is 1/5 in fixed point
////      printf("alt size\n");
////   }
////   blah = !blah;
//
////    glRotatef((float)d*360/1024, 0, 0, 1);
//  //same thing:
////    glRotatex(INT2FNUM((d*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((d*360)<<6, 0, 0, INT2FNUM(1));
//
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_STRIP);
////    glVertex3f(-sz, -sz+sz2,  0);
////    glVertex3f( 0, sz+sz2,  0);
////    glVertex3f( sz, -sz+sz2,  0);
////    glEnd();
//
//  // for speedup on wiz:
////    glEnable(GL_BLEND);
//
//////    glColor4i(r, g, b, 150);
////    glColor4i(r, g, b, 150);
////
////   //senquack - converting to 2D for speed:
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex2f(-sz, -sz+sz2);
////    glVertex2f( sz, -sz+sz2);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex2f( 0, sz+sz2);
////    glEnd();
//
//  //senquack - might be unnecessary since these are the same as the initial quad core that is drawn:
////  shapecolors[0] = shapecolors[4] = r;
////  shapecolors[1] = shapecolors[5] = g;
////  shapecolors[2] = shapecolors[6] = b;
//
// shapecolors[3] = shapecolors[7] = shapecolors[11] = 150;
// shapecolors[8] = SHAPE_BASE_COLOR_R; shapecolors[9] = SHAPE_BASE_COLOR_G; shapecolors[10] = SHAPE_BASE_COLOR_B;
// shapevertices[0] = -fsz;   shapevertices[1] = -fsz + fsz2;  
// shapevertices[2] = fsz;    shapevertices[3] = -fsz + fsz2;
// shapevertices[4] = 0;      shapevertices[5] = fsz + fsz2;
////  glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
////  glVertexPointer(2, GL_FIXED, 0, shapevertices);
// glDrawArrays(GL_TRIANGLES, 0, 3);
//
//    break;
//  case 6:
//  //senquack - hexagonal shape
////    sz = size/2;
//  fsz = size>>1;
//
//  //senquack
////    glRotatef((float)((cnt*13)&1023)*360/1024, 0, 0, 1);
////    glRotatex(INT2FNUM((((cnt*13)&1023)*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((((cnt*13)&1023)*360)<<6, 0, 0, INT2FNUM(1));
//
//  //senquack - no need for this
////    glDisable(GL_BLEND);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f(  0, -sz,  0);
////    glVertex3f( sz,   0,  0);
////    glVertex3f( sz,  sz,  0);
////    glVertex3f(  0,  sz,  0);
////    glVertex3f(-sz,   0,  0);
////    glEnd();
//
//  //senquack - converting to 2D for speed:
//
//  // for speedup on wiz:
////    glEnable(GL_BLEND);
//
//////    glColor4i(r, g, b, 210);
////    glColor4i(r, g, b, 210);
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f(  0, -sz,  0);
////    glVertex3f( sz,   0,  0);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f( sz,  sz,  0);
////    glVertex3f(  0,  sz,  0);
////    glVertex3f(-sz,   0,  0);
////    glEnd();
//
////    glEnable(GL_BLEND);
//////    glColor4i(r, g, b, 210);
////    glColor4i(r, g, b, 210);
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex2f(-sz, -sz);
////    glVertex2f(  0, -sz);
////    glVertex2f( sz,   0);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex2f( sz,  sz);
////    glVertex2f(  0,  sz);
////    glVertex2f(-sz,   0);
////    glEnd();
//
// shapecolors[3] = shapecolors[7] = shapecolors[11] = 210;
// shapecolors[12] = shapecolors[16] = shapecolors[20] = SHAPE_BASE_COLOR_R;
// shapecolors[13] = shapecolors[17] = shapecolors[21] = SHAPE_BASE_COLOR_G;
// shapecolors[14] = shapecolors[18] = shapecolors[22] = SHAPE_BASE_COLOR_B;
// shapecolors[15] = shapecolors[19] = shapecolors[23] = 150;
// shapevertices[0] = -fsz;   shapevertices[1] = -fsz;   
// shapevertices[2] = 0;      shapevertices[3] = -fsz;
// shapevertices[4] = fsz;    shapevertices[5] = 0;
// shapevertices[6] = fsz;    shapevertices[7] = fsz;
// shapevertices[8] = 0;      shapevertices[9] = fsz;
// shapevertices[10] = -fsz;  shapevertices[11] = 0;
////  glVertexPointer(2, GL_FIXED, 0, shapevertices);
////  glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
//
//    break;
//  }
// 
//
//  glPopMatrix();
//}
void
drawShapex (GLfixed fx, GLfixed fy, GLfixed size, int d, int cnt, int type,
            int r, int g, int b)
{
   GLfixed fsz, fsz2;           // fixed point versions of above 

//  //senquack - experiment
//  if (lastd == d) {
////    samectr++;
//   shouldrotate = 0;
//  } else {
////    diffctr++;
//   shouldrotate = 1;
//  }
//  lastd = d;
//
//
////  glPushMatrix();
//  if (shouldrotate) {
//   glPopMatrix();
//   glPushMatrix();
//  }
   glPushMatrix ();

//  glTranslatef(x, y, 0);
   glTranslatex (fx, fy, 0);

// //core of shapes
// shapecolors[0] = shapecolors[4] = shapecolors[8] = shapecolors[12] = r;
// shapecolors[1] = shapecolors[5] = shapecolors[9] = shapecolors[13] = g;
// shapecolors[2] = shapecolors[6] = shapecolors[10] = shapecolors[14] = b;
// shapecolors[3] = shapecolors[7] = shapecolors[11] = shapecolors[15] = 255;
// shapevertices[0] = -SHAPE_POINT_SIZE_X;   shapevertices[1] = -SHAPE_POINT_SIZE_X;
// shapevertices[2] = SHAPE_POINT_SIZE_X;    shapevertices[3] = -SHAPE_POINT_SIZE_X;
// shapevertices[4] = SHAPE_POINT_SIZE_X;    shapevertices[5] = SHAPE_POINT_SIZE_X;
// shapevertices[6] = -SHAPE_POINT_SIZE_X;   shapevertices[7] = SHAPE_POINT_SIZE_X;
// glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 255;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 255;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 255;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 255;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 255;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 255;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 128;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 128;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 128;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 128;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 128;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 128;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 192;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 192;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 192;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 192;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 192;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 192;
   *shapeptcolptr++ = r;
   *shapeptcolptr++ = g;
   *shapeptcolptr++ = b;
   *shapeptcolptr++ = 220;
   *shapeptcolptr++ = r;
   *shapeptcolptr++ = g;
   *shapeptcolptr++ = b;
   *shapeptcolptr++ = 220;
   *shapeptcolptr++ = r;
   *shapeptcolptr++ = g;
   *shapeptcolptr++ = b;
   *shapeptcolptr++ = 220;
   *shapeptcolptr++ = r;
   *shapeptcolptr++ = g;
   *shapeptcolptr++ = b;
   *shapeptcolptr++ = 220;
   *shapeptcolptr++ = r;
   *shapeptcolptr++ = g;
   *shapeptcolptr++ = b;
   *shapeptcolptr++ = 220;
   *shapeptcolptr++ = r;
   *shapeptcolptr++ = g;
   *shapeptcolptr++ = b;
   *shapeptcolptr++ = 220;
   *shapeptvertptr++ = fx - SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fy - SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fx - SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fy + SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fx + SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fy - SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fx + SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fy + SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fx + SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fy - SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fx - SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fy + SHAPE_POINT_SIZE_X;

   switch (type) {
   case 0:
      //triangle shape
//    sz = size/2;
      fsz = size >> 1;

//    glRotatef((float)d*360/1024, 0, 0, 1);
      glRotatef ((float) ((d * 360) >> 10), 0, 0, 1);
//    if (shouldrotate) glRotatef((float)((d*360)>>10), 0, 0, 1);

      //same thing:
//    glRotatex(INT2FNUM((d*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((d*360)<<6, 0, 0, INT2FNUM(1));

      //senquack - no need for this
//    glDisable(GL_BLEND);
//    glBegin(GL_LINE_LOOP);
//    glVertex3f(-sz, -sz,  0);
//    glVertex3f( sz, -sz,  0);
//    glVertex3f( 0, size,  0);
//    glEnd();

      // for speedup on wiz:
//    glEnable(GL_BLEND);

////    glColor4i(r, g, b, 150);
//    glColor4i(r, g, b, 150);
//
//  //senquack - converting to 2D for speed:
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz, -sz,  0);
////    glVertex3f( sz, -sz,  0);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f( 0, size,  0);
////    glEnd();
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz, -sz);
//    glVertex2f( sz, -sz);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f( 0, size);
//    glEnd();

      shapecolors[0] = shapecolors[4] = r;
      shapecolors[1] = shapecolors[5] = g;
      shapecolors[2] = shapecolors[6] = b;
      shapecolors[3] = shapecolors[7] = shapecolors[11] = 150;
      shapecolors[8] = SHAPE_BASE_COLOR_R;
      shapecolors[9] = SHAPE_BASE_COLOR_G;
      shapecolors[10] = SHAPE_BASE_COLOR_B;
      shapevertices[0] = -fsz;
      shapevertices[1] = -fsz;
      shapevertices[2] = fsz;
      shapevertices[3] = -fsz;
      shapevertices[4] = 0;
      shapevertices[5] = size;
// glVertexPointer(2, GL_FIXED, 0, shapevertices);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
      glDrawArrays (GL_TRIANGLES, 0, 3);

      break;
   case 1:
      // senquack - diamond shape
//    sz = size/2;
      fsz = size >> 1;
//    glRotatef((float)((cnt*23)&1023)*360/1024, 0, 0, 1);
      glRotatef ((float) ((((cnt * 23) & 1023) * 360) >> 10), 0, 0, 1);
//    if (shouldrotate) glRotatef((float)((((cnt*23)&1023)*360)>>10), 0, 0, 1);
      //senquack - same thing:
//    glRotatex(INT2FNUM((((cnt*23)&1023)*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((((cnt*23)&1023)*360)<<6, 0, 0, INT2FNUM(1));

      //senquack - no need for this
//    glDisable(GL_BLEND);
//    glBegin(GL_LINE_LOOP);
//    glVertex3f(  0, -size,  0);
//    glVertex3f( sz,     0,  0);
//    glVertex3f(  0,  size,  0);
//    glVertex3f(-sz,     0,  0);
//    glEnd();

      // for speedup on wiz:
//    glEnable(GL_BLEND);

////    glColor4i(r, g, b, 180);
//    glColor4i(r, g, b, 180);
//
//  //senquack - converting to 2D for speed:
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(  0, -size,  0);
////    glVertex3f( sz,     0,  0);
//////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
////    glVertex3f(  0,  size,  0);
////    glVertex3f(-sz,     0,  0);
////    glEnd();
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(  0, -size);
//    glVertex2f( sz,     0);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f(  0,  size);
//    glVertex2f(-sz,     0);
//    glEnd();

      shapecolors[0] = shapecolors[4] = r;
      shapecolors[1] = shapecolors[5] = g;
      shapecolors[2] = shapecolors[6] = b;
      shapecolors[3] = shapecolors[7] = 180;
      shapecolors[8] = shapecolors[12] = SHAPE_BASE_COLOR_R;
      shapecolors[9] = shapecolors[13] = SHAPE_BASE_COLOR_G;
      shapecolors[10] = shapecolors[14] = SHAPE_BASE_COLOR_B;
      shapecolors[11] = shapecolors[15] = 150;
      shapevertices[0] = 0;
      shapevertices[1] = -size;
      shapevertices[2] = fsz;
      shapevertices[3] = 0;
      shapevertices[4] = 0;
      shapevertices[5] = size;
      shapevertices[6] = -fsz;
      shapevertices[7] = 0;
// glVertexPointer(2, GL_FIXED, 0, shapevertices);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
      glDrawArrays (GL_TRIANGLE_FAN, 0, 4);

      break;
   case 2:
      //rectangular shape
//    sz =3 size/4; sz2 = size/3*2;
//    fsz = size>>2; fsz2 = FDIV(size,INT2FNUM(3)) << 1;
//    fsz = size>>2; fsz2 = f2x(x2f(size) / 3.0 * 2.0);

      fsz = size >> 2;
      fsz2 = FMUL (size, 43691);    //43690 = 2/3 in fixed point

//  // TEMP DEBUGGING:
//  if (!blah)
//  {
//     fsz = f2x(x2f(size) / 4.0);
//     fsz2 = f2x(x2f(size) / 3.0 * 2.0);
//  } else {
//     fsz = size>>2; fsz2 = FMUL(size,43691);  //43690 = 2/3 in fixed point
//  }
//  blah = !blah;

//    glRotatef((float)d*360/1024, 0, 0, 1);
      glRotatef ((float) ((d * 360) >> 10), 0, 0, 1);
//    if (shouldrotate) glRotatef((float)((d*360)>>10), 0, 0, 1);
      //same thing:
//    glRotatex(INT2FNUM((d*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((d*360)<<6, 0, 0, INT2FNUM(1));

      //senquack - no need for this
//    glDisable(GL_BLEND);
//    glBegin(GL_LINE_LOOP);
//    glVertex3f(-sz, -sz2,  0);
//    glVertex3f( sz, -sz2,  0);
//    glVertex3f( sz,  sz2,  0);
//    glVertex3f(-sz,  sz2,  0);
//    glEnd();

      //senquack - converting to 2D for speed:
//    glEnable(GL_BLEND);
////    glColor4i(r, g, b, 120);
//    glColor4i(r, g, b, 120);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex3f(-sz, -sz2,  0);
//    glVertex3f( sz, -sz2,  0);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex3f( sz, sz2,  0);
//    glVertex3f(-sz, sz2,  0);
//    glEnd();

      // for speedup on wiz:
//    glEnable(GL_BLEND);

////    glColor4i(r, g, b, 120);
//    glColor4i(r, g, b, 120);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz, -sz2);
//    glVertex2f( sz, -sz2);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f( sz, sz2);
//    glVertex2f(-sz, sz2);
//    glEnd();

      shapecolors[0] = shapecolors[4] = r;
      shapecolors[1] = shapecolors[5] = g;
      shapecolors[2] = shapecolors[6] = b;
      shapecolors[3] = shapecolors[7] = 120;
      shapecolors[8] = shapecolors[12] = SHAPE_BASE_COLOR_R;
      shapecolors[9] = shapecolors[13] = SHAPE_BASE_COLOR_G;
      shapecolors[10] = shapecolors[14] = SHAPE_BASE_COLOR_B;
      shapecolors[11] = shapecolors[15] = 150;
      shapevertices[0] = -fsz;
      shapevertices[1] = -fsz2;
      shapevertices[2] = fsz;
      shapevertices[3] = -fsz2;
      shapevertices[4] = fsz;
      shapevertices[5] = fsz2;
      shapevertices[6] = -fsz;
      shapevertices[7] = fsz2;
// glVertexPointer(2, GL_FIXED, 0, shapevertices);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
      glDrawArrays (GL_TRIANGLE_FAN, 0, 4);
      break;

   case 3:
      //senquack - square shape
//    sz = size/2;
      fsz = size >> 1;

      //senquack
//    glRotatef((float)((cnt*37)&1023)*360/1024, 0, 0, 1);
      glRotatef ((float) ((((cnt * 37) & 1023) * 360) >> 10), 0, 0, 1);
//    if (shouldrotate) glRotatef((float)((((cnt*37)&1023)*360)>>10), 0, 0, 1);

      //same thing:
//    glRotatex(INT2FNUM((((cnt*37)&1023)*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((((cnt*37)&1023)*360)<<6, 0, 0, INT2FNUM(1));

      //senquack - no need for this
//    glDisable(GL_BLEND);
//    glBegin(GL_LINE_LOOP);
//    glVertex3f(-sz, -sz,  0);
//    glVertex3f( sz, -sz,  0);
//    glVertex3f( sz,  sz,  0);
//    glVertex3f(-sz,  sz,  0);
//    glEnd();

      //senquack - converting to 2D for speed:

      // for speedup on wiz:
//    glEnable(GL_BLEND);

////    glColor4i(r, g, b, 180);
//    glColor4i(r, g, b, 180);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex3f(-sz, -sz,  0);
//    glVertex3f( sz, -sz,  0);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex3f( sz,  sz,  0);
//    glVertex3f(-sz,  sz,  0);
//    glEnd();

//    glEnable(GL_BLEND);
////    glColor4i(r, g, b, 180);
//    glColor4i(r, g, b, 180);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz, -sz);
//    glVertex2f( sz, -sz);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f( sz,  sz);
//    glVertex2f(-sz,  sz);
//    glEnd();

      shapecolors[0] = shapecolors[4] = r;
      shapecolors[1] = shapecolors[5] = g;
      shapecolors[2] = shapecolors[6] = b;
      shapecolors[3] = shapecolors[7] = 180;
      shapecolors[8] = shapecolors[12] = SHAPE_BASE_COLOR_R;
      shapecolors[9] = shapecolors[13] = SHAPE_BASE_COLOR_G;
      shapecolors[10] = shapecolors[14] = SHAPE_BASE_COLOR_B;
      shapecolors[11] = shapecolors[15] = 150;
      shapevertices[0] = -fsz;
      shapevertices[1] = -fsz;
      shapevertices[2] = fsz;
      shapevertices[3] = -fsz;
      shapevertices[4] = fsz;
      shapevertices[5] = fsz;
      shapevertices[6] = -fsz;
      shapevertices[7] = fsz;
// glVertexPointer(2, GL_FIXED, 0, shapevertices);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
      glDrawArrays (GL_TRIANGLE_FAN, 0, 4);

      break;
   case 4:
      // circular shape
//    sz = size/2;
      fsz = size >> 1;
      fsz2 = size >> 2;         // added this because of simplicity later on

      //senquack
//    glRotatef((float)((cnt*53)&1023)*360/1024, 0, 0, 1);
      glRotatef ((float) ((((cnt * 53) & 1023) * 360) >> 10), 0, 0, 1);
//    if (shouldrotate) glRotatef((float)((((cnt*53)&1023)*360)>>10), 0, 0, 1);
      //same thing:
//    glRotatex(INT2FNUM((((cnt*53)&1023)*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((((cnt*53)&1023)*360)<<6, 0, 0, INT2FNUM(1));

      //senquack - no need for this
//    glDisable(GL_BLEND);
//    glBegin(GL_LINE_LOOP);
//    glVertex3f(-sz/2, -sz,  0);
//    glVertex3f( sz/2, -sz,  0);
//    glVertex3f( sz,  -sz/2,  0);
//    glVertex3f( sz,   sz/2,  0);
//    glVertex3f( sz/2,  sz,  0);
//    glVertex3f(-sz/2,  sz,  0);
//    glVertex3f(-sz,   sz/2,  0);
//    glVertex3f(-sz,  -sz/2,  0);
//    glEnd();

      // for speedup on wiz:
//    glEnable(GL_BLEND);

////    glColor4i(r, g, b, 220);
//    glColor4i(r, g, b, 220);
//
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz/2, -sz);
//    glVertex2f( sz/2, -sz);
//    glVertex2f( sz,  -sz/2);
//    glVertex2f( sz,   sz/2);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f( sz/2,  sz);
//    glVertex2f(-sz/2,  sz);
//    glVertex2f(-sz,   sz/2);
//    glVertex2f(-sz,  -sz/2);
//    glEnd();

      shapecolors[0] = shapecolors[4] = shapecolors[8] = shapecolors[12] = r;
      shapecolors[1] = shapecolors[5] = shapecolors[9] = shapecolors[13] = g;
      shapecolors[2] = shapecolors[6] = shapecolors[10] = shapecolors[14] = b;
      shapecolors[3] = shapecolors[7] = shapecolors[11] = shapecolors[15] =
         220;
      shapecolors[16] = shapecolors[20] = shapecolors[24] = shapecolors[28] =
         SHAPE_BASE_COLOR_R;
      shapecolors[17] = shapecolors[21] = shapecolors[25] = shapecolors[29] =
         SHAPE_BASE_COLOR_G;
      shapecolors[18] = shapecolors[22] = shapecolors[26] = shapecolors[30] =
         SHAPE_BASE_COLOR_B;
      shapecolors[19] = shapecolors[23] = shapecolors[27] = shapecolors[31] =
         150;
      shapevertices[0] = -fsz2;
      shapevertices[1] = -fsz;
      shapevertices[2] = fsz2;
      shapevertices[3] = -fsz;
      shapevertices[4] = fsz;
      shapevertices[5] = -fsz2;
      shapevertices[6] = fsz;
      shapevertices[7] = fsz2;
      shapevertices[8] = fsz2;
      shapevertices[9] = fsz;
      shapevertices[10] = -fsz2;
      shapevertices[11] = fsz;
      shapevertices[12] = -fsz;
      shapevertices[13] = fsz2;
      shapevertices[14] = -fsz;
      shapevertices[15] = -fsz2;
// glVertexPointer(2, GL_FIXED, 0, shapevertices);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
      glDrawArrays (GL_TRIANGLE_FAN, 0, 8);
      break;

   case 5:
      // triangle shape
      // tested working
//    sz = size*2/3; sz2 = size/5;
//  fsz = FDIV((size<<1), INT2FNUM(3));

      fsz = FMUL (size, 43691); //43690 = 2/3 in fixed point
//  fsz2 = FDIV(size, INT2FNUM(5));
      fsz2 = FMUL (size, 13107);    //13107 is 1/5 in fixed point

      // TEMP DEBUGGING:
//  if (!blah)
//  {
//     fsz = f2x(x2f(size) * 2.0 / 3.0);
//     fsz2 = f2x(x2f(size) / 5.0);
//     printf("orig size\n");
//  } else {
//     fsz = FMUL(size,43691);   //43690 = 2/3 in fixed point
////      fsz2 = FDIV(size, INT2FNUM(5));
//     fsz2 = FMUL(size,13107);  //13107 is 1/5 in fixed point
//     printf("alt size\n");
//  }
//  blah = !blah;

//    glRotatef((float)d*360/1024, 0, 0, 1);
      glRotatef ((float) ((d * 360) >> 10), 0, 0, 1);
//    if (shouldrotate) glRotatef((float)((d*360)>>10), 0, 0, 1);
      //same thing:
//    glRotatex(INT2FNUM((d*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((d*360)<<6, 0, 0, INT2FNUM(1));

      //senquack - no need for this
//    glDisable(GL_BLEND);
//    glBegin(GL_LINE_STRIP);
//    glVertex3f(-sz, -sz+sz2,  0);
//    glVertex3f( 0, sz+sz2,  0);
//    glVertex3f( sz, -sz+sz2,  0);
//    glEnd();

      // for speedup on wiz:
//    glEnable(GL_BLEND);

////    glColor4i(r, g, b, 150);
//    glColor4i(r, g, b, 150);
//
//  //senquack - converting to 2D for speed:
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz, -sz+sz2);
//    glVertex2f( sz, -sz+sz2);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f( 0, sz+sz2);
//    glEnd();

      shapecolors[0] = shapecolors[4] = r;
      shapecolors[1] = shapecolors[5] = g;
      shapecolors[2] = shapecolors[6] = b;
      shapecolors[3] = shapecolors[7] = shapecolors[11] = 150;
      shapecolors[8] = SHAPE_BASE_COLOR_R;
      shapecolors[9] = SHAPE_BASE_COLOR_G;
      shapecolors[10] = SHAPE_BASE_COLOR_B;
      shapevertices[0] = -fsz;
      shapevertices[1] = -fsz + fsz2;
      shapevertices[2] = fsz;
      shapevertices[3] = -fsz + fsz2;
      shapevertices[4] = 0;
      shapevertices[5] = fsz + fsz2;
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
// glVertexPointer(2, GL_FIXED, 0, shapevertices);
      glDrawArrays (GL_TRIANGLES, 0, 3);

      break;
   case 6:
      //senquack - hexagonal shape
//    sz = size/2;
      fsz = size >> 1;

      //senquack
//    glRotatef((float)((cnt*13)&1023)*360/1024, 0, 0, 1);
      glRotatef ((float) ((((cnt * 13) & 1023) * 360) >> 10), 0, 0, 1);
//    if (shouldrotate) glRotatef((float)((((cnt*13)&1023)*360)>>10), 0, 0, 1);
//    glRotatex(INT2FNUM((((cnt*13)&1023)*360)>>10), 0, 0, INT2FNUM(1));
//    glRotatex((((cnt*13)&1023)*360)<<6, 0, 0, INT2FNUM(1));

      //senquack - no need for this
//    glDisable(GL_BLEND);
//    glBegin(GL_LINE_LOOP);
//    glVertex3f(-sz, -sz,  0);
//    glVertex3f(  0, -sz,  0);
//    glVertex3f( sz,   0,  0);
//    glVertex3f( sz,  sz,  0);
//    glVertex3f(  0,  sz,  0);
//    glVertex3f(-sz,   0,  0);
//    glEnd();

      //senquack - converting to 2D for speed:

      // for speedup on wiz:
//    glEnable(GL_BLEND);

////    glColor4i(r, g, b, 210);
//    glColor4i(r, g, b, 210);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex3f(-sz, -sz,  0);
//    glVertex3f(  0, -sz,  0);
//    glVertex3f( sz,   0,  0);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex3f( sz,  sz,  0);
//    glVertex3f(  0,  sz,  0);
//    glVertex3f(-sz,   0,  0);
//    glEnd();

//    glEnable(GL_BLEND);
////    glColor4i(r, g, b, 210);
//    glColor4i(r, g, b, 210);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz, -sz);
//    glVertex2f(  0, -sz);
//    glVertex2f( sz,   0);
////    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glColor4i(SHAPE_BASE_COLOR_R, SHAPE_BASE_COLOR_G, SHAPE_BASE_COLOR_B, 150);
//    glVertex2f( sz,  sz);
//    glVertex2f(  0,  sz);
//    glVertex2f(-sz,   0);
//    glEnd();

      shapecolors[0] = shapecolors[4] = shapecolors[8] = r;
      shapecolors[1] = shapecolors[5] = shapecolors[9] = g;
      shapecolors[2] = shapecolors[6] = shapecolors[10] = b;
      shapecolors[3] = shapecolors[7] = shapecolors[11] = 210;
      shapecolors[12] = shapecolors[16] = shapecolors[20] =
         SHAPE_BASE_COLOR_R;
      shapecolors[13] = shapecolors[17] = shapecolors[21] =
         SHAPE_BASE_COLOR_G;
      shapecolors[14] = shapecolors[18] = shapecolors[22] =
         SHAPE_BASE_COLOR_B;
      shapecolors[15] = shapecolors[19] = shapecolors[23] = 150;
      shapevertices[0] = -fsz;
      shapevertices[1] = -fsz;
      shapevertices[2] = 0;
      shapevertices[3] = -fsz;
      shapevertices[4] = fsz;
      shapevertices[5] = 0;
      shapevertices[6] = fsz;
      shapevertices[7] = fsz;
      shapevertices[8] = 0;
      shapevertices[9] = fsz;
      shapevertices[10] = -fsz;
      shapevertices[11] = 0;
// glVertexPointer(2, GL_FIXED, 0, shapevertices);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, shapecolors);
      glDrawArrays (GL_TRIANGLE_FAN, 0, 6);

      break;
   }
   glPopMatrix ();
//  if (shouldrotate) glPopMatrix();
}

static int ikaClr[2][3][3] = {
   {{230, 230, 255}, {100, 100, 200}, {50, 50, 150}},
   {{0, 0, 0}, {200, 0, 0}, {100, 0, 0}},
};

//senquack - IKA mode causes immediate freezing too, trying to fix that:
//void drawShapeIka(GLfloat x, GLfloat y, GLfloat size, int d, int cnt, int type, int c) {
//  GLfloat sz, sz2, sz3;
//  glPushMatrix();
//  glTranslatef(x, y, 0);
//  glColor4i(ikaClr[c][0][0], ikaClr[c][0][1], ikaClr[c][0][2], 255);
//  glDisable(GL_BLEND);
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
//  glVertex3f( SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
//  glVertex3f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
//  glVertex3f(-SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
//  glEnd();
//  glColor4i(ikaClr[c][0][0], ikaClr[c][0][1], ikaClr[c][0][2], 255);
//  switch ( type ) {
//  case 0:
//    sz = size/2; sz2 = sz/3; sz3 = size*2/3;
//    glRotatef((float)d*360/1024, 0, 0, 1);
//    glBegin(GL_LINE_LOOP);
//    glVertex3f(-sz, -sz3,  0);
//    glVertex3f( sz, -sz3,  0);
//    glVertex3f( sz2, sz3,  0);
//    glVertex3f(-sz2, sz3,  0);
//    glEnd();
//    glEnable(GL_BLEND);
//    glColor4i(ikaClr[c][1][0], ikaClr[c][1][1], ikaClr[c][1][2], 250);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex3f(-sz, -sz3,  0);
//    glVertex3f( sz, -sz3,  0);
//    glColor4i(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
//    glVertex3f( sz2, sz3,  0);
//    glVertex3f(-sz2, sz3,  0);
//    glEnd();
//    break;
//  case 1:
//    sz = size/2;
//    glRotatef((float)((cnt*53)&1023)*360/1024, 0, 0, 1);
//    glBegin(GL_LINE_LOOP);
//    glVertex3f(-sz/2, -sz,  0);
//    glVertex3f( sz/2, -sz,  0);
//    glVertex3f( sz,  -sz/2,  0);
//    glVertex3f( sz,   sz/2,  0);
//    glVertex3f( sz/2,  sz,  0);
//    glVertex3f(-sz/2,  sz,  0);
//    glVertex3f(-sz,   sz/2,  0);
//    glVertex3f(-sz,  -sz/2,  0);
//    glEnd();
//    glEnable(GL_BLEND);
//    glColor4i(ikaClr[c][1][0], ikaClr[c][1][1], ikaClr[c][1][2], 250);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex3f(-sz/2, -sz,  0);
//    glVertex3f( sz/2, -sz,  0);
//    glVertex3f( sz,  -sz/2,  0);
//    glVertex3f( sz,   sz/2,  0);
//    glColor4i(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
//    glVertex3f( sz/2,  sz,  0);
//    glVertex3f(-sz/2,  sz,  0);
//    glVertex3f(-sz,   sz/2,  0);
//    glVertex3f(-sz,  -sz/2,  0);
//    glEnd();
//    break;
//  }
//  glPopMatrix();
//}
//void drawShapeIka(GLfloat x, GLfloat y, GLfloat size, int d, int cnt, int type, int c) {
//  GLfloat sz, sz2, sz3;
//  glPushMatrix();
//  glTranslatef(x, y, 0);
//  glColor4i(ikaClr[c][0][0], ikaClr[c][0][1], ikaClr[c][0][2], 255);
//  glDisable(GL_BLEND);
//
//  //senquack
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex3f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
////  glVertex3f( SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
////  glVertex3f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
////  glVertex3f(-SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
////  glEnd();
//  glBegin(GL_TRIANGLES);
//  glVertex3f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
//  glVertex3f( SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
//  glVertex3f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
//  glEnd();
//
//  glBegin(GL_TRIANGLES);
//  glVertex3f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
//  glVertex3f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
//  glVertex3f(-SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
//  glEnd();
//
//
//  glColor4i(ikaClr[c][0][0], ikaClr[c][0][1], ikaClr[c][0][2], 255);
//  switch ( type ) {
//  case 0:
//    sz = size/2; sz2 = sz/3; sz3 = size*2/3;
//    glRotatef((float)d*360/1024, 0, 0, 1);
//  //senquack
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz, -sz3,  0);
////    glVertex3f( sz, -sz3,  0);
////    glVertex3f( sz2, sz3,  0);
////    glVertex3f(-sz2, sz3,  0);
////    glEnd();
//    glBegin(GL_LINES);
//    glVertex2f(-sz, -sz3);
//    glVertex2f( sz, -sz3);
//  glEnd();
//
//  glBegin(GL_LINES);
//    glVertex2f( sz, -sz3);
//    glVertex2f( sz2, sz3);
//  glEnd();
//
//  glBegin(GL_LINES);
//    glVertex2f( sz2, sz3);
//    glVertex2f(-sz2, sz3);
//  glEnd();
//
//  glBegin(GL_LINES);
//    glVertex2f(-sz2, sz3);
//    glVertex2f(-sz, -sz3);
//    glEnd();
//
//    glEnable(GL_BLEND);
//    glColor4i(ikaClr[c][1][0], ikaClr[c][1][1], ikaClr[c][1][2], 250);
//
//  //senquack
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz, -sz3,  0);
////    glVertex3f( sz, -sz3,  0);
////    glColor4i(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
////    glVertex3f( sz2, sz3,  0);
////    glVertex3f(-sz2, sz3,  0);
////    glEnd();
//    glBegin(GL_TRIANGLES);
//    glVertex3f(-sz, -sz3,  0);
//    glVertex3f( sz, -sz3,  0);
//    glColor4i(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
//    glVertex3f( sz2, sz3,  0);
//  glEnd();
//
//  glBegin(GL_TRIANGLES);
//    glVertex3f(-sz, -sz3,  0);
//    glVertex3f( sz2, sz3,  0);
//    glVertex3f(-sz2, sz3,  0);
//    glEnd();
//    break;
//  case 1:
//    sz = size/2;
//    glRotatef((float)((cnt*53)&1023)*360/1024, 0, 0, 1);
//  //senquack
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz/2, -sz,  0);
////    glVertex3f( sz/2, -sz,  0);
////    glVertex3f( sz,  -sz/2,  0);
////    glVertex3f( sz,   sz/2,  0);
////    glVertex3f( sz/2,  sz,  0);
////    glVertex3f(-sz/2,  sz,  0);
////    glVertex3f(-sz,   sz/2,  0);
////    glVertex3f(-sz,  -sz/2,  0);
////    glEnd();
//    glBegin(GL_LINES);
//    glVertex2f(-sz/2, -sz);
//    glVertex2f( sz/2, -sz);
//  glEnd();
//
//  glBegin(GL_LINES);
//    glVertex2f( sz/2, -sz);
//    glVertex2f( sz,  -sz/2);
//  glEnd();
//
//  glBegin(GL_LINES);
//    glVertex2f( sz,  -sz/2);
//    glVertex2f( sz,   sz/2);
//  glEnd();
//
//  glBegin(GL_LINES);
//    glVertex2f( sz,   sz/2);
//    glVertex2f( sz/2,  sz);
//  glEnd();
//
//  glBegin(GL_LINES);
//    glVertex2f( sz/2,  sz);
//    glVertex2f(-sz/2,  sz);
//  glEnd();
//
//  glBegin(GL_LINES);
//    glVertex2f(-sz/2,  sz);
//    glVertex2f(-sz,   sz/2);
//  glEnd();
//
//  glBegin(GL_LINES);
//    glVertex2f(-sz,   sz/2);
//    glVertex2f(-sz,  -sz/2);
//  glEnd();
//
//  glBegin(GL_LINES);
//    glVertex2f(-sz,  -sz/2);
//    glVertex2f(-sz/2, -sz);
//    glEnd();
//
//
//    glEnable(GL_BLEND);
//    glColor4i(ikaClr[c][1][0], ikaClr[c][1][1], ikaClr[c][1][2], 250);
//
//  //senquack
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz/2, -sz,  0);
////    glVertex3f( sz/2, -sz,  0);
////    glVertex3f( sz,  -sz/2,  0);
////    glVertex3f( sz,   sz/2,  0);
////    glColor4i(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
////    glVertex3f( sz/2,  sz,  0);
////    glVertex3f(-sz/2,  sz,  0);
////    glVertex3f(-sz,   sz/2,  0);
////    glVertex3f(-sz,  -sz/2,  0);
////    glEnd();
//    glBegin(GL_TRIANGLES);
//    glVertex3f(-sz/2, -sz,  0);
//    glVertex3f( sz/2, -sz,  0);
//    glVertex3f( sz,  -sz/2,  0);
//  glEnd();
//
//  glBegin(GL_TRIANGLES);
//    glVertex3f(-sz/2, -sz,  0);
//    glVertex3f( sz,  -sz/2,  0);
//    glVertex3f( sz,   sz/2,  0);
//  glEnd();
//
//    glColor4i(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
//
//  glBegin(GL_TRIANGLES);
//    glVertex3f(-sz/2, -sz,  0);
//    glVertex3f( sz,   sz/2,  0);
//    glVertex3f( sz/2,  sz,  0);
//  glEnd();
//
//  glBegin(GL_TRIANGLES);
//    glVertex3f(-sz/2, -sz,  0);
//    glVertex3f( sz/2,  sz,  0);
//    glVertex3f(-sz/2,  sz,  0);
//  glEnd();
//
//  glBegin(GL_TRIANGLES);
//    glVertex3f(-sz/2, -sz,  0);
//    glVertex3f(-sz/2,  sz,  0);
//    glVertex3f(-sz,   sz/2,  0);
//  glEnd();
//
//  glBegin(GL_TRIANGLES);
//    glVertex3f(-sz/2, -sz,  0);
//    glVertex3f(-sz,   sz/2,  0);
//    glVertex3f(-sz,  -sz/2,  0);
//    glEnd();
//    break;
//  }
//  glPopMatrix();
//}
//senquack - converted to fixed point
//void drawShapeIka(GLfloat x, GLfloat y, GLfloat size, int d, int cnt, int type, int c) {
//  GLfloat sz, sz2, sz3;
//  glPushMatrix();
//  glTranslatef(x, y, 0);
////  glColor4i(ikaClr[c][0][0], ikaClr[c][0][1], ikaClr[c][0][2], 255);
//  glColor4i(ikaClr[c][0][0], ikaClr[c][0][1], ikaClr[c][0][2], 255);
//  glDisable(GL_BLEND);
//
//  //senquack - converting to 2D for speed
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex3f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
////  glVertex3f( SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE,  0);
////  glVertex3f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
////  glVertex3f(-SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE,  0);
////  glEnd();
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex2f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE);
//  glVertex2f( SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE);
//  glVertex2f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE);
//  glVertex2f(-SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE);
//  glEnd();
//
////  glColor4i(ikaClr[c][0][0], ikaClr[c][0][1], ikaClr[c][0][2], 255);
//  glColor4i(ikaClr[c][0][0], ikaClr[c][0][1], ikaClr[c][0][2], 255);
//  switch ( type ) {
//  case 0:
//    sz = size/2; sz2 = sz/3; sz3 = size*2/3;
//    glRotatef((float)d*360/1024, 0, 0, 1);
//  //senquack - no need for this
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz, -sz3,  0);
////    glVertex3f( sz, -sz3,  0);
////    glVertex3f( sz2, sz3,  0);
////    glVertex3f(-sz2, sz3,  0);
////    glEnd();
//    glEnable(GL_BLEND);
////    glColor4i(ikaClr[c][1][0], ikaClr[c][1][1], ikaClr[c][1][2], 250);
//    glColor4i(ikaClr[c][1][0], ikaClr[c][1][1], ikaClr[c][1][2], 250);
//
//  //senquack - converting to 2D for speed
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz, -sz3,  0);
////    glVertex3f( sz, -sz3,  0);
//////    glColor4i(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
////    glColor4i(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
////    glVertex3f( sz2, sz3,  0);
////    glVertex3f(-sz2, sz3,  0);
////    glEnd();
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz, -sz3);
//    glVertex2f( sz, -sz3);
////    glColor4i(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
//    glColor4i(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
//    glVertex2f( sz2, sz3);
//    glVertex2f(-sz2, sz3);
//    glEnd();
//    break;
//  case 1:
//    sz = size/2;
//    glRotatef((float)((cnt*53)&1023)*360/1024, 0, 0, 1);
//  //senquack - no need for this
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(-sz/2, -sz,  0);
////    glVertex3f( sz/2, -sz,  0);
////    glVertex3f( sz,  -sz/2,  0);
////    glVertex3f( sz,   sz/2,  0);
////    glVertex3f( sz/2,  sz,  0);
////    glVertex3f(-sz/2,  sz,  0);
////    glVertex3f(-sz,   sz/2,  0);
////    glVertex3f(-sz,  -sz/2,  0);
////    glEnd();
//    glEnable(GL_BLEND);
////    glColor4i(ikaClr[c][1][0], ikaClr[c][1][1], ikaClr[c][1][2], 250);
//    glColor4i(ikaClr[c][1][0], ikaClr[c][1][1], ikaClr[c][1][2], 250);
//
//  //senquack - converting to 2D for speed:
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex3f(-sz/2, -sz,  0);
////    glVertex3f( sz/2, -sz,  0);
////    glVertex3f( sz,  -sz/2,  0);
////    glVertex3f( sz,   sz/2,  0);
//////    glColor4i(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
////    glColor4i(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
////    glVertex3f( sz/2,  sz,  0);
////    glVertex3f(-sz/2,  sz,  0);
////    glVertex3f(-sz,   sz/2,  0);
////    glVertex3f(-sz,  -sz/2,  0);
////    glEnd();
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz/2, -sz);
//    glVertex2f( sz/2, -sz);
//    glVertex2f( sz,  -sz/2);
//    glVertex2f( sz,   sz/2);
////    glColor4i(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
//    glColor4i(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
//    glVertex2f( sz/2,  sz);
//    glVertex2f(-sz/2,  sz);
//    glVertex2f(-sz,   sz/2);
//    glVertex2f(-sz,  -sz/2);
//    glEnd();
//    break;
//  }
//  glPopMatrix();
//}
//void drawShapeIkax(GLfixed x, GLfixed y, GLfixed size, int d, int cnt, int type, int c) {
//  GLfixed fsz, fsz2, fsz3;
//  glPushMatrix();
//  glTranslatex(x, y, 0);
////  glColor4i(ikaClr[c][0][0], ikaClr[c][0][1], ikaClr[c][0][2], 255);
//  glDisable(GL_BLEND);
//
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex2f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE);
////  glVertex2f( SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE);
////  glVertex2f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE);
////  glVertex2f(-SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE);
////  glEnd();
// shapecolors[0] = shapecolors[4] = shapecolors[8] = shapecolors[12] = ikaClr[c][0][0];
// shapecolors[1] = shapecolors[5] = shapecolors[9] = shapecolors[13] = ikaClr[c][0][1];
// shapecolors[2] = shapecolors[6] = shapecolors[10] = shapecolors[14] = ikaClr[c][0][2];
// shapecolors[3] = shapecolors[7] = shapecolors[11] = shapecolors[15] = 255;
//
// shapevertices[0] = -SHAPE_POINT_SIZE_X;   shapevertices[1] = -SHAPE_POINT_SIZE_X;
// shapevertices[2] = SHAPE_POINT_SIZE_X;    shapevertices[3] = -SHAPE_POINT_SIZE_X;
// shapevertices[4] = SHAPE_POINT_SIZE_X;    shapevertices[5] = SHAPE_POINT_SIZE_X;
// shapevertices[6] = -SHAPE_POINT_SIZE_X;   shapevertices[7] = SHAPE_POINT_SIZE_X;
// glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//
////  glColor4i(ikaClr[c][0][0], ikaClr[c][0][1], ikaClr[c][0][2], 255);
//    glEnable(GL_BLEND);
//
//  switch ( type ) {
//  case 0:
////    sz = size/2; sz2 = sz/3; sz3 = size*2/3;
//    fsz = size>>1; fsz2 = FMUL(fsz,21845); fsz3 = FMUL(size,43691);   // 43690 = 2/3 in fixed point
////    glRotatef((float)d*360/1024, 0, 0, 1);
//    glRotatex((d*360)<<6, 0, 0, INT2FNUM(1));
//
////    glColor4i(ikaClr[c][1][0], ikaClr[c][1][1], ikaClr[c][1][2], 250);
//
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex2f(-sz, -sz3);
////    glVertex2f( sz, -sz3);
//////    glColor4i(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
////    glVertex2f( sz2, sz3);
////    glVertex2f(-sz2, sz3);
//  
// shapecolors[0] = shapecolors[4] = ikaClr[c][1][0];
// shapecolors[1] = shapecolors[5] = ikaClr[c][1][1];
// shapecolors[2] = shapecolors[6] = ikaClr[c][1][2];
//
// shapecolors[8] = shapecolors[12] = ikaClr[c][2][0];
// shapecolors[9] = shapecolors[13] = ikaClr[c][2][1];
// shapecolors[10] = shapecolors[14] = ikaClr[c][2][2];
//
// shapecolors[3] = shapecolors[7] = shapecolors[11] = shapecolors[15] = 250;
//
// shapevertices[0] = -fsz;   shapevertices[1] = -fsz3;
// shapevertices[2] = fsz;    shapevertices[3] = -fsz3;
// shapevertices[4] = fsz2;   shapevertices[5] = fsz3;
// shapevertices[6] = -fsz2;  shapevertices[7] = fsz3;
// glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//
////    glEnd();
//    break;
//  case 1:
////    sz = size/2;
// fsz = size>>1;
// fsz2 = size>>2;   // added this because of simplicity later on
////    glRotatef((float)((cnt*53)&1023)*360/1024, 0, 0, 1);
//    glRotatex((((cnt*53)&1023)*360)<<6, 0, 0, INT2FNUM(1));
//
//
////    glColor4i(ikaClr[c][1][0], ikaClr[c][1][1], ikaClr[c][1][2], 250);
////    glBegin(GL_TRIANGLE_FAN);
////    glVertex2f(-sz/2, -sz);
////    glVertex2f( sz/2, -sz);
////    glVertex2f( sz,  -sz/2);
////    glVertex2f( sz,   sz/2);
////    glColor4i(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
////    glVertex2f( sz/2,  sz);
////    glVertex2f(-sz/2,  sz);
////    glVertex2f(-sz,   sz/2);
////    glVertex2f(-sz,  -sz/2);
// shapecolors[0] = shapecolors[4] = shapecolors[8] = shapecolors[12] = ikaClr[c][1][0];
// shapecolors[1] = shapecolors[5] = shapecolors[9] = shapecolors[13] = ikaClr[c][1][1];
// shapecolors[2] = shapecolors[6] = shapecolors[10] = shapecolors[14] = ikaClr[c][1][2];
// shapecolors[16] = shapecolors[20] = shapecolors[24] = shapecolors[28] = ikaClr[c][2][0];
// shapecolors[17] = shapecolors[21] = shapecolors[25] = shapecolors[29] = ikaClr[c][2][1];
// shapecolors[18] = shapecolors[22] = shapecolors[26] = shapecolors[30] = ikaClr[c][2][2];
// shapecolors[3] = shapecolors[7] = shapecolors[11] = shapecolors[15] = 
// shapecolors[19] = shapecolors[23] = shapecolors[27] = shapecolors[31] = 250;
// shapevertices[0] = -fsz2;  shapevertices[1] = -fsz;   
// shapevertices[2] = fsz2;   shapevertices[3] = -fsz;
// shapevertices[4] = fsz;    shapevertices[5] = -fsz2;
// shapevertices[6] = fsz;    shapevertices[7] = fsz2;
// shapevertices[8] = fsz2;   shapevertices[9] = fsz; 
// shapevertices[10] = -fsz2; shapevertices[11] = fsz;
// shapevertices[12] = -fsz;     shapevertices[13] = fsz2;
// shapevertices[14] = -fsz;     shapevertices[15] = -fsz2;
// glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
////    glEnd();
//    break;
//  }
//  glPopMatrix();
//}
//senquack - move point drawing to the end so it can all be done in one batch:
void
drawShapeIkax (GLfixed fx, GLfixed fy, GLfixed size, int d, int cnt, int type,
               int c)
{
   GLfixed fsz, fsz2, fsz3;

//  //senquack - experiment
//  if (lastd == d) {
////    samectr++;
//   shouldrotate = 0;
//  } else {
////    diffctr++;
//   shouldrotate = 1;
//  }
//  lastd = d;

   glPushMatrix ();
//  if (shouldrotate) glPushMatrix();
   glTranslatex (fx, fy, 0);
//  glColor4i(ikaClr[c][0][0], ikaClr[c][0][1], ikaClr[c][0][2], 255);
//  glDisable(GL_BLEND);

////  glBegin(GL_TRIANGLE_FAN);
////  glVertex2f(-SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE);
////  glVertex2f( SHAPE_POINT_SIZE, -SHAPE_POINT_SIZE);
////  glVertex2f( SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE);
////  glVertex2f(-SHAPE_POINT_SIZE,  SHAPE_POINT_SIZE);
////  glEnd();
// shapecolors[0] = shapecolors[4] = shapecolors[8] = shapecolors[12] = ikaClr[c][0][0];
// shapecolors[1] = shapecolors[5] = shapecolors[9] = shapecolors[13] = ikaClr[c][0][1];
// shapecolors[2] = shapecolors[6] = shapecolors[10] = shapecolors[14] = ikaClr[c][0][2];
// shapecolors[3] = shapecolors[7] = shapecolors[11] = shapecolors[15] = 255;
//
// shapevertices[0] = -SHAPE_POINT_SIZE_X;   shapevertices[1] = -SHAPE_POINT_SIZE_X;
// shapevertices[2] = SHAPE_POINT_SIZE_X;    shapevertices[3] = -SHAPE_POINT_SIZE_X;
// shapevertices[4] = SHAPE_POINT_SIZE_X;    shapevertices[5] = SHAPE_POINT_SIZE_X;
// shapevertices[6] = -SHAPE_POINT_SIZE_X;   shapevertices[7] = SHAPE_POINT_SIZE_X;
// glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
   GLubyte r, g, b;
   r = ikaClr[c][0][0];
   g = ikaClr[c][0][1];
   b = ikaClr[c][0][2];
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 192;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 192;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 192;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 192;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 192;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 192;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 128;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 128;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 128;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 128;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 128;
// *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 128;
   *shapeptcolptr++ = r;
   *shapeptcolptr++ = g;
   *shapeptcolptr++ = b;
   *shapeptcolptr++ = 96;
   *shapeptcolptr++ = r;
   *shapeptcolptr++ = g;
   *shapeptcolptr++ = b;
   *shapeptcolptr++ = 96;
   *shapeptcolptr++ = r;
   *shapeptcolptr++ = g;
   *shapeptcolptr++ = b;
   *shapeptcolptr++ = 96;
   *shapeptcolptr++ = r;
   *shapeptcolptr++ = g;
   *shapeptcolptr++ = b;
   *shapeptcolptr++ = 96;
   *shapeptcolptr++ = r;
   *shapeptcolptr++ = g;
   *shapeptcolptr++ = b;
   *shapeptcolptr++ = 96;
   *shapeptcolptr++ = r;
   *shapeptcolptr++ = g;
   *shapeptcolptr++ = b;
   *shapeptcolptr++ = 96;
   *shapeptvertptr++ = fx - SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fy - SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fx - SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fy + SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fx + SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fy - SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fx + SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fy + SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fx + SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fy - SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fx - SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = fy + SHAPE_POINT_SIZE_X;

//  glColor4i(ikaClr[c][0][0], ikaClr[c][0][1], ikaClr[c][0][2], 255);
//    glEnable(GL_BLEND);

   switch (type) {
   case 0:
//    sz = size/2; sz2 = sz/3; sz3 = size*2/3;
      fsz = size >> 1;
      fsz2 = FMUL (fsz, 21845);
      fsz3 = FMUL (size, 43691);    // 43690 = 2/3 in fixed point
//    glRotatef((float)d*360/1024, 0, 0, 1);
      glRotatef ((float) ((d * 360) >> 10), 0, 0, 1);
//    if (shouldrotate) glRotatef((float)((d*360)>>10), 0, 0, 1);
//    glRotatex((d*360)<<6, 0, 0, INT2FNUM(1));

//    glColor4i(ikaClr[c][1][0], ikaClr[c][1][1], ikaClr[c][1][2], 250);

//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz, -sz3);
//    glVertex2f( sz, -sz3);
////    glColor4i(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
//    glVertex2f( sz2, sz3);
//    glVertex2f(-sz2, sz3);

      shapecolors[0] = shapecolors[4] = ikaClr[c][1][0];
      shapecolors[1] = shapecolors[5] = ikaClr[c][1][1];
      shapecolors[2] = shapecolors[6] = ikaClr[c][1][2];

      shapecolors[8] = shapecolors[12] = ikaClr[c][2][0];
      shapecolors[9] = shapecolors[13] = ikaClr[c][2][1];
      shapecolors[10] = shapecolors[14] = ikaClr[c][2][2];

      shapecolors[3] = shapecolors[7] = shapecolors[11] = shapecolors[15] =
         250;

      shapevertices[0] = -fsz;
      shapevertices[1] = -fsz3;
      shapevertices[2] = fsz;
      shapevertices[3] = -fsz3;
      shapevertices[4] = fsz2;
      shapevertices[5] = fsz3;
      shapevertices[6] = -fsz2;
      shapevertices[7] = fsz3;
      glDrawArrays (GL_TRIANGLE_FAN, 0, 4);

//    glEnd();
      break;
   case 1:
//    sz = size/2;
      fsz = size >> 1;
      fsz2 = size >> 2;         // added this because of simplicity later on
//    glRotatef((float)((cnt*53)&1023)*360/1024, 0, 0, 1);
      glRotatef ((float) ((((cnt * 53) & 1023) * 360) >> 10), 0, 0, 1);
//    if (shouldrotate) glRotatef((float)((((cnt*53)&1023)*360)>>10), 0, 0, 1);
//    glRotatex((((cnt*53)&1023)*360)<<6, 0, 0, INT2FNUM(1));

//    glColor4i(ikaClr[c][1][0], ikaClr[c][1][1], ikaClr[c][1][2], 250);
//    glBegin(GL_TRIANGLE_FAN);
//    glVertex2f(-sz/2, -sz);
//    glVertex2f( sz/2, -sz);
//    glVertex2f( sz,  -sz/2);
//    glVertex2f( sz,   sz/2);
//    glColor4i(ikaClr[c][2][0], ikaClr[c][2][1], ikaClr[c][2][2], 250);
//    glVertex2f( sz/2,  sz);
//    glVertex2f(-sz/2,  sz);
//    glVertex2f(-sz,   sz/2);
//    glVertex2f(-sz,  -sz/2);
      shapecolors[0] = shapecolors[4] = shapecolors[8] = shapecolors[12] =
         ikaClr[c][1][0];
      shapecolors[1] = shapecolors[5] = shapecolors[9] = shapecolors[13] =
         ikaClr[c][1][1];
      shapecolors[2] = shapecolors[6] = shapecolors[10] = shapecolors[14] =
         ikaClr[c][1][2];
      shapecolors[16] = shapecolors[20] = shapecolors[24] = shapecolors[28] =
         ikaClr[c][2][0];
      shapecolors[17] = shapecolors[21] = shapecolors[25] = shapecolors[29] =
         ikaClr[c][2][1];
      shapecolors[18] = shapecolors[22] = shapecolors[26] = shapecolors[30] =
         ikaClr[c][2][2];
      shapecolors[3] = shapecolors[7] = shapecolors[11] = shapecolors[15] =
         shapecolors[19] = shapecolors[23] = shapecolors[27] =
         shapecolors[31] = 250;
      shapevertices[0] = -fsz2;
      shapevertices[1] = -fsz;
      shapevertices[2] = fsz2;
      shapevertices[3] = -fsz;
      shapevertices[4] = fsz;
      shapevertices[5] = -fsz2;
      shapevertices[6] = fsz;
      shapevertices[7] = fsz2;
      shapevertices[8] = fsz2;
      shapevertices[9] = fsz;
      shapevertices[10] = -fsz2;
      shapevertices[11] = fsz;
      shapevertices[12] = -fsz;
      shapevertices[13] = fsz2;
      shapevertices[14] = -fsz;
      shapevertices[15] = -fsz2;
      glDrawArrays (GL_TRIANGLE_FAN, 0, 8);
//    glEnd();
      break;
   }
   glPopMatrix ();
//  if (shouldrotate) glPopMatrix();
}

#define SHOT_WIDTH 0.1
#define SHOT_HEIGHT 0.2

static int shtClr[3][3][3] = {
   {{200, 200, 225}, {50, 50, 200}, {200, 200, 225}},
   {{100, 0, 0}, {100, 0, 0}, {200, 0, 0}},
   {{100, 200, 100}, {50, 100, 50}, {100, 200, 100}},
};

//senquack - 2/11
//void drawShot(GLfloat x, GLfloat y, GLfloat d, int c, float width, float height) {
//  glPushMatrix();
//  glTranslatef(x, y, 0);
//  glRotatef(d, 0, 0, 1);
//  glColor4i(shtClr[c][0][0], shtClr[c][0][1], shtClr[c][0][2], 240);
//  glDisable(GL_BLEND);
//  glBegin(GL_LINES);
//  glVertex3f(-width, -height, 0);
//  glVertex3f(-width,  height, 0);
//  glVertex3f( width, -height, 0);
//  glVertex3f( width,  height, 0);
//  glEnd();
//  glEnable(GL_BLEND);
//
//  glColor4i(shtClr[c][1][0], shtClr[c][1][1], shtClr[c][1][2], 240);
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(-width, -height, 0);
//  glVertex3f( width, -height, 0);
//  glColor4i(shtClr[c][2][0], shtClr[c][2][1], shtClr[c][2][2], 240);
//  glVertex3f( width,  height, 0);
//  glVertex3f(-width,  height, 0);
//  glEnd();
//  glPopMatrix();
//}
//void drawShot(GLfloat x, GLfloat y, GLfloat d, int c, float width, float height) {
//  glPushMatrix();
//  glTranslatef(x, y, 0);
//  glRotatef(d, 0, 0, 1);
//
//  //senquack - no need for this
////  glColor4i(shtClr[c][0][0], shtClr[c][0][1], shtClr[c][0][2], 240);
////  glDisable(GL_BLEND);
////  glBegin(GL_LINE_LOOP);
////  glVertex2f(-width, -height);
////  glVertex2f(-width,  height);
////  glEnd();
////  glBegin(GL_LINE_LOOP);
////  glVertex2f( width, -height);
////  glVertex2f( width,  height);
////  glEnd();
////  glEnable(GL_BLEND);
//
//  glColor4i(shtClr[c][1][0], shtClr[c][1][1], shtClr[c][1][2], 240);
//
//  //senquack
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex3f(-width, -height, 0);
////  glVertex3f( width, -height, 0);
////  glColor4i(shtClr[c][2][0], shtClr[c][2][1], shtClr[c][2][2], 240);
////  glVertex3f( width,  height, 0);
////  glVertex3f(-width,  height, 0);
////  glEnd();
//  // 2D version (hopefully a tiny bit faster:)
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex2f(-width, -height);
//  glVertex2f( width, -height);
//  glColor4i(shtClr[c][2][0], shtClr[c][2][1], shtClr[c][2][2], 240);
//  glVertex2f( width,  height);
//  glVertex2f(-width,  height);
//  glEnd();
//
//  //senquack - converted:
////  glBegin(GL_TRIANGLES);
////  glVertex3f(-width, -height, 0);
////  glVertex3f( width, -height, 0);
////  glColor4i(shtClr[c][2][0], shtClr[c][2][1], shtClr[c][2][2], 240);
////  glVertex3f( width,  height, 0);
////  glEnd();
////  glBegin(GL_TRIANGLES);
////  glVertex3f(-width, -height, 0);
////  glVertex3f( width,  height, 0);
////  glVertex3f(-width,  height, 0);
////  glEnd();
//
//  glPopMatrix();
//}
//senquack - converted to fixed point:
void
drawShotx (GLfixed x, GLfixed y, GLfixed d, int c, GLfixed width,
           GLfixed height)
{
   GLubyte colors[4 * 4];
   GLfixed vertices[4 * 2];

   glPushMatrix ();
   glTranslatex (x, y, 0);
   glRotatex (d, 0, 0, INT2FNUM (1));

   //senquack - no need for this
//  glColor4i(shtClr[c][0][0], shtClr[c][0][1], shtClr[c][0][2], 240);
//  glDisable(GL_BLEND);
//  glBegin(GL_LINE_LOOP);
//  glVertex2f(-width, -height);
//  glVertex2f(-width,  height);
//  glEnd();
//  glBegin(GL_LINE_LOOP);
//  glVertex2f( width, -height);
//  glVertex2f( width,  height);
//  glEnd();
//  glEnable(GL_BLEND);

   glVertexPointer (2, GL_FIXED, 0, vertices);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);

   colors[0] = colors[4] = colors[8] = colors[12] = shtClr[c][0][0];
   colors[1] = colors[5] = colors[9] = colors[13] = shtClr[c][0][1];
   colors[2] = colors[6] = colors[10] = colors[14] = shtClr[c][0][2];
   colors[3] = colors[7] = colors[11] = colors[15] = 240;
   vertices[0] = -width;
   vertices[1] = -height;
   vertices[2] = -width;
   vertices[3] = height;
   vertices[4] = width;
   vertices[5] = -height;
   vertices[6] = width;
   vertices[7] = height;
   glDrawArrays (GL_LINE_LOOP, 0, 4);

//  glColor4i(shtClr[c][1][0], shtClr[c][1][1], shtClr[c][1][2], 240);
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex2f(-width, -height);
//  glVertex2f( width, -height);
//  glColor4i(shtClr[c][2][0], shtClr[c][2][1], shtClr[c][2][2], 240);
//  glVertex2f( width,  height);
//  glVertex2f(-width,  height);
//  glEnd();

   colors[0] = colors[4] = shtClr[c][1][0];
   colors[1] = colors[5] = shtClr[c][1][1];
   colors[2] = colors[6] = shtClr[c][1][2];

   colors[8] = colors[12] = shtClr[c][2][0];
   colors[9] = colors[13] = shtClr[c][2][1];
   colors[10] = colors[14] = shtClr[c][2][2];
   //senquack - redundant
// colors[3] = colors[7] = colors[11] = colors[15] = 240;
   vertices[0] = -width;
   vertices[1] = -height;
   vertices[2] = width;
   vertices[3] = -height;
   vertices[4] = width;
   vertices[5] = height;
   vertices[6] = -width;
   vertices[7] = height;
   glDrawArrays (GL_TRIANGLE_FAN, 0, 4);
   glPopMatrix ();
}

//senquack - why are there two pushmatrixes here but only one popmatrix in the endDrawBoards() function??
//void startDrawBoards() {
//  glMatrixMode(GL_PROJECTION);
//  glPushMatrix();
//  glLoadIdentity();
//  glOrtho(0, 640, 480, 0, -1, 1);
//  glMatrixMode(GL_MODELVIEW);
//  glPushMatrix();
//  glLoadIdentity();
//}
//void startDrawBoards() {
//  glMatrixMode(GL_PROJECTION);
//  //senquack - dunno why we do this
////  glPushMatrix();
//  glLoadIdentity();
//  glOrtho(0, 640, 480, 0, -1, 1);
//  glMatrixMode(GL_MODELVIEW);
//  glPushMatrix();
//  glLoadIdentity();
//}
//senquack - converted to opengles
void
startDrawBoards ()
{
   glMatrixMode (GL_PROJECTION);
   //senquack - dunno why we do this
//  glPushMatrix();
   glLoadIdentity ();
   glOrthox (INT2FNUM (0), INT2FNUM (640), INT2FNUM (480), 0, INT2FNUM (-1),
             INT2FNUM (1));
   glMatrixMode (GL_MODELVIEW);
   glPushMatrix ();
   glLoadIdentity ();
}

//senquack
//void endDrawBoards() {
//  glPopMatrix();
//  screenResized();
//}
void
endDrawBoards ()
{
   //senquack - box drawing is now done as a batch:
   finishDrawBoxes ();

   glPopMatrix ();
   screenResized ();
}

//senquack - converted to openGLES
//static void drawBoard(int x, int y, int width, int height) {
//  glColor4i(0, 0, 0, 255);
//  glBegin(GL_QUADS);
//  glVertex2f(x,y);
//  glVertex2f(x+width,y);
//  glVertex2f(x+width,y+height);
//  glVertex2f(x,y+height);
//  glEnd();
//}
static void
drawBoard (int x, int y, int width, int height)
{
//  glColor4i(0, 0, 0, 255);

   //senquack
//  glBegin(GL_QUADS);
//  glVertex2f(x,y);
//  glVertex2f(x+width,y);
//  glVertex2f(x+width,y+height);
//  glVertex2f(x,y+height);
//  glEnd();
   GLubyte colors[4 * 4];
   GLfixed vertices[4 * 2];
   colors[0] = colors[4] = colors[8] = colors[12] = 0;
   colors[1] = colors[5] = colors[9] = colors[13] = 0;
   colors[2] = colors[6] = colors[10] = colors[14] = 0;
   colors[3] = colors[7] = colors[11] = colors[15] = 255;

   vertices[0] = INT2FNUM (x);
   vertices[1] = INT2FNUM (y);
   vertices[2] = INT2FNUM (x + width);
   vertices[3] = INT2FNUM (y);
   vertices[4] = INT2FNUM (x + width);
   vertices[5] = INT2FNUM (y + height);
   vertices[6] = INT2FNUM (x);
   vertices[7] = INT2FNUM (y + height);


// glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer (2, GL_FIXED, 0, vertices);
// glEnableClientState(GL_COLOR_ARRAY);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
   glDrawArrays (GL_TRIANGLE_FAN, 0, 4);
}

//senquack - 2/11
//void drawSideBoards() {
//  glDisable(GL_BLEND);
//  drawBoard(0, 0, 160, 480);
//  drawBoard(480, 0, 160, 480);
//  glEnable(GL_BLEND);
//  drawScore();
//  drawRPanel();
//}
//void drawSideBoards() {
// //senquack - since we aren't drawing the background anymore, no need for these
//  glDisable(GL_BLEND);
//  drawBoard(0, 0, 160, 480);
//  drawBoard(480, 0, 160, 480);
//  glEnable(GL_BLEND);
//  drawScore();
//  drawRPanel();
//}
void
drawSideBoards ()
{
// if (screenRotated) {
   if (settings.rotated) {
      glEnable (GL_BLEND);

      //senquack - box drawing is now done as a batch:
      prepareDrawBoxes ();

      drawRPanel_rotated ();
   } else {
      glDisable (GL_BLEND);
      drawBoard (0, 0, 160, 480);
      drawBoard (480, 0, 160, 480);
      glEnable (GL_BLEND);

      //senquack - box drawing is now done as a batch:
      prepareDrawBoxes ();

      drawScore ();
      drawRPanel ();
   }
}

// 2/11 - new efforts to convert all triangle fans and line loops to something else:
//senquack
//void drawTitleBoard() {
//  
// //senquack
//////  printf("drawTitleBoard() start\n");
////  //glEnable(GL_TEXTURE_2D);
////  printf("*");
////  //glBindTexture(GL_TEXTURE_2D, titleTexture);
////  printf("*");
////  glColor4i(255, 255, 255, 255);
////  printf("*");
////  glBegin(GL_TRIANGLE_FAN);
////  glTexCoord2f(0.0f, 0.0f);
////  glVertex3f(350, 78,  0);
////  glTexCoord2f(1.0f, 0.0f);
////  glVertex3f(470, 78,  0);
////  glTexCoord2f(1.0f, 1.0f);
////  glVertex3f(470, 114,  0);
////  glTexCoord2f(0.0f, 1.0f);
////  glVertex3f(350, 114,  0);
////  printf("*");
////  glEnd();
//  
//  //glDisable(GL_TEXTURE_2D); //TODO: Make sure to uncomment this line if I fix texturing....
//  printf("*");
//  glColor4i(200, 200, 200, 255);
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(350, 30, 0);
//  glVertex3f(400, 30, 0);
//  glVertex3f(380, 56, 0);
//  glVertex3f(380, 80, 0);
//  glVertex3f(350, 80, 0);
//  glEnd();
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(404, 80, 0);
//  glVertex3f(404, 8, 0);
//  glVertex3f(440, 8, 0);
//  glVertex3f(440, 44, 0);
//  glVertex3f(465, 80, 0);
//  glEnd();
//  glColor4i(255, 255, 255, 255);
//  glBegin(GL_LINE_LOOP);
//  glVertex3f(350, 30, 0);
//  glVertex3f(400, 30, 0);
//  glVertex3f(380, 56, 0);
//  glVertex3f(380, 80, 0);
//  glVertex3f(350, 80, 0);
//  glEnd();
//  glBegin(GL_LINE_LOOP);
//  glVertex3f(404, 80, 0);
//  glVertex3f(404, 8, 0);
//  glVertex3f(440, 8, 0);
//  glVertex3f(440, 44, 0);
//  glVertex3f(465, 80, 0);
//  glEnd();
//  //senquack
////  printf("Done drawing drawTitleBoard\n"); fflush(stdout);
//}
//senquack - 2nd try
//void drawTitleBoard() {
//  
// //senquack
////  printf("drawTitleBoard() start\n");
//  //glEnable(GL_TEXTURE_2D);
//  printf("*");
//  //glBindTexture(GL_TEXTURE_2D, titleTexture);
//  printf("*");
//  glColor4i(255, 255, 255, 255);
//  printf("*");
//  glBegin(GL_TRIANGLE_FAN);
//  glTexCoord2f(0.0f, 0.0f);
//  glVertex3f(350, 78,  0);
//  glTexCoord2f(1.0f, 0.0f);
//  glVertex3f(470, 78,  0);
//  glTexCoord2f(1.0f, 1.0f);
//  glVertex3f(470, 114,  0);
//  glTexCoord2f(0.0f, 1.0f);
//  glVertex3f(350, 114,  0);
//  printf("*");
//  glEnd();
//  
//  //glDisable(GL_TEXTURE_2D); //TODO: Make sure to uncomment this line if I fix texturing....
//  printf("*");
//  glColor4i(200, 200, 200, 255);
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(350, 30, 0);
//  glVertex3f(400, 30, 0);
//  glVertex3f(380, 56, 0);
//  glVertex3f(380, 80, 0);
//  glVertex3f(350, 80, 0);
//  glEnd();
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(404, 80, 0);
//  glVertex3f(404, 8, 0);
//  glVertex3f(440, 8, 0);
//  glVertex3f(440, 44, 0);
//  glVertex3f(465, 80, 0);
//  glEnd();
//  glColor4i(255, 255, 255, 255);
//
//  //senquack
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(350, 30, 0);
////  glVertex3f(400, 30, 0);
////  glVertex3f(380, 56, 0);
////  glVertex3f(380, 80, 0);
////  glVertex3f(350, 80, 0);
////  glEnd();
//  glBegin(GL_LINES);
//  glVertex3f(350, 30, 0);
//  glVertex3f(400, 30, 0);
//
//  glVertex3f(400, 30, 0);
//  glVertex3f(380, 56, 0);
//
//  glVertex3f(380, 56, 0);
//  glVertex3f(380, 80, 0);
//
//  glVertex3f(380, 80, 0);
//  glVertex3f(350, 80, 0);
//
//  glVertex3f(350, 80, 0);
//  glVertex3f(350, 30, 0);
//  glEnd();
//
//  //senquack
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(404, 80, 0);
////  glVertex3f(404, 8, 0);
////  glVertex3f(440, 8, 0);
////  glVertex3f(440, 44, 0);
////  glVertex3f(465, 80, 0);
////  glEnd();
//  glBegin(GL_LINES);
//  glVertex3f(404, 80, 0);
//  glVertex3f(404, 8, 0);
//
//  glVertex3f(404, 8, 0);
//  glVertex3f(440, 8, 0);
//
//  glVertex3f(440, 8, 0);
//  glVertex3f(440, 44, 0);
//
//  glVertex3f(440, 44, 0);
//  glVertex3f(465, 80, 0);
//
//  glVertex3f(465, 80, 0);
//  glVertex3f(404, 80, 0);
//  glEnd();
//  //senquack
////  printf("Done drawing drawTitleBoard\n"); fflush(stdout);
//}
////senquack - 3rd try (lines are OK, triangles are way off)
//void drawTitleBoard() {
//  
// //senquack
////  printf("drawTitleBoard() start\n");
//  //glEnable(GL_TEXTURE_2D);
//  //glBindTexture(GL_TEXTURE_2D, titleTexture);
////  glColor4i(255, 255, 255, 255);
//
//  //senquack - enabling this causes dozens of vertical stripes to appear instead of anything intelligible
////  glBegin(GL_TRIANGLE_FAN);
////  glTexCoord2f(0.0f, 0.0f);
////  glVertex3f(350, 78,  0);
////  glTexCoord2f(1.0f, 0.0f);
////  glVertex3f(470, 78,  0);
////  glTexCoord2f(1.0f, 1.0f);
////  glVertex3f(470, 114,  0);
////  glTexCoord2f(0.0f, 1.0f);
////  glVertex3f(350, 114,  0);
////  printf("*");
////  glEnd();
//  //senquack - changing it to this is no help at all, still has vertical stripes:
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex3f(350, 78,  0);
////  glVertex3f(470, 78,  0);
////  glVertex3f(470, 114,  0);
////  glVertex3f(350, 114,  0);
////  glEnd();
//  
//  //glDisable(GL_TEXTURE_2D); //TODO: Make sure to uncomment this line if I fix texturing....
//  glColor4i(200, 200, 200, 255);
//
//  //senquack - these two don't cause vertical stripes but the triangles are alllll messed up
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(350, 30, 0);
//  glVertex3f(400, 30, 0);
//  glVertex3f(380, 56, 0);
//  glVertex3f(380, 80, 0);
//  glVertex3f(350, 80, 0);
//  glEnd();
//  glColor4i(200, 200, 200, 255);
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(404, 80, 0);
//  glVertex3f(404, 8, 0);
//  glVertex3f(440, 8, 0);
//  glVertex3f(440, 44, 0);
//  glVertex3f(465, 80, 0);
//  glEnd();
//
//  //senquack - test conversions here:
////  glBegin(GL_TRIANGLES);
////  glVertex3f(350, 30, 0);
////  glVertex3f(400, 30, 0);
////  glVertex3f(380, 56, 0);
////  glEnd();
////
////  glBegin(GL_TRIANGLES);
////  glVertex3f(350, 30, 0);
////  glVertex3f(380, 56, 0);
////  glVertex3f(380, 80, 0);
////  glEnd();
////
////  glBegin(GL_TRIANGLES);
////  glVertex3f(350, 30, 0);
////  glVertex3f(380, 80, 0);
////  glVertex3f(350, 80, 0);
////  glEnd();
//
////  glBegin(GL_TRIANGLES);
////  glVertex3f(404, 80, 0);
////  glVertex3f(404, 8, 0);
////  glVertex3f(440, 8, 0);
////
////  glVertex3f(404, 80, 0);
////  glVertex3f(440, 8, 0);
////  glVertex3f(440, 44, 0);
////
////  glVertex3f(404, 80, 0);
////  glVertex3f(440, 44, 0);
////  glVertex3f(465, 80, 0);
////  glEnd();
//
//  glColor4i(255, 255, 255, 255);
//
//  //senquack
//  glBegin(GL_LINE_LOOP);
//  glVertex2f(350, 30);
//  glVertex2f(400, 30);
//  glVertex2f(380, 56);
//  glVertex2f(380, 80);
//  glVertex2f(350, 80);
//  glVertex2f(350, 30);
//  glEnd();
////  glBegin(GL_LINES);
////  glVertex2f(350, 30);
////  glVertex2f(400, 30);
////  glEnd();
////
////  glBegin(GL_LINES);
////  glVertex2f(400, 30);
////  glVertex2f(380, 56);
////  glEnd();
////
////  glBegin(GL_LINES);
////  glVertex2f(380, 56);
////  glVertex2f(380, 80);
////  glEnd();
////
////  glBegin(GL_LINES);
////  glVertex2f(380, 80);
////  glVertex2f(350, 80);
////  glEnd();
////
////  glBegin(GL_LINES);
////  glVertex2f(350, 80);
////  glVertex2f(350, 30);
////  glEnd();
//
//  //senquack
//  glBegin(GL_LINE_LOOP);
//  glVertex2f(404, 80);
//  glVertex2f(404, 8);
//  glVertex2f(440, 8);
//  glVertex2f(440, 44);
//  glVertex2f(465, 80);
//  glVertex2f(404, 80);
//  glEnd();
////  glBegin(GL_LINES);
////  glVertex2f(404, 80);
////  glVertex2f(404, 8);
////  glEnd();
////
////  glBegin(GL_LINES);
////  glVertex2f(404, 8);
////  glVertex2f(440, 8);
////  glEnd();
////
////  glBegin(GL_LINES);
////  glVertex2f(440, 8);
////  glVertex2f(440, 44);
////  glEnd();
////
////  glBegin(GL_LINES);
////  glVertex2f(440, 44);
////  glVertex2f(465, 80);
////  glEnd();
////
////  glBegin(GL_LINES);
////  glVertex2f(465, 80);
////  glVertex2f(404, 80);
////  glEnd();
//  //senquack
////  printf("Done drawing drawTitleBoard\n"); fflush(stdout);
//}
////senquack - converted to openGLES
//void drawTitleBoard() {
//  
//
////  glColor4i(255, 255, 255, 255);
////  glBegin(GL_TRIANGLE_FAN);
////  glTexCoord2f(0.0f, 0.0f);
////  glVertex3f(350, 78,  0);
////  glTexCoord2f(1.0f, 0.0f);
////  glVertex3f(470, 78,  0);
////  glTexCoord2f(1.0f, 1.0f);
////  glVertex3f(470, 114,  0);
////  glTexCoord2f(0.0f, 1.0f);
////  glVertex3f(350, 114,  0);
////  glEnd();
//
//    GLubyte colors[5*4]; 
// GLfixed vertices[5*2];
////  GLfixed texvertices[4*2];
//
////  glEnable(GL_TEXTURE_2D);
////  glBindTexture(GL_TEXTURE_2D, titleTexture);
////
////  memset(colors,255,4*4);
////  vertices[0] = INT2FNUM(350);  vertices[1] = INT2FNUM(78);
////  vertices[2] = INT2FNUM(470);  vertices[3] = INT2FNUM(78);
////  vertices[4] = INT2FNUM(470);  vertices[5] = INT2FNUM(114);
////  vertices[6] = INT2FNUM(350);  vertices[7] = INT2FNUM(114);
////  texvertices[0] = 0;              texvertices[1] = 0;
////  texvertices[2] = INT2FNUM(1);    texvertices[3] = 0;
////  texvertices[4] = INT2FNUM(1);    texvertices[5] = INT2FNUM(1);
////  texvertices[6] = 0;              texvertices[7] = INT2FNUM(1);
////
//////   glEnableClientState(GL_VERTEX_ARRAY);
////  glVertexPointer(2, GL_FIXED, 0, vertices);
//////   glEnableClientState(GL_COLOR_ARRAY);
////  glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
////  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
////  glTexCoordPointer(2, GL_FIXED, 0, texvertices);
////  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
////  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
////  glDisable(GL_TEXTURE_2D); 
//
////  glColor4i(200, 200, 200, 255);
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex3f(350, 30, 0);
////  glVertex3f(400, 30, 0);
////  glVertex3f(380, 56, 0);
////  glVertex3f(380, 80, 0);
////  glVertex3f(350, 80, 0);
////  glEnd();
//
// colors[0] = colors[1] = colors[2] =
// colors[4] = colors[5] = colors[6] =
// colors[8] = colors[9] = colors[10] =
// colors[12] = colors[13] = colors[14] =
// colors[16] = colors[17] = colors[18] = 200;
// colors[3] = colors[7] = colors[11] = colors[15] = 255;
//
// vertices[0] = INT2FNUM(350);  vertices[1] = INT2FNUM(30);
// vertices[2] = INT2FNUM(400);  vertices[3] = INT2FNUM(30);
// vertices[4] = INT2FNUM(380);  vertices[5] = INT2FNUM(56);
// vertices[6] = INT2FNUM(380);  vertices[7] = INT2FNUM(80);
// vertices[8] = INT2FNUM(350);  vertices[9] = INT2FNUM(80);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 5);
//
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex3f(404, 80, 0);
////  glVertex3f(404, 8, 0);
////  glVertex3f(440, 8, 0);
////  glVertex3f(440, 44, 0);
////  glVertex3f(465, 80, 0);
////  glEnd();
//
// vertices[0] = INT2FNUM(404);  vertices[1] = INT2FNUM(80);
// vertices[2] = INT2FNUM(404);  vertices[3] = INT2FNUM(8);
// vertices[4] = INT2FNUM(440);  vertices[5] = INT2FNUM(8);
// vertices[6] = INT2FNUM(440);  vertices[7] = INT2FNUM(44);
// vertices[8] = INT2FNUM(465);  vertices[9] = INT2FNUM(80);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 5);
//
//  glColor4i(255, 255, 255, 255);
//  memset(colors,255,20);
//
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(350, 30, 0);
////  glVertex3f(400, 30, 0);
////  glVertex3f(380, 56, 0);
////  glVertex3f(380, 80, 0);
////  glVertex3f(350, 80, 0);
////  glEnd();
// vertices[0] = INT2FNUM(350);  vertices[1] = INT2FNUM(30);
// vertices[2] = INT2FNUM(400);  vertices[3] = INT2FNUM(30);
// vertices[4] = INT2FNUM(380);  vertices[5] = INT2FNUM(56);
// vertices[6] = INT2FNUM(380);  vertices[7] = INT2FNUM(80);
// vertices[8] = INT2FNUM(350);  vertices[9] = INT2FNUM(80);
// glDrawArrays(GL_LINE_LOOP, 0, 5);
//
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(404, 80, 0);
////  glVertex3f(404, 8, 0);
////  glVertex3f(440, 8, 0);
////  glVertex3f(440, 44, 0);
////  glVertex3f(465, 80, 0);
////  glEnd();
// vertices[0] = INT2FNUM(404);  vertices[1] = INT2FNUM(80);
// vertices[2] = INT2FNUM(404);  vertices[3] = INT2FNUM(8);
// vertices[4] = INT2FNUM(440);  vertices[5] = INT2FNUM(8);
// vertices[6] = INT2FNUM(440);  vertices[7] = INT2FNUM(44);
// vertices[8] = INT2FNUM(465);  vertices[9] = INT2FNUM(80);
// glDrawArrays(GL_LINE_LOOP, 0, 5);
//}
//senquack - converted to openGLES
//void drawTitleBoard() {
//  
//
////  glColor4i(255, 255, 255, 255);
////  glBegin(GL_TRIANGLE_FAN);
////  glTexCoord2f(0.0f, 0.0f);
////  glVertex3f(350, 78,  0);
////  glTexCoord2f(1.0f, 0.0f);
////  glVertex3f(470, 78,  0);
////  glTexCoord2f(1.0f, 1.0f);
////  glVertex3f(470, 114,  0);
////  glTexCoord2f(0.0f, 1.0f);
////  glVertex3f(350, 114,  0);
////  glEnd();
//
//    GLubyte colors[5*4]; 
// GLfixed vertices[5*2];
////  GLfixed texvertices[4*2];
//
////  glEnable(GL_TEXTURE_2D);
////  glBindTexture(GL_TEXTURE_2D, titleTexture);
////
////  memset(colors,255,4*4);
////  vertices[0] = INT2FNUM(350);  vertices[1] = INT2FNUM(78);
////  vertices[2] = INT2FNUM(470);  vertices[3] = INT2FNUM(78);
////  vertices[4] = INT2FNUM(470);  vertices[5] = INT2FNUM(114);
////  vertices[6] = INT2FNUM(350);  vertices[7] = INT2FNUM(114);
////  texvertices[0] = 0;              texvertices[1] = 0;
////  texvertices[2] = INT2FNUM(1);    texvertices[3] = 0;
////  texvertices[4] = INT2FNUM(1);    texvertices[5] = INT2FNUM(1);
////  texvertices[6] = 0;              texvertices[7] = INT2FNUM(1);
////
//////   glEnableClientState(GL_VERTEX_ARRAY);
// glVertexPointer(2, GL_FIXED, 0, vertices);
//////   glEnableClientState(GL_COLOR_ARRAY);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
////  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
////  glTexCoordPointer(2, GL_FIXED, 0, texvertices);
////  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
////  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
////  glDisable(GL_TEXTURE_2D); 
//
////  glColor4i(200, 200, 200, 255);
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex3f(350, 30, 0);
////  glVertex3f(400, 30, 0);
////  glVertex3f(380, 56, 0);
////  glVertex3f(380, 80, 0);
////  glVertex3f(350, 80, 0);
////  glEnd();
//
// colors[0] = colors[1] = colors[2] =
// colors[4] = colors[5] = colors[6] =
// colors[8] = colors[9] = colors[10] =
// colors[12] = colors[13] = colors[14] =
// colors[16] = colors[17] = colors[18] = 200;
// colors[3] = colors[7] = colors[11] = colors[15] = 255;
//
//////   vertices[0] = f2x(350); vertices[1] = f2x(30);
//////   vertices[2] = f2x(400); vertices[3] = f2x(30);
//////   vertices[4] = f2x(380); vertices[5] = f2x(56);
//////   vertices[6] = f2x(380); vertices[7] = f2x(80);
//////   vertices[8] = f2x(350); vertices[9] = f2x(80);
////  vertices[0] = INT2FNUM(350);  vertices[1] = INT2FNUM(30);
////  vertices[2] = INT2FNUM(400);  vertices[3] = INT2FNUM(30);
////  vertices[4] = INT2FNUM(380);  vertices[5] = INT2FNUM(56);
////  vertices[6] = INT2FNUM(380);  vertices[7] = INT2FNUM(80);
////  vertices[8] = INT2FNUM(350);  vertices[9] = INT2FNUM(80);
////  glDrawArrays(GL_TRIANGLE_FAN, 0, 5);
//
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex3f(404, 80, 0);
////  glVertex3f(404, 8, 0);
////  glVertex3f(440, 8, 0);
////  glVertex3f(440, 44, 0);
////  glVertex3f(465, 80, 0);
////  glEnd();
//
// vertices[0] = INT2FNUM(404);  vertices[1] = INT2FNUM(80);
// vertices[2] = INT2FNUM(404);  vertices[3] = INT2FNUM(8);
// vertices[4] = INT2FNUM(440);  vertices[5] = INT2FNUM(8);
// vertices[6] = INT2FNUM(440);  vertices[7] = INT2FNUM(44);
// vertices[8] = INT2FNUM(465);  vertices[9] = INT2FNUM(80);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 5);
//
////  glColor4i(255, 255, 255, 255);
//  memset(colors,255,20);
//
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(350, 30, 0);
////  glVertex3f(400, 30, 0);
////  glVertex3f(380, 56, 0);
////  glVertex3f(380, 80, 0);
////  glVertex3f(350, 80, 0);
////  glEnd();
// vertices[0] = INT2FNUM(350);  vertices[1] = INT2FNUM(30);
// vertices[2] = INT2FNUM(400);  vertices[3] = INT2FNUM(30);
// vertices[4] = INT2FNUM(380);  vertices[5] = INT2FNUM(56);
// vertices[6] = INT2FNUM(380);  vertices[7] = INT2FNUM(80);
// vertices[8] = INT2FNUM(350);  vertices[9] = INT2FNUM(80);
// glDrawArrays(GL_LINE_LOOP, 0, 5);
//
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(404, 80, 0);
////  glVertex3f(404, 8, 0);
////  glVertex3f(440, 8, 0);
////  glVertex3f(440, 44, 0);
////  glVertex3f(465, 80, 0);
////  glEnd();
// vertices[0] = INT2FNUM(404);  vertices[1] = INT2FNUM(80);
// vertices[2] = INT2FNUM(404);  vertices[3] = INT2FNUM(8);
// vertices[4] = INT2FNUM(440);  vertices[5] = INT2FNUM(8);
// vertices[6] = INT2FNUM(440);  vertices[7] = INT2FNUM(44);
// vertices[8] = INT2FNUM(465);  vertices[9] = INT2FNUM(80);
// glDrawArrays(GL_LINE_LOOP, 0, 5);
//}
//senquack - converted to openGLES
//senquack - adding texturing back in:
//void drawTitleBoard() {
//  
//
////  glColor4i(255, 255, 255, 255);
////  glBegin(GL_TRIANGLE_FAN);
////  glTexCoord2f(0.0f, 0.0f);
////  glVertex3f(350, 78,  0);
////  glTexCoord2f(1.0f, 0.0f);
////  glVertex3f(470, 78,  0);
////  glTexCoord2f(1.0f, 1.0f);
////  glVertex3f(470, 114,  0);
////  glTexCoord2f(0.0f, 1.0f);
////  glVertex3f(350, 114,  0);
////  glEnd();
//
//    GLubyte colors[5*4]; 
// GLfixed vertices[5*2];
////  GLfixed texvertices[4*2];
//
////  glEnable(GL_TEXTURE_2D);
////  glBindTexture(GL_TEXTURE_2D, titleTexture);
////
////  memset(colors,255,4*4);
////  vertices[0] = INT2FNUM(350);  vertices[1] = INT2FNUM(78);
////  vertices[2] = INT2FNUM(470);  vertices[3] = INT2FNUM(78);
////  vertices[4] = INT2FNUM(470);  vertices[5] = INT2FNUM(114);
////  vertices[6] = INT2FNUM(350);  vertices[7] = INT2FNUM(114);
////  texvertices[0] = 0;              texvertices[1] = 0;
////  texvertices[2] = INT2FNUM(1);    texvertices[3] = 0;
////  texvertices[4] = INT2FNUM(1);    texvertices[5] = INT2FNUM(1);
////  texvertices[6] = 0;              texvertices[7] = INT2FNUM(1);
////
//////   glEnableClientState(GL_VERTEX_ARRAY);
// glVertexPointer(2, GL_FIXED, 0, vertices);
//////   glEnableClientState(GL_COLOR_ARRAY);
// glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
////  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
////  glTexCoordPointer(2, GL_FIXED, 0, texvertices);
////  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
////  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
////  glDisable(GL_TEXTURE_2D); 
//
////  glColor4i(200, 200, 200, 255);
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex3f(350, 30, 0);
////  glVertex3f(400, 30, 0);
////  glVertex3f(380, 56, 0);
////  glVertex3f(380, 80, 0);
////  glVertex3f(350, 80, 0);
////  glEnd();
//
// colors[0] = colors[1] = colors[2] =
// colors[4] = colors[5] = colors[6] =
// colors[8] = colors[9] = colors[10] =
// colors[12] = colors[13] = colors[14] =
// colors[16] = colors[17] = colors[18] =
// colors[20] = colors[21] = colors[22] = 200;
// colors[3] = colors[7] = colors[11] = colors[15] = colors[19] = 255;
//
// vertices[0] = INT2FNUM(350);  vertices[1] = INT2FNUM(30);
// vertices[2] = INT2FNUM(400);  vertices[3] = INT2FNUM(30);
// vertices[4] = INT2FNUM(380);  vertices[5] = INT2FNUM(56);
// vertices[6] = INT2FNUM(380);  vertices[7] = INT2FNUM(80);
// vertices[8] = INT2FNUM(350);  vertices[9] = INT2FNUM(80);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 5);
//
////  glBegin(GL_TRIANGLE_FAN);
////  glVertex3f(404, 80, 0);
////  glVertex3f(404, 8, 0);
////  glVertex3f(440, 8, 0);
////  glVertex3f(440, 44, 0);
////  glVertex3f(465, 80, 0);
////  glEnd();
//
// vertices[0] = INT2FNUM(404);  vertices[1] = INT2FNUM(80);
// vertices[2] = INT2FNUM(404);  vertices[3] = INT2FNUM(8);
// vertices[4] = INT2FNUM(440);  vertices[5] = INT2FNUM(8);
// vertices[6] = INT2FNUM(440);  vertices[7] = INT2FNUM(44);
// vertices[8] = INT2FNUM(465);  vertices[9] = INT2FNUM(80);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 5);
//
////  glColor4i(255, 255, 255, 255);
//  memset(colors,255,20);
//
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(350, 30, 0);
////  glVertex3f(400, 30, 0);
////  glVertex3f(380, 56, 0);
////  glVertex3f(380, 80, 0);
////  glVertex3f(350, 80, 0);
////  glEnd();
// vertices[0] = INT2FNUM(350);  vertices[1] = INT2FNUM(30);
// vertices[2] = INT2FNUM(400);  vertices[3] = INT2FNUM(30);
// vertices[4] = INT2FNUM(380);  vertices[5] = INT2FNUM(56);
// vertices[6] = INT2FNUM(380);  vertices[7] = INT2FNUM(80);
// vertices[8] = INT2FNUM(350);  vertices[9] = INT2FNUM(80);
// glDrawArrays(GL_LINE_LOOP, 0, 5);
//
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(404, 80, 0);
////  glVertex3f(404, 8, 0);
////  glVertex3f(440, 8, 0);
////  glVertex3f(440, 44, 0);
////  glVertex3f(465, 80, 0);
////  glEnd();
// vertices[0] = INT2FNUM(404);  vertices[1] = INT2FNUM(80);
// vertices[2] = INT2FNUM(404);  vertices[3] = INT2FNUM(8);
// vertices[4] = INT2FNUM(440);  vertices[5] = INT2FNUM(8);
// vertices[6] = INT2FNUM(440);  vertices[7] = INT2FNUM(44);
// vertices[8] = INT2FNUM(465);  vertices[9] = INT2FNUM(80);
// glDrawArrays(GL_LINE_LOOP, 0, 5);
//}
void
drawTitleBoard ()
{


//  glColor4i(255, 255, 255, 255);
//  glBegin(GL_TRIANGLE_FAN);
//  glTexCoord2f(0.0f, 0.0f);
//  glVertex3f(350, 78,  0);
//  glTexCoord2f(1.0f, 0.0f);
//  glVertex3f(470, 78,  0);
//  glTexCoord2f(1.0f, 1.0f);
//  glVertex3f(470, 114,  0);
//  glTexCoord2f(0.0f, 1.0f);
//  glVertex3f(350, 114,  0);
//  glEnd();

   GLubyte colors[5 * 4];
   GLfixed vertices[5 * 2];
// GLfixed texvertices[4*2];
   GLfixed texvertices[4 * 2];

   glEnable (GL_TEXTURE_2D);
   glBindTexture (GL_TEXTURE_2D, titleTexture);
   glEnable (GL_BLEND);
// glBlendFunc(GL_ONE, GL_SRC_COLOR);
   glBlendFunc (GL_ONE, GL_SRC_ALPHA);
//    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 
//    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

   memset (colors, 255, 4 * 4);
// vertices[0] = INT2FNUM(350);  vertices[1] = INT2FNUM(78);
// vertices[2] = INT2FNUM(470);  vertices[3] = INT2FNUM(78);
// vertices[4] = INT2FNUM(470);  vertices[5] = INT2FNUM(114);
// vertices[6] = INT2FNUM(350);  vertices[7] = INT2FNUM(114);
   vertices[0] = INT2FNUM (350);
   vertices[1] = INT2FNUM (114);
   vertices[2] = INT2FNUM (470);
   vertices[3] = INT2FNUM (114);
   vertices[4] = INT2FNUM (350);
   vertices[5] = INT2FNUM (78);
   vertices[6] = INT2FNUM (470);
   vertices[7] = INT2FNUM (78);
// texvertices[0] = 0;              texvertices[1] = 0;
// texvertices[2] = INT2FNUM(1);    texvertices[3] = 0;
// texvertices[4] = INT2FNUM(1);    texvertices[5] = INT2FNUM(1);
// texvertices[6] = 0;              texvertices[7] = INT2FNUM(1);
   texvertices[0] = 0;
   texvertices[1] = INT2FNUM (1);
   texvertices[2] = INT2FNUM (1);
   texvertices[3] = INT2FNUM (1);
   texvertices[4] = 0;
   texvertices[5] = 0;
   texvertices[6] = INT2FNUM (1);
   texvertices[7] = 0;
//
////  glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer (2, GL_FIXED, 0, vertices);
////  glEnableClientState(GL_COLOR_ARRAY);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
   glEnableClientState (GL_TEXTURE_COORD_ARRAY);
   glTexCoordPointer (2, GL_FIXED, 0, texvertices);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
   glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
   glDisableClientState (GL_TEXTURE_COORD_ARRAY);
   glDisable (GL_TEXTURE_2D);


   glDisable (GL_BLEND);
   glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//  glColor4i(200, 200, 200, 255);
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(350, 30, 0);
//  glVertex3f(400, 30, 0);
//  glVertex3f(380, 56, 0);
//  glVertex3f(380, 80, 0);
//  glVertex3f(350, 80, 0);
//  glEnd();

   colors[0] = colors[1] = colors[2] =
      colors[4] = colors[5] = colors[6] =
      colors[8] = colors[9] = colors[10] =
      colors[12] = colors[13] = colors[14] =
      colors[16] = colors[17] = colors[18] =
      colors[20] = colors[21] = colors[22] = 200;
   colors[3] = colors[7] = colors[11] = colors[15] = colors[19] = 255;

   vertices[0] = INT2FNUM (350);
   vertices[1] = INT2FNUM (30);
   vertices[2] = INT2FNUM (400);
   vertices[3] = INT2FNUM (30);
   vertices[4] = INT2FNUM (380);
   vertices[5] = INT2FNUM (56);
   vertices[6] = INT2FNUM (380);
   vertices[7] = INT2FNUM (80);
   vertices[8] = INT2FNUM (350);
   vertices[9] = INT2FNUM (80);
   glDrawArrays (GL_TRIANGLE_FAN, 0, 5);

//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(404, 80, 0);
//  glVertex3f(404, 8, 0);
//  glVertex3f(440, 8, 0);
//  glVertex3f(440, 44, 0);
//  glVertex3f(465, 80, 0);
//  glEnd();

   vertices[0] = INT2FNUM (404);
   vertices[1] = INT2FNUM (80);
   vertices[2] = INT2FNUM (404);
   vertices[3] = INT2FNUM (8);
   vertices[4] = INT2FNUM (440);
   vertices[5] = INT2FNUM (8);
   vertices[6] = INT2FNUM (440);
   vertices[7] = INT2FNUM (44);
   vertices[8] = INT2FNUM (465);
   vertices[9] = INT2FNUM (80);
   glDrawArrays (GL_TRIANGLE_FAN, 0, 5);

//  glColor4i(255, 255, 255, 255);
   memset (colors, 255, 20);

//  glBegin(GL_LINE_LOOP);
//  glVertex3f(350, 30, 0);
//  glVertex3f(400, 30, 0);
//  glVertex3f(380, 56, 0);
//  glVertex3f(380, 80, 0);
//  glVertex3f(350, 80, 0);
//  glEnd();
   vertices[0] = INT2FNUM (350);
   vertices[1] = INT2FNUM (30);
   vertices[2] = INT2FNUM (400);
   vertices[3] = INT2FNUM (30);
   vertices[4] = INT2FNUM (380);
   vertices[5] = INT2FNUM (56);
   vertices[6] = INT2FNUM (380);
   vertices[7] = INT2FNUM (80);
   vertices[8] = INT2FNUM (350);
   vertices[9] = INT2FNUM (80);
   glDrawArrays (GL_LINE_LOOP, 0, 5);

//  glBegin(GL_LINE_LOOP);
//  glVertex3f(404, 80, 0);
//  glVertex3f(404, 8, 0);
//  glVertex3f(440, 8, 0);
//  glVertex3f(440, 44, 0);
//  glVertex3f(465, 80, 0);
//  glEnd();
   vertices[0] = INT2FNUM (404);
   vertices[1] = INT2FNUM (80);
   vertices[2] = INT2FNUM (404);
   vertices[3] = INT2FNUM (8);
   vertices[4] = INT2FNUM (440);
   vertices[5] = INT2FNUM (8);
   vertices[6] = INT2FNUM (440);
   vertices[7] = INT2FNUM (44);
   vertices[8] = INT2FNUM (465);
   vertices[9] = INT2FNUM (80);
   glDrawArrays (GL_LINE_LOOP, 0, 5);
}

// Draw the numbers.
int
drawNum (int n, int x, int y, int s, int r, int g, int b)
{
   for (;;) {
      drawLetter (n % 10, x, y, s, 3, r, g, b);
      y += s * 1.7f;
      n /= 10;
      if (n <= 0)
         break;
   }
   return y;
}

int
drawNumRight (int n, int x, int y, int s, int r, int g, int b)
{
   int d, nd, drawn = 0;
   for (d = 100000000; d > 0; d /= 10) {
      nd = (int) (n / d);
      if (nd > 0 || drawn) {
         n -= d * nd;
         drawLetter (nd % 10, x, y, s, 1, r, g, b);
         y += s * 1.7f;
         drawn = 1;
      }
   }
   if (!drawn) {
      drawLetter (0, x, y, s, 1, r, g, b);
      y += s * 1.7f;
   }
   return y;
}

int
drawNumCenter (int n, int x, int y, int s, int r, int g, int b)
{
   for (;;) {
      drawLetter (n % 10, x, y, s, 0, r, g, b);
      x -= s * 1.7f;
      n /= 10;
      if (n <= 0)
         break;
   }
   return y;
}

int
drawTimeCenter (int n, int x, int y, int s, int r, int g, int b)
{
   int i;
   for (i = 0; i < 7; i++) {
      if (i != 4) {
         drawLetter (n % 10, x, y, s, 0, r, g, b);
         n /= 10;
      } else {
         drawLetter (n % 6, x, y, s, 0, r, g, b);
         n /= 6;
      }
      if ((i & 1) == 1 || i == 0) {
         switch (i) {
         case 3:
            drawLetter (41, x + s * 1.16f, y, s, 0, r, g, b);
            break;
         case 5:
            drawLetter (40, x + s * 1.16f, y, s, 0, r, g, b);
            break;
         }
         x -= s * 1.7f;
      } else {
         x -= s * 2.2f;
      }
      if (n <= 0)
         break;
   }
   return y;
}

#define JOYSTICK_AXIS 16384

//senquack - adding support for rotated screen:
//int getPadState() {
//  int x = 0, y = 0;
//  int pad = 0;
//  int gp2x_up = 0, gp2x_upleft = 0, gp2x_left = 0, gp2x_downleft = 0, gp2x_down = 0, 
//      gp2x_downright = 0, gp2x_right = 0, gp2x_upright = 0; 
//      
//  if ( stick != NULL ) {
//    //x = SDL_JoystickGetAxis(stick, 0);
//    //y = SDL_JoystickGetAxis(stick, 1);
//    
//    gp2x_up = SDL_JoystickGetButton(stick, GP2X_BUTTON_UP);
//    gp2x_upleft = SDL_JoystickGetButton(stick, GP2X_BUTTON_UPLEFT);
//    gp2x_left = SDL_JoystickGetButton(stick, GP2X_BUTTON_LEFT);
//    gp2x_downleft = SDL_JoystickGetButton(stick, GP2X_BUTTON_DOWNLEFT);
//    gp2x_down = SDL_JoystickGetButton(stick, GP2X_BUTTON_DOWN);
//    gp2x_downright = SDL_JoystickGetButton(stick, GP2X_BUTTON_DOWNRIGHT);
//    gp2x_right = SDL_JoystickGetButton(stick, GP2X_BUTTON_RIGHT);
//    gp2x_upright = SDL_JoystickGetButton(stick, GP2X_BUTTON_UPRIGHT);  
//    
//  }
//  
//  //senquack
////  if ( keys[SDLK_RIGHT] == SDL_PRESSED || keys[SDLK_KP6] == SDL_PRESSED || x > JOYSTICK_AXIS || gp2x_right || gp2x_downright || gp2x_upright) {
////    pad |= PAD_RIGHT;
////  }
////  if ( keys[SDLK_LEFT] == SDL_PRESSED || keys[SDLK_KP4] == SDL_PRESSED || x < -JOYSTICK_AXIS || gp2x_left || gp2x_downleft || gp2x_upleft) {
////    pad |= PAD_LEFT;
////  }
////  if ( keys[SDLK_DOWN] == SDL_PRESSED || keys[SDLK_KP2] == SDL_PRESSED || y > JOYSTICK_AXIS || gp2x_down || gp2x_downright || gp2x_downleft) {
////    pad |= PAD_DOWN;
////  }
////  if ( keys[SDLK_UP] == SDL_PRESSED ||  keys[SDLK_KP8] == SDL_PRESSED || y < -JOYSTICK_AXIS || gp2x_up || gp2x_upright || gp2x_upleft) {
////    pad |= PAD_UP;
////  }
//  if ( gp2x_right || gp2x_downright || gp2x_upright) {
//    pad |= PAD_RIGHT;
//  }
//  if ( gp2x_left || gp2x_downleft || gp2x_upleft) {
//    pad |= PAD_LEFT;
//  }
//  if ( gp2x_down || gp2x_downright || gp2x_downleft) {
//    pad |= PAD_DOWN;
//  }
//  if ( gp2x_up || gp2x_upright || gp2x_upleft) {
//    pad |= PAD_UP;
//  }
//  
//  return pad;
//}
int
getPadState ()
{
   int x = 0, y = 0;
   int pad = 0;
   int gp2x_up = 0, gp2x_upleft = 0, gp2x_left = 0, gp2x_downleft =
      0, gp2x_down = 0, gp2x_downright = 0, gp2x_right = 0, gp2x_upright = 0;

   if (stick != NULL) {
      //x = SDL_JoystickGetAxis(stick, 0);
      //y = SDL_JoystickGetAxis(stick, 1);
//       if (screenRotated) {
      if (settings.rotated) {
         gp2x_up = SDL_JoystickGetButton (stick, GP2X_BUTTON_RIGHT);
         gp2x_upleft = SDL_JoystickGetButton (stick, GP2X_BUTTON_UPRIGHT);
         gp2x_left = SDL_JoystickGetButton (stick, GP2X_BUTTON_UP);
         gp2x_downleft = SDL_JoystickGetButton (stick, GP2X_BUTTON_UPLEFT);
         gp2x_down = SDL_JoystickGetButton (stick, GP2X_BUTTON_LEFT);
         gp2x_downright = SDL_JoystickGetButton (stick, GP2X_BUTTON_DOWNLEFT);
         gp2x_right = SDL_JoystickGetButton (stick, GP2X_BUTTON_DOWN);
         gp2x_upright = SDL_JoystickGetButton (stick, GP2X_BUTTON_DOWNRIGHT);
      } else {
         gp2x_up = SDL_JoystickGetButton (stick, GP2X_BUTTON_UP);
         gp2x_upleft = SDL_JoystickGetButton (stick, GP2X_BUTTON_UPLEFT);
         gp2x_left = SDL_JoystickGetButton (stick, GP2X_BUTTON_LEFT);
         gp2x_downleft = SDL_JoystickGetButton (stick, GP2X_BUTTON_DOWNLEFT);
         gp2x_down = SDL_JoystickGetButton (stick, GP2X_BUTTON_DOWN);
         gp2x_downright =
            SDL_JoystickGetButton (stick, GP2X_BUTTON_DOWNRIGHT);
         gp2x_right = SDL_JoystickGetButton (stick, GP2X_BUTTON_RIGHT);
         gp2x_upright = SDL_JoystickGetButton (stick, GP2X_BUTTON_UPRIGHT);
      }
   }
   //senquack
//  if ( keys[SDLK_RIGHT] == SDL_PRESSED || keys[SDLK_KP6] == SDL_PRESSED || x > JOYSTICK_AXIS || gp2x_right || gp2x_downright || gp2x_upright) {
//    pad |= PAD_RIGHT;
//  }
//  if ( keys[SDLK_LEFT] == SDL_PRESSED || keys[SDLK_KP4] == SDL_PRESSED || x < -JOYSTICK_AXIS || gp2x_left || gp2x_downleft || gp2x_upleft) {
//    pad |= PAD_LEFT;
//  }
//  if ( keys[SDLK_DOWN] == SDL_PRESSED || keys[SDLK_KP2] == SDL_PRESSED || y > JOYSTICK_AXIS || gp2x_down || gp2x_downright || gp2x_downleft) {
//    pad |= PAD_DOWN;
//  }
//  if ( keys[SDLK_UP] == SDL_PRESSED ||  keys[SDLK_KP8] == SDL_PRESSED || y < -JOYSTICK_AXIS || gp2x_up || gp2x_upright || gp2x_upleft) {
//    pad |= PAD_UP;
//  }
   if (gp2x_right || gp2x_downright || gp2x_upright) {
      pad |= PAD_RIGHT;
   }
   if (gp2x_left || gp2x_downleft || gp2x_upleft) {
      pad |= PAD_LEFT;
   }
   if (gp2x_down || gp2x_downright || gp2x_downleft) {
      pad |= PAD_DOWN;
   }
   if (gp2x_up || gp2x_upright || gp2x_upleft) {
      pad |= PAD_UP;
   }

   return pad;
}

int buttonReversed = 0;

//senquack - added to allow the laser to always be firing *except* when the fire button is pressed:
int laserOnByDefault = 0;

//senquack - adding support for rotated screen:
//int getButtonState() {
//  int btn = 0;
//  int btn1 = 0, btn2 = 0;
//  int btn_volup = 0, btn_voldown = 0;
////  int volchanged = 0;
//  
//  // Albert's original code:
////  if ( stick != NULL ) {
////    btn1 = SDL_JoystickGetButton(stick, GP2X_BUTTON_B);
////    //btn2 = SDL_JoystickGetButton(stick, 1);
////    //btn3 = SDL_JoystickGetButton(stick, 2);
////    //btn4 = SDL_JoystickGetButton(stick, 3);
////    btn2 = SDL_JoystickGetButton(stick, GP2X_BUTTON_X);
////    btn3 = SDL_JoystickGetButton(stick, GP2X_BUTTON_Y);
////    btn4 = SDL_JoystickGetButton(stick, GP2X_BUTTON_A);
////    btn5 = SDL_JoystickGetButton(stick, GP2X_BUTTON_START);
////    btn6 = SDL_JoystickGetButton(stick, GP2X_BUTTON_L);
////    btn_volup = SDL_JoystickGetButton(stick, GP2X_BUTTON_VOLUP);
////    btn_voldown = SDL_JoystickGetButton(stick, GP2X_BUTTON_VOLDOWN);
////
////  }
//  if ( stick != NULL ) {
//    btn1 = SDL_JoystickGetButton(stick, GP2X_BUTTON_B) |
//           SDL_JoystickGetButton(stick, GP2X_BUTTON_Y) |
//           SDL_JoystickGetButton(stick, GP2X_BUTTON_R);
//
//    btn2 = SDL_JoystickGetButton(stick, GP2X_BUTTON_X) |
//          SDL_JoystickGetButton(stick, GP2X_BUTTON_A) |
//          SDL_JoystickGetButton(stick, GP2X_BUTTON_L);
//
////    btn3 = SDL_JoystickGetButton(stick, GP2X_BUTTON_Y);
////    btn4 = SDL_JoystickGetButton(stick, GP2X_BUTTON_A);
//
////    btn_quit = SDL_JoystickGetButton(stick, GP2X_BUTTON_SELECT); 
////   btn_pause = SDL_JoystickGetButton(stick, GP2X_BUTTON_START);  
//
//    btn_volup = SDL_JoystickGetButton(stick, GP2X_BUTTON_VOLUP);
//    btn_voldown = SDL_JoystickGetButton(stick, GP2X_BUTTON_VOLDOWN);
//  }
//
////  if ( keys[SDLK_z] == SDL_PRESSED || btn1 ) {
//  if ( btn1 ) {
//    if ( !buttonReversed ) {
//      //printf("pressed!!!\n\n\n\n\n");
//      btn |= PAD_BUTTON1;
//    } else {
//      btn |= PAD_BUTTON2;
//    }
//  }
////  if ( keys[SDLK_x] == SDL_PRESSED || btn2 ) {
//  if ( btn2 ) {
//    if ( !buttonReversed ) {
//      btn |= PAD_BUTTON2;
//    } else {
//      btn |= PAD_BUTTON1;
//    }
//  }
//  
//  if (btn_voldown) {
//   gp2x_change_volume(-2);
//  } else if (btn_volup) {
//   gp2x_change_volume(2);
//  }
//  
//  //senquack - this was the cause of the segfault on exit Albert got:
//  //Quit
////  if (btn5)
////     closeSDL();
//  //senquack - my temporary replacement code:
////  if (btn_quit) done = 1;
////  if (btn_quit) 
////    btn |= PAD_BUTTON_QUIT;
//  
//  return btn;
//}
int
getButtonState ()
{
   int btn = 0;
   int btn1 = 0, btn2 = 0;
   int btn_volup = 0, btn_voldown = 0;
//  int volchanged = 0;

   // Albert's original code:
//  if ( stick != NULL ) {
//    btn1 = SDL_JoystickGetButton(stick, GP2X_BUTTON_B);
//    //btn2 = SDL_JoystickGetButton(stick, 1);
//    //btn3 = SDL_JoystickGetButton(stick, 2);
//    //btn4 = SDL_JoystickGetButton(stick, 3);
//    btn2 = SDL_JoystickGetButton(stick, GP2X_BUTTON_X);
//    btn3 = SDL_JoystickGetButton(stick, GP2X_BUTTON_Y);
//    btn4 = SDL_JoystickGetButton(stick, GP2X_BUTTON_A);
//    btn5 = SDL_JoystickGetButton(stick, GP2X_BUTTON_START);
//    btn6 = SDL_JoystickGetButton(stick, GP2X_BUTTON_L);
//    btn_volup = SDL_JoystickGetButton(stick, GP2X_BUTTON_VOLUP);
//    btn_voldown = SDL_JoystickGetButton(stick, GP2X_BUTTON_VOLDOWN);
//
//  }

//  if ( stick != NULL ) {
//    if (screenRotated) {
//       btn1 = SDL_JoystickGetButton(stick, GP2X_BUTTON_START) |
//                SDL_JoystickGetButton(stick, GP2X_BUTTON_L);
//       btn2 = SDL_JoystickGetButton(stick, GP2X_BUTTON_R);
//    } else {
//       btn1 = SDL_JoystickGetButton(stick, GP2X_BUTTON_B) |
//              SDL_JoystickGetButton(stick, GP2X_BUTTON_Y) |
//              SDL_JoystickGetButton(stick, GP2X_BUTTON_R);
//
//       btn2 = SDL_JoystickGetButton(stick, GP2X_BUTTON_X) |
//             SDL_JoystickGetButton(stick, GP2X_BUTTON_A) |
//             SDL_JoystickGetButton(stick, GP2X_BUTTON_L);
//    }
//
////    btn3 = SDL_JoystickGetButton(stick, GP2X_BUTTON_Y);
////    btn4 = SDL_JoystickGetButton(stick, GP2X_BUTTON_A);
//
////    btn_quit = SDL_JoystickGetButton(stick, GP2X_BUTTON_SELECT); 
////   btn_pause = SDL_JoystickGetButton(stick, GP2X_BUTTON_START);  
//
//    btn_volup = SDL_JoystickGetButton(stick, GP2X_BUTTON_VOLUP);
//    btn_voldown = SDL_JoystickGetButton(stick, GP2X_BUTTON_VOLDOWN);
//  }
   if (stick != NULL) {
//    if (screenRotated) {
      if (settings.rotated) {
         btn1 = SDL_JoystickGetButton (stick, settings.rbuttons[FIRE_IDX]) |
            SDL_JoystickGetButton (stick, settings.rbuttons[FIRE2_IDX]);
         btn2 =
            SDL_JoystickGetButton (stick,
                                   settings.rbuttons[SPECIAL_IDX]) |
            SDL_JoystickGetButton (stick, settings.rbuttons[SPECIAL2_IDX]);
         btn_volup =
            SDL_JoystickGetButton (stick, settings.rbuttons[VOLUP_IDX]);
         btn_voldown =
            SDL_JoystickGetButton (stick, settings.rbuttons[VOLDOWN_IDX]);
      } else {
         btn1 = SDL_JoystickGetButton (stick, settings.buttons[FIRE_IDX]) |
            SDL_JoystickGetButton (stick, settings.buttons[FIRE2_IDX]);
         btn2 =
            SDL_JoystickGetButton (stick,
                                   settings.buttons[SPECIAL_IDX]) |
            SDL_JoystickGetButton (stick, settings.buttons[SPECIAL2_IDX]);
         btn_volup =
            SDL_JoystickGetButton (stick, settings.buttons[VOLUP_IDX]);
         btn_voldown =
            SDL_JoystickGetButton (stick, settings.buttons[VOLDOWN_IDX]);
      }

   }
//  if ( keys[SDLK_z] == SDL_PRESSED || btn1 ) {
   if (btn1) {
      if (!buttonReversed) {
         //printf("pressed!!!\n\n\n\n\n");
         btn |= PAD_BUTTON1;
      } else {
         btn |= PAD_BUTTON2;
      }
   }
//  if ( keys[SDLK_x] == SDL_PRESSED || btn2 ) {
   if (btn2) {
      if (!buttonReversed) {
         btn |= PAD_BUTTON2;
      } else {
         btn |= PAD_BUTTON1;
      }
   }
//  if (btn_voldown) {
//   gp2x_change_volume(-2);
//  } else if (btn_volup) {
//   gp2x_change_volume(2);
   if (btn_voldown) {
      gp2x_change_volume (-0.5f);
   } else if (btn_volup) {
      gp2x_change_volume (0.5f);
   }
   //senquack - this was the cause of the segfault on exit Albert got:
   //Quit
//  if (btn5)
//    closeSDL();
   //senquack - my temporary replacement code:
//  if (btn_quit) done = 1;
//  if (btn_quit) 
//   btn |= PAD_BUTTON_QUIT;

   return btn;
}

// to perform cross product between 2 vectors in FadiGluLookAt
//void CrossProd(float x1, float y1, float z1, float x2, float y2, float z2, float res[3])
//{
// res[0] = y1*z2 - y2*z1;
// res[1] = x2*z1 - x1*z2;
// res[2] = x1*y2 - x2*y1;
//}

//senquack - it appears that Albert added these implementations of gluLookAt:
// crappy implementation
//void FadiGluLookAt(float eyeX, float eyeY, float eyeZ, float lookAtX, float lookAtY, float lookAtZ, float upX, float upY, float upZ)
//{
// // i am not using here proper implementation for vectors.
// // if you want, you can replace the arrays with your own
// // vector types
// float f[3];
//
// // calculating the viewing vector
// f[0] = lookAtX - eyeX;
// f[1] = lookAtY - eyeY;
// f[2] = lookAtZ - eyeZ;
//
// float fMag, upMag;
// fMag = sqrt(f[0]*f[0] + f[1]*f[1] + f[2]*f[2]); //Optimize me - GP2X!
// upMag = sqrt(upX*upX + upY*upY + upZ*upZ);
// //Math::Sqrt(fMag, f[0]*f[0] + f[1]*f[1] + f[2]*f[2]);
// //Math::Sqrt(upMag, upX*upX + upY*upY + upZ*upZ);
//
// // normalizing the viewing vector
// if( fMag != 0)
// {
//    f[0] = f[0]/fMag;
//    f[1] = f[1]/fMag;
//    f[2] = f[2]/fMag;
// }
//
// // normalising the up vector. no need for this here if you have your
// // up vector already normalised, which is mostly the case.
// if( upMag != 0 )
// {
//    upX = upX/upMag;
//    upY = upY/upMag;
//    upZ = upZ/upMag;
// }
//
// float s[3], u[3];
//
// CrossProd(f[0], f[1], f[2], upX, upY, upZ, s);
// CrossProd(s[0], s[1], s[2], f[0], f[1], f[2], u);
//
// float M[]=
// {
// s[0], u[0], -f[0], 0,
// s[1], u[1], -f[1], 0,
// s[2], u[2], -f[2], 0,
// 0, 0, 0, 1
// };
//
// glMultMatrixf(M);
// glTranslatef (-eyeX, -eyeY, -eyeZ);
//}

//senquack - it appears that Albert added these implementations of gluLookAt:
//Mesa's implementation, switched to floats from doubles in a desparate attempt at getting more speed.
//senquack - for wiz OpenGLES, we disable our internal version:
//void gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez,
//   GLfloat centerx, GLfloat centery, GLfloat centerz,
//   GLfloat upx, GLfloat upy, GLfloat upz)
//{
//   GLfloat m[16];
//   GLfloat x[3], y[3], z[3];
//   GLfloat mag;
//
//   /* Make rotation matrix */
//
//   /* Z vector */
//   z[0] = eyex - centerx;
//   z[1] = eyey - centery;
//   z[2] = eyez - centerz;
//   mag = sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
//   if (mag) {         /* mpichler, 19950515 */
//      z[0] /= mag;
//      z[1] /= mag;
//      z[2] /= mag;
//   }
//
//   /* Y vector */
//   y[0] = upx;
//   y[1] = upy;
//   y[2] = upz;
//
//   /* X vector = Y cross Z */
//   x[0] = y[1] * z[2] - y[2] * z[1];
//   x[1] = -y[0] * z[2] + y[2] * z[0];
//   x[2] = y[0] * z[1] - y[1] * z[0];
//
//   /* Recompute Y = Z cross X */
//   y[0] = z[1] * x[2] - z[2] * x[1];
//   y[1] = -z[0] * x[2] + z[2] * x[0];
//   y[2] = z[0] * x[1] - z[1] * x[0];
//
//   /* mpichler, 19950515 */
//   /* cross product gives area of parallelogram, which is < 1.0 for
//    * non-perpendicular unit-length vectors; so normalize x, y here
//    */
//
//   mag = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
//   if (mag) {
//      x[0] /= mag;
//      x[1] /= mag;
//      x[2] /= mag;
//   }
//
//   mag = sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
//   if (mag) {
//      y[0] /= mag;
//      y[1] /= mag;
//      y[2] /= mag;
//   }
//
//#define M(row,col)  m[col*4+row]
//   M(0, 0) = x[0];
//   M(0, 1) = x[1];
//   M(0, 2) = x[2];
//   M(0, 3) = 0.0;
//   M(1, 0) = y[0];
//   M(1, 1) = y[1];
//   M(1, 2) = y[2];
//   M(1, 3) = 0.0;
//   M(2, 0) = z[0];
//   M(2, 1) = z[1];
//   M(2, 2) = z[2];
//   M(2, 3) = 0.0;
//   M(3, 0) = 0.0;
//   M(3, 1) = 0.0;
//   M(3, 2) = 0.0;
//   M(3, 3) = 1.0;
//#undef M
//   glMultMatrixf(m);
//
//   /* Translate Eye to Origin */
//   glTranslatef(-eyex, -eyey, -eyez);
//
//}
void
gluLookAtx (GLfixed eyex, GLfixed eyey, GLfixed eyez,
            GLfixed centerx, GLfixed centery, GLfixed centerz,
            GLfixed upx, GLfixed upy, GLfixed upz)
{
   GLfixed m[16];
   GLfixed x[3], y[3], z[3];
   GLfixed mag;

   /* Make rotation matrix */

   /* Z vector */
   z[0] = eyex - centerx;
   z[1] = eyey - centery;
   z[2] = eyez - centerz;
//   mag = sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
   mag = FSQRT (FMUL (z[0], z[0]) + FMUL (z[1], z[1]) + FMUL (z[2], z[2]));
   if (mag) {                   /* mpichler, 19950515 */
//      z[0] /= mag;
//      z[1] /= mag;
//      z[2] /= mag;
      z[0] = FDIV (z[0], mag);
      z[1] = FDIV (z[1], mag);
      z[2] = FDIV (z[2], mag);
   }

   /* Y vector */
   y[0] = upx;
   y[1] = upy;
   y[2] = upz;

   /* X vector = Y cross Z */
//   x[0] = y[1] * z[2] - y[2] * z[1];
//   x[1] = -y[0] * z[2] + y[2] * z[0];
//   x[2] = y[0] * z[1] - y[1] * z[0];
   x[0] = FMUL (y[1], z[2]) - FMUL (y[2], z[1]);
   x[1] = FMUL (-y[0], z[2]) + FMUL (y[2], z[0]);
   x[2] = FMUL (y[0], z[1]) - FMUL (y[1], z[0]);

   /* Recompute Y = Z cross X */
//   y[0] = z[1] * x[2] - z[2] * x[1];
//   y[1] = -z[0] * x[2] + z[2] * x[0];
//   y[2] = z[0] * x[1] - z[1] * x[0];
   y[0] = FMUL (z[1], x[2]) - FMUL (z[2], x[1]);
   y[1] = FMUL (-z[0], x[2]) + FMUL (z[2], x[0]);
   y[2] = FMUL (z[0], x[1]) - FMUL (z[1], x[0]);

   /* mpichler, 19950515 */
   /* cross product gives area of parallelogram, which is < 1.0 for
    * non-perpendicular unit-length vectors; so normalize x, y here
    */

//   mag = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
   mag = FSQRT (FMUL (x[0], x[0]) + FMUL (x[1], x[1]) + FMUL (x[2], x[2]));
   if (mag) {
//      x[0] /= mag;
//      x[1] /= mag;
//      x[2] /= mag;
      x[0] = FDIV (x[0], mag);
      x[1] = FDIV (x[1], mag);
      x[2] = FDIV (x[2], mag);
   }
//   mag = sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
   mag = FSQRT (FMUL (y[0], y[0]) + FMUL (y[1], y[1]) + FMUL (y[2], y[2]));
   if (mag) {
//      y[0] /= mag;
//      y[1] /= mag;
//      y[2] /= mag;
      y[0] = FDIV (y[0], mag);
      y[1] = FDIV (y[1], mag);
      y[2] = FDIV (y[2], mag);
   }
#define M(row,col)  m[col*4+row]
   M (0, 0) = x[0];
   M (0, 1) = x[1];
   M (0, 2) = x[2];
//   M(0, 3) = 0.0;
   M (0, 3) = 0;
   M (1, 0) = y[0];
   M (1, 1) = y[1];
   M (1, 2) = y[2];
//   M(1, 3) = 0.0;
   M (1, 3) = 0;
   M (2, 0) = z[0];
   M (2, 1) = z[1];
   M (2, 2) = z[2];
//   M(2, 3) = 0.0;
//   M(3, 0) = 0.0;
//   M(3, 1) = 0.0;
//   M(3, 2) = 0.0;
//   M(3, 3) = 1.0;
   M (2, 3) = 0;
   M (3, 0) = 0;
   M (3, 1) = 0;
   M (3, 2) = 0;
   M (3, 3) = INT2FNUM (1);
#undef M
//   glMultMatrixf(m);
   glMultMatrixx (m);

   /* Translate Eye to Origin */
//   glTranslatef(-eyex, -eyey, -eyez);
   glTranslatex (-eyex, -eyey, -eyez);

}

//senquack - 2/11 - disabled for now:
////Added by Albert to help with surface format conversion:
///*
// * SDL surface conversion to OpenGL texture formats
// *
// * Mattias Engdegård
// *
// * Use, modification and distribution of this source is allowed without
// * limitation, warranty or liability of any kind.
// */
//
///*
// * Convert a surface into one suitable as an OpenGL texture;
// * in RGBA format if want_alpha is nonzero, or in RGB format otherwise.
// * 
// * The surface may have a colourkey, which is then translated to an alpha
// * channel if RGBA is desired.
// *
// * Return the resulting texture, or NULL on error. The original surface is
// * always freed.
// */
SDL_Surface *
conv_surf_gl (SDL_Surface * s, int want_alpha)
{
   Uint32 rmask, gmask, bmask, amask;
   SDL_Surface *conv;
   int bpp = want_alpha ? 32 : 24;

   /* SDL interprets each pixel as a 24 or 32-bit number, so our
      masks must depend on the endianness (byte order) of the
      machine. */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
   rmask = 0xff000000 >> (32 - bpp);
   gmask = 0x00ff0000 >> (32 - bpp);
   bmask = 0x0000ff00 >> (32 - bpp);
   amask = 0x000000ff >> (32 - bpp);
#else
   rmask = 0x000000ff;
   gmask = 0x0000ff00;
   bmask = 0x00ff0000;
   amask = want_alpha ? 0xff000000 : 0;
#endif

   /* check if the surface happens to be in the right format */
   if (s->format->BitsPerPixel == bpp
       && s->format->Rmask == rmask
       && s->format->Gmask == gmask
       && s->format->Bmask == bmask
       && s->format->Amask == amask && !(s->flags & SDL_SRCCOLORKEY)) {
      /* no conversion needed */
      return s;
   }

   /* wrong format, conversion necessary */

   /* SDL surfaces are created with lines padded to start at 32-bit boundaries
      which suits OpenGL well (as long as GL_UNPACK_ALIGNMENT remains
      unchanged from its initial value of 4) */
   conv = SDL_CreateRGBSurface (SDL_SWSURFACE, s->w, s->h, bpp,
                                rmask, gmask, bmask, amask);
   if (!conv) {
      SDL_FreeSurface (conv);
      return NULL;
   }

   if (want_alpha) {
      /* SDL sets the SDL_SRCALPHA flag on all surfaces with an
         alpha channel. We need to clear that flag for the copy,
         since SDL would attempt to alpha-blend our image otherwise */
      SDL_SetAlpha (s, 0, 255);
   }

   /*
    * Do the conversion. If the source surface has a colourkey, then it
    * will be used in the blit. We use the fact that newly created software
    * surfaces are zero-filled, so the pixels not blitted will remain
    * transparent.
    */
   if (SDL_BlitSurface (s, NULL, conv, NULL) < 0) {
      /* blit error */
      SDL_FreeSurface (conv);
      conv = NULL;
   }
   SDL_FreeSurface (s);

   return conv;
}

/*
 * A sample use of conv_surf_gl():
 *
 * Load an image from a file, and convert it to RGB or RGBA format,
 * depending on the image.
 *
 * Return the resulting surface, or NULL on error
 */
//SDL_Surface *load_gl_texture(char *file)
//{
//    SDL_Surface *s = IMG_Load(file);
//    if(!s)
//    return NULL;
//    return conv_surf_gl(s, s->format->Amask || (s->flags & SDL_SRCCOLORKEY));
//}
