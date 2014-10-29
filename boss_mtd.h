/*
 * $Id: boss_mtd.h,v 1.2 2003/03/21 02:59:48 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * Boss methods.
 *
 * @version $Revision: 1.2 $
 */
#ifndef BOSS_MTD_H
#define BOSS_MTD_H
#include "vector.h"

//senquack TODO: make sure conversion to floats from doubles here didn't mess up the bullet patterns, etc:
//senquack - complete conversion to floats:
//void createBoss(int seed, double rank, int round);
void createBoss (int seed, float rank, int round);
void initBoss ();
void moveBoss ();
void drawBoss ();
int checkHitDownside (int x);
int checkHitUpside ();
void damageBoss (int dmg);
void damageBossLaser (int cnt);
void weakenBoss ();
void drawBossState ();
//senquack - for when the screen is rotated:
void drawBossState_rotated ();
Vector *getBossPos ();
#endif //BOSS_MTD_H
