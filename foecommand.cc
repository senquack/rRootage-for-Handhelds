/*
 * $Id: foecommand.cc,v 1.2 2003/08/15 07:06:52 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * Handle bullet commands.
 *
 * @version $Revision: 1.2 $
 */
#include "bulletml/bulletmlparser.h"
#include "bulletml/bulletmlparser-tinyxml.h"
#include "bulletml/bulletmlrunner.h"
#include "foe.h"

extern "C"
{
#include "rr.h"
#include "genmcr.h"
#include "degutil.h"
#include "ship.h"
}

//senquack BIG TODO: For Wiz, I had left all these member funcs as returning double.. I could probably convert them
//       to floats for speedup:
//senquack TODO: make sure conversion to floats from doubles here didn't mess up the bullet patterns, etc:

//senquack - converted all doubles to floats:
FoeCommand::FoeCommand (BulletMLParser * parser, Foe * f)
   :
BulletMLRunner (parser)
{
   foe = f;
}

FoeCommand::FoeCommand (BulletMLState * state, Foe * f)
: BulletMLRunner (state)
{
   foe = f;
}

FoeCommand::~FoeCommand ()
{
}

float
FoeCommand::getBulletDirection ()
{
   return (float) foe->d * 360 / DIV;
}

float
FoeCommand::getAimDirection ()
{
   int d = getPlayerDeg (foe->pos.x, foe->pos.y);
   if (foe->xReverse == -1)
      d = (-d) & 1023;
   return ((float) d * 360 / DIV);
}

float
FoeCommand::getBulletSpeed ()
{
   return ((float) foe->spd) / COMMAND_SCREEN_SPD_RATE;
}

float
FoeCommand::getDefaultSpeed ()
{
   return 1;
}

//senquack - converted to fixed point: (not sure if this is even ever called)
//senquack TODO: make sure conversion to floats from doubles here didn't mess up the bullet patterns, etc:
//double FoeCommand::getRank() {
//  return foe->rank;
//}
float
FoeCommand::getRank ()
{
#ifdef FIXEDMATH
//senquack TODO: poss. optimization, see if this is ever called:
   return x2f(foe->frank);
#else
   return foe->rank;
#endif //FIXEDMATH
}

void
FoeCommand::createSimpleBullet (float direction, float speed)
{
   int d = (int) (direction * DIV / 360);
   d &= (DIV - 1);
   addFoeNormalBullet (foe, d, (int) (speed * COMMAND_SCREEN_SPD_RATE),
                       foe->color + 1);
   foe->fireCnt++;
}

void
FoeCommand::createBullet (BulletMLState * state, float direction,
                          float speed)
{
   int d = (int) (direction * DIV / 360);
   d &= (DIV - 1);
   addFoeActiveBullet (foe, d, (int) (speed * COMMAND_SCREEN_SPD_RATE),
                       foe->color + 1, state);
   foe->fireCnt++;
}

int
FoeCommand::getTurn ()
{
   return tick;
}

void
FoeCommand::doVanish ()
{
   removeFoeCommand (foe);
}

void
FoeCommand::doChangeDirection (float d)
{
   foe->d = (int) (d * DIV / 360);
}

void
FoeCommand::doChangeSpeed (float s)
{
   foe->spd = (int) (s * COMMAND_SCREEN_SPD_RATE);
}

void
FoeCommand::doAccelX (float ax)
{
   foe->vel.x = (int) (ax * COMMAND_SCREEN_VEL_RATE);
}

void
FoeCommand::doAccelY (float ay)
{
   foe->vel.y = (int) (ay * COMMAND_SCREEN_VEL_RATE);
}

float
FoeCommand::getBulletSpeedX ()
{
   return ((float) foe->vel.x / COMMAND_SCREEN_VEL_RATE);
}

float
FoeCommand::getBulletSpeedY ()
{
   return ((float) foe->vel.y / COMMAND_SCREEN_VEL_RATE);
}
