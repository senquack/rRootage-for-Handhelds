/*
 * $Id: screen.h,v 1.4 2003/04/26 03:24:16 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * Opengl screen functions header file.
 *
 * @version $Revision: 1.4 $
 */
#ifndef SCREEN_H
#define SCREEN_H

// For GL type defines:
#include "GLES/gl.h"
#include "GLES/egl.h"

// For joystick
#include "SDL.h"

//Original rrootage code:
#define PAD_UP 1
#define PAD_DOWN 2
#define PAD_LEFT 4
#define PAD_RIGHT 8
#define PAD_BUTTON1 16
#define PAD_BUTTON2 32


#if defined(WIZ) || defined(GP2X)
//GP2X mappings
#define GP2X_BUTTON_UP              (0)
#define GP2X_BUTTON_DOWN            (4)
#define GP2X_BUTTON_LEFT            (2)
#define GP2X_BUTTON_RIGHT           (6)
#define GP2X_BUTTON_UPLEFT          (1)
#define GP2X_BUTTON_UPRIGHT         (7)
#define GP2X_BUTTON_DOWNLEFT        (3)
#define GP2X_BUTTON_DOWNRIGHT       (5)
#define GP2X_BUTTON_CLICK           (18)
#define GP2X_BUTTON_A               (12)
#define GP2X_BUTTON_B               (13)
#define GP2X_BUTTON_X               (14)
#define GP2X_BUTTON_Y               (15)
#define GP2X_BUTTON_L               (10)
#define GP2X_BUTTON_R               (11)
#define GP2X_BUTTON_START           (8)
#define GP2X_BUTTON_SELECT          (9)
#define GP2X_BUTTON_VOLUP           (16)
#define GP2X_BUTTON_VOLDOWN         (17)
#endif

#define DEFAULT_BRIGHTNESS 224

//#define glColor4hack(r,g,b,a) (glColor4x(r * Q(0, 1, 255), g * Q(0, 1, 255), b * Q(0, 1, 255), a * Q(0, 1, 255)))

//#define glColor4hack(r,g,b,a) glColor4x(255 * (Q(0,1,255)), 255 * (Q(0,1,255)), 255 * (Q(0,1,255)), 255 * (Q(0,1,255)))

extern float eyeX, eyeY, eyeZ;
extern float pitch, roll;
extern float zoom;
//senquack
//extern Uint8 *keys;
//extern SDL_Joystick *stick;
#ifdef GCW
extern SDL_Joystick *joy_analog;
#endif

extern int buttonReversed;
extern int lowres;
extern int windowMode;
extern int brightness;

/* senquack - added this struct to aid vertice/color interleaving for batch-drawing: */
typedef struct {
#ifdef FIXEDMATH
   GLfixed x,y;
#else
   GLfloat x,y;
#endif //FIXEDMATH
   union {
      uint32_t color_rgba;
      GLubyte r,g,b,a;
   };
} gl_vertex;

int getPadState ();
int getButtonState ();

void loadModel (char *fileName, GLuint * model);
void loadGLTexture (char *, GLuint *);
void generateTexture (GLuint *);
void deleteTexture (GLuint *);
void initSDL ();
void closeSDL ();
void resized (int, int);
void drawGLSceneStart ();
void drawGLSceneEnd ();
void swapGLScene ();

int initGLES();
int closeGLES();

//senquack - added our own copy of gluPerspective
#ifdef FIXEDMATH
void gluPerspective (GLfixed fovy, GLfixed ratio, GLfixed near, GLfixed far);
#else
void gluPerspective (GLfloat fovy, GLfloat ratio, GLfloat near, GLfloat far);
#endif //FIXEDMATH



void setScreenShake (int type, int cnt);
void moveScreenShake ();

#ifdef FIXEDMATH
void drawBox(GLfixed fx, GLfixed fy, GLfixed fwidth, GLfixed fheight, int r, int g, int b);
#else
void drawBox(GLfloat x, GLfloat y, GLfloat width, GLfloat height, int r, int g, int b);
#endif //FIXEDMATH

//senquack - added these to support batch drawing of boxes:
void prepareDrawBoxes (void);
void finishDrawBoxes (void);

//senquack - added these to support batch drawing of boxes:
void prepareDrawLines();
void finishDrawLines();
//senquack - converted to 2D (no need for z coordinate)
//void drawLine(GLfloat x1, GLfloat y1, GLfloat z1,
//       GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b, int a);
#ifdef FIXEDMATH
void drawLine (GLfixed x1, GLfixed y1, GLfixed x2, GLfixed y2, int r, int g, int b, int a);
#else
void drawLine (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, int r, int g, int b, int a);
#endif //FIXEDMATH

//senquack - converted to 2D (no need for z coordinate)
//void drawLinePart (GLfloat x1, GLfloat y1, GLfloat z1,
//                   GLfloat x2, GLfloat y2, GLfloat z2, int r, int g, int b,
//                   int a, int len);
#ifdef FIXEDMATH
void drawLinePart (GLfixed x1, GLfixed y1, GLfixed x2, GLfixed y2, int r,
                    int g, int b, int a, int len);
#else
void drawLinePart (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, int r,
                    int g, int b, int a, int len);
#endif //FIXEDMATH

//senquack - these were for drawing the background lines, and I directly draw the lines there now, no need for this anymore:
//#ifdef FIXEDMATH
//void drawRollLineAbs (GLfixed x1, GLfixed y1, GLfixed z1,
//                  GLfixed x2, GLfixed y2, GLfixed z2, int r, int g, int b,
//                  int a, int d1)
//#else
//void drawRollLineAbs (GLfloat x1, GLfloat y1, GLfloat z1, GLfloat x2,
//                      GLfloat y2, GLfloat z2, int r, int g, int b, int a,
//                      int d1);
//#endif //FIXEDMATH

//void drawRollLine (GLfloat x, GLfloat y, GLfloat z, GLfloat width, int r,
//                   int g, int b, int a, int d1, int d2);

//senquack - (only called from frag.c)
// note: we dropped the a parameter (always 255 it turns out)
#ifdef FIXEDMATH
void drawRollLine (GLfixed x, GLfixed y, GLfixed z, GLfixed width,
                    int r, int g, int b, int d1, int d2);
#else
void drawRollLine (GLfloat x, GLfloat y, GLfloat z, GLfloat width,
                    int r, int g, int b, int d1, int d2);
#endif //FIXEDMATH

//senquack - this allows us to avoid a few hundred unnecessary calls to two functions:
void prepareDrawRollLine (void);

#ifdef FIXEDMATH
void drawSquare (GLfixed x1, GLfixed y1, GLfixed z1,
                  GLfixed x2, GLfixed y2, GLfixed z2,
                  GLfixed x3, GLfixed y3, GLfixed z3,
                  GLfixed x4, GLfixed y4, GLfixed z4, int r, int g, int b);
#else
void drawSquare (GLfloat x1, GLfloat y1, GLfloat z1,
                 GLfloat x2, GLfloat y2, GLfloat z2,
                 GLfloat x3, GLfloat y3, GLfloat z3,
                 GLfloat x4, GLfloat y4, GLfloat z4, int r, int g, int b);
#endif //FIXEDMATH

//void drawStar(int f, GLfloat x, GLfloat y, GLfloat z, int r, int g, int b, float size);
#ifdef FIXEDMATH
void drawStar (int f, GLfixed x, GLfixed y, int r, int g, int b, GLfixed size);
#else
void drawStar (int f, GLfloat x, GLfloat y, int r, int g, int b, GLfloat size);
#endif //FIXEDMATH

//senquack - converted to fixed point, only one color passed
//void drawLaser(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
//        int cc1, int cc2, int cc3, int cc4, int cnt, int type);
//void drawLaserx(GLfixed fx, GLfixed fy, GLfixed fwidth, GLfixed fheight,
//        int cc1, int cc2, int cc3, int cc4, int cnt, int type);
#ifdef FIXEDMATH
void drawLaser (GLfixed x, GLfixed y, GLfixed width, GLfixed height,
                 int cc1, int cnt, int type);
#else
void drawLaser (GLfloat x, GLfloat y, GLfloat width, GLfloat height,
                 int cc1, int cnt, int type);
#endif //FIXEDMATH

//senquack - two new functions :
void prepareDrawLaser (void);
void finishDrawLaser (void);

#ifdef FIXEDMATH
void drawCore(GLfixed x, GLfixed y, int cnt, int r, int g, int b);
#else
void drawCore(GLfloat x, GLfloat y, int cnt, int r, int g, int b);
#endif //FIXEDMATH

#ifdef FIXEDMATH
void drawShipShape(GLfixed x, GLfixed y, float d, int inv);
#else
void drawShipShape(GLfloat x, GLfloat y, float d, int inv);
#endif //FIXEDMATH

#ifdef FIXEDMATH
void drawBomb(GLfixed x, GLfixed y, GLfixed width, int cnt);
#else
void drawBomb(GLfloat x, GLfloat y, GLfloat width, int cnt);
#endif //FIXEDMATH

#ifdef FIXEDMATH
void drawCircle(GLfixed x, GLfixed y, GLfixed width, int cnt,
                  int r1, int g1, int b1, int r2, int b2, int g2);
#else
void drawCircle(GLfloat x, GLfloat y, GLfloat width, int cnt,
                  int r1, int g1, int b1, int r2, int b2, int g2);
#endif //FIXEDMATH

//senquack - conversion to fixed point
#ifdef FIXEDMATH
void drawShape (GLfixed x, GLfixed y, GLfixed size, int d, int cnt, int type, int r, int g, int b);
void drawShapeIka (GLfixed x, GLfixed y, GLfixed size, int d, int cnt, int type, int c);
#else
void drawShape(GLfloat x, GLfloat y, GLfloat size, int d, int cnt, int type, int r, int g, int b);
void drawShapeIka(GLfloat x, GLfloat y, GLfloat size, int d, int cnt, int type, int c);
#endif //FIXEDMATH

//senquack - new functions called once before and after a series of calls to drawShape (for openglES speedup)
void prepareDrawShapes (void);
void finishDrawShapes (void);

// senquack - moved drawing shots back to 100% float
//#ifdef FIXEDMATH
//void drawShot(GLfixed x, GLfixed y, GLfixed d, int c, GLfixed width, GLfixed height);
//#else
void drawShot(GLfloat x, GLfloat y, GLfloat d, int c, float width, float height);
//#endif //FIXEDMATH
void startDrawBoards ();
void endDrawBoards ();
void drawSideBoards ();
void drawTitleBoard ();

int drawNum (int n, int x, int y, int s, int r, int g, int b);
//senquack made a new function to draw horizontally:
int drawNumHoriz (int n, int x, int y, int s, int r, int g, int b);
int drawNumRight (int n, int x, int y, int s, int r, int g, int b);
int drawNumCenter (int n, int x, int y, int s, int r, int g, int b);
int drawTimeCenter (int n, int x, int y, int s, int r, int g, int b);

//void CrossProd(float x1, float y1, float z1, float x2, float y2, float z2, float res[3]);

//senquack - no need for this:
//void FadiGluLookAt(float eyeX, float eyeY, float eyeZ, float lookAtX, float lookAtY, float lookAtZ, float upX, float upY, float upZ);

#ifdef FIXEDMATH
void gluLookAt(GLfixed eyex, GLfixed eyey, GLfixed eyez,
                 GLfixed centerx, GLfixed centery, GLfixed centerz,
                 GLfixed upx, GLfixed upy, GLfixed upz);
#else
void gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez,
                GLfloat centerx, GLfloat centery, GLfloat centerz,
                GLfloat upx, GLfloat upy, GLfloat upz);
#endif //FIXEDMATH

//senquack - Albert had added this I think
//void drawTestPoly();

#endif //SCREEN_H
