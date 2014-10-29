/*
 * $Id: laser.h,v 1.1.1.1 2003/03/16 07:03:49 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * Players laser.
 *
 * @version $Revision: 1.1.1.1 $
 */
#ifndef LASER_H
#define LASER_H

#include "vector.h"

typedef struct
{
   int y;
   int color;
   int cnt;
} Laser;

void initLasers ();
void moveLasers ();
void drawLasers ();
void addLaser ();
#endif // LASER_H
