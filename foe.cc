/*
 * $Id: foe.cc,v 1.6 2003/08/15 07:06:51 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * Handle batteries/bullets.
 *
 * @version $Revision: 1.6 $
 */
#include "barragemanager.h"
#include "foe.h"

extern "C"
{
#include "SDL.h"
#include <stdlib.h>
#include <stdio.h>

#include "rr.h"
#include "genmcr.h"
#include "screen.h"
#include "vector.h"
#include "degutil.h"
#include "soundmanager.h"
#include "attractmanager.h"
#include "brgmng_mtd.h"
#include "ship.h"
#include "frag.h"
#include "shot.h"
}

#define FOE_MAX 1024

static Foe foe[FOE_MAX];

void
removeFoeCommand (Foe * fe)
{
   if (fe->spc == BATTERY)
      return;
   fe->spc = NOT_EXIST_TMP;
}

void
removeFoeForced (Foe * fe)
{
   fe->spc = NOT_EXIST;
   if (fe->cmd) {
      delete fe->cmd;
      fe->cmd = NULL;
   }
}

void
removeFoe (Foe * fe)
{
   if (fe->spc == BATTERY)
      return;
   removeFoeForced (fe);
}

void
initFoes ()
{
   int i;
   for (i = 0; i < FOE_MAX; i++) {
      removeFoeForced (&(foe[i]));
   }
}

void
closeFoes ()
{
   int i;
   for (i = 0; i < FOE_MAX; i++) {
      if (foe[i].cmd)
         delete foe[i].cmd;
   }
}

static int foeIdx = FOE_MAX;

static Foe *
getNextFoe ()
{
   int i;
   for (i = 0; i < FOE_MAX; i++) {
      foeIdx--;
      if (foeIdx < 0)
         foeIdx = FOE_MAX - 1;
      if (foe[foeIdx].spc == NOT_EXIST)
         break;
   }
   if (i >= FOE_MAX)
      return NULL;
   return &(foe[foeIdx]);
}

static Foe *
addFoe (Foe * foe, int d, int spd, int color)
{
   Foe *fe;
   if (foe->limiter->on && foe->spc == BATTERY)
      return NULL;
   fe = getNextFoe ();
   if (!fe)
      return NULL;
   *fe = *foe;
   fe->spos = fe->ppos = fe->pos;
   fe->vel.x = fe->vel.y = 0;
   fe->mv.x = fe->mv.y = 0;
   fe->cnt = 0;
   fe->slowMvCnt = 0;
   fe->d = d;
   fe->spd = spd;
   fe->grzRng = 0;
   fe->color = color;
   fe->shapeType = 2;
   if (mode == IKA_MODE) {
      switch (foe->ikaType) {
      case IKA_FIX:
      case IKA_ALTERNATE:
         fe->color--;
         break;
      case IKA_HALF:
         if (fe->fireCnt & 1)
            fe->color--;
         break;
      case IKA_ALTERNATE_SHOT:
         break;
      }
      fe->color &= 1;
   }
   fe->limiter->cnt++;
   fe->spc = BULLET;
   return fe;
}

//senquack TODO: make sure conversion to floats from doubles here didn't mess up the bullet patterns, etc:
//senquack - complete conversion to floats/fixeds
//Foe* addFoeBattery(int x, int y, double rank, int d, int spd, int xReverse, 
//       BulletMLParser *morphParser[], int morphCnt, int morphHalf, double morphRank,
//       double speedRank,
//       int color, int bulletShape[], float bulletSize[],
//       struct limiter *limiter,
//       int ikaType,
//       BulletMLParser *parser) {
//  Foe foe;
//  int i;
//  foe.pos.x = x; foe.pos.y = y;
//  foe.rank = rank;
//  foe.xReverse = xReverse;
//  for ( i=0 ; i<MORPH_PATTERN_MAX ; i++ ) {
//    foe.morphParser[i] = morphParser[i];
//  }
//  foe.morphCnt = morphCnt; foe.morphHalf = morphHalf; foe.morphRank = morphRank;
//  foe.speedRank = speedRank;
//  for ( i=0 ; i<BULLET_TYPE_NUM ; i++ ) {
//    foe.bulletShape[i] = bulletShape[i]; foe.bulletSize[i] = bulletSize[i];
//  }
//  foe.limiter = limiter;
//  foe.ikaType = ikaType;
//  foe.cntTotal = 0;
//  Foe *fe = addFoe(&foe, d, spd, color);
//  if ( !fe ) return NULL;
//  fe->cmd = new FoeCommand(parser, fe);
//  fe->spc = BATTERY;
//  fe->parser = parser;
//  fe->fireCnt = randN(2);
//  return fe;
//}
#ifdef FIXEDMATH
Foe* addFoeBattery (int x, int y, GLfixed frank, int d, int spd, int xReverse,
               BulletMLParser * morphParser[], int morphCnt, int morphHalf,
               GLfixed fmorphRank, GLfixed fspeedRank, int color,
               int bulletShape[], GLfixed fbulletSize[],
               struct limiter * limiter, int ikaType, BulletMLParser * parser)
{
   Foe foe;
   int i;
   foe.pos.x = x;
   foe.pos.y = y;
//  foe.rank = rank;
   foe.frank = frank;
   foe.xReverse = xReverse;
   for (i = 0; i < MORPH_PATTERN_MAX; i++) {
      foe.morphParser[i] = morphParser[i];
   }
//  foe.morphCnt = morphCnt; foe.morphHalf = morphHalf; foe.morphRank = morphRank;
   foe.morphCnt = morphCnt;
   foe.morphHalf = morphHalf;
   foe.fmorphRank = fmorphRank;
//  foe.speedRank = speedRank;
   foe.fspeedRank = fspeedRank;
   for (i = 0; i < BULLET_TYPE_NUM; i++) {
//    foe.bulletShape[i] = bulletShape[i]; foe.bulletSize[i] = bulletSize[i];
      foe.bulletShape[i] = bulletShape[i]; foe.fbulletSize[i] = fbulletSize[i];
   }
   foe.limiter = limiter;
   foe.ikaType = ikaType;
   foe.cntTotal = 0;
   Foe *fe = addFoe (&foe, d, spd, color);
   if (!fe)
      return NULL;
   fe->cmd = new FoeCommand (parser, fe);
   fe->spc = BATTERY;
   fe->parser = parser;
   fe->fireCnt = randN (2);
   return fe;
}
#else
Foe* addFoeBattery (int x, int y, float rank, int d, int spd, int xReverse,
               BulletMLParser * morphParser[], int morphCnt, int morphHalf,
               float morphRank, float speedRank, int color,
               int bulletShape[], float bulletSize[],
               struct limiter * limiter, int ikaType, BulletMLParser * parser)
{
   Foe foe;
   int i;
   foe.pos.x = x;
   foe.pos.y = y;
   foe.rank = rank;
   foe.xReverse = xReverse;
   for (i = 0; i < MORPH_PATTERN_MAX; i++) {
      foe.morphParser[i] = morphParser[i];
   }
   foe.morphCnt = morphCnt; foe.morphHalf = morphHalf; foe.morphRank = morphRank;
   foe.speedRank = speedRank;
   for (i = 0; i < BULLET_TYPE_NUM; i++) {
      foe.bulletShape[i] = bulletShape[i]; foe.bulletSize[i] = bulletSize[i];
   }
   foe.limiter = limiter;
   foe.ikaType = ikaType;
   foe.cntTotal = 0;
   Foe *fe = addFoe (&foe, d, spd, color);
   if (!fe)
      return NULL;
   fe->cmd = new FoeCommand (parser, fe);
   fe->spc = BATTERY;
   fe->parser = parser;
   fe->fireCnt = randN (2);
   return fe;
}
#endif //FIXEDMATH

void
addFoeActiveBullet (Foe * foe, int d, int spd, int color,
                    BulletMLState * state)
{
   Foe *fe;
   fe = addFoe (foe, d, spd, color);
   if (!fe)
      return;
   fe->cmd = new FoeCommand (state, fe);
   fe->spc = ACTIVE_BULLET;
   if (fe->morphCnt > 0)
      fe->shapeType = 0;
   else
      fe->shapeType = 1;
}

//senquack - conversion to fixed point:
//void addFoeNormalBullet(Foe *foe, int d, int spd, int color) {
//  Foe *fe;
//  if ( foe->morphCnt > 0 && (!foe->morphHalf || (foe->fireCnt&1)) ) {
//    fe = addFoe(foe, d, spd, color);
//    if ( !fe ) return;
//    fe->morphCnt--;
//    fe->morphRank *= 0.77;
//    fe->cmd = new FoeCommand(foe->morphParser[fe->morphCnt&(MORPH_PATTERN_MAX-1)], fe);
//
//    fe->spc = ACTIVE_BULLET;
//    if ( fe->morphCnt > 0 ) fe->shapeType = 0;
//    else                    fe->shapeType = 1;
//    return;
//  }
//  fe = addFoe(foe, d, spd, color);
//  if ( !fe ) return;
//  fe->cmd = NULL;
//  fe->spc = BULLET;
//  fe->shapeType = 2;
//}
void
addFoeNormalBullet (Foe * foe, int d, int spd, int color)
{
   Foe *fe;
   if (foe->morphCnt > 0 && (!foe->morphHalf || (foe->fireCnt & 1))) {
      fe = addFoe (foe, d, spd, color);
      if (!fe)
         return;
      fe->morphCnt--;
#ifdef FIXEDMATH
      fe->fmorphRank = FMUL (fe->fmorphRank, 50463);
#else
      fe->morphRank *= 0.77;
#endif //FIXEDMATH
      fe->cmd =
         new
         FoeCommand (foe->morphParser[fe->morphCnt & (MORPH_PATTERN_MAX - 1)],
                     fe);

      fe->spc = ACTIVE_BULLET;
      if (fe->morphCnt > 0)
         fe->shapeType = 0;
      else
         fe->shapeType = 1;
      return;
   }
   fe = addFoe (foe, d, spd, color);
   if (!fe)
      return;
   fe->cmd = NULL;
   fe->spc = BULLET;
   fe->shapeType = 2;
}

//senquack - converting sizes to fixed point to speed up rendering:
//static void clearFoeShape(Foe *fe, int shape) {
//  float x, y;
//  int d;
//  if ( fe->spc == BATTERY ) return;
//  x =  (float)fe->pos.x / FIELD_SCREEN_RATIO;
//  y = -(float)fe->pos.y / FIELD_SCREEN_RATIO;
//  d = (fe->d * fe->xReverse)&1023;
//  if ( shape ) {
//    addShapeFrag(x, y, fe->bulletSize[fe->shapeType], d, fe->cnt, 
//     fe->bulletShape[fe->shapeType], fe->mv.x, fe->mv.y);
//  } else {
//    addShapeFrag(x, y, fe->bulletSize[fe->shapeType], d, fe->cnt, 
//     -1, fe->mv.x, fe->mv.y);
//  }
//}
static void
clearFoeShape (Foe * fe, int shape)
{
#ifdef FIXEDMATH
//  float x, y;
   GLfixed x, y;
   int d;
   if (fe->spc == BATTERY)
      return;
//senquack TODO: poss. optimization w/ inverse:
//  x =  (float)fe->pos.x / FIELD_SCREEN_RATIO;
//  y = -(float)fe->pos.y / FIELD_SCREEN_RATIO;
   x = f2x ((float) fe->pos.x / FIELD_SCREEN_RATIO);
   y = -f2x ((float) fe->pos.y / FIELD_SCREEN_RATIO);
   d = (fe->d * fe->xReverse) & 1023;
   if (shape) {
      addShapeFragx (x, y, fe->fbulletSize[fe->shapeType], d, fe->cnt,
                     fe->bulletShape[fe->shapeType], fe->mv.x, fe->mv.y);
   } else {
      addShapeFragx (x, y, fe->fbulletSize[fe->shapeType], d, fe->cnt,
                     -1, fe->mv.x, fe->mv.y);
   }
#else
   float x, y;
   int d;
   if ( fe->spc == BATTERY ) return;
//senquack TODO: poss. optimization w/ inverse:
   x =  (float)fe->pos.x / FIELD_SCREEN_RATIO;
   y = -(float)fe->pos.y / FIELD_SCREEN_RATIO;
   d = (fe->d * fe->xReverse)&1023;
   if ( shape ) {
      addShapeFrag(x, y, fe->bulletSize[fe->shapeType], d, fe->cnt, 
            fe->bulletShape[fe->shapeType], fe->mv.x, fe->mv.y);
   } else {
      addShapeFrag(x, y, fe->bulletSize[fe->shapeType], d, fe->cnt, 
            -1, fe->mv.x, fe->mv.y);
   }
#endif //FIXEDMATH
}

#define SHIP_HIT_WIDTH 300*300
#define SLOW_MOVE 64
#define SLOW_MOVE_VANISH_CNT 60
#define LIMITER_VANISH_CNT 8
#define GRZ_METER_UP_ROLLING 12
#define GRZ_METER_UP 10

#define DEFAULT_SPEED_DOWN_BULLETS_NUM 180

int processSpeedDownBulletsNum = DEFAULT_SPEED_DOWN_BULLETS_NUM;
int nowait = 0;

//senquack - conversion to fixed point
// This is the original function, of which a lot of lines were commented out in the original source.. (wasn't me)
//void moveFoes() {
//  int i;
//  Foe *fe;
//  int foeNum = 0;
//  int mx, my;
//  Vector bmv, sofs;
//  int hd, inab, inaa;
//  int sd, sdx, sdy;
//  Vector* bossPos = getBossPos();
//
//  for ( i=0 ; i<FOE_MAX ; i++ ) {
//    if ( foe[i].spc == NOT_EXIST ) continue;
//    fe = &(foe[i]);
//    if ( fe->cmd ) {
//      if ( fe->spc == BATTERY ) {
// if ( fe->cmd->isEnd() ) {
//   delete fe->cmd;
//   fe->cmd = new FoeCommand(fe->parser, fe);
//   if ( mode == IKA_MODE ) {
//     if ( fe->ikaType == IKA_ALTERNATE || fe->ikaType == IKA_ALTERNATE_SHOT ) {
//       fe->color ^= 1;
//     }
//   }
//   //fe->cmd->reset();
// }
//      }
//      fe->cmd->run();
//      if ( fe->spc == NOT_EXIST_TMP ) {
//     removeFoeForced(fe);
//     continue;
//      }
//      if ( fe->cntTotal < LIMITER_VANISH_CNT &&
//    (fe->spc == ACTIVE_BULLET || fe->spc == BULLET ) && fe->limiter->on ) {
//      clearFoeShape(fe, 0);
//      removeFoe(fe);
//      continue;
//      }
//    }
//    fe->cnt++; fe->cntTotal++;
//    fe->ppos = fe->pos;
//    mx = (int)(( ((sctbl[fe->d]    *fe->spd)>>8) + fe->vel.x) * fe->speedRank);
//    my = (int)((-((sctbl[fe->d+256]*fe->spd)>>8) + fe->vel.y) * fe->speedRank);
//    fe->mv.x = (mx*fe->xReverse);
//    fe->mv.y = my;
//    fe->pos.x += fe->mv.x;
//    fe->pos.y += my;
//    if ( fe->spc != BATTERY ) {
//      if ( absN(mx) + absN(my) < SLOW_MOVE ) {
// fe->slowMvCnt++;
// if ( fe->slowMvCnt > SLOW_MOVE_VANISH_CNT ) {
//   removeFoe(fe);
//   continue;
// }
//      } else {
// fe->slowMvCnt = 0;
//      }
//    }
//
//    if ( fe->spc != BATTERY && status == IN_GAME ) {
//      // Check if the bullet hits the ship.
//      bmv = fe->pos;
//      //vctSub(&bmv, &(fe->ppos));
//      bmv.x -= fe->ppos.x;
//      bmv.y -= fe->ppos.y;
//      bmv.x >>= 2; bmv.y >>= 2; 
//      //inaa = vctInnerProduct(&bmv, &bmv);
//      inaa = bmv.x*bmv.x + bmv.y*bmv.y;
//      //if ( inaa > 1.0f ) {
//      if ( inaa > 1 ) {
// sofs = ship.pos;
// //vctSub(&sofs, &(fe->ppos));
// sofs.x -= fe->ppos.x; sofs.y -= fe->ppos.y;
// sofs.x >>= 2; sofs.y >>= 2;
// //inab = vctInnerProduct(&bmv, &sofs);
// inab = bmv.x*sofs.x + bmv.y*sofs.y;
// //ht =  inab / inaa;
// //if ( ht > 0.0f && ht < 1.0f ) {
// if ( inab > 0 && inab < inaa ) {
//   //hd = vctInnerProduct(&sofs, &sofs) - inab*inab/inaa/inaa;
//   hd = sofs.x*sofs.x+sofs.y*sofs.y - inab*inab/inaa/inaa;
//   //if ( hd >= 0 && hd < SHIP_HIT_WIDTH && vctSize(&bmv) < 1280 ) {
//   if ( hd >= 0 && hd < SHIP_HIT_WIDTH ) {
//     destroyShip();
//     removeFoe(fe);
//     continue;
//   }
// }
//      }
//      switch ( mode ) {
//      case PSY_MODE:
// if ( ship.invCnt <= 0 ) {
//   sdx = fe->pos.x - ship.pos.x; sdy =  fe->pos.y - ship.pos.y;
//   sd = getDistance(sdx, sdy);
//   if ( fe->grzRng > 0 ) {
//     fe->grzRng--;
//   } else if ( sd < ship.grzWdt ) {
//     fe->grzRng = -1;
//   } else if ( fe->grzRng == -1 ) {
//     // Ship grazes a bullet.
//     addGrazeFrag(ship.pos.x, ship.pos.y, sdx, sdy);
//     if ( ship.rollingCnt > 0 ) {
//       ship.grzCnt += GRZ_METER_UP_ROLLING;
//     } else {
//       ship.grzCnt += GRZ_METER_UP;
//     }
//     ship.grzf = 1;
//     fe->grzRng = 24;
//     addScore(50);
//   }
//   break;
// }
//      case IKA_MODE:
// sdx = fe->pos.x - ship.pos.x; sdy =  fe->pos.y - ship.pos.y;
// sd = getDistance(sdx, sdy);
// if ( sd < ship.fldWdt && fe->color == ship.color ) {
//   // Ship absorbs a bullet that has same color.
//   addScore(100);
//   ship.absEng++;
//   removeFoe(fe);
//   continue;
// }
// break;
//      case GW_MODE:
// if ( ship.rfCnt > 0 ) {
//   sdx = fe->pos.x - ship.pos.x; sdy =  fe->pos.y - ship.pos.y;
//   sd = getDistance(sdx, sdy);
//   if ( sd < ship.rfWdt ) {
//     // Ship reflects a bullet.
//     addScore(100);
//     ship.reflects = 1;
//     addShot(fe->pos.x, fe->pos.y, bossPos->x-fe->pos.x, bossPos->y-fe->pos.y, 2);
//     removeFoe(fe);
//     continue;
//   }
// }
// break;
//      }
//    }
//    if ( fe->ppos.x < -FIELD_WIDTH_8/2  || fe->ppos.x >= FIELD_WIDTH_8/2 ||
//  fe->ppos.y < -FIELD_HEIGHT_8/2 || fe->ppos.y >= FIELD_HEIGHT_8/2 ) {
//      removeFoe(fe);
//      continue;
//    }
//    foeNum++;
//  }
//
//  // A game speed becomes slow as many bullets appears.
//  interval = INTERVAL_BASE;
//  if ( !nowait && foeNum > processSpeedDownBulletsNum ) {
//    interval += (foeNum-processSpeedDownBulletsNum) * INTERVAL_BASE / 
//      processSpeedDownBulletsNum;
//    if ( interval > INTERVAL_BASE*2 ) interval = INTERVAL_BASE*2;
//  }
//}
void
moveFoes ()
{
   int i;
   Foe *fe;
   int foeNum = 0;
   int mx, my;
   Vector bmv, sofs;
   int hd, inab, inaa;
   int sd, sdx, sdy;
   Vector *bossPos = getBossPos ();

   for (i = 0; i < FOE_MAX; i++) {
      if (foe[i].spc == NOT_EXIST)
         continue;
      fe = &(foe[i]);
      if (fe->cmd) {
         if (fe->spc == BATTERY) {
            if (fe->cmd->isEnd ()) {
               delete fe->cmd;
               fe->cmd = new FoeCommand (fe->parser, fe);
               if (mode == IKA_MODE) {
                  if (fe->ikaType == IKA_ALTERNATE
                      || fe->ikaType == IKA_ALTERNATE_SHOT) {
                     fe->color ^= 1;
                  }
               }
               //fe->cmd->reset();
            }
         }
         fe->cmd->run ();
         if (fe->spc == NOT_EXIST_TMP) {
            removeFoeForced (fe);
            continue;
         }
         if (fe->cntTotal < LIMITER_VANISH_CNT &&
             (fe->spc == ACTIVE_BULLET || fe->spc == BULLET)
             && fe->limiter->on) {
            clearFoeShape (fe, 0);
            removeFoe (fe);
            continue;
         }
      }
      fe->cnt++;
      fe->cntTotal++;
      fe->ppos = fe->pos;
#ifdef FIXEDMATH
      mx = FNUM2INT (FMUL (INT2FNUM ((sctbl[fe->d] * fe->spd >> 8) + fe->vel.x), fe->fspeedRank));
      my = FNUM2INT (FMUL (INT2FNUM (-((sctbl[fe->d + 256] * fe->spd) >> 8) + fe->vel.y), fe->fspeedRank));
#else
      mx = (int)(( ((sctbl[fe->d]    *fe->spd)>>8) + fe->vel.x) * fe->speedRank);
      my = (int)((-((sctbl[fe->d+256]*fe->spd)>>8) + fe->vel.y) * fe->speedRank);
#endif //FIXEDMATH
      fe->mv.x = (mx * fe->xReverse);
      fe->mv.y = my;
      fe->pos.x += fe->mv.x;
      fe->pos.y += my;
      if (fe->spc != BATTERY) {
         if (absN (mx) + absN (my) < SLOW_MOVE) {
            fe->slowMvCnt++;
            if (fe->slowMvCnt > SLOW_MOVE_VANISH_CNT) {
               removeFoe (fe);
               continue;
            }
         } else {
            fe->slowMvCnt = 0;
         }
      }

      if (fe->spc != BATTERY && status == IN_GAME) {
         // Check if the bullet hits the ship.
         bmv = fe->pos;
         //vctSub(&bmv, &(fe->ppos));
         bmv.x -= fe->ppos.x;
         bmv.y -= fe->ppos.y;
         bmv.x >>= 2;
         bmv.y >>= 2;
         //inaa = vctInnerProduct(&bmv, &bmv);
         inaa = bmv.x * bmv.x + bmv.y * bmv.y;
         //if ( inaa > 1.0f ) {
         if (inaa > 1) {
            sofs = ship.pos;
            //vctSub(&sofs, &(fe->ppos));
            sofs.x -= fe->ppos.x;
            sofs.y -= fe->ppos.y;
            sofs.x >>= 2;
            sofs.y >>= 2;
            //inab = vctInnerProduct(&bmv, &sofs);
            inab = bmv.x * sofs.x + bmv.y * sofs.y;
            //ht =  inab / inaa;
            //if ( ht > 0.0f && ht < 1.0f ) {
            if (inab > 0 && inab < inaa) {
               //hd = vctInnerProduct(&sofs, &sofs) - inab*inab/inaa/inaa;
               hd =
                  sofs.x * sofs.x + sofs.y * sofs.y -
                  inab * inab / inaa / inaa;
               //if ( hd >= 0 && hd < SHIP_HIT_WIDTH && vctSize(&bmv) < 1280 ) {
               if (hd >= 0 && hd < SHIP_HIT_WIDTH) {
                  destroyShip ();
                  removeFoe (fe);
                  continue;
               }
            }
         }
         switch (mode) {
         case PSY_MODE:
            if (ship.invCnt <= 0) {
               sdx = fe->pos.x - ship.pos.x;
               sdy = fe->pos.y - ship.pos.y;
               sd = getDistance (sdx, sdy);
               if (fe->grzRng > 0) {
                  fe->grzRng--;
               } else if (sd < ship.grzWdt) {
                  fe->grzRng = -1;
               } else if (fe->grzRng == -1) {
                  // Ship grazes a bullet.
                  addGrazeFrag (ship.pos.x, ship.pos.y, sdx, sdy);
                  if (ship.rollingCnt > 0) {
                     ship.grzCnt += GRZ_METER_UP_ROLLING;
                  } else {
                     ship.grzCnt += GRZ_METER_UP;
                  }
                  ship.grzf = 1;
                  fe->grzRng = 24;
                  addScore (50);
               }
               break;
            }
         case IKA_MODE:
            sdx = fe->pos.x - ship.pos.x;
            sdy = fe->pos.y - ship.pos.y;
            sd = getDistance (sdx, sdy);
            if (sd < ship.fldWdt && fe->color == ship.color) {
               // Ship absorbs a bullet that has same color.
               addScore (100);
               ship.absEng++;
               removeFoe (fe);
               continue;
            }
            break;
         case GW_MODE:
            if (ship.rfCnt > 0) {
               sdx = fe->pos.x - ship.pos.x;
               sdy = fe->pos.y - ship.pos.y;
               sd = getDistance (sdx, sdy);
               if (sd < ship.rfWdt) {
                  // Ship reflects a bullet.
                  addScore (100);
                  ship.reflects = 1;
                  addShot (fe->pos.x, fe->pos.y, bossPos->x - fe->pos.x,
                           bossPos->y - fe->pos.y, 2);
                  removeFoe (fe);
                  continue;
               }
            }
            break;
         }
      }
      if (fe->ppos.x < -FIELD_WIDTH_8 / 2 || fe->ppos.x >= FIELD_WIDTH_8 / 2
          || fe->ppos.y < -FIELD_HEIGHT_8 / 2
          || fe->ppos.y >= FIELD_HEIGHT_8 / 2) {
         removeFoe (fe);
         continue;
      }
      foeNum++;
   }

   // A game speed becomes slow as many bullets appears.
   interval = INTERVAL_BASE;
   if (!nowait && foeNum > processSpeedDownBulletsNum) {
      interval += (foeNum - processSpeedDownBulletsNum) * INTERVAL_BASE /
         processSpeedDownBulletsNum;
      if (interval > INTERVAL_BASE * 2)
         interval = INTERVAL_BASE * 2;
   }
}

void
clearFoes ()
{
   int i;
   Foe *fe;
   for (i = 0; i < FOE_MAX; i++) {
      if (foe[i].spc == NOT_EXIST)
         continue;
      fe = &(foe[i]);
      clearFoeShape (fe, 0);
      removeFoeForced (fe);
   }
}

void
clearFoesZako ()
{
   int i;
   Foe *fe;
   for (i = 0; i < FOE_MAX; i++) {
      if (foe[i].spc == NOT_EXIST || foe[i].spc == BATTERY)
         continue;
      fe = &(foe[i]);
      clearFoeShape (fe, 0);
      removeFoeForced (fe);
   }
}

void
wipeBullets (Vector * pos, int width)
{
   int i;
   Foe *fe;
   for (i = 0; i < FOE_MAX; i++) {
      if (foe[i].spc == NOT_EXIST || foe[i].spc == BATTERY)
         continue;
      fe = &(foe[i]);
      if (vctDist (pos, &(fe->pos)) < width) {
         clearFoeShape (fe, 1);
         addScore (10);
         removeFoeForced (fe);
      }
   }
}

//senquack - fixed point conversion:
//void drawBulletsWake() {
//  int i;
//  Foe *fe;
//  float x, y, sx, sy;
//  for ( i=0 ; i<FOE_MAX ; i++ ) {
//    //if ( foe[i].spc == NOT_EXIST || foe[i].spc == BATTERY || foe[i].cnt >= 64 ) continue;
//    //Changed the above line to the logic below, because it causes hangs -- Albert
//    //(I know it sounds like gibberish, but it's true...)
//    if ( foe[i].spc != NOT_EXIST && foe[i].spc != BATTERY && foe[i].cnt < 64 )
//    {
//        fe = &(foe[i]);
//        x =  (float)fe->pos.x / FIELD_SCREEN_RATIO;
//        y = -(float)fe->pos.y / FIELD_SCREEN_RATIO;
//        sx =  (float)fe->spos.x / FIELD_SCREEN_RATIO;
//        sy = -(float)fe->spos.y / FIELD_SCREEN_RATIO;
//        drawLine(x, y, 0, sx, sy, 0, 150, 180, 90, (63-fe->cnt)*3);        
//    }
//  }
//}
void
drawBulletsWake ()
{
   int i;
   Foe *fe;
#ifdef FIXEDMATH
   GLfixed x, y, sx, sy;
#else
   float x, y, sx, sy;
#endif //FIXEDMATH
   for (i = 0; i < FOE_MAX; i++) {
      //if ( foe[i].spc == NOT_EXIST || foe[i].spc == BATTERY || foe[i].cnt >= 64 ) continue;
      //Changed the above line to the logic below, because it causes hangs -- Albert
      //(I know it sounds like gibberish, but it's true...)
      if (foe[i].spc != NOT_EXIST && foe[i].spc != BATTERY && foe[i].cnt < 64) {
         fe = &(foe[i]);
//senquack TODO: poss. optimization w/ inverse:
#ifdef FIXEDMATH
         x = f2x ((float) fe->pos.x / FIELD_SCREEN_RATIO);
         y = f2x (-(float) fe->pos.y / FIELD_SCREEN_RATIO);
         sx = f2x ((float) fe->spos.x / FIELD_SCREEN_RATIO);
         sy = f2x (-(float) fe->spos.y / FIELD_SCREEN_RATIO);
         drawLinex (x, y, sx, sy, 150, 180, 90, (63 - fe->cnt) * 3);
#else
         x =  (float)fe->pos.x / FIELD_SCREEN_RATIO;
         y = -(float)fe->pos.y / FIELD_SCREEN_RATIO;
         sx =  (float)fe->spos.x / FIELD_SCREEN_RATIO;
         sy = -(float)fe->spos.y / FIELD_SCREEN_RATIO;
         drawLine(x, y, 0, sx, sy, 0, 150, 180, 90, (63-fe->cnt)*3);        
#endif //FIXEDMATH
      }
   }
}

static int bulletColor[BULLET_COLOR_NUM][3] = {
   {180, 100, 50}, {100, 100, 140}, {150, 100, 120}, {100, 120, 150},
};

//senquack - NEW further diagnosing for IKA mode:
//senquack - changing this to a stub does not help freezes
//void drawBullets() {
//  int i;
//  Foe *fe;
//  float x, y;
//  int bc;
//  int d, bt;
//  for ( i=0 ; i<FOE_MAX ; i++ ) {
//    //if ( foe[i].spc == NOT_EXIST || foe[i].spc == BATTERY ) continue;
//    //Changed by Albert
//    if (foe[i].spc != NOT_EXIST && foe[i].spc != BATTERY)
//    {
//        fe = &(foe[i]);
//        x =  (float)fe->pos.x / FIELD_SCREEN_RATIO;
//        y = -(float)fe->pos.y / FIELD_SCREEN_RATIO;
//        d = 1023 - getDeg(fe->pos.x - fe->ppos.x, fe->pos.y - fe->ppos.y);
//        bt = fe->shapeType;
//        if ( mode == IKA_MODE ) {
//          drawShapeIka(x, y, fe->bulletSize[bt], d, fe->cnt&1, fe->bulletShape[bt], fe->color);
//        } else {
//          bc = fe->color%BULLET_COLOR_NUM;
//          drawShape(x, y, fe->bulletSize[bt], d, fe->cnt, fe->bulletShape[bt],
//          bulletColor[bc][0], bulletColor[bc][1], bulletColor[bc][2]); 
//        }
//    }
//  }
//}
//void drawBullets() {
// //senquack - new function called once before a series of calls to drawShape (for openglES speedup)
// prepareDrawShapes();
//
//  int i;
//  Foe *fe;
//  float x, y;
//  int bc;
//  int d, bt;
//  for ( i=0 ; i<FOE_MAX ; i++ ) {
//    //if ( foe[i].spc == NOT_EXIST || foe[i].spc == BATTERY ) continue;
//    //Changed by Albert
//    if (foe[i].spc != NOT_EXIST && foe[i].spc != BATTERY)
//    {
//        fe = &(foe[i]);
//      printf("fe->pos.x = %d   fe->pos.y = %d\n", fe->pos.x, fe->pos.y);
//        x =  (float)fe->pos.x / FIELD_SCREEN_RATIO;
//        y = -(float)fe->pos.y / FIELD_SCREEN_RATIO;
//        d = 1023 - getDeg(fe->pos.x - fe->ppos.x, fe->pos.y - fe->ppos.y);
//        bt = fe->shapeType;
//        if ( mode == IKA_MODE ) {
////          drawShapeIka(x, y, fe->bulletSize[bt], d, fe->cnt&1, fe->bulletShape[bt], fe->color);
//          drawShapeIka(x, y, fe->fbulletSize[bt], d, fe->cnt&1, fe->bulletShape[bt], fe->color);
//        } else {
//          bc = fe->color%BULLET_COLOR_NUM;
////          drawShape(x, y, fe->bulletSize[bt], d, fe->cnt, fe->bulletShape[bt],
////        bulletColor[bc][0], bulletColor[bc][1], bulletColor[bc][2]); 
//          drawShape(x, y, fe->fbulletSize[bt], d, fe->cnt, fe->bulletShape[bt],
//          bulletColor[bc][0], bulletColor[bc][1], bulletColor[bc][2]); 
//        }
//    }
//  }
//}
void
drawBullets ()
{
   //senquack - new function called once before a series of calls to drawShape (for openglES speedup)
   prepareDrawShapes ();

   int i;
   Foe *fe;
#ifdef FIXEDMATH
   GLfixed fx, fy;
#else
   float x, y;
#endif //FIXEDMATH
   int bc;
   int d, bt;
   for (i = 0; i < FOE_MAX; i++) {
      if (foe[i].spc == NOT_EXIST || foe[i].spc == BATTERY)
         continue;
      {
         fe = &(foe[i]);
#ifdef FIXEDMATH
         fx = (int) ((float) fe->pos.x * 6.5536f);  // roll fixed point conversion and division into one multiply
         fy = (int) (-(float) fe->pos.y * 6.5536f);
#else
//senquack TODO: poss. optimization w/ inverse:
         x =  (float)fe->pos.x / FIELD_SCREEN_RATIO;
         y = -(float)fe->pos.y / FIELD_SCREEN_RATIO;
#endif //FIXEDMATH

         d = 1023 - getDeg (fe->pos.x - fe->ppos.x, fe->pos.y - fe->ppos.y);
         bt = fe->shapeType;
         if (mode == IKA_MODE) {
#ifdef FIXEDMATH
            drawShapeIka(fx, fy, fe->fbulletSize[bt], d, fe->cnt & 1, fe->bulletShape[bt], fe->color);
#else
            drawShapeIka(x, y, fe->bulletSize[bt], d, fe->cnt&1, fe->bulletShape[bt], fe->color);
#endif //FIXEDMATH
         } else {
//          bc = fe->color%BULLET_COLOR_NUM;
            bc = fe->color & 0x3;   // BULLET_COLOR_NUM is 4 (compiler probably already does this though)

#ifdef FIXEDMATH
            drawShapex (fx, fy, fe->fbulletSize[bt], d, fe->cnt, fe->bulletShape[bt], 
                  bulletColor[bc][0], bulletColor[bc][1], bulletColor[bc][2]);
#else
            drawShape(x, y, fe->bulletSize[bt], d, fe->cnt, fe->bulletShape[bt],
                  bulletColor[bc][0], bulletColor[bc][1], bulletColor[bc][2]); 
#endif //FIXEDMATH
         }
      }
   }
   finishDrawShapes ();
}
