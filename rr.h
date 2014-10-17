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
#define CAPTION "rRootage"
#define VERSION_NUM 22

#define INTERVAL_BASE 16

extern int status;
extern int interval;
extern int tick;

//senquack  - new settings configurable from an external utility
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
typedef struct
{
   int laser_on_by_default;
   int rotated;
   int cpu_freq;                // if this is 0, overclocking is disabled
   int fast_ram;
   int music;
   int buttons[NUM_DEFS], rbuttons[NUM_DEFS];   //rbuttons is for rotated button definitions
} rrsettings;
extern rrsettings settings;

#define TITLE 0
#define IN_GAME 1
#define GAMEOVER 2
#define STAGE_CLEAR 3
#define PAUSE 4

//senquack
#define NUM_BUTTONS  (19)       // total number of buttons we'll need to keep track of
#define FIRE_IDX				(0) //Indexes into button redefinitions in settings structure
#define FIRE2_IDX				(1)
#define SPECIAL_IDX			(2)
#define SPECIAL2_IDX			(3)
#define PAUSE_IDX				(4)
#define EXIT_IDX				(5)
#define VOLDOWN_IDX			(6)
#define VOLUP_IDX				(7)

void quitLast ();
void initTitleStage (int stg);
void initTitle ();
void initGame ();
void initGameover ();
