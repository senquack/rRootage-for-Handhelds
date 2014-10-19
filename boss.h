/*
 * $Id: boss.h,v 1.3 2003/04/04 03:46:20 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * Boss data.
 *
 * @version $Revision: 1.3 $
 */
#ifndef BOSS_H_
#define BOSS_H_

#define MORPH_PATTERN_MAX 8

#include "foe.h"


//senquack:
#include <GLES/gl.h>
#include <stdint.h>

#define IKA_FIX 0
#define IKA_ALTERNATE 1
#define IKA_HALF 2
#define IKA_ALTERNATE_SHOT 3

//senquack - conversion to fixed point:
//typedef struct {
//  int barrageType, barrageIdx;
////senquack - complete conversion to floats:
////  double rank;
//  float rank;
//  int xReverse ,xrAlter;
//  int morphIdx[MORPH_PATTERN_MAX];
//  int morphCnt;
//  int morphHalf;
//  int morphType;
////senquack - complete conversion to floats:
////  double morphRank;
////  double speedRank;
//  float morphRank;
//  float speedRank;
//  int ikaType;
//} Attack;
typedef struct
{
   int barrageType, barrageIdx;
   int xReverse, xrAlter;
   int morphIdx[MORPH_PATTERN_MAX];
   int morphCnt;
   int morphHalf;
   int morphType;
   int ikaType;
#ifdef FIXEDMATH
   //senquack - converted to fixed point:
   GLfixed frank;
   GLfixed fmorphRank;
   GLfixed fspeedRank;
#else
   //senquack - TODO: make sure converting from double to float here didn't mess anything up (bullet paterns, etc)
   // converted these to floats from doubles:
   float rank;
   float morphRank; 
   float speedRank;
#endif //FIXEDMATH
} Attack;

typedef struct
{
   struct foe *foe;
   int x, y;
} Battery;

//senquack - partial conversion to fixed point 
//typedef struct
//{
//   int color;
//   int bulletShape[BULLET_TYPE_NUM];
//   float bulletSize[BULLET_TYPE_NUM];
//} BatteryShape;
typedef struct
{
   int color;
   int bulletShape[BULLET_TYPE_NUM];
#ifdef FIXEDMATH
   GLfixed fbulletSize[BULLET_TYPE_NUM];
#else
   float bulletSize[BULLET_TYPE_NUM];
#endif //FIXEDMATH
} BatteryShape;

struct limiter
{
   int cnt, max;
   int on;
};

typedef struct limiter Limiter;

#define BATTERY_PATTERN_MAX 6
#define BATTERY_MAX 3

typedef struct
{
   Attack attack[BATTERY_PATTERN_MAX];
   Battery battery[BATTERY_MAX];
   int batteryNum;
   BatteryShape shape;
   Limiter limiter;
} BatteryGroup;

#define BATTERY_GROUP_MAX 10
#define MOVE_POINT_MAX 4
#define COLLISION_NUM 5

#define CREATING 0
#define ATTACKING 1
#define CHANGE 2
#define LAST_ATTACK 3
#define DESTROIED 4
#define DESTROIED_END 5

//senquack - conversion to fixed-point:
//typedef struct {
//  Attack topAttack[BATTERY_PATTERN_MAX];
//  Battery topBattery;
//  Limiter topLimiter;
//  BatteryShape shape;
//  BatteryGroup batteryGroup[BATTERY_GROUP_MAX];
//  int batteryGroupNum;
//  int x, y, d;
//  int mpx[MOVE_POINT_MAX], mpy[MOVE_POINT_MAX];
//  int mpNum, mpIdx;
//  int speed, md;
//  int onRoute;
//  int patternIdx, patternNum;
//  int patternCnt, patternLgt;
//
//  int color;
//  int bulletShape[3];
//  float bulletSize[3];
//
//  int collisionX[COLLISION_NUM], collisionY[COLLISION_NUM], collisionYUp;
//  int shield, patternChangeShield;
//  int damaged, damageCnt, cnt;
//  int state, stateCnt;
//  int r, g, b;
//} Boss;
typedef struct
{
   Attack topAttack[BATTERY_PATTERN_MAX];
   Battery topBattery;
   Limiter topLimiter;
   BatteryShape shape;
   BatteryGroup batteryGroup[BATTERY_GROUP_MAX];
   int batteryGroupNum;
   int x, y, d;
   int mpx[MOVE_POINT_MAX], mpy[MOVE_POINT_MAX];
   int mpNum, mpIdx;
   int speed, md;
   int onRoute;
   int patternIdx, patternNum;
   int patternCnt, patternLgt;

   int color;
   int bulletShape[3];
#ifdef FIXEDMATH
   GLfixed fbulletSize[3];
#else
   float bulletSize[3];
#endif //FIXEDMATH

   int collisionX[COLLISION_NUM], collisionY[COLLISION_NUM], collisionYUp;
   int shield, patternChangeShield;
   int damaged, damageCnt, cnt;
   int state, stateCnt;
   int r, g, b;
} Boss;

#define BOSS_WING_MAX 2

//senquack - converting to fixed point
//typedef struct {
//  float x[BOSS_WING_MAX][2], y[BOSS_WING_MAX][2], z[BOSS_WING_MAX][2];
//  int wingNum;
//  float size;
//} BossWing;
typedef struct
{
   int wingNum;
#ifdef FIXEDMATH
   GLfixed fx[BOSS_WING_MAX][2], fy[BOSS_WING_MAX][2], fz[BOSS_WING_MAX][2];
   GLfixed fsize;
#else
   float x[BOSS_WING_MAX][2], y[BOSS_WING_MAX][2], z[BOSS_WING_MAX][2];
   float size;
#endif //FIXEDMATH
} BossWing;

#define TREE_MAX_LENGTH 5

//senquack - converting to fixed point
//typedef struct {
//  float x[TREE_MAX_LENGTH], y[TREE_MAX_LENGTH], z[TREE_MAX_LENGTH];
//  float ex[BATTERY_MAX], ey[BATTERY_MAX], ez[BATTERY_MAX];
//  BossWing wing[TREE_MAX_LENGTH], eWing[BATTERY_MAX];
//  int posNum, epNum;
//  int diffuse;
//} BossTree;
typedef struct
{
#ifdef FIXEDMATH
   GLfixed fx[TREE_MAX_LENGTH], fy[TREE_MAX_LENGTH], fz[TREE_MAX_LENGTH];
   GLfixed fex[BATTERY_MAX], fey[BATTERY_MAX], fez[BATTERY_MAX];
#else
   float x[TREE_MAX_LENGTH], y[TREE_MAX_LENGTH], z[TREE_MAX_LENGTH];
   float ex[BATTERY_MAX], ey[BATTERY_MAX], ez[BATTERY_MAX];
#endif //FIXEDMATH
   BossWing wing[TREE_MAX_LENGTH], eWing[BATTERY_MAX];
   int posNum, epNum;
   int diffuse;
} BossTree;

typedef struct
{
   BossTree tree[BATTERY_GROUP_MAX];
   int r, g, b;
   int diffuse;
} BossShape;

extern "C"
{
#include "boss_mtd.h"
}
#endif
