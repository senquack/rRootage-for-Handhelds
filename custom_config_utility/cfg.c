/*	rRootage for Handhelds port configurator	
   Copyright (C) 2014 Dan Silsby (Senor Quack)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 */
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <unistd.h>
#include <errno.h>
#include	<SDL.h>
#include	<SDL_image.h>
#include <SDL_gfxPrimitives.h>
#include	"SFont.h"	

#include "../portcfg.h"    // rRootage-specific port configuration header
#include "cfg.h"
#include "menu_game_settings.h"
#include "menu_control_settings.h"

#ifdef GCW
#include "gcw.h"
#elif defined (GP2X) || defined (WIZ)
#include "wiz_gp2x.h"
#endif

//#define NUM_USABLE_BUTTONS	(10)	//number of GP2X buttons that can be assigned 

portcfg_settings settings, settings_backup;     //settings_backup is for cancelling changes in a submenu

//global surfaces
SDL_Surface* screen;				// framebuffer
SDL_Surface* background;		

//global fonts
SFont_Font *font_wh; //white

////global joysticks
//SDL_Joystick* joy;

const int menu_entry_spacing = 2;         // Number of pixels between each line in the menus
const int menu_entry_borders = 4;         // Size of left, top, and bottom inside-borders of menu
const int menu_delay = 165;
const int menu_x_offset = 30;
//const int menu_y_center = 144;            // Y-coordinate to center the menu top-to-bottom on (leaving room for text below)
const int menu_y_center = 130;            // Y-coordinate to center the menu top-to-bottom on (leaving room for text below)
int exit_menu = 0;                        // Set to 1 when a submenu completes, then immediately reset back to 0

int control_state[MAP_UTIL_NUMMAPPINGS];
const char *base_portcfg_filename = "rr.conf";
const char *base_rrbin_filename = "rr.bin";
char *full_portcfg_filename = NULL;
char *full_rrbin_filename = NULL;

int clamp(int x, int min, int max)
{
   if (x < min) { return min; }
   else if (x > max) { return max; }
   else return x;

}

char *trim_string(char *buf)
{
   int len;

   while (*buf == ' ') buf++;

   len = strlen(buf);

   while (len != 0 && buf[len - 1] == ' ')
   {
      len--;
      buf[len] = 0;
   }

   return buf;
}

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

int load_graphics()
{
   SDL_Surface *tmp_sur, *tmp_sur2;

   // Background image with test gradients
   tmp_sur = IMG_Load("cfg/bg.png");
   if (!tmp_sur)
   {
      printf("Fatal error loading: cfg/bg.png\n");
      return 0;
   }
   background = SDL_DisplayFormat(tmp_sur);
   SDL_FreeSurface(tmp_sur);

   //Font
   tmp_sur = IMG_Load("cfg/font_wh.png");
   if (!tmp_sur)
   {
      printf("Fatal error loading: cfg/font_wh.png\n" );
      return 0;
   }
   tmp_sur2 = SDL_DisplayFormat(tmp_sur);
   font_wh = SFont_InitFont(tmp_sur2);
   if (!font_wh)
   {
      printf("Fatal error intializing SFont on font_wh.png\n" );
      return 0;
   }
   SDL_FreeSurface(tmp_sur);
   return 1;
}

void unload_graphics()
{
   if (background)
      SDL_FreeSurface(background);

   if (font_wh) 
      SFont_FreeFont(font_wh);
}

void shutdown()
{
   if (full_portcfg_filename) 
      free( full_portcfg_filename );   
   unload_graphics();
   SDL_Quit();
}

void shutdown_and_exit()
{
   printf("Shutting down and exiting..\n");
   shutdown();
   exit(0);
}

void display_first_run_message()
{
   char *message[] = {
      "-Important note for first-time players-",
      "This is probably the most heavily-optimized port of",
      "rRootage in existence, in addition to being the",
      "most customizable.  It is capable of rendering nearly",
      "every scene at a perfect 60fps on the GCW Zero.",
      "However, the original game is designed to limit the",
      "frame-rate and speed of the game when many bullets",
      "appear at once, to make the game more fair.  If you",
      "wish to disable the bullet slow-down, you may do so",
      "in the \"Configure game settings\" sub-menu."
   };

   int entry_spacing = 0;
   int num_lines = 10;
   int font_h = SFont_TextHeight(font_wh);
   int menu_h = (2 * menu_entry_borders) + (num_lines*font_h) + ((num_lines - 1) * entry_spacing) - 1;
   int menu_w = SCREEN_WIDTH - (menu_x_offset * 2);
   int menu_y = menu_y_center - (menu_h/2);
   int desc_y = SCREEN_HEIGHT - 1 - font_h - entry_spacing;  // Y coord of line of text for proceed message
   SDL_Rect dstrect;

   // Animated expansion of window
   for (int i = 0; i < (menu_w / 2); i+=16) {
      SDL_BlitSurface(background, NULL, screen, NULL);
      rectangleRGBA(screen, ((SCREEN_WIDTH/2) - i), menu_y, ((SCREEN_WIDTH/2) + i), menu_y + menu_h,
            255, 255, 255, 255);		
      dstrect.x = (SCREEN_WIDTH/2) - i + 1;
      dstrect.y = menu_y + 1;
      dstrect.w = ((SCREEN_WIDTH/2) + i) - ((SCREEN_WIDTH/2) - i) - 2;
      dstrect.h = menu_h - 2;
      SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 50, 50, 50));
      SDL_Flip(screen);
   }

   int x,y;
   int proceed = 0;
   int delay_has_passed = 0;
   while (!proceed) {      // menu loop
      SDL_BlitSurface(background, NULL, screen, NULL);
      rectangleRGBA(screen, menu_x_offset, menu_y, menu_x_offset+menu_w, menu_y+menu_h, 255, 255, 255, 255);
      dstrect.x = menu_x_offset + 1;
      dstrect.y = menu_y + 1;
      dstrect.w = menu_w - 1;
      dstrect.h = menu_h - 2;
      SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 50, 50, 50));
      
      //draw text:
      x = menu_x_offset + 1 + menu_entry_borders + 20;
      y = menu_y + menu_entry_borders;
      SFont_WriteCenter(screen, font_wh, y, message[0] );   //Title of text
      y += font_h + entry_spacing;

      for (int i = 1; i < num_lines; i++) {
//         SFont_Write(screen, font_wh, x, y, message[i]);
         SFont_WriteCenter(screen, font_wh, y, message[i]);
         y += font_h + entry_spacing;
      }
      if (delay_has_passed) {
         SFont_WriteCenter(screen, font_wh, desc_y, "Press START button to proceed");
      }
      SDL_Flip(screen);
      if (!delay_has_passed) {
         SDL_Delay(1000);
         delay_has_passed = 1;
      }
      
      if(!update_control_state()) {
         // SDL_QUIT encountered in event queue, abort program:
         printf("SDL_QUIT event received, exiting program..\n");
         shutdown_and_exit();
      }

      if (control_state[MAP_UTIL_STARTBUTTON] && delay_has_passed) {
         proceed = 1;
      }
   }

   SDL_Delay(menu_delay);
}

void run_menu(menu *m)
{
   reset_control_state();     // Helps keep button presses from interfering with new menus

   if ((m->num_entries < 1) || (!m->entries)) {
      printf("Error: empty menu\n");
      return;
   }

   //Find first non-special entry in the menu and select it as the current selection:
   for (int i = m->num_entries-1; i >= 0; i--) {
      if (!m->entries[i].is_special) {
         m->cur_entry = i;
      }
   }

   int font_h = SFont_TextHeight(font_wh);
   int menu_h = (2 * menu_entry_borders) + (m->num_entries * font_h) + ((m->num_entries - 1) * menu_entry_spacing) - 1;
   int menu_w = SCREEN_WIDTH - (menu_x_offset * 2);
   int menu_y = menu_y_center - (menu_h/2);
   int desc_y = SCREEN_HEIGHT - 1 - font_h - menu_entry_spacing;  // Y coord of line of text for entry description
   SDL_Rect dstrect;

   // Animated expansion of menu
   for (int i = 0; i < (menu_w / 2); i+=16)
   {
      SDL_BlitSurface(background, NULL, screen, NULL);
      rectangleRGBA(screen, ((SCREEN_WIDTH/2) - i), menu_y, ((SCREEN_WIDTH/2) + i), menu_y + menu_h,
            255, 255, 255, 255);		
      dstrect.x = (SCREEN_WIDTH/2) - i + 1;
      dstrect.y = menu_y + 1;
      dstrect.w = ((SCREEN_WIDTH/2) + i) - ((SCREEN_WIDTH/2) - i) - 2;
      dstrect.h = menu_h - 2;
      SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 50, 50, 50));
      SDL_Flip(screen);
   }

   int x,y;
   while (!exit_menu) {      // menu loop
      SDL_BlitSurface(background, NULL, screen, NULL);
      rectangleRGBA(screen, menu_x_offset, menu_y, menu_x_offset+menu_w, menu_y+menu_h, 255, 255, 255, 255);
      dstrect.x = menu_x_offset + 1;
      dstrect.y = menu_y + 1;
      dstrect.w = menu_w - 1;
      dstrect.h = menu_h - 2;
      SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 50, 50, 50));
      
      //draw menu entries' text and values:
      x = menu_x_offset + 1 + menu_entry_borders + 1;
      y = menu_y + menu_entry_borders;
      for (int i = 0; i < m->num_entries; i++) {
         //draw the selection bar if this is the current entry selected
         if (m->cur_entry == i) {
            dstrect.x = x - 3;
            dstrect.y = y - 1;
            dstrect.h = font_h + 2;
            dstrect.w = SCREEN_WIDTH - ((x - 1) * 2) + 5;
            SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 100, 100, 100));
         }

         if (m->entries[i].text) {
            if (m->entries[i].is_special) {
               SFont_WriteCenter(screen, font_wh, y, m->entries[i].text);  // This is either a menu title or a blank line
            } else {
               SFont_Write(screen, font_wh, x, y, m->entries[i].text);     // Entry's text
               if (m->entries[i].func_get_val_text != NULL) {              // Entry's value (if applicable)
                  SFont_Write(screen, font_wh,                             
                       ( SCREEN_WIDTH - menu_x_offset - menu_entry_borders - 
                        SFont_TextWidth(font_wh, m->entries[i].func_get_val_text())),
                       y, m->entries[i].func_get_val_text());
               }
            }
         }
         y += font_h + menu_entry_spacing;
      }

      // Draw description text below menu:
      if (!m->entries[m->cur_entry].is_special && m->entries[m->cur_entry].desc_text) {
         SFont_WriteCenter(screen, font_wh, desc_y, m->entries[m->cur_entry].desc_text);
      }

      SDL_Flip(screen);
      
      if(!update_control_state()) {
         // SDL_QUIT encountered in event queue, abort program:
         printf("SDL_QUIT event received, exiting program..\n");
         shutdown_and_exit();
      }

      if (control_state[MAP_UTIL_SELECT]) {
         if (m->entries[m->cur_entry].func_select) {
            m->entries[m->cur_entry].func_select();
         }
         SDL_Delay(menu_delay);
      } else if (control_state[MAP_UTIL_LEFT]) {
         if (m->entries[m->cur_entry].func_left) {
            m->entries[m->cur_entry].func_left();
         }
         SDL_Delay(menu_delay);
      } else if (control_state[MAP_UTIL_RIGHT]) {
         if (m->entries[m->cur_entry].func_right) {
            m->entries[m->cur_entry].func_right();
         }
         SDL_Delay(menu_delay);
      } else if (control_state[MAP_UTIL_UP]) {
         do {
            m->cur_entry--;
            if (m->cur_entry < 0) {
               m->cur_entry = m->num_entries - 1;
            }
         } while (m->entries[m->cur_entry].is_special);
         SDL_Delay(menu_delay);
      } else if (control_state[MAP_UTIL_DOWN]) {
         do {
            m->cur_entry++;
            if (m->cur_entry == m->num_entries) {
               m->cur_entry = 0;
            }
         } while (m->entries[m->cur_entry].is_special);
         SDL_Delay(menu_delay);
      }
      SDL_Delay(10);
   }
   exit_menu = 0;    // Reset global exit trigger since we might be a sub-menu
}

int initialize(void)
{

   if (!(screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SCREEN_FLAGS))) 
   {
      fprintf(stderr, "cannot set video mode with SDL: %s\n", SDL_GetError());
      return 0;
   } 

   SDL_ShowCursor(SDL_DISABLE);
	SDL_EnableKeyRepeat(0, 0); // No key repeat

//   if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
   if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      fprintf(stderr, "Cannot initialize SDL system: %s\n", SDL_GetError());
      return 0;
   }

//   joy = SDL_JoystickOpen(0);
//   if (joy == NULL)
//   {
//      fprintf(stderr, "Cannot initialize GP2X joystick: %s\n", SDL_GetError());
//   }

   if (!load_graphics()) {
      fprintf(stderr, "Error loading sprites/fonts.\n");
      return 0;
   }

   return 1;
}

void run_rrootage() 
{
   shutdown();
   printf("Executing ./rr\n");
   execl("./rr", "rr", (char *)NULL);
   printf("ERROR: execution of ./rr failed!\n");
}

void load_defaults_for_rotated_left()
{
   settings = default_rotated_left_settings;
   settings_save_changes();
}

void load_defaults_for_rotated_right()
{
   settings = default_rotated_right_settings;
   settings_save_changes();
}

void load_defaults_for_horizontal()
{
   settings = default_horizontal_settings;
   settings_save_changes();
}

void run_game_settings_menu()
{
   settings_backup = settings;   // Make backup first in case user wants to cancel changes
   run_menu(&game_settings_menu);
}

void run_control_settings_menu()
{
   settings_backup = settings;   // Make backup first in case user wants to cancel changes
   run_menu(&control_settings_menu);
}

void settings_save_changes()
{
   if (!write_new_portcfg(full_portcfg_filename)) {
      printf("Error writing new configuration to: %s\n", full_portcfg_filename);
   }
   exit_current_menu();
}

void settings_cancel_changes()
{
   settings = settings_backup;
   exit_current_menu();
}

void run_defaults_menu() 
{
   run_menu(&defaults_menu); 
}

void run_reset_menu() 
{
   run_menu(&reset_menu); 
}

void reset_scores()
{
   printf("Deleting file: %s\n", full_rrbin_filename);
   if (unlink(full_rrbin_filename)) {
      printf("ERROR deleting file: %s\n", full_rrbin_filename);
   }
   exit_current_menu();
}

void exit_current_menu()
{
   exit_menu = 1;
}

int main(int argc, char *argv[])
{
   // Initialize graphics, load sprits/fonts etc
   if (!initialize())
   {
      fprintf(stderr, "Aborting because of initialization errors.\n");
      return 1;
   }

   settings = default_rotated_right_settings;
   set_filenames();

   //  Return 0 on success, -1 on error.  Read settings into settings structure.
   if (!read_portcfg_settings(full_portcfg_filename))
   {
      printf("Unable to read settings from %s, using defaults values.\n", full_portcfg_filename);
      settings = default_rotated_right_settings;
      //create new settings file:
      settings_save_changes();
      //display one-time-only message:
      display_first_run_message();
   }

   exit_menu = 0;
   run_menu(&main_menu);
   shutdown();

   return 0;
}

