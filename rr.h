/*
 * $Id: rr.h,v 1.4 2003/04/26 03:24:16 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * rRootage header file.
 *
 * @version $Revision: 1.4 $
 */
#ifndef RR_H
#define RR_H

#define CAPTION "rRootage"
#define VERSION_NUM 22

#define INTERVAL_BASE 16

extern int status;
extern int interval;
extern int tick;

#define TITLE 0
#define IN_GAME 1
#define GAMEOVER 2
#define STAGE_CLEAR 3
#define PAUSE 4

void quitLast ();
void initTitleStage (int stg);
void initTitle ();
void initGame ();
void initGameover ();

//senquack - added this for custom port configuration:
extern const char *settings_dir;    // Both the two files below will be written into this dir.
                                             //   This dir will normally exist in the $HOME dir,
                                             //    but on GP2X/Wiz it will exist in the current dir. 

extern const char *portcfg_filename;    // This is where we store settings for the custom configurator
                                                   //    not included with the original rrootage. -senquack
                                                   //  (Things like custom controls, other new features I added)
                                                   //    It is a text-mode file handhled here in this source file.

extern const char *prefs_filename;    // This is where we store the standard settings like Hi-score,

enum {   SCREEN_HORIZ,
         SCREEN_ROTATED_LEFT,
         SCREEN_ROTATED_RIGHT
};

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
typedef struct portcfg_settings
{
   int laser_on_by_default;   
   int rotated;               // Is screen rotated? Assigned to one of: 
                              //    SCREEN_HORIZ, SCREEN_ROTATED_LEFT, SCREEN_ROTATED_RIGHT
   int music;                 // Is music enabled?
   int buttons_swapped;       // Are laser / bomb buttons swapped?
   int analog_deadzone;       // Analog joystick deadzone
   int analog_enabled;        // Analog joystick enalbed?
#if defined(WIZ) || defined(GP2X)
   int cpu_freq;                // if this is 0, overclocking is disabled
   int fast_ram;
   int buttons[NUM_DEFS], rbuttons[NUM_DEFS];   //rbuttons is for rotated button definitions
#endif //WIZ/GP2X
} portcfg_settings;

extern portcfg_settings settings;    //portcfg is our global "current-settings"
extern char *full_prefs_filename;      // Fully-qualified prefs filename

//senquack - new controls handling code:
enum {
   CUP,
   CDOWN,
   CLEFT,
   CRIGHT,
   CPAUSE,
   CESCAPE,
   CVOLUP,
   CVOLDOWN,
   CBUTTON1,
   CBUTTON2,
   CANALOGUP,
   CANALOGDOWN,
   CANALOGLEFT,
   CANALOGRIGHT,
   CNUMCONTROLS,
};
extern int control_state[CNUMCONTROLS];      // Control state abstraction

int create_dir(const char *dir);
#endif //RR_H
