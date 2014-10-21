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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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

//senquack - TODO: fix this for GCW/dingux
static const char *config_filename = "rr.conf";
static const char *config_dir = ".rrootage";

//senquack - MANDATORY TODO - make GCW version of these settings:
#if defined(GP2X) || defined(WIZ)
const rrsettings default_settings = {
   1, 1, 700, 1, 1,
   // non-rotated button defs:
   {GP2X_BUTTON_B, GP2X_BUTTON_Y, GP2X_BUTTON_X, GP2X_BUTTON_A,
    GP2X_BUTTON_START, GP2X_BUTTON_SELECT, GP2X_BUTTON_VOLDOWN,
    GP2X_BUTTON_VOLUP},
   // rotated button defs: //19 means disabled
   {GP2X_BUTTON_START, GP2X_BUTTON_X, GP2X_BUTTON_R, GP2X_BUTTON_A,
    GP2X_BUTTON_Y, GP2X_BUTTON_SELECT, GP2X_BUTTON_VOLDOWN, GP2X_BUTTON_VOLUP}
};
#endif // GP2X/Wiz

rrsettings settings;

//senquack
int
clamp (int x, int min, int max)
{
   if (x < min) {
      return min;
   } else if (x > max) {
      return max;
   } else
      return x;

}

//senquack
char *
trim_string (char *buf)
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

//senquack - added a settings file for my Wiz port
//  Return 1 on success, 0 on error.  Read settings into settings structure
int
read_saved_settings (const char *filename)
{
   FILE *f;
   char buf[8192];
   char *str, *param;

   f = fopen (filename, "r");
   if (f == NULL) {
      printf ("Error opening file: %s\n", filename);
      return 0;
   } else {
      printf ("Loading settings from file: %s\n", filename);
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

      if (strcasecmp (str, "laser_on_by_default") == 0)
         settings.laser_on_by_default = clamp (atoi (param), 0, 1);
      else if (strcasecmp (str, "rotated") == 0)
         settings.rotated = clamp (atoi (param), 0, 1);
#if defined(WIZ)
      else if (strcasecmp (str, "fast_ram") == 0)
         settings.fast_ram = clamp (atoi (param), 0, 1);
      else if (strcasecmp (str, "cpu_freq") == 0) {
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
#endif // WIZ
      } else if (strcasecmp (str, "music") == 0)
         settings.music = clamp (atoi (param), 0, 1);
      else if (strcasecmp (str, "buttons_fire") == 0)
         settings.buttons[FIRE_IDX] = clamp (atoi (param), 0, NUM_BUTTONS);
      else if (strcasecmp (str, "buttons_fire2") == 0)
         settings.buttons[FIRE2_IDX] = clamp (atoi (param), 0, NUM_BUTTONS);
      else if (strcasecmp (str, "buttons_special") == 0)
         settings.buttons[SPECIAL_IDX] = clamp (atoi (param), 0, NUM_BUTTONS);
      else if (strcasecmp (str, "buttons_special2") == 0)
         settings.buttons[SPECIAL2_IDX] =
            clamp (atoi (param), 0, NUM_BUTTONS);
      else if (strcasecmp (str, "buttons_pause") == 0)
         settings.buttons[PAUSE_IDX] = clamp (atoi (param), 0, NUM_BUTTONS);
      else if (strcasecmp (str, "buttons_exit") == 0)
         settings.buttons[EXIT_IDX] = clamp (atoi (param), 0, NUM_BUTTONS);
      else if (strcasecmp (str, "buttons_voldown") == 0)
         settings.buttons[VOLDOWN_IDX] = clamp (atoi (param), 0, NUM_BUTTONS);
      else if (strcasecmp (str, "buttons_volup") == 0)
         settings.buttons[VOLUP_IDX] = clamp (atoi (param), 0, NUM_BUTTONS);
      else if (strcasecmp (str, "rbuttons_fire") == 0)
         settings.rbuttons[FIRE_IDX] = clamp (atoi (param), 0, NUM_BUTTONS);
      else if (strcasecmp (str, "rbuttons_fire2") == 0)
         settings.rbuttons[FIRE2_IDX] = clamp (atoi (param), 0, NUM_BUTTONS);
      else if (strcasecmp (str, "rbuttons_special") == 0)
         settings.rbuttons[SPECIAL_IDX] =
            clamp (atoi (param), 0, NUM_BUTTONS);
      else if (strcasecmp (str, "rbuttons_special2") == 0)
         settings.rbuttons[SPECIAL2_IDX] =
            clamp (atoi (param), 0, NUM_BUTTONS);
      else if (strcasecmp (str, "rbuttons_pause") == 0)
         settings.rbuttons[PAUSE_IDX] = clamp (atoi (param), 0, NUM_BUTTONS);
      else if (strcasecmp (str, "rbuttons_exit") == 0)
         settings.rbuttons[EXIT_IDX] = clamp (atoi (param), 0, NUM_BUTTONS);
      else if (strcasecmp (str, "rbuttons_voldown") == 0)
         settings.rbuttons[VOLDOWN_IDX] =
            clamp (atoi (param), 0, NUM_BUTTONS);
      else if (strcasecmp (str, "rbuttons_volup") == 0)
         settings.rbuttons[VOLUP_IDX] = clamp (atoi (param), 0, NUM_BUTTONS);
      else {
         printf ("Ignoring unknown setting: %s\n", str);
      }
   }

   fclose (f);
   return 1;
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
static void
draw ()
{
   switch (status) {
   case TITLE:
      //printf("draw(): Drawing TITLE\n");
      //printf("draw(): Drawing TITLE - drawBackground\n");
      drawBackground ();
      //printf("draw(): Drawing TITLE - drawBoss\n");
      drawBoss ();
      //printf("draw(): Drawing TITLE - drawBulletsWake\n");
      drawBulletsWake ();
      //printf("draw(): Drawing TITLE - drawBullets\n");
      drawBullets ();
      //printf("draw(): Drawing TITLE - startDrawBoards\n");
      startDrawBoards ();
      //printf("draw(): Drawing TITLE - drawSideBoards\n");
      drawSideBoards ();
      //printf("draw(): Drawing TITLE - drawTitle\n");
// if (screenRotated) 
      if (settings.rotated)
         drawTitle_rotated ();
      else
         drawTitle ();

      //printf("draw(): Drawing TITLE - endDrawBoards\n");
      endDrawBoards ();
      //printf("draw(): Drawing TITLE - drawTestPoly\n");
//    drawTestPoly();
      break;
   case IN_GAME:
   case STAGE_CLEAR:
      //senquack
//    printf("draw(): Drawing STAGE_CLEAR\n");
      drawBackground ();
      drawBoss ();
      drawLasers ();
      drawShots ();
      drawBulletsWake ();
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
      endDrawBoards ();
      break;
   case GAMEOVER:
      //senquack
//    printf("draw(): Drawing GAMEOVER\n");
      drawBackground ();
      drawBoss ();
      drawBulletsWake ();
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
      drawBoss ();
      drawLasers ();
      drawShots ();
      drawBulletsWake ();
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
   fprintf (stderr, "Usage: %s [-nosound] [-nowait] [-accframe]\n", argv0);
}

static void
parseArgs (int argc, char *argv[])
{
   int i;
   for (i = 1; i < argc; i++) {
//    if ( strcmp(argv[i], "-lowres") == 0 ) {
//      lowres = 1;
//    } else if ( strcmp(argv[i], "-nosound") == 0 ) {
      if (strcmp (argv[i], "-nosound") == 0) {
         noSound = 1;
      }
//    } else if ( strcmp(argv[i], "-window") == 0 ) {
//      windowMode = 1;
//    } else if ( strcmp(argv[i], "-reverse") == 0 ) {
//      buttonReversed = 1;
//    }
      /* else if ( (strcmp(argv[i], "-brightness") == 0) && argv[i+1] ) {
         i++;
         brightness = (int)atoi(argv[i]);
         if ( brightness < 0 || brightness > 256 ) {
         brightness = DEFAULT_BRIGHTNESS;
         }
         } */
      else if (strcmp (argv[i], "-nowait") == 0) {
         nowait = 1;
      } else if (strcmp (argv[i], "-accframe") == 0) {
         accframe = 1;
      } else {
         usage (argv[0]);
         exit (1);
      }
   }
}

int interval = INTERVAL_BASE;
int tick = 0;
static int pPrsd = 1;

int
main (int argc, char *argv[])
{

   int done = 0;

   long prvTickCount = 0;
   int i;
   SDL_Event event;
   long nowTick;
   int frame;

   parseArgs (argc, argv);


/*here whatever you do with configdir*/

   //senquack - new settings configurable from external utility:

   char *tmp_filename = malloc(strlen(getenv("HOME")) + 1 + strlen(settings_dir) + 1 + strlen(settings_filename) + 1);
   sprintf(tmp_filename, "%s/%s/%s", getenv("HOME"), settings_dir, settings_filename);

   if (getenv("HOME") && settings_filename && settings_dir &&  read_saved_settings (tmp_filename)) {
      printf ("Successfully loaded port settings from %s.\n", tmp_filename);
   } else {
      printf ("Error reading saved port settings from %s.\n  Loading default settings instead.\n", tmp_filename);
      settings = default_settings;
   }

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
      SDL_PollEvent (&event);
//    keys = SDL_GetKeyState(NULL);


      //senquack - all this button handling sure is a ugly mess, let's just hack it and try to forget
//    if ( keys[SDLK_ESCAPE] == SDL_PRESSED || event.type == SDL_QUIT ) done = 1;
//    if ( keys[SDLK_p] == SDL_PRESSED ) {
//      if ( !pPrsd ) {
//       if ( status == IN_GAME ) {
//         status = PAUSE;
//       } else if ( status == PAUSE ) {
//         status = IN_GAME;
//       }
//      }
//      pPrsd = 1;
//    } else {
//      pPrsd = 0;
//    }

      //senquack - Quitting is handled from the main menu.  The quit button merely exits to the
      //            main menu and that logic is handled elsewhere now.
      //senquack - adding support for rotated screen for Wiz:
//    if ( SDL_JoystickGetButton(stick, GP2X_BUTTON_START)) {
//      if ( !pPrsd ) {
//       if ( status == IN_GAME ) {
//         status = PAUSE;
//       } else if ( status == PAUSE ) {
//         status = IN_GAME;
//       }
//      }
//      pPrsd = 1;
//    } else {
//      pPrsd = 0;
//    }
//
//  //senquack
//  if ( SDL_JoystickGetButton(stick, GP2X_BUTTON_SELECT) && status == IN_GAME) {
//     initGameover();
//  }

//  if (screenRotated) {
//     if ( SDL_JoystickGetButton(stick, GP2X_BUTTON_A) ||
//          SDL_JoystickGetButton(stick, GP2X_BUTTON_B) ||  
//          SDL_JoystickGetButton(stick, GP2X_BUTTON_X) ||
//          SDL_JoystickGetButton(stick, GP2X_BUTTON_Y) )
//     {
//          if ( !pPrsd ) {
//          if ( status == IN_GAME ) {
//            status = PAUSE;
//          } else if ( status == PAUSE ) {
//            status = IN_GAME;
//          }
//          }
//          pPrsd = 1;
//     } else {
//          pPrsd = 0;
//     }
//
//     //senquack
//     if ( SDL_JoystickGetButton(stick, GP2X_BUTTON_SELECT) && status == IN_GAME) {
//        initGameover();
//     }
//  } else {
//     if ( SDL_JoystickGetButton(stick, GP2X_BUTTON_START)) {
//       if ( !pPrsd ) {
//          if ( status == IN_GAME ) {
//            status = PAUSE;
//          } else if ( status == PAUSE ) {
//            status = IN_GAME;
//          }
//       }
//       pPrsd = 1;
//     } else {
//       pPrsd = 0;
//     }
//
//     //senquack
//     if ( SDL_JoystickGetButton(stick, GP2X_BUTTON_SELECT) && status == IN_GAME) {
//        initGameover();
//     }
//  }

//  if (screenRotated) {
      if (settings.rotated) {
         if (SDL_JoystickGetButton (stick, settings.rbuttons[PAUSE_IDX])) {
            if (!pPrsd) {
               if (status == IN_GAME) {
                  status = PAUSE;
               } else if (status == PAUSE) {
                  status = IN_GAME;
               }
            }
            pPrsd = 1;
         } else {
            pPrsd = 0;
         }

         //senquack
         if (SDL_JoystickGetButton (stick, settings.rbuttons[EXIT_IDX])
             && status == IN_GAME) {
            initGameover ();
         }
      } else {
         if (SDL_JoystickGetButton (stick, settings.buttons[PAUSE_IDX])) {
            if (!pPrsd) {
               if (status == IN_GAME) {
                  status = PAUSE;
               } else if (status == PAUSE) {
                  status = IN_GAME;
               }
            }
            pPrsd = 1;
         } else {
            pPrsd = 0;
         }

         //senquack
         if (SDL_JoystickGetButton (stick, settings.buttons[EXIT_IDX])
             && status == IN_GAME) {
            initGameover ();
         }
      }

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

//  //senquack - display FPS so we can optimize
//  fpsctr_frames++;
//  fpsctr_newticks = SDL_GetTicks();
//  if ((fpsctr_newticks - fpsctr_oldticks) >= 1000)
//  {
//     printf("\nFPS: %d\n", fpsctr_frames); fflush(stdout);
//     fpsctr_frames = 0;
//     fpsctr_oldticks = fpsctr_newticks;
//  }

      drawGLSceneStart ();
      draw ();
      drawGLSceneEnd ();
      swapGLScene ();
   }
   quitLast ();
   return 0;
}
