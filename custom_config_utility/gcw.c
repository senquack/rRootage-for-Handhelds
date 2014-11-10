//#include <sys/stat.h>
//#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include	<SDL.h>

#include "cfg.h"
#include "../portcfg.h"
#include "gcw.h"

static const char *base_settings_path = ".rrootage/";

portcfg_settings default_rotated_right_settings = {    
   .laser_on_by_default    = 1,                        // Is laser on by default? (more comfortable on handhelds) 
   .rotated                = SCREEN_ROTATED_RIGHT,     // Is screen rotated? Assigned to one of: 
                                                       //    SCREEN_HORIZ, SCREEN_ROTATED_LEFT, SCREEN_ROTATED_RIGHT
   .music                  = 1,                        // Is music enabled?
   .analog_deadzone        = ANALOG_DEADZONE_DEFAULT,  // Analog joystick deadzone
   .draw_outlines          = DRAW_OUTLINES_IKA,        // Which mode of bullet-outline drawing to use
   .extra_lives            = 0,                        // Cheat which adds up to 6 extra lives at start 
                                                       //   (but disables ability to save new high scores)
   .extra_bombs            = 0,                        // Cheat which adds up to 6 extra bombs at start 
                                                       //   (but disables ability to save new high scores)
   .no_wait                = 0,                        // Enables the --nowait option, where automatic bullet slowdown (and fps limiting) is disabled
   .show_fps               = 0,                        // Show FPS counter
   .map                    = {
      .move     = MAP_DPAD,
      .btn1     = MAP_R,      //Laser mapping
      .btn2     = MAP_ANALOG, //Bomb mapping
      .btn1_alt = MAP_A,      //Laser alternate mapping
      .btn2_alt = MAP_Y,      //Bomb alternate mapping
      .pause    = MAP_START,  //Pause mapping
      .exit     = MAP_SELECT  //Exit to menu mapping 
   }
};     

portcfg_settings default_rotated_left_settings = {    
   .laser_on_by_default    = 1,                           // Is laser on by default? (more comfortable on handhelds) 
   .rotated                = SCREEN_ROTATED_LEFT,         // Is screen rotated? Assigned to one of: 
                                                          //    SCREEN_HORIZ, SCREEN_ROTATED_LEFT, SCREEN_ROTATED_RIGHT
   .music                  = 1,                           // Is music enabled?
   .analog_deadzone        = ANALOG_DEADZONE_DEFAULT,                        // Analog joystick deadzone
   .draw_outlines          = DRAW_OUTLINES_IKA,        // Which mode of bullet-outline drawing to use
   .extra_lives            = 0,                        // Cheat which adds up to 6 extra lives at start 
                                                       //   (but disables ability to save new high scores)
   .extra_bombs            = 0,                        // Cheat which adds up to 6 extra bombs at start 
                                                       //   (but disables ability to save new high scores)
   .no_wait                = 0,                        // Enables the --nowait option, where automatic bullet slowdown (and fps limiting) is disabled
   .show_fps               = 0,                        // Show FPS counter
   .map                    = {
      .move     = MAP_ABXY,   //Movement mapping
      .btn1     = MAP_SELECT, //Laser mapping
      .btn2     = MAP_START,  //Bomb mapping
      .btn1_alt = MAP_NONE,      //Laser alternate mapping
      .btn2_alt = MAP_NONE,      //Bomb alternate mapping
      .pause    = MAP_L,  //Pause mapping
      .exit     = MAP_DPAD  //Exit to menu mapping 
   }
};     

portcfg_settings default_horizontal_settings = {    
   .laser_on_by_default    = 1,                           // Is laser on by default? (more comfortable on handhelds) 
   .rotated                = SCREEN_HORIZ,                // Is screen rotated? Assigned to one of: 
                                                          //    SCREEN_HORIZ, SCREEN_ROTATED_LEFT, SCREEN_ROTATED_RIGHT
   .music                  = 1,                           // Is music enabled?
   .analog_deadzone        = ANALOG_DEADZONE_DEFAULT,                        // Analog joystick deadzone
   .draw_outlines          = DRAW_OUTLINES_IKA,        // Which mode of bullet-outline drawing to use
   .extra_lives            = 0,                        // Cheat which adds up to 6 extra lives at start 
                                                       //   (but disables ability to save new high scores)
   .extra_bombs            = 0,                        // Cheat which adds up to 6 extra bombs at start 
                                                       //   (but disables ability to save new high scores)
   .no_wait                = 0,                        // Enables the --nowait option, where automatic bullet slowdown (and fps limiting) is disabled
   .show_fps               = 0,                        // Show FPS counter
   .map                    = {
      .move     = MAP_DPAD,   //Movement mapping
      .btn1     = MAP_X,      //Laser mapping
      .btn2     = MAP_B,      //Bomb mapping
      .btn1_alt = MAP_A,      //Laser alternate mapping
      .btn2_alt = MAP_R,      //Bomb alternate mapping
      .pause    = MAP_START,  //Pause mapping
      .exit     = MAP_SELECT  //Exit to menu mapping 
   }
};     

// Each platform must provide a version of this to set the location of the port-specific configuration settings filename
int set_filenames() {
   //Create base settings directory if it doesn't exist yet:
   char *tmp_pathname = (char *) malloc( strlen(getenv("HOME")) + 1 + strlen(base_settings_path) + 1);
   sprintf(tmp_pathname, "%s/%s", getenv("HOME"), base_settings_path);
   printf("Ensuring settings directory exists, creating if not: %s\n", tmp_pathname);
   if ( !create_dir(tmp_pathname) ) {
      printf("Unable to create missing settings directory.\n");
      return 0;
   }
   free(tmp_pathname);

   if (getenv("HOME")) {
      //Set port-specific configuration filename (rr.conf)
      full_portcfg_filename = (char *) malloc( strlen(getenv("HOME")) + 1 + 
            strlen(base_settings_path) + strlen(base_portcfg_filename) + 1);
      sprintf(full_portcfg_filename, "%s/%s%s", getenv("HOME"), base_settings_path, base_portcfg_filename);
      printf("Using %s file: %s\n", base_portcfg_filename, full_portcfg_filename);

      //Set rrootage's binary preferences filename (rr.bin)
      full_rrbin_filename = (char *) malloc( strlen(getenv("HOME")) + 1 + 
            strlen(base_settings_path) + strlen(base_rrbin_filename) + 1);
      sprintf(full_rrbin_filename, "%s/%s%s", getenv("HOME"), base_settings_path, base_rrbin_filename);
      printf("Using %s file: %s\n", base_rrbin_filename, full_rrbin_filename);
   } else {
      printf("Failed to get $HOME directory environment variable\n");
      return 0;
   }

   return 1;
}

//  Return 1 on success, 0 on error.  Write settings from structure.
int write_new_portcfg(const char *filename)
{
   if (!filename)
   {
      printf("Error: NULL passed to write_new_portcfg.\n");
      return 0;
   }

	FILE *f;

	f = fopen(filename, "w");
	if (f == NULL)
	{
	  printf("Error opening file for writing: %s\n", filename);
	  return 0;
	}
	else
	{
	  printf("Writing settings to file: %s\n", filename);
	}
	fprintf(f, "laser_on_by_default=%d\n", settings.laser_on_by_default);
	fprintf(f, "rotated=%d\n", settings.rotated);
	fprintf(f, "music=%d\n", settings.music);
	fprintf(f, "analog_deadzone=%d\n", settings.analog_deadzone);
	fprintf(f, "draw_outlines=%d\n", settings.draw_outlines);
	fprintf(f, "extra_lives=%d\n", settings.extra_lives);
	fprintf(f, "extra_bombs=%d\n", settings.extra_bombs);
	fprintf(f, "no_wait=%d\n", settings.no_wait);
	fprintf(f, "show_fps=%d\n", settings.show_fps);
	fprintf(f, "map_move=%d\n", settings.map.move);
	fprintf(f, "map_btn1=%d\n", settings.map.btn1);
	fprintf(f, "map_btn1_alt=%d\n", settings.map.btn1_alt);
	fprintf(f, "map_btn2=%d\n", settings.map.btn2);
	fprintf(f, "map_btn2_alt=%d\n", settings.map.btn2_alt);
	fprintf(f, "map_pause=%d\n", settings.map.pause);
	fprintf(f, "map_exit=%d\n", settings.map.exit);
	return (fclose(f) == 0);
}

//  Return 1 on success, 0 on error.  Read settings into new_settings structure
int read_portcfg_settings(const char *filename)
{
	FILE *f;
	char buf[8192];
	char *str, *param;

   if (!filename)
   {
      printf("Error: NULL passed to read_portcfg_settings.\n");
      return 0;
   }

	f = fopen(filename, "r");
	if (f == NULL)
	{
		printf("Error opening file: %s\n", filename);
		return 0;
	}
	else
	{
		printf("Loading settings from file: %s\n", filename);
	}

	while (!feof(f))
	{
		// skip empty lines
		fscanf(f, "%8192[\n\r]", buf);

		// read line
		buf[0] = 0;
		fscanf(f, "%8192[^\n^\r]", buf);

		// trim line
		str = trim_string(buf);

		if (str[0] == 0) continue;
		if (str[0] == '#') continue;

		// find parameter (after '=')
		param = strchr(str, '=');

		if (param == NULL) continue;

		// split string into two strings
		*param = 0;
		param++;

		// trim them
		str = trim_string(str);
		param = trim_string(param);

		if ( strcasecmp(str, "laser_on_by_default") == 0 )
			settings.laser_on_by_default = clamp(atoi(param), 0, 1);
		else if ( strcasecmp(str, "rotated") == 0 )
			settings.rotated = clamp(atoi(param), 0, NUM_ROTATIONS-1);
		else if ( strcasecmp(str, "music") == 0 )
			settings.music = clamp(atoi(param), 0, 1);
		else if ( strcasecmp(str, "analog_deadzone") == 0 )
			settings.analog_deadzone = clamp(atoi(param), ANALOG_DEADZONE_MIN, ANALOG_DEADZONE_MAX);
		else if ( strcasecmp(str, "draw_outlines") == 0 )
			settings.draw_outlines = clamp(atoi(param), 0, NUM_DRAW_OUTLINES-1);
		else if ( strcasecmp(str, "extra_lives") == 0 )
			settings.extra_lives = clamp(atoi(param), 0, MAX_EXTRA_LIVES);
		else if ( strcasecmp(str, "extra_bombs") == 0 )
			settings.extra_bombs = clamp(atoi(param), 0, MAX_EXTRA_BOMBS);
		else if ( strcasecmp(str, "no_wait") == 0 )
			settings.no_wait = clamp(atoi(param), 0, 1);
		else if ( strcasecmp(str, "show_fps") == 0 )
			settings.show_fps = clamp(atoi(param), 0, 1);
		else if ( strcasecmp(str, "map_move") == 0 )
			settings.map.move = clamp(atoi(param), MAP_DPAD, NUM_MAPS-1);
		else if ( strcasecmp(str, "map_btn1") == 0 )
			settings.map.btn1 = clamp(atoi(param), 0, NUM_MAPS-1);
		else if ( strcasecmp(str, "map_btn1_alt") == 0 )
			settings.map.btn1_alt = clamp(atoi(param), 0, NUM_MAPS-1);
		else if ( strcasecmp(str, "map_btn2") == 0 )
			settings.map.btn2 = clamp(atoi(param), 0, NUM_MAPS-1);
		else if ( strcasecmp(str, "map_btn2_alt") == 0 )
			settings.map.btn2_alt = clamp(atoi(param), 0, NUM_MAPS-1);
		else if ( strcasecmp(str, "map_pause") == 0 )
			settings.map.pause = clamp(atoi(param), 0, NUM_MAPS-1);
		else if ( strcasecmp(str, "map_exit") == 0 )
			settings.map.exit = clamp(atoi(param), 0, NUM_MAPS-1);
		else
		{
			printf("Ignoring unknown setting: %s\n", str);
		}
	}

	fclose(f);
	return 1;
}

void reset_control_state()
{
	memset(control_state, 0, sizeof( control_state));
}

// Returns 0 if an SDL_QUIT event is received
int update_control_state()
{
   SDL_Event event;

   while(SDL_PollEvent(&event)) {
      switch(event.type) {
         case SDL_QUIT:
            return 0;
            break;
         case SDL_KEYDOWN:
         case SDL_KEYUP:
            switch(event.key.keysym.sym) {
               case SDLK_UP:
                  control_state[MAP_UTIL_UP] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                  break;
               case SDLK_DOWN:
                  control_state[MAP_UTIL_DOWN] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                  break;
               case SDLK_LEFT:
                  control_state[MAP_UTIL_LEFT] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                  break;
               case SDLK_RIGHT:
                  control_state[MAP_UTIL_RIGHT] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                  break;
               case SDLK_SPACE:	// GCW Y button
               case SDLK_LALT:	// GCW B button
               case SDLK_LSHIFT:	// GCW X button
               case SDLK_LCTRL:	// GCW A button
                  control_state[MAP_UTIL_SELECT] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                  break;
               case SDLK_BACKSPACE:	// GCW R trigger
               case SDLK_TAB:	      // GCW L trigger
               case SDLK_RETURN:	// GCW Start button
                  control_state[MAP_UTIL_STARTBUTTON] = (event.type == SDL_KEYDOWN) ? 1: 0;
                  break;
               case SDLK_ESCAPE:	// GCW Select button
                  break;
                  // Do nothing
               default:
                  break;
            }
      }
   }
   return 1;
}

