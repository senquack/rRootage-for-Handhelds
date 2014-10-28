#ifndef BULLETRUNNER_H_
#define BULLETRUNNER_H_

#include "bulletmltree.h"
#include "bulletmlcommon.h"

#include <vector>
#include <boost/smart_ptr.hpp>

class BulletMLParser;
class BulletMLNode;
class BulletMLRunnerImpl;

//senquack - complete conversion to floats:
//typedef std::vector<double> BulletMLParameter;
typedef
std::vector < float >
   BulletMLParameter;

class BulletMLState {
 public:
   DECLSPEC
      BulletMLState (BulletMLParser * bulletml,
                     const std::vector < BulletMLNode * >&node,
                     boost::shared_ptr < BulletMLParameter > para)
      :
   bulletml_ (bulletml),
   node_ (node.begin (), node.end ()),
   para_ (para)
   {
   }

   DECLSPEC BulletMLParser *
   getBulletML ()
   {
      return bulletml_;
   }
   DECLSPEC const
      std::vector <
   BulletMLNode * >&
   getNode () const
   {
      return
         node_;
   }
   DECLSPEC
      boost::shared_ptr <
      BulletMLParameter >
   getParameter ()
   {
      return para_;
   }

 private:
   BulletMLParser * bulletml_;
   std::vector < BulletMLNode * >node_;
   boost::shared_ptr < BulletMLParameter > para_;

};


class BulletMLRunner {
 public:
   DECLSPEC explicit BulletMLRunner (BulletMLParser * bulletml);
   DECLSPEC
      explicit
   BulletMLRunner (BulletMLState * state);
   DECLSPEC virtual ~ BulletMLRunner ();

   DECLSPEC void
   run ();

 public:
   DECLSPEC bool isEnd ()const;

 public:
//senquack - complete conversion to floats:
//  DECLSPEC virtual double getBulletDirection() =0;
//  DECLSPEC virtual double getAimDirection() =0;
//  DECLSPEC virtual double getBulletSpeed() =0;
//  DECLSPEC virtual double getDefaultSpeed() =0;
//  DECLSPEC virtual double getRank() =0;
//  DECLSPEC virtual void createSimpleBullet(double direction, double speed) =0;
//  DECLSPEC virtual void createBullet(BulletMLState* state,
//                                     double direction, double speed) =0;
//  DECLSPEC virtual int getTurn() =0;
//  DECLSPEC virtual void doVanish() =0;
//  DECLSPEC virtual void doChangeDirection(double) {}
//  DECLSPEC virtual void doChangeSpeed(double) {}
//  DECLSPEC virtual void doAccelX(double) {}
//  DECLSPEC virtual void doAccelY(double) {}
//  DECLSPEC virtual double getBulletSpeedX() { return 0; }
//  DECLSPEC virtual double getBulletSpeedY() { return 0; }
//  DECLSPEC virtual double getRand() { return (double)rand() / RAND_MAX; }

   DECLSPEC virtual float
   getBulletDirection () = 0;
   DECLSPEC virtual float
   getAimDirection () = 0;
   DECLSPEC virtual float
   getBulletSpeed () = 0;
   DECLSPEC virtual float
   getDefaultSpeed () = 0;
   DECLSPEC virtual float
   getRank () = 0;
   DECLSPEC virtual void
   createSimpleBullet (float direction, float speed) = 0;
   DECLSPEC virtual void
   createBullet (BulletMLState * state, float direction, float speed) = 0;
   DECLSPEC virtual int
   getTurn () = 0;
   DECLSPEC virtual void
   doVanish () = 0;
   DECLSPEC virtual void
   doChangeDirection (float)
   {
   }
   DECLSPEC virtual void
   doChangeSpeed (float)
   {
   }
   DECLSPEC virtual void
   doAccelX (float)
   {
   }
   DECLSPEC virtual void
   doAccelY (float)
   {
   }
   DECLSPEC virtual float
   getBulletSpeedX ()
   {
      return 0;
   }
   DECLSPEC virtual float
   getBulletSpeedY ()
   {
      return 0;
   }
   DECLSPEC virtual float
   getRand ()
   {
      return (float) rand () / RAND_MAX;
   }
 private:
   DECLSPEC virtual BulletMLRunnerImpl * makeImpl (BulletMLState * state);

 protected:
   std::vector < BulletMLRunnerImpl * >impl_;

};

#endif // ! BULLETRUNNER_H_
