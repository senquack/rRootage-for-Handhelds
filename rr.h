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

//senquack - holds our port-specific definitions
#include "portcfg.h"

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
extern portcfg_settings settings;    //portcfg is our global "current-settings"
extern char *full_prefs_filename;      // Fully-qualified prefs filename
extern int control_state[CNUMCONTROLS];    // Tracks state of each individual button/control on physical device
extern int ext_to_int_map[NUM_MAPS];  //Logical mapping between portcfg's external mapping to rrootage's internal

int create_dir(const char *dir);
#endif //RR_H
