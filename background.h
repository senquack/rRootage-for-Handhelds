/*
 * $Id: background.h,v 1.1.1.1 2003/03/16 07:03:49 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * Screen background.
 *
 * @version $Revision: 1.1.1.1 $
 */

//senquack - for fixed-point stuff:
#include "GLES/gl.h"
#include "degutil.h"

//senquack - converting to fixed point:
//typedef struct {
//  float x, y, z, ox, oy;
//  float mx, my;
//  int d1;
//  float width, height;
//  int xn, yn;
//  int r, g, b, a;
//} Plane;
typedef struct
{
   GLfixed fx, fy, fz, fox, foy;
   GLfixed fmx, fmy;
   int d1;
   GLfixed fwidth, fheight;
   int xn, yn;
   int r, g, b, a;
} Plane;

void initBackground ();
void moveBackground ();
void drawBackground ();
