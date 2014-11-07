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

#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_image.h"

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

//senquack - holds our port-specific definitions
#include "portcfg.h"

#define FAR_PLANE 720

#define SHARE_LOC "rr_share/"

typedef struct eglConnection {
   EGLDisplay display;
   EGLContext context;
   EGLSurface surface;
   NativeWindowType window;
} eglConnection;

static eglConnection egl_screen = {
   EGL_NO_DISPLAY,
   EGL_NO_CONTEXT,
   EGL_NO_SURFACE,
   (NativeWindowType)0
};

SDL_Surface * conv_surf_gl (SDL_Surface * s, int want_alpha);

// fps() returns the current FPS as an integer. Meant to be called once every frame.
int fps()
{
   static int frames_drawn = 0; // Frame counter (resets after recording new FPS value)
   static int current_fps = 0;  // Current FPS

   // Remembers last time we reported a FPS value:
   static struct timespec lasttime = {.tv_sec=0, .tv_nsec=0};
   static uint64_t lasttime_in_ns = 0;

   // Get current time:
   struct timespec curtime = {.tv_sec=0, .tv_nsec=0};
   clock_gettime(CLOCK_MONOTONIC_RAW, &curtime);
   uint64_t curtime_in_ns = (uint64_t)curtime.tv_nsec + (uint64_t)curtime.tv_sec * 1000000000;

   // If one second has passed, record the time and reset the frame counter:
   if ((curtime_in_ns - lasttime_in_ns) >= 1000000000) {
      clock_gettime(CLOCK_MONOTONIC_RAW, &lasttime);
      lasttime_in_ns = (uint64_t)lasttime.tv_nsec + (uint64_t)lasttime.tv_sec * 1000000000;
      current_fps = frames_drawn;
      frames_drawn = 0;
   } else {
      frames_drawn++;
   }

   return current_fps;
}

void swapGLScene ()
{
   if (eglSwapBuffers(egl_screen.display, egl_screen.surface) != GL_TRUE) {
      printf("OpenGLES: eglSwapBuffers failed!\n");
   }
   //debug
//   } else {
//      printf("FPS: %d\n", fps());
//   }
}

//OpenGLES-related:
/** @brief Error checking function
 * @param file : string reference that contains the source file that the check is occuring in
 * @param line : numeric reference that contains the line number that the check is occuring in
 * @return : 0 if the function passed, else 1
 */
int8_t CheckGLESErrors( const char* file, uint16_t line )
{
   EGLenum error;
   const char* errortext;
   const char* description;
   error = eglGetError();
   if (error != EGL_SUCCESS && error != 0)
   {
      switch (error)
      {
         case EGL_NOT_INITIALIZED:
            errortext = "EGL_NOT_INITIALIZED.";
            description = "EGL is not or could not be initialized, for the specified display.";
            break;
         case EGL_BAD_ACCESS:
            errortext = "EGL_BAD_ACCESS EGL";
            description = "Cannot access a requested resource (for example, a context is bound in another thread).";
            break;
         case EGL_BAD_ALLOC:
            errortext = "EGL_BAD_ALLOC EGL";
            description = "Failed to allocate resources for the requested operation.";
            break;
         case EGL_BAD_ATTRIBUTE:
            errortext = "EGL_BAD_ATTRIBUTE";
            description = "An unrecognized attribute or attribute value was passed in anattribute list.";
            break;
         case EGL_BAD_CONFIG:
            errortext = "EGL_BAD_CONFIG";
            description = "An EGLConfig argument does not name a valid EGLConfig.";
            break;
         case EGL_BAD_CONTEXT:
            errortext = "EGL_BAD_CONTEXT";
            description = "An EGLContext argument does not name a valid EGLContext.";
            break;
         case EGL_BAD_CURRENT_SURFACE:
            errortext = "EGL_BAD_CURRENT_SURFACE";
            description = "The current surface of the calling thread is a window, pbuffer,or pixmap that is no longer valid.";
            break;
         case EGL_BAD_DISPLAY:
            errortext = "EGL_BAD_DISPLAY";
            description = "An EGLDisplay argument does not name a valid EGLDisplay.";
            break;
         case EGL_BAD_MATCH:
            errortext = "EGL_BAD_MATCH";
            description = "Arguments are inconsistent; for example, an otherwise valid context requires buffers (e.g. depth or stencil) not allocated by an otherwise valid surface.";
            break;
         case EGL_BAD_NATIVE_PIXMAP:
            errortext = "EGL_BAD_NATIVE_PIXMAP";
            description = "An EGLNativePixmapType argument does not refer to a validnative pixmap.";
            break;
         case EGL_BAD_NATIVE_WINDOW:
            errortext = "EGL_BAD_NATIVE_WINDOW";
            description = "An EGLNativeWindowType argument does not refer to a validnative window.";
            break;
         case EGL_BAD_PARAMETER:
            errortext = "EGL_BAD_PARAMETER";
            description = "One or more argument values are invalid.";
            break;
         case EGL_BAD_SURFACE:
            errortext = "EGL_BAD_SURFACE";
            description = "An EGLSurface argument does not name a valid surface (window,pbuffer, or pixmap) configured for rendering";
            break;
         case EGL_CONTEXT_LOST:
            errortext = "EGL_CONTEXT_LOST";
            description = "A power management event has occurred. The application must destroy all contexts and reinitialise client API state and objects to continue rendering.";
            break;
         default:
            errortext = "Unknown EGL Error";
            description = "";
            break;
      }
      printf( "OpenGLES ERROR: EGL Error detected in file %s at line %d: %s (0x%X)\n Description: %s\n", file, line, errortext, error, description );
      return 1;
   }
   return 0;
}

//Initialize OpenGLES 1.1 (return 0 for success, 1 for error)
int initGLES()
{
   // These settings are correct for the GCW0
   EGLint egl_config_attr[] = {
////#if SCREEN_BPP > 16
////      // RGBA8888
//      EGL_RED_SIZE,      8,
//      EGL_BLUE_SIZE,     8,
//      EGL_GREEN_SIZE,    8,
//      EGL_ALPHA_SIZE,    8,
////      EGL_BUFFER_SIZE,   32,
////#else
////      // RGBA5650
////      EGL_RED_SIZE,      5,
////      EGL_BLUE_SIZE,     6,
////      EGL_GREEN_SIZE,    5,
////      EGL_ALPHA_SIZE,    0,
////      EGL_BUFFER_SIZE,   16,
////#endif
//      // senquack TODO - Do we really even need a depth buffer?
//      EGL_DEPTH_SIZE,    16,     
//////      EGL_DEPTH_SIZE,    0,
////      EGL_STENCIL_SIZE,  0,
//      EGL_SURFACE_TYPE,     EGL_WINDOW_BIT,
//      EGL_RENDERABLE_TYPE,  EGL_OPENGL_ES_BIT,   // We want OpenGLES 1, not 2
//      EGL_NONE           // This is important as it tells EGL when to stop looking in the array

//#if SCREEN_BPP > 16
//      // RGBA8888
//      EGL_RED_SIZE,      8,
//      EGL_BLUE_SIZE,     8,
//      EGL_GREEN_SIZE,    8,
//      EGL_ALPHA_SIZE,    8,
//      EGL_BUFFER_SIZE,   32,
//#else
//      // RGBA5650
//      EGL_RED_SIZE,      5,
//      EGL_BLUE_SIZE,     6,
//      EGL_GREEN_SIZE,    5,
//      EGL_ALPHA_SIZE,    0,

//      EGL_BUFFER_SIZE,   16,
      EGL_BUFFER_SIZE,   SCREEN_BPP,
//#endif
      // senquack TODO - Do we really even need a depth buffer?
      EGL_DEPTH_SIZE,    16,     
////      EGL_DEPTH_SIZE,    0,
      EGL_STENCIL_SIZE,  0,
      EGL_SURFACE_TYPE,     EGL_WINDOW_BIT,
//      EGL_RENDERABLE_TYPE,  EGL_OPENGL_ES_BIT,   // We want OpenGLES 1, not 2
      EGL_NONE           // This is important as it tells EGL when to stop looking in the array
   };


   EGLConfig egl_config;
   EGLint num_configs_found;
   EGLBoolean result;
   EGLint eglMajorVer, eglMinorVer;
   const char* output;

   // Start up OpenGLES
   printf( "OpenGLES: Opening EGL display\n" );
   egl_screen.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if (egl_screen.display == EGL_NO_DISPLAY)
   {
      CheckGLESErrors( __FILE__, __LINE__ );
      printf( "OpenGLES ERROR: Unable to create EGL display.\n" );
      return 1;
   }

   printf( "OpenGLES: Initializing\n" );
   result = eglInitialize(egl_screen.display, &eglMajorVer, &eglMinorVer );
   if (result != EGL_TRUE )
   {
      CheckGLESErrors( __FILE__, __LINE__ );
      printf( "OpenGLES ERROR: Unable to initialize EGL display.\n" );
      return 1;
   }

   /* Get EGL Library Information */
   printf( "EGL Implementation Version: Major %d Minor %d\n", eglMajorVer, eglMinorVer );
   output = eglQueryString( egl_screen.display, EGL_VENDOR );
   printf( "EGL_VENDOR: %s\n", output );
   output = eglQueryString( egl_screen.display, EGL_VERSION );
   printf( "EGL_VERSION: %s\n", output );
   output = eglQueryString( egl_screen.display, EGL_EXTENSIONS );
   printf( "EGL_EXTENSIONS: %s\n", output );

   printf("OpenGLES: Requesting %dx%d %dbpp configuration\n", SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP);
   result = eglChooseConfig(egl_screen.display, egl_config_attr, &egl_config, 1, &num_configs_found);

   if (result != EGL_TRUE || num_configs_found == 0)
   {
      CheckGLESErrors( __FILE__, __LINE__ );
      printf( "EGLport ERROR: Unable to query for available configs, found %d.\n", num_configs_found );
      return 1;
   }
   printf( "EGLport: Found %d matching configs. Using first one.\n", num_configs_found );

   printf( "OpenGLES: Creating context\n" );
   egl_screen.context = eglCreateContext(egl_screen.display, egl_config, EGL_NO_CONTEXT, NULL);
   if (egl_screen.context == EGL_NO_CONTEXT)
   {
      CheckGLESErrors( __FILE__, __LINE__ );
      printf( "OpenGLES ERROR: Unable to create GLES context!\n");
      return 1;
   }

   // For raw FBDEV on GCW0:
   egl_screen.window = 0;

   printf( "OpenGLES: Creating window surface\n" );
   egl_screen.surface = eglCreateWindowSurface(egl_screen.display, egl_config, egl_screen.window, NULL);
   if (egl_screen.surface == EGL_NO_SURFACE)
   {
      CheckGLESErrors( __FILE__, __LINE__ );
      printf( "OpenGLES ERROR: Unable to create EGL surface!\n" );
      return 1;
   }

   printf( "OpenGLES: Making current the new context\n" );
   result = eglMakeCurrent(egl_screen.display, egl_screen.surface, egl_screen.surface, egl_screen.context);
   if (result != EGL_TRUE)
   {
      CheckGLESErrors( __FILE__, __LINE__ );
      printf( "OpenGLES ERROR: Unable to make GLES context current\n" );
      return 1;
   }

   printf( "OpenGLES: Setting swap interval\n" );
   eglSwapInterval(egl_screen.display, 1);      // We want VSYNC

   printf( "OpenGLES: Initialization complete\n" );
   CheckGLESErrors( __FILE__, __LINE__ );

   glViewport (0, 0, 320, 240);

   //senquack - tried disabling:
   glClearColor (0.0f, 0.0f, 0.0f, 0.0f);

   glLineWidth(1);
   glEnable(GL_LINE_SMOOTH);
   glEnable (GL_BLEND);
//   glBlendFunc(GL_SRC_ALPHA, GL_ONE);   // original rr code had this
   glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);    // GL docs say is best for transparency
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

//   //DEBUG
//  //new support for screen rotations:
//   glMatrixMode(GL_MODELVIEW);

//  //DEBUG:
//  settings.rotated = SCREEN_ROTATED_RIGHT;
//   if (settings.rotated == SCREEN_ROTATED_LEFT) {
//      glRotatef (90.0f, 0.0f, 0.0f, 1.0f);
//      glScalef(1.5f,1.34f,1.0f); // As close to perfect as I can manage
//   } else if (settings.rotated == SCREEN_ROTATED_RIGHT) {
//      glRotatef (-90.0f, 0.0f, 0.0f, 1.0f);
//      glScalef(1.5f,1.34f,1.0f); // As close to perfect as I can manage
////      glScalef(.1,.1,0);
//   }
//   glPushMatrix();

//
  //new support for screen rotations:
//   glMatrixMode(GL_PROJECTION);

  //DEBUG:
//  settings.rotated = SCREEN_ROTATED_RIGHT;
//   if (settings.rotated == SCREEN_ROTATED_LEFT) {
//      glRotatef (-90.0, 0, 0, 1.0);
//   } else if (settings.rotated == SCREEN_ROTATED_RIGHT) {
//      glRotatef (90.0, 0, 0, 1.0);
//   }
//   glPushMatrix();

   //senquack - TODO : gotta stop calling resize() all the time
   resized (SCREEN_WIDTH, SCREEN_HEIGHT);
   return 0;
}

// Close down OpenGLES (return 0 for success, 1 for error)
int closeGLES()
{
//   //DEBUG rotation
//   glMatrixMode(GL_MODELVIEW);
//   glPopMatrix();

   EGLBoolean result;
   printf("Closing OpenGLES..\n");
   result = eglMakeCurrent(egl_screen.display, NULL, NULL, EGL_NO_CONTEXT);
   if (result != EGL_TRUE)
   {
      CheckGLESErrors( __FILE__, __LINE__ );
      return 1;
   }
   printf("Destroying OpenGLES surface..\n");
   result = eglDestroySurface(egl_screen.display, egl_screen.surface);
   if (result != EGL_TRUE)
   {
      CheckGLESErrors( __FILE__, __LINE__ );
      return 1;
   }
   printf("Destroying OpenGLES context..\n");
   result = eglDestroyContext(egl_screen.display, egl_screen.context);
   if (result != EGL_TRUE)
   {
      CheckGLESErrors( __FILE__, __LINE__ );
      return 1;
   }
   egl_screen.surface = 0;
   egl_screen.context = 0;
//   egl_config = 0;
   printf("Terminating OpenGLES display..\n");
   result = eglTerminate(egl_screen.display);
   if (result != EGL_TRUE)
   {
      CheckGLESErrors( __FILE__, __LINE__ );
      return 1;
   }
   egl_screen.display = 0;
   return 0;
}

// Reset viewport when the screen is resized.
//static void screenResized() {
//  glViewport(0, 0, screenWidth, screenHeight);
//  glMatrixMode(GL_PROJECTION);
//  glLoadIdentity();
//  gluPerspective(45.0f, (GLfloat)screenWidth/(GLfloat)screenHeight, 0.1f, FAR_PLANE);
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


   //DEBUG
//   if (settings.rotated) {
////    glViewport((int)(-87.0 * (400.0/320.0)), 0, 460, 320);
//      glViewport ((int) (-90.0 * (400.0 / 320.0)), 0, 463, 320);
//   } else {
//      glViewport (0, 0, 320, 240);
      glViewport (0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
//   }

   glMatrixMode (GL_PROJECTION);
   //senquack - note: calling glLoadIdentity() first pushes the new matrix generated by gluPerspective onto the stack:
   glLoadIdentity ();
//  gluPerspective(45.0f, (GLfloat)screenWidth/(GLfloat)screenHeight, 0.1f, FAR_PLANE);

   //senquack - TODO do we really need to call this every single screen refresh!?
#ifdef FIXEDMATH
   gluPerspective(2949120, 87381, 6554, FAR_PLANE_X);
#else
   //debug - THIS DID NOOOOT HELP, everything stretched top/bottom
//   if (settings.rotated == SCREEN_ROTATED_LEFT) {
//  gluPerspective(45.0f, (GLfloat)SCREEN_HEIGHT/(GLfloat)SCREEN_WIDTH, 0.1f, FAR_PLANE);
//   } else if (settings.rotated == SCREEN_ROTATED_RIGHT) {
//  gluPerspective(45.0f, (GLfloat)SCREEN_HEIGHT/(GLfloat)SCREEN_WIDTH, 0.1f, FAR_PLANE);
//   } else {
//   
  gluPerspective(45.0f, (GLfloat)SCREEN_WIDTH/(GLfloat)SCREEN_HEIGHT, 0.1f, FAR_PLANE);
//}
////   gluPerspective(45.0f, 320.0f/240.0f, 0.1f, FAR_PLANE);
#endif //FIXEDMATH

   glMatrixMode (GL_MODELVIEW);

}

//senquack - support screen rotation:
void
resized (int width, int height)
{
//   screenWidth = width;
//   screenHeight = height;
   screenResized ();
}

//senquack - added our own copy:
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
#ifdef FIXEDMATH
void gluPerspective(GLfixed fovy, GLfixed ratio, GLfixed near, GLfixed far)
{
// GLfloat top = near * tan(fovy * M_PI/360.0f);
// GLfloat top = near * tanf(fovy * 3.14159f/360.0f);
// GLfloat bottom = -top;
// GLfloat right = top * ratio;
// GLfloat left = -right;

// GLfixed top = FMUL(near,INT2FNUM(tantbl[FNUM2INT(FMUL(fovy,572))]));
// GLfloat top = near * tanf(fovy * 3.14159f/360.0f);
   //senquack TODO - I renabled the below line vs the one below it, as it's probably faster.. check on this
 GLfixed top = f2x(near * tanf(fovy * 3.14159f/360.0f));
//   GLfixed top = FMUL (near, f2x (tanf (x2f (fovy) * 0.008726646f)));
   GLfixed bottom = -top;
   GLfixed right = FMUL (top, ratio);
   GLfixed left = -right;

// glFrustum(left, right, bottom, top, near, far);
   glFrustumx (left, right, bottom, top, near, far);
}
#else
void gluPerspective(GLfloat fovy, GLfloat ratio, GLfloat near, GLfloat far)
{
   GLfloat top = near * tanf(fovy * 3.14159f/360.0f);
   GLfloat bottom = -top;
   GLfloat right = top * ratio;
   GLfloat left = -right;

   glFrustumf(left, right, bottom, top, near, far);
} 
#endif //FIXEDMATH

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
//// Init OpenGL.
//static void initGL ()
//{
//
//   //senquack - trying to support rotated, zoomed screen
////  glViewport(0, 0, screenWidth, screenHeight);
////  glViewport(320-80, 0, 240, 320);
//
//   //senquack - border on left, flashing border on right, drawboard on top:
////  glViewport(0, -80, 320 , 240);
//   //senquack - slight borders on top and bottom (no drawboards), shifted to right 80
////  glViewport(-80, 0, 400, 320);
//   //senquack - shifted to the right 160 pixels, slight borders on top and bottom (top larger than bottom):
////  glViewport(-80, 80, 400, 320);
//   //senquack - bottom border (maybe 40 pixels), left border (maybe 80 pixels) ,(squished on games' x axis 40 pixels)
////  glViewport((int)(-80.0 * (400.0/320.0)), -80, 400, 320);
//   //senquack - left border (80 pixels maybe), top drawboard
////  glViewport(0,-80, 400, 320);
//   //still has left border of 80 pixels maybe,  shifted left 80 pixels, bottom border of 40 pixels maybe
////  glViewport((int)(-80.0 * (400.0/320.0)), -160, 400, 320);
////  glViewport((int)(-80.0 * (400.0/320.0)), -80, 400, 320);
//   //senquack - almost there (squished 40 pix on game's x axis)
////  glViewport((int)(-80.0 * (400.0/320.0)), 0, 400, 320);
//   //senquack - almost there, slight border on top and bottom:
////  glViewport((int)(-80.0 * (400.0/320.0)), 0, 440, 320);
//   //senquack - close: (shifted a bit to the top)
////  glViewport((int)(-90.0 * (400.0/320.0)), 0, 460, 320);
////  glViewport((int)(-87.0 * (400.0/320.0)), 0, 460, 320);
//
//// if (screenRotated) {
//   if (settings.rotated) {
////    glViewport((int)(-87.0 * (400.0/320.0)), 0, 460, 320);
//      glViewport ((int) (-90.0 * (400.0 / 320.0)), 0, 463, 320);
//   } else {
//      glViewport (0, 0, 320, 240);
//   }
//
//   //senquack - tried disabling:
//   glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
//
//   glLineWidth(1);
//   glEnable(GL_LINE_SMOOTH);
//   glEnable (GL_BLEND);
//   glBlendFunc(GL_SRC_ALPHA, GL_ONE);   // original rr code had this
//// senquack - for some reason I changed it to this for Wiz/GP2X (opengl docs say it is better for transparency)
////   glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
//   glDisable (GL_LIGHTING);
//   glDisable (GL_CULL_FACE);
//   glDisable (GL_DEPTH_TEST);
//   glDisable (GL_TEXTURE_2D);
//   glDisable (GL_COLOR_MATERIAL);
//
//   //senquack - for OpenGLES, we enable these once and leave them enabled:
//   glEnableClientState (GL_VERTEX_ARRAY);
//   glEnableClientState (GL_COLOR_ARRAY);
//   glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//   glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//   resized (screenWidth, screenHeight);
//
////  //senquack - temporary:
//////  printf("sizeof(shapes) = %d\n", sizeof(shapes));
////  memset(shapes, sizeof(shapes),0);
//
//}


//// Load bitmaps and convert to textures.
void
loadGLTexture (char *fileName, GLuint * texture)
{
   SDL_Surface *surface;
   int mode;                    //The bit-depth of the texture.
   //senquack: far too short for safety:
//   char name[32];
   char name[1024];
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
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
   //senquack - shouldn't be using mipmap if we don't use it below anymore
//   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

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
//int joystickMode = 1;
//SDL_Joystick *stick = NULL;

//senquack - GCW's analog stick
#ifdef GCW
SDL_Joystick *joy_analog = NULL;
#endif


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
void initSDL ()
{

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

   // We do not neet to initialize SDL video subsystem when using GLES directly:
//   /* Initialize SDL */
//   if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
//      fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
//      exit(1);
//   }

   printf("Initializing SDL joystick subsystem..\n");
   if ( SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0 ) {
      printf ("Unable to initialize SDL joystick subsystem: %s\n",
            SDL_GetError ());
//      joystickMode = 0;
   } else {
#ifdef GCW
	printf ("Initializing GCW analog joystick driver..\n");
	for (int i = 0; i < SDL_NumJoysticks(); i++)
	{
		printf("Joystick %u: \"%s\"\n", i, SDL_JoystickName(i));
		if ( (SDL_NumJoysticks() == 1) ||      // If there's only one joystick, use it, otherwise be more selective:
            strcmp(SDL_JoystickName(i), "linkdev device (Analog 2-axis 8-button 2-hat)") == 0)
		{
			joy_analog = SDL_JoystickOpen(i);
			if (joy_analog)
				printf("Recognized GCW Zero's built-in analog stick.\n");
			else
				printf("ERROR: Failed to recognize GCW Zero's built-in analog stick..\n");
      }
	}
	SDL_JoystickEventState(SDL_IGNORE); // We'll do our own polling
#endif
   }

	//DKS - also disable keyboard events:
	SDL_EnableKeyRepeat(0, 0); // No key repeat

//   screenWidth = SCREEN_WIDTH;
//   screenHeight = SCREEN_HEIGHT;
//   screenBPP = SCREEN_BPP;

   printf("Changing video mode to %dx%d %dbpp:\n", SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP);
   if ( SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SCREEN_FLAGS) == NULL ) {
      printf("ERROR: Unable to change video mode using SDL: %s\n", SDL_GetError());
      SDL_Quit();
      exit(1);
   } 

   SDL_ShowCursor (SDL_DISABLE);

//   if (joystickMode == 1) {
//      SDL_JoystickEventState (SDL_ENABLE);
//      stick = SDL_JoystickOpen (0);
//   }

//   /* Set the title bar in environments that support it */
//   SDL_WM_SetCaption (CAPTION, NULL);

   initGLES();

   loadGLTexture (STAR_BMP, &starTexture);
   loadGLTexture (SMOKE_BMP, &smokeTexture);
   loadGLTexture (TITLE_BMP, &titleTexture);

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

////senquack - for wiz OpenGLES
//   eglMakeCurrent (glDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
//   eglDestroySurface (glDisplay, glSurface);
//   eglDestroyContext (glDisplay, glContext);
//   eglTerminate (glDisplay);

   //senquack - no longer needed after OpenGLES conversion
// nanoGL_Destroy();    

//    /* clean up the window */
//    SDL_GL_DeleteContext(glcontext);
//    SDL_DestroyWindow(window);

#ifdef GCW
  if (joy_analog) SDL_JoystickClose(joy_analog);
#endif

   closeGLES();

   SDL_Quit ();

}

//senquack - interestingly enough, this is only used in gluLookat and never changes it seems:
#ifdef FIXEDMATH
////senquack - fixed point version (only used in gluLookat interestingly enough)
GLfixed fzoom = INT2FNUM (15);
#else
float zoom = 15;
#endif //FIXEDMATH

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
#ifdef FIXEDMATH
static void setEyepos ()
{
   GLfixed x, y;
   glPushMatrix ();
   if (screenShakeCnt > 0) {
      switch (screenShakeType) {
      case 0:
//      x = (float)randNS2(256)/5000.0f;
//      y = (float)randNS2(256)/5000.0f;
//senquack TODO: poss. optimization w/ inverse:
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
      gluLookAt(0, 0, fzoom, x, y, 0, 0, INT2FNUM (1), 0);
   } else {
//     gluLookAt(0, 0, zoom, 0, 0, 0, 0.0f, 1.0f, 0.0f); //changed for gpu940
      gluLookAt(0, 0, fzoom, 0, 0, 0, 0, INT2FNUM (1), 0);
   }
}
#else
static void setEyepos() {
   glMatrixMode(GL_MODELVIEW);      //senquack - just to me sure we're in modelview

   float x, y;
   glPushMatrix();
   if ( screenShakeCnt > 0 ) {
      switch ( screenShakeType ) {
         case 0:
            x = (float)randNS2(256)/5000.0f;
            y = (float)randNS2(256)/5000.0f;
            break;
         default:
            x = (float)randNS2(256)*screenShakeCnt/21000.0f;
            y = (float)randNS2(256)*screenShakeCnt/21000.0f;
            break;
      }
      gluLookAt(0, 0, zoom, x, y, 0, 0.0f, 1.0f, 0.0f); 
   } else {
      gluLookAt(0, 0, zoom, 0, 0, 0, 0.0f, 1.0f, 0.0f);
   }
}
#endif //FIXEDMATH

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

//senquack DEBUG
//void drawGLSceneStart() {
//   glClear(GL_COLOR_BUFFER_BIT);
//   setEyepos();
//}
void drawGLSceneStart() 
{
//   glPushMatrix();


   glClear(GL_COLOR_BUFFER_BIT);
   setEyepos();

//  //DEBUG:
//  //new support for screen rotations:
//   glMatrixMode(GL_MODELVIEW);
//  settings.rotated = SCREEN_ROTATED_RIGHT;
//   if (settings.rotated == SCREEN_ROTATED_LEFT) {
//      glRotatef (90.0, 0, 0, 1.0);
//   } else if (settings.rotated == SCREEN_ROTATED_RIGHT) {
//      glRotatef (-90.0, 0, 0, 1.0);
//   }
//   glPushMatrix();
//   glMatrixMode(GL_MODELVIEW);

  //DEBUG:
   glMatrixMode(GL_MODELVIEW);
//   glLoadIdentity();        // senquack - normally I'd load identity here to keep from rotating over and over
                              //       the same matrix, but, at least with etnaviv, I can't do that, it messes
                              //       things up.

//  static float transx = 0.0f, transy = 0.0f;
//  static float scalex = 1.0f, scaley = 1.0f;
//  static float step = .01f;
//  if (control_state[CY]) transx -= step;
//  if (control_state[CB]) transx += step;
//  if (control_state[CX]) transy -= step;
//  if (control_state[CA]) transy += step;
//
//  if (control_state[CANALOGUP]) scalex -= step;
//  if (control_state[CANALOGDOWN]) scalex += step;
//  if (control_state[CANALOGLEFT]) scaley -= step;
//  if (control_state[CANALOGRIGHT]) scaley += step;
//
//  printf("transx: %f  transy: %f   scalex: %f  scaley: %f\n", transx, transy, scalex, scaley);


   if (settings.rotated == SCREEN_ROTATED_LEFT) {
      glRotatef (90.0f, 0.0f, 0.0f, 1.0f);
//      glScalef(1.5f,1.34f,1.0f); // As close to perfect as I can manage
      glScalef(1.4f, 1.4f, 1.0f);      // If you want maximum screen area, but hitbox can move all way top-to-bottom
   } else if (settings.rotated == SCREEN_ROTATED_RIGHT) {
//      glRotatef (-90.0f, 0.0f, 0.0f, 1.0f);
//      glScalef(1.5f,1.34f,1.0f); // As close to perfect as I can manage
      glRotatef (-90.0f, 0.0f, 0.0f, 1.0f);
//      glTranslatef(transx, transy, 0);
//      glScalef(scalex, scaley, 1.0f);
//      glScalef(1.35f, 1.35f, 1.0f);    // If you want less borders to where the hitbox can move shown on screen
      glScalef(1.4f, 1.4f, 1.0f);      // If you want maximum screen area, but hitbox can move all way top-to-bottom
   }
//   glPushMatrix();

}

//void drawGLSceneEnd ()
//{
//   glPopMatrix ();
//}
void drawGLSceneEnd ()
{
   glMatrixMode(GL_MODELVIEW);   // should be a modelview matrix we're popping
   glPopMatrix ();

   //new for rotation:
//   glPopMatrix();
}

//senquack - for my custom 3d transform routines:
typedef struct { GLfloat x; GLfloat y; GLfloat z; } gl_point_3d;

////senquack - for my custom 3D transform routines:
//inline gl_point_3d rotate3d_x(gl_point_3d input, int d);    // Rotates coordinates about x-axis by d degrees
//inline gl_point_3d rotate3d_y(gl_point_3d input, int d);    // Rotates coordinates about y-axis by d degrees
//inline gl_point_3d rotate3d_z(gl_point_3d input, int d);    // Rotates coordinates about z-axis by d degrees

//senquack - wrote these custom 3d transform routines, mainly to allow drawRollLine to use batch-mode
inline gl_point_3d rotate3d_x(gl_point_3d input, int d)
{
   GLfloat b2 = COS_LOOKUP(d);  GLfloat b3 = -SIN_LOOKUP(d);
   GLfloat c2 = -b2;            GLfloat c3 = b2;

   gl_point_3d output;
   output.x = input.x;
   output.y = b2 * input.y + b3 * input.z;
   output.z = c2 * input.y + c3 * input.z;

   return output;
}

inline gl_point_3d rotate3d_y(gl_point_3d input, int d)
{
   GLfloat a1 = COS_LOOKUP(d);                          GLfloat a3 = SIN_LOOKUP(d);
   GLfloat c1 = -a3;                                    GLfloat c3 = a1;

   gl_point_3d output;
   output.x = a1 * input.x + a3 * input.z;
   output.y = input.y;
   output.z = c1 * input.x + c3 * input.z;

   return output;
}

inline gl_point_3d rotate3d_z(gl_point_3d input, int d)
{
   GLfloat a1 = COS_LOOKUP(d); GLfloat a2 = -SIN_LOOKUP(d);
   GLfloat b1 = -a2;           GLfloat b2 = a1;

   gl_point_3d output;
   output.x = a1 * input.x + a2 * input.y;
   output.y = b1 * input.x + b2 * input.y;
   output.z = input.z;

   return output;
}

//senquack - Converted everything I could to one huge triangle and line batch that gets dispatched through
//             these large arrays:
// Most complicated shape (circle) requires 18 vertexes, and each point inside the shapes needs 6 vertexes:
#define FOE_MAX 1024  // pulled from foe.cc
//static gl_vertex triangles[FOE_MAX * 18 + FOE_MAX * 6]; 
static gl_vertex triangles[50000];     // TODO: start with a huge number and we'll work down from here later
static gl_vertex *triangles_ptr = NULL;
//static gl_vertex lines[FOE_MAX * 18];
static gl_vertex lines[50000];
static gl_vertex *lines_ptr = NULL;

// NOTE
//senquack - disabling these next 3-4 functions allowed program to run longer before hang:

//void drawBox(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
//      int r, int g, int b) {
//  glPushMatrix();
//  glTranslatef(x, y, 0);
//  glColor4i(r, g, b, 128);
//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(-width, -height,  0);
//  glVertex3f( width, -height,  0);
//  glVertex3f( width,  height,  0);
//  glVertex3f(-width,  height,  0);
//  glEnd();
//  glColor4i(r, g, b, 255);
//  glBegin(GL_LINE_LOOP);
//  glVertex3f(-width, -height,  0);
//  glVertex3f( width, -height,  0);
//  glVertex3f( width,  height,  0);
//  glVertex3f(-width,  height,  0);
//  glEnd();
//  glPopMatrix();
//}
//senquack - boxes are now drawn via batch-interleaved GLES.
//             New parameter "outlined" tells if it should have a outline.
#ifdef FIXEDMATH
void drawBox(GLfixed x, GLfixed y, GLfixed width, GLfixed height, int r, int g, int b)
{
#else
void drawBox(GLfloat x, GLfloat y, GLfloat width, GLfloat height, int r, int g, int b)
{
#endif //FIXEDMATH
   gl_vertex vertices[4];
   uint32_t color;
   color = (128 << 24) | (b << 16) | (g << 8) | r;
   vertices[0].x = x - width;     vertices[0].y = y - height;    vertices[0].color_rgba = color;
   vertices[1].x = x - width;     vertices[1].y = y + height;    vertices[1].color_rgba = color;
   vertices[2].x = x + width;     vertices[2].y = y + height;    vertices[2].color_rgba = color;
   vertices[3].x = x + width;     vertices[3].y = y - height;    vertices[3].color_rgba = color;
   *triangles_ptr++ = vertices[0];
   *triangles_ptr++ = vertices[1];
   *triangles_ptr++ = vertices[2];
   *triangles_ptr++ = vertices[0];
   *triangles_ptr++ = vertices[2];
   *triangles_ptr++ = vertices[3];

   //      color = (255 << 24) | (b << 16) | (g << 8) | r;
   color &= 0x00FFFFFF;    color |= (255 << 24);
   vertices[0].color_rgba = vertices[1].color_rgba = vertices[2].color_rgba = vertices[3].color_rgba = color;
   *lines_ptr++ = vertices[0];
   *lines_ptr++ = vertices[1];
   *lines_ptr++ = vertices[1];
   *lines_ptr++ = vertices[2];
   *lines_ptr++ = vertices[2];
   *lines_ptr++ = vertices[3];
   *lines_ptr++ = vertices[3];
   *lines_ptr++ = vertices[0];
}

//senquack - new routine for drawing boxes with no outline, i.e., for tiny text on handhelds
#ifdef FIXEDMATH
void drawBoxNoOutline(GLfixed x, GLfixed y, GLfixed width, GLfixed height, int r, int g, int b)
#else
void drawBoxNoOutline(GLfloat x, GLfloat y, GLfloat width, GLfloat height, int r, int g, int b)
#endif
{
   gl_vertex vertices[4];
   uint32_t color = (128 << 24) | (b << 16) | (g << 8) | r;
   vertices[0].x = x - width;     vertices[0].y = y - height;    vertices[0].color_rgba = color;
   vertices[1].x = x - width;     vertices[1].y = y + height;    vertices[1].color_rgba = color;
   vertices[2].x = x + width;     vertices[2].y = y + height;    vertices[2].color_rgba = color;
   vertices[3].x = x + width;     vertices[3].y = y - height;    vertices[3].color_rgba = color;
   *triangles_ptr++ = vertices[0];
   *triangles_ptr++ = vertices[1];
   *triangles_ptr++ = vertices[2];
   *triangles_ptr++ = vertices[0];
   *triangles_ptr++ = vertices[2];
   *triangles_ptr++ = vertices[3];
}

//senquack - converted to GLES and now lines are drawn in one huge batch
//void drawLine(GLfloat x1, GLfloat y1, GLfloat z1,
//	      GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a) {
//  glColor4ub(r, g, b, a);
//  glBegin(GL_LINES);
//  glVertex3f(x1, y1, z1);
//  glVertex3f(x2, y2, z2);
//  glEnd();
//}
#ifdef FIXEDMATH
void drawLine(GLfixed x1, GLfixed y1, GLfixed x2, GLfixed y2, int r, int g, int b, int a)
#else
inline void drawLine(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, int r, int g, int b, int a)
{
   uint32_t color = (a << 24) | (b << 16) | (g << 8) | r;
   lines_ptr->x = x1; lines_ptr->y = y1;
   lines_ptr->color_rgba = color;
   lines_ptr++;
   lines_ptr->x = x2; lines_ptr->y = y2;
   lines_ptr->color_rgba = color;
   lines_ptr++;
}
#endif //FIXEDMATH

//void drawLinePart(GLfloat x1, GLfloat y1, GLfloat z1,
//      GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a, int len) {
//  glColor4i(r, g, b,a);
//  glBegin(GL_LINES);
//  glVertex3f(x1, y1, z1);
//  glVertex3f(x1+(x2-x1)*len/256, y1+(y2-y1)*len/256, z1+(z2-z1)*len/256);
//  glEnd();
//}
#ifdef FIXEDMATH
//senquack - TODO: convert my old fixed-point version to the faster batch-drawn one below it:
void drawLinePart(GLfixed x1, GLfixed y1, GLfixed x2, GLfixed y2, int r, int g, int b, int a, int len)
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
#else
void drawLinePart(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, int r, int g, int b, int a, int len)
{
   uint32_t color = (a << 24) | (b << 16) | (g << 8) | r;
   lines_ptr->x = x1; lines_ptr->y = y1;
   lines_ptr->color_rgba = color;
   lines_ptr++;
   lines_ptr->x = x1 + (((x2 - x1) * len) * (1.0f/256.0f)); lines_ptr->y = y1 + (((y2 - y1) * len) * (1.0f/256.0f)); 
   lines_ptr->color_rgba = color;
   lines_ptr++;
}
#endif //FIXEDMATH

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
//senquack - these were for drawing the background lines, and I directly draw the lines there now, no need for this anymore:
//void
//drawRollLineAbsx (GLfixed x1, GLfixed y1, GLfixed z1,
//                  GLfixed x2, GLfixed y2, GLfixed z2, int r, int g, int b,
//                  int a, int d1)
//{
//   glPushMatrix ();
////  glRotatef((float)d1*360/1024, 0, 0, 1);
//   glRotatex ((d1 * 360) << 6, 0, 0, INT2FNUM (1));
//
//   GLfloat line[6];
//   line[0] = x1;
//   line[1] = y1;
//   line[2] = z1;
//   line[3] = x2;
//   line[4] = y2;
//   line[5] = z2;
//// GLfixed line[6];
//// line[0] = f2x(x1); line[1] = f2x(y1); line[2] = f2x(z1);
//// line[3] = f2x(x2); line[4] = f2x(y2); line[5] = f2x(z2);
//
//
//   GLubyte colors[8] = { r, g, b, a, r, g, b, a };
//
//// glVertexPointer(3, GL_FIXED, 0, line);
//   glVertexPointer (3, GL_FLOAT, 0, line);
//   glEnableClientState (GL_VERTEX_ARRAY);
//   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
//   glEnableClientState (GL_COLOR_ARRAY);
//
//   glDrawArrays (GL_LINES, 0, 2);
//   glPopMatrix ();
//}



//senquack - converted to batch-drawn OpenGLES with rotation done using custom trig routines, to allow
//             for batch drawing and elimination of many calls into GLES. Note that z paramter passed
//             was always 0, so I eliminated that parameter. Lines are still drawn 3D, however.
// Original GL1.2 code:
//void drawRollLine(GLfloat x, GLfloat y, GLfloat z, GLfloat width,
//		  int r, int g, int b, int a, int d1, int d2) {
//  glPushMatrix();
//  glTranslatef(x, y, z);
//  glRotatef((float)d1*360/1024, 0, 0, 1);
//  glRotatef((float)d2*360/1024, 1, 0, 0);
//  glColor4ub(r, g, b, a);
//  glBegin(GL_LINES);
//  glVertex3f(0, -width, 0);
//  glVertex3f(0,  width, 0);
//  glEnd();
//  glPopMatrix();
//}
#define FRAG_MAX 512 // Pulled from frag.h - senquack
static gl_vertex_3d roll_lines[2 * FRAG_MAX + 12];       // Shouldn't need more than 1024 total, but be a bit safe
static gl_vertex_3d *roll_lines_ptr = NULL;

inline void prepareDrawRollLines()
{
   roll_lines_ptr = &roll_lines[0];
}

void finishDrawRollLines()
{
   int numvertices = ((unsigned int) roll_lines_ptr - (unsigned int) (&roll_lines[0])) / sizeof(gl_vertex_3d);
   if (numvertices < 2) return;     // Is there anything to draw?
   //   glEnable (GL_BLEND);
#ifdef FIXEDMATH
   glVertexPointer (3, GL_FIXED, sizeof(gl_vertex_3d), &roll_lines[0].x);
#else
   glVertexPointer (3, GL_FLOAT, sizeof(gl_vertex_3d), &roll_lines[0].x);
#endif //FIXEDMATH
   glColorPointer (4, GL_UNSIGNED_BYTE, sizeof(gl_vertex_3d), &roll_lines[0].r);
   glDrawArrays (GL_LINES, 0, numvertices);
//   printf("printing roll_lines with %d vertices\n", numvertices);
}

#ifdef FIXEDMATH
//senquack - TODO: update my old fixed-point code to match the newer float code
void drawRollLine (GLfixed x, GLfixed y, GLfixed z, GLfixed width, int r, int g, int b, int d1, int d2)
{
   printf("ERROR: call to broken drawRollLine, please update its code to match the new float version below it.\n");
}
#else
// senquack - Now, we do our own 3D trig rotations so everything's done in a huge batch without needing glRotate():
void drawRollLine (GLfloat x, GLfloat y, GLfloat z, GLfloat width, int r, int g, int b, int d1, int d2)
{
   gl_point_3d vert1 = { .x = 0, .y = -width, .z = 0 };
   vert1 = rotate3d_z(vert1, d1);
   vert1 = rotate3d_x(vert1, d2);

   gl_point_3d vert2 = { .x = 0, .y = width, .z = 0 };
   vert2 = rotate3d_z(vert2, d1);
   vert2 = rotate3d_x(vert2, d2);

   uint32_t color = (255 << 24) | (b << 16) | (g << 8) | r;

   roll_lines_ptr->x = x + vert1.x;   roll_lines_ptr->y = y + vert1.y;   roll_lines_ptr->z = z + vert1.z;
   roll_lines_ptr->color_rgba = color;
   roll_lines_ptr++;
   roll_lines_ptr->x = x + vert2.x;   roll_lines_ptr->y = y + vert2.y;   roll_lines_ptr->z = z + vert2.z;
   roll_lines_ptr->color_rgba = color;
   roll_lines_ptr++;
}
#endif //FIXEDMATH


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
// senquack - I think I accidentally left these old unused functions in for the Wiz release:
//void
//drawSquare (GLfloat x1, GLfloat y1, GLfloat z1,
//            GLfloat x2, GLfloat y2, GLfloat z2,
//            GLfloat x3, GLfloat y3, GLfloat z3,
//            GLfloat x4, GLfloat y4, GLfloat z4, int r, int g, int b)
//{
//   GLubyte colors[4 * 4];
//// GLfloat vertices[4*3];
//   GLfixed vertices[4 * 3];
//   colors[0] = colors[4] = colors[8] = colors[12] = r;
//   colors[1] = colors[5] = colors[9] = colors[13] = g;
//   colors[2] = colors[6] = colors[10] = colors[14] = b;
//   colors[3] = colors[7] = colors[11] = colors[15] = 64;
//
//// vertices[0] = x1; vertices[1] = y1;    vertices[2] = z1;
//// vertices[3] = x2; vertices[4] = y2;    vertices[5] = z2;
//// vertices[6] = x3; vertices[7] = y3;    vertices[8] = z3;
//// vertices[9] = x4; vertices[10] = y4; vertices[11] = z4;
//   vertices[0] = f2x (x1);
//   vertices[1] = f2x (y1);
//   vertices[2] = f2x (z1);
//   vertices[3] = f2x (x2);
//   vertices[4] = f2x (y2);
//   vertices[5] = f2x (z2);
//   vertices[6] = f2x (x3);
//   vertices[7] = f2x (y3);
//   vertices[8] = f2x (z3);
//   vertices[9] = f2x (x4);
//   vertices[10] = f2x (y4);
//   vertices[11] = f2x (z4);
//
//   glEnableClientState (GL_VERTEX_ARRAY);
//// glVertexPointer(3, GL_FLOAT, 0, vertices);
//   glVertexPointer (3, GL_FIXED, 0, vertices);
//   glEnableClientState (GL_COLOR_ARRAY);
//   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
//   glDrawArrays (GL_TRIANGLE_FAN, 0, 4);
//}
//
//void
//drawSquarex (GLfixed x1, GLfixed y1, GLfixed z1,
//             GLfixed x2, GLfixed y2, GLfixed z2,
//             GLfixed x3, GLfixed y3, GLfixed z3,
//             GLfixed x4, GLfixed y4, GLfixed z4, int r, int g, int b)
//{
//   GLubyte colors[4 * 4];
//   GLfixed vertices[4 * 3];
//   colors[0] = colors[4] = colors[8] = colors[12] = r;
//   colors[1] = colors[5] = colors[9] = colors[13] = g;
//   colors[2] = colors[6] = colors[10] = colors[14] = b;
//   colors[3] = colors[7] = colors[11] = colors[15] = 64;
//
//   vertices[0] = x1;
//   vertices[1] = y1;
//   vertices[2] = z1;
//   vertices[3] = x2;
//   vertices[4] = y2;
//   vertices[5] = z2;
//   vertices[6] = x3;
//   vertices[7] = y3;
//   vertices[8] = z3;
//   vertices[9] = x4;
//   vertices[10] = y4;
//   vertices[11] = z4;
//
//// glEnableClientState(GL_VERTEX_ARRAY);
//   glVertexPointer (3, GL_FIXED, 0, vertices);
//// glEnableClientState(GL_COLOR_ARRAY);
//   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
//   glDrawArrays (GL_TRIANGLE_FAN, 0, 4);
//}

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
//senquack TODO - check I got the coordinates right in this conversion, no idea:
#ifdef FIXEDMATH
void drawStar (int f, GLfixed x, GLfixed y, int r, int g, int b, GLfixed size)
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
   vertices[0] = size; vertices[1] = -size;
   vertices[2] = size; vertices[3] = size;
   vertices[4] = -size; vertices[5] = -size;
   vertices[6] = -size; vertices[7] = size;
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
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
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
#else
//senquack TODO - reduce the size later maybe
static gl_textured_vertex star_vertices[512];   // 512 to be safe, never saw much above 200 or so
static gl_textured_vertex *star_vertex_ptr = NULL;
static gl_textured_vertex smoke_vertices[512];   // 512 to be safe, have seen as high as 264 (rarely)
static gl_textured_vertex *smoke_vertex_ptr = NULL;

void prepareDrawStars()
{
   star_vertex_ptr = &star_vertices[0];
   smoke_vertex_ptr = &smoke_vertices[0];
}

void finishDrawStars()
{
//   glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
   glBlendFunc(GL_ONE, GL_SRC_ALPHA);
   glEnableClientState (GL_TEXTURE_COORD_ARRAY);
   glEnable (GL_TEXTURE_2D);

   int numvertices;
   // Draw stars:
   numvertices = ((unsigned int) star_vertex_ptr - (unsigned int) &star_vertices[0]) / sizeof(gl_textured_vertex);

   if (numvertices > 6) {     // Are there any stars to draw?

#ifdef FIXEDMATH
      glVertexPointer (2, GL_FIXED, sizeof(gl_textured_vertex), &star_vertices[0].x);
      glTexCoordPointer (2, GL_FIXED, sizeof(gl_textured_vertex), &star_vertices[0].tx);
#else
      glVertexPointer (2, GL_FLOAT, sizeof(gl_textured_vertex), &star_vertices[0].x);
      glTexCoordPointer (2, GL_FLOAT, sizeof(gl_textured_vertex), &star_vertices[0].tx);
#endif
      glColorPointer (4, GL_UNSIGNED_BYTE, sizeof(gl_textured_vertex), &star_vertices[0].r);
      glBindTexture (GL_TEXTURE_2D, starTexture);
      glDrawArrays (GL_TRIANGLES, 0, numvertices);
   }

// Debug info:
//   static int max_star_verts = 0;
//   if (numvertices > max_star_verts) max_star_verts = numvertices;
//   printf("star vertices: %d  (max: %d)  ", numvertices, max_star_verts);

   // Draw smoke:
   numvertices = ((unsigned int) smoke_vertex_ptr - (unsigned int) &smoke_vertices[0]) / sizeof(gl_textured_vertex);

   if (numvertices > 6) {     // Is there any smoke to draw?

#ifdef FIXEDMATH
      glVertexPointer (2, GL_FIXED, sizeof(gl_textured_vertex), &smoke_vertices[0].x);
      glTexCoordPointer (2, GL_FIXED, sizeof(gl_textured_vertex), &smoke_vertices[0].tx);
#else
      glVertexPointer (2, GL_FLOAT, sizeof(gl_textured_vertex), &smoke_vertices[0].x);
      glTexCoordPointer (2, GL_FLOAT, sizeof(gl_textured_vertex), &smoke_vertices[0].tx);
#endif
      glColorPointer (4, GL_UNSIGNED_BYTE, sizeof(gl_textured_vertex), &smoke_vertices[0].r);
      glBindTexture (GL_TEXTURE_2D, smokeTexture);
      glDrawArrays (GL_TRIANGLES, 0, numvertices);
   }

// Debug info:
//   static int max_smoke_verts = 0;
//   if (numvertices > max_smoke_verts) max_smoke_verts = numvertices;
//   printf("smoke vertices: %d  (max: %d)  ", numvertices, max_smoke_verts);

   glDisable (GL_TEXTURE_2D);
   glDisableClientState (GL_TEXTURE_COORD_ARRAY);
   glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
}

//senquack TODO - check I got the coordinates right in this conversion, no idea:
void drawStar (int f, GLfloat x, GLfloat y, int r, int g, int b, float size)
{

   uint32_t color = (255 << 24) | (b << 16) | (g << 8) | r;
//   glRotatef((float)(rand()%360), 0.0f, 0.0f, 1.0f);
   int deg = rand() & 0x3FF;  // Random number between 0-1023
   
   float v1x, v1y, v2x, v2y, v3x, v3y, v4x, v4y;
   v1x = x + X_ROT(size, -size, deg);    v1y = y + Y_ROT(size,-size, deg);
   v2x = x + X_ROT(size,size, deg);      v2y = y + Y_ROT(size,size, deg);
   v3x = x + X_ROT(-size,-size, deg);    v3y = y + Y_ROT(-size,-size, deg);
   v4x = x + X_ROT(-size,size, deg);     v4y = y + Y_ROT(-size,size, deg);
   
   // Draw a textured quad using two triangles:
   if (f) {
      star_vertex_ptr->x = v1x;             star_vertex_ptr->y = v1y;
      star_vertex_ptr->tx = 1.0f;           star_vertex_ptr->ty = 0;
      star_vertex_ptr->color_rgba = color;  star_vertex_ptr++;

      star_vertex_ptr->x = v2x;             star_vertex_ptr->y = v2y;
      star_vertex_ptr->tx = 1.0f;           star_vertex_ptr->ty = 1.0f;
      star_vertex_ptr->color_rgba = color;  star_vertex_ptr++;

      star_vertex_ptr->x = v3x;             star_vertex_ptr->y = v3y;
      star_vertex_ptr->tx = 0;              star_vertex_ptr->ty = 0;
      star_vertex_ptr->color_rgba = color;  star_vertex_ptr++;

      star_vertex_ptr->x = v2x;             star_vertex_ptr->y = v2y;
      star_vertex_ptr->tx = 1.0f;           star_vertex_ptr->ty = 1.0f;
      star_vertex_ptr->color_rgba = color;  star_vertex_ptr++;

      star_vertex_ptr->x = v3x;             star_vertex_ptr->y = v3y;
      star_vertex_ptr->tx = 0;              star_vertex_ptr->ty = 0;
      star_vertex_ptr->color_rgba = color;  star_vertex_ptr++;

      star_vertex_ptr->x = v4x;             star_vertex_ptr->y = v4y;
      star_vertex_ptr->tx = 0;              star_vertex_ptr->ty = 1.0f;
      star_vertex_ptr->color_rgba = color;  star_vertex_ptr++;
   } else {
      smoke_vertex_ptr->x = v1x;             smoke_vertex_ptr->y = v1y;
      smoke_vertex_ptr->tx = 1.0f;           smoke_vertex_ptr->ty = 0;
      smoke_vertex_ptr->color_rgba = color;  smoke_vertex_ptr++;

      smoke_vertex_ptr->x = v2x;             smoke_vertex_ptr->y = v2y;
      smoke_vertex_ptr->tx = 1.0f;           smoke_vertex_ptr->ty = 1.0f;
      smoke_vertex_ptr->color_rgba = color;  smoke_vertex_ptr++;

      smoke_vertex_ptr->x = v3x;             smoke_vertex_ptr->y = v3y;
      smoke_vertex_ptr->tx = 0;              smoke_vertex_ptr->ty = 0;
      smoke_vertex_ptr->color_rgba = color;  smoke_vertex_ptr++;

      smoke_vertex_ptr->x = v2x;             smoke_vertex_ptr->y = v2y;
      smoke_vertex_ptr->tx = 1.0f;           smoke_vertex_ptr->ty = 1.0f;
      smoke_vertex_ptr->color_rgba = color;  smoke_vertex_ptr++;

      smoke_vertex_ptr->x = v3x;             smoke_vertex_ptr->y = v3y;
      smoke_vertex_ptr->tx = 0;              smoke_vertex_ptr->ty = 0;
      smoke_vertex_ptr->color_rgba = color;  smoke_vertex_ptr++;

      smoke_vertex_ptr->x = v4x;             smoke_vertex_ptr->y = v4y;
      smoke_vertex_ptr->tx = 0;              smoke_vertex_ptr->ty = 1.0f;
      smoke_vertex_ptr->color_rgba = color;  smoke_vertex_ptr++;
   }
   
//   // Draw a textured quad using two triangles:
//   if (f) {
//      star_vertex_ptr->x = x + size;        star_vertex_ptr->y = y - size;
//      star_vertex_ptr->tx = 1.0f;           star_vertex_ptr->ty = 0;
//      star_vertex_ptr->color_rgba = color;  star_vertex_ptr++;
//
//      star_vertex_ptr->x = x + size;        star_vertex_ptr->y = y + size;
//      star_vertex_ptr->tx = 1.0f;           star_vertex_ptr->ty = 1.0f;
//      star_vertex_ptr->color_rgba = color;  star_vertex_ptr++;
//
//      star_vertex_ptr->x = x - size;        star_vertex_ptr->y = y - size;
//      star_vertex_ptr->tx = 0;              star_vertex_ptr->ty = 0;
//      star_vertex_ptr->color_rgba = color;  star_vertex_ptr++;
//
//      star_vertex_ptr->x = x + size;        star_vertex_ptr->y = y + size;
//      star_vertex_ptr->tx = 1.0f;           star_vertex_ptr->ty = 1.0f;
//      star_vertex_ptr->color_rgba = color;  star_vertex_ptr++;
//
//      star_vertex_ptr->x = x - size;        star_vertex_ptr->y = y - size;
//      star_vertex_ptr->tx = 0;              star_vertex_ptr->ty = 0;
//      star_vertex_ptr->color_rgba = color;  star_vertex_ptr++;
//
//      star_vertex_ptr->x = x - size;        star_vertex_ptr->y = y + size;
//      star_vertex_ptr->tx = 0;              star_vertex_ptr->ty = 1.0f;
//      star_vertex_ptr->color_rgba = color;  star_vertex_ptr++;
//   } else {
//      smoke_vertex_ptr->x = x + size;        smoke_vertex_ptr->y = y - size;
//      smoke_vertex_ptr->tx = 1.0f;           smoke_vertex_ptr->ty = 0;
//      smoke_vertex_ptr->color_rgba = color;  smoke_vertex_ptr++;
//
//      smoke_vertex_ptr->x = x + size;        smoke_vertex_ptr->y = y + size;
//      smoke_vertex_ptr->tx = 1.0f;           smoke_vertex_ptr->ty = 1.0f;
//      smoke_vertex_ptr->color_rgba = color;  smoke_vertex_ptr++;
//
//      smoke_vertex_ptr->x = x - size;        smoke_vertex_ptr->y = y - size;
//      smoke_vertex_ptr->tx = 0;              smoke_vertex_ptr->ty = 0;
//      smoke_vertex_ptr->color_rgba = color;  smoke_vertex_ptr++;
//
//      smoke_vertex_ptr->x = x + size;        smoke_vertex_ptr->y = y + size;
//      smoke_vertex_ptr->tx = 1.0f;           smoke_vertex_ptr->ty = 1.0f;
//      smoke_vertex_ptr->color_rgba = color;  smoke_vertex_ptr++;
//
//      smoke_vertex_ptr->x = x - size;        smoke_vertex_ptr->y = y - size;
//      smoke_vertex_ptr->tx = 0;              smoke_vertex_ptr->ty = 0;
//      smoke_vertex_ptr->color_rgba = color;  smoke_vertex_ptr++;
//
//      smoke_vertex_ptr->x = x - size;        smoke_vertex_ptr->y = y + size;
//      smoke_vertex_ptr->tx = 0;              smoke_vertex_ptr->ty = 1.0f;
//      smoke_vertex_ptr->color_rgba = color;  smoke_vertex_ptr++;
//   }
}
#endif //FIXEDMATH

#define LASER_ALPHA 100
#define LASER_LINE_ALPHA 50
#define LASER_LINE_ROLL_SPEED 17
#define LASER_LINE_UP_SPEED 16

// Original GL 1.2 code:
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

//static GLfixed laservertices[2*380];    // only 354 max seem to be used but to be safe we will reserve 380
//static GLubyte lasercolors[4*380];      

//senquack TODO: interleave
static gl_vertex laser_vertices[71];   // Only seem to need 61, this is just to be safe.
static gl_vertex *laser_vertex_ptr = NULL;

//senquack - new function called once before a series of calls to drawlaser (for openglES speedup)
inline void prepareDrawLaser (void)
{
   laser_vertex_ptr = &laser_vertices[0];
}

//senquack - with new triangle strip code, only 61 vertices total max (down from 300+)
inline void finishDrawLaser (void)
{
   int numvertices =
      ((unsigned int) laser_vertex_ptr - (unsigned int) &laser_vertices[0]) / sizeof(gl_vertex);
   if (numvertices == 0) return;    // Is there anything to draw?
#ifdef FIXEDMATH
   glVertexPointer (2, GL_FIXED, sizeof(gl_vertex), &laser_vertices[0].x);
#else
   glVertexPointer (2, GL_FLOAT, sizeof(gl_vertex), &laser_vertices[0].x);
#endif //FIXEDMATH
   glColorPointer (4, GL_UNSIGNED_BYTE, sizeof(gl_vertex), &laser_vertices[0].r);
   glDrawArrays (GL_TRIANGLE_STRIP, 0, numvertices);
   laser_vertex_ptr = &(laser_vertices[0]);
}

//senquack - heavily optimized triangle strip version:
#ifdef FIXEDMATH
void drawLaser (GLfixed x, GLfixed y, GLfixed width, GLfixed height,
            int cc1, int cnt, int type)
#else
void drawLaser (GLfloat x, GLfloat y, GLfloat width, GLfloat height,
            int cc1, int cnt, int type)
#endif //FIXEDMATH
{
   uint32_t color = (LASER_ALPHA << 24) | (cc1 << 8);
   if (type != 0) {
      laser_vertex_ptr->x = x - width;    laser_vertex_ptr->y = y+height;  laser_vertex_ptr->color_rgba = color;
      laser_vertex_ptr++;
      laser_vertex_ptr->x = x + width;    laser_vertex_ptr->y = y+height;  laser_vertex_ptr->color_rgba = color;
      laser_vertex_ptr++;
   } else {
      finishDrawLaser ();
      laser_vertex_ptr->x = x;    laser_vertex_ptr->y = y;  laser_vertex_ptr->color_rgba = color;
      laser_vertex_ptr++;
      laser_vertex_ptr->x = x - width;    laser_vertex_ptr->y = y+height;  laser_vertex_ptr->color_rgba = color;
      laser_vertex_ptr++;
      laser_vertex_ptr->x = x + width;    laser_vertex_ptr->y = y+height;  laser_vertex_ptr->color_rgba = color;
      laser_vertex_ptr++;
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

// Original GL 1.2 code:
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
static gl_vertex_3d ring_vertices[48];
static gl_vertex_3d *ring_vertex_ptr = NULL;
static inline void prepareDrawRings (void)
{
#ifdef FIXEDMATH
   glVertexPointer (3, GL_FIXED, sizeof(gl_vertex_3d), &ring_vertices[0].x);
#else
   glVertexPointer (3, GL_FLOAT, sizeof(gl_vertex_3d), &ring_vertices[0].x);
#endif //FIXEDMATH
   glColorPointer (4, GL_UNSIGNED_BYTE, sizeof(gl_vertex_3d), &ring_vertices[0].r);
}

#ifdef FIXEDMATH
static void drawRing (GLfixed x, GLfixed y, int d1, int d2, uint32_t color);
{
   printf("ERROR: Call to stub for fixed-point drawRing, please adapt drawRing to the new code\n");
   return;
//
//   int i, d;
////   float x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
//   GLfixed x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
//   glPushMatrix ();
//   glTranslatex (x, y, 0);
////  glRotatef((float)d1*360/1024, 0, 0, 1);
////  glRotatef((float)d2*360/1024, 1, 0, 0);
//   glRotatef ((float) ((d1 * 360) >> 10), 0, 0, 1);
//   glRotatef ((float) ((d2 * 360) >> 10), 1, 0, 0);
//
////  glColor4i(r, g, b, 255);
//   x1 = x2 = 0;
////  y1 = y4 =  CORE_HEIGHT/2;
//   y1 = y4 = CORE_HEIGHT_X >> 1;
////  y2 = y3 = -CORE_HEIGHT/2;
//   y2 = y3 = -(CORE_HEIGHT_X >> 1);
////  z1 = z2 = CORE_RING_SIZE;
//   z1 = z2 = CORE_RING_SIZE_X;
//   for (i = 0, d = 0; i < 8; i++) {
//      ringcolptr = &(ringcolors[0]);
//      ringvertptr = &(ringvertices[0]);
//      d += (1024 / 8);
//      d &= 1023;
////    x3 = x4 = sctbl[d+256]*CORE_RING_SIZE/256;
////    z3 = z4 = sctbl[d]    *CORE_RING_SIZE/256;
//      x3 = x4 = FMUL (INT2FNUM (sctbl[d + 256]), CORE_RING_SIZE_X) >> 8;
//      z3 = z4 = FMUL (INT2FNUM (sctbl[d]), CORE_RING_SIZE_X) >> 8;
////    drawSquare(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, r, g, b);
////    drawSquarex(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, r, g, b);
//      *ringcolptr++ = r; *ringcolptr++ = g; *ringcolptr++ = b; *ringcolptr++ = 64;
//      *ringcolptr++ = r; *ringcolptr++ = g; *ringcolptr++ = b; *ringcolptr++ = 64;
//      *ringcolptr++ = r; *ringcolptr++ = g; *ringcolptr++ = b; *ringcolptr++ = 64;
//      *ringcolptr++ = r; *ringcolptr++ = g; *ringcolptr++ = b; *ringcolptr++ = 64;
//      *ringvertptr++ = x1; *ringvertptr++ = y1; *ringvertptr++ = z1;
//      *ringvertptr++ = x2; *ringvertptr++ = y2; *ringvertptr++ = z2;
//      *ringvertptr++ = x3; *ringvertptr++ = y3; *ringvertptr++ = z3;
//      *ringvertptr++ = x4; *ringvertptr++ = y4; *ringvertptr++ = z4;
//      glDrawArrays (GL_TRIANGLE_FAN, 0, 4);
//      x1 = x3; y1 = y3; z1 = z3;
//      x2 = x4; y2 = y4; z2 = z4;
//   }
//   glPopMatrix ();
}
#else
//senquack - optimized GLES version:
static inline void drawRing (GLfloat x, GLfloat y, int d1, int d2, uint32_t color)
{
   int i, d;
   float x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
   glPushMatrix ();
   glTranslatef(x, y, 0);
   glRotatef ((float) ((d1 * 360) >> 10), 0, 0, 1);
   glRotatef ((float) ((d2 * 360) >> 10), 1, 0, 0);

   x1 = x2 = 0;
   y1 = y4 =  CORE_HEIGHT/2;
   y2 = y3 = -CORE_HEIGHT/2;
   z1 = z2 = CORE_RING_SIZE;

   ring_vertex_ptr = &ring_vertices[0];

   for (i = 0, d = 0; i < 8; i++) {
      d += (1024 / 8);
      d &= 1023;
      x3 = x4 = sctbl[d+256]*CORE_RING_SIZE/256;
      z3 = z4 = sctbl[d]    *CORE_RING_SIZE/256;
      ring_vertex_ptr->x = x1;  ring_vertex_ptr->y = y1;  ring_vertex_ptr->z = z1;
      ring_vertex_ptr->color_rgba = color;  ring_vertex_ptr++;
      ring_vertex_ptr->x = x2;  ring_vertex_ptr->y = y2;  ring_vertex_ptr->z = z2;
      ring_vertex_ptr->color_rgba = color;  ring_vertex_ptr++;
      ring_vertex_ptr->x = x3;  ring_vertex_ptr->y = y3;  ring_vertex_ptr->z = z3;
      ring_vertex_ptr->color_rgba = color;  ring_vertex_ptr++;

      ring_vertex_ptr->x = x1;  ring_vertex_ptr->y = y1;  ring_vertex_ptr->z = z1;
      ring_vertex_ptr->color_rgba = color;  ring_vertex_ptr++;
      ring_vertex_ptr->x = x3;  ring_vertex_ptr->y = y3;  ring_vertex_ptr->z = z3;
      ring_vertex_ptr->color_rgba = color;  ring_vertex_ptr++;
      ring_vertex_ptr->x = x4;  ring_vertex_ptr->y = y4;  ring_vertex_ptr->z = z4;
      ring_vertex_ptr->color_rgba = color;  ring_vertex_ptr++;

      x1 = x3; y1 = y3; z1 = z3;
      x2 = x4; y2 = y4; z2 = z4;
   }
   int num_vertices = ((unsigned int)ring_vertex_ptr - (unsigned int)&ring_vertices[0]) / sizeof(gl_vertex_3d);
   glDrawArrays (GL_TRIANGLE_STRIP, 0, num_vertices);
   glPopMatrix ();
}
#endif //FIXEDMATH

// Original GL 1.2 code:
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
#ifdef FIXEDMATH
void drawCore (GLfixed x, GLfixed y, int cnt, int r, int g, int b)
{
   printf("ERROR: Call to stub for fixed-point drawCore, please adapt drawRing to the new code\n");
   return;
//   int i;
////  float cy;
//   GLfixed cy;
//   glPushMatrix ();
//   glTranslatex (x, y, 0);
////  glColor4i(r, g, b, 255);
//
//   GLubyte colors[4 * 4];
//   GLfixed vertices[4 * 2];
//   colors[0] = colors[4] = colors[8] = colors[12] = r;
//   colors[1] = colors[5] = colors[9] = colors[13] = g;
//   colors[2] = colors[6] = colors[10] = colors[14] = b;
//   colors[3] = colors[7] = colors[11] = colors[15] = 255;
//
//   vertices[0] = -SHAPE_POINT_SIZE_L_X;
//   vertices[1] = -SHAPE_POINT_SIZE_L_X;
//   vertices[2] = SHAPE_POINT_SIZE_L_X;
//   vertices[3] = -SHAPE_POINT_SIZE_L_X;
//   vertices[4] = SHAPE_POINT_SIZE_L_X;
//   vertices[5] = SHAPE_POINT_SIZE_L_X;
//   vertices[6] = -SHAPE_POINT_SIZE_L_X;
//   vertices[7] = SHAPE_POINT_SIZE_L_X;
//
//// glEnableClientState(GL_VERTEX_ARRAY);
//   glVertexPointer (2, GL_FIXED, 0, vertices);
//// glEnableClientState(GL_COLOR_ARRAY);
//   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
//   glDrawArrays (GL_TRIANGLE_FAN, 0, 4);
//
//   glPopMatrix ();
//
////  cy = y - CORE_HEIGHT*2.5f;
//   cy = y - FMUL (CORE_HEIGHT_X, 163840);
//
//   prepareDrawRing ();
////  for ( i=0 ; i<4 ; i++, cy+=CORE_HEIGHT ) {
//   for (i = 0; i < 4; i++, cy += CORE_HEIGHT_X) {
////    drawRing(x, cy, (cnt*(4+i))&1023, (sctbl[(cnt*(5+i))&1023]/4)&1023, r, g, b);
////    drawRingx(fx, fcy, (cnt*(4+i))&1023, (sctbl[(cnt*(5+i))&1023]/4)&1023, r, g, b);
//      drawRing (x, cy, (cnt * (4 + i)) & 1023,
//                 (sctbl[(cnt * (5 + i)) & 1023] >> 2) & 1023, r, g, b);
//   }
}
#else
//senquack - optimized GLES version:
void drawCore (GLfloat x, GLfloat y, int cnt, int r, int g, int b)
{
   int i;
   float cy;
   uint32_t color = (255 << 24) | (b << 16) | (g << 8) | r;

   triangles_ptr->x = x - SHAPE_POINT_SIZE_L;    triangles_ptr->y = y - SHAPE_POINT_SIZE_L;
   triangles_ptr->color_rgba = color;            triangles_ptr++;
   triangles_ptr->x = x + SHAPE_POINT_SIZE_L;    triangles_ptr->y = y - SHAPE_POINT_SIZE_L;
   triangles_ptr->color_rgba = color;            triangles_ptr++;
   triangles_ptr->x = x + SHAPE_POINT_SIZE_L;    triangles_ptr->y = y + SHAPE_POINT_SIZE_L;
   triangles_ptr->color_rgba = color;            triangles_ptr++;
   triangles_ptr->x = x - SHAPE_POINT_SIZE_L;    triangles_ptr->y = y - SHAPE_POINT_SIZE_L;
   triangles_ptr->color_rgba = color;            triangles_ptr++;
   triangles_ptr->x = x + SHAPE_POINT_SIZE_L;    triangles_ptr->y = y + SHAPE_POINT_SIZE_L;
   triangles_ptr->color_rgba = color;            triangles_ptr++;
   triangles_ptr->x = x - SHAPE_POINT_SIZE_L;    triangles_ptr->y = y + SHAPE_POINT_SIZE_L;
   triangles_ptr->color_rgba = color;            triangles_ptr++;

   cy = y - CORE_HEIGHT*2.5f;
   prepareDrawRings ();
   color = (64 << 24) | (b << 16) | (g << 8) | r;
   for ( i=0 ; i<4 ; i++, cy+=CORE_HEIGHT ) {
      drawRing(x, cy, (cnt*(4+i))&1023, (sctbl[(cnt*(5+i))&1023]/4)&1023, color);
   }
}
#endif //FIXEDMATH

#define SHIP_DRUM_R 0.4f
#define SHIP_DRUM_R_X 26214     //fixed point version -senquack
#define SHIP_DRUM_WIDTH 0.05f
#define SHIP_DRUM_WIDTH_X 3277  //fixed point version -senquack
#define SHIP_DRUM_HEIGHT 0.35f
#define SHIP_DRUM_HEIGHT_X 22938    //fixed point version -senquack

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
#ifdef FIXEDMATH
//senquack TODO: adapt my older fixed point code to the new interleaved form below
void drawShipShape (GLfixed x, GLfixed y, float d, int inv)
{
   int i;
   glPushMatrix ();
   glTranslatex (x, y, 0);

   GLubyte colors[4 * 4];
   GLfixed vertices[4 * 3];
   colors[0] = colors[4] = colors[8] = colors[12] = 255;
   colors[1] = colors[5] = colors[9] = colors[13] = colors[2] = colors[6] = colors[10] = colors[14] = 100;
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
#else
//senquack - optimized GLES version:
void drawShipShape (GLfloat x, GLfloat y, float d, int inv)
{
   glPushMatrix ();
   glTranslatef (x, y, 0);
   
   int i;
   uint32_t color;
   gl_vertex_3d vertices[5];
   gl_vertex_3d *vertex_ptr = &vertices[0];

   color = (255 << 24) | (100 << 16) | (100 << 8) | 255;
   vertex_ptr->x = -SHAPE_POINT_SIZE_L; vertex_ptr->y = -SHAPE_POINT_SIZE_L; vertex_ptr->z = 0;
   vertex_ptr->color_rgba = color; vertex_ptr++;
   vertex_ptr->x = SHAPE_POINT_SIZE_L; vertex_ptr->y = -SHAPE_POINT_SIZE_L; vertex_ptr->z = 0;
   vertex_ptr->color_rgba = color; vertex_ptr++;
   vertex_ptr->x = SHAPE_POINT_SIZE_L; vertex_ptr->y = SHAPE_POINT_SIZE_L; vertex_ptr->z = 0;
   vertex_ptr->color_rgba = color; vertex_ptr++;
   vertex_ptr->x = -SHAPE_POINT_SIZE_L; vertex_ptr->y = SHAPE_POINT_SIZE_L; vertex_ptr->z = 0;
   vertex_ptr->color_rgba = color; vertex_ptr++;

   glVertexPointer (3, GL_FLOAT, sizeof(gl_vertex_3d), &vertices[0].x);
   glColorPointer (4, GL_UNSIGNED_BYTE, sizeof(gl_vertex_3d), &vertices[0].r);
   glDrawArrays (GL_TRIANGLE_FAN, 0, 4);

   if (inv) {
      glPopMatrix ();
      return;
   }

   //senquack - working around bug on etnaviv, have to use LINE_STRIP instead of LINE_LOOP (1 extra vertex needed)
   glRotatef (d, 0, 1.0f, 0);
   //    glColor4i(120, 220, 100, 150);
   color = (150 << 24) | (100 << 16) | (220 << 8) | 120;
   vertex_ptr = &vertices[0];
   vertex_ptr->x = -SHIP_DRUM_WIDTH;   vertex_ptr->y = -SHIP_DRUM_HEIGHT;  vertex_ptr->z = SHIP_DRUM_R;
   vertex_ptr->color_rgba = color;  vertex_ptr++;
   vertex_ptr->x =  SHIP_DRUM_WIDTH;   vertex_ptr->y = -SHIP_DRUM_HEIGHT;  vertex_ptr->z = SHIP_DRUM_R;
   vertex_ptr->color_rgba = color;  vertex_ptr++;
   vertex_ptr->x =  SHIP_DRUM_WIDTH;   vertex_ptr->y =  SHIP_DRUM_HEIGHT;  vertex_ptr->z = SHIP_DRUM_R;
   vertex_ptr->color_rgba = color;  vertex_ptr++;
   vertex_ptr->x = -SHIP_DRUM_WIDTH;   vertex_ptr->y =  SHIP_DRUM_HEIGHT;  vertex_ptr->z = SHIP_DRUM_R;
   vertex_ptr->color_rgba = color;  vertex_ptr++;
   vertex_ptr->x = -SHIP_DRUM_WIDTH;   vertex_ptr->y = -SHIP_DRUM_HEIGHT;  vertex_ptr->z = SHIP_DRUM_R;
   vertex_ptr->color_rgba = color;  vertex_ptr++;

   glVertexPointer (3, GL_FLOAT, sizeof(gl_vertex_3d), &vertices[0].x);
   glColorPointer (4, GL_UNSIGNED_BYTE, sizeof(gl_vertex_3d), &vertices[0].r);

   for ( i=0 ; i<8 ; i++ ) {
      glRotatef (45.0f, 0, 1.0f, 0);
      glDrawArrays (GL_LINE_STRIP, 0 , 5);
   }

   glPopMatrix ();
}
#endif //FIXEDMATH

#ifdef FIXEDMATH
// My older fixed-style code that needs updating
void drawBomb (GLfixed x, GLfixed y, GLfixed width, int cnt)
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
#else
//senquack - optimized GLES float version
void drawBomb (GLfloat x, GLfloat y, GLfloat width, int cnt)
{
   int i, d, od, c;
   gl_vertex vertices[17];
   gl_vertex *vertex_ptr = &vertices[0];
   glVertexPointer (2, GL_FLOAT, sizeof(gl_vertex), &vertices[0].x);
   glColorPointer (4, GL_UNSIGNED_BYTE, sizeof(gl_vertex), &vertices[0].r);
   uint32_t color = 0xFFFFFFFF;  // Pure white color

   d = cnt * 48;
   d &= 1023;
   c = 4 + (cnt >> 3);
   if (c > 16)
      c = 16;
   od = 1024 / c;

   vertex_ptr->x = (sctbl[d]    *width) * (1.0f / 256.0f) + x;
   vertex_ptr->y = (sctbl[d+256]*width) * (1.0f / 256.0f) + y;
   vertex_ptr->color_rgba = color;
   vertex_ptr++;

   for (i = 0; i < c; i++) {
      d += od;
      d &= 1023;
      vertex_ptr->x = (sctbl[d]    *width) * (1.0f / 256.0f) + x;
      vertex_ptr->y = (sctbl[d+256]*width) * (1.0f / 256.0f) + y;
      vertex_ptr->color_rgba = color;
      vertex_ptr++;
   }
   glDrawArrays (GL_LINE_STRIP, 0, 1 + c);
}
#endif //FIXEDMATH

#ifdef FIXEDMATH
//senquack - TODO: adapt my older fixed-point code to my more optimized code in the float version below
void drawCircle (GLfixed x, GLfixed y, GLfixed width, int cnt,
                   int r1, int g1, int b1, int r2, int b2, int g2)
{
   int i, d;
   GLfixed x1, y1, x2, y2;

   GLubyte colors[33 * 4];
   GLfixed vertices[33 * 2];

   if ((cnt & 1) == 0) {
      colors[0] = r1; colors[1] = g1; colors[2] = b1;
   } else {
      //senquack - TODO: double check I did this right for the Wiz port:
      colors[0] = colors[1] = colors[2] = 64;
   }
   colors[3] = 64;
   vertices[0] = x;
   vertices[1] = y;
   d = cnt * 48;
   d &= 1023;
//  x1 = (sctbl[d]    *width)/256 + x;
//  y1 = (sctbl[d+256]*width)/256 + y;
   x1 = (FMUL (INT2FNUM (sctbl[d]), width) >> 8) + x;
   y1 = (FMUL (INT2FNUM (sctbl[d + 256]), width) >> 8) + y;
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
      vertices[2 + (i << 2)] = x1;
      vertices[2 + (i << 2) + 1] = y1;
      vertices[2 + (i << 2) + 2] = x2;
      vertices[2 + (i << 2) + 3] = y2;
      x1 = x2;
      y1 = y2;
   }
   glVertexPointer (2, GL_FIXED, 0, vertices);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);
   glDrawArrays (GL_TRIANGLE_FAN, 0, 33);
}
#else
//senquack - Optimized GLES float version
void drawCircle (GLfloat x, GLfloat y, GLfloat width, int cnt,
                   int r1, int g1, int b1, int r2, int b2, int g2)
{
   int i, d;
   GLfloat x1, y1, x2, y2;
   uint32_t color;
   gl_vertex vertices[33];
   gl_vertex *vertex_ptr = &vertices[0];

   if ( (cnt&1) == 0 ) {
      color = (64 << 24) | (b1 << 16) | (g1 << 8) | r1;
   } else {
      color = (64 << 24) | 0xFFFFFF;
   }
  
   vertex_ptr->x = x;   vertex_ptr->y = y;   vertex_ptr->color_rgba = color;  vertex_ptr++;

   d = cnt*48; d &= 1023;
   x1 = ((sctbl[d]    *width) * (1.0f/256.0f)) + x;
   y1 = ((sctbl[d+256]*width) * (1.0f/256.0f)) + y;
   color = (150 << 24) | (b2 << 16) | (g2 << 8) | r2;

   for ( i=0 ; i<16 ; i++ ) {
      d += 64; d &= 1023;
      x2 = ((sctbl[d]    *width) * (1.0f/256.0f)) + x;
      y2 = ((sctbl[d+256]*width) * (1.0f/256.0f)) + y;
      vertex_ptr->x = x1;   vertex_ptr->y = y1;   vertex_ptr->color_rgba = color;  vertex_ptr++;
      vertex_ptr->x = x2;   vertex_ptr->y = y2;   vertex_ptr->color_rgba = color;  vertex_ptr++;
      x1 = x2; y1 = y2;
   }

   glVertexPointer (2, GL_FLOAT, sizeof(gl_vertex), &vertices[0].x);
   glColorPointer (4, GL_UNSIGNED_BYTE, sizeof(gl_vertex), &vertices[0].r);
   glDrawArrays (GL_TRIANGLE_FAN, 0, 33);
}
#endif //FIXEDMATH

#ifdef FIXEDMATH
/*senquack - Original GLES Wiz port code is below, and it has NOT been optimized fully. Currently broken
             after tweaking the float version.
//             TODO: Update it to be compatible with the  highly-optimized float version found below */
void drawShape (GLfixed x, GLfixed y, GLfixed size, int d, int cnt, int type, int r, int g, int b)
{
   printf("ERROR: Call to fixed-point version of drawShape() that is not ready for use yet.\n");
   return;

//   GLfixed sz, sz2;
//   glPushMatrix ();
//   glTranslatex (x, y, 0);
//   
//   // The core dots that make up each shape are drawn after all shapes are drawn in one huge batch:
//   *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 220;
//   *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 220;
//   *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 220;
//   *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 220;
//   *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 220;
//   *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 220;
//   *shapeptvertptr++ = x - SHAPE_POINT_SIZE_X;
//   *shapeptvertptr++ = y - SHAPE_POINT_SIZE_X;
//   *shapeptvertptr++ = x - SHAPE_POINT_SIZE_X;
//   *shapeptvertptr++ = y + SHAPE_POINT_SIZE_X;
//   *shapeptvertptr++ = x + SHAPE_POINT_SIZE_X;
//   *shapeptvertptr++ = y - SHAPE_POINT_SIZE_X;
//   *shapeptvertptr++ = x + SHAPE_POINT_SIZE_X;
//   *shapeptvertptr++ = y + SHAPE_POINT_SIZE_X;
//   *shapeptvertptr++ = x + SHAPE_POINT_SIZE_X;
//   *shapeptvertptr++ = y - SHAPE_POINT_SIZE_X;
//   *shapeptvertptr++ = x - SHAPE_POINT_SIZE_X;
//   *shapeptvertptr++ = y + SHAPE_POINT_SIZE_X;
//
//   switch (type) {
//   case 0:
//      sz = size >> 1;
//      glRotatef ((float) ((d * 360) >> 10), 0, 0, 1);
//      shapecolors[0] = shapecolors[4] = r;
//      shapecolors[1] = shapecolors[5] = g;
//      shapecolors[2] = shapecolors[6] = b;
//      shapecolors[3] = shapecolors[7] = shapecolors[11] = 150;
//      shapecolors[8] = SHAPE_BASE_COLOR_R;
//      shapecolors[9] = SHAPE_BASE_COLOR_G;
//      shapecolors[10] = SHAPE_BASE_COLOR_B;
//      shapevertices[0] = -sz;
//      shapevertices[1] = -sz;
//      shapevertices[2] = sz;
//      shapevertices[3] = -sz;
//      shapevertices[4] = 0;
//      shapevertices[5] = size;
//      glDrawArrays (GL_TRIANGLES, 0, 3);
//      break;
//   case 1:
//      sz = size >> 1;
//      glRotatef ((float) ((((cnt * 23) & 1023) * 360) >> 10), 0, 0, 1);
//      shapecolors[0] = shapecolors[4] = r;
//      shapecolors[1] = shapecolors[5] = g;
//      shapecolors[2] = shapecolors[6] = b;
//      shapecolors[3] = shapecolors[7] = 180;
//      shapecolors[8] = shapecolors[12] = SHAPE_BASE_COLOR_R;
//      shapecolors[9] = shapecolors[13] = SHAPE_BASE_COLOR_G;
//      shapecolors[10] = shapecolors[14] = SHAPE_BASE_COLOR_B;
//      shapecolors[11] = shapecolors[15] = 150;
//      shapevertices[0] = 0;
//      shapevertices[1] = -size;
//      shapevertices[2] = sz;
//      shapevertices[3] = 0;
//      shapevertices[4] = 0;
//      shapevertices[5] = size;
//      shapevertices[6] = -sz;
//      shapevertices[7] = 0;
//      glDrawArrays (GL_TRIANGLE_FAN, 0, 4);
//      break;
//   case 2:
//      sz = size >> 2;
//      sz2 = FMUL (size, 43691);    //43690 = 2/3 in fixed point
//      glRotatef ((float) ((d * 360) >> 10), 0, 0, 1);
//      shapecolors[0] = shapecolors[4] = r;
//      shapecolors[1] = shapecolors[5] = g;
//      shapecolors[2] = shapecolors[6] = b;
//      shapecolors[3] = shapecolors[7] = 120;
//      shapecolors[8] = shapecolors[12] = SHAPE_BASE_COLOR_R;
//      shapecolors[9] = shapecolors[13] = SHAPE_BASE_COLOR_G;
//      shapecolors[10] = shapecolors[14] = SHAPE_BASE_COLOR_B;
//      shapecolors[11] = shapecolors[15] = 150;
//      shapevertices[0] = -sz;
//      shapevertices[1] = -sz2;
//      shapevertices[2] = sz;
//      shapevertices[3] = -sz2;
//      shapevertices[4] = sz;
//      shapevertices[5] = sz2;
//      shapevertices[6] = -sz;
//      shapevertices[7] = sz2;
//      glDrawArrays (GL_TRIANGLE_FAN, 0, 4);
//      break;
//   case 3:
//      sz = size >> 1;
//      glRotatef ((float) ((((cnt * 37) & 1023) * 360) >> 10), 0, 0, 1);
//      shapecolors[0] = shapecolors[4] = r;
//      shapecolors[1] = shapecolors[5] = g;
//      shapecolors[2] = shapecolors[6] = b;
//      shapecolors[3] = shapecolors[7] = 180;
//      shapecolors[8] = shapecolors[12] = SHAPE_BASE_COLOR_R;
//      shapecolors[9] = shapecolors[13] = SHAPE_BASE_COLOR_G;
//      shapecolors[10] = shapecolors[14] = SHAPE_BASE_COLOR_B;
//      shapecolors[11] = shapecolors[15] = 150;
//      shapevertices[0] = -sz;
//      shapevertices[1] = -sz;
//      shapevertices[2] = sz;
//      shapevertices[3] = -sz;
//      shapevertices[4] = sz;
//      shapevertices[5] = sz;
//      shapevertices[6] = -sz;
//      shapevertices[7] = sz;
//      glDrawArrays (GL_TRIANGLE_FAN, 0, 4);
//      break;
//   case 4:
//      sz = size >> 1;
//      sz2 = size >> 2;         // added this because of simplicity later on
//      glRotatef ((float) ((((cnt * 53) & 1023) * 360) >> 10), 0, 0, 1);
//      shapecolors[0] = shapecolors[4] = shapecolors[8] = shapecolors[12] = r;
//      shapecolors[1] = shapecolors[5] = shapecolors[9] = shapecolors[13] = g;
//      shapecolors[2] = shapecolors[6] = shapecolors[10] = shapecolors[14] = b;
//      shapecolors[3] = shapecolors[7] = shapecolors[11] = shapecolors[15] =
//         220;
//      shapecolors[16] = shapecolors[20] = shapecolors[24] = shapecolors[28] =
//         SHAPE_BASE_COLOR_R;
//      shapecolors[17] = shapecolors[21] = shapecolors[25] = shapecolors[29] =
//         SHAPE_BASE_COLOR_G;
//      shapecolors[18] = shapecolors[22] = shapecolors[26] = shapecolors[30] =
//         SHAPE_BASE_COLOR_B;
//      shapecolors[19] = shapecolors[23] = shapecolors[27] = shapecolors[31] =
//         150;
//      shapevertices[0] = -sz2;
//      shapevertices[1] = -sz;
//      shapevertices[2] = sz2;
//      shapevertices[3] = -sz;
//      shapevertices[4] = sz;
//      shapevertices[5] = -sz2;
//      shapevertices[6] = sz;
//      shapevertices[7] = sz2;
//      shapevertices[8] = sz2;
//      shapevertices[9] = sz;
//      shapevertices[10] = -sz2;
//      shapevertices[11] = sz;
//      shapevertices[12] = -sz;
//      shapevertices[13] = sz2;
//      shapevertices[14] = -sz;
//      shapevertices[15] = -sz2;
//      glDrawArrays (GL_TRIANGLE_FAN, 0, 8);
//      break;
//
//   case 5:
//      sz = FMUL (size, 43691); //43690 = 2/3 in fixed point
//      sz2 = FMUL (size, 13107);    //13107 is 1/5 in fixed point
//
//      glRotatef ((float) ((d * 360) >> 10), 0, 0, 1);
//      shapecolors[0] = shapecolors[4] = r;
//      shapecolors[1] = shapecolors[5] = g;
//      shapecolors[2] = shapecolors[6] = b;
//      shapecolors[3] = shapecolors[7] = shapecolors[11] = 150;
//      shapecolors[8] = SHAPE_BASE_COLOR_R;
//      shapecolors[9] = SHAPE_BASE_COLOR_G;
//      shapecolors[10] = SHAPE_BASE_COLOR_B;
//      shapevertices[0] = -sz;
//      shapevertices[1] = -sz + sz2;
//      shapevertices[2] = sz;
//      shapevertices[3] = -sz + sz2;
//      shapevertices[4] = 0;
//      shapevertices[5] = sz + sz2;
//      glDrawArrays (GL_TRIANGLES, 0, 3);
//      break;
//   case 6:
//      sz = size >> 1;
//      glRotatef ((float) ((((cnt * 13) & 1023) * 360) >> 10), 0, 0, 1);
//      shapecolors[0] = shapecolors[4] = shapecolors[8] = r;
//      shapecolors[1] = shapecolors[5] = shapecolors[9] = g;
//      shapecolors[2] = shapecolors[6] = shapecolors[10] = b;
//      shapecolors[3] = shapecolors[7] = shapecolors[11] = 210;
//      shapecolors[12] = shapecolors[16] = shapecolors[20] = SHAPE_BASE_COLOR_R;
//      shapecolors[13] = shapecolors[17] = shapecolors[21] = SHAPE_BASE_COLOR_G;
//      shapecolors[14] = shapecolors[18] = shapecolors[22] = SHAPE_BASE_COLOR_B;
//      shapecolors[15] = shapecolors[19] = shapecolors[23] = 150;
//      shapevertices[0] = -sz;
//      shapevertices[1] = -sz;
//      shapevertices[2] = 0;
//      shapevertices[3] = -sz;
//      shapevertices[4] = sz;
//      shapevertices[5] = 0;
//      shapevertices[6] = sz;
//      shapevertices[7] = sz;
//      shapevertices[8] = 0;
//      shapevertices[9] = sz;
//      shapevertices[10] = -sz;
//      shapevertices[11] = 0;
//      glDrawArrays (GL_TRIANGLE_FAN, 0, 6);
//      break;
//   }
//   glPopMatrix ();
}
#else

/* senquack - HIGHLY OPTIMIZED POST-WIZ-PORT BATCH-DRAWN SHAPE CODE BEGINS HERE: */
void prepareDrawBatch (void)
{
   triangles_ptr = &triangles[0];
   lines_ptr = &lines[0];
}

void finishDrawBatch (void)
{
   int num_vertices;

   //First, draw the triangles:
   num_vertices = ((unsigned int)triangles_ptr - (unsigned int)(&triangles[0])) / sizeof(gl_vertex);
   if (num_vertices >= 3) {
#ifdef FIXEDMATH
      glVertexPointer (2, GL_FIXED, sizeof(gl_vertex), &triangles[0].x);
#else
      glVertexPointer (2, GL_FLOAT, sizeof(gl_vertex), &triangles[0].x);
#endif //FIXEDMATH
      glColorPointer (4, GL_UNSIGNED_BYTE, sizeof(gl_vertex), &triangles[0].r);
      glDrawArrays (GL_TRIANGLES, 0, num_vertices);
//      printf("drawing triangles with %d vertices, %d allocated\n", num_vertices, sizeof(triangles) / sizeof(gl_vertex));
   }

   //Second, draw the lines:
   num_vertices = ((unsigned int)lines_ptr - (unsigned int)(&lines[0])) / sizeof(gl_vertex);
   if (num_vertices >= 2) {
#ifdef FIXEDMATH
      glVertexPointer (2, GL_FIXED, sizeof(gl_vertex), &lines[0].x);
#else
      glVertexPointer (2, GL_FLOAT, sizeof(gl_vertex), &lines[0].x);
#endif //FIXEDMATH
      glColorPointer (4, GL_UNSIGNED_BYTE, sizeof(gl_vertex), &lines[0].r);
      glDrawArrays (GL_LINES, 0, num_vertices);
//      printf("drawing lines with %d vertices, %d allocated\n", num_vertices, sizeof(triangles) / sizeof(gl_vertex));
   }
}

/* senquack - For OpenGLES conversion, all bullet drawing is now done in one huge batch.
   In order to allow this, all vertex rotation is now done on the CPU through the
   game's orginal sin/cos lookup table.  This saves thousands of calls into OpenGLES
   I achieved a 6-fold increase in framerate on the GCW (at minimum) in many scenes. */
void drawShape (GLfloat x, GLfloat y, GLfloat size, int d, int cnt, int type, int r, int g, int b)
{
   GLfloat sz, sz2;       
   GLfloat shiftx, shifty;
   gl_vertex vertices[16];
   uint32_t base_color = (b << 16) | (g << 8) | r;
   uint32_t core_color = (255 << 24) | base_color;
   uint32_t color; 

   switch (type) {
      case 0: /* CASE 0 : SMALL TRIANGLE SHAPE */
         sz = size * 0.5f; sz2 = size * 0.75f;
         shiftx = -SIN_LOOKUP(d) * size * 0.25f;   shifty = COS_LOOKUP(d) * size * 0.25f;
         color = (150 << 24) | base_color;
         vertices[0].x = x + X_ROT(-sz, -sz2, d) + shiftx; vertices[0].y = y + Y_ROT(-sz, -sz2, d) + shifty; 
         vertices[0].color_rgba = color;
         vertices[1].x = x + X_ROT(sz, -sz2, d) + shiftx;  vertices[1].y = y + Y_ROT(sz, -sz2, d) + shifty;  
         vertices[1].color_rgba = color;
         color = (150 << 24) | (SHAPE_BASE_COLOR_B << 16) | (SHAPE_BASE_COLOR_G << 8) | SHAPE_BASE_COLOR_R;
         vertices[2].x = x + X_ROT(0, sz2, d) + shiftx;  vertices[2].y = y + Y_ROT(0, sz2, d) + shifty;  
         vertices[2].color_rgba = color;
         *triangles_ptr++ = vertices[0];   //Triangle 1 / 1
         *triangles_ptr++ = vertices[1];
         *triangles_ptr++ = vertices[2];

         /* Lines: */
         if (settings.draw_outlines == DRAW_OUTLINES_ALL) {
            vertices[0].color_rgba = core_color;
            vertices[1].color_rgba = core_color;
            vertices[2].color_rgba = core_color;
            *lines_ptr++ = vertices[0];
            *lines_ptr++ = vertices[1];
            *lines_ptr++ = vertices[1];
            *lines_ptr++ = vertices[2];
            *lines_ptr++ = vertices[2];
            *lines_ptr++ = vertices[0];
         }
         break;
      case 1:
         /* CASE 1: DIAMOND SHAPE */
         sz = size * 0.5f;
         d = (cnt*23)&1023;
         color = (180 << 24) | base_color;
         vertices[0].x = x + X_ROT(0, -size, d); vertices[0].y = y + Y_ROT(0, -size, d); vertices[0].color_rgba = color;
         vertices[1].x = x + X_ROT(sz, 0, d);    vertices[1].y = y + Y_ROT(sz, 0, d);    vertices[1].color_rgba = color;
         color = (150 << 24) | (SHAPE_BASE_COLOR_B << 16) | (SHAPE_BASE_COLOR_G << 8) | SHAPE_BASE_COLOR_R;
         vertices[2].x = x + X_ROT(0, size, d);  vertices[2].y = y + Y_ROT(0, size, d);  vertices[2].color_rgba = color;
         vertices[3].x = x + X_ROT(-sz, 0, d);   vertices[3].y = y + Y_ROT(-sz, 0, d);   vertices[3].color_rgba = color;
         *triangles_ptr++ = vertices[0];   //Triangle 1
         *triangles_ptr++ = vertices[1];
         *triangles_ptr++ = vertices[2];
         *triangles_ptr++ = vertices[2];   //Triangle 2
         *triangles_ptr++ = vertices[3];
         *triangles_ptr++ = vertices[0];

         /* Lines: */
         if (settings.draw_outlines == DRAW_OUTLINES_ALL) {
            vertices[0].color_rgba = core_color;
            vertices[1].color_rgba = core_color;
            vertices[2].color_rgba = core_color;
            vertices[3].color_rgba = core_color;
            *lines_ptr++ = vertices[0];
            *lines_ptr++ = vertices[1];
            *lines_ptr++ = vertices[1];
            *lines_ptr++ = vertices[2];
            *lines_ptr++ = vertices[2];
            *lines_ptr++ = vertices[3];
            *lines_ptr++ = vertices[3];
            *lines_ptr++ = vertices[0];
         }
         break;
      case 2: /* CASE 2 - RECTANGULAR SHAPE */
         sz = size * 0.25f;
         sz2 = size * (2.0f/3.0f);
         color = (120 << 24) | base_color;
         vertices[0].x = x + X_ROT(-sz,-sz2,d); vertices[0].y = y + Y_ROT(-sz,-sz2,d); vertices[0].color_rgba = color;
         vertices[1].x = x + X_ROT(sz,-sz2, d); vertices[1].y = y + Y_ROT(sz,-sz2, d); vertices[1].color_rgba = color;
         color = (150 << 24) | (SHAPE_BASE_COLOR_B << 16) | (SHAPE_BASE_COLOR_G << 8) | SHAPE_BASE_COLOR_R;
         vertices[2].x = x + X_ROT(sz,sz2,d);   vertices[2].y = y + Y_ROT(sz,sz2,d);   vertices[2].color_rgba = color;
         vertices[3].x = x + X_ROT(-sz,sz2,d);   vertices[3].y = y + Y_ROT(-sz,sz2,d); vertices[3].color_rgba = color;
         *triangles_ptr++ = vertices[0];   //Triangle 1 / 2
         *triangles_ptr++ = vertices[1];   
         *triangles_ptr++ = vertices[2];   
         *triangles_ptr++ = vertices[2];   //Triangle 2 / 2
         *triangles_ptr++ = vertices[3];  
         *triangles_ptr++ = vertices[0];  

         /* Lines: */
         if (settings.draw_outlines == DRAW_OUTLINES_ALL) {
            vertices[0].color_rgba = core_color;
            vertices[1].color_rgba = core_color;
            vertices[2].color_rgba = core_color;
            vertices[3].color_rgba = core_color;
            *lines_ptr++ = vertices[0];
            *lines_ptr++ = vertices[1];
            *lines_ptr++ = vertices[1];
            *lines_ptr++ = vertices[2];
            *lines_ptr++ = vertices[2];
            *lines_ptr++ = vertices[3];
            *lines_ptr++ = vertices[3];
            *lines_ptr++ = vertices[0];
         }
         break;
      case 3:
         /* CASE 3 - SQUARE SHAPE */
         sz = size * 0.5f;
         d = (cnt*37)&1023;
         color = (180 << 24) | base_color;
         vertices[0].x = x + X_ROT(-sz,-sz,d); vertices[0].y = y + Y_ROT(-sz,-sz,d); vertices[0].color_rgba = color;
         vertices[1].x = x + X_ROT(sz,-sz, d); vertices[1].y = y + Y_ROT(sz,-sz, d); vertices[1].color_rgba = color;
         color = (150 << 24) | (SHAPE_BASE_COLOR_B << 16) | (SHAPE_BASE_COLOR_G << 8) | SHAPE_BASE_COLOR_R;
         vertices[2].x = x + X_ROT(sz,sz,d);   vertices[2].y = y + Y_ROT(sz,sz,d);   vertices[2].color_rgba = color;
         vertices[3].x = x + X_ROT(-sz,sz,d);  vertices[3].y = y + Y_ROT(-sz,sz,d);  vertices[3].color_rgba = color;
         *triangles_ptr++ = vertices[0];   //Triangle 1 / 2
         *triangles_ptr++ = vertices[1];   
         *triangles_ptr++ = vertices[2];   
         *triangles_ptr++ = vertices[2];   //Triangle 2 / 2
         *triangles_ptr++ = vertices[3];   
         *triangles_ptr++ = vertices[0];   

         /* Lines: */
         if (settings.draw_outlines == DRAW_OUTLINES_ALL) {
            vertices[0].color_rgba = core_color;
            vertices[1].color_rgba = core_color;
            vertices[2].color_rgba = core_color;
            vertices[3].color_rgba = core_color;
            *lines_ptr++ = vertices[0];
            *lines_ptr++ = vertices[1];
            *lines_ptr++ = vertices[1];
            *lines_ptr++ = vertices[2];
            *lines_ptr++ = vertices[2];
            *lines_ptr++ = vertices[3];
            *lines_ptr++ = vertices[3];
            *lines_ptr++ = vertices[0];
         }
         break;
      case 4: //      /* CASE 4 - CIRCULAR SHAPE */
         sz = size * 0.5f;
         sz2 = size * 0.25f;        
         d = (cnt * 53) & 1023;
         d = (cnt*37)&1023;
         color = (220 << 24) | base_color;
         vertices[0].x = x + X_ROT(-sz2,-sz,d);  vertices[0].y = y + Y_ROT(-sz2,-sz,d);  vertices[0].color_rgba = color;
         vertices[1].x = x + X_ROT(sz2,-sz, d);  vertices[1].y = y + Y_ROT(sz2,-sz, d);  vertices[1].color_rgba = color;
         vertices[2].x = x + X_ROT(sz,-sz2,d);   vertices[2].y = y + Y_ROT(sz,-sz2,d);   vertices[2].color_rgba = color;
         vertices[3].x = x + X_ROT(sz,sz2,d);    vertices[3].y = y + Y_ROT(sz,sz2,d);    vertices[3].color_rgba = color;
         color = (150 << 24) | (SHAPE_BASE_COLOR_B << 16) | (SHAPE_BASE_COLOR_G << 8) | SHAPE_BASE_COLOR_R;
         vertices[4].x = x + X_ROT(sz2,sz,d);    vertices[4].y = y + Y_ROT(sz2,sz,d);    vertices[4].color_rgba = color;
         vertices[5].x = x + X_ROT(-sz2,sz, d);  vertices[5].y = y + Y_ROT(-sz2,sz, d);  vertices[5].color_rgba = color;
         vertices[6].x = x + X_ROT(-sz,sz2,d);   vertices[6].y = y + Y_ROT(-sz,sz2,d);   vertices[6].color_rgba = color;
         vertices[7].x = x + X_ROT(-sz,-sz2,d);  vertices[7].y = y + Y_ROT(-sz,-sz2,d);  vertices[7].color_rgba = color;
         *triangles_ptr++ = vertices[0];   //Triangle 1 / 6
         *triangles_ptr++ = vertices[1];  
         *triangles_ptr++ = vertices[2];  
         *triangles_ptr++ = vertices[0];   //Triangle 2 / 6
         *triangles_ptr++ = vertices[2];  
         *triangles_ptr++ = vertices[3];  
         *triangles_ptr++ = vertices[0];   //Triangle 3 / 6
         *triangles_ptr++ = vertices[3];  
         *triangles_ptr++ = vertices[4];  
         *triangles_ptr++ = vertices[0];   //Triangle 4 / 6
         *triangles_ptr++ = vertices[4];  
         *triangles_ptr++ = vertices[5];  
         *triangles_ptr++ = vertices[0];   //Triangle 5 / 6
         *triangles_ptr++ = vertices[5];  
         *triangles_ptr++ = vertices[6];  
         *triangles_ptr++ = vertices[0];   //Triangle 6 / 6
         *triangles_ptr++ = vertices[6];  
         *triangles_ptr++ = vertices[7];  

         /* Lines: */
         if (settings.draw_outlines == DRAW_OUTLINES_ALL) {
            vertices[0].color_rgba = core_color;
            vertices[1].color_rgba = core_color;
            vertices[2].color_rgba = core_color;
            vertices[3].color_rgba = core_color;
            vertices[4].color_rgba = core_color;
            vertices[5].color_rgba = core_color;
            vertices[6].color_rgba = core_color;
            vertices[7].color_rgba = core_color;
            *lines_ptr++ = vertices[0];
            *lines_ptr++ = vertices[1];
            *lines_ptr++ = vertices[1];
            *lines_ptr++ = vertices[2];
            *lines_ptr++ = vertices[2];
            *lines_ptr++ = vertices[3];
            *lines_ptr++ = vertices[3];
            *lines_ptr++ = vertices[4];
            *lines_ptr++ = vertices[4];
            *lines_ptr++ = vertices[5];
            *lines_ptr++ = vertices[5];
            *lines_ptr++ = vertices[6];
            *lines_ptr++ = vertices[6];
            *lines_ptr++ = vertices[7];
            *lines_ptr++ = vertices[7];
            *lines_ptr++ = vertices[0];
         }
         break;
      case 5:
         /* CASE 5 - BIG TRIANGLE SHAPE */
         sz = size * (2.0f/3.0f);
         sz2 = size * 0.2f;
         shiftx = -SIN_LOOKUP(d) * sz2;   shifty = COS_LOOKUP(d) * sz2;
         color = (150 << 24) | base_color;
         vertices[0].x = x + X_ROT(-sz, -sz, d) + shiftx; vertices[0].y = y + Y_ROT(-sz, -sz, d) + shifty; 
         vertices[0].color_rgba = color;
         vertices[1].x = x + X_ROT(sz, -sz, d) + shiftx;  vertices[1].y = y + Y_ROT(sz, -sz, d) + shifty;  
         vertices[1].color_rgba = color;
         color = (150 << 24) | (SHAPE_BASE_COLOR_B << 16) | (SHAPE_BASE_COLOR_G << 8) | SHAPE_BASE_COLOR_R;
         vertices[2].x = x + X_ROT(0, sz, d) + shiftx;  vertices[2].y = y + Y_ROT(0, sz, d) + shifty;  
         vertices[2].color_rgba = color;
         *triangles_ptr++ = vertices[0];   // Triangle 1 / 1
         *triangles_ptr++ = vertices[1];
         *triangles_ptr++ = vertices[2];
         /* Lines: */
         if (settings.draw_outlines == DRAW_OUTLINES_ALL) {
            vertices[0].color_rgba = core_color;
            vertices[1].color_rgba = core_color;
            vertices[2].color_rgba = core_color;
            *lines_ptr++ = vertices[0];
            *lines_ptr++ = vertices[1];
            *lines_ptr++ = vertices[1];
            *lines_ptr++ = vertices[2];
            *lines_ptr++ = vertices[2];
            *lines_ptr++ = vertices[0];
         }
         break;
      case 6:
         /* CASE 6: HEXAGON */
         sz = size * 0.5f;
         d = (cnt * 13) & 1023;
         color = (210 << 24) | base_color;
         vertices[0].x = x + X_ROT(-sz, -sz, d); vertices[0].y = y + Y_ROT(-sz, -sz, d); vertices[0].color_rgba = color;
         vertices[1].x = x + X_ROT(0, -sz, d);   vertices[1].y = y + Y_ROT(0, -sz, d);   vertices[1].color_rgba = color;
         vertices[2].x = x + X_ROT(sz, 0, d);    vertices[2].y = y + Y_ROT(sz, 0, d);    vertices[2].color_rgba = color;
         color = (150 << 24) | (SHAPE_BASE_COLOR_B << 16) | (SHAPE_BASE_COLOR_G << 8) | SHAPE_BASE_COLOR_R;
         vertices[3].x = x + X_ROT(sz, sz, d);   vertices[3].y = y + Y_ROT(sz, sz, d);   vertices[3].color_rgba = color;
         vertices[4].x = x + X_ROT(0, sz, d);    vertices[4].y = y + Y_ROT(0, sz, d);    vertices[4].color_rgba = color;
         vertices[5].x = x + X_ROT(-sz, 0, d);   vertices[5].y = y + Y_ROT(-sz, 0, d);   vertices[5].color_rgba = color;
         *triangles_ptr++ = vertices[0];  //Triangle 1 / 4
         *triangles_ptr++ = vertices[1];   
         *triangles_ptr++ = vertices[2];   
         *triangles_ptr++ = vertices[0];  //Triangle 2 / 4
         *triangles_ptr++ = vertices[2];   
         *triangles_ptr++ = vertices[3];   
         *triangles_ptr++ = vertices[0];  //Triangle 3 / 4
         *triangles_ptr++ = vertices[3];   
         *triangles_ptr++ = vertices[4];   
         *triangles_ptr++ = vertices[0];  //Triangle 4 / 4 
         *triangles_ptr++ = vertices[4];   
         *triangles_ptr++ = vertices[5];   

         /* Lines: */
         if (settings.draw_outlines == DRAW_OUTLINES_ALL) {
            vertices[0].color_rgba = core_color;
            vertices[1].color_rgba = core_color;
            vertices[2].color_rgba = core_color;
            vertices[3].color_rgba = core_color;
            vertices[4].color_rgba = core_color;
            vertices[5].color_rgba = core_color;
            *lines_ptr++ = vertices[0];
            *lines_ptr++ = vertices[1];
            *lines_ptr++ = vertices[1];
            *lines_ptr++ = vertices[2];
            *lines_ptr++ = vertices[2];
            *lines_ptr++ = vertices[3];
            *lines_ptr++ = vertices[3];
            *lines_ptr++ = vertices[4];
            *lines_ptr++ = vertices[4];
            *lines_ptr++ = vertices[5];
            *lines_ptr++ = vertices[5];
            *lines_ptr++ = vertices[0];
         }
      break;
   }

   // Draw the core of the shape:
   vertices[0].x = x - SHAPE_POINT_SIZE; vertices[0].y = y - SHAPE_POINT_SIZE; vertices[0].color_rgba = core_color;
   vertices[1].x = x - SHAPE_POINT_SIZE; vertices[1].y = y + SHAPE_POINT_SIZE; vertices[1].color_rgba = core_color;
   vertices[2].x = x + SHAPE_POINT_SIZE; vertices[2].y = y + SHAPE_POINT_SIZE; vertices[2].color_rgba = core_color;
   vertices[3].x = x + SHAPE_POINT_SIZE; vertices[3].y = y - SHAPE_POINT_SIZE; vertices[3].color_rgba = core_color;
   *triangles_ptr++ = vertices[0];   //Triangle 1
   *triangles_ptr++ = vertices[1];
   *triangles_ptr++ = vertices[2];
   *triangles_ptr++ = vertices[0];   //Triangle 2
   *triangles_ptr++ = vertices[2];
   *triangles_ptr++ = vertices[3];
}
#endif //FIXEDMATH

static int ikaClr[2][3][3] = {
   {{230, 230, 255}, {100, 100, 200}, {50, 50, 150}},
   {{0, 0, 0}, {200, 0, 0}, {100, 0, 0}},
};


#ifdef FIXEDMATH
//senquack - my old fixed-point code below needs updating after major optimization overhaul
void drawShapeIka (GLfixed x, GLfixed y, GLfixed size, int d, int cnt, int type, int c)
{
   printf("ERROR: Call to currently-broken drawShapeIka(). Needs code-merge with newer float GLES code.\n");
   return;
   GLfixed sz, sz2, sz3;

   glPushMatrix ();
//  if (shouldrotate) glPushMatrix();
   glTranslatex (x, y, 0);
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
   r = ikaClr[c][0][0]; g = ikaClr[c][0][1]; b = ikaClr[c][0][2];
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
   *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 96;
   *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 96;
   *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 96;
   *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 96;
   *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 96;
   *shapeptcolptr++ = r; *shapeptcolptr++ = g; *shapeptcolptr++ = b; *shapeptcolptr++ = 96;
   *shapeptvertptr++ = x - SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = y - SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = x - SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = y + SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = x + SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = y - SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = x + SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = y + SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = x + SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = y - SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = x - SHAPE_POINT_SIZE_X;
   *shapeptvertptr++ = y + SHAPE_POINT_SIZE_X;

//  glColor4i(ikaClr[c][0][0], ikaClr[c][0][1], ikaClr[c][0][2], 255);
//    glEnable(GL_BLEND);

   switch (type) {
   case 0:
//    sz = size/2; sz2 = sz/3; sz3 = size*2/3;
      sz = size >> 1;
      sz2 = FMUL (sz, 21845);
      sz3 = FMUL (size, 43691);    // 43690 = 2/3 in fixed point
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

      shapevertices[0] = -sz;
      shapevertices[1] = -sz3;
      shapevertices[2] = sz;
      shapevertices[3] = -sz3;
      shapevertices[4] = sz2;
      shapevertices[5] = sz3;
      shapevertices[6] = -sz2;
      shapevertices[7] = sz3;
      glDrawArrays (GL_TRIANGLE_FAN, 0, 4);

//    glEnd();
      break;
   case 1:
//    sz = size/2;
      sz = size >> 1;
      sz2 = size >> 2;         // added this because of simplicity later on
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
      shapevertices[0] = -sz2;
      shapevertices[1] = -sz;
      shapevertices[2] = sz2;
      shapevertices[3] = -sz;
      shapevertices[4] = sz;
      shapevertices[5] = -sz2;
      shapevertices[6] = sz;
      shapevertices[7] = sz2;
      shapevertices[8] = sz2;
      shapevertices[9] = sz;
      shapevertices[10] = -sz2;
      shapevertices[11] = sz;
      shapevertices[12] = -sz;
      shapevertices[13] = sz2;
      shapevertices[14] = -sz;
      shapevertices[15] = -sz2;
      glDrawArrays (GL_TRIANGLE_FAN, 0, 8);
//    glEnd();
      break;
   }
   glPopMatrix ();
//  if (shouldrotate) glPopMatrix();
}
#else   
/* senquack - HIGHLY OPTIMIZED POST-WIZ-PORT BATCH-DRAWN IKA-SHAPE CODE BEGINS HERE: */
void drawShapeIka (GLfloat x, GLfloat y, GLfloat size, int d, int cnt, int type, int c)
{
   GLfloat sz, sz2, sz3;
   gl_vertex vertices[8];
   uint32_t color = 0;

   switch (type) {
      case 0:  /* WEDGE SHAPE */
         sz = size * 0.5f;
         sz2 = size * (1.0f / 3.0f);
         sz3 = size * (2.0f / 3.0f);

         color = (250 << 24) | (ikaClr[c][1][2] << 16) | (ikaClr[c][1][1] << 8) | ikaClr[c][1][0];
         vertices[0].x = x + X_ROT(-sz, -sz3, d); vertices[0].y = y + Y_ROT(-sz, -sz3, d); vertices[0].color_rgba = color;
         vertices[1].x = x + X_ROT(sz, -sz3, d);  vertices[1].y = y + Y_ROT(sz, -sz3, d);  vertices[1].color_rgba = color;
         color = (250 << 24) | (ikaClr[c][2][2] << 16) | (ikaClr[c][2][1] << 8) | ikaClr[c][2][0];
         vertices[2].x = x + X_ROT(sz2, sz3, d);  vertices[2].y = y + Y_ROT(sz2, sz3, d);  vertices[2].color_rgba = color;
         vertices[3].x = x + X_ROT(-sz2, sz3, d); vertices[3].y = y + Y_ROT(-sz2, sz3, d); vertices[3].color_rgba = color;
         *triangles_ptr++ = vertices[0];   //Triangle 1 / 2
         *triangles_ptr++ = vertices[1];
         *triangles_ptr++ = vertices[2];
         *triangles_ptr++ = vertices[0];   //Triangle 2 / 2
         *triangles_ptr++ = vertices[2];
         *triangles_ptr++ = vertices[3];

         /* Lines: */
         color = (255 << 24) | (ikaClr[c][0][2] << 16) | (ikaClr[c][0][1] << 8) | ikaClr[c][0][0];
         if (settings.draw_outlines != DRAW_OUTLINES_NONE) {
            vertices[0].color_rgba = color; 
            vertices[1].color_rgba = color;
            vertices[2].color_rgba = color;
            vertices[3].color_rgba = color;
            *lines_ptr++ = vertices[0];
            *lines_ptr++ = vertices[1];
            *lines_ptr++ = vertices[1];
            *lines_ptr++ = vertices[2];
            *lines_ptr++ = vertices[2];
            *lines_ptr++ = vertices[3];
            *lines_ptr++ = vertices[3];
            *lines_ptr++ = vertices[0];
         }
         break;
      case 1:  /* CIRCLE SHAPE */
         sz = size * 0.5f;
         sz2 = size * 0.25f;   

         color = (250 << 24) | (ikaClr[c][1][2] << 16) | (ikaClr[c][1][1] << 8) | ikaClr[c][1][0];
         vertices[0].x = x + X_ROT(-sz2, -sz, d); vertices[0].y = y + Y_ROT(-sz2, -sz, d); 
         vertices[0].color_rgba = color;
         vertices[1].x = x + X_ROT(sz2, -sz, d);  vertices[1].y = y + Y_ROT(sz2, -sz, d);  
         vertices[1].color_rgba = color;
         vertices[2].x = x + X_ROT(sz, -sz2, d);  vertices[2].y = y + Y_ROT(sz, -sz2, d);  
         vertices[2].color_rgba = color;
         vertices[3].x = x + X_ROT(sz, sz2, d);   vertices[3].y = y + Y_ROT(sz, sz2, d);   
         vertices[3].color_rgba = color;
         color = (250 << 24) | (ikaClr[c][2][2] << 16) | (ikaClr[c][2][1] << 8) | ikaClr[c][2][0];
         vertices[4].x = x + X_ROT(sz2, sz, d);   vertices[4].y = y + Y_ROT(sz2, sz, d);   
         vertices[4].color_rgba = color;
         vertices[5].x = x + X_ROT(-sz2, sz, d);  vertices[5].y = y + Y_ROT(-sz2, sz, d);  
         vertices[5].color_rgba = color;
         vertices[6].x = x + X_ROT(-sz, sz2, d);  vertices[6].y = y + Y_ROT(-sz, sz2, d);  
         vertices[6].color_rgba = color;
         vertices[7].x = x + X_ROT(-sz, -sz2, d); vertices[7].y = y + Y_ROT(-sz, -sz2, d); 
         vertices[7].color_rgba = color;
         *triangles_ptr++ = vertices[0];   //Triangle 1 / 2
         *triangles_ptr++ = vertices[1];
         *triangles_ptr++ = vertices[2];
         *triangles_ptr++ = vertices[0];   //Triangle 1 / 2
         *triangles_ptr++ = vertices[2];
         *triangles_ptr++ = vertices[3];
         *triangles_ptr++ = vertices[0];   //Triangle 1 / 2
         *triangles_ptr++ = vertices[3];
         *triangles_ptr++ = vertices[4];
         *triangles_ptr++ = vertices[0];   //Triangle 1 / 2
         *triangles_ptr++ = vertices[4];
         *triangles_ptr++ = vertices[5];
         *triangles_ptr++ = vertices[0];   //Triangle 1 / 2
         *triangles_ptr++ = vertices[5];
         *triangles_ptr++ = vertices[6];
         *triangles_ptr++ = vertices[0];   //Triangle 1 / 2
         *triangles_ptr++ = vertices[6];
         *triangles_ptr++ = vertices[7];

         /* Lines: */
         color = (255 << 24) | (ikaClr[c][0][2] << 16) | (ikaClr[c][0][1] << 8) | ikaClr[c][0][0];
         if (settings.draw_outlines != DRAW_OUTLINES_NONE) {
            vertices[0].color_rgba = color; 
            vertices[1].color_rgba = color;
            vertices[2].color_rgba = color;
            vertices[3].color_rgba = color;
            vertices[4].color_rgba = color; 
            vertices[5].color_rgba = color;
            vertices[6].color_rgba = color;
            vertices[7].color_rgba = color;
            *lines_ptr++ = vertices[0];
            *lines_ptr++ = vertices[1];
            *lines_ptr++ = vertices[1];
            *lines_ptr++ = vertices[2];
            *lines_ptr++ = vertices[2];
            *lines_ptr++ = vertices[3];
            *lines_ptr++ = vertices[3];
            *lines_ptr++ = vertices[4];
            *lines_ptr++ = vertices[4];
            *lines_ptr++ = vertices[5];
            *lines_ptr++ = vertices[5];
            *lines_ptr++ = vertices[6];
            *lines_ptr++ = vertices[6];
            *lines_ptr++ = vertices[7];
            *lines_ptr++ = vertices[7];
            *lines_ptr++ = vertices[0];
         }
   }

   // Draw the core of the shape:
   vertices[0].x = x - SHAPE_POINT_SIZE; vertices[0].y = y - SHAPE_POINT_SIZE; vertices[0].color_rgba = color;
   vertices[1].x = x - SHAPE_POINT_SIZE; vertices[1].y = y + SHAPE_POINT_SIZE; vertices[1].color_rgba = color;
   vertices[2].x = x + SHAPE_POINT_SIZE; vertices[2].y = y + SHAPE_POINT_SIZE; vertices[2].color_rgba = color;
   vertices[3].x = x + SHAPE_POINT_SIZE; vertices[3].y = y - SHAPE_POINT_SIZE; vertices[3].color_rgba = color;
   *triangles_ptr++ = vertices[0];   //Triangle 1
   *triangles_ptr++ = vertices[1];
   *triangles_ptr++ = vertices[2];
   *triangles_ptr++ = vertices[0];   //Triangle 2
   *triangles_ptr++ = vertices[2];
   *triangles_ptr++ = vertices[3];
}
#endif //FIXEDMATH

#define SHOT_WIDTH 0.1
#define SHOT_HEIGHT 0.2

static int shtClr[3][3][3] = {
   {{200, 200, 225}, {50, 50, 200}, {200, 200, 225}},
   {{100, 0, 0}, {100, 0, 0}, {200, 0, 0}},
   {{100, 200, 100}, {50, 100, 50}, {100, 200, 100}},
};

//senquack - converted to batch-drawn GLES by replacing glRotate w/ pre-rotated coordinates
//          NOTE: sometimes at angles like 40deg or -40deg or so, shots can overshoot the
//             boss's shape, but the original game did this too. TODO: maybe correct this.
void drawShot (GLfloat x, GLfloat y, GLfloat d, int c, float width, float height)
{
   uint32_t color;
   gl_vertex vertices[4];
   int deg = d * (1024.0f/ 360.0f); // Convert degrees back to index suitable for game's lookup table
   
   // First, the triangles:
   color = (240 << 24) | (shtClr[c][1][2] << 16) | (shtClr[c][1][1] << 8) | shtClr[c][1][0];
   vertices[0].x = x + X_ROT(-width,-height,deg); vertices[0].y = y + Y_ROT(-width,-height,deg); 
   vertices[0].color_rgba = color;
   vertices[1].x = x + X_ROT(width,-height,deg);  vertices[1].y = y + Y_ROT(width,-height,deg); 
   vertices[1].color_rgba = color;
   color = (240 << 24) | (shtClr[c][2][2] << 16) | (shtClr[c][2][1] << 8) | shtClr[c][2][0];
   vertices[2].x = x + X_ROT(width,height,deg);   vertices[2].y = y + Y_ROT(width,height,deg); 
   vertices[2].color_rgba = color;
   vertices[3].x = x + X_ROT(-width,height,deg);  vertices[3].y = y + Y_ROT(-width,height,deg); 
   vertices[3].color_rgba = color;
   *triangles_ptr++ = vertices[0];   //Triangle 1 / 2
   *triangles_ptr++ = vertices[1];   
   *triangles_ptr++ = vertices[2];   
   *triangles_ptr++ = vertices[2];   //Triangle 2 / 2
   *triangles_ptr++ = vertices[3];  
   *triangles_ptr++ = vertices[0];  

   // Then, the outline:
   if (settings.draw_outlines != DRAW_OUTLINES_NONE) {
      color = (240 << 24) | (shtClr[c][0][2] << 16) | (shtClr[c][0][1] << 8) | shtClr[c][0][0];
      vertices[0].color_rgba = color;
      vertices[1].color_rgba = color;
      vertices[2].color_rgba = color;
      vertices[3].color_rgba = color;
      *lines_ptr++ = vertices[0];
      *lines_ptr++ = vertices[1];
      *lines_ptr++ = vertices[1];
      *lines_ptr++ = vertices[2];
      *lines_ptr++ = vertices[2];
      *lines_ptr++ = vertices[3];
      *lines_ptr++ = vertices[3];
      *lines_ptr++ = vertices[0];
   }
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
#ifdef FIXEDMATH
//senquack - converted to fixed point
void startDrawBoards ()
{
   //senquack TODO: do we really need to call all this each time a frame is drawn?
   glMatrixMode (GL_PROJECTION);
   //senquack - dunno why we do this
   glPushMatrix();
   glLoadIdentity ();
   glOrthox (0, INT2FNUM (640), INT2FNUM (480), 0, INT2FNUM (-1),
         INT2FNUM (1));
   glMatrixMode (GL_MODELVIEW);
   glPushMatrix ();
   glLoadIdentity ();
}
#else
void startDrawBoards() {
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrthof(0, 640, 480, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

//  static float transx = 0.0f, transy = 0.0f;
//  static float scalex = 1.0f, scaley = 1.0f;
////  static float step = .01f;
//  static float stept = 5.0f;
//  static float steps = .01f;
//  if (control_state[CY]) transx -= stept;
//  if (control_state[CB]) transx += stept;
//  if (control_state[CX]) transy -= stept;
//  if (control_state[CA]) transy += stept;
//
//  if (control_state[CANALOGUP]) scalex -= steps;
//  if (control_state[CANALOGDOWN]) scalex += steps;
//  if (control_state[CANALOGLEFT]) scaley -= steps;
//  if (control_state[CANALOGRIGHT]) scaley += steps;
//
//  printf("transx: %f  transy: %f   scalex: %f  scaley: %f\n", transx, transy, scalex, scaley);

   if (settings.rotated == SCREEN_ROTATED_LEFT) {
//      glRotatef (-90.0f, 0.0f, 0.0f, 1.0f);
      // Now, trying to get translate for left-rotation:
//      glScalef(1.5f,1.34f,1.0f); // As close to perfect as I can manage
//      glScalef(1.5f,1.34f,1.0f); // As close to perfect as I can manage
////      glTranslatef(-720.0f, 0.0f, 0.0f); // here, the ship is centered nicely left-to-right but gameover is a bit to right
//
//      // here, less negative means shifted further to right:
//      glTranslatef(-660.0, 0.0f, 0.0f); // here, the ship is centered nicely left-to-right but gameover is a bit to right
////      glScalef(.1f,.1f,1.0f); // As close to perfect as I can manage
////      glScalef(1.5f,1.34f,1.0f); // As close to perfect as I can manage
//      glScalef(1.30f,1.34f,1.0f); // As close to perfect as I can manage

      //OK good values: transx: -690, transy: -5, scalex: 1.41, scaley: 1.36
      glRotatef (-90.0f, 0.0f, 0.0f, 1.0f);
      glTranslatef(-690.0, -5.0f, 0.0f); 
      glScalef(1.41f, 1.36f, 1.0f);
   } else if (settings.rotated == SCREEN_ROTATED_RIGHT) {
//   glTranslatef(-90.0f, -550.0f, 0.0f); // Move horizontally the screen Height  // TOO FAR DOWN AND RIGHT
   //THE MORE NEGATIVE THE Y VALUE HERE, THE HIGHER THE ROTATED-RIGHT IMAGE IS, -550.0f is about centered
   // THE MORE NEGATIVE THE X VALUE HERE, THE MORE-TO-THE-LEFT THE ROTATED-RIGHT IMAGE IS
//   glTranslatef(0.0f, -550.0f, 0.0f); // Move horizontally the screen Height  // TOO FAR DOWN AND RIGHT

   //OK, -650 for y is damned close,
   // OK, now "GAME OVER" is centered perfectly, but title screen is too far down and to right!?!:
//   glTranslatef(-240.0f, -640.0f, 0.0f); // Move horizontally the screen Height  // TOO FAR DOWN AND RIGHT
//      glScalef(1.5f,1.34f,1.0f); // As close to perfect as I can manage

      //OK good values: transx: -210 transy: -645, scalex: 1.41, scaley: 1.36
      glRotatef (90.0f, 0.0f, 0.0f, 1.0f);
      glTranslatef(-210.0f, -645.0f, 0.0f); 
      glScalef(1.41f,1.36f,1.0f); 
   }

}
#endif //FIXEDMATH

//senquack
//void endDrawBoards() {
//  glPopMatrix();
//  screenResized();
//}
void
endDrawBoards ()
{
   glBlendFunc (GL_SRC_ALPHA, GL_DST_COLOR); // best choice for letter rendering overall
   finishDrawBatch();     
   glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);    // GL docs say is best for transparency

   glMatrixMode(GL_MODELVIEW);   // senquack - just to be sure we're in modelview
   glPopMatrix ();

//senquack TODO: make sure we really need to be calling this all the time like in the original code:
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
#ifdef FIXEDMATH
static void drawBoard (int x, int y, int width, int height)
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
#else
static void drawBoard (int x, int y, int width, int height)
{
   gl_vertex vertices[4];
   uint32_t color = 0xFF000000;

   vertices[0].x = x;            vertices[0].y = y;            vertices[0].color_rgba = color;
   vertices[1].x = x + width;    vertices[1].y = y;            vertices[1].color_rgba = color;
   vertices[2].x = x + width;    vertices[2].y = y + height;   vertices[2].color_rgba = color;
   vertices[3].x = x;            vertices[3].y = y + height;   vertices[3].color_rgba = color;

   glVertexPointer (2, GL_FLOAT, sizeof(gl_vertex), &vertices[0].x);
   glColorPointer (4, GL_UNSIGNED_BYTE, sizeof(gl_vertex), &vertices[0].r);
   glDrawArrays (GL_TRIANGLE_FAN, 0, 4);
}
#endif //FIXEDMATH

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
      prepareDrawBatch();
// if (screenRotated) {
   if (settings.rotated) {
      drawScore();
      drawRPanel_rotated ();
      glDisable (GL_BLEND);
      // We only need very slender sideboards when rotated:
      drawBoard (140, 0, 19, 480);
      drawBoard (480, 0, 20, 480);
      glEnable (GL_BLEND);
   } else {
      drawScore ();
      drawRPanel ();
      glDisable (GL_BLEND);
      drawBoard (0, 0, 160, 480);
      drawBoard (480, 0, 160, 480);
      glEnable (GL_BLEND);
   }
}

//senquack -  converted to GLES fixed/float point
//senquack TODO: make sure no more than is necessary is drawn when screen is rotated
#ifdef FIXEDMATH
void drawTitleBoard ()
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
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

   memset (colors, 255, 5 * 4);
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
      colors[16] = colors[17] = colors[18] = 200;
//      colors[20] = colors[21] = colors[22] = 200;
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
   memset (colors, 255, 5*4);

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
#else
void drawTitleBoard ()
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

//   GLubyte colors[5 * 4];
//   GLfloat vertices[5 * 2];

   //senquack - line loops appear to be broken under GCW's etnaviv driver, turned into line strips:
   GLubyte colors[6 * 4];
   GLfloat vertices[6 * 2];
   GLfloat texvertices[4 * 2];

   glVertexPointer (2, GL_FLOAT, 0, vertices);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, colors);

   glEnable (GL_TEXTURE_2D);
   glBindTexture (GL_TEXTURE_2D, titleTexture);
   glEnable (GL_BLEND);
// glBlendFunc(GL_ONE, GL_SRC_COLOR);
   glBlendFunc (GL_ONE, GL_SRC_ALPHA);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

   memset (colors, 255, 5 * 4);
// vertices[0] = INT2FNUM(350);  vertices[1] = INT2FNUM(78);
// vertices[2] = INT2FNUM(470);  vertices[3] = INT2FNUM(78);
// vertices[4] = INT2FNUM(470);  vertices[5] = INT2FNUM(114);
// vertices[6] = INT2FNUM(350);  vertices[7] = INT2FNUM(114);
   //senquack TODO - make sure I got these right in the Wiz code:
   vertices[0] = 350;
   vertices[1] = 114;
   vertices[2] = 470;
   vertices[3] = 114;
   vertices[4] = 350;
   vertices[5] = 78;
   vertices[6] = 470;
   vertices[7] = 78;
// texvertices[0] = 0;              texvertices[1] = 0;
// texvertices[2] = INT2FNUM(1);    texvertices[3] = 0;
// texvertices[4] = INT2FNUM(1);    texvertices[5] = INT2FNUM(1);
// texvertices[6] = 0;              texvertices[7] = INT2FNUM(1);
   texvertices[0] = 0;
   texvertices[1] = 1;
   texvertices[2] = 1;
   texvertices[3] = 1;
   texvertices[4] = 0;
   texvertices[5] = 0;
   texvertices[6] = 1;
   texvertices[7] = 0;

////  glEnableClientState(GL_VERTEX_ARRAY);
////  glEnableClientState(GL_COLOR_ARRAY);
   glEnableClientState (GL_TEXTURE_COORD_ARRAY);
   glTexCoordPointer (2, GL_FLOAT, 0, texvertices);
// glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
   glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
   glDisableClientState (GL_TEXTURE_COORD_ARRAY);
   glDisable (GL_TEXTURE_2D);

   glDisable (GL_BLEND);
//   glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE);   // original rr code had this


   colors[0] = colors[1] = colors[2] =
      colors[4] = colors[5] = colors[6] =
      colors[8] = colors[9] = colors[10] =
      colors[12] = colors[13] = colors[14] =
      colors[16] = colors[17] = colors[18] = 200;
//      colors[20] = colors[21] = colors[22] = 200;
   colors[3] = colors[7] = colors[11] = colors[15] = colors[19] = 255;

   vertices[0] = 350;
   vertices[1] = 30;
   vertices[2] = 400;
   vertices[3] = 30;
   vertices[4] = 380;
   vertices[5] = 56;
   vertices[6] = 380;
   vertices[7] = 80;
   vertices[8] = 350;
   vertices[9] = 80;
   glDrawArrays (GL_TRIANGLE_FAN, 0, 5);

//  glBegin(GL_TRIANGLE_FAN);
//  glVertex3f(404, 80, 0);
//  glVertex3f(404, 8, 0);
//  glVertex3f(440, 8, 0);
//  glVertex3f(440, 44, 0);
//  glVertex3f(465, 80, 0);
//  glEnd();

   vertices[0] = 404;
   vertices[1] = 80;
   vertices[2] = 404;
   vertices[3] = 8;
   vertices[4] = 440;
   vertices[5] = 8;
   vertices[6] = 440;
   vertices[7] = 44;
   vertices[8] = 465;
   vertices[9] = 80;
   glDrawArrays (GL_TRIANGLE_FAN, 0, 5);

//  glColor4i(255, 255, 255, 255);
//   memset (colors, 255, 5*4);
   memset (colors, 255, 6*4);

   // I disabled the line outlines, because it added little artistically and caused gaps in pixels
   //    in different scalings, and got sick of futzing with it.
   
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(350, 30, 0);
////  glVertex3f(400, 30, 0);
////  glVertex3f(380, 56, 0);
////  glVertex3f(380, 80, 0);
////  glVertex3f(350, 80, 0);
////  glEnd();
//
//   vertices[0] = 350;
//   vertices[1] = 30;
//   vertices[2] = 400;
//   vertices[3] = 30;
//   vertices[4] = 380;
////   vertices[5] = 56;
//   vertices[5] = 54;
//   vertices[6] = 380;
//   vertices[7] = 80;
//   vertices[8] = 350;
//   vertices[9] = 80;
//   vertices[10] = 350;
//   vertices[11] = 30;
//   //senquack - line loops appear to be broken under GCW's etnaviv driver, turned into line strips:
////   glDrawArrays (GL_LINE_LOOP, 0, 5);
//   glDrawArrays (GL_LINE_STRIP, 0, 6);
//
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(404, 80, 0);
////  glVertex3f(404, 8, 0);
////  glVertex3f(440, 8, 0);
////  glVertex3f(440, 44, 0);
////  glVertex3f(465, 80, 0);
////  glEnd();
//
//   //adjusting a bit to correct gaps in pixels
//   vertices[0] = 404;
//   vertices[1] = 80;
//   vertices[2] = 404;
//   vertices[3] = 8;
////   vertices[4] = 440;
////   vertices[5] = 8;
//   vertices[4] = 439;
//   vertices[5] = 8;
////   vertices[6] = 440;
////   vertices[7] = 44;
//   vertices[6] = 439;
//   vertices[7] = 44;
////   vertices[8] = 465;
////   vertices[9] = 80;
//   vertices[8] = 463;
//   vertices[9] = 80;
//   vertices[10] = 404;
//   vertices[11] = 80;
//   //senquack - line loops appear to be broken under GCW's etnaviv driver, turned into line strips:
////   glDrawArrays (GL_LINE_LOOP, 0, 5);
//   glDrawArrays (GL_LINE_STRIP, 0, 6);

   glEnable (GL_BLEND);
}
#endif //FIXEDMATH

// Draw the numbers.
int drawNum (int n, int x, int y, int s, int r, int g, int b)
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

////senquack - made a version to draw a number right-justified, horizontally)
//int drawNumHoriz (int n, int x, int y, int s, int r, int g, int b)
//{
//   for (;;) {
//      drawLetter (n % 10, x, y, s, 0, r, g, b);
//      x -= s * 1.7f;
//      n /= 10;
//      if (n <= 0)
//         break;
//   }
//   return x;
//}
//senquack - made a version to draw a number left-justified, horizontally)
int drawNumHoriz (int n, int x, int y, int s, int r, int g, int b)
{
//   for (;;) {
//      drawLetter (n % 10, x, y, s, 0, r, g, b);
//      x -= s * 1.7f;
//      n /= 10;
//      if (n <= 0)
//         break;
//   }
//   return x;
   int d, nd, drawn = 0;
   for (d = 100000000; d > 0; d /= 10) {
      nd = (int) (n / d);
      if (nd > 0 || drawn) {
         n -= d * nd;
         drawLetter (nd % 10, x, y, s, 0, r, g, b);
         x += s * 1.7f;
         drawn = 1;
      }
   }
   if (!drawn) {
      drawLetter (0, x, y, s, 0, r, g, b);
      x += s * 1.7f;
   }
   return x;
}

int drawNumRight (int n, int x, int y, int s, int r, int g, int b)
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

int drawNumCenter (int n, int x, int y, int s, int r, int g, int b)
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

int drawTimeCenter (int n, int x, int y, int s, int r, int g, int b)
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


//senquack - adding support for rotated screen:
//#define JOYSTICK_AXIS 16384
//int getPadState() {
//  int x = 0, y = 0;
//  int pad = 0;
//  if ( stick != NULL ) {
//    x = SDL_JoystickGetAxis(stick, 0);
//    y = SDL_JoystickGetAxis(stick, 1);
//  }
//  if ( keys[SDLK_RIGHT] == SDL_PRESSED || keys[SDLK_KP6] == SDL_PRESSED || x > JOYSTICK_AXIS ) {
//    pad |= PAD_RIGHT;
//  }
//  if ( keys[SDLK_LEFT] == SDL_PRESSED || keys[SDLK_KP4] == SDL_PRESSED || x < -JOYSTICK_AXIS ) {
//    pad |= PAD_LEFT;
//  }
//  if ( keys[SDLK_DOWN] == SDL_PRESSED || keys[SDLK_KP2] == SDL_PRESSED || y > JOYSTICK_AXIS ) {
//    pad |= PAD_DOWN;
//  }
//  if ( keys[SDLK_UP] == SDL_PRESSED ||  keys[SDLK_KP8] == SDL_PRESSED || y < -JOYSTICK_AXIS ) {
//    pad |= PAD_UP;
//  }
//  return pad;
//}
int getPadState ()
{
   int pad = 0;
#if GCW
//   if (settings.rotated == SCREEN_HORIZ) {
//      if (control_state[CUP]        || control_state[CANALOGUP])       pad |= PAD_UP;
//      if (control_state[CDOWN]      || control_state[CANALOGDOWN])     pad |= PAD_DOWN;
//      if (control_state[CLEFT]      || control_state[CANALOGLEFT])     pad |= PAD_LEFT;
//      if (control_state[CRIGHT]     || control_state[CANALOGRIGHT])    pad |= PAD_RIGHT;
//   } else if (settings.rotated == SCREEN_ROTATED_RIGHT) {
//      if (control_state[CUP]    )  pad |= PAD_LEFT;         //   (unless buttons are reversed)
//      if (control_state[CDOWN]  )  pad |= PAD_RIGHT;
//      if (control_state[CLEFT]  )  pad |= PAD_DOWN;
//      if (control_state[CRIGHT] )  pad |= PAD_UP;
//   } else { // Screen must be rotated left, then
//      if (control_state[CX])       pad |= PAD_UP;
//      if (control_state[CA])       pad |= PAD_DOWN;
//      if (control_state[CB])       pad |= PAD_LEFT;
//      if (control_state[CY])       pad |= PAD_RIGHT;
//   }
   if (settings.rotated == SCREEN_HORIZ) {
      if (settings.map.move == MAP_DPAD) {
         if (control_state[CUP])       pad |= PAD_UP;
         if (control_state[CDOWN])     pad |= PAD_DOWN;
         if (control_state[CLEFT])     pad |= PAD_LEFT;
         if (control_state[CRIGHT])    pad |= PAD_RIGHT;
      } else if (settings.map.move == MAP_ANALOG) {
         if (control_state[CANALOGUP])       pad |= PAD_UP;
         if (control_state[CANALOGDOWN])     pad |= PAD_DOWN;
         if (control_state[CANALOGLEFT])     pad |= PAD_LEFT;
         if (control_state[CANALOGRIGHT])    pad |= PAD_RIGHT;
      } else if (settings.map.move == MAP_ABXY) {
         if (control_state[CY])    pad |= PAD_UP;
         if (control_state[CB])    pad |= PAD_DOWN;
         if (control_state[CX])    pad |= PAD_LEFT;
         if (control_state[CA])    pad |= PAD_RIGHT;
      }
   } else if (settings.rotated == SCREEN_ROTATED_RIGHT) {
      if (settings.map.move == MAP_DPAD) {
         if (control_state[CUP]    )  pad |= PAD_LEFT;  
         if (control_state[CDOWN]  )  pad |= PAD_RIGHT;
         if (control_state[CLEFT]  )  pad |= PAD_DOWN;
         if (control_state[CRIGHT] )  pad |= PAD_UP;
      } else if (settings.map.move == MAP_ABXY) {
         if (control_state[CA])    pad |= PAD_UP;
         if (control_state[CX])    pad |= PAD_DOWN;
         if (control_state[CY])    pad |= PAD_LEFT;
         if (control_state[CB])    pad |= PAD_RIGHT;
      } else if (settings.map.move == MAP_ANALOG) {
         if (control_state[CANALOGUP]    )  pad |= PAD_LEFT;  
         if (control_state[CANALOGDOWN]  )  pad |= PAD_RIGHT;
         if (control_state[CANALOGLEFT]  )  pad |= PAD_DOWN;
         if (control_state[CANALOGRIGHT] )  pad |= PAD_UP;
      }
   } else { // Screen must be rotated left, then
      if (settings.map.move == MAP_DPAD) {
         if (control_state[CUP]    )  pad |= PAD_RIGHT;  
         if (control_state[CDOWN]  )  pad |= PAD_LEFT;
         if (control_state[CLEFT]  )  pad |= PAD_UP;
         if (control_state[CRIGHT] )  pad |= PAD_DOWN;
      } else if (settings.map.move == MAP_ABXY) {
         if (control_state[CA])    pad |= PAD_DOWN;
         if (control_state[CX])    pad |= PAD_UP;
         if (control_state[CY])    pad |= PAD_RIGHT;
         if (control_state[CB])    pad |= PAD_LEFT;
      } else if (settings.map.move == MAP_ANALOG) {
         if (control_state[CANALOGUP]    )  pad |= PAD_RIGHT;  
         if (control_state[CANALOGDOWN]  )  pad |= PAD_LEFT;
         if (control_state[CANALOGLEFT]  )  pad |= PAD_UP;
         if (control_state[CANALOGRIGHT] )  pad |= PAD_DOWN;
      }
   }
#endif //GCW

   return pad;
}

int buttonReversed = 0;
//int getButtonState() {
//  int btn = 0;
//  int btn1 = 0, btn2 = 0, btn3 = 0, btn4 = 0;
//  if ( stick != NULL ) {
//    btn1 = SDL_JoystickGetButton(stick, 0);
//    btn2 = SDL_JoystickGetButton(stick, 1);
//    btn3 = SDL_JoystickGetButton(stick, 2);
//    btn4 = SDL_JoystickGetButton(stick, 3);
//  }
//  if ( keys[SDLK_z] == SDL_PRESSED || btn1 || btn4 ) {
//    if ( !buttonReversed ) {
//      btn |= PAD_BUTTON1;
//    } else {
//      btn |= PAD_BUTTON2;
//    }
//  }
//  if ( keys[SDLK_x] == SDL_PRESSED || btn2 || btn3 ) {
//    if ( !buttonReversed ) {
//      btn |= PAD_BUTTON2;
//    } else {
//      btn |= PAD_BUTTON1;
//    }
//  }
//  return btn;
//}
int getButtonState ()
{
   int btn = 0;
   if (control_state[settings.map.btn1] || control_state[settings.map.btn1_alt]) {
      btn |= PAD_BUTTON1;
   }

   if (control_state[settings.map.btn2] || control_state[settings.map.btn2_alt]) {
      btn |= PAD_BUTTON2;
   }
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
//Mesa's implementation, switched to floats from doubles for speedup
#ifdef FIXEDMATH
void gluLookAt(GLfixed eyex, GLfixed eyey, GLfixed eyez,
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
   //senquack TODO: possible speedup w/ multiplication by inverse:
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
#else
void gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez,
   GLfloat centerx, GLfloat centery, GLfloat centerz,
   GLfloat upx, GLfloat upy, GLfloat upz)
{
   GLfloat m[16];
   GLfloat x[3], y[3], z[3];
   GLfloat mag;

   /* Make rotation matrix */

   /* Z vector */
   z[0] = eyex - centerx;
   z[1] = eyey - centery;
   z[2] = eyez - centerz;
   mag = sqrtf(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
   if (mag) {         /* mpichler, 19950515 */
      z[0] /= mag;
      z[1] /= mag;
      z[2] /= mag;
   }

   /* Y vector */
   y[0] = upx;
   y[1] = upy;
   y[2] = upz;

   /* X vector = Y cross Z */
   x[0] = y[1] * z[2] - y[2] * z[1];
   x[1] = -y[0] * z[2] + y[2] * z[0];
   x[2] = y[0] * z[1] - y[1] * z[0];

   /* Recompute Y = Z cross X */
   y[0] = z[1] * x[2] - z[2] * x[1];
   y[1] = -z[0] * x[2] + z[2] * x[0];
   y[2] = z[0] * x[1] - z[1] * x[0];

   /* mpichler, 19950515 */
   /* cross product gives area of parallelogram, which is < 1.0 for
    * non-perpendicular unit-length vectors; so normalize x, y here
    */

   mag = sqrtf(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
   if (mag) {
      x[0] /= mag;
      x[1] /= mag;
      x[2] /= mag;
   }

   mag = sqrtf(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
   if (mag) {
      y[0] /= mag;
      y[1] /= mag;
      y[2] /= mag;
   }

#define M(row,col)  m[col*4+row]
   M(0, 0) = x[0];
   M(0, 1) = x[1];
   M(0, 2) = x[2];
   M(0, 3) = 0.0;
   M(1, 0) = y[0];
   M(1, 1) = y[1];
   M(1, 2) = y[2];
   M(1, 3) = 0.0;
   M(2, 0) = z[0];
   M(2, 1) = z[1];
   M(2, 2) = z[2];
   M(2, 3) = 0.0;
   M(3, 0) = 0.0;
   M(3, 1) = 0.0;
   M(3, 2) = 0.0;
   M(3, 3) = 1.0;
#undef M
   glMultMatrixf(m);

   /* Translate Eye to Origin */
   glTranslatef(-eyex, -eyey, -eyez);
}
#endif //FIXEDMATH

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
