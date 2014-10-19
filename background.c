/*
 * $Id: background.c,v 1.1.1.1 2003/03/16 07:03:49 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * Screen background.
 *
 * @version $Revision: 1.1.1.1 $
 */
#include "genmcr.h"
#include "background.h"
#include "screen.h"

#define PLANE_MAX 4

static Plane plane[PLANE_MAX];
static int planeNum;

//senquack - conversion to fixed point:
//void initBackground(int s) {
//  int i;
//  Plane *pl;
//  switch ( s ) {
//  case 0:
//    planeNum = 3;
//    for ( i=0 ; i<planeNum ; i++ ) {
//      pl = &(plane[i]);
//      pl->xn = 8; pl->yn = 16;
//      pl->width = 3.0f; pl->height = 2.0f;
//      pl->x = -pl->width*pl->xn/2; 
//      pl->y = -pl->height*pl->yn/2;
//      pl->z = -10.0f;
//      pl->ox = pl->oy = 0;
//      switch ( i ) {
//      case 0:
//	pl->r = 250; pl->g = 200; pl->b = 20; pl->a = 100;
//	pl->d1 = 0;
//	pl->mx = 0; pl->my = 0.05;
//	break;
//      case 1:
//	pl->r = 200; pl->g = 150; pl->b = 20; pl->a = 50;
//	pl->d1 = 32;
//	pl->mx = 0.04; pl->my = 0.05;
//	break;
//      case 2:
//	pl->r = 200; pl->g = 150; pl->b = 20; pl->a = 50;
//	pl->d1 = -32&1023;
//	pl->mx = -0.04; pl->my = 0.05;
//	break;
//      }
//    }
//    break;
//  case 1:
//    planeNum = 2;
//    for ( i=0 ; i<planeNum ; i++ ) {
//      pl = &(plane[i]);
//      pl->xn = 8; pl->yn = 8;
//      pl->width = 3.0f; pl->height = 3.0f;
//      pl->x = -pl->width*pl->xn/2; 
//      pl->y = -pl->height*pl->yn/2;
//      pl->z = -10.0f;
//      pl->ox = pl->oy = 0;
//      switch ( i ) {
//      case 0:
//	pl->r = 200; pl->g = 100; pl->b = 200; pl->a = 150;
//	pl->d1 = 4;
//	pl->mx = 0; pl->my = 0.12;
//	break;
//      case 1:
//	pl->r = 120; pl->g = 100; pl->b = 120; pl->a = 120;
//	pl->d1 = 4;
//	pl->mx = 0; pl->my = -0.05;
//	break;
//      }
//    }
//    break;
//  case 2:
//    planeNum = 3;
//    for ( i=0 ; i<planeNum ; i++ ) {
//      pl = &(plane[i]);
//      switch ( i ) {
//      case 0:
//	pl->xn = 6; pl->yn = 6;
//	pl->width = 5.0f; pl->height = 5.0f;
//	break;
//      case 1:
//      case 2:
//	pl->xn = 16; pl->yn = 16;
//	pl->width = 1.5f; pl->height = 1.5f;
//	break;
//      }
//      pl->x = -pl->width*pl->xn/2; 
//      pl->y = -pl->height*pl->yn/2;
//      pl->z = -10.0f;
//      pl->ox = pl->oy = 0;
//      switch ( i ) {
//      case 0:
//	pl->r = 150; pl->g = 200; pl->b = 150; pl->a = 125;
//	pl->d1 = 0;
//	pl->mx = 0; pl->my = 0.04;
//	break;
//      case 1:
//	pl->r = 170; pl->g = 200; pl->b = 170; pl->a = 60;
//	pl->d1 = 0;
//	pl->mx = 0.01; pl->my = 0.01;
//	break;
//      case 2:
//	pl->r = 170; pl->g = 200; pl->b = 170; pl->a = 60;
//	pl->d1 = 0;
//	pl->mx = -0.01; pl->my = 0.01;
//	break;
//      }
//    }
//    break;
//  case 3:
//    planeNum = 4;
//    for ( i=0 ; i<planeNum ; i++ ) {
//      pl = &(plane[i]);
//      pl->xn = 8; pl->yn = 16;
//      pl->width = 4.0f; pl->height = 2.5f;
//      pl->x = -pl->width*pl->xn/2; 
//      pl->y = -pl->height*pl->yn/2;
//      pl->z = -10.0f;
//      pl->ox = pl->oy = 0;
//      pl->r = 200; pl->g = 200; pl->b = 100; 
//      switch ( i ) {
//      case 0:
//	pl->a = 72;
//	pl->d1 = 0;
//	pl->mx = -0.05; pl->my = 0.1;
//	break;
//      case 1:
//	pl->a = 40;
//	pl->d1 = 10;
//	pl->mx = -0.025; pl->my = 0;
//	break;
//      case 2:
//	pl->a = 40;
//	pl->d1 = -10;
//	pl->mx = 0.025; pl->my = 0;
//	break;
//      case 3:
//	pl->a = 72;
//	pl->d1 = 0;
//	pl->mx = -0.025; pl->my = 0.1;
//	break;
//      }
//    }
//    break;
//  }
//}
void initBackground(int s) {
#ifdef FIXEDMATH
   int i;
   Plane *pl;
   switch ( s ) {
      case 0:
         planeNum = 3;
         for ( i=0 ; i<planeNum ; i++ ) {
            pl = &(plane[i]);
            pl->xn = 8; pl->yn = 16;
            //      pl->width = 3.0f; pl->height = 2.0f;
            pl->fwidth = f2x(3.0f); pl->fheight = f2x(2.0f);
            //      pl->x = -pl->width*pl->xn/2; 
            //      pl->y = -pl->height*pl->yn/2;
            //      pl->z = -10.0f;
            pl->fx = FMUL(-pl->fwidth,INT2FNUM(pl->xn))>>1; 
            pl->fy = FMUL(-pl->fheight,INT2FNUM(pl->yn))>>1;
            pl->fz = f2x(-10.0f);
            //      pl->ox = pl->oy = 0;
            pl->fox = pl->foy = 0;
            switch ( i ) {
               case 0:
                  pl->r = 250; pl->g = 200; pl->b = 20; pl->a = 100;
                  pl->d1 = 0;
                  //	pl->mx = 0; pl->my = 0.05;
                  pl->fmx = 0; pl->fmy = f2x(0.05);
                  break;
               case 1:
                  pl->r = 200; pl->g = 150; pl->b = 20; pl->a = 50;
                  pl->d1 = 32;
                  //	pl->mx = 0.04; pl->my = 0.05;
                  pl->fmx = f2x(0.04); pl->fmy = f2x(0.05);
                  break;
               case 2:
                  pl->r = 200; pl->g = 150; pl->b = 20; pl->a = 50;
                  pl->d1 = -32&1023;
                  //	pl->mx = -0.04; pl->my = 0.05;
                  pl->fmx = f2x(-0.04); pl->fmy = f2x(0.05);
                  break;
            }
         }
         break;
      case 1:
         planeNum = 2;
         for ( i=0 ; i<planeNum ; i++ ) {
            pl = &(plane[i]);
            pl->xn = 8; pl->yn = 8;
            //      pl->width = 3.0f; pl->height = 3.0f;
            pl->fwidth = f2x(3.0f); pl->fheight = f2x(3.0f);
            //      pl->x = -pl->width*pl->xn/2; 
            //      pl->y = -pl->height*pl->yn/2;
            //      pl->z = -10.0f;
            //      pl->ox = pl->oy = 0;
            pl->fx = FMUL(-pl->fwidth,INT2FNUM(pl->xn))>>1; 
            pl->fy = FMUL(-pl->fheight,INT2FNUM(pl->yn))>>1;
            pl->fz = f2x(-10.0f);
            pl->fox = pl->foy = 0;
            switch ( i ) {
               case 0:
                  pl->r = 200; pl->g = 100; pl->b = 200; pl->a = 150;
                  pl->d1 = 4;
                  //	pl->mx = 0; pl->my = 0.12;
                  pl->fmx = 0; pl->fmy = f2x(0.12);
                  break;
               case 1:
                  pl->r = 120; pl->g = 100; pl->b = 120; pl->a = 120;
                  pl->d1 = 4;
                  //	pl->mx = 0; pl->my = -0.05;
                  pl->fmx = 0; pl->fmy = f2x(-0.05);
                  break;
            }
         }
         break;
      case 2:
         planeNum = 3;
         for ( i=0 ; i<planeNum ; i++ ) {
            pl = &(plane[i]);
            switch ( i ) {
               case 0:
                  pl->xn = 6; pl->yn = 6;
                  //	pl->width = 5.0f; pl->height = 5.0f;
                  pl->fwidth = f2x(5.0f); pl->fheight = f2x(5.0f);
                  break;
               case 1:
               case 2:
                  pl->xn = 16; pl->yn = 16;
                  //	pl->width = 1.5f; pl->height = 1.5f;
                  pl->fwidth = f2x(1.5f); pl->fheight = f2x(1.5f);
                  break;
            }
            //      pl->x = -pl->width*pl->xn/2; 
            //      pl->y = -pl->height*pl->yn/2;
            //      pl->z = -10.0f;
            //      pl->ox = pl->oy = 0;
            pl->fx = FMUL(-pl->fwidth,INT2FNUM(pl->xn))>>1; 
            pl->fy = FMUL(-pl->fheight,INT2FNUM(pl->yn))>>1;
            pl->fz = f2x(-10.0f);
            pl->fox = pl->foy = 0;
            switch ( i ) {
               case 0:
                  pl->r = 150; pl->g = 200; pl->b = 150; pl->a = 125;
                  pl->d1 = 0;
                  //	pl->mx = 0; pl->my = 0.04;
                  pl->fmx = 0; pl->fmy = f2x(0.04);
                  break;
               case 1:
                  pl->r = 170; pl->g = 200; pl->b = 170; pl->a = 60;
                  pl->d1 = 0;
                  //	pl->mx = 0.01; pl->my = 0.01;
                  pl->fmx = f2x(0.01); pl->fmy = f2x(0.01);
                  break;
               case 2:
                  pl->r = 170; pl->g = 200; pl->b = 170; pl->a = 60;
                  pl->d1 = 0;
                  //	pl->mx = -0.01; pl->my = 0.01;
                  pl->fmx = f2x(-0.01); pl->fmy = f2x(0.01);
                  break;
            }
         }
         break;
      case 3:
         planeNum = 4;
         for ( i=0 ; i<planeNum ; i++ ) {
            pl = &(plane[i]);
            pl->xn = 8; pl->yn = 16;
            //      pl->width = 4.0f; pl->height = 2.5f;
            //      pl->x = -pl->width*pl->xn/2; 
            //      pl->y = -pl->height*pl->yn/2;
            //      pl->z = -10.0f;
            //      pl->ox = pl->oy = 0;
            pl->fwidth = f2x(4.0f); pl->fheight = f2x(2.5f);
            pl->fx = FMUL(-pl->fwidth,INT2FNUM(pl->xn))>>1; 
            pl->fy = FMUL(-pl->fheight,INT2FNUM(pl->yn))>>1;
            pl->fz = f2x(-10.0f);
            pl->fox = pl->foy = 0;
            pl->r = 200; pl->g = 200; pl->b = 100; 
            switch ( i ) {
               case 0:
                  pl->a = 72;
                  pl->d1 = 0;
                  //	pl->mx = -0.05; pl->my = 0.1;
                  pl->fmx = f2x(-0.05); pl->fmy = f2x(0.1);
                  break;
               case 1:
                  pl->a = 40;
                  pl->d1 = 10;
                  //	pl->mx = -0.025; pl->my = 0;
                  pl->fmx = f2x(-0.025); pl->fmy = 0;
                  break;
               case 2:
                  pl->a = 40;
                  pl->d1 = -10;
                  //	pl->mx = 0.025; pl->my = 0;
                  pl->fmx = f2x(0.025); pl->fmy = 0;
                  break;
               case 3:
                  pl->a = 72;
                  pl->d1 = 0;
                  //	pl->mx = -0.025; pl->my = 0.1;
                  pl->fmx = f2x(-0.025); pl->fmy = f2x(0.1);
                  break;
            }
         }
         break;
   }
#else
  int i;
  Plane *pl;
  switch ( s ) {
  case 0:
    planeNum = 3;
    for ( i=0 ; i<planeNum ; i++ ) {
      pl = &(plane[i]);
      pl->xn = 8; pl->yn = 16;
      pl->width = 3.0f; pl->height = 2.0f;
      pl->x = -pl->width*pl->xn/2; 
      pl->y = -pl->height*pl->yn/2;
      pl->z = -10.0f;
      pl->ox = pl->oy = 0;
      switch ( i ) {
      case 0:
	pl->r = 250; pl->g = 200; pl->b = 20; pl->a = 100;
	pl->d1 = 0;
	pl->mx = 0; pl->my = 0.05;
	break;
      case 1:
	pl->r = 200; pl->g = 150; pl->b = 20; pl->a = 50;
	pl->d1 = 32;
	pl->mx = 0.04; pl->my = 0.05;
	break;
      case 2:
	pl->r = 200; pl->g = 150; pl->b = 20; pl->a = 50;
	pl->d1 = -32&1023;
	pl->mx = -0.04; pl->my = 0.05;
	break;
      }
    }
    break;
  case 1:
    planeNum = 2;
    for ( i=0 ; i<planeNum ; i++ ) {
      pl = &(plane[i]);
      pl->xn = 8; pl->yn = 8;
      pl->width = 3.0f; pl->height = 3.0f;
      pl->x = -pl->width*pl->xn/2; 
      pl->y = -pl->height*pl->yn/2;
      pl->z = -10.0f;
      pl->ox = pl->oy = 0;
      switch ( i ) {
      case 0:
	pl->r = 200; pl->g = 100; pl->b = 200; pl->a = 150;
	pl->d1 = 4;
	pl->mx = 0; pl->my = 0.12;
	break;
      case 1:
	pl->r = 120; pl->g = 100; pl->b = 120; pl->a = 120;
	pl->d1 = 4;
	pl->mx = 0; pl->my = -0.05;
	break;
      }
    }
    break;
  case 2:
    planeNum = 3;
    for ( i=0 ; i<planeNum ; i++ ) {
      pl = &(plane[i]);
      switch ( i ) {
      case 0:
	pl->xn = 6; pl->yn = 6;
	pl->width = 5.0f; pl->height = 5.0f;
	break;
      case 1:
      case 2:
	pl->xn = 16; pl->yn = 16;
	pl->width = 1.5f; pl->height = 1.5f;
	break;
      }
      pl->x = -pl->width*pl->xn/2; 
      pl->y = -pl->height*pl->yn/2;
      pl->z = -10.0f;
      pl->ox = pl->oy = 0;
      switch ( i ) {
      case 0:
	pl->r = 150; pl->g = 200; pl->b = 150; pl->a = 125;
	pl->d1 = 0;
	pl->mx = 0; pl->my = 0.04;
	break;
      case 1:
	pl->r = 170; pl->g = 200; pl->b = 170; pl->a = 60;
	pl->d1 = 0;
	pl->mx = 0.01; pl->my = 0.01;
	break;
      case 2:
	pl->r = 170; pl->g = 200; pl->b = 170; pl->a = 60;
	pl->d1 = 0;
	pl->mx = -0.01; pl->my = 0.01;
	break;
      }
    }
    break;
  case 3:
    planeNum = 4;
    for ( i=0 ; i<planeNum ; i++ ) {
      pl = &(plane[i]);
      pl->xn = 8; pl->yn = 16;
      pl->width = 4.0f; pl->height = 2.5f;
      pl->x = -pl->width*pl->xn/2; 
      pl->y = -pl->height*pl->yn/2;
      pl->z = -10.0f;
      pl->ox = pl->oy = 0;
      pl->r = 200; pl->g = 200; pl->b = 100; 
      switch ( i ) {
      case 0:
	pl->a = 72;
	pl->d1 = 0;
	pl->mx = -0.05; pl->my = 0.1;
	break;
      case 1:
	pl->a = 40;
	pl->d1 = 10;
	pl->mx = -0.025; pl->my = 0;
	break;
      case 2:
	pl->a = 40;
	pl->d1 = -10;
	pl->mx = 0.025; pl->my = 0;
	break;
      case 3:
	pl->a = 72;
	pl->d1 = 0;
	pl->mx = -0.025; pl->my = 0.1;
	break;
      }
    }
    break;
  }
#endif //FIXEDMATH
}

//senquack - converted to fixed point:
//void moveBackground() {
//  int i;
//  Plane *pl;
//  for ( i=0 ; i<planeNum ; i++ ) {
//    pl = &(plane[i]);
//    pl->ox -= pl->mx;
//    if ( pl->ox < 0 )          pl->ox += pl->width;
//    if ( pl->ox >= pl->width ) pl->ox -= pl->width;
//    pl->oy -= pl->my;
//    if ( pl->oy < 0 )           pl->oy += pl->height;
//    if ( pl->oy >= pl->height ) pl->oy -= pl->height;
//  }
//}
void moveBackground() {
#ifdef FIXEDMATH
   int i;
   Plane *pl;
   for ( i=0 ; i<planeNum ; i++ ) {
      pl = &(plane[i]);
      pl->fox -= pl->fmx;
      if ( pl->fox < 0 )          pl->fox += pl->fwidth;
      if ( pl->fox >= pl->fwidth ) pl->fox -= pl->fwidth;
      pl->foy -= pl->fmy;
      if ( pl->foy < 0 )           pl->foy += pl->fheight;
      if ( pl->foy >= pl->fheight ) pl->foy -= pl->fheight;
   }
#else
   int i;
   Plane *pl;
   for ( i=0 ; i<planeNum ; i++ ) {
      pl = &(plane[i]);
      pl->ox -= pl->mx;
      if ( pl->ox < 0 )          pl->ox += pl->width;
      if ( pl->ox >= pl->width ) pl->ox -= pl->width;
      pl->oy -= pl->my;
      if ( pl->oy < 0 )           pl->oy += pl->height;
      if ( pl->oy >= pl->height ) pl->oy -= pl->height;
   }
#endif //FIXEDMATH
}

//senquack - optimizing this by bringing OpenGLES calls here, converting to fixed point:
//void drawBackground() {
//  int lx, ly, i;
//  float x, y;
//  Plane *pl;
//  for ( i=0 ; i<planeNum ; i++ ) {
//    pl = &(plane[i]);
//    x = pl->x+pl->ox;
//    for ( lx=0 ; lx<pl->xn ; lx++, x+=pl->width ) {
//      drawRollLineAbs(x, pl->y+pl->oy, pl->z, x, pl->y+pl->oy+pl->height*pl->yn, pl->z,
//		      pl->r, pl->g, pl->b, pl->a, pl->d1);
//    }
//    y = pl->y + pl->oy;
//    for ( ly=0 ; ly<pl->yn ; ly++, y+=pl->height ) {
//      drawRollLineAbs(pl->x+pl->ox, y, pl->z, pl->x+pl->ox+pl->width*pl->xn, y, pl->z,
//		      pl->r, pl->g, pl->b, pl->a, pl->d1);
//    }
//  }
//}
//senquack TODO: I believe I disabled the background for Wiz, so we need to check both this fixed and the lower float code
//senquack TODO: interleave
#ifdef FIXEDMATH
void drawBackground() {
#ifdef FIXEDMATH
   GLfixed bgvertices[64*3];	// array of line vertices (only ever 32 maximum drawn lines)
   GLubyte bgcolors[64*4];		// array of line vertices' colors
   GLfixed	*bgverticeptr;				// ptr to next vertice to enter data to
   GLubyte	*bgcolorptr;					// ptr to next vertice color to enter data to

   glVertexPointer(3, GL_FIXED, 0, bgvertices);
   glColorPointer(4, GL_UNSIGNED_BYTE, 0, bgcolors);
   //	glEnable(GL_BLEND);

   int lx, ly, i;
   GLfixed x, y;
   Plane *pl;
   GLubyte tmpr, tmpg, tmpb, tmpa;
   int	numbgvertices;
   for ( i=0 ; i<planeNum ; i++ ) {
      bgverticeptr = &(bgvertices[0]);		bgcolorptr = &(bgcolors[0]);

      pl = &(plane[i]);
      x = pl->fx+pl->fox;
      tmpr = pl->r; tmpg = pl->g; tmpb = pl->b; tmpa = pl->a;
      //Pull out and pre-convert xn and yn to fixed before loops:
      GLfixed tmpfxn = INT2FNUM(pl->xn);
      GLfixed tmpfyn = INT2FNUM(pl->yn);
      //Pull out these values before loops:
      GLfixed tmpfx = pl->fx;
      GLfixed tmpfy = pl->fy;
      GLfixed tmpfz = pl->fz;
      GLfixed tmpfox = pl->fox;
      GLfixed tmpfoy = pl->foy;
      GLfixed tmpfheight = pl->fheight;
      GLfixed tmpfwidth = pl->fwidth;
      int tmpxn = pl->xn;
      int tmpyn = pl->yn;
      GLfixed tmpxn_times_width = FMUL(INT2FNUM(tmpxn), tmpfwidth);
      GLfixed tmpyn_times_height = FMUL(INT2FNUM(tmpyn), tmpfheight);
      for ( lx=0 ; lx < tmpxn ; lx++, x += tmpfwidth ) {
         //senquack TODO: I dunno if I ever got all this conversion right for the Wiz, better check it twice:
         //      drawRollLineAbs(x, pl->y+pl->oy, pl->z, x, pl->y+pl->oy+pl->height*pl->yn, pl->z,
         //		      pl->r, pl->g, pl->b, pl->a, pl->d1);
         *bgverticeptr++ = x; 	
//         *bgverticeptr++ = pl->fy+pl->foy;		
         *bgverticeptr++ = tmpfy + tmpfoy;
//         *bgverticeptr++ = pl->fz;
         *bgverticeptr++ = tmpfz;
         *bgverticeptr++ = x;		
//         *bgverticeptr++ = pl->fy+pl->foy +FMUL(pl->fheight,INT2FNUM(pl->yn));	
         *bgverticeptr++ = tmpfy + tmpfoy + tmpyn_times_height;
         *bgverticeptr++ = tmpfz;
         //senquack TODO: optimize this to store RGBA all at once (do we worry about endian-ness?)
         *bgcolorptr++ = tmpr;	*bgcolorptr++ = tmpg;	*bgcolorptr++ = tmpb;	*bgcolorptr++ = tmpa;
         *bgcolorptr++ = tmpr;	*bgcolorptr++ = tmpg;	*bgcolorptr++ = tmpb;	*bgcolorptr++ = tmpa;
      }
//      y = pl->fy + pl->foy;
      y = tmpfy + tmpfoy;
      for ( ly=0 ; ly < tmpyn ; ly++, y += tmpfheight ) {
         //senquack TODO: I dunno if I ever got all this conversion right for the Wiz, better check it twice:
         //      drawRollLineAbs(pl->x+pl->ox, y, pl->z, pl->x+pl->ox+pl->width*pl->xn, y, pl->z,
         //		      pl->r, pl->g, pl->b, pl->a, pl->d1);
//         *bgverticeptr++ = pl->fx+pl->fox; 
         *bgverticeptr++ = tmpfx + tmpfox; 
         *bgverticeptr++ = y; 
//         *bgverticeptr++ = pl->fz;
         *bgverticeptr++ = tmpfz;
//         *bgverticeptr++ = pl->fx+pl->fox+FMUL(pl->fwidth,INT2FNUM(pl->xn));
         *bgverticeptr++ = tmpfx + tmpfox + tmpxn_times_width;
         *bgverticeptr++ = y;
//         *bgverticeptr++ = pl->fz;
         *bgverticeptr++ = tmpfz;
         *bgcolorptr++ = tmpr;	*bgcolorptr++ = tmpg;	*bgcolorptr++ = tmpb;	*bgcolorptr++ = tmpa;
         *bgcolorptr++ = tmpr;	*bgcolorptr++ = tmpg;	*bgcolorptr++ = tmpb;	*bgcolorptr++ = tmpa;
      }
      numbgvertices = ((unsigned int)bgcolorptr - (unsigned int)bgcolors) >> 2;
      //	printf("drawing %d vertices\n", numbgvertices);
      glPushMatrix();
      // rolled d1*360/1024 + fixed-conversion into this:
      glRotatex((pl->d1*360)<<6, 0, 0, INT2FNUM(1));
      glDrawArrays(GL_LINES, 0, numbgvertices);
      glPopMatrix();
   }
#else
   GLfloat  bgvertices[64*3];	// array of line vertices (only ever 32 maximum drawn lines)
   GLubyte  bgcolors[64*4];		// array of line vertices' colors
   GLfloat	*bgverticeptr;				// ptr to next vertice to enter data to
   GLubyte	*bgcolorptr;					// ptr to next vertice color to enter data to

   glVertexPointer(3, GL_FIXED, 0, bgvertices);
   glColorPointer(4, GL_UNSIGNED_BYTE, 0, bgcolors);
   //	glEnable(GL_BLEND);

   int lx, ly, i;
   GLfloat x, y;
   Plane *pl;
   GLubyte tmpr, tmpg, tmpb, tmpa;
   int	numbgvertices;
   for ( i=0 ; i<planeNum ; i++ ) {
      bgverticeptr = &(bgvertices[0]);		bgcolorptr = &(bgcolors[0]);
      pl = &(plane[i]);
      x = pl->x + pl->ox;
      tmpr = pl->r; tmpg = pl->g; tmpb = pl->b; tmpa = pl->a;
      //Pull out and pre-convert xn and yn to float before loops:
      GLfloat tmpxn = (float)pl->xn;
      GLfloat tmpyn = (float)pl->yn;
      //Pull out these values before loops:
      GLfloat tmpx = pl->x;
      GLfloat tmpy = pl->y;
      GLfloat tmpz = pl->z;
      GLfloat tmpox = pl->ox;
      GLfloat tmpoy = pl->oy;
      GLfloat tmpheight = pl->height;
      GLfloat tmpwidth = pl->width;
      int tmpxn = pl->xn;
      int tmpyn = pl->yn;
      GLfloat tmpxn_times_width = (float)tmpxn * tmpwidth;
      GLfloat tmpyn_times_height = (float)tmpyn * tmpheight;
      for ( lx=0 ; lx < tmpxn ; lx++, x += tmpwidth ) {
         //senquack TODO: I dunno if I ever got all this conversion right for the Wiz, better check it twice:
         //      drawRollLineAbs(x, pl->y+pl->oy, pl->z, x, pl->y+pl->oy+pl->height*pl->yn, pl->z,
         //		      pl->r, pl->g, pl->b, pl->a, pl->d1);
         *bgverticeptr++ = x; 	
         *bgverticeptr++ = tmpy + tmpoy;
         *bgverticeptr++ = tmpz;
         *bgverticeptr++ = x;		
         *bgverticeptr++ = tmpy + tmpoy + tmpyn_times_height;
         *bgverticeptr++ = tmpfz;
         //senquack TODO: interleave
         //senquack TODO: optimize this to store RGBA all at once (do we worry about endian-ness?)
         *bgcolorptr++ = tmpr;	*bgcolorptr++ = tmpg;	*bgcolorptr++ = tmpb;	*bgcolorptr++ = tmpa;
         *bgcolorptr++ = tmpr;	*bgcolorptr++ = tmpg;	*bgcolorptr++ = tmpb;	*bgcolorptr++ = tmpa;
      }
      y = tmpy + tmpoy;
      for ( ly=0 ; ly < tmpyn ; ly++, y += tmpheight ) {
         //senquack TODO: I dunno if I ever got all this conversion right for the Wiz, better check it twice:
         //      drawRollLineAbs(pl->x+pl->ox, y, pl->z, pl->x+pl->ox+pl->width*pl->xn, y, pl->z,
         //		      pl->r, pl->g, pl->b, pl->a, pl->d1);
         *bgverticeptr++ = tmpx + tmpox; 
         *bgverticeptr++ = y; 
         *bgverticeptr++ = tmpz;
         *bgverticeptr++ = tmpx + tmpox + tmpxn_times_width;
         *bgverticeptr++ = y;
         *bgverticeptr++ = tmpz;
//senquack TODO: interleave
         *bgcolorptr++ = tmpr;	*bgcolorptr++ = tmpg;	*bgcolorptr++ = tmpb;	*bgcolorptr++ = tmpa;
         *bgcolorptr++ = tmpr;	*bgcolorptr++ = tmpg;	*bgcolorptr++ = tmpb;	*bgcolorptr++ = tmpa;
      }
      numbgvertices = ((unsigned int)bgcolorptr - (unsigned int)bgcolors) >> 2;
      //	printf("drawing %d vertices\n", numbgvertices);
      glPushMatrix();
      //  (float)d1*360/1024 is what drawRollLineAbs does
      glRotatef((float)pl->d1 * 360/1024, 0, 0, 1.0);
      glDrawArrays(GL_LINES, 0, numbgvertices);
      glPopMatrix();
   }
#endif //FIXEDMATH
}
