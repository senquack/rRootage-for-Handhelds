/*
 * $Id: foe.h,v 1.5 2003/08/15 07:06:52 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * Battery/Bullet data.
 *
 * @version $Revision: 1.5 $
 */
#ifndef FOE_H_
#define FOE_H_

extern "C"
{
#include "vector.h"
#include "foe_mtd.h"
}

#include "bulletml/bulletmlparser.h"
#include "bulletml/bulletmlparser-tinyxml.h"
#include "bulletml/bulletmlrunner.h"
#include "foecommand.h"
#include "barragemanager.h"
#include "boss.h"

//senquack - for GLfixed type
typedef int GLfixed;

#define BATTERY 0
#define ACTIVE_BULLET 1
#define BULLET 2

#define NOT_EXIST_TMP (NOT_EXIST-1)

//senquack TODO: make sure conversion to floats from doubles here didn't mess up the bullet patterns, etc:
struct foe
{
   Vector pos, vel, ppos, spos, mv;
   int d, spd;
   FoeCommand *cmd;
//  double rank;
#ifdef FIXEDMATH
   GLfixed frank;
#else
   float rank;
#endif //FIXEDMATH
   int spc;
   int cnt, cntTotal;
   int xReverse;
   int fireCnt;
   int slowMvCnt;
   BulletMLParser *parser;

   BulletMLParser *morphParser[MORPH_PATTERN_MAX];
   int morphCnt;
   int morphHalf;
//senquack - complete conversion to floats:
//  double morphRank;
//  double speedRank;
#ifdef FIXEDMATH
   GLfixed fmorphRank;
   GLfixed fspeedRank;
#else
   float morphRank;
   float speedRank;
#endif //FIXEDMATH

   int color;
   int shapeType;
   int bulletShape[3];
#ifdef FIXEDMATH
   GLfixed fbulletSize[3];
#else
   float bulletSize[3];
#endif //FIXEDMATH

   struct limiter *limiter;

   int ikaType;

   int grzRng;
};

typedef struct foe Foe;

//senquack TODO: make sure conversion to floats from doubles here didn't mess up the bullet patterns, etc:
//senquack - complete conversion to floats/fixeds
//Foe* addFoeBattery(int x, int y, double rank, int d, int spd, int xReverse, 
//       BulletMLParser *morphParser[], int morphCnt, int morphHalf, double morphRank,
//       double speedRank,
//       int color, int bulletShape[], float bulletSize[],
//       struct limiter *limiter,
//       int ikaType,
//       BulletMLParser *parser);
#ifdef FIXEDMATH
Foe* addFoeBattery (int x, int y, GLfixed frank, int d, int spd, int xReverse,
                    BulletMLParser * morphParser[], int morphCnt,
                    int morphHalf, GLfixed fmorphRank, GLfixed fspeedRank,
                    int color, int bulletShape[], GLfixed fbulletSize[],
                    struct limiter *limiter, int ikaType,
                    BulletMLParser * parser);
#else
Foe* addFoeBattery(int x, int y, float rank, int d, int spd, int xReverse, 
       BulletMLParser *morphParser[], int morphCnt, int morphHalf, float morphRank,
       float speedRank,
       int color, int bulletShape[], float bulletSize[],
       struct limiter *limiter,
       int ikaType,
       BulletMLParser *parser);
#endif //FIXEDMATH

void addFoeActiveBullet (Foe * foe, int d, int spd, int color,
                         BulletMLState * state);
void addFoeNormalBullet (Foe * foe, int d, int spd, int color);
void removeFoeCommand (Foe * fe);
void removeFoe (Foe * fe);
void removeFoeForced (Foe * fe);
void wipeBullets (Vector * pos, int width);
#endif
