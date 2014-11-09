#include <stdlib.h>
#include "../portcfg.h"
#include "cfg.h"
#include "gcw.h"
#include "menu_game_settings.h"

//Generic boolean-to-text function:
char* settings_get_text_for_boolean(int setting)
{
   static char* text[2] = { "Off", "On" };
   int i = (setting) ? 1 : 0;
   return text[i];
}

//settings.rotated stuff:
void settings_dec_rotated() 
{
   settings.rotated--;
   if (settings.rotated < 0) {
      settings.rotated = NUM_ROTATIONS - 1;
   }
}

void settings_inc_rotated() 
{
   settings.rotated++;
   if (settings.rotated >= NUM_ROTATIONS) {
      settings.rotated = 0;
   }
}

char* settings_get_text_for_rotated()
{
   static char* text[NUM_ROTATIONS] = { "Horizontal", "Rotated left", "Rotated right" };
   return text[settings.rotated];
}


//settings.no_wait stuff:
void settings_toggle_no_wait() 
{
   settings.no_wait = !settings.no_wait;
}

char* settings_get_text_for_no_wait()
{
   int i = !settings.no_wait;    //no_wait takes the opposite of the meaning of the displayed yes/no
   return settings_get_text_for_boolean(i);
}


//settings.laser_on_by_default stuff:
void settings_toggle_laser_on_by_default()
{
   settings.laser_on_by_default = !settings.laser_on_by_default;
}

char* settings_get_text_for_laser_on_by_default()
{
   return settings_get_text_for_boolean(settings.laser_on_by_default);
}


//settings.music stuff:
void settings_toggle_music()
{
   settings.music = !settings.music;
}
   
char* settings_get_text_for_music()
{
   return settings_get_text_for_boolean(settings.music);
}



//settings.draw_outlines stuff:
void settings_inc_draw_outlines()
{
   settings.draw_outlines++;
   if (settings.draw_outlines >= NUM_DRAW_OUTLINES) {
      settings.draw_outlines = 0;
   }
}

//settings.draw_outlines stuff:
void settings_dec_draw_outlines()
{
   settings.draw_outlines--;
   if (settings.draw_outlines < 0) {
      settings.draw_outlines = NUM_DRAW_OUTLINES - 1;
   }
}

char* settings_get_text_for_draw_outlines()
{
   static char* text[NUM_DRAW_OUTLINES] = { "Never", "IKA mode only", "Always" };
   return text[settings.draw_outlines];
}



//settings.show_fps stuff:
void settings_toggle_show_fps()
{
   settings.show_fps = !settings.show_fps;
}

char* settings_get_text_for_show_fps()
{
   return settings_get_text_for_boolean(settings.show_fps);
}


//settings.extra_lives stuff: 
void settings_inc_extra_lives()
{
   settings.extra_lives++;
   if (settings.extra_lives > MAX_EXTRA_LIVES) {
      settings.extra_lives = 0;
   }
};

void settings_dec_extra_lives()
{
   settings.extra_lives--;
   if (settings.extra_lives < 0) {
      settings.extra_lives = MAX_EXTRA_LIVES;
   }
}

char* settings_get_text_for_extra_lives()
{
   static char* text[MAX_EXTRA_LIVES+1] = { "None", "1", "2", "3", "4", "5", "6" };
   return text[settings.extra_lives];
}



//settings.extra_bombs stuff:
void settings_inc_extra_bombs()
{
   settings.extra_bombs++;
   if (settings.extra_bombs > MAX_EXTRA_BOMBS) {
      settings.extra_bombs = 0;
   }
};

void settings_dec_extra_bombs()
{
   settings.extra_bombs--;
   if (settings.extra_bombs < 0) {
      settings.extra_bombs = MAX_EXTRA_BOMBS;
   }
}

char* settings_get_text_for_extra_bombs()
{
   static char* text[MAX_EXTRA_BOMBS+1] = { "None", "1", "2", "3", "4", "5", "6" };
   return text[settings.extra_bombs];
}

menu_entry game_settings_menu_entries[] = {
   {  .is_special = 0,
      .text = "Screen orientation:",
      .desc_text = "Rotate and zoom-in on playfield (Remember to adjust controls!)",
      .func_get_val_text = settings_get_text_for_rotated,
      .func_select = settings_inc_rotated,
      .func_right = settings_inc_rotated,
      .func_left = settings_dec_rotated },
   {  .is_special = 0,
      .text = "Bullet-time slowdown:",
      .desc_text = "The original game limits the frame-rate when many bullets appear",
      .func_get_val_text = settings_get_text_for_no_wait,
      .func_select = settings_toggle_no_wait,
      .func_right = settings_toggle_no_wait,
      .func_left = settings_toggle_no_wait  },
   {  .is_special = 0,
      .text = "Laser on by default:",
      .desc_text = "You do not have to hold a button to fire the laser",
      .func_get_val_text = settings_get_text_for_laser_on_by_default,
      .func_select = settings_toggle_laser_on_by_default,
      .func_right = settings_toggle_laser_on_by_default,
      .func_left = settings_toggle_laser_on_by_default  },
   {  .is_special = 0,
      .text = "Music:",
      .desc_text = "Turn the game music on or off",
      .func_get_val_text = settings_get_text_for_music,
      .func_select = settings_toggle_music,
      .func_right = settings_toggle_music,
      .func_left = settings_toggle_music  },
   {  .is_special = 0,
      .text = "Draw outlines for bullet shapes:",
      .desc_text = "Draw lines around the edges of bullet shapes",
      .func_get_val_text = settings_get_text_for_draw_outlines,
      .func_select = settings_inc_draw_outlines,
      .func_right = settings_inc_draw_outlines,
      .func_left = settings_dec_draw_outlines  },
   {  .is_special = 0,
      .text = "FPS counter:",
      .desc_text = "Show the current frames-per-second",
      .func_get_val_text = settings_get_text_for_show_fps,
      .func_select = settings_toggle_show_fps,
      .func_right = settings_toggle_show_fps,
      .func_left = settings_toggle_show_fps  },
   {  .is_special = 0,
      .text = "Extra lives (CHEAT):",
      .desc_text = "Warning: You will not be able to record new high scores",
      .func_get_val_text = settings_get_text_for_extra_lives,
      .func_select = settings_inc_extra_lives,
      .func_right = settings_inc_extra_lives,
      .func_left = settings_dec_extra_lives },
   {  .is_special = 0,
      .text = "Extra bombs (CHEAT):",
      .desc_text = "Warning: You will not be able to record new high scores",
      .func_get_val_text = settings_get_text_for_extra_bombs,
      .func_select = settings_inc_extra_bombs,
      .func_right = settings_inc_extra_bombs,
      .func_left = settings_dec_extra_bombs },
   {  .is_special = 0,
      .text = "Save changes and return to main menu",
      .desc_text = "All changes will be saved",
      .func_get_val_text = NULL,
      .func_select = settings_save_changes,
      .func_right = NULL,
      .func_left = NULL },
   {  .is_special = 0,
      .text = "Cancel changes and return to main menu",
      .desc_text = "No changes will be saved",
      .func_get_val_text = NULL,
      .func_select = settings_cancel_changes,
      .func_right = NULL,
      .func_left = NULL }
};

menu game_settings_menu = {
   .num_entries = 10,
   .cur_entry = 0,
   .entries = game_settings_menu_entries
};
