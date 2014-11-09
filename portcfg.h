#ifndef PORTCFG_H
#define PORTCFG_H
//senquack - portcfg.h is a new include file that holds port-specific settings

/***************** WIZ / GP2X SETTINGS ***************/
//TODO: clean these up and get them working with new portcfg code
#if defined(WIZ) || defined(GP2X)

#define MAX_CPU_FREQ		(990)
#define MIN_CPU_FREQ		(540)
#define NUM_DEFS 				(8) //number of button redefinition settings
#define FIRE_IDX				(0)
#define FIRE2_IDX				(1)
#define SPECIAL_IDX			(2)
#define SPECIAL2_IDX			(3)
#define PAUSE_IDX				(4)
#define EXIT_IDX				(5)
#define VOLDOWN_IDX			(6)
#define VOLUP_IDX				(7)

#define NUM_BUTTONS  (19)       // total number of buttons we'll need to keep track of
#endif //WIZ/GP2X

/**************** CUSTOM PORT SETTINGS *****************/
enum {   SCREEN_HORIZ,
         SCREEN_ROTATED_LEFT,
         SCREEN_ROTATED_RIGHT,
         NUM_ROTATIONS
};

enum {
   DRAW_OUTLINES_NONE,
   DRAW_OUTLINES_IKA,
   DRAW_OUTLINES_ALL,
   NUM_DRAW_OUTLINES
};

typedef struct portcfg_settings
{
   int laser_on_by_default;   
   int rotated;               // Is screen rotated? Assigned to one of: 
                              //    SCREEN_HORIZ, SCREEN_ROTATED_LEFT, SCREEN_ROTATED_RIGHT
   int music;                 // Is music enabled?
   int analog_deadzone;       // Analog joystick deadzone
   int draw_outlines;         // Which bullet outlines to draw, if any
   int extra_lives;           // Cheat which adds up to 6 extra lives at start 
                              // (but disables ability to save new high scores)
   int extra_bombs;           // Cheat which adds up to 6 extra bomb at start 
                              // (but disables ability to save new high scores)
   int no_wait;               // Enables the --nowait option, where automatic bullet slowdown (and fps limiting) is disabled
   int show_fps;              // Show FPS counter
   struct {
      int move;      //Movement mapping
      int btn1;      //Laser mapping
      int btn2;      //Bomb mapping
      int btn1_alt;  //Laser alternate mapping
      int btn2_alt;  //Bomb alternate mapping
      int pause;     //Pause mapping
      int exit;      //Exit to menu mapping 
   } map;

//#if defined(WIZ) || defined(GP2X)
//      // LEFTOVER CRUFT FROM WIZ PORT THAT MAYBE WILL EVENTUALLY BE MERGED BACK IN:
//   int cpu_freq;                // if this is 0, overclocking is disabled
//   int fast_ram;
//   int buttons[NUM_DEFS], rbuttons[NUM_DEFS];   //rbuttons is for rotated button definitions
//#endif //WIZ/GP2X
} portcfg_settings;

#ifdef GCW
//senquack - new controls handling code:

enum {   // For accessing our internal controls state (order of these in list does not matter, but recommend C_NONE be 0)
   CNONE,           // This is for when a control is not mapped to anything
   CUP,
   CDOWN,
   CLEFT,
   CRIGHT,
   CA,
   CB,
   CX,
   CY,
   CL,
   CR,
   CSELECT,
   CSTART,
   CANALOGUP,
   CANALOGDOWN,
   CANALOGLEFT,
   CANALOGRIGHT,
   C_ANY_DPAD,       // This is for seeing if *any* direction of the DPAD is pressed
   C_ANY_ABXY,       // This is for seeing if *any* of the A/B/X/Y buttons are pressed
   C_ANY_ANALOG,     // This is for seeing if *any* direction of the analog stick is pressed
   C_ANY_DPAD_OR_ANALOG,
   CNUMCONTROLS
};


enum {   // For mapping internal to  externally-configurable controls (order of these in list MATTERS)
   MAP_NONE,
   MAP_A,
   MAP_B,
   MAP_X,
   MAP_Y,
   MAP_START,
   MAP_SELECT,
   MAP_L,
   MAP_R,
   MAP_DPAD,
   MAP_ABXY,
   MAP_ANALOG,
   MAP_DPAD_OR_ANALOG,
   NUM_MAPS
};

#define ANALOG_DEADZONE_MAX   30000
#define ANALOG_DEADZONE_MIN   1000
#define ANALOG_DEADZONE_DEFAULT  8000
#define MAX_EXTRA_LIVES 6
#define MAX_EXTRA_BOMBS 6

#endif //GCW

#endif //PORTCFG_H
