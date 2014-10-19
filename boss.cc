/*
 * $Id: boss.cc,v 1.5 2003/08/10 03:21:28 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * Boss.
 *
 * @version $Revision: 1.5 $
 */
#include <math.h>

extern "C"
{
#include "genmcr.h"
#include "degutil.h"
#include "ship.h"
#include "screen.h"
#include "frag.h"
#include "soundmanager.h"
#include "letterrender.h"
#include "attractmanager.h"
#include "rr.h"
}
#include "boss.h"
#include "foe.h"
#include "barragemanager.h"

static Boss boss;
static BossShape bossShape;
int bossTimer;

#define BOSS_INITIAL_X 0
#define BOSS_INITIAL_Y (-FIELD_HEIGHT_8/3)

void
initBoss ()
{
   boss.x = BOSS_INITIAL_X;
   boss.y = BOSS_INITIAL_Y;
   boss.d = 0;
   boss.cnt = 0;
}

static Vector bossPos;

Vector *
getBossPos ()
{
   bossPos.x = boss.x;
   bossPos.y = boss.y;
   return &bossPos;
}

static void
setBatteryGroupPos (BatteryGroup * right, BatteryGroup * left, int cx, int cy)
{
   int r, i, x, y, bn;
   int d, ox, oy;
   bn = right->batteryNum;
   if (randN (4) == 0) {
      // Set in the circle shape.
      r = randN (5000) + 5000;
      d = randN (1024);
      for (i = 0; i < bn; i++, d += 1024 / bn) {
         d &= 1023;
         x = cx + ((sctbl[d] * r) >> 8);
         y = cy + ((sctbl[d + 256] * r) >> 8);
         right->battery[i].x = x;
         right->battery[i].y = y;
         left->battery[i].x = -x;
         left->battery[i].y = y;
      }
   } else {
      // Set in the line shape.
      r = randN (4000) + 4000;
      d = randN (1024);
      ox = ((sctbl[d] * r) >> 8);
      oy = ((sctbl[d + 256] * r) >> 8);
      x = cx - (bn - 1) * ox / 2;
      y = cy - (bn - 1) * oy / 2;
      for (i = 0; i < bn; i++) {
         x += ox;
         y += oy;
         right->battery[i].x = x;
         right->battery[i].y = y;
         left->battery[i].x = -x;
         left->battery[i].y = y;
      }
   }
}

static float baseSize[] = { 0.4f, 0.36f, 0.3f };

//senquack TODO: figure out why I left this commented-out in Wiz version, and if I should re-enabled it
////senquack - converting to fixed point:
//static GLfixed fbaseSize[] = {26214, 23593, 19661};

static int shapePtn[3][8] =
   { {3, 2, 4, 6}, {5, 0, 2, 3, 4, 5}, {4, 0, 1, 4, 5} };

//senquack - partial conversion to fixed point to speed up rendering
//static void setBatteryShape(BatteryShape *shape) {
//  int i;
//  if ( mode == IKA_MODE ) {
//    shape->color = randN(2);
//  } else {
//    shape->color = randN(BULLET_COLOR_NUM);
//  }
//  for ( i=0 ; i<BULLET_TYPE_NUM ; i++ ) {
//    if ( mode == IKA_MODE ) {
//      shape->bulletShape[i] = randN(2);
//      shape->bulletSize[i] = baseSize[i] * ((float)(256+randN(72))/256.0f);
//    } else {
//      shape->bulletShape[i] = shapePtn[i][randN(shapePtn[i][0]+1)];
//      shape->bulletSize[i] = baseSize[i] * ((float)(224+randN(96))/256.0f);
//    }
//  }
//}
static void
setBatteryShape (BatteryShape * shape)
{
   int i;
   if (mode == IKA_MODE) {
      shape->color = randN (2);
   } else {
      shape->color = randN (BULLET_COLOR_NUM);
   }
   for (i = 0; i < BULLET_TYPE_NUM; i++) {
      if (mode == IKA_MODE) {
         shape->bulletShape[i] = randN (2);
#ifdef FIXEDMATH
         shape->fbulletSize[i] = f2x (baseSize[i] * ((float) (256 + randN (72)) / 256.0f));
#else
         shape->bulletSize[i] = baseSize[i] * ((float)(256+randN(72))/256.0f);
#endif //FIXEDMATH
      } else {
         shape->bulletShape[i] = shapePtn[i][randN (shapePtn[i][0] + 1)];
#ifdef FIXEDMATH
         shape->fbulletSize[i] = f2x (baseSize[i] * ((float) (224 + randN (96)) / 256.0f));
#else
         shape->bulletSize[i] = baseSize[i] * ((float)(224+randN(96))/256.0f);
#endif //FIXEDMATH
      }
   }
}

static void
setAttackIndex (Attack * at, int center)
{
   int i;
   if (center) {
      switch (randN (3)) {
      case 0:
      case 1:
         at->barrageType = NORMAL_BARRAGE;
         break;
      case 2:
         at->barrageType = SIMPLE_BARRAGE;
         break;
      }
   } else {
      switch (randN (2)) {
      case 0:
         at->barrageType = NORMAL_BARRAGE;
         break;
      case 1:
         at->barrageType = REVERSIBLE_BARRAGE;
         break;
      }
   }
   at->xReverse = randN (2) * 2 - 1;
   if (at->barrageType == REVERSIBLE_BARRAGE && randN (4) == 0) {
      at->xrAlter = 1;
   } else {
      at->xrAlter = 0;
   }
   at->barrageIdx = randN (barragePatternNum[at->barrageType]);
   for (i = 0; i < MORPH_PATTERN_MAX; i++) {
      at->morphIdx[i] = randN (barragePatternNum[MORPH_BARRAGE]);
   }
   if (at->barrageType != SIMPLE_BARRAGE && randN (4) == 0)
      at->morphHalf = 1;
   else
      at->morphHalf = 0;
   if (mode == IKA_MODE) {
      int ir = randN (8), it;
      if (center) {
         if (ir < 4)
            it = IKA_ALTERNATE;
         else if (ir < 7)
            it = IKA_ALTERNATE_SHOT;
         else
            it = IKA_HALF;
      } else {
         if (ir < 3)
            it = IKA_FIX;
         else if (ir < 6)
            it = IKA_ALTERNATE;
         else
            it = IKA_ALTERNATE_SHOT;
      }
      at->ikaType = it;
   }
}

//senquack - complete conversion to floats:
//static void setAttackRank(Attack *at, double rank) {
//senquack - converting to fixed-point:
//static void setAttackRank(Attack *at, float rank) {
//  if ( rank <= 0.3 ) {
//    at->rank = rank;
//    at->morphCnt = 0;
//    at->speedRank = 1;
//  } else {
//    at->rank = rank*(90+randN(38))/256.0;
//    if ( at->rank > 0.8 ) {
//      at->rank = 0.2*(randN(8)+1)/8.0 + 0.8;
//    }
//    rank /= (at->rank+2);
//    if ( mode == IKA_MODE ) {
//      at->speedRank = sqrt(rank)*(randN(80)+256)/256;
//    } else if ( mode == GW_MODE ) {
//      at->speedRank = sqrt(rank)*(randN(92)+236)/256;
//    } else {
//      at->speedRank = sqrt(rank)*(randN(128)+192)/256;
//    }
//    if ( at->speedRank < 0.8 ) at->speedRank = 0.8;
//    at->morphRank = rank / at->speedRank;
//    at->morphCnt = 0;
//    while ( at->morphRank > 1 ) {
//      at->morphCnt++;
//      at->morphRank /= 3;
//    }
//  }
//  at->morphType = -1; 
//  if ( at->barrageType == SIMPLE_BARRAGE ) {
//    if ( at->morphCnt == 0 ) at->morphCnt++;
//    at->morphType = MORPH_HEAVY_BARRAGE;
//    at->morphIdx[(at->morphCnt-1)&(MORPH_PATTERN_MAX-1)] = 
//      randN(barragePatternNum[MORPH_HEAVY_BARRAGE]);
//  }
//  if ( at->morphCnt == 0 ) {
//    switch ( mode ) {
//    case PSY_MODE:
//      at->morphCnt = 1;
//      at->morphType = PSY_MORPH_BARRAGE;
//      at->morphIdx[0] = randN(barragePatternNum[PSY_MORPH_BARRAGE]);
//      break;
//    }
//    at->morphRank = 0.5+randN(6)*0.1;
//  }
//  switch ( mode ) {
//  case NORMAL_MODE:
//    //at->speedRank *= 0.7f;
//    at->morphRank *= 0.7f;
//    at->speedRank *= 0.8f;
//    break;
//  case PSY_MODE:
//    at->speedRank *= 0.72f;
//    break;
//  case IKA_MODE:
//    //at->speedRank *= 0.74f;
//    at->morphRank *= 0.8f;
//    at->speedRank *= 0.77f;
//    break;
//  case GW_MODE:
//    at->speedRank *= 0.8f;
//    break;
//  }
//}
//senquack TODO: make sure converting rank paramter here to float from double didn't mess anything up in patterns:
//senquack TODO ALSO: for gcw0 port, I went ahead and replaced all sqrts below to sqrtf's: (forgot to do that for Wiz)
static void
setAttackRank (Attack * at, float rank)
{
#ifdef FIXEDMATH
   if (rank <= 0.3) {
//    at->rank = rank;
      at->frank = f2x (rank);
      at->morphCnt = 0;
//    at->speedRank = 1;
      at->fspeedRank = INT2FNUM (1);
   } else {
//    at->rank = rank*(90+randN(38))/256.0;
      at->frank = f2x (rank * (90 + randN (38)) / 256.0);
//    if ( at->rank > 0.8 ) {
      if (at->frank > f2x (0.8)) {
//      at->rank = 0.2*(randN(8)+1)/8.0 + 0.8;
         at->frank = f2x (0.2 * (randN (8) + 1) / 8.0 + 0.8);
      }
//    rank /= (at->rank+2);
      rank /= (x2f (at->frank) + 2);
      if (mode == IKA_MODE) {
//      at->speedRank = sqrt(rank)*(randN(80)+256)/256;
         at->fspeedRank = f2x (sqrtf (rank) * (randN (80) + 256) / 256);
      } else if (mode == GW_MODE) {
//      at->speedRank = sqrt(rank)*(randN(92)+236)/256;
         at->fspeedRank = f2x (sqrtf (rank) * (randN (92) + 236) / 256);
      } else {
//      at->speedRank = sqrt(rank)*(randN(128)+192)/256;
         at->fspeedRank = f2x (sqrtf (rank) * (randN (128) + 192) / 256);
      }
//    if ( at->speedRank < 0.8 ) at->speedRank = 0.8;
      if (at->fspeedRank < f2x (0.8))
         at->fspeedRank = f2x (0.8);
//    at->morphRank = rank / at->speedRank;
      at->fmorphRank = f2x (rank / x2f (at->fspeedRank));
      at->morphCnt = 0;
//    while ( at->morphRank > 1 ) {
      while (at->fmorphRank > INT2FNUM (1)) {
         at->morphCnt++;
//      at->morphRank /= 3;
         at->fmorphRank = FDIV (at->fmorphRank, INT2FNUM (3));
      }
   }
   at->morphType = -1;
   if (at->barrageType == SIMPLE_BARRAGE) {
      if (at->morphCnt == 0)
         at->morphCnt++;
      at->morphType = MORPH_HEAVY_BARRAGE;
      at->morphIdx[(at->morphCnt - 1) & (MORPH_PATTERN_MAX - 1)] =
         randN (barragePatternNum[MORPH_HEAVY_BARRAGE]);
   }
   if (at->morphCnt == 0) {
      switch (mode) {
      case PSY_MODE:
         at->morphCnt = 1;
         at->morphType = PSY_MORPH_BARRAGE;
         at->morphIdx[0] = randN (barragePatternNum[PSY_MORPH_BARRAGE]);
         break;
      }
//    at->morphRank = 0.5+randN(6)*0.1;
      at->fmorphRank = f2x (0.5 + randN (6) * 0.1);
   }
   switch (mode) {
   case NORMAL_MODE:
      //senquack - NOTE: this line was commented out in original rRootage source code (it wasn't me)
      //at->speedRank *= 0.7f;  

//    at->morphRank *= 0.7f;
//    at->speedRank *= 0.8f;
      at->fmorphRank = FMUL (at->fmorphRank, f2x (0.7));
      at->fspeedRank = FMUL (at->fspeedRank, f2x (0.8));
      break;
   case PSY_MODE:
//    at->speedRank *= 0.72f;
      at->fspeedRank = FMUL (at->fspeedRank, f2x (0.72));
      break;
   case IKA_MODE:
      //senquack - NOTE: this line was commented out in original rRootage source code (it wasn't me)
      //at->speedRank *= 0.74f;

//    at->morphRank *= 0.8f;
//    at->speedRank *= 0.77f;
      at->fmorphRank = FMUL (at->fmorphRank, f2x (0.8));
      at->fspeedRank = FMUL (at->fspeedRank, f2x (0.77));
      break;
   case GW_MODE:
//    at->speedRank *= 0.8f;
      at->fspeedRank = FMUL (at->fspeedRank, f2x (0.8));
      break;
   }
#else
   if ( rank <= 0.3 ) {
      at->rank = rank;
      at->morphCnt = 0;
      at->speedRank = 1;
   } else {
      at->rank = rank*(90+randN(38))/256.0;
      if ( at->rank > 0.8 ) {
         at->rank = 0.2*(randN(8)+1)/8.0 + 0.8;
      }
      rank /= (at->rank+2);
      if ( mode == IKA_MODE ) {
//         at->speedRank = sqrt(rank)*(randN(80)+256)/256;
         at->speedRank = sqrtf(rank)*(randN(80)+256)/256;
      } else if ( mode == GW_MODE ) {
//         at->speedRank = sqrt(rank)*(randN(92)+236)/256;
         at->speedRank = sqrtf(rank)*(randN(92)+236)/256;
      } else {
//         at->speedRank = sqrt(rank)*(randN(128)+192)/256;
         at->speedRank = sqrtf(rank)*(randN(128)+192)/256;
      }
      if ( at->speedRank < 0.8 ) at->speedRank = 0.8;
      at->morphRank = rank / at->speedRank;
      at->morphCnt = 0;
      while ( at->morphRank > 1 ) {
         at->morphCnt++;
         at->morphRank /= 3;
      }
   }
   at->morphType = -1; 
   if ( at->barrageType == SIMPLE_BARRAGE ) {
      if ( at->morphCnt == 0 ) at->morphCnt++;
      at->morphType = MORPH_HEAVY_BARRAGE;
      at->morphIdx[(at->morphCnt-1)&(MORPH_PATTERN_MAX-1)] = 
         randN(barragePatternNum[MORPH_HEAVY_BARRAGE]);
   }
   if ( at->morphCnt == 0 ) {
      switch ( mode ) {
         case PSY_MODE:
            at->morphCnt = 1;
            at->morphType = PSY_MORPH_BARRAGE;
            at->morphIdx[0] = randN(barragePatternNum[PSY_MORPH_BARRAGE]);
            break;
      }
      at->morphRank = 0.5+randN(6)*0.1;
   }
   switch ( mode ) {
      case NORMAL_MODE:
         //senquack - NOTE: this line was commented out in original rRootage source code (it wasn't me)
         //at->speedRank *= 0.7f;
         at->morphRank *= 0.7f;
         at->speedRank *= 0.8f;
         break;
      case PSY_MODE:
         at->speedRank *= 0.72f;
         break;
      case IKA_MODE:
         //senquack - NOTE: this line was commented out in original rRootage source code (it wasn't me)
         //at->speedRank *= 0.74f;
         at->morphRank *= 0.8f;
         at->speedRank *= 0.77f;
         break;
      case GW_MODE:
         at->speedRank *= 0.8f;
         break;
   }
#endif //FIXEDMATH
}

//senquack - complete conversion to floats:
//senquack TODO: double-check this conversion to float didn't screw up barrages:
//static void setAttack(Attack *at, double rank, int center) {
static void
setAttack (Attack * at, float rank, int center)
{
   setAttackIndex (at, center);
   setAttackRank (at, rank);
}

//senquack - partial conversion to fixed point to speed up rendering
//static void setFoeBattery(Boss *bs, Battery *bt, Attack *at, 
//         BatteryShape *sp, Limiter *lt, int idx) {
//  BulletMLParser *mrp[MORPH_PATTERN_MAX];
//  int i, mi;
//  int xr;
//  if ( at->barrageType == NOT_EXIST ) {
//    bt->foe = NULL;
//    return;
//  }
//  for ( i=0 ; i<MORPH_PATTERN_MAX ; i++ ) {
//    mrp[i] = barragePattern[MORPH_BARRAGE][at->morphIdx[i]].bulletml;
//  }
//  if ( at->morphType >= 0 ) {
//    switch ( at->morphType ) {
//    case PSY_MORPH_BARRAGE:
//      mrp[0] = barragePattern[at->morphType][at->morphIdx[0]].bulletml;
//      break;
//    default:
//      mi = (at->morphCnt-1)&(MORPH_PATTERN_MAX-1);
//      mrp[mi] = barragePattern[at->morphType][at->morphIdx[mi]].bulletml;
//      break;
//    }
//  }

//  xr = at->xReverse;
//  if ( at->xrAlter && (idx&1) == 1 ) xr = -xr;
//  bt->foe = addFoeBattery(bs->x+bt->x, bs->y+bt->y, at->rank, 512, 0, xr,
//         mrp,
//         at->morphCnt, at->morphHalf, at->morphRank, at->speedRank,
//         sp->color, sp->bulletShape, sp->bulletSize,
//         lt,
//         at->ikaType,
//         barragePattern[at->barrageType][at->barrageIdx].bulletml);
//}
//senquack - now, a more complete conversion
//static void setFoeBattery(Boss *bs, Battery *bt, Attack *at, 
//         BatteryShape *sp, Limiter *lt, int idx) {
//  BulletMLParser *mrp[MORPH_PATTERN_MAX];
//  int i, mi;
//  int xr;
//  if ( at->barrageType == NOT_EXIST ) {
//    bt->foe = NULL;
//    return;
//  }
//  for ( i=0 ; i<MORPH_PATTERN_MAX ; i++ ) {
//    mrp[i] = barragePattern[MORPH_BARRAGE][at->morphIdx[i]].bulletml;
//  }
//  if ( at->morphType >= 0 ) {
//    switch ( at->morphType ) {
//    case PSY_MORPH_BARRAGE:
//      mrp[0] = barragePattern[at->morphType][at->morphIdx[0]].bulletml;
//      break;
//    default:
//      mi = (at->morphCnt-1)&(MORPH_PATTERN_MAX-1);
//      mrp[mi] = barragePattern[at->morphType][at->morphIdx[mi]].bulletml;
//      break;
//    }
//  }
//  xr = at->xReverse;
//  if ( at->xrAlter && (idx&1) == 1 ) xr = -xr;
//  bt->foe = addFoeBattery(bs->x+bt->x, bs->y+bt->y, at->rank, 512, 0, xr,
//         mrp,
//         at->morphCnt, at->morphHalf, at->morphRank, at->speedRank,
//         sp->color, sp->bulletShape, sp->fbulletSize,
//         lt,
//         at->ikaType,
//         barragePattern[at->barrageType][at->barrageIdx].bulletml);
//}
static void
setFoeBattery (Boss * bs, Battery * bt, Attack * at,
               BatteryShape * sp, Limiter * lt, int idx)
{
   BulletMLParser *mrp[MORPH_PATTERN_MAX];
   int i, mi;
   int xr;
   if (at->barrageType == NOT_EXIST) {
      bt->foe = NULL;
      return;
   }
   for (i = 0; i < MORPH_PATTERN_MAX; i++) {
      mrp[i] = barragePattern[MORPH_BARRAGE][at->morphIdx[i]].bulletml;
   }
   if (at->morphType >= 0) {
      switch (at->morphType) {
      case PSY_MORPH_BARRAGE:
         mrp[0] = barragePattern[at->morphType][at->morphIdx[0]].bulletml;
         break;
      default:
         mi = (at->morphCnt - 1) & (MORPH_PATTERN_MAX - 1);
         mrp[mi] = barragePattern[at->morphType][at->morphIdx[mi]].bulletml;
         break;
      }
   }
   xr = at->xReverse;
   if (at->xrAlter && (idx & 1) == 1)
      xr = -xr;
#ifdef FIXEDMATH
   bt->foe =
      addFoeBattery (bs->x + bt->x, bs->y + bt->y, at->frank, 512, 0, xr, mrp,
                     at->morphCnt, at->morphHalf, at->fmorphRank,
                     at->fspeedRank, sp->color, sp->bulletShape,
                     sp->fbulletSize, lt, at->ikaType,
                     barragePattern[at->barrageType][at->barrageIdx].bulletml);
#else
   bt->foe = addFoeBattery(bs->x+bt->x, bs->y+bt->y, at->rank, 512, 0, xr, mrp,
                           at->morphCnt, at->morphHalf, at->morphRank, 
                           at->speedRank, sp->color, sp->bulletShape, 
                           sp->bulletSize, lt, at->ikaType,
                           barragePattern[at->barrageType][at->barrageIdx].bulletml);
#endif //FIXEDMATH
}

//senquack - converting to fixed point:
//static void setBossWing(BossWing *lw, BossWing *rw, int size, int num) {
//  int i, j;
//  lw->wingNum = rw->wingNum = num;
//  lw->size = rw->size = 0;
//  for ( i=0 ; i<num ; i++ ) {
//    for ( j=0 ; j<2 ; j++ ) {
//      lw->x[i][j] = ((float)(randN(size/2)+size/2) / FIELD_SCREEN_RATIO) * (randN(2)*2-1);
//      lw->y[i][j] = ((float)(randN(size/2)+size/2) / FIELD_SCREEN_RATIO) * (randN(2)*2-1);
//      lw->z[i][j] = ((float)(randN(size/2)+size/2) / FIELD_SCREEN_RATIO) * (randN(2)*2-1);
//      rw->x[i][j] = -lw->x[i][j];
//      rw->y[i][j] =  lw->y[i][j];
//      rw->z[i][j] =  lw->z[i][j];
//    }
//  }
//}
static void
setBossWing (BossWing * lw, BossWing * rw, int size, int num)
{
   int i, j;
   lw->wingNum = rw->wingNum = num;
#ifdef FIXEDMATH
   lw->fsize = rw->fsize = 0;
#else
   lw->size = rw->size = 0;
#endif //FIXEDMATH
   for (i = 0; i < num; i++) {
      for (j = 0; j < 2; j++) {
#ifdef FIXEDMATH
         lw->fx[i][j] = f2x(((float)(randN(size/2)+size/2) / FIELD_SCREEN_RATIO) * (randN(2)*2-1));
         lw->fy[i][j] = f2x(((float)(randN(size/2)+size/2) / FIELD_SCREEN_RATIO) * (randN(2)*2-1));
         lw->fz[i][j] = f2x(((float)(randN(size/2)+size/2) / FIELD_SCREEN_RATIO) * (randN(2)*2-1));
         rw->fx[i][j] = -lw->fx[i][j];
         rw->fy[i][j] = lw->fy[i][j];
         rw->fz[i][j] = lw->fz[i][j];
#else
         lw->x[i][j] = ((float)(randN(size/2)+size/2) / FIELD_SCREEN_RATIO) * (randN(2)*2-1);
         lw->y[i][j] = ((float)(randN(size/2)+size/2) / FIELD_SCREEN_RATIO) * (randN(2)*2-1);
         lw->z[i][j] = ((float)(randN(size/2)+size/2) / FIELD_SCREEN_RATIO) * (randN(2)*2-1);
         rw->x[i][j] = -lw->x[i][j];
         rw->y[i][j] =  lw->y[i][j];
         rw->z[i][j] =  lw->z[i][j];
#endif //FIXEDMATH
      }
   }
}

//senquack - converting to fixed point:
//static void setBossTree(BatteryGroup *bg, BossTree *left, BossTree *right) {
//  int cx, cy, tn, bn, x, y;
//  int i;
//  left->diffuse = right->diffuse = 0;
//  bn = bg->batteryNum;
//  left->epNum = right->epNum = bn;
//  cx = cy = 0;
//  for ( i=0 ; i<bn ; i++ ) {
//    cx += bg->battery[i].x; cy += bg->battery[i].y;
//    left->ex[i] = (float)bg->battery[i].x / FIELD_SCREEN_RATIO;
//    left->ey[i] = (float)bg->battery[i].y / FIELD_SCREEN_RATIO;
//    left->ez[i] = 0;
//    right->ex[i] = -(float)bg->battery[i].x / FIELD_SCREEN_RATIO;
//    right->ey[i] =  (float)bg->battery[i].y / FIELD_SCREEN_RATIO;
//    right->ez[i] = 0;
//    setBossWing(&(left->eWing[i]), &(right->eWing[i]), 5000, 1);
//  }
//  cx /= bn; cy /= bn; 
//  cx /= 2; cy /= 2;
//  tn = 2 + randN(TREE_MAX_LENGTH-2);
//  left->posNum = right->posNum = tn+1;
//  x = y = 0; cx /= tn; cy /= tn;
//  for ( i=0 ; i<=tn ; i++ ) {
//    if ( i == 0 ) {
//      left->x[i] = left->y[i] = left->z[i] = 0;
//    } else {
//      left->x[i] = (float)(x+randNS(2000)) / FIELD_SCREEN_RATIO;
//      left->y[i] = (float)(y+randNS(2000)) / FIELD_SCREEN_RATIO;
//      left->z[i] = (float)randNS(3000) / FIELD_SCREEN_RATIO;
//    }
//    right->x[i] = -left->x[i];
//    right->y[i] =  left->y[i];
//    right->z[i] =  left->z[i];
//    x += cx; y += cy;
//    setBossWing(&(left->wing[i]), &(right->wing[i]), 10000, 2);
//  }
//}
static void
setBossTree (BatteryGroup * bg, BossTree * left, BossTree * right)
{
   int cx, cy, tn, bn, x, y;
   int i;
   left->diffuse = right->diffuse = 0;
   bn = bg->batteryNum;
   left->epNum = right->epNum = bn;
   cx = cy = 0;
   for (i = 0; i < bn; i++) {
      cx += bg->battery[i].x;
      cy += bg->battery[i].y;
#ifdef FIXEDMATH
      left->fex[i] = f2x ((float) bg->battery[i].x / FIELD_SCREEN_RATIO);
      left->fey[i] = f2x ((float) bg->battery[i].y / FIELD_SCREEN_RATIO);
      left->fez[i] = 0;
      right->fex[i] = f2x (-(float) bg->battery[i].x / FIELD_SCREEN_RATIO);
      right->fey[i] = f2x ((float) bg->battery[i].y / FIELD_SCREEN_RATIO);
      right->fez[i] = 0;
#else
      left->ex[i] = (float)bg->battery[i].x / FIELD_SCREEN_RATIO;
      left->ey[i] = (float)bg->battery[i].y / FIELD_SCREEN_RATIO;
      left->ez[i] = 0;
      right->ex[i] = -(float)bg->battery[i].x / FIELD_SCREEN_RATIO;
      right->ey[i] =  (float)bg->battery[i].y / FIELD_SCREEN_RATIO;
      right->ez[i] = 0;
#endif //FIXEDMATH
      setBossWing (&(left->eWing[i]), &(right->eWing[i]), 5000, 1);
   }
   cx /= bn;
   cy /= bn;
   cx /= 2;
   cy /= 2;
   tn = 2 + randN (TREE_MAX_LENGTH - 2);
   left->posNum = right->posNum = tn + 1;
   x = y = 0;
   cx /= tn;
   cy /= tn;
   for (i = 0; i <= tn; i++) {
      if (i == 0) {
#ifdef FIXEDMATH
         left->fx[i] = left->fy[i] = left->fz[i] = 0;
#else
         left->x[i] = left->y[i] = left->z[i] = 0;
#endif //FIXEDMATH
      } else {
#ifdef FIXEDMATH
         left->fx[i] = f2x ((float) (x + randNS (2000)) / FIELD_SCREEN_RATIO);
         left->fy[i] = f2x ((float) (y + randNS (2000)) / FIELD_SCREEN_RATIO);
         left->fz[i] = f2x ((float) randNS (3000) / FIELD_SCREEN_RATIO);
#else
         left->x[i] = (float)(x+randNS(2000)) / FIELD_SCREEN_RATIO;
         left->y[i] = (float)(y+randNS(2000)) / FIELD_SCREEN_RATIO;
         left->z[i] = (float)randNS(3000) / FIELD_SCREEN_RATIO;
#endif //FIXEDMATH
      }
#ifdef FIXEDMATH
      right->fx[i] = -left->fx[i];
      right->fy[i] = left->fy[i];
      right->fz[i] = left->fz[i];
#else
      right->x[i] = -left->x[i];
      right->y[i] =  left->y[i];
      right->z[i] =  left->z[i];
#endif //FIXEDMATH
      x += cx;
      y += cy;
      setBossWing (&(left->wing[i]), &(right->wing[i]), 10000, 2);
   }
}

#define BOSS_PATTERN_CHANGE_CNT 100
#define BOSS_SHIELD 30000
#define BOSS_SHIELD_MAX 70000

#define BOSS_TIMER_COUNT_UP 17
#define BOSS_TIME_UP (1000*90)
static int bossDstBaseTime;

//senquack - complete conversion to floats:
//void createBoss(int seed, double rank, int round) {
void
createBoss (int seed, float rank, int round)
{
   int bn, bgn = 0, lbn, bgni = 0;
   int i, j;
   int wx, wy, cx, cy;
   //senquack TODO: make sure conversion to floats from doubles here didn't mess up the bullet patterns, etc:
//senquack - complete conversion to floats:
//  double tr, sr, sra;
   float tr, sr, sra;
   int bx, by;
   int idx1, idx2;
   int maxX = -999999, maxXY;
   int vbgn, vbg[BATTERY_GROUP_MAX / 2], vn;
   cx = maxXY = vbgn = 0;

   setSeed (seed);

   // Boss size.
   wx = 18000 + randN (round * 2000 + 5000);
   wy = 12000 + randN (round * 1500 + 3000);

   // Set the number of batterys.
   bn = round / 2 + 4 + randN (round / 2 + 2);
   for (; bn > 0 && bgn < BATTERY_GROUP_MAX / 2; bgni += 2) {
      bgn++;
      lbn = randN (3) + 1;
      if (lbn >= bn) {
         boss.batteryGroup[bgni].batteryNum = bn;
         boss.batteryGroup[bgni + 1].batteryNum = bn;
         break;
      }
      bn -= lbn;
      boss.batteryGroup[bgni].batteryNum = lbn;
      boss.batteryGroup[bgni + 1].batteryNum = lbn;
   }
   boss.batteryGroupNum = bgn * 2;

   // Set the positions of batterys groups.
   cy = -wy;
   for (i = 0; i < bgn; i++) {
      cx = randN (wx * 2 / 3) + wx / 3;
      cy += wy * 3 / (bgn + 1);
      setBatteryGroupPos (&(boss.batteryGroup[i * 2]),
                          &(boss.batteryGroup[i * 2 + 1]), cx, cy);
      if (cx > maxX) {
         maxX = cx;
         maxXY = cy;
      }
      if (i == 0) {
         boss.collisionYUp = cy;
      }
   }
   boss.collisionX[0] = -maxX * 4 / 3;
   boss.collisionY[0] = maxXY;
   boss.collisionX[4] = maxX * 4 / 3;
   boss.collisionY[4] = maxXY;
   boss.collisionX[1] = -cx * 4 / 3;
   boss.collisionY[1] = cy;
   boss.collisionX[3] = cx * 4 / 3;
   boss.collisionY[3] = cy;
   boss.collisionX[2] = 0;
   boss.collisionY[2] = cy / 2;
   if (boss.collisionYUp > maxXY - 10000) {
      boss.collisionYUp = maxXY - 10000;
   }

   boss.shield = BOSS_SHIELD + round * (BOSS_SHIELD_MAX - BOSS_SHIELD) / 4;
   boss.patternChangeShield = boss.shield * (70 + randN (8)) / 256;
   boss.damaged = boss.damageCnt = 0;
   bossDstBaseTime = (boss.shield / 40) * BOSS_TIMER_COUNT_UP;

   // Set the shapes of batterys bullets.
   setBatteryShape (&(boss.shape));
   for (i = 0; i < bgn; i++) {
      setBatteryShape (&(boss.batteryGroup[i * 2].shape));
      boss.batteryGroup[i * 2 + 1].shape = boss.batteryGroup[i * 2].shape;
      if (mode == IKA_MODE) {
         boss.batteryGroup[i * 2 + 1].shape.color ^= 1;
      }
   }

   // Set the attack patterns.
   boss.patternNum = 4 + randN (3);
   for (j = 0; j < boss.patternNum; j++) {
      for (i = 0; i < bgn; i++) {
         setAttackIndex (&(boss.batteryGroup[i * 2].attack[j]), 0);
      }
   }
   for (j = 0; j < boss.patternNum; j++) {
      tr = rank;
      if (j == 0) {
         tr *= 1.2;
         vbgn = 1 + bgn / 2;
      } else {
         switch (randN (6)) {
         case 0:
            vbgn = 0;
            break;
         case 1:
         case 2:
         case 3:
         case 4:
            vbgn = 1;
            break;
         case 5:
            vbgn = 2;
            break;
         }
      }
      if (vbgn > bgn)
         vbgn = bgn;
      for (i = 0; i < bgn; i++)
         vbg[i] = 0;
      for (i = 0; i < vbgn; i++) {
         vn = randN (bgn);
         while (vbg[vn]) {
            vn--;
            if (vn < 0)
               vn += bgn;
         }
         vbg[vn] = 1;
      }
      sra = tr / vbgn;
      for (i = 0; i < bgn; i++) {
         if (tr > 0 && vbg[i]) {
   //senquack TODO: make sure conversion to floats from doubles here didn't mess up the bullet patterns, etc:
//senquack - complete conversion to floats:
// sr = ((double)randN(((int)(sra*256+1))))/256 + sra/2;
            sr = ((float) randN (((int) (sra * 256 + 1)))) / 256 + sra / 2;
            if (sr > tr)
               sr = tr;
            bn = boss.batteryGroup[i * 2].batteryNum;
            tr -= sr;
            sr /= (bn * bn);
            setAttackRank (&(boss.batteryGroup[i * 2].attack[j]), sr / bn);
            boss.batteryGroup[i * 2 + 1].attack[j] =
               boss.batteryGroup[i * 2].attack[j];
            /*if ( boss.batteryGroup[i*2+1].attack[j].barrageType == REVERSIBLE_BARRAGE ) {
               boss.batteryGroup[i*2+1].attack[j].xReverse *= -1;
               } */
            boss.batteryGroup[i * 2 + 1].attack[j].xReverse *= -1;
            boss.batteryGroup[i * 2].limiter.max =
               boss.batteryGroup[i * 2 + 1].limiter.max =
               (int) (rank * 5) + 1;
         } else {
            boss.batteryGroup[i * 2].attack[j].barrageType = NOT_EXIST;
            boss.batteryGroup[i * 2 + 1].attack[j].barrageType = NOT_EXIST;
         }
      }
      if (tr > 0) {
         setAttack (&(boss.topAttack[j]), tr, 1);
         boss.topLimiter.max = (int) (rank * 12) + 2;
      } else {
         boss.topAttack[j].barrageType = NOT_EXIST;
      }
   }
   boss.patternLgt = 420 + randN (90);
   boss.patternIdx = 1;
   boss.patternCnt = BOSS_PATTERN_CHANGE_CNT;
   boss.state = CREATING;
   boss.stateCnt = BOSS_PATTERN_CHANGE_CNT;

   // Set the moving patterns.
   // Boss move size.
   wx = 10000 - round * 2000 + randN (4001 - round * 1000);
   wy = 4500 - round * 800 + randN (2001 - round * 500);

   boss.mpNum = randN (3) + 2;
   for (i = 0; i < boss.mpNum / 2; i++) {
      boss.mpx[i * 2] = randN (wx / 2) + wx / 2;
      boss.mpx[i * 2 + 1] = -boss.mpx[i * 2];
      boss.mpy[i * 2] = boss.mpy[i * 2 + 1] = randN (wy * 2) - wy + boss.y;
   }
   if (boss.mpNum == 3) {
      boss.mpx[2] = 0;
      boss.mpy[2] = randN (wy * 2) - wy + boss.y;
   }
   for (i = 0; i < 8; i++) {
      idx1 = randN (boss.mpNum);
      idx2 = randN (boss.mpNum);
      if (idx1 == idx2) {
         idx2++;
         if (idx2 >= boss.mpNum)
            idx2 = 0;
      }
      bx = boss.mpx[idx1];
      by = boss.mpy[idx2];
      boss.mpx[idx1] = boss.mpx[idx2];
      boss.mpy[idx1] = boss.mpy[idx2];
      boss.mpx[idx2] = bx;
      boss.mpy[idx2] = by;
   }
   boss.speed = randN (48) + 64 - 8 * round;
   boss.md = 2 + randN (3);
   boss.mpIdx = 0;
   boss.onRoute = 0;

   // Set the boss shape.
   bossShape.r = 240;
   bossShape.g = 240;
   bossShape.b = 120;
   for (i = 0; i < boss.batteryGroupNum; i += 2) {
      BatteryGroup *bg;
      bg = &(boss.batteryGroup[i]);
      setBossTree (bg, &(bossShape.tree[i]), &(bossShape.tree[i + 1]));
   }
   bossTimer = 0;
}

// Add fragments.

//senquack - converting to fixed point:
//static void addBossTreeFrag(BossTree *bt) {
//  int i, dst, deg;
//  int ox, oy;
//  float x, y;
//  int bpn;
//  x =  (float)boss.x / FIELD_SCREEN_RATIO;
//  y = -(float)boss.y / FIELD_SCREEN_RATIO;
//  bpn = bt->posNum-1;
//  for ( i=0 ; i<bpn ; i++ ) {
//    ox =  (int)((bt->x[i+1]-bt->x[i])*256);
//    oy = -(int)((bt->y[i+1]-bt->y[i])*256);
//    dst = getDistance(ox, oy);
//    deg = getDeg(ox, oy);
//    addBossFrag((bt->x[i+1]+bt->x[i])/2 + x,
//    -(bt->y[i+1]+bt->y[i])/2 + y,
//    (bt->z[i+1]+bt->z[i])/2, 
//    (float)dst/512, deg);
//  }
//  for ( i=0 ; i<bt->epNum ; i++ ) {
//    ox =  (int)((bt->ex[i]-bt->x[bpn])*256);
//    oy = -(int)((bt->ey[i]-bt->y[bpn])*256);
//    dst = getDistance(ox, oy);
//    deg = getDeg(ox, oy);
//    addBossFrag((bt->ex[i]+bt->x[bpn])/2 + x,
//    -(bt->ey[i]+bt->y[bpn])/2 + y,
//    (bt->ez[i]+bt->z[bpn])/2, 
//    (float)dst/512, deg);
//  }
//}
static void
addBossTreeFrag (BossTree * bt)
{
#ifdef FIXEDMATH
   int i, dst, deg;
   int ox, oy;
//  float x, y;
   GLfixed fx, fy;
   int bpn;
   //senquack TODO: this can be optimized probably:
//  x =  (float)boss.x / FIELD_SCREEN_RATIO;
//  y = -(float)boss.y / FIELD_SCREEN_RATIO;
   fx = f2x ((float) boss.x / FIELD_SCREEN_RATIO);
   fy = f2x (-(float) boss.y / FIELD_SCREEN_RATIO);

   bpn = bt->posNum - 1;
   for (i = 0; i < bpn; i++) {
//    ox =  (int)((bt->x[i+1]-bt->x[i])*256);
//    oy = -(int)((bt->y[i+1]-bt->y[i])*256);
      ox = FNUM2INT (((bt->fx[i + 1] - bt->fx[i]) << 8));
      oy = -FNUM2INT (((bt->fy[i + 1] - bt->fy[i]) << 8));
      dst = getDistance (ox, oy);
      deg = getDeg (ox, oy);
//    addBossFrag((bt->x[i+1]+bt->x[i])/2 + x,
//    -(bt->y[i+1]+bt->y[i])/2 + y,
//    (bt->z[i+1]+bt->z[i])/2, 
//    (float)dst/512, deg);
//    addBossFragx(((bt->fx[i+1]+bt->fx[i])>>1) + fx,
//    -((bt->fy[i+1]+bt->fy[i])>>1) + fy,
//    (bt->fz[i+1]+bt->fz[i])>>1, 
//    dst<<7, deg);
//    addBossFragx(((bt->fx[i+1]+bt->fx[i])>>1) + fx,
//    -((bt->fy[i+1]+bt->fy[i])>>1) + fy,
//    (bt->fz[i+1]+bt->fz[i])>>1, 
//    f2x((float)dst/512.0f), deg);
//    addBossFragx(((bt->fx[i+1]+bt->fx[i])>>1) + fx,
//    -((bt->fy[i+1]+bt->fy[i])>>1) + fy,
//    (bt->fz[i+1]+bt->fz[i])>>1, 
//    INT2FNUM(dst>>9), deg);
//senquack TODO: figure out why I used FDIV here instead of shifting: .. also clean the crap above it
      addBossFragx (((bt->fx[i + 1] + bt->fx[i]) >> 1) + fx,
                    -((bt->fy[i + 1] + bt->fy[i]) >> 1) + fy,
                    (bt->fz[i + 1] + bt->fz[i]) >> 1,
                    FDIV (INT2FNUM (dst), 33554432), deg);
   }
   for (i = 0; i < bt->epNum; i++) {
//    ox =  (int)((bt->ex[i]-bt->x[bpn])*256);
//    oy = -(int)((bt->ey[i]-bt->y[bpn])*256);
      ox = FNUM2INT (((bt->fex[i] - bt->fx[bpn]) << 8));
      oy = -FNUM2INT (((bt->fey[i] - bt->fy[bpn]) << 8));
      dst = getDistance (ox, oy);
      deg = getDeg (ox, oy);
//    addBossFrag((bt->ex[i]+bt->x[bpn])/2 + x,
//    -(bt->ey[i]+bt->y[bpn])/2 + y,
//    (bt->ez[i]+bt->z[bpn])/2, 
//    (float)dst/512, deg);
//    addBossFragx(((bt->fex[i]+bt->fx[bpn])>>1) + fx,
//    -((bt->fey[i]+bt->fy[bpn])>>1) + fy,
//    (bt->fez[i]+bt->fz[bpn])>>1, 
//    dst<<7, deg);
//    addBossFragx(((bt->fex[i]+bt->fx[bpn])>>1) + fx,
//    -((bt->fey[i]+bt->fy[bpn])>>1) + fy,
//    (bt->fez[i]+bt->fz[bpn])>>1, 
//    f2x((float)dst/512.0f), deg);
//    addBossFragx(((bt->fex[i]+bt->fx[bpn])>>1) + fx,
//    -((bt->fey[i]+bt->fy[bpn])>>1) + fy,
//    (bt->fez[i]+bt->fz[bpn])>>1, 
//    INT2FNUM(dst>>9), deg);
//senquack TODO: figure out why I used FDIV here instead of shifting: .. also clean the crap above it
      addBossFragx (((bt->fex[i] + bt->fx[bpn]) >> 1) + fx,
                    -((bt->fey[i] + bt->fy[bpn]) >> 1) + fy,
                    (bt->fez[i] + bt->fz[bpn]) >> 1,
                    FDIV (INT2FNUM (dst), 33554432), deg);
   }
#else
  int i, dst, deg;
  int ox, oy;
  float x, y;
  int bpn;
  x =  (float)boss.x / FIELD_SCREEN_RATIO;
  y = -(float)boss.y / FIELD_SCREEN_RATIO;
  bpn = bt->posNum-1;
  for ( i=0 ; i<bpn ; i++ ) {
    ox =  (int)((bt->x[i+1]-bt->x[i])*256);
    oy = -(int)((bt->y[i+1]-bt->y[i])*256);
    dst = getDistance(ox, oy);
    deg = getDeg(ox, oy);
    //senquack TODO: can probably optimize the dst/512 here:
    addBossFrag((bt->x[i+1]+bt->x[i])/2 + x,
       -(bt->y[i+1]+bt->y[i])/2 + y,
       (bt->z[i+1]+bt->z[i])/2, 
       (float)dst/512, deg);
  }
  for ( i=0 ; i<bt->epNum ; i++ ) {
    ox =  (int)((bt->ex[i]-bt->x[bpn])*256);
    oy = -(int)((bt->ey[i]-bt->y[bpn])*256);
    dst = getDistance(ox, oy);
    deg = getDeg(ox, oy);
    //senquack TODO: can probably optimize the dst/512 here:
    addBossFrag((bt->ex[i]+bt->x[bpn])/2 + x,
       -(bt->ey[i]+bt->y[bpn])/2 + y,
       (bt->ez[i]+bt->z[bpn])/2, 
       (float)dst/512, deg);
  }
#endif //FIXEDMATH
}

static void
addBossTreeFragPart ()
{
   addBossTreeFrag (&(bossShape.tree[randN (boss.batteryGroupNum)]));
}

static void
destroyBoss ()
{
   boss.state = DESTROIED;
   boss.patternCnt = 999999;
   boss.stateCnt = BOSS_PATTERN_CHANGE_CNT * 2;
   clearFoes ();
   playChunk (5);
   setScreenShake (0, BOSS_PATTERN_CHANGE_CNT * 2);
   ship.absEng = 0;
}

// Boss movements.

static int
handleLimiter (Limiter * lt, int allLmtOn)
{
   if (!lt->on) {
      if (lt->cnt > lt->max) {
         lt->on = 1;
         lt->cnt /= 3;
         lt->cnt += 32;
      }
   }
   if ((boss.cnt & 3) == 0 && lt->cnt > 0) {
      lt->cnt--;
      if (allLmtOn)
         lt->cnt -= 2;
      if (lt->on && lt->cnt <= 0) {
         lt->on = 0;
         lt->cnt = 0;
      }
   }
   return lt->on;
}

static int allLmtOn = 0;

//senquack - converting to fixed point
//void moveBoss() {
//  int bpi;
//  int i, j;
//  int ax, ay, d, od, aod, emd;
//  int dfsChg;
//  int lmtOn;
//
//  // Change the barrages pattern.
//  boss.patternCnt--;
//  if ( boss.patternCnt < 0 ) {
//    if ( boss.state == LAST_ATTACK ) {
//      boss.patternCnt = 999999;
//    } else {
//      boss.patternCnt = boss.patternLgt;
//    }
//
//    bpi = boss.patternIdx;
//    boss.topLimiter.cnt = 0; boss.topLimiter.on = 0; 
//    setFoeBattery(&boss, &(boss.topBattery), &(boss.topAttack[bpi]), &(boss.shape), 
//      &(boss.topLimiter), 0);
//    for ( i=0 ; i<boss.batteryGroupNum ; i++ ) {
//      BatteryGroup *bg = &(boss.batteryGroup[i]);
//      bg->limiter.cnt = 0; bg->limiter.on = 0;
//      for ( j=0 ; j<bg->batteryNum ; j++ ) {
// setFoeBattery(&boss, &(bg->battery[j]), &(bg->attack[bpi]), 
//          &(bg->shape), &(bg->limiter), j);
//      }
//    }
//  } else if ( boss.patternCnt == BOSS_PATTERN_CHANGE_CNT ) {
//    if ( boss.topBattery.foe ) removeFoeForced(boss.topBattery.foe);
//    for ( i=0 ; i<boss.batteryGroupNum ; i++ ) {
//      BatteryGroup *bg = &(boss.batteryGroup[i]);
//      for ( j=0 ; j<bg->batteryNum ; j++ ) {
// if ( bg->battery[j].foe ) removeFoeForced(bg->battery[j].foe);
//      }
//    }
//  } else if ( boss.patternCnt < BOSS_PATTERN_CHANGE_CNT ) {
//    if ( boss.patternCnt <= BOSS_PATTERN_CHANGE_CNT/2 ) {
//      if ( boss.patternCnt == BOSS_PATTERN_CHANGE_CNT/2 && boss.state == ATTACKING ) {
// boss.patternIdx++;
// if ( boss.patternIdx >= boss.patternNum ) boss.patternIdx = 1;
//      }
//      dfsChg = 6;
//    } else {
//      dfsChg = -6;
//    }
//    if ( boss.topAttack[boss.patternIdx].barrageType != NOT_EXIST ) {
//      bossShape.diffuse += dfsChg;
//      if ( bossShape.diffuse < 0 ) bossShape.diffuse = 0;
//      else if ( bossShape.diffuse > 255 ) bossShape.diffuse = 255;
//    }
//    for ( i=0 ; i<boss.batteryGroupNum ; i++ ) {
//      BatteryGroup *bg = &(boss.batteryGroup[i]);
//      if ( bg->attack[boss.patternIdx].barrageType != NOT_EXIST ) {
// bossShape.tree[i].diffuse += dfsChg;
// if ( bossShape.tree[i].diffuse < 0 ) bossShape.tree[i].diffuse = 0;
// else if ( bossShape.tree[i].diffuse > 255 ) bossShape.tree[i].diffuse = 255;
//      }
//    }
//  }
//
//  // Movement.
//  if ( boss.state >= DESTROIED_END ) {
//    boss.d = 0;
//    boss.x += (BOSS_INITIAL_X - boss.x)>>6;
//    boss.y += (BOSS_INITIAL_Y - boss.y)>>6;
//  } else {
//    ax = boss.mpx[boss.mpIdx];
//    ay = boss.mpy[boss.mpIdx];
//    d = getDeg(ax - boss.x, ay - boss.y);
//    od = d - boss.d;
//    if ( od > 512 ) od -= 1024;
//    if ( od < -512 ) od += 1024;
//    aod = absN(od);
//    if ( !boss.onRoute ) {
//      if ( aod < 256 ) {
// boss.onRoute = 1;
//      }
//    } else {
//      if ( aod > 256 ) {
// boss.onRoute = 0;
// boss.mpIdx++; if ( boss.mpIdx >= boss.mpNum ) boss.mpIdx = 0;
//      }
//    }
//    emd = boss.md;
//    if ( aod < emd ) {
//      boss.d = d;
//    } else if ( od > 0 ) {
//      boss.d += emd;
//    } else {
//      boss.d -= emd;
//    }
//    boss.d &= 1023;
//    boss.x += (sctbl[boss.d]    *boss.speed)>>8;
//    boss.y -= (sctbl[boss.d+256]*boss.speed)>>8;
//  }
//  if ( boss.y < -FIELD_HEIGHT_8/2 ) {
//    boss.y = -FIELD_HEIGHT_8/2;
//  }
//
//  lmtOn = 1;
//  if ( boss.topBattery.foe ) {
//    boss.topBattery.foe->pos.x = boss.x;
//    boss.topBattery.foe->pos.y = boss.y;
//  }
//  if ( boss.topAttack[boss.patternIdx].barrageType != NOT_EXIST ) {
//    lmtOn &= handleLimiter(&(boss.topLimiter), allLmtOn);
//  }
//  for ( i=0 ; i<boss.batteryGroupNum ; i++ ) {
//    BatteryGroup *bg = &(boss.batteryGroup[i]);
//    if ( bg->attack[boss.patternIdx].barrageType != NOT_EXIST ) {
//      lmtOn &= handleLimiter(&(bg->limiter), allLmtOn);
//    }
//    for ( j=0 ; j<bg->batteryNum ; j++ ) {
//      Battery *bt = &(bg->battery[j]);
//      if ( bt->foe ) {
// bt->foe->pos.x = boss.x + bt->x;
// bt->foe->pos.y = boss.y + bt->y;
//      }
//    }
//  }
//  if ( lmtOn ) { 
//    allLmtOn = 1;
//  } else {
//    allLmtOn = 0;
//  }
//
//  boss.r = bossShape.r; boss.g = bossShape.g; boss.b = bossShape.b;
//  if ( boss.damaged ) {
//    if ( (boss.damageCnt&1) == 0 ) {
//      boss.r = bossShape.r/2; boss.g = 255; boss.b = bossShape.b/2;
//      if ( (boss.damageCnt&31) == 0 ) {
// playChunk(2);
//      }
//    }
//    boss.damageCnt++;
//    switch ( mode ) {
//    case NORMAL_MODE:
//      if ( (boss.cnt&7) == 0 ) {
// addScore(bonusScore/10*10);
//      }
//      break;
//    case PSY_MODE:
//    case IKA_MODE:
//    case GW_MODE:
//      if ( (boss.cnt&15) == 0 ) {
// addScore(bonusScore/10*10);
//      }
//      break;
//    }
//  } else {
//    if ( boss.damageCnt > 0 ) {
//      boss.damageCnt = 0;
//      haltChunk(2);
//    }
//    bonusScore -= 10;
//    if ( bonusScore < 10 ) bonusScore = 10;
//  }
//  boss.damaged = 0;
//  boss.cnt++;
//
//  boss.stateCnt--;
//  switch ( boss.state ) {
//  case CREATING:
//    if ( boss.stateCnt <= 0 ) {
//      boss.state = ATTACKING;
//    }
//    break;
//  case CHANGE:
//    if ( boss.stateCnt <= 0 ) {
//      boss.state = LAST_ATTACK;
//    }
//    break;
//  case DESTROIED:
//    if ( randN(7) == 0 ) addBossTreeFragPart();
//    if ( randN(15) == 0 ) playChunk(6);
//    for ( i=0 ; i<boss.batteryGroupNum ; i++ ) {
//      BossTree *bt = &(bossShape.tree[i]);
//      for ( j=0 ; j<bt->posNum ; j++ ) {
// bt->wing[j].size *= 0.99;
//      }
//      for ( j=0 ; j<boss.batteryGroup[i].batteryNum ; j++ ) {
// bt->eWing[j].size *= 0.985;
//      }
//    }
//    if ( boss.stateCnt <= 0 ) {
//      int bs;
//      for ( i=0 ; i<boss.batteryGroupNum ; i++ ) {
// BossTree *bt = &(bossShape.tree[i]);
// for ( j=0 ; j<16 ; j++ ) addBossTreeFrag(bt);
//      }
//      boss.stateCnt = 999999;
//      boss.state = DESTROIED_END;
//      playChunk(4);
//      setScreenShake(1, BOSS_PATTERN_CHANGE_CNT);
//      switch ( mode ) {
//      case NORMAL_MODE:
// bs = 7000;
// break;
//      case PSY_MODE:
//      case IKA_MODE:
//      case GW_MODE:
// bs = 5000;
// break;
//      }
//      bs = (long)(bossDstBaseTime*3 - bossTimer)*5000/(bossDstBaseTime*2);
//      if ( bs < 0 || bossTimer >= BOSS_TIME_UP ) bs = 0;
//      bs *= 100;
//      initBossScoreAtr(bs);
//    }
//    break;
//  case DESTROIED_END:
//    moveBossScoreAtr();
//    break;
//  }
//  if ( boss.state == ATTACKING || boss.state == LAST_ATTACK ) {
//    bossTimer += BOSS_TIMER_COUNT_UP;
//    if ( bossTimer >= BOSS_TIME_UP && status == IN_GAME ) {
//      bossTimer = BOSS_TIME_UP;
//      destroyBoss();
//    }
//  }
//}
void
moveBoss ()
{
   int bpi;
   int i, j;
   int ax, ay, d, od, aod, emd;
   int dfsChg;
   int lmtOn;

   // Change the barrages pattern.
   boss.patternCnt--;
   if (boss.patternCnt < 0) {
      if (boss.state == LAST_ATTACK) {
         boss.patternCnt = 999999;
      } else {
         boss.patternCnt = boss.patternLgt;
      }

      bpi = boss.patternIdx;
      boss.topLimiter.cnt = 0;
      boss.topLimiter.on = 0;
      setFoeBattery (&boss, &(boss.topBattery), &(boss.topAttack[bpi]),
                     &(boss.shape), &(boss.topLimiter), 0);
      for (i = 0; i < boss.batteryGroupNum; i++) {
         BatteryGroup *bg = &(boss.batteryGroup[i]);
         bg->limiter.cnt = 0;
         bg->limiter.on = 0;
         for (j = 0; j < bg->batteryNum; j++) {
            setFoeBattery (&boss, &(bg->battery[j]), &(bg->attack[bpi]),
                           &(bg->shape), &(bg->limiter), j);
         }
      }
   } else if (boss.patternCnt == BOSS_PATTERN_CHANGE_CNT) {
      if (boss.topBattery.foe)
         removeFoeForced (boss.topBattery.foe);
      for (i = 0; i < boss.batteryGroupNum; i++) {
         BatteryGroup *bg = &(boss.batteryGroup[i]);
         for (j = 0; j < bg->batteryNum; j++) {
            if (bg->battery[j].foe)
               removeFoeForced (bg->battery[j].foe);
         }
      }
   } else if (boss.patternCnt < BOSS_PATTERN_CHANGE_CNT) {
      if (boss.patternCnt <= BOSS_PATTERN_CHANGE_CNT / 2) {
         if (boss.patternCnt == BOSS_PATTERN_CHANGE_CNT / 2
             && boss.state == ATTACKING) {
            boss.patternIdx++;
            if (boss.patternIdx >= boss.patternNum)
               boss.patternIdx = 1;
         }
         dfsChg = 6;
      } else {
         dfsChg = -6;
      }
      if (boss.topAttack[boss.patternIdx].barrageType != NOT_EXIST) {
         bossShape.diffuse += dfsChg;
         if (bossShape.diffuse < 0)
            bossShape.diffuse = 0;
         else if (bossShape.diffuse > 255)
            bossShape.diffuse = 255;
      }
      for (i = 0; i < boss.batteryGroupNum; i++) {
         BatteryGroup *bg = &(boss.batteryGroup[i]);
         if (bg->attack[boss.patternIdx].barrageType != NOT_EXIST) {
            bossShape.tree[i].diffuse += dfsChg;
            if (bossShape.tree[i].diffuse < 0)
               bossShape.tree[i].diffuse = 0;
            else if (bossShape.tree[i].diffuse > 255)
               bossShape.tree[i].diffuse = 255;
         }
      }
   }
   // Movement.
   if (boss.state >= DESTROIED_END) {
      boss.d = 0;
      boss.x += (BOSS_INITIAL_X - boss.x) >> 6;
      boss.y += (BOSS_INITIAL_Y - boss.y) >> 6;
   } else {
      ax = boss.mpx[boss.mpIdx];
      ay = boss.mpy[boss.mpIdx];
      d = getDeg (ax - boss.x, ay - boss.y);
      od = d - boss.d;
      if (od > 512)
         od -= 1024;
      if (od < -512)
         od += 1024;
      aod = absN (od);
      if (!boss.onRoute) {
         if (aod < 256) {
            boss.onRoute = 1;
         }
      } else {
         if (aod > 256) {
            boss.onRoute = 0;
            boss.mpIdx++;
            if (boss.mpIdx >= boss.mpNum)
               boss.mpIdx = 0;
         }
      }
      emd = boss.md;
      if (aod < emd) {
         boss.d = d;
      } else if (od > 0) {
         boss.d += emd;
      } else {
         boss.d -= emd;
      }
      boss.d &= 1023;
      boss.x += (sctbl[boss.d] * boss.speed) >> 8;
      boss.y -= (sctbl[boss.d + 256] * boss.speed) >> 8;
   }
   if (boss.y < -FIELD_HEIGHT_8 / 2) {
      boss.y = -FIELD_HEIGHT_8 / 2;
   }

   lmtOn = 1;
   if (boss.topBattery.foe) {
      boss.topBattery.foe->pos.x = boss.x;
      boss.topBattery.foe->pos.y = boss.y;
   }
   if (boss.topAttack[boss.patternIdx].barrageType != NOT_EXIST) {
      lmtOn &= handleLimiter (&(boss.topLimiter), allLmtOn);
   }
   for (i = 0; i < boss.batteryGroupNum; i++) {
      BatteryGroup *bg = &(boss.batteryGroup[i]);
      if (bg->attack[boss.patternIdx].barrageType != NOT_EXIST) {
         lmtOn &= handleLimiter (&(bg->limiter), allLmtOn);
      }
      for (j = 0; j < bg->batteryNum; j++) {
         Battery *bt = &(bg->battery[j]);
         if (bt->foe) {
            bt->foe->pos.x = boss.x + bt->x;
            bt->foe->pos.y = boss.y + bt->y;
         }
      }
   }
   if (lmtOn) {
      allLmtOn = 1;
   } else {
      allLmtOn = 0;
   }

   boss.r = bossShape.r;
   boss.g = bossShape.g;
   boss.b = bossShape.b;
   if (boss.damaged) {
      if ((boss.damageCnt & 1) == 0) {
         boss.r = bossShape.r / 2;
         boss.g = 255;
         boss.b = bossShape.b / 2;
         if ((boss.damageCnt & 31) == 0) {
            playChunk (2);
         }
      }
      boss.damageCnt++;
      switch (mode) {
      case NORMAL_MODE:
         if ((boss.cnt & 7) == 0) {
            addScore (bonusScore / 10 * 10);
         }
         break;
      case PSY_MODE:
      case IKA_MODE:
      case GW_MODE:
         if ((boss.cnt & 15) == 0) {
            addScore (bonusScore / 10 * 10);
         }
         break;
      }
   } else {
      if (boss.damageCnt > 0) {
         boss.damageCnt = 0;
         haltChunk (2);
      }
      bonusScore -= 10;
      if (bonusScore < 10)
         bonusScore = 10;
   }
   boss.damaged = 0;
   boss.cnt++;

   boss.stateCnt--;
   switch (boss.state) {
   case CREATING:
      if (boss.stateCnt <= 0) {
         boss.state = ATTACKING;
      }
      break;
   case CHANGE:
      if (boss.stateCnt <= 0) {
         boss.state = LAST_ATTACK;
      }
      break;
   case DESTROIED:
      if (randN (7) == 0)
         addBossTreeFragPart ();
      if (randN (15) == 0)
         playChunk (6);
      for (i = 0; i < boss.batteryGroupNum; i++) {
         BossTree *bt = &(bossShape.tree[i]);
         for (j = 0; j < bt->posNum; j++) {
#ifdef FIXEDMATH
            bt->wing[j].fsize = FMUL (bt->wing[j].fsize, 64881);
#else
            bt->wing[j].size *= 0.99;
#endif //FIXEDMATH
         }
         for (j = 0; j < boss.batteryGroup[i].batteryNum; j++) {
#ifdef FIXEDMATH
            bt->eWing[j].fsize = FMUL (bt->eWing[j].fsize, 64553);
#else
            bt->eWing[j].size *= 0.985;
#endif //FIXEDMATH
         }
      }
      if (boss.stateCnt <= 0) {
         int bs;
         for (i = 0; i < boss.batteryGroupNum; i++) {
            BossTree *bt = &(bossShape.tree[i]);
            for (j = 0; j < 16; j++)
               addBossTreeFrag (bt);
         }
         boss.stateCnt = 999999;
         boss.state = DESTROIED_END;
         playChunk (4);
         setScreenShake (1, BOSS_PATTERN_CHANGE_CNT);
         switch (mode) {
         case NORMAL_MODE:
            bs = 7000;
            break;
         case PSY_MODE:
         case IKA_MODE:
         case GW_MODE:
            bs = 5000;
            break;
         }
         bs =
            (long) (bossDstBaseTime * 3 -
                    bossTimer) * 5000 / (bossDstBaseTime * 2);
         if (bs < 0 || bossTimer >= BOSS_TIME_UP)
            bs = 0;
         bs *= 100;
         initBossScoreAtr (bs);
      }
      break;
   case DESTROIED_END:
      moveBossScoreAtr ();
      break;
   }
   if (boss.state == ATTACKING || boss.state == LAST_ATTACK) {
      bossTimer += BOSS_TIMER_COUNT_UP;
      if (bossTimer >= BOSS_TIME_UP && status == IN_GAME) {
         bossTimer = BOSS_TIME_UP;
         destroyBoss ();
      }
   }
}

//senquack - converting to fixed point:
//void damageBoss(int dmg) {
//  int tn, pn, bn;
//  int i, j;
//  BossTree *bt;
//  tn = randN(TREE_MAX_LENGTH);
//  if ( tn < boss.batteryGroupNum ) {
//    bt = &(bossShape.tree[tn]);
//    pn = randN(bt->posNum);
//    bt->wing[pn].size *= 0.996;
//    bn = randN(boss.batteryGroup[tn].batteryNum);
//    bt->eWing[bn].size *= 0.996;
//  }
//  boss.shield -= dmg;
//  boss.damaged = 1;
//  switch ( boss.state ) {
//  case ATTACKING:
//    if ( boss.shield <= boss.patternChangeShield ) {
//      boss.shield = boss.patternChangeShield;
//      for ( i=0 ; i<boss.batteryGroupNum ; i++ ) {
// BossTree *bt = &(bossShape.tree[i]);
// for ( j=0 ; j<4 ; j++ ) addBossTreeFrag(bt);
// for ( j=0 ; j<bt->posNum ; j++ ) {
//   bt->wing[j].size = 0;
// }
// for ( j=0 ; j<boss.batteryGroup[i].batteryNum ; j++ ) {
//   bt->eWing[j].size = 0;
// }
//      }
//      boss.state = CHANGE; 
//      boss.patternCnt = BOSS_PATTERN_CHANGE_CNT+1;
//      boss.stateCnt = BOSS_PATTERN_CHANGE_CNT;
//      boss.patternIdx = 0;
//      clearFoes();
//      playChunk(5);
//    }
//    break;
//  case LAST_ATTACK:
//    if ( boss.shield <= 0 ) {
//      for ( i=0 ; i<boss.batteryGroupNum ; i++ ) {
// BossTree *bt = &(bossShape.tree[i]);
// for ( j=0 ; j<2 ; j++ ) addBossTreeFrag(bt);
//      }
//      destroyBoss();
//    }
//    break;
//  }
//}
void
damageBoss (int dmg)
{
   int tn, pn, bn;
   int i, j;
   BossTree *bt;
   tn = randN (TREE_MAX_LENGTH);
   if (tn < boss.batteryGroupNum) {
      bt = &(bossShape.tree[tn]);
      pn = randN (bt->posNum);
      bn = randN (boss.batteryGroup[tn].batteryNum);
#ifdef FIXEDMATH
      bt->wing[pn].fsize = FMUL (bt->wing[pn].fsize, 65274);
      bt->eWing[bn].fsize = FMUL (bt->eWing[bn].fsize, 65274);
#else
      bt->wing[pn].size *= 0.996;
      bt->eWing[bn].size *= 0.996;
#endif //FIXEDMATH
   }
   boss.shield -= dmg;
   boss.damaged = 1;
   switch (boss.state) {
   case ATTACKING:
      if (boss.shield <= boss.patternChangeShield) {
         boss.shield = boss.patternChangeShield;
         for (i = 0; i < boss.batteryGroupNum; i++) {
            BossTree *bt = &(bossShape.tree[i]);
            for (j = 0; j < 4; j++)
               addBossTreeFrag (bt);
            for (j = 0; j < bt->posNum; j++) {
#ifdef FIXEDMATH
               bt->wing[j].fsize = 0;
#else
               bt->wing[j].size = 0;
#endif //FIXEDMATH
            }
            for (j = 0; j < boss.batteryGroup[i].batteryNum; j++) {
#ifdef FIXEDMATH
               bt->eWing[j].fsize = 0;
#else
               bt->eWing[j].size = 0;
#endif //FIXEDMATH
            }
         }
         boss.state = CHANGE;
         boss.patternCnt = BOSS_PATTERN_CHANGE_CNT + 1;
         boss.stateCnt = BOSS_PATTERN_CHANGE_CNT;
         boss.patternIdx = 0;
         clearFoes ();
         playChunk (5);
      }
      break;
   case LAST_ATTACK:
      if (boss.shield <= 0) {
         for (i = 0; i < boss.batteryGroupNum; i++) {
            BossTree *bt = &(bossShape.tree[i]);
            for (j = 0; j < 2; j++)
               addBossTreeFrag (bt);
         }
         destroyBoss ();
      }
      break;
   }
}

void
damageBossLaser (int cnt)
{
   if (mode == NORMAL_MODE || mode == PSY_MODE) {
      damageBoss (40 - cnt);
   } else {
      damageBoss ((40 - cnt) * 2 / 3);
   }
   bonusScore += (40 - cnt) / 8;
   if (bonusScore > 1000)
      bonusScore = 1000;
   ship.grzCnt++;               // Laser hitting also increases the graze meter.
}

// Return a collision of the boss.
int
checkHitDownside (int x)
{
   int i;
   int x1, x2, y1, y2;
   if (boss.state != ATTACKING && boss.state != LAST_ATTACK)
      return -999999;
   for (i = 0; i < 4; i++) {
      x1 = boss.collisionX[i] + boss.x;
      x2 = boss.collisionX[i + 1] + boss.x;
      if (x1 <= x && x < x2) {
         y1 = boss.collisionY[i] + boss.y;
         y2 = boss.collisionY[i + 1] + boss.y;
         return (y2 - y1) * (x - x1) / (x2 - x1) + y1;
      }
   }
   return -999999;
}

int
checkHitUpside ()
{
   return boss.y + boss.collisionYUp;
}

//senquack - TODO: clean up this cruft from figuring out GP2X freezing was ultimately bug in line drawing in GPU940:
                  /// ALSO: clean up all these old iterations left commented-out as I worked on Wiz version!
//senquack - one of the 2 causes of freezing is in drawBossWing 
//static void drawBossWing(float x1, float y1, float z1, float x2, float y2, float z2,
//        BossWing *wg) {
//  int i;
//  float sz = wg->size;
//  for ( i=0 ; i<wg->wingNum ; i++ ) {
//    drawSquare(x2, y2, z2, x1, y1, z1,
//        x1+wg->x[i][0]*sz, y1+wg->y[i][0]*sz, z1+wg->z[i][0]*sz, 
//        x2+wg->x[i][1]*sz, y2+wg->y[i][1]*sz, z2+wg->z[i][1]*sz,
//        boss.r, boss.g, boss.b);
//  }
//}
//static void drawBossWing(float x1, float y1, float z1, float x2, float y2, float z2,
//        BossWing *wg) {
//  int i;
//  float sz = wg->size;
//  for ( i=0 ; i<wg->wingNum ; i++ ) {
//    drawSquare(x2, y2, z2, x1, y1, z1,
//        x1+wg->x[i][0]*sz, y1+wg->y[i][0]*sz, z1+wg->z[i][0]*sz, 
//        x2+wg->x[i][1]*sz, y2+wg->y[i][1]*sz, z2+wg->z[i][1]*sz,
//        boss.r, boss.g, boss.b);
//  }
//}
//static void drawBossWingx(GLfixed x1, GLfixed y1, GLfixed z1, GLfixed x2, GLfixed y2, GLfixed z2,
//        BossWing *wg) {
//
// //senquack - experiment:
// z1 = 0; z2 = 0;
//
//  int i;
////  float sz = wg->size;
//  GLfixed sz = wg->fsize;
//  for ( i=0 ; i<wg->wingNum ; i++ ) {
////    drawSquare(x2, y2, z2, x1, y1, z1,
////         x1+wg->x[i][0]*sz, y1+wg->y[i][0]*sz, z1+wg->z[i][0]*sz, 
////         x2+wg->x[i][1]*sz, y2+wg->y[i][1]*sz, z2+wg->z[i][1]*sz,
////         boss.r, boss.g, boss.b);
//    drawSquarex(x2, y2, z2, x1, y1, z1,
//        x1+FMUL(wg->fx[i][0],sz), y1+FMUL(wg->fy[i][0],sz), z1+FMUL(wg->fz[i][0],sz), 
//        x2+FMUL(wg->fx[i][1],sz), y2+FMUL(wg->fy[i][1],sz), z2+FMUL(wg->fz[i][1],sz),
//        boss.r, boss.g, boss.b);
////    drawWingx(x2, y2, z2, x1, y1, z1,
////         x1+FMUL(wg->fx[i][0],sz), y1+FMUL(wg->fy[i][0],sz), z1+FMUL(wg->fz[i][0],sz), 
////         x2+FMUL(wg->fx[i][1],sz), y2+FMUL(wg->fy[i][1],sz), z2+FMUL(wg->fz[i][1],sz),
////         boss.r, boss.g, boss.b);
//  }
//}
//senquack TODO: these probably can be shrunk down greatly from the Wiz version, since I noted below that
//       only 400 total vertices ever seems to be drawn:
#ifdef FIXEDMATH
static GLfixed wingvertices[1024 * 2];
static GLubyte wingcolors[1024 * 4];
static GLfixed *wingvertptr;
static GLubyte *wingcolptr;

//senquack - added this for GLES1.1 fixed-point
static void
prepareDrawBossWingsx (void)
{
   wingvertptr = &(wingvertices[0]);
   wingcolptr = &(wingcolors[0]);
}

//senquack - added this for GLES1.1 fixed-point
static void
finishDrawBossWingsx (void)
{
   int numwingvertices =
      ((unsigned int) wingcolptr - (unsigned int) &(wingcolors[0])) >> 2;
   //senquack - never seemed to go over 400 vertices total:
// printf("Drawing wings with %d vertices\n", numwingvertices);
   glVertexPointer (2, GL_FIXED, 0, wingvertices);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, wingcolors);
   glDrawArrays (GL_TRIANGLES, 0, numwingvertices);
}

//senquack - added this for GLES1.1 fixed-point
static void
drawBossWingx (GLfixed x1, GLfixed y1, GLfixed x2, GLfixed y2, BossWing * wg)
{
   int i;
   GLfixed sz = wg->fsize;
   GLubyte r = boss.r, g = boss.g, b = boss.b;
   GLfixed tmpx, tmpy;
   for (i = 0; i < wg->wingNum; i++) {

      tmpx = x1 + FMUL (wg->fx[i][0], sz);
      tmpy = y1 + FMUL (wg->fy[i][0], sz);

      //senquack TODO: optimize RGBA here, interleave also
      *wingcolptr++ = r; *wingcolptr++ = g; *wingcolptr++ = b; *wingcolptr++ = 64;
      *wingcolptr++ = r; *wingcolptr++ = g; *wingcolptr++ = b; *wingcolptr++ = 64;
      *wingcolptr++ = r; *wingcolptr++ = g; *wingcolptr++ = b; *wingcolptr++ = 64;
      *wingcolptr++ = r; *wingcolptr++ = g; *wingcolptr++ = b; *wingcolptr++ = 64;
      *wingcolptr++ = r; *wingcolptr++ = g; *wingcolptr++ = b; *wingcolptr++ = 64;
      *wingcolptr++ = r; *wingcolptr++ = g; *wingcolptr++ = b; *wingcolptr++ = 64;
      *wingvertptr++ = x2;
      *wingvertptr++ = y2;
      *wingvertptr++ = x1;
      *wingvertptr++ = y1;
      *wingvertptr++ = tmpx;
      *wingvertptr++ = tmpy;
      *wingvertptr++ = x1;
      *wingvertptr++ = y1;
      *wingvertptr++ = tmpx;
      *wingvertptr++ = tmpy;
      *wingvertptr++ = x2 + FMUL (wg->fx[i][1], sz);
      *wingvertptr++ = y2 + FMUL (wg->fy[i][1], sz);
   }
}
#else
//senquack TODO: these probably can be shrunk down greatly from the Wiz version, since I noted below that
//       only 400 total vertices ever seems to be drawn:
static GLfloat wingvertices[1024 * 2];
static GLubyte wingcolors[1024 * 4];
static GLfloat *wingvertptr;
static GLubyte *wingcolptr;

//senquack - added this for GLES1.1 float-point
static void
prepareDrawBossWings(void)
{
   wingvertptr = &(wingvertices[0]);
   wingcolptr = &(wingcolors[0]);
}

//senquack - added this for GLES1.1 float-point
static void
finishDrawBossWings(void)
{
   //senquack TODO: this is not the best way, because what if we're running on a 64-bit CPU?:
   int numwingvertices = ((unsigned int) wingcolptr - (unsigned int) &(wingcolors[0])) >> 2;
   //senquack - never seemed to go over 400 vertices total:
// printf("Drawing wings with %d vertices\n", numwingvertices);
   glVertexPointer (2, GL_FLOAT, 0, wingvertices);
   glColorPointer (4, GL_UNSIGNED_BYTE, 0, wingcolors);
   glDrawArrays (GL_TRIANGLES, 0, numwingvertices);
}

//senquack - added this for GLES1.1 fixed-point
static void
drawBossWing(GLfoat x1, GLfloat y1, GLfloat x2, GLfloat y2, BossWing * wg)
{
   int i;
   GLfloat sz = wg->size;
   GLubyte r = boss.r, g = boss.g, b = boss.b;
   GLfloat tmpx, tmpy;
   for (i = 0; i < wg->wingNum; i++) {

      tmpx = x1 + wg->x[i][0] * sz;
      tmpy = y1 + wg->y[i][0] * sz;

      //senquack TODO: optimize RGBA here, interleave also
      *wingcolptr++ = r; *wingcolptr++ = g; *wingcolptr++ = b; *wingcolptr++ = 64;
      *wingcolptr++ = r; *wingcolptr++ = g; *wingcolptr++ = b; *wingcolptr++ = 64;
      *wingcolptr++ = r; *wingcolptr++ = g; *wingcolptr++ = b; *wingcolptr++ = 64;
      *wingcolptr++ = r; *wingcolptr++ = g; *wingcolptr++ = b; *wingcolptr++ = 64;
      *wingcolptr++ = r; *wingcolptr++ = g; *wingcolptr++ = b; *wingcolptr++ = 64;
      *wingcolptr++ = r; *wingcolptr++ = g; *wingcolptr++ = b; *wingcolptr++ = 64;
      *wingvertptr++ = x2;
      *wingvertptr++ = y2;
      *wingvertptr++ = x1;
      *wingvertptr++ = y1;
      *wingvertptr++ = tmpx;
      *wingvertptr++ = tmpy;
      *wingvertptr++ = x1;
      *wingvertptr++ = y1;
      *wingvertptr++ = tmpx;
      *wingvertptr++ = tmpy;
      *wingvertptr++ = x2 + wg->x[i][1] * sz;
      *wingvertptr++ = y2 + wg->y[i][1] * sz;
   }
}
#endif //FIXEDMATH


//senquack TODO: clean this up!!!
//senquack - optimizing for wiz v1.1
//void drawBoss() {
//  float x, y;
//  float x1, y1, z1, x2, y2, z2;
//  int i, j;
//  int df;
//  int crBpn, crBpl;
//  int bpn;
//  crBpn = crBpl = 0;
//
//  x =  (float)boss.x / FIELD_SCREEN_RATIO;
//  y = -(float)boss.y / FIELD_SCREEN_RATIO;
//
//  if ( bossShape.diffuse > 0  && boss.state < DESTROIED ) {
//    df = bossShape.diffuse;
////    drawStar(1, x, y, 0, df, df, df, (float)(df+256)/500.0f);
////    drawStar(1, x, y, 0, df, df, df, (float)(df+randN(256))/500.0f);
//  }
//  
//  for ( i=0 ; i<boss.batteryGroupNum ; i++ ) {
//    BossTree *bt = &(bossShape.tree[i]);
//    bpn = bt->posNum-1;
//    x1 = x; y1 = y; z1 = 0;
//    switch ( boss.state ) {
//    case CREATING:
//    case CHANGE:
//      crBpn = (bpn+1)*(BOSS_PATTERN_CHANGE_CNT-boss.stateCnt-1)/BOSS_PATTERN_CHANGE_CNT;
//      crBpl = 255 - 
// (boss.stateCnt%(BOSS_PATTERN_CHANGE_CNT/(bpn+1))*256)/(BOSS_PATTERN_CHANGE_CNT/(bpn+1));
//      break;
//    }
//    for ( j=0 ; j<bpn ; j++ ) {
//
//      x2 =  x + bt->x[j+1];
//      y2 =  y - bt->y[j+1];
//      z2 =  bt->z[j+1];
//
//      switch ( boss.state ) {
//      case ATTACKING:
//      case LAST_ATTACK:
//      case DESTROIED:
// drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 240);
////  glColor4hack(bossShape.r, bossShape.g, bossShape.b, 240);
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(x1, y1, z1);
////  glVertex3f(x2, y2, z2);
////  glEnd();
//
// drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->wing[j]));
// break;
//      case CREATING:
// if ( j == crBpn ) {
//   drawLinePart(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 240, crBpl);
// } else if ( j < crBpn ) {
//   drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 240);
////    glColor4hack(bossShape.r, bossShape.g, bossShape.b, 240);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(x1, y1, z1);
////    glVertex3f(x2, y2, z2);
////    glEnd();
// }
// if ( crBpn == bpn ) {
//   bt->wing[j].size = (float)crBpl/255;
//   drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->wing[j]));
// }
// break;
//      case CHANGE:
// break;
//      }
//      if ( bt->diffuse > 0 && boss.state != CHANGE && boss.state < DESTROIED ) {
// df = bt->diffuse;
////  drawStar(0, x2, y2, z2, df, df, df, (float)(df+256)/900.0f);
////  drawStar(0, x2, y2, z2, df, df, df, (float)(df+randN(256))/900.0f);
//      }
//      x1 = x2; y1 = y2; z1 = z2;
//    }
//    
//    x1 = x + bt->x[bpn];
//    y1 = y - bt->y[bpn];
//    z1 = bt->z[bpn];
//
//    for ( j=0 ; j<bt->epNum ; j++ ) {
//      x2 = x + bt->ex[j];
//      y2 = y - bt->ey[j];
//      z2 = bt->ez[j];
//
//      switch ( boss.state ) {
//      case ATTACKING:
//      case LAST_ATTACK:
//      case DESTROIED:
// drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 220);
////    glColor4hack(bossShape.r, bossShape.g, bossShape.b, 220);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(x1, y1, z1);
////    glVertex3f(x2, y2, z2);
////    glEnd();
//
// drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->eWing[j]));
// break;
//      case CREATING:
// if ( crBpn == bpn ) {
//   drawLinePart(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 220, crBpl);
//   bt->eWing[j].size = (float)crBpl/255;
//   drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->eWing[j]));
// }
// break;
//      case CHANGE:
// drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 220);
////    glColor4hack(bossShape.r, bossShape.g, bossShape.b, 220);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(x1, y1, z1);
////    glVertex3f(x2, y2, z2);
////    glEnd();
//
// if ( crBpn == bpn ) {
//   bt->eWing[j].size = (float)crBpl/128;
//   drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->eWing[j]));
// }
// break;
//      }
//      if ( bt->diffuse > 0 && boss.state != CHANGE && boss.state < DESTROIED ) {
// df = bt->diffuse;
////  drawStar(1, x2, y2, z2, df, df, df, (float)(df+256)/640.0f);
////  drawStar(1, x2, y2, z2, df, df, df, (float)(df+randN(256))/640.0f);
//      }
//    }
//  }
////  drawCore(x, y, boss.cnt, boss.r, boss.g, boss.b);
//  drawCorex(f2x(x), f2x(y), boss.cnt, boss.r, boss.g, boss.b);
//}
//senquack - optimizing further (no need for z axis it seems)
//void drawBoss() {
////  float x, y;
////  float x1, y1, z1, x2, y2, z2;
//  GLfixed fx, fy;
//  GLfixed fx1, fy1, fz1, fx2, fy2, fz2;
//  int i, j;
//  int df;
//  int crBpn, crBpl;
//  int bpn;
//  crBpn = crBpl = 0;
//
////  x =  (float)boss.x / FIELD_SCREEN_RATIO;
////  y = -(float)boss.y / FIELD_SCREEN_RATIO;
////  fx =  f2x((float)boss.x / FIELD_SCREEN_RATIO);
////  fy = f2x(-(float)boss.y / FIELD_SCREEN_RATIO);
//  fx =  FDIV(INT2FNUM(boss.x), FIELD_SCREEN_RATIO_X);
//  //senquack - y positions tend to overflow with fixed point
////  fy = f2x(-(float)boss.y / FIELD_SCREEN_RATIO);
//  fy = (int)(-(float)boss.y * 6.5536f); // roll division and fixed point conversion into one multiply
//
//  if ( bossShape.diffuse > 0  && boss.state < DESTROIED ) {
//    df = bossShape.diffuse;
////    drawStar(1, x, y, 0, df, df, df, (float)(df+256)/500.0f);
////    drawStar(1, x, y, 0, df, df, df, (float)(df+randN(256))/500.0f);
//  }
//  
//  for ( i=0 ; i<boss.batteryGroupNum ; i++ ) {
//    BossTree *bt = &(bossShape.tree[i]);
//    bpn = bt->posNum-1;
////    x1 = x; y1 = y; z1 = 0;
//    fx1 = fx; fy1 = fy; fz1 = 0;
//    switch ( boss.state ) {
//    case CREATING:
//    case CHANGE:
//      crBpn = (bpn+1)*(BOSS_PATTERN_CHANGE_CNT-boss.stateCnt-1)/BOSS_PATTERN_CHANGE_CNT;
//      crBpl = 255 - 
// (boss.stateCnt%(BOSS_PATTERN_CHANGE_CNT/(bpn+1))*256)/(BOSS_PATTERN_CHANGE_CNT/(bpn+1));
//      break;
//    }
//    for ( j=0 ; j<bpn ; j++ ) {
//
////      x2 =  x + bt->x[j+1];
////      y2 =  y - bt->y[j+1];
////      z2 =  bt->z[j+1];
//      fx2 =  fx + bt->fx[j+1];
//      fy2 =  fy - bt->fy[j+1];
//      fz2 =  bt->fz[j+1];
//
//      switch ( boss.state ) {
//      case ATTACKING:
//      case LAST_ATTACK:
//      case DESTROIED:
////  drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 240);
////  drawLinex(fx1, fy1, fz1, fx2, fy2, fz2, bossShape.r, bossShape.g, bossShape.b, 240);
// drawLinex(fx1, fy1, fx2, fy2, bossShape.r, bossShape.g, bossShape.b, 240);
////  glColor4hack(bossShape.r, bossShape.g, bossShape.b, 240);
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(x1, y1, z1);
////  glVertex3f(x2, y2, z2);
////  glEnd();
//
////  drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->wing[j]));
////  drawBossWingx(fx1, fy1, fx2, fy2, &(bt->wing[j]));
////  drawBossWingx(fx1, fy1, 0, fx2, fy2, 0, &(bt->wing[j]));
// drawBossWingx(fx1, fy1, fz1, fx2, fy2, fz2, &(bt->wing[j]));
// break;
//      case CREATING:
// if ( j == crBpn ) {
////    drawLinePart(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 240, crBpl);
//    //senquack - don't need 3D vertices for this
//   drawLinePartx(fx1, fy1, fx2, fy2, bossShape.r, bossShape.g, bossShape.b, 240, crBpl);
// } else if ( j < crBpn ) {
////    drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 240);
//   drawLinex(fx1, fy1, fx2, fy2, bossShape.r, bossShape.g, bossShape.b, 240);
////    glColor4hack(bossShape.r, bossShape.g, bossShape.b, 240);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(x1, y1, z1);
////    glVertex3f(x2, y2, z2);
////    glEnd();
// }
// if ( crBpn == bpn ) {
////    bt->wing[j].size = (float)crBpl/255;
////    bt->wing[j].fsize = INT2FNUM(crBpl>>8);
////    bt->wing[j].fsize = f2x((float)crBpl/255.0f);
//   bt->wing[j].fsize = FDIV(INT2FNUM(crBpl),16711680);
////    drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->wing[j]));
////    drawBossWingx(fx1, fy1, fx2, fy2, &(bt->wing[j]));
////    drawBossWingx(fx1, fy1, 0, fx2, fy2, 0, &(bt->wing[j]));
//   drawBossWingx(fx1, fy1, fz1, fx2, fy2, fz2, &(bt->wing[j]));
// }
// break;
//      case CHANGE:
// break;
//      }
//      if ( bt->diffuse > 0 && boss.state != CHANGE && boss.state < DESTROIED ) {
// df = bt->diffuse;
////  drawStar(0, x2, y2, z2, df, df, df, (float)(df+256)/900.0f);
////  drawStar(0, x2, y2, z2, df, df, df, (float)(df+randN(256))/900.0f);
//      }
////      x1 = x2; y1 = y2; z1 = z2;
//      fx1 = fx2; fy1 = fy2; fz1 = fz2;
//    }
//    
////    x1 = x + bt->x[bpn];
////    y1 = y - bt->y[bpn];
////    z1 = bt->z[bpn];
//    fx1 = fx + bt->fx[bpn];
//    fy1 = fy - bt->fy[bpn];
//    fz1 = bt->fz[bpn];
//
//    for ( j=0 ; j<bt->epNum ; j++ ) {
////      x2 = x + bt->ex[j];
////      y2 = y - bt->ey[j];
////      z2 = bt->ez[j];
//      fx2 = fx + bt->fex[j];
//      fy2 = fy - bt->fey[j];
//      fz2 = bt->fez[j];
//
//      switch ( boss.state ) {
//      case ATTACKING:
//      case LAST_ATTACK:
//      case DESTROIED:
////  drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 220);
// drawLinex(fx1, fy1, fx2, fy2, bossShape.r, bossShape.g, bossShape.b, 220);
////    glColor4hack(bossShape.r, bossShape.g, bossShape.b, 220);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(x1, y1, z1);
////    glVertex3f(x2, y2, z2);
////    glEnd();
//
////  drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->eWing[j]));
// drawBossWingx(fx1, fy1, fz1, fx2, fy2, fz2, &(bt->eWing[j]));
// break;
//      case CREATING:
// if ( crBpn == bpn ) {
////    drawLinePart(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 220, crBpl);
//   drawLinePartx(fx1, fy1, fx2, fy2, bossShape.r, bossShape.g, bossShape.b, 220, crBpl);
////    bt->eWing[j].size = (float)crBpl/255;
////    bt->eWing[j].fsize = INT2FNUM(crBpl>>8);
////    bt->eWing[j].fsize =  f2x((float)crBpl/255.0f); 
//   bt->eWing[j].fsize =  FDIV(INT2FNUM(crBpl) , 16711680);
//
////    drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->eWing[j]));
//   drawBossWingx(fx1, fy1, fz1, fx2, fy2, fz2, &(bt->eWing[j]));
// }
// break;
//      case CHANGE:
////  drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 220);
// drawLinex(fx1, fy1, fx2, fy2, bossShape.r, bossShape.g, bossShape.b, 220);
////    glColor4hack(bossShape.r, bossShape.g, bossShape.b, 220);
////    glBegin(GL_LINE_LOOP);
////    glVertex3f(x1, y1, z1);
////    glVertex3f(x2, y2, z2);
////    glEnd();
//
// if ( crBpn == bpn ) {
////    bt->eWing[j].size = (float)crBpl/128;
////    bt->eWing[j].fsize = INT2FNUM(crBpl>>7);
////    bt->eWing[j].fsize = f2x((float)crBpl/128.0f);
//   bt->eWing[j].fsize = FDIV(INT2FNUM(crBpl), 8388608);
////    drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->eWing[j]));
//   drawBossWingx(fx1, fy1, fz1, fx2, fy2, fz2, &(bt->eWing[j]));
// }
// break;
//      }
//      if ( bt->diffuse > 0 && boss.state != CHANGE && boss.state < DESTROIED ) {
// df = bt->diffuse;
////  drawStar(1, x2, y2, z2, df, df, df, (float)(df+256)/640.0f);
////  drawStar(1, x2, y2, z2, df, df, df, (float)(df+randN(256))/640.0f);
//      }
//    }
//  }
////  drawCore(x, y, boss.cnt, boss.r, boss.g, boss.b);
////  drawCorex(f2x(x), f2x(y), boss.cnt, boss.r, boss.g, boss.b);
//  drawCorex(fx, fy, boss.cnt, boss.r, boss.g, boss.b);
//}
// ----------------------------------
//senquack TODO: this last commented-out function is the one I used for Wiz 1.1.. I have cleaned it up down below 
//for the fixed-point version of the github port version:
// -------------------------------
//void
//drawBoss ()
//{
//   prepareDrawBossWingsx ();
//
////  float x, y;
////  float x1, y1, z1, x2, y2, z2;
//   GLfixed fx, fy;
////  GLfixed fx1, fy1, fz1, fx2, fy2, fz2;
//   GLfixed fx1, fy1, fx2, fy2;
//   int i, j;
//   int df;
//   int crBpn, crBpl;
//   int bpn;
//   crBpn = crBpl = 0;
//
////  x =  (float)boss.x / FIELD_SCREEN_RATIO;
////  y = -(float)boss.y / FIELD_SCREEN_RATIO;
////  fx =  f2x((float)boss.x / FIELD_SCREEN_RATIO);
////  fy = f2x(-(float)boss.y / FIELD_SCREEN_RATIO);
//   fx = FDIV (INT2FNUM (boss.x), FIELD_SCREEN_RATIO_X);
//   //senquack - y positions tend to overflow with fixed point
////  fy = f2x(-(float)boss.y / FIELD_SCREEN_RATIO);
//   fy = (int) (-(float) boss.y * 6.5536f);  // roll division and fixed point conversion into one multiply
//
//   if (bossShape.diffuse > 0 && boss.state < DESTROIED) {
//      df = bossShape.diffuse;
////    drawStar(1, x, y, 0, df, df, df, (float)(df+256)/500.0f);
////    drawStar(1, x, y, 0, df, df, df, (float)(df+randN(256))/500.0f);
//      drawStarx (1, fx, fy, df, df, df,
//                 FDIV (INT2FNUM (df + 256), INT2FNUM (500)));
//      drawStarx (1, fx, fy, df, df, df,
//                 FDIV (INT2FNUM (df + randN (256)), INT2FNUM (500)));
//   }
//
//   for (i = 0; i < boss.batteryGroupNum; i++) {
//      BossTree *bt = &(bossShape.tree[i]);
//      bpn = bt->posNum - 1;
////    x1 = x; y1 = y; z1 = 0;
////    fx1 = fx; fy1 = fy; fz1 = 0;
//      fx1 = fx;
//      fy1 = fy;
//      switch (boss.state) {
//      case CREATING:
//      case CHANGE:
//         crBpn =
//            (bpn + 1) * (BOSS_PATTERN_CHANGE_CNT - boss.stateCnt -
//                         1) / BOSS_PATTERN_CHANGE_CNT;
//         crBpl =
//            255 -
//            (boss.stateCnt % (BOSS_PATTERN_CHANGE_CNT / (bpn + 1)) * 256) /
//            (BOSS_PATTERN_CHANGE_CNT / (bpn + 1));
//         break;
//      }
//      for (j = 0; j < bpn; j++) {
//
////      x2 =  x + bt->x[j+1];
////      y2 =  y - bt->y[j+1];
////      z2 =  bt->z[j+1];
//         fx2 = fx + bt->fx[j + 1];
//         fy2 = fy - bt->fy[j + 1];
//         //senquack - no need for z axis
////      fz2 =  bt->fz[j+1];
//
//         switch (boss.state) {
//         case ATTACKING:
//         case LAST_ATTACK:
//         case DESTROIED:
//// drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 240);
//// drawLinex(fx1, fy1, fz1, fx2, fy2, fz2, bossShape.r, bossShape.g, bossShape.b, 240);
//            drawLinex (fx1, fy1, fx2, fy2, bossShape.r, bossShape.g,
//                       bossShape.b, 240);
////  glColor4hack(bossShape.r, bossShape.g, bossShape.b, 240);
////  glBegin(GL_LINE_LOOP);
////  glVertex3f(x1, y1, z1);
////  glVertex3f(x2, y2, z2);
////  glEnd();
//
//// drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->wing[j]));
//// drawBossWingx(fx1, fy1, fx2, fy2, &(bt->wing[j]));
//// drawBossWingx(fx1, fy1, 0, fx2, fy2, 0, &(bt->wing[j]));
//// drawBossWingx(fx1, fy1, fz1, fx2, fy2, fz2, &(bt->wing[j]));
//            drawBossWingx (fx1, fy1, fx2, fy2, &(bt->wing[j]));
//            break;
//         case CREATING:
//            if (j == crBpn) {
////   drawLinePart(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 240, crBpl);
//               //senquack - don't need 3D vertices for this
//               drawLinePartx (fx1, fy1, fx2, fy2, bossShape.r, bossShape.g,
//                              bossShape.b, 240, crBpl);
//            } else if (j < crBpn) {
////   drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 240);
//               drawLinex (fx1, fy1, fx2, fy2, bossShape.r, bossShape.g,
//                          bossShape.b, 240);
////   glColor4hack(bossShape.r, bossShape.g, bossShape.b, 240);
////   glBegin(GL_LINE_LOOP);
////   glVertex3f(x1, y1, z1);
////   glVertex3f(x2, y2, z2);
////   glEnd();
//            }
//            if (crBpn == bpn) {
////   bt->wing[j].size = (float)crBpl/255;
////   bt->wing[j].fsize = INT2FNUM(crBpl>>8);
////   bt->wing[j].fsize = f2x((float)crBpl/255.0f);
////   bt->wing[j].fsize = FDIV(INT2FNUM(crBpl),16711680);
//               bt->wing[j].fsize = FDIV (INT2FNUM (crBpl), 16711680);
//
////   drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->wing[j]));
////   drawBossWingx(fx1, fy1, fx2, fy2, &(bt->wing[j]));
////   drawBossWingx(fx1, fy1, 0, fx2, fy2, 0, &(bt->wing[j]));
////   drawBossWingx(fx1, fy1, fz1, fx2, fy2, fz2, &(bt->wing[j]));
//               drawBossWingx (fx1, fy1, fx2, fy2, &(bt->wing[j]));
//            }
//            break;
//         case CHANGE:
//            break;
//         }
//         if (bt->diffuse > 0 && boss.state != CHANGE
//             && boss.state < DESTROIED) {
//            df = bt->diffuse;
//// drawStar(0, x2, y2, z2, df, df, df, (float)(df+256)/900.0f);
//// drawStar(0, x2, y2, z2, df, df, df, (float)(df+randN(256))/900.0f);
//         }
////      x1 = x2; y1 = y2; z1 = z2;
////      fx1 = fx2; fy1 = fy2; fz1 = fz2;
//         fx1 = fx2;
//         fy1 = fy2;
//      }
//
////    x1 = x + bt->x[bpn];
////    y1 = y - bt->y[bpn];
////    z1 = bt->z[bpn];
//      fx1 = fx + bt->fx[bpn];
//      fy1 = fy - bt->fy[bpn];
////    fz1 = bt->fz[bpn];
//
//      for (j = 0; j < bt->epNum; j++) {
////      x2 = x + bt->ex[j];
////      y2 = y - bt->ey[j];
////      z2 = bt->ez[j];
//         fx2 = fx + bt->fex[j];
//         fy2 = fy - bt->fey[j];
////      fz2 = bt->fez[j];
//
//         switch (boss.state) {
//         case ATTACKING:
//         case LAST_ATTACK:
//         case DESTROIED:
//// drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 220);
//            drawLinex (fx1, fy1, fx2, fy2, bossShape.r, bossShape.g,
//                       bossShape.b, 220);
////   glColor4hack(bossShape.r, bossShape.g, bossShape.b, 220);
////   glBegin(GL_LINE_LOOP);
////   glVertex3f(x1, y1, z1);
////   glVertex3f(x2, y2, z2);
////   glEnd();
//
//// drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->eWing[j]));
//// drawBossWingx(fx1, fy1, fz1, fx2, fy2, fz2, &(bt->eWing[j]));
//            drawBossWingx (fx1, fy1, fx2, fy2, &(bt->eWing[j]));
//            break;
//         case CREATING:
//            if (crBpn == bpn) {
////   drawLinePart(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 220, crBpl);
//               drawLinePartx (fx1, fy1, fx2, fy2, bossShape.r, bossShape.g,
//                              bossShape.b, 220, crBpl);
////   bt->eWing[j].size = (float)crBpl/255;
////   bt->eWing[j].fsize = INT2FNUM(crBpl>>8);
////   bt->eWing[j].fsize =  f2x((float)crBpl/255.0f); 
//               bt->eWing[j].fsize = FDIV (INT2FNUM (crBpl), 16711680);
//
////   drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->eWing[j]));
////   drawBossWingx(fx1, fy1, fz1, fx2, fy2, fz2, &(bt->eWing[j]));
//               drawBossWingx (fx1, fy1, fx2, fy2, &(bt->eWing[j]));
//            }
//            break;
//         case CHANGE:
//// drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 220);
//            drawLinex (fx1, fy1, fx2, fy2, bossShape.r, bossShape.g,
//                       bossShape.b, 220);
////   glColor4hack(bossShape.r, bossShape.g, bossShape.b, 220);
////   glBegin(GL_LINE_LOOP);
////   glVertex3f(x1, y1, z1);
////   glVertex3f(x2, y2, z2);
////   glEnd();
//
//            if (crBpn == bpn) {
////   bt->eWing[j].size = (float)crBpl/128;
////   bt->eWing[j].fsize = INT2FNUM(crBpl>>7);
////   bt->eWing[j].fsize = f2x((float)crBpl/128.0f);
//               bt->eWing[j].fsize = FDIV (INT2FNUM (crBpl), 8388608);
////   drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->eWing[j]));
////   drawBossWingx(fx1, fy1, fz1, fx2, fy2, fz2, &(bt->eWing[j]));
//               drawBossWingx (fx1, fy1, fx2, fy2, &(bt->eWing[j]));
//            }
//            break;
//         }
//         if (bt->diffuse > 0 && boss.state != CHANGE
//             && boss.state < DESTROIED) {
//            df = bt->diffuse;
//// drawStar(1, x2, y2, z2, df, df, df, (float)(df+256)/640.0f);
//// drawStar(1, x2, y2, z2, df, df, df, (float)(df+randN(256))/640.0f);
//            drawStarx (1, fx2, fy2, df, df, df,
//                       FDIV (INT2FNUM (df + 256), INT2FNUM (640)));
//            drawStarx (1, fx2, fy2, df, df, df,
//                       FDIV (INT2FNUM (df + randN (256)), INT2FNUM (640)));
//         }
//      }
//   }
//   finishDrawBossWingsx ();
//
////  drawCore(x, y, boss.cnt, boss.r, boss.g, boss.b);
////  drawCorex(f2x(x), f2x(y), boss.cnt, boss.r, boss.g, boss.b);
//   drawCorex (fx, fy, boss.cnt, boss.r, boss.g, boss.b);
//}
void drawBoss ()
{
#ifdef FIXEDMATH
   prepareDrawBossWingsx ();

//  float x, y;
//  float x1, y1, z1, x2, y2, z2;
   GLfixed fx, fy;
//  GLfixed fx1, fy1, fz1, fx2, fy2, fz2;
   GLfixed fx1, fy1, fx2, fy2;
   int i, j;
   int df;
   int crBpn, crBpl;
   int bpn;
   crBpn = crBpl = 0;

//  x =  (float)boss.x / FIELD_SCREEN_RATIO;
//  y = -(float)boss.y / FIELD_SCREEN_RATIO;
//  fx =  f2x((float)boss.x / FIELD_SCREEN_RATIO);
//  fy = f2x(-(float)boss.y / FIELD_SCREEN_RATIO);
   fx = FDIV (INT2FNUM (boss.x), FIELD_SCREEN_RATIO_X);
   //senquack - y positions tend to overflow with fixed point
//  fy = f2x(-(float)boss.y / FIELD_SCREEN_RATIO);
   fy = (int) (-(float) boss.y * 6.5536f);  // roll division and fixed point conversion into one multiply

   if (bossShape.diffuse > 0 && boss.state < DESTROIED) {
      df = bossShape.diffuse;
//    drawStar(1, x, y, 0, df, df, df, (float)(df+256)/500.0f);
//    drawStar(1, x, y, 0, df, df, df, (float)(df+randN(256))/500.0f);
      drawStarx (1, fx, fy, df, df, df,
                 FDIV (INT2FNUM (df + 256), INT2FNUM (500)));
      drawStarx (1, fx, fy, df, df, df,
                 FDIV (INT2FNUM (df + randN (256)), INT2FNUM (500)));
   }

   for (i = 0; i < boss.batteryGroupNum; i++) {
      BossTree *bt = &(bossShape.tree[i]);
      bpn = bt->posNum - 1;
//    x1 = x; y1 = y; z1 = 0;
//    fx1 = fx; fy1 = fy; fz1 = 0;
      fx1 = fx;
      fy1 = fy;
      switch (boss.state) {
      case CREATING:
      case CHANGE:
         crBpn =
            (bpn + 1) * (BOSS_PATTERN_CHANGE_CNT - boss.stateCnt -
                         1) / BOSS_PATTERN_CHANGE_CNT;
         crBpl =
            255 -
            (boss.stateCnt % (BOSS_PATTERN_CHANGE_CNT / (bpn + 1)) * 256) /
            (BOSS_PATTERN_CHANGE_CNT / (bpn + 1));
         break;
      }
      for (j = 0; j < bpn; j++) {

//      x2 =  x + bt->x[j+1];
//      y2 =  y - bt->y[j+1];
//      z2 =  bt->z[j+1];
         fx2 = fx + bt->fx[j + 1];
         fy2 = fy - bt->fy[j + 1];
         //senquack - no need for z axis
//      fz2 =  bt->fz[j+1];

         switch (boss.state) {
         case ATTACKING:
         case LAST_ATTACK:
         case DESTROIED:
// drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 240);
// drawLinex(fx1, fy1, fz1, fx2, fy2, fz2, bossShape.r, bossShape.g, bossShape.b, 240);
            drawLinex (fx1, fy1, fx2, fy2, bossShape.r, bossShape.g,
                       bossShape.b, 240);
//  glColor4hack(bossShape.r, bossShape.g, bossShape.b, 240);
//  glBegin(GL_LINE_LOOP);
//  glVertex3f(x1, y1, z1);
//  glVertex3f(x2, y2, z2);
//  glEnd();

// drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->wing[j]));
// drawBossWingx(fx1, fy1, fx2, fy2, &(bt->wing[j]));
// drawBossWingx(fx1, fy1, 0, fx2, fy2, 0, &(bt->wing[j]));
// drawBossWingx(fx1, fy1, fz1, fx2, fy2, fz2, &(bt->wing[j]));
            drawBossWingx (fx1, fy1, fx2, fy2, &(bt->wing[j]));
            break;
         case CREATING:
            if (j == crBpn) {
//   drawLinePart(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 240, crBpl);
               //senquack - don't need 3D vertices for this
               drawLinePartx (fx1, fy1, fx2, fy2, bossShape.r, bossShape.g,
                              bossShape.b, 240, crBpl);
            } else if (j < crBpn) {
//   drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 240);
               drawLinex (fx1, fy1, fx2, fy2, bossShape.r, bossShape.g,
                          bossShape.b, 240);
//   glColor4hack(bossShape.r, bossShape.g, bossShape.b, 240);
//   glBegin(GL_LINE_LOOP);
//   glVertex3f(x1, y1, z1);
//   glVertex3f(x2, y2, z2);
//   glEnd();
            }
            if (crBpn == bpn) {
//   bt->wing[j].size = (float)crBpl/255;
//   bt->wing[j].fsize = INT2FNUM(crBpl>>8);
//   bt->wing[j].fsize = f2x((float)crBpl/255.0f);
//   bt->wing[j].fsize = FDIV(INT2FNUM(crBpl),16711680);
               bt->wing[j].fsize = FDIV (INT2FNUM (crBpl), 16711680);

//   drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->wing[j]));
//   drawBossWingx(fx1, fy1, fx2, fy2, &(bt->wing[j]));
//   drawBossWingx(fx1, fy1, 0, fx2, fy2, 0, &(bt->wing[j]));
//   drawBossWingx(fx1, fy1, fz1, fx2, fy2, fz2, &(bt->wing[j]));
               drawBossWingx (fx1, fy1, fx2, fy2, &(bt->wing[j]));
            }
            break;
         case CHANGE:
            break;
         }
         if (bt->diffuse > 0 && boss.state != CHANGE
             && boss.state < DESTROIED) {
            df = bt->diffuse;
            //senquack TODO: why did I leave these two commented-out for Wiz port? I re-enabled them again:
// drawStar(0, x2, y2, z2, df, df, df, (float)(df+256)/900.0f);
// drawStar(0, x2, y2, z2, df, df, df, (float)(df+randN(256))/900.0f);
            drawStarx(0, x2, y2, df, df, df, FDIV(INT2FNUM(df+256), f2x(900.0)));
            drawStarx(0, x2, y2, df, df, df, FDIV(INT2FNUM(df+randN(256)), f2x(900.0)));
         }
//      x1 = x2; y1 = y2; z1 = z2;
//      fx1 = fx2; fy1 = fy2; fz1 = fz2;
         fx1 = fx2; fy1 = fy2;
      }

//    x1 = x + bt->x[bpn];
//    y1 = y - bt->y[bpn];
//    z1 = bt->z[bpn];
      fx1 = fx + bt->fx[bpn];
      fy1 = fy - bt->fy[bpn];
//    fz1 = bt->fz[bpn];

      for (j = 0; j < bt->epNum; j++) {
//      x2 = x + bt->ex[j];
//      y2 = y - bt->ey[j];
//      z2 = bt->ez[j];
         fx2 = fx + bt->fex[j];
         fy2 = fy - bt->fey[j];
//      fz2 = bt->fez[j];

         switch (boss.state) {
         case ATTACKING:
         case LAST_ATTACK:
         case DESTROIED:
// drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 220);
            drawLinex (fx1, fy1, fx2, fy2, bossShape.r, bossShape.g,
                       bossShape.b, 220);
//   glColor4hack(bossShape.r, bossShape.g, bossShape.b, 220);
//   glBegin(GL_LINE_LOOP);
//   glVertex3f(x1, y1, z1);
//   glVertex3f(x2, y2, z2);
//   glEnd();

// drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->eWing[j]));
// drawBossWingx(fx1, fy1, fz1, fx2, fy2, fz2, &(bt->eWing[j]));
            drawBossWingx (fx1, fy1, fx2, fy2, &(bt->eWing[j]));
            break;
         case CREATING:
            if (crBpn == bpn) {
//   drawLinePart(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 220, crBpl);
               drawLinePartx (fx1, fy1, fx2, fy2, bossShape.r, bossShape.g,
                              bossShape.b, 220, crBpl);
//   bt->eWing[j].size = (float)crBpl/255;
//   bt->eWing[j].fsize = INT2FNUM(crBpl>>8);
//   bt->eWing[j].fsize =  f2x((float)crBpl/255.0f); 
               bt->eWing[j].fsize = FDIV (INT2FNUM (crBpl), 16711680);

//   drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->eWing[j]));
//   drawBossWingx(fx1, fy1, fz1, fx2, fy2, fz2, &(bt->eWing[j]));
               drawBossWingx (fx1, fy1, fx2, fy2, &(bt->eWing[j]));
            }
            break;
         case CHANGE:
// drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 220);
            drawLinex (fx1, fy1, fx2, fy2, bossShape.r, bossShape.g,
                       bossShape.b, 220);
//   glColor4hack(bossShape.r, bossShape.g, bossShape.b, 220);
//   glBegin(GL_LINE_LOOP);
//   glVertex3f(x1, y1, z1);
//   glVertex3f(x2, y2, z2);
//   glEnd();

            if (crBpn == bpn) {
//   bt->eWing[j].size = (float)crBpl/128;
//   bt->eWing[j].fsize = INT2FNUM(crBpl>>7);
//   bt->eWing[j].fsize = f2x((float)crBpl/128.0f);
               bt->eWing[j].fsize = FDIV (INT2FNUM (crBpl), 8388608);
//   drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->eWing[j]));
//   drawBossWingx(fx1, fy1, fz1, fx2, fy2, fz2, &(bt->eWing[j]));
               drawBossWingx (fx1, fy1, fx2, fy2, &(bt->eWing[j]));
            }
            break;
         }
         if (bt->diffuse > 0 && boss.state != CHANGE
             && boss.state < DESTROIED) {
            df = bt->diffuse;
// drawStar(1, x2, y2, z2, df, df, df, (float)(df+256)/640.0f);
// drawStar(1, x2, y2, z2, df, df, df, (float)(df+randN(256))/640.0f);
            drawStarx (1, fx2, fy2, df, df, df,
                       FDIV (INT2FNUM (df + 256), INT2FNUM (640)));
            drawStarx (1, fx2, fy2, df, df, df,
                       FDIV (INT2FNUM (df + randN (256)), INT2FNUM (640)));
         }
      }
   }
   finishDrawBossWingsx ();

//  drawCore(x, y, boss.cnt, boss.r, boss.g, boss.b);
//  drawCorex(f2x(x), f2x(y), boss.cnt, boss.r, boss.g, boss.b);
   drawCorex (fx, fy, boss.cnt, boss.r, boss.g, boss.b);
#else
   prepareDrawBossWings();
  float x, y;
  // senquack - converted to 2D for speedup:
//  float x1, y1, z1, x2, y2, z2;
   float x1, y1, x2, y2;

   int i, j;
   int df;
   int crBpn, crBpl;
   int bpn;
   crBpn = crBpl = 0;

   //senquack - BIG TODO: create constant float inverse of FIELD_SCREEN_RATIO and multiply instead of divide for this 
   //       and many other sections of code, including fixed version of this above:
   x =  (float)boss.x / FIELD_SCREEN_RATIO;
   y = -(float)boss.y / FIELD_SCREEN_RATIO;

   if (bossShape.diffuse > 0 && boss.state < DESTROIED) {
      df = bossShape.diffuse;
      // converted to 2D:
//    drawStar(1, x, y, 0, df, df, df, (float)(df+256)/500.0f);
//    drawStar(1, x, y, 0, df, df, df, (float)(df+randN(256))/500.0f);
      drawStar(1, x, y, df, df, df, (float)(df+256)/500.0f);
      drawStar(1, x, y, df, df, df, (float)(df+randN(256))/500.0f);
   }

   for (i = 0; i < boss.batteryGroupNum; i++) {
      BossTree *bt = &(bossShape.tree[i]);
      bpn = bt->posNum - 1;
//    x1 = x; y1 = y; z1 = 0;
      x1 = x; y1 = y;
      switch (boss.state) {
      case CREATING:
      case CHANGE:
         crBpn =
            (bpn + 1) * (BOSS_PATTERN_CHANGE_CNT - boss.stateCnt -
                         1) / BOSS_PATTERN_CHANGE_CNT;
         crBpl =
            255 -
            (boss.stateCnt % (BOSS_PATTERN_CHANGE_CNT / (bpn + 1)) * 256) /
            (BOSS_PATTERN_CHANGE_CNT / (bpn + 1));
         break;
      }
      for (j = 0; j < bpn; j++) {

      x2 =  x + bt->x[j+1];
      y2 =  y - bt->y[j+1];
//      z2 =  bt->z[j+1];

         switch (boss.state) {
         case ATTACKING:
         case LAST_ATTACK:
         case DESTROIED:
// drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 240);
            drawLine(x1, y1, x2, y2, bossShape.r, bossShape.g, bossShape.b, 240);

// drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->wing[j]));
            drawBossWing(x1, y1, x2, y2, &(bt->wing[j]));
            break;
         case CREATING:
            if (j == crBpn) {
//   drawLinePart(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 240, crBpl);
               drawLinePart(x1, y1, x2, y2, bossShape.r, bossShape.g, bossShape.b, 240, crBpl);
            } else if (j < crBpn) {
//   drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 240);
               drawLine(x1, y1, x2, y2, bossShape.r, bossShape.g, bossShape.b, 240);
            }
            if (crBpn == bpn) {
               //senquack TODO: can we optimize this by shifting crBpl before divide?:
               bt->wing[j].size = (float)crBpl/255;
//   drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->wing[j]));
               drawBossWing(x1, y1, x2, y2, &(bt->wing[j]));
            }
            break;
         case CHANGE:
            break;
         }
         if (bt->diffuse > 0 && boss.state != CHANGE && boss.state < DESTROIED) {
            df = bt->diffuse;
            //senquack TODO: why did I leave these two commented-out for Wiz port? I re-enabled them again:
//            drawStar(0, x2, y2, z2, df, df, df, (float)(df+256)/900.0f);
//            drawStar(0, x2, y2, z2, df, df, df, (float)(df+randN(256))/900.0f);
            drawStar(0, x2, y2, df, df, df, (float)(df+256)/900.0f);
            drawStar(0, x2, y2, df, df, df, (float)(df+randN(256))/900.0f);
         }
//      x1 = x2; y1 = y2; z1 = z2;
         x1 = x2; y1 = y2; 
      }

      x1 = x + bt->x[bpn];
      y1 = y - bt->y[bpn];
//    z1 = bt->z[bpn];

      for (j = 0; j < bt->epNum; j++) {
         x2 = x + bt->ex[j];
         y2 = y - bt->ey[j];
//      z2 = bt->ez[j];

         switch (boss.state) {
         case ATTACKING:
         case LAST_ATTACK:
         case DESTROIED:
// drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 220);
            drawLine(x1, y1, x2, y2, bossShape.r, bossShape.g, bossShape.b, 220);

// drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->eWing[j]));
            drawBossWing(x1, y1, x2, y2, &(bt->eWing[j]));
            break;
         case CREATING:
            if (crBpn == bpn) {
//   drawLinePart(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 220, crBpl);
               drawLinePart(x1, y1, x2, y2, bossShape.r, bossShape.g, bossShape.b, 220, crBpl);
               //senquack TODO: possible optimization:
               bt->eWing[j].size = (float)crBpl/255;

//   drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->eWing[j]));
               drawBossWing(x1, y1, x2, y2, &(bt->eWing[j]));
            }
            break;
         case CHANGE:
// drawLine(x1, y1, z1, x2, y2, z2, bossShape.r, bossShape.g, bossShape.b, 220);
            drawLine(x1, y1, x2, y2, bossShape.r, bossShape.g, bossShape.b, 220);

            if (crBpn == bpn) {
               bt->eWing[j].size = (float)crBpl/128;
//   drawBossWing(x1, y1, z1, x2, y2, z2, &(bt->eWing[j]));
               drawBossWing(x1, y1, x2, y2, &(bt->eWing[j]));
            }
            break;
         }
         if (bt->diffuse > 0 && boss.state != CHANGE
             && boss.state < DESTROIED) {
            df = bt->diffuse;
// drawStar(1, x2, y2, z2, df, df, df, (float)(df+256)/640.0f);
// drawStar(1, x2, y2, z2, df, df, df, (float)(df+randN(256))/640.0f);
            drawStar(1, x2, y2, df, df, df, (float)(df+256)/640.0f);
            drawStar(1, x2, y2, df, df, df, (float)(df+randN(256))/640.0f);
         }
      }
   }
   finishDrawBossWings();

   drawCore(x, y, boss.cnt, boss.r, boss.g, boss.b);
#endif //FIXEDMATH
}


//senquack - some fixed point
//void drawBossState() {
//  int wd, cwd;
//  if ( boss.state >= ATTACKING ) {
//    if ( boss.state < DESTROIED || (boss.cnt&31) < 16 ) {
//      drawTimeCenter(bossTimer, 470, 44, 10, 210, 240, 210);
//    }
//    if ( boss.state == DESTROIED_END ) {
//      drawBossScoreAtr();
//    }
//  }
//  if ( boss.state >= DESTROIED ) return;
//  if ( boss.state == CREATING ) {
//    wd = boss.shield*300/BOSS_SHIELD_MAX *
//      (BOSS_PATTERN_CHANGE_CNT-boss.stateCnt)/BOSS_PATTERN_CHANGE_CNT;
//  } else {
//    wd = boss.shield*300/BOSS_SHIELD_MAX;
//  }
//  drawBox(180+wd/2, 24, wd/2, 6, 240, 240, 210);
//  drawNumCenter(boss.shield, 176+wd, 10, 6, 210, 210, 240);
//  cwd = boss.patternChangeShield*300/BOSS_SHIELD_MAX;
//  if ( wd > cwd ) {
//    drawNumCenter(boss.patternChangeShield, 176+cwd, 10, 6, 240, 210, 210);
//  }
//}
void
drawBossState ()
{
   int wd, cwd;
   if (boss.state >= ATTACKING) {
      if (boss.state < DESTROIED || (boss.cnt & 31) < 16) {
         drawTimeCenter (bossTimer, 470, 44, 10, 210, 240, 210);
      }
      if (boss.state == DESTROIED_END) {
         drawBossScoreAtr ();
      }
   }
   if (boss.state >= DESTROIED)
      return;
   if (boss.state == CREATING) {
      wd = boss.shield * 300 / BOSS_SHIELD_MAX *
         (BOSS_PATTERN_CHANGE_CNT - boss.stateCnt) / BOSS_PATTERN_CHANGE_CNT;
   } else {
      wd = boss.shield * 300 / BOSS_SHIELD_MAX;
   }
#ifdef FIXEDMATH
   drawBoxx (INT2FNUM (180 + (wd >> 1)), INT2FNUM (24), INT2FNUM (wd >> 1),
             INT2FNUM (6), 240, 240, 210);
#else
//   drawBox(180+wd/2, 24, wd/2, 6, 240, 240, 210);
   drawBox((float)(180+wd/2), 24.0, (float)(wd/2), 6.0, 240, 240, 210);
#endif //FIXEDMATH

//senquack TODO: investigate why I changed 176+wd to 165 here: (probably to allow more stuff displayed up top at low-res)
//  drawNumCenter(boss.shield, 176+wd, 10, 6, 210, 210, 240);
   drawNumCenter (boss.shield, 165 + wd, 10, 6, 210, 210, 240);
   cwd = boss.patternChangeShield * 300 / BOSS_SHIELD_MAX;
   if (wd > cwd) {
//    drawNumCenter(boss.patternChangeShield, 176+cwd, 10, 6, 240, 210, 210);
      drawNumCenter (boss.patternChangeShield, 165 + cwd, 10, 6, 240, 210,
                     210);
   }
}

void
drawBossState_rotated ()
{
   int wd, cwd;
   if (boss.state >= ATTACKING) {
      if (boss.state < DESTROIED || (boss.cnt & 31) < 16) {
//senquack - move it down a bit because we are putting a box above it
//      drawTimeCenter(bossTimer, 470, 44, 10, 210, 240, 210);
         drawTimeCenter (bossTimer, 470, 50, 9, 210, 240, 210);
      }
      if (boss.state == DESTROIED_END) {
         drawBossScoreAtr ();
      }
   }
   if (boss.state >= DESTROIED)
      return;
   if (boss.state == CREATING) {
      wd = boss.shield * 300 / BOSS_SHIELD_MAX *
         (BOSS_PATTERN_CHANGE_CNT - boss.stateCnt) / BOSS_PATTERN_CHANGE_CNT;
   } else {
      wd = boss.shield * 300 / BOSS_SHIELD_MAX;
   }
#ifdef FIXEDMATH
   drawBoxx (INT2FNUM (180 + (wd >> 1)), INT2FNUM (24), INT2FNUM (wd >> 1),
             INT2FNUM (6), 240, 240, 210);
#else
//  drawBox(180+wd/2, 24, wd/2, 6, 240, 240, 210);
   drawBox((float)(180+wd/2), 24.0, (float)(wd/2), 6.0, 240, 240, 210);
#endif //FIXEDMATH

//senquack TODO: investigate why I changed 176+wd to 165 here: (probably to allow more stuff displayed up top at low-res)
//  drawNumCenter(boss.shield, 176+wd, 10, 6, 210, 210, 240);
   drawNumCenter (boss.shield, 165 + wd, 10, 6, 210, 210, 240);
   cwd = boss.patternChangeShield * 300 / BOSS_SHIELD_MAX;
   if (wd > cwd) {
//    drawNumCenter(boss.patternChangeShield, 176+cwd, 10, 6, 240, 210, 210);
      drawNumCenter (boss.patternChangeShield, 165 + cwd, 10, 6, 240, 210,
                     210);
   }
}
