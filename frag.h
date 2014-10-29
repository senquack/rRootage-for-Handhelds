/*
 * $Id: frag.h,v 1.2 2003/03/21 02:59:49 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * Fragment data.
 *
 * @version $Revision: 1.2 $
 */
#ifndef FRAG_H
#define FRAG_H

#include "vector.h"

#define FRAG_COLOR_NUM 2

//senquack - converted to fixed point:
//typedef struct {
//  float x, y, z;
//  float mx, my, mz;
//  float width, height;
//  int d1, d2, md1, md2;
//  int r[FRAG_COLOR_NUM], g[FRAG_COLOR_NUM], b[FRAG_COLOR_NUM];
//  int cnt;
//} Frag;
typedef struct
{
#ifdef FIXEDMATH
   GLfixed fx, fy, fz;
   GLfixed fmx, fmy, fmz;
   GLfixed fwidth, fheight;
#else
   float x, y, z;
   float mx, my, mz;
   float width, height;
#endif //FIXEDMATH
//senquack TODO: poss. optimization, should we change the degrees to floats?:
   int d1, d2, md1, md2;
   int r[FRAG_COLOR_NUM], g[FRAG_COLOR_NUM], b[FRAG_COLOR_NUM];
   int cnt;
} Frag;

#define FRAG_MAX 512

void initFrags ();
void moveFrags ();
void drawFrags ();
void addLaserFrag (int x, int y, int width);

#ifdef FIXEDMATH
//senquack - fixed point version:
void addBossFrag(GLfixed x, GLfixed y, GLfixed z, GLfixed width, int d);
#else
void addBossFrag(float x, float y, float z, float width, int d);
#endif //FIXEDMATH

#ifdef FIXEDMATH
//senquack - fixed point version:
void addShipFrag(GLfixed x, GLfixed y);
#else
void addShipFrag(float x, float y);
#endif //FIXEDMATH

void addGrazeFrag (int x, int y, int mx, int my);

#ifdef FIXEDMATH
void addShapeFrag(GLfixed x, GLfixed y, GLfixed size, int d, int cnt, int type, int mx, int my);
#else
void addShapeFrag(GLfloat x, GLfloat y, GLfloat size, int d, int cnt, int type, int mx, int my);
#endif //FIXEDMATH
#endif // FRAG_H
