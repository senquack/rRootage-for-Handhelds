/*
 * $Id: foecommand.h,v 1.1.1.1 2003/03/16 07:03:49 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * Foe commands data.
 *
 * @version $Revision: 1.1.1.1 $
 */
#ifndef FOECOMMAND_H_
#define FOECOMMAND_H_

#include "bulletml/bulletmlparser.h"
#include "bulletml/bulletmlparser-tinyxml.h"
#include "bulletml/bulletmlrunner.h"
#include "foe.h"

#define COMMAND_SCREEN_SPD_RATE 512
#define COMMAND_SCREEN_VEL_RATE 512

//senquack changed all doubles to floats:
class FoeCommand:public BulletMLRunner
{
 public:
   FoeCommand (BulletMLParser * parser, struct foe *f);
     FoeCommand (BulletMLState * state, struct foe *f);

     virtual ~ FoeCommand ();

   virtual float getBulletDirection ();
   virtual float getAimDirection ();
   virtual float getBulletSpeed ();
   virtual float getDefaultSpeed ();
   virtual float getRank ();
   virtual void createSimpleBullet (float direction, float speed);
   virtual void createBullet (BulletMLState * state, float direction,
                              float speed);
   virtual int getTurn ();
   virtual void doVanish ();

   virtual void doChangeDirection (float d);
   virtual void doChangeSpeed (float s);
   virtual void doAccelX (float ax);
   virtual void doAccelY (float ay);
   virtual float getBulletSpeedX ();
   virtual float getBulletSpeedY ();

 private:
   struct foe *foe;
};
#endif
