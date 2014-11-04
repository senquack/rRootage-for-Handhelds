/*
 * $Id: rr.c,v 1.4 2003/04/26 03:24:16 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * rRootage main routine.
 *
 * @version $Revision: 1.4 $
 */
#include "SDL.h"

#include "GLES/gl.h"
#include "GLES/egl.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

//senquack
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "rr.h"
#include "screen.h"
#include "vector.h"
#include "foe_mtd.h"
#include "brgmng_mtd.h"
#include "degutil.h"
#include "boss_mtd.h"
#include "ship.h"
#include "laser.h"
#include "frag.h"
#include "shot.h"
#include "background.h"
#include "soundmanager.h"
#include "attractmanager.h"

static int noSound = 0;

//senquack - modified code to store files in subdir, modified filenames:
#if defined(GP2X) || defined(WIZ)
static const char *base_settings_path = "./";
#else
static const char *base_settings_path = ".rrootage/";
#endif


static const char *base_portcfg_filename = "rr.conf";    // This is where we store settings for the custom configurator
                                                   //    not included with the original rrootage. -senquack
                                                   //  (Things like custom controls, other new features I added)
                                                   //    It is a text-mode file handhled here in this source file.

static const char *base_prefs_filename = "rr.bin";    // This is where we store the standard settings like Hi-score,
                                                //    game mode, etc, from the original rRootage.
                                                //    It is a binary-mode file handled in attractmanager.c

char *full_prefs_filename = NULL;               // Fully-qualified prefs filename (used in attractmanager.c)

// portcfg holds our default settings until the config file is read:
//DEBUG
//portcfg_settings settings = {    
//   .laser_on_by_default    = 1,                           // Is laser on by default? (more comfortable on handhelds) 
//   .rotated                = SCREEN_HORIZ,                // Is screen rotated? Assigned to one of: 
//                                                          //    SCREEN_HORIZ, SCREEN_ROTATED_LEFT, SCREEN_ROTATED_RIGHT
//   .music                  = 1,                           // Is music enabled?
//   .analog_deadzone        = 8000,                        // Analog joystick deadzone
//   .map                    = {
//      .move     = MAP_DPAD,   //Movement mapping
//      .btn1     = MAP_X,      //Laser mapping
//      .btn2     = MAP_B,      //Bomb mapping
//      .btn1_alt = MAP_A,      //Laser alternate mapping
//      .btn2_alt = MAP_R,      //Bomb alternate mapping
//      .pause    = MAP_START,  //Pause mapping
//      .exit     = MAP_SELECT  //Exit to menu mapping 
//   }
//};     
portcfg_settings settings = {    
   .laser_on_by_default    = 1,                           // Is laser on by default? (more comfortable on handhelds) 
   .rotated                = SCREEN_ROTATED_RIGHT,        // Is screen rotated? Assigned to one of: 
                                                          //    SCREEN_HORIZ, SCREEN_ROTATED_LEFT, SCREEN_ROTATED_RIGHT
   .music                  = 1,                           // Is music enabled?
   .analog_deadzone        = 8000,                        // Analog joystick deadzone
   .draw_outlines          = DRAW_OUTLINES_IKA,           // Which mode of bullet-outline drawing to use
   .map                    = {
      .move     = MAP_DPAD,
      .btn1     = MAP_R,      //Laser mapping
      .btn2     = MAP_ANALOG, //Bomb mapping
      .btn1_alt = MAP_Y,      //Laser alternate mapping
      .btn2_alt = MAP_A,      //Bomb alternate mapping
      .pause    = MAP_START,  //Pause mapping
      .exit     = MAP_SELECT  //Exit to menu mapping 
   }
};     

//senquack - TODO: clean up crufty old Wiz port settings code, adapt it to new portcfg code:
//#if defined(GP2X) || defined(WIZ)
//const portcfg_settings default_settings = {
//   1, 1, 700, 1, 1,
//   // non-rotated button defs:
//   {GP2X_BUTTON_B, GP2X_BUTTON_Y, GP2X_BUTTON_X, GP2X_BUTTON_A,
//    GP2X_BUTTON_START, GP2X_BUTTON_SELECT, GP2X_BUTTON_VOLDOWN,
//    GP2X_BUTTON_VOLUP},
//   // rotated button defs: //19 means disabled
//   {GP2X_BUTTON_START, GP2X_BUTTON_X, GP2X_BUTTON_R, GP2X_BUTTON_A,
//    GP2X_BUTTON_Y, GP2X_BUTTON_SELECT, GP2X_BUTTON_VOLDOWN, GP2X_BUTTON_VOLUP}
//};
//#endif // GP2X/Wiz

//senquack
// Create the specified directory if it doesn't yet exist. Returns 1 on success, 0 on error.
int create_dir(const char *dir)
{
	struct stat	st;
   if ( stat(dir, &st) != 0 ) {
      printf("Dir not found, creating: %s\n", dir);
      if ( mkdir(dir, 0755) == 0 ) {
         printf("Successfully created dir.\n");
         return 1;
      } else {
         printf("Failed to create dir.\n");
         return 0;
      }
   } else {
      if( S_ISDIR(st.st_mode) ) {
         printf("Successfully found existing dir: %s\n", dir);
         return 1;
      } else {
         printf("Found, but unable to access existing dir: %s\n", dir);
         printf("(Is it a file or invalid symlink instead of a dir?)\n");
         return 0;
      }
   }
   return 0;   //shouldn't get here
}

//senquack
int clamp (int x, int min, int max)
{
   if (x < min) {
      return min;
   } else if (x > max) {
      return max;
   } else
      return x;

}

//senquack
char* trim_string (char *buf)
{
   int len;

   while (*buf == ' ')
      buf++;

   len = strlen (buf);

   while (len != 0 && buf[len - 1] == ' ') {
      len--;
      buf[len] = 0;
   }

   return buf;
}

//senquack - added a settings file for my handheld ports
//  Return 1 on success, 0 on error.  Read settings into settings structure
int read_portcfg_settings (const char *filename)
{
   FILE *f;
   char buf[8192];
   char *str, *param;

   f = fopen (filename, "r");
   if (f == NULL) {
      printf ("Error opening file: %s\n", filename);
      return 0;
   } 

   while (!feof (f)) {
      // skip empty lines
      fscanf (f, "%8192[\n\r]", buf);

      // read line
      buf[0] = 0;
      fscanf (f, "%8192[^\n^\r]", buf);

      // trim line
      str = trim_string (buf);

      if (str[0] == 0)
         continue;
      if (str[0] == '#')
         continue;

      // find parameter (after '=')
      param = strchr (str, '=');

      if (param == NULL)
         continue;

      // split string into two strings
      *param = 0;
      param++;

      // trim them
      str = trim_string (str);
      param = trim_string (param);

      if (strcasecmp (str, "laser_on_by_default") == 0) {
         settings.laser_on_by_default = clamp (atoi (param), 0, 1);
      } else if (strcasecmp (str, "rotated") == 0) {
         settings.rotated = clamp (atoi (param), 0, NUM_SCREEN-1);
      } else if (strcasecmp (str, "music") == 0) {
         settings.music = clamp (atoi (param), 0, 1);
      } else if (strcasecmp (str, "analog_deadzone") == 0) {
         settings.analog_deadzone = clamp (atoi (param), 1000, 30000);
      } else if (strcasecmp (str, "draw_outlines") == 0) {
         settings.draw_outlines = clamp (atoi (param), 0, NUM_DRAW_OUTLINES-1);
      } else if (strcasecmp (str, "map_move") == 0) {
         settings.map.move = clamp(atoi (param), 0, NUM_MAPS-1);
      } else if (strcasecmp (str, "map_btn1") == 0) {
         settings.map.btn1 = clamp(atoi (param), 0, NUM_MAPS-1);
      } else if (strcasecmp (str, "map_btn2") == 0) {
         settings.map.btn2 = clamp(atoi (param), 0, NUM_MAPS-1);
      } else if (strcasecmp (str, "map_btn1_alt") == 0) {
         settings.map.btn1_alt = clamp(atoi (param), 0, NUM_MAPS-1);
      } else if (strcasecmp (str, "map_btn2_alt") == 0) {
         settings.map.btn2_alt = clamp(atoi (param), 0, NUM_MAPS-1);
      } else if (strcasecmp (str, "map_pause") == 0) {
         settings.map.pause = clamp(atoi (param), 0, NUM_MAPS-1);
      } else if (strcasecmp (str, "map_exit") == 0) {
         settings.map.exit = clamp(atoi (param), 0, NUM_MAPS-1);
#if defined(WIZ)
         // These will need updating since a lot of code has changed since the Wiz port:
      } else if (strcasecmp (str, "fast_ram") == 0) {
         settings.fast_ram = clamp (atoi (param), 0, 1);
      } else if (strcasecmp (str, "cpu_freq") == 0) {
         if ((strlen (param) > 0) && (strlen (param) < 4)) {
            settings.cpu_freq = atoi (param);
            if (settings.cpu_freq) {
               if ((settings.cpu_freq < MIN_CPU_FREQ)
                   || (settings.cpu_freq > MAX_CPU_FREQ)) {
                  printf
                     ("Error: \"cpu_freq\" parameter out of valid range in %s\n",
                      filename);
                  printf
                     ("Valid range is %d-%d or 0 if overclocking is disabled.\n",
                      MIN_CPU_FREQ, MAX_CPU_FREQ);
                  printf ("Setting parameter to default value %d.\n",
                          default_settings.cpu_freq);
                  settings.cpu_freq = default_settings.cpu_freq;
               }
            }
         } else {
            printf ("Error parsing \"cpu_freq\" parameter in %s\n", filename);
            printf ("Setting parameter to default value of %d.\n",
                    default_settings.cpu_freq);
            settings.cpu_freq = default_settings.cpu_freq;
         }
      } else if (strcasecmp (str, "buttons_fire") == 0) {
         settings.buttons[FIRE_IDX] = clamp (atoi (param), 0, NUM_BUTTONS-1);
      } else if (strcasecmp (str, "buttons_fire2") == 0) {
         settings.buttons[FIRE2_IDX] = clamp (atoi (param), 0, NUM_BUTTONS-1);
      } else if (strcasecmp (str, "buttons_special") == 0) {
         settings.buttons[SPECIAL_IDX] = clamp (atoi (param), 0, NUM_BUTTONS-1);
      } else if (strcasecmp (str, "buttons_special2") == 0) {
         settings.buttons[SPECIAL2_IDX] = clamp (atoi (param), 0, NUM_BUTTONS-1);
      } else if (strcasecmp (str, "buttons_pause") == 0) {
         settings.buttons[PAUSE_IDX] = clamp (atoi (param), 0, NUM_BUTTONS-1);
      } else if (strcasecmp (str, "buttons_exit") == 0) {
         settings.buttons[EXIT_IDX] = clamp (atoi (param), 0, NUM_BUTTONS-1);
      } else if (strcasecmp (str, "buttons_voldown") == 0) {
         settings.buttons[VOLDOWN_IDX] = clamp (atoi (param), 0, NUM_BUTTONS-1);
      } else if (strcasecmp (str, "buttons_volup") == 0) {
         settings.buttons[VOLUP_IDX] = clamp (atoi (param), 0, NUM_BUTTONS-1);
      } else if (strcasecmp (str, "rbuttons_fire") == 0) {
         settings.rbuttons[FIRE_IDX] = clamp (atoi (param), 0, NUM_BUTTONS-1);
      } else if (strcasecmp (str, "rbuttons_fire2") == 0) {
         settings.rbuttons[FIRE2_IDX] = clamp (atoi (param), 0, NUM_BUTTONS-1);
      } else if (strcasecmp (str, "rbuttons_special") == 0) {
         settings.rbuttons[SPECIAL_IDX] = clamp (atoi (param), 0, NUM_BUTTONS-1);
      } else if (strcasecmp (str, "rbuttons_special2") == 0) {
         settings.rbuttons[SPECIAL2_IDX] = clamp (atoi (param), 0, NUM_BUTTONS-1);
      } else if (strcasecmp (str, "rbuttons_pause") == 0) {
         settings.rbuttons[PAUSE_IDX] = clamp (atoi (param), 0, NUM_BUTTONS-1);
      } else if (strcasecmp (str, "rbuttons_exit") == 0) {
         settings.rbuttons[EXIT_IDX] = clamp (atoi (param), 0, NUM_BUTTONS-1);
      } else if (strcasecmp (str, "rbuttons_voldown") == 0) {
         settings.rbuttons[VOLDOWN_IDX] = clamp (atoi (param), 0, NUM_BUTTONS-1);
      } else if (strcasecmp (str, "rbuttons_volup") == 0) {
         settings.rbuttons[VOLUP_IDX] = clamp (atoi (param), 0, NUM_BUTTONS-1);
#endif // GP2X/WIZ
      } else {
         printf ("Ignoring unknown setting: %s\n", str);
      }
   }
   fclose (f);
   return 1;
}

//senquack - added control state abstractions
int control_state[CNUMCONTROLS];    // Tracks state of each individual button/control on physical device

//senquack - added this hack function to fix power slider issues on GCW and when enabling/disabling gsensor/analog stick
// Details: slider issues SDLK_PAUSE keypress before slider daemon
// handles volume/lcd brightness adjustment, this somehow can interfere
// with game's directional keystates after it is released.
// Intended to be called from control_update() below
static void control_reset()
{
	memset(control_state, 0, sizeof control_state);
}

#if defined(WIZ)
void initOverclocking (void)
{
   char cmd_line[200];
   char tmp_str[20];
   strcpy (cmd_line,
           "./pollux_set 'lcd_timings=397,1,37,277,341,0,17,337;dpc_clkdiv0=9");
   if (settings.cpu_freq) {
      sprintf (tmp_str, ";cpuclk=%d", settings.cpu_freq);
      strcat (cmd_line, tmp_str);
   }
   if (settings.fast_ram) {
      strcat (cmd_line, ";ram_timings=2,9,4,1,1,1,1");
      // what we were calling via launch script before:
//       ./pollux_set 'cpuclk=750;lcd_timings=397,1,37,277,341,0,17,337;dpc_clkdiv0=9;ram_timings=2,9,4,1,1,1,1'
   }
   strcat (cmd_line, "'");
   printf ("Adjusting Wiz clocks with command line:\n%s\n", cmd_line);
   system (cmd_line);
}
#endif //WIZ

// Initialize and load preference.
static void
initFirst ()
{

   time_t timer;
   time (&timer);
   srand (timer);

   loadPreference ();
   initBarragemanager ();
   initAttractManager ();
   if (!noSound)
      initSound ();
   initGameStateFirst ();
}

// Quit and save preference.
void
quitLast ()
{
   if (!noSound)
      closeSound ();
   savePreference ();
   closeFoes ();
   closeBarragemanager ();
   closeSDL ();
   SDL_Quit ();
   exit (1);
}

int status;

void
initTitleStage (int stg)
{
   initFoes ();
   initStageState (stg);
}

void
initTitle ()
{
   int stg;
   status = TITLE;

   stg = initTitleAtr ();
   initBoss ();
   initShip ();
   initLasers ();
   initFrags ();
   initShots ();
   initBackground (0);
   initTitleStage (stg);
   left = -1;
}

void
initGame (int stg)
{
   int sn;
   status = IN_GAME;

   initBoss ();
   initFoes ();
   initShip ();
   initLasers ();
   initFrags ();
   initShots ();

   initGameState (stg);
   sn = stg % SAME_RANK_STAGE_NUM;
   initBackground (sn);
   if (sn == SAME_RANK_STAGE_NUM - 1) {
      playMusic (rand () % (SAME_RANK_STAGE_NUM - 1));
   } else {
      playMusic (sn);
   }
}

void
initGameover ()
{
   status = GAMEOVER;
   initGameoverAtr ();
}

static void
move ()
{
   switch (status) {
   case TITLE:
      moveTitleMenu ();
      moveBoss ();
      moveFoes ();
      moveBackground ();
      break;
   case IN_GAME:
   case STAGE_CLEAR:
      moveShip ();
      moveBoss ();
      moveLasers ();
      moveShots ();
      moveFoes ();
      moveFrags ();
      moveBackground ();
      break;
   case GAMEOVER:
      moveGameover ();
      moveBoss ();
      moveFoes ();
      moveFrags ();
      moveBackground ();
      break;
   case PAUSE:
      movePause ();
      break;
   }
   moveScreenShake ();
}


//NOTE - senquack - when I disable everything but drawBackground here it will not freeze:
// ANOTHER NOTE: simply disabling the calls to drawBoss here allow it to run much longer:
//senquack - tried tweaking this to fix hang:
//static void draw() {
//  switch ( status ) {
//  case TITLE:
//    //printf("draw(): Drawing TITLE\n");
//    //printf("draw(): Drawing TITLE - drawBackground\n");
//    drawBackground();
//    //printf("draw(): Drawing TITLE - drawBoss\n");
//    drawBoss();
//    //printf("draw(): Drawing TITLE - drawBulletsWake\n");
//    drawBulletsWake();
//    
//    //TODO: FIX CRASH IN drawBulletsWake() !!!!!!!!!!!!!!!!! **********
//    // When drawBulletsWake used (or buttons pushed), it crashes?
//    
//    
//    //printf("draw(): Drawing TITLE - drawBullets\n");
//    drawBullets();
//    //printf("draw(): Drawing TITLE - startDrawBoards\n");
//    startDrawBoards();
//    //printf("draw(): Drawing TITLE - drawSideBoards\n");
//    drawSideBoards();
//    //printf("draw(): Drawing TITLE - drawTitle\n");
//    drawTitle();
//    //printf("draw(): Drawing TITLE - endDrawBoards\n");
//    endDrawBoards();
//    //printf("draw(): Drawing TITLE - drawTestPoly\n");
//    //drawTestPoly();
//    break;
//  case IN_GAME:
//  case STAGE_CLEAR:
//    printf("draw(): Drawing STAGE_CLEAR\n");
//    drawBackground();
//    drawBoss();
//    drawLasers();
//    drawShots();
//    drawBulletsWake();
//    drawFrags();
//    drawShip();
//    drawBullets();
//    startDrawBoards();
//    drawSideBoards();
//    drawBossState();
//    endDrawBoards();
//    break;
//  case GAMEOVER:
//    printf("draw(): Drawing GAMEOVER\n");
//    drawBackground();
//    drawBoss();
//    drawBulletsWake();
//    drawFrags();
//    drawBullets();
//    startDrawBoards();
//    drawSideBoards();
//    drawGameover();
//    endDrawBoards();
//    break;
//  case PAUSE:
//    printf("draw(): Drawing PAUSE\n");
//    drawBackground();
//    drawBoss();
//    drawLasers();
//    drawShots();
//    drawBulletsWake();
//    drawFrags();
//    drawShip();
//    drawBullets();
//    startDrawBoards();
//    drawSideBoards();
//    drawBossState();
//    drawPause();
//    endDrawBoards();
//    break;
//  }
//}
//senquack - DEBUGGING GCW:
//static void draw ()
//{
//   switch (status) {
//   case TITLE:
//      //printf("draw(): Drawing TITLE\n");
//      //printf("draw(): Drawing TITLE - drawBackground\n");
//      drawBackground ();
//      //printf("draw(): Drawing TITLE - drawBoss\n");
//      drawBoss ();
//      //printf("draw(): Drawing TITLE - drawBulletsWake\n");
//      drawBulletsWake ();
//      //printf("draw(): Drawing TITLE - drawBullets\n");
//      drawBullets ();
//      //printf("draw(): Drawing TITLE - startDrawBoards\n");
//      startDrawBoards ();
//      //printf("draw(): Drawing TITLE - drawSideBoards\n");
//      drawSideBoards ();
//      //printf("draw(): Drawing TITLE - drawTitle\n");
//// if (screenRotated) 
//      if (settings.rotated)
//         drawTitle_rotated ();
//      else
//         drawTitle ();
//
//      //printf("draw(): Drawing TITLE - endDrawBoards\n");
//      endDrawBoards ();
//      //printf("draw(): Drawing TITLE - drawTestPoly\n");
////    drawTestPoly();
//      break;
//   case IN_GAME:
//   case STAGE_CLEAR:
//      //senquack
////    printf("draw(): Drawing STAGE_CLEAR\n");
//      drawBackground ();
//      drawBoss ();
//      drawLasers ();
//      drawShots ();
//      drawBulletsWake ();
//      drawFrags ();
//      drawShip ();
//      drawBullets ();
//      startDrawBoards ();
//      drawSideBoards ();
////senquack - support rotated screen:
////  drawBossState();
////    if (screenRotated) 
//      if (settings.rotated)
//         drawBossState_rotated ();
//      else
//         drawBossState ();
//      endDrawBoards ();
//      break;
//   case GAMEOVER:
//      //senquack
////    printf("draw(): Drawing GAMEOVER\n");
//      drawBackground ();
//      drawBoss ();
//      drawBulletsWake ();
//      drawFrags ();
//      drawBullets ();
//      startDrawBoards ();
//      drawSideBoards ();
//      drawGameover ();
//      endDrawBoards ();
//      break;
//   case PAUSE:
//      //senquack
////    printf("draw(): Drawing PAUSE\n");
//      drawBackground ();
//      drawBoss ();
//      drawLasers ();
//      drawShots ();
//      drawBulletsWake ();
//      drawFrags ();
//      drawShip ();
//      drawBullets ();
//      startDrawBoards ();
//      drawSideBoards ();
////senquack - support rotated screen:
////  drawBossState();
////    if (screenRotated) 
//      if (settings.rotated)
//         drawBossState_rotated ();
//      else
//         drawBossState ();
//      drawPause ();
//      endDrawBoards ();
//      break;
//   }
//}
static void draw ()
{
   switch (status) {
   case TITLE:
      //printf("draw(): Drawing TITLE\n");
      //printf("draw(): Drawing TITLE - drawBackground\n");
      drawBackground ();
//      //printf("draw(): Drawing TITLE - drawBoss\n");
      prepareDrawLines();
      drawBoss ();
////      //printf("draw(): Drawing TITLE - drawBulletsWake\n");
      drawBulletsWake ();
////      //printf("draw(): Drawing TITLE - drawBullets\n");
      finishDrawLines();
      drawBullets ();
////      //printf("draw(): Drawing TITLE - startDrawBoards\n");
      startDrawBoards ();
//      //printf("draw(): Drawing TITLE - drawSideBoards\n");
      drawSideBoards ();
//      //printf("draw(): Drawing TITLE - drawTitle\n");
      if (settings.rotated)
         drawTitle_rotated ();
      else
         drawTitle ();
//
//      //printf("draw(): Drawing TITLE - endDrawBoards\n");
      endDrawBoards ();
//      //printf("draw(): Drawing TITLE - drawTestPoly\n");
////    drawTestPoly();
      break;
   case IN_GAME:
   case STAGE_CLEAR:
      //senquack
//    printf("draw(): Drawing STAGE_CLEAR\n");
      drawBackground ();
      prepareDrawLines();
      drawBoss ();
      drawLasers ();
      drawShots ();
      drawBulletsWake ();
      finishDrawLines();
      drawFrags ();
      drawShip ();
      drawBullets ();
      startDrawBoards ();
      drawSideBoards ();
////senquack - support rotated screen:
//  drawBossState();
//    if (screenRotated) 
      if (settings.rotated)
         drawBossState_rotated ();
      else
         drawBossState ();
      endDrawBoards ();
      break;
   case GAMEOVER:
      //senquack
//    printf("draw(): Drawing GAMEOVER\n");
      drawBackground ();
      prepareDrawLines();
      drawBoss ();
      drawBulletsWake ();
      finishDrawLines();
      drawFrags ();
      drawBullets ();
      startDrawBoards ();
      drawSideBoards ();
      drawGameover ();
      endDrawBoards ();
      break;
   case PAUSE:
      //senquack
//    printf("draw(): Drawing PAUSE\n");
      drawBackground ();
      prepareDrawLines();
      drawBoss ();
      drawLasers ();
      drawShots ();
      drawBulletsWake ();
      finishDrawLines();
      drawFrags ();
      drawShip ();
      drawBullets ();
      startDrawBoards ();
      drawSideBoards ();
//senquack - support rotated screen:
//  drawBossState();
//    if (screenRotated) 
      if (settings.rotated)
         drawBossState_rotated ();
      else
         drawBossState ();
      drawPause ();
      endDrawBoards ();
      break;
   }
}

static int accframe = 0;

static void
usage (char *argv0)
{
//  fprintf(stderr, "Usage: %s [-rotate] [-laser] [-nosound] [-reverse] [-nowait] [-accframe]\n", argv0);
//   fprintf (stderr, "Usage: %s [-nosound] [-nowait] [-accframe]\n", argv0);
}

//senquack - don't need to parse args
static void
parseArgs (int argc, char *argv[])
{
   //senquack DEBUG
   nowait = 1;

//   int i;
//   for (i = 1; i < argc; i++) {
////    if ( strcmp(argv[i], "-lowres") == 0 ) {
////      lowres = 1;
////    } else if ( strcmp(argv[i], "-nosound") == 0 ) {
////      if (strcmp (argv[i], "-nosound") == 0) {
////         noSound = 1;
////      }
////    } else if ( strcmp(argv[i], "-window") == 0 ) {
////      windowMode = 1;
////    } else if ( strcmp(argv[i], "-reverse") == 0 ) {
////      buttonReversed = 1;
////    }
//      /* else if ( (strcmp(argv[i], "-brightness") == 0) && argv[i+1] ) {
//         i++;
//         brightness = (int)atoi(argv[i]);
//         if ( brightness < 0 || brightness > 256 ) {
//         brightness = DEFAULT_BRIGHTNESS;
//         }
//         } */
//      else if (strcmp (argv[i], "-nowait") == 0) {
//         nowait = 1;
//      } else if (strcmp (argv[i], "-accframe") == 0) {
//         accframe = 1;
//      } else {
//         usage (argv[0]);
//         exit (1);
//      }
//   }
}

int interval = INTERVAL_BASE;
int tick = 0;
static int pPrsd = 1;

int main (int argc, char *argv[])
{

   int done = 0;

   long prvTickCount = 0;
   int i;
   SDL_Event event;
   long nowTick;
   int frame;

   parseArgs (argc, argv);

   //senquack - added support for custom configuration, as well as storing both it and the main "preferences"
   //    (hi-score, mode) file to be stored in a subdir $HOME/.rrootage/

   char *full_portcfg_filename = NULL;
#if defined(GP2X) || defined(WIZ)

   // On GCW/Wiz, settings go into the current working dir
   full_prefs_filename = (char *) malloc( sizeof(base_settings_path) + sizeof(base_prefs_filename) + 1);
   strcpy(full_prefs_filename, base_settings_path);
   strcat(full_prefs_filename, base_prefs_filename);

   full_portcfg_filename = (char *) malloc( sizeof(base_settings_path) + sizeof(base_portcfg_filename) + 1);
   strcpy(full_portcfg_filename, base_settings_path);
   strcat(full_portcfg_filename, base_portcfg_filename);

#else

   // On all other platforms, settings dir goes into the $HOME dir
   if (getenv("HOME")) {
      printf("Got $HOME directory environment variable: %s\n", getenv("HOME"));
      full_prefs_filename = (char *) malloc( strlen(getenv("HOME")) + 1 + 
                                             strlen(base_settings_path) + strlen(base_prefs_filename) + 1);
      sprintf(full_prefs_filename, "%s/%s%s", getenv("HOME"), base_settings_path, base_prefs_filename);

      full_portcfg_filename = (char *) malloc( strlen(getenv("HOME")) + 1 + 
                                             strlen(base_settings_path) + strlen(base_portcfg_filename) + 1);
      sprintf(full_portcfg_filename, "%s/%s%s", getenv("HOME"), base_settings_path, base_portcfg_filename);
   } else {
      printf("Failed to get $HOME directory environment variable, aborting program.\n");
      return 1;
   }

   char *tmp_pathname = (char *) malloc( strlen(getenv("HOME")) + 1 + strlen(base_settings_path) + 1);
   sprintf(tmp_pathname, "%s/%s", getenv("HOME"), base_settings_path);
   printf("Ensuring settings directory exists, creating if not: %s\n", tmp_pathname);
   if ( !create_dir(tmp_pathname) ) {
      printf("Unable to create missing settings directory, aborting program.\n");
      return 1;
   }
   free(tmp_pathname);
#endif

   printf("Loading portcfg settings from: %s\n", full_portcfg_filename);
   if ( read_portcfg_settings(full_portcfg_filename) ) {
      printf("Successfully read settings.\n");
   } else {
      printf("Failed to read settings, using defaults.\n");
   }
   free( full_portcfg_filename );   // Don't need anymore 

   //senquack TODO: remember to add WIZ define to Makefile
#ifdef WIZ
   initOverclocking ();         //senquack - perform any overclocking of CPU or RAM timings via pollux_set
#endif

   initDegutil ();
   initSDL ();
   initFirst ();
   initTitle ();

   //senquack
//printf("Init title done\n");
//fflush(stdout);

   while (!done) {
      //TEMP DEBUGGING:
      // Quit if button/key pressed:
//      while(SDL_PollEvent(&event)){
//         switch (event.type) {
//            case SDL_KEYDOWN:
//            case SDL_KEYUP:
//               done = 1;
//               break;
//            default:
//               break;
//
//         }
//      }

//      SDL_PollEvent (&event);
//    keys = SDL_GetKeyState(NULL);

//    Uint8 *keys = SDL_GetKeyState(NULL);
//
//
//      //senquack - all this button handling sure is a ugly mess, let's just hack it and try to forget
//    if ( keys[SDLK_ESCAPE] == SDL_PRESSED || event.type == SDL_QUIT ) done = 1;

////    if ( keys[SDLK_p] == SDL_PRESSED ) {
////      if ( !pPrsd ) {
////       if ( status == IN_GAME ) {
////         status = PAUSE;
////       } else if ( status == PAUSE ) {
////         status = IN_GAME;
////       }
////      }
////      pPrsd = 1;
////    } else {
////      pPrsd = 0;
////    }
//
//      //senquack - Quitting is handled from the main menu.  The quit button merely exits to the
//      //            main menu and that logic is handled elsewhere now.
//      //senquack - adding support for rotated screen for Wiz:
////    if ( SDL_JoystickGetButton(stick, GP2X_BUTTON_START)) {
////      if ( !pPrsd ) {
////       if ( status == IN_GAME ) {
////         status = PAUSE;
////       } else if ( status == PAUSE ) {
////         status = IN_GAME;
////       }
////      }
////      pPrsd = 1;
////    } else {
////      pPrsd = 0;
////    }
////
////  //senquack
////  if ( SDL_JoystickGetButton(stick, GP2X_BUTTON_SELECT) && status == IN_GAME) {
////     initGameover();
////  }
//
////  if (screenRotated) {
////     if ( SDL_JoystickGetButton(stick, GP2X_BUTTON_A) ||
////          SDL_JoystickGetButton(stick, GP2X_BUTTON_B) ||  
////          SDL_JoystickGetButton(stick, GP2X_BUTTON_X) ||
////          SDL_JoystickGetButton(stick, GP2X_BUTTON_Y) )
////     {
////          if ( !pPrsd ) {
////          if ( status == IN_GAME ) {
////            status = PAUSE;
////          } else if ( status == PAUSE ) {
////            status = IN_GAME;
////          }
////          }
////          pPrsd = 1;
////     } else {
////          pPrsd = 0;
////     }
////
////     //senquack
////     if ( SDL_JoystickGetButton(stick, GP2X_BUTTON_SELECT) && status == IN_GAME) {
////        initGameover();
////     }
////  } else {
////     if ( SDL_JoystickGetButton(stick, GP2X_BUTTON_START)) {
////       if ( !pPrsd ) {
////          if ( status == IN_GAME ) {
////            status = PAUSE;
////          } else if ( status == PAUSE ) {
////            status = IN_GAME;
////          }
////       }
////       pPrsd = 1;
////     } else {
////       pPrsd = 0;
////     }
////
////     //senquack
////     if ( SDL_JoystickGetButton(stick, GP2X_BUTTON_SELECT) && status == IN_GAME) {
////        initGameover();
////     }
////  }
//
////  if (screenRotated) {
//      if (settings.rotated) {
//         if (SDL_JoystickGetButton (stick, settings.rbuttons[PAUSE_IDX])) {
//            if (!pPrsd) {
//               if (status == IN_GAME) {
//                  status = PAUSE;
//               } else if (status == PAUSE) {
//                  status = IN_GAME;
//               }
//            }
//            pPrsd = 1;
//         } else {
//            pPrsd = 0;
//         }
//
//         //senquack
//         if (SDL_JoystickGetButton (stick, settings.rbuttons[EXIT_IDX])
//             && status == IN_GAME) {
//            initGameover ();
//         }
//      } else {
//         if (SDL_JoystickGetButton (stick, settings.buttons[PAUSE_IDX])) {
//            if (!pPrsd) {
//               if (status == IN_GAME) {
//                  status = PAUSE;
//               } else if (status == PAUSE) {
//                  status = IN_GAME;
//               }
//            }
//            pPrsd = 1;
//         } else {
//            pPrsd = 0;
//         }
//
//         //senquack
//         if (SDL_JoystickGetButton (stick, settings.buttons[EXIT_IDX])
//             && status == IN_GAME) {
//            initGameover ();
//         }
//      }

//senquack TODO: adapt this newer control handling to the older Wiz port stuff:
#ifdef GCW
      while(SDL_PollEvent(&event)) {
         switch(event.type) {
            case SDL_QUIT:
               done = 1;
               break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
               switch(event.key.keysym.sym) {
                  case SDLK_UP:
                     control_state[CUP] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                     break;
                  case SDLK_DOWN:
                     control_state[CDOWN] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                     break;
                  case SDLK_LEFT:
                     control_state[CLEFT] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                     break;
                  case SDLK_RIGHT:
                     control_state[CRIGHT] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                     break;
                  case SDLK_RETURN:	// GCW Start button
                     control_state[CSTART] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                     break;
                  case SDLK_ESCAPE:	// GCW Select button
                     control_state[CSELECT] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                     break;
                  case SDLK_SPACE:	// GCW Y button
                     control_state[CY] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                     break;
                  case SDLK_LALT:	// GCW B button
                     control_state[CB] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                     break;
                  case SDLK_LSHIFT:	// GCW X button
                     control_state[CX] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                     break;
                  case SDLK_LCTRL:	// GCW A button
                     control_state[CA] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                     break;
                  case SDLK_BACKSPACE:	// GCW R trigger
                     control_state[CR] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                     break;
                  case SDLK_TAB:	      // GCW L trigger
                     control_state[CL] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                     break;
                  default:
                     break;
               }
         }
      }

      // ANALOG JOY:
      if (joy_analog) {
         Sint16 xmove, ymove;
         xmove=SDL_JoystickGetAxis(joy_analog,0);
         ymove=SDL_JoystickGetAxis(joy_analog,1);
         control_state[C_ANY_ANALOG] = 0;
         control_state[C_ANY_ANALOG] |=    control_state[CANALOGLEFT] 	= (xmove < -settings.analog_deadzone);
         control_state[C_ANY_ANALOG] |=    control_state[CANALOGRIGHT] 	= (xmove > settings.analog_deadzone);
         control_state[C_ANY_ANALOG] |=    control_state[CANALOGUP] 		= (ymove < -settings.analog_deadzone);
         control_state[C_ANY_ANALOG] |=    control_state[CANALOGDOWN] 	= (ymove > settings.analog_deadzone);
      }

      /*    Working-around GCW power-slider daemon's bug: if pressed while any other control is pressed,
         it will eat the key up events and the game's control state can become confused. Resetting the control
         state is not enough, so we'll do what little we can here to patch it up. 
            Also, this will help with using A/B/X/Y as a directional control, so game is never confused and gets
         signal to move up and down or left and right at the same time. */
      if (settings.map.move == MAP_DPAD) {
         if (control_state[CUP])       control_state[CDOWN] = 0;
         if (control_state[CDOWN])     control_state[CUP] = 0;
         if (control_state[CLEFT])     control_state[CRIGHT] = 0;
         if (control_state[CRIGHT])    control_state[CLEFT] = 0;
         // same trick won't work for abxy
//      } else if (settings.map.move == MAP_ABXY) {
//         if (control_state[CX])  control_state[CA] = 0;
//         if (control_state[CA])  control_state[CX] = 0;
//         if (control_state[CB])  control_state[CY] = 0;
//         if (control_state[CY])  control_state[CB] = 0;
      }
      /* END POWER-SLIDER-BUG WORKAROUND */

      control_state[C_ANY_DPAD] = control_state[CUP] || control_state[CDOWN] ||
                                    control_state[CLEFT] || control_state[CRIGHT];
      control_state[C_ANY_ABXY] = control_state[CA] || control_state[CB] ||
                                    control_state[CX] || control_state[CY];
      // Handling pausing:
      if (control_state[settings.map.pause]) {
         if ( !pPrsd ) {
            if ( status == IN_GAME ) {
               status = PAUSE;
            } else if ( status == PAUSE ) {
               status = IN_GAME;
            }
         }
         pPrsd = 1;
      } else {
         pPrsd = 0;

         // Handle quitting current game (as long as we're not paused)
         if (control_state[settings.map.exit] && (status == IN_GAME)) {    
            initGameover();
         }
      }

#endif //GCW

      nowTick = SDL_GetTicks ();
      frame = (int) (nowTick - prvTickCount) / interval;
      if (frame <= 0) {
         frame = 1;
         SDL_Delay (prvTickCount + interval - nowTick);
         if (accframe) {
            prvTickCount = SDL_GetTicks ();
         } else {
            prvTickCount += interval;
         }
      } else if (frame > 5) {
         frame = 5;
         prvTickCount = nowTick;
      } else {
         prvTickCount += frame * interval;
      }
      for (i = 0; i < frame; i++) {
         move ();
         tick++;
      }

      drawGLSceneStart ();
      draw ();
      drawGLSceneEnd ();
      swapGLScene ();
      //senquack - at least try to be nice to other processes:
      usleep(0);
   }

   quitLast ();

   if (full_prefs_filename) free(full_prefs_filename);
   return 0;
}
