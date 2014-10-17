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

//senquack - even more complete conversion to fixed point:
//struct foe {
//  Vector pos, vel, ppos, spos, mv;
//  int d, spd;
//  FoeCommand *cmd;
////senquack - complete conversion to floats:
////  double rank;
//  float rank;
//  int spc;
//  int cnt, cntTotal;
//  int xReverse;
//  int fireCnt;
//  int slowMvCnt;
//  BulletMLParser *parser;
//
//  BulletMLParser *morphParser[MORPH_PATTERN_MAX];
//  int morphCnt;
//  int morphHalf;
////senquack - complete conversion to floats:
////  double morphRank;
////  double speedRank;
//  float morphRank;
//  float speedRank;
//
//  int color;
//  int shapeType;
//  int bulletShape[3];
////  float bulletSize[3];
//  //senquack - fixed point version of above number to speed up rendering
//  GLfixed fbulletSize[3];   
//
//  struct limiter *limiter;
//
//  int ikaType;
//
//  int grzRng;
//};
struct foe
{
   Vector pos, vel, ppos, spos, mv;
   int d, spd;
   FoeCommand *cmd;
//senquack - complete conversion to floats:
//  double rank;
   GLfixed frank;
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
//  float morphRank;
//  float speedRank;
   GLfixed fmorphRank;
   GLfixed fspeedRank;

   int color;
   int shapeType;
   int bulletShape[3];
//  float bulletSize[3];
   //senquack - fixed point version of above number to speed up rendering
   GLfixed fbulletSize[3];

   struct limiter *limiter;

   int ikaType;

   int grzRng;
};

typedef struct foe Foe;

//senquack - complete conversion to floats/fixeds
//Foe* addFoeBattery(int x, int y, double rank, int d, int spd, int xReverse, 
//       BulletMLParser *morphParser[], int morphCnt, int morphHalf, double morphRank,
//       double speedRank,
//       int color, int bulletShape[], float bulletSize[],
//       struct limiter *limiter,
//       int ikaType,
//       BulletMLParser *parser);
//senquack - even more complete conversion:
//Foe* addFoeBattery(int x, int y, float rank, int d, int spd, int xReverse, 
//       BulletMLParser *morphParser[], int morphCnt, int morphHalf, float morphRank,
//       float speedRank,
//       int color, int bulletShape[], GLfixed fbulletSize[],
//       struct limiter *limiter,
//       int ikaType,
//       BulletMLParser *parser);
Foe *addFoeBattery (int x, int y, GLfixed frank, int d, int spd, int xReverse,
                    BulletMLParser * morphParser[], int morphCnt,
                    int morphHalf, GLfixed fmorphRank, GLfixed fspeedRank,
                    int color, int bulletShape[], GLfixed fbulletSize[],
                    struct limiter *limiter, int ikaType,
                    BulletMLParser * parser);
void addFoeActiveBullet (Foe * foe, int d, int spd, int color,
                         BulletMLState * state);
void addFoeNormalBullet (Foe * foe, int d, int spd, int color);
void removeFoeCommand (Foe * fe);
void removeFoe (Foe * fe);
void removeFoeForced (Foe * fe);
void wipeBullets (Vector * pos, int width);
#endif
