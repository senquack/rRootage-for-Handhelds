#include <stdlib.h>

#include "../portcfg.h"
#include "cfg.h"
#include "gcw.h"

#include "menu_control_settings.h"

//settings.map.move stuff:
void settings_dec_move() 
{
   settings.map.move--;
   if (settings.map.move < MAP_DPAD) {
      settings.map.move = MAP_ANALOG;
   }
}

void settings_inc_move() 
{
   settings.map.move++;
   if (settings.map.move > MAP_ANALOG) {
      settings.map.move = MAP_DPAD;
   }
}

char* settings_get_text_for_move()
{
   static char* text[3] = { "DPAD", "A/B/X/Y", "Analog stick" };
   int i = settings.map.move - MAP_DPAD;
   return text[i];
}

char* settings_get_text_for_mapping(int mapping)
{
   static char* text[NUM_MAPS] = { "None", "A", "B", "X", "Y", "Start", "Select", "L Trigger", "R Trigger",
                                    "DPAD", "A/B/X/Y", "Analog stick" };
   return text[mapping];
}

//settings.map.btn1 stuff:
void settings_dec_btn1() 
{
   settings.map.btn1--;
   if (settings.map.btn1 < 0) {
      settings.map.btn1 = NUM_MAPS-1;
   }
}

void settings_inc_btn1() 
{
   settings.map.btn1++;
   if (settings.map.btn1 >= NUM_MAPS) {
      settings.map.btn1 = 0;
   }
}

char* settings_get_text_for_btn1()
{
   return settings_get_text_for_mapping(settings.map.btn1);
}

//settings.map.btn1_alt stuff:
void settings_dec_btn1_alt() 
{
   settings.map.btn1_alt--;
   if (settings.map.btn1_alt < 0) {
      settings.map.btn1_alt = NUM_MAPS-1;
   }
}

void settings_inc_btn1_alt() 
{
   settings.map.btn1_alt++;
   if (settings.map.btn1_alt >= NUM_MAPS) {
      settings.map.btn1_alt = 0;
   }
}

char* settings_get_text_for_btn1_alt()
{
   return settings_get_text_for_mapping(settings.map.btn1_alt);
}

//settings.map.btn2 stuff:
void settings_dec_btn2() 
{
   settings.map.btn2--;
   if (settings.map.btn2 < 0) {
      settings.map.btn2 = NUM_MAPS-1;
   }
}

void settings_inc_btn2() 
{
   settings.map.btn2++;
   if (settings.map.btn2 >= NUM_MAPS) {
      settings.map.btn2 = 0;
   }
}

char* settings_get_text_for_btn2()
{
   return settings_get_text_for_mapping(settings.map.btn2);
}

//settings.map.btn2_alt stuff:
void settings_dec_btn2_alt() 
{
   settings.map.btn2_alt--;
   if (settings.map.btn2_alt < 0) {
      settings.map.btn2_alt = NUM_MAPS-1;
   }
}

void settings_inc_btn2_alt() 
{
   settings.map.btn2_alt++;
   if (settings.map.btn2_alt >= NUM_MAPS) {
      settings.map.btn2_alt = 0;
   }
}

char* settings_get_text_for_btn2_alt()
{
   return settings_get_text_for_mapping(settings.map.btn2_alt);
}

//settings.map.pause stuff:
void settings_dec_pause() 
{
   settings.map.pause--;
   if (settings.map.pause < 0) {
      settings.map.pause = NUM_MAPS-1;
   }
}

void settings_inc_pause() 
{
   settings.map.pause++;
   if (settings.map.pause >= NUM_MAPS) {
      settings.map.pause = 0;
   }
}

char* settings_get_text_for_pause()
{
   return settings_get_text_for_mapping(settings.map.pause);
}

//settings.map.exit stuff:
void settings_dec_exit() 
{
   settings.map.exit--;
   if (settings.map.exit < 0) {
      settings.map.exit = NUM_MAPS-1;
   }
}

void settings_inc_exit() 
{
   settings.map.exit++;
   if (settings.map.exit >= NUM_MAPS) {
      settings.map.exit = 0;
   }
}

char* settings_get_text_for_exit()
{
   return settings_get_text_for_mapping(settings.map.exit);
}

//settings.analog_deadzone stuff:
void settings_dec_analog_deadzone() 
{
   settings.analog_deadzone -= 1000;
   if (settings.analog_deadzone < ANALOG_DEADZONE_MIN) {
      settings.analog_deadzone = ANALOG_DEADZONE_MAX;
   }
}

void settings_inc_analog_deadzone() 
{
   settings.analog_deadzone += 1000;
   if (settings.analog_deadzone > ANALOG_DEADZONE_MAX) {
      settings.analog_deadzone = ANALOG_DEADZONE_MIN;
   }
}

char* settings_get_text_for_analog_deadzone()
{
   static char* text[] = { "1000", "2000", "3000", "4000", "5000", "6000", "7000", "8000", "9000", "10000",
                    "11000", "12000", "13000", "14000", "15000", "16000", "17000", "18000", "19000", "20000",
                    "21000", "22000", "23000", "24000", "25000", "26000", "27000", "28000", "29000", "30000" };
   return text[settings.analog_deadzone / 1000 - 1];
}

menu_entry control_settings_menu_entries[] = {
   {  .is_special = 0,
      .text = "Move:",
      .desc_text = "Ship movement control",
      .func_get_val_text = settings_get_text_for_move,
      .func_select = settings_inc_move,
      .func_right = settings_inc_move,
      .func_left = settings_dec_move },
   {  .is_special = 0,
      .text = "Laser:",
      .desc_text = "Firing of laser",
      .func_get_val_text = settings_get_text_for_btn1,
      .func_select = settings_inc_btn1,
      .func_right = settings_inc_btn1,
      .func_left = settings_dec_btn1  },
   {  .is_special = 0,
      .text = "Laser (alternate):",
      .desc_text = "Firing of laser (additional button mapping)",
      .func_get_val_text = settings_get_text_for_btn1_alt,
      .func_select = settings_inc_btn1_alt,
      .func_right = settings_inc_btn1_alt,
      .func_left = settings_dec_btn1_alt  },
   {  .is_special = 0,
      .text = "Bomb:",
      .desc_text = "Dropping of bombs",
      .func_get_val_text = settings_get_text_for_btn2,
      .func_select = settings_inc_btn2,
      .func_right = settings_inc_btn2,
      .func_left = settings_dec_btn2  },
   {  .is_special = 0,
      .text = "Bomb (alternate):",
      .desc_text = "Dropping of bombs (additional button mapping)",
      .func_get_val_text = settings_get_text_for_btn2_alt,
      .func_select = settings_inc_btn2_alt,
      .func_right = settings_inc_btn2_alt,
      .func_left = settings_dec_btn2_alt  },
   {  .is_special = 0,
      .text = "Pause:",
      .desc_text = "Pause the game",
      .func_get_val_text = settings_get_text_for_pause,
      .func_select = settings_inc_pause,
      .func_right = settings_inc_pause,
      .func_left = settings_dec_pause  },
   {  .is_special = 0,
      .text = "Quit:",
      .desc_text = "Quit current game and return to main menu",
      .func_get_val_text = settings_get_text_for_exit,
      .func_select = settings_inc_exit,
      .func_right = settings_inc_exit,
      .func_left = settings_dec_exit  },
   {  .is_special = 0,
      .text = "Analog stick deadzone:",
      .desc_text = "A lower number makes the analog nub more sensitive",
      .func_get_val_text = settings_get_text_for_analog_deadzone,
      .func_select = settings_inc_analog_deadzone,
      .func_right = settings_inc_analog_deadzone,
      .func_left = settings_dec_analog_deadzone  },
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

menu control_settings_menu = {
   .num_entries = 10,
   .cur_entry = 0,
   .entries = control_settings_menu_entries
};

