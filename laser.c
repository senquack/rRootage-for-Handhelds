/*
 * $Id: laser.c,v 1.3 2003/04/26 03:24:15 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * Handle players laser.
 *
 * @version $Revision: 1.3 $
 */
#include "SDL.h"

#include "genmcr.h"
#include "screen.h"
#include "vector.h"
#include "degutil.h"
#include "laser.h"
#include "ship.h"
#include "frag.h"
#include "boss_mtd.h"

#define LASER_MAX 64

//senquack - fixed point version appended with _X:
#define LASER_SPEED 4096
#define LASER_WIDTH 4800
#define LASER_WIDTH_ADD 480
#define LASER_WIDTH_X 31457     //predivided by screen ratio
#define LASER_WIDTH_ADD_X 3146  //predivided by screen ratio

Laser laser[LASER_MAX];
//senquack - converting to fixed point:
#ifdef FIXEDMATH
static GLfixed laserfWidth;
static int laserCnt;
#else
static int laserWidth, laserCnt;
#endif //FIXEDMATH

//senquack - converting to fixed point
//void initLasers() {
//  int i;
//  for ( i=0 ; i<LASER_MAX ; i++ ) {
//    laser[i].cnt = NOT_EXIST;
//  }
//  laserWidth = laserCnt = 0;
//}
void
initLasers ()
{
   int i;
   for (i = 0; i < LASER_MAX; i++) {
      laser[i].cnt = NOT_EXIST;
   }
#ifdef FIXEDMATH
   laserfWidth = laserCnt = 0;
#else
   laserWidth = laserCnt = 0;
#endif //FIXEDMATH
}

#define LASER_COLOR_SPEED 12

static int laserIdx = LASER_MAX;
static int laserColor = 0;
static int laserAdded = 0;

//senquack - converting to fixed point
//void addLaser() {
//  int i;
//  for ( i=0 ; i<LASER_MAX ; i++ ) {
//    laserIdx--; if ( laserIdx < 0 ) laserIdx = LASER_MAX-1;
//    if ( laser[laserIdx].cnt == NOT_EXIST ) break;
//  }
//  if ( i >= LASER_MAX ) return;
//  laser[laserIdx].y = LASER_SPEED;
//  laser[laserIdx].color = laserColor;
//  laserColor -= LASER_COLOR_SPEED; laserColor &= 255;
//  laser[laserIdx].cnt = 0;
//  laserWidth += LASER_WIDTH_ADD;
//  if ( laserWidth > LASER_WIDTH ) laserWidth = LASER_WIDTH;
//  laserAdded = 1;
//}
void
addLaser ()
{
#ifdef FIXEDMATH
   int i;
   for (i = 0; i < LASER_MAX; i++) {
      laserIdx--;
      if (laserIdx < 0)
         laserIdx = LASER_MAX - 1;
      if (laser[laserIdx].cnt == NOT_EXIST)
         break;
   }
   if (i >= LASER_MAX)
      return;
   laser[laserIdx].y = LASER_SPEED;
   laser[laserIdx].color = laserColor;
   laserColor -= LASER_COLOR_SPEED;
   laserColor &= 255;
   laser[laserIdx].cnt = 0;
//  laserWidth += LASER_WIDTH_ADD;
   laserfWidth += LASER_WIDTH_ADD_X;
//  if ( laserWidth > LASER_WIDTH ) laserWidth = LASER_WIDTH;
   if (laserfWidth > LASER_WIDTH_X)
      laserfWidth = LASER_WIDTH_X;
   laserAdded = 1;
#else
  int i;
  for ( i=0 ; i<LASER_MAX ; i++ ) {
    laserIdx--; if ( laserIdx < 0 ) laserIdx = LASER_MAX-1;
    if ( laser[laserIdx].cnt == NOT_EXIST ) break;
  }
  if ( i >= LASER_MAX ) return;
  laser[laserIdx].y = LASER_SPEED;
  laser[laserIdx].color = laserColor;
  laserColor -= LASER_COLOR_SPEED; laserColor &= 255;
  laser[laserIdx].cnt = 0;
  laserWidth += LASER_WIDTH_ADD;
  if ( laserWidth > LASER_WIDTH ) laserWidth = LASER_WIDTH;
  laserAdded = 1;
#endif //FIXEDMATH
}

//senquack - converting to fixed point:
//void moveLasers() {
//  int i;
//  Laser *ls;
//  int ry;
//  int huy, hdy;
//  if ( !laserAdded ) {
//    laserWidth -= LASER_WIDTH_ADD;
//    if ( laserWidth < 0 ) {
//      laserWidth = 0;
//      for ( i=0 ; i<LASER_MAX ; i++ ) {
// laser[i].cnt = NOT_EXIST;
//      }
//      return;
//    }
//  } else {
//    laserAdded = 0;
//  }
//  hdy = checkHitDownside(ship.pos.x);
//  huy = checkHitUpside();
//  for ( i=0 ; i<LASER_MAX ; i++ ) {
//    if ( laser[i].cnt == NOT_EXIST ) continue;
//    else if ( laser[i].cnt == -1 ) { 
//      laser[i].cnt = NOT_EXIST;
//      continue;
//    }
//    ls = &(laser[i]);
//    ls->y -= LASER_SPEED;
//    ry = ship.pos.y + ls->y;
//    if ( huy < ry && ry < hdy ) {
//      damageBossLaser(ls->cnt);
//      if ( (laserCnt&3) == 0 ) {
// addLaserFrag(ship.pos.x, -ship.pos.y-ls->y, LASER_WIDTH);
//      }
//      ls->cnt = -1;
//      continue;
//    }
//    if ( ry < -FIELD_HEIGHT_8/2 ) {
//      ls->cnt = NOT_EXIST;
//      continue;
//    }
//    ls->cnt++;
//  }
//  laserCnt++;
//}
void
moveLasers ()
{
   int i;
   Laser *ls;
   int ry;
   int huy, hdy;
   if (!laserAdded) {
#ifdef FIXEDMATH
      laserfWidth -= LASER_WIDTH_ADD_X;
      if (laserfWidth < 0) {
         laserfWidth = 0;
#else
      laserWidth -= LASER_WIDTH_ADD;
      if ( laserWidth < 0 ) {
         laserWidth = 0;
#endif //FIXEDMATH
         for (i = 0; i < LASER_MAX; i++) {
            laser[i].cnt = NOT_EXIST;
         }
         return;
      }
   } else {
      laserAdded = 0;
   }
   hdy = checkHitDownside (ship.pos.x);
   huy = checkHitUpside ();
   for (i = 0; i < LASER_MAX; i++) {
      if (laser[i].cnt == NOT_EXIST)
         continue;
      else if (laser[i].cnt == -1) {
         laser[i].cnt = NOT_EXIST;
         continue;
      }
      ls = &(laser[i]);
      ls->y -= LASER_SPEED;
      ry = ship.pos.y + ls->y;
      if (huy < ry && ry < hdy) {
         damageBossLaser (ls->cnt);
         if ((laserCnt & 3) == 0) {
            addLaserFrag (ship.pos.x, -ship.pos.y - ls->y, LASER_WIDTH);
         }
         ls->cnt = -1;
         continue;
      }
      if (ry < -FIELD_HEIGHT_8 / 2) {
         ls->cnt = NOT_EXIST;
         continue;
      }
      ls->cnt++;
   }
   laserCnt++;
}

#define LASER_SCREEN_HEIGHT (LASER_SPEED/FIELD_SCREEN_RATIO)
//senquack - fixed point version:
#define LASER_SCREEN_HEIGHT_X 26844

//void drawLasers() {
//  float x, y;
//  int i;
//  Laser *ls;
//  int t;
//  for ( i=0 ; i<LASER_MAX ; i++ ) {
//    if ( laser[i].cnt == NOT_EXIST ) continue;
//    ls = &(laser[i]);
//    x =  (float)ship.pos.x / FIELD_SCREEN_RATIO;
//    y = -(float)(ship.pos.y+ls->y) / FIELD_SCREEN_RATIO;
//    if ( ls->cnt > 1 )       t = 1;
//    else if ( ls->cnt == 1 ) t = 0;
//    else                     t = 2;
//    drawLaser(x, y, (float)laserWidth/FIELD_SCREEN_RATIO, LASER_SCREEN_HEIGHT,
//       ls->color, 
//       (ls->color+LASER_COLOR_SPEED)&255,
//       (ls->color+LASER_COLOR_SPEED*2)&255,  
//       (ls->color+LASER_COLOR_SPEED*3)&255,
//       laserCnt, t);
//  }
//}
////senquack - some fixed point speedups
//void drawLasers() {
//  float x, y;
//  int i;
//  Laser *ls;
//  int t;
//  for ( i=0 ; i<LASER_MAX ; i++ ) {
//    if ( laser[i].cnt == NOT_EXIST ) continue;
//    //Changed by Albert because of strange crashes...
////    if ( laser[i].cnt != NOT_EXIST )
//    {
//        ls = &(laser[i]);
//        x =  (float)ship.pos.x / FIELD_SCREEN_RATIO;
//        y = -(float)(ship.pos.y+ls->y) / FIELD_SCREEN_RATIO;
//        if ( ls->cnt > 1 )       t = 1;
//        else if ( ls->cnt == 1 ) t = 0;
//        else                     t = 2;
//        drawLaser(x, y, (float)laserWidth/FIELD_SCREEN_RATIO, LASER_SCREEN_HEIGHT,
//             ls->color, 
//             (ls->color+LASER_COLOR_SPEED)&255,
//             (ls->color+LASER_COLOR_SPEED*2)&255,  
//             (ls->color+LASER_COLOR_SPEED*3)&255,
//             laserCnt, t);
// }
//  }
//}
//void drawLasers() {
////  float x, y;
//  GLfixed fx, fy;
//  int i;
//  Laser *ls;
//  int t;
//  for ( i=0 ; i<LASER_MAX ; i++ ) {
//    if ( laser[i].cnt == NOT_EXIST ) continue;
//    //Changed by Albert because of strange crashes...
////    if ( laser[i].cnt != NOT_EXIST )
//    {
//        ls = &(laser[i]);
////        x =  (float)ship.pos.x / FIELD_SCREEN_RATIO;
//        fx =  FDIV(INT2FNUM(ship.pos.x) , FIELD_SCREEN_RATIO_X);
////        y = -(float)(ship.pos.y+ls->y) / FIELD_SCREEN_RATIO;
//        fy = -FDIV(INT2FNUM(ship.pos.y+ls->y) , FIELD_SCREEN_RATIO_X);
//        if ( ls->cnt > 1 )       t = 1;
//        else if ( ls->cnt == 1 ) t = 0;
//        else                     t = 2;
////        drawLaser(x, y, (float)laserWidth/FIELD_SCREEN_RATIO, LASER_SCREEN_HEIGHT,
////           ls->color, 
////           (ls->color+LASER_COLOR_SPEED)&255,
////           (ls->color+LASER_COLOR_SPEED*2)&255,  
////           (ls->color+LASER_COLOR_SPEED*3)&255,
////           laserCnt, t);
//        drawLaserx(fx, fy, FDIV(INT2FNUM(laserWidth),FIELD_SCREEN_RATIO_X), 
//            FDIV(INT2FNUM(LASER_SPEED), FIELD_SCREEN_RATIO_X),
//             ls->color, 
//             (ls->color+LASER_COLOR_SPEED)&255,
//             (ls->color+LASER_COLOR_SPEED*2)&255,  
//             (ls->color+LASER_COLOR_SPEED*3)&255,
//             laserCnt, t);
// }
//  }
//}
//void drawLasers() {
//  float x, y;
//  int i;
//  Laser *ls;
//  int t;
//  for ( i=0 ; i<LASER_MAX ; i++ ) {
//    if ( laser[i].cnt == NOT_EXIST ) continue;
//    //Changed by Albert because of strange crashes...
////    if ( laser[i].cnt != NOT_EXIST )
//    {
//        ls = &(laser[i]);
//        x =  (float)ship.pos.x / FIELD_SCREEN_RATIO;
//        y = -(float)(ship.pos.y+ls->y) / FIELD_SCREEN_RATIO;
//        if ( ls->cnt > 1 )       t = 1;
//        else if ( ls->cnt == 1 ) t = 0;
//        else                     t = 2;
//        drawLaserx(f2x(x), f2x(y), f2x((float)laserWidth/FIELD_SCREEN_RATIO), f2x(LASER_SCREEN_HEIGHT),
//             ls->color, 
//             (ls->color+LASER_COLOR_SPEED)&255,
//             (ls->color+LASER_COLOR_SPEED*2)&255,  
//             (ls->color+LASER_COLOR_SPEED*3)&255,
//             laserCnt, t);
// }
//  }
//}
//senquack - converting to OpenGLES and doing drawing right here:
//void drawLasers() {
////  float x, y;
//  GLfixed fx, fy;
//  int i;
//  Laser *ls;
//  int t;
//
// prepareDrawLaserx();
//
//  //senquack - moved this outside the loop
// fx = (int)((float)ship.pos.x * 6.5536);   // roll float->fixed conversion and division by screen ratio together
//
//  for ( i=0 ; i<LASER_MAX ; i++ ) {
//
//    if ( laser[i].cnt == NOT_EXIST ) continue;
//    //Changed by Albert because of strange crashes...
////    if ( laser[i].cnt != NOT_EXIST )
//    {
//        ls = &(laser[i]);
////        x =  (float)ship.pos.x / FIELD_SCREEN_RATIO;
////        fx =  FDIV(INT2FNUM(ship.pos.x) , FIELD_SCREEN_RATIO_X);
////        y = -(float)(ship.pos.y+ls->y) / FIELD_SCREEN_RATIO;
//      //senquack - overflows:
////        fy = -FDIV(INT2FNUM(ship.pos.y+ls->y) , FIELD_SCREEN_RATIO_X);
////        fy = f2x(-(float)(ship.pos.y+ls->y) / FIELD_SCREEN_RATIO);
//      //senquack - roll float->fixed conversion and division together
//        fy = (int)(-(float)(ship.pos.y+ls->y) * 6.5536);
//
//        if ( ls->cnt > 1 )       t = 1;
//        else if ( ls->cnt == 1 ) t = 0;
//        else                     t = 2;
////        drawLaser(x, y, (float)laserWidth/FIELD_SCREEN_RATIO, LASER_SCREEN_HEIGHT,
////           ls->color, 
////           (ls->color+LASER_COLOR_SPEED)&255,
////           (ls->color+LASER_COLOR_SPEED*2)&255,  
////           (ls->color+LASER_COLOR_SPEED*3)&255,
////           laserCnt, t);
//        drawLaserx(fx, fy, laserfWidth, 
//         LASER_SCREEN_HEIGHT_X,
//             ls->color,
//             laserCnt, t);
// }
//  }
//
// finishDrawLaserx();
//}
void drawLasers ()
{
#ifdef FIXEDMATH
//  float x, y;
   GLfixed fx, fy;
   int i;
   Laser *ls;
   int t;

   prepareDrawLaser ();

   //senquack - moved this outside the loop
   // senquack - rolled divide-by-10000 and convert-to-fixed into one multiply:
////        x =  (float)ship.pos.x / FIELD_SCREEN_RATIO;
//senquack TODO: poss. optimization w/ inverse or further conversion to fixed:
   fx = (int) ((float) ship.pos.x * 6.5536);    // roll float->fixed conversion and division by screen ratio together

   for (i = 0; i < LASER_MAX; i++) {

      if (laser[i].cnt == NOT_EXIST)
         continue;
      {
         ls = &(laser[i]);
//        x =  (float)ship.pos.x / FIELD_SCREEN_RATIO;
         //senquack TODO: poss. optimization w/ inverse:
//        y = -(float)(ship.pos.y+ls->y) / FIELD_SCREEN_RATIO;
         // senquack - rolled divide-by-10000 and convert-to-fixed into one multiply:
//        fy = f2x(-(float)(ship.pos.y+ls->y) / FIELD_SCREEN_RATIO);
//senquack TODO: poss. optimization w/ inverse or further conversion to fixed:
         fy = (int) (-(float) (ship.pos.y + ls->y) * 6.5536);

         if (ls->cnt > 1)
            t = 1;
         else if (ls->cnt == 1)
            t = 0;
         else
            t = 2;
//        drawLaser(x, y, (float)laserWidth/FIELD_SCREEN_RATIO, LASER_SCREEN_HEIGHT,
//             ls->color, 
//             (ls->color+LASER_COLOR_SPEED)&255,
//             (ls->color+LASER_COLOR_SPEED*2)&255,  
//             (ls->color+LASER_COLOR_SPEED*3)&255,
//             laserCnt, t);
         drawLaserx (fx, fy, laserfWidth,
                     LASER_SCREEN_HEIGHT_X, ls->color, laserCnt, t);
      }
   }

   finishDrawLaser ();
#else
   float x, y;
   prepareDrawLaser ();
   int i;
   Laser *ls;
   int t;

   //moved this out of the loop
   x =  (float)ship.pos.x / FIELD_SCREEN_RATIO;

   for ( i=0 ; i<LASER_MAX ; i++ ) {
      if ( laser[i].cnt == NOT_EXIST ) continue;
      //Changed by Albert because of strange crashes...
      //    if ( laser[i].cnt != NOT_EXIST )
      {
         ls = &(laser[i]);
         y = -(float)(ship.pos.y+ls->y) / FIELD_SCREEN_RATIO;
         if ( ls->cnt > 1 )       t = 1;
         else if ( ls->cnt == 1 ) t = 0;
         else                     t = 2;
//         drawLaser(x, y, (float)laserWidth/FIELD_SCREEN_RATIO, LASER_SCREEN_HEIGHT,
//               ls->color, 
//               (ls->color+LASER_COLOR_SPEED)&255,
//               (ls->color+LASER_COLOR_SPEED*2)&255,  
//               (ls->color+LASER_COLOR_SPEED*3)&255,
//               laserCnt, t);
         drawLaser(x, y, (float)laserWidth/FIELD_SCREEN_RATIO, LASER_SCREEN_HEIGHT,
               ls->color, laserCnt, t);
      }
   }

   finishDrawLaser ();
#endif //FIXEDMATH
}
