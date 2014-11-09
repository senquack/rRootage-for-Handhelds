#include "wiz_gp2x.h"
// This is not a functioning source file yet, a lot needs to be updated to the more modern GCW-era port.

static const char *base_settings_path = "./";

// Each platform must provide a version of this to set the location of the port-specific configuration settings filename
int set_portcfg_filename() {
   full_portcfg_filename = (char *) malloc( sizeof(base_settings_path) + sizeof(base_portcfg_filename) + 1);
   strcpy(full_portcfg_filename, base_settings_path);
   strcat(full_portcfg_filename, base_portcfg_filename);
}

//TODO: update this to read cpufreq, fast_ram, etc
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
	fprintf(f, "laser_on_by_default=%d\n", new_settings.laser_on_by_default);
	fprintf(f, "rotated=%d\n", new_settings.rotated);
	fprintf(f, "music=%d\n", new_settings.music);
	fprintf(f, "analog_deadzone=%d\n", new_settings.analog_deadzone);
	fprintf(f, "draw_outlines=%d\n", new_settings.draw_outlines);
	fprintf(f, "extra_lives=%d\n", new_settings.extra_lives);
	fprintf(f, "extra_bombs=%d\n", new_settings.extra_bombs);
	fprintf(f, "no_wait=%d\n", new_settings.no_wait);
	fprintf(f, "map_move=%d\n", new_settings.map.move);
	fprintf(f, "map_btn1=%d\n", new_settings.map.btn1);
	fprintf(f, "map_btn1_alt=%d\n", new_settings.map.btn1_alt);
	fprintf(f, "map_btn2=%d\n", new_settings.map.btn2);
	fprintf(f, "map_btn2_alt=%d\n", new_settings.map.btn2_alt);
	fprintf(f, "map_pause=%d\n", new_settings.map.pause);
	fprintf(f, "map_exit=%d\n", new_settings.map.exit);
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
			new_settings.laser_on_by_default = clamp(atoi(param), 0, 1);
		else if ( strcasecmp(str, "rotated") == 0 )
			new_settings.rotated = clamp(atoi(param), 0, NUM_ROTATIONS-1);
		else if ( strcasecmp(str, "fast_ram") == 0 )
			new_settings.fast_ram = clamp(atoi(param), 0, 1);
		else if ( strcasecmp(str, "cpu_freq") == 0 )
		{
			if ((strlen(param) > 0) && (strlen(param) < 4))
			{
				new_settings.cpu_freq = atoi(param);
				if (new_settings.cpu_freq)
				{
					if ((new_settings.cpu_freq < MIN_CPU_FREQ) || (new_settings.cpu_freq > MAX_CPU_FREQ))
					{ 
						printf("Error: \"cpu_freq\" parameter out of valid range in %s\n", filename);
						printf("Valid range is %d-%d or 0 if overclocking is disabled.\n",
							MIN_CPU_FREQ, MAX_CPU_FREQ);
						printf("Setting parameter to default value %d.\n", default_settings.cpu_freq);
						new_settings.cpu_freq = default_settings.cpu_freq;
					}	
				}
			} else {
				printf("Error parsing \"cpu_freq\" parameter in %s\n", filename);
				printf("Setting parameter to default value of %d.\n", default_settings.cpu_freq);
				new_settings.cpu_freq = default_settings.cpu_freq;
			}
		}
		else if ( strcasecmp(str, "music") == 0 )
			new_settings.music = clamp(atoi(param), 0, 1);
		else if ( strcasecmp(str, "analog_deadzone") == 0 )
			new_settings.analog_deadzone = clamp(atoi(param), ANALOG_DEADZONE_MIN, ANALOG_DEADZONE_MAX);
		else if ( strcasecmp(str, "draw_outlines") == 0 )
			new_settings.draw_outlines = clamp(atoi(param), 0, NUM_DRAW_OUTLINES-1);
		else if ( strcasecmp(str, "extra_lives") == 0 )
			new_settings.extra_lives = clamp(atoi(param), 0, MAX_EXTRA_LIVES);
		else if ( strcasecmp(str, "extra_bombs") == 0 )
			new_settings.extra_bombs = clamp(atoi(param), 0, MAX_EXTRA_BOMBS);
		else if ( strcasecmp(str, "no_wait") == 0 )
			new_settings.no_wait = clamp(atoi(param), 0, 1);
		else if ( strcasecmp(str, "map_move") == 0 )
			new_settings.map.move = clamp(atoi(param), 0, NUM_MAPS-1);
		else if ( strcasecmp(str, "map_btn1") == 0 )
			new_settings.map.btn1 = clamp(atoi(param), 0, NUM_MAPS-1);
		else if ( strcasecmp(str, "map_btn1_alt") == 0 )
			new_settings.map.btn1_alt = clamp(atoi(param), 0, NUM_MAPS-1);
		else if ( strcasecmp(str, "map_btn2") == 0 )
			new_settings.map.btn2 = clamp(atoi(param), 0, NUM_MAPS-1);
		else if ( strcasecmp(str, "map_btn2_alt") == 0 )
			new_settings.map.btn2_alt = clamp(atoi(param), 0, NUM_MAPS-1);
		else if ( strcasecmp(str, "map_pause") == 0 )
			new_settings.map.pause = clamp(atoi(param), 0, NUM_MAPS-1);
		else if ( strcasecmp(str, "map_exit") == 0 )
			new_settings.map.exit = clamp(atoi(param), 0, NUM_MAPS-1);
		else
		{
			printf("Ignoring unknown setting: %s\n", str);
		}
	}

	fclose(f);
	return 1;
}
//TODO: Complete stub function:
// Returns 0 if an SDL_QUIT event is received
int update_control_state()
{
   SDL_Event event;

   while(SDL_PollEvent(&event)) {
      switch(event.type) {
         case SDL_QUIT:
            return 0;
            break;


         // INCOMPLETE: needs joystick button handling code here
      }
   }
   return 1;
}

static const char *base_settings_path = "./";

