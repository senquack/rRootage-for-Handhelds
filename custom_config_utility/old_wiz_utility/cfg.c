/*	rRootage Wiz port configurator	
 	Copyright (C) 2008 Dan Silsby (Senor Quack)
	 Portions of code based on :

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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <unistd.h>
#include <stropts.h>
#include <errno.h>
#include	<SDL.h>
#include	<SDL_image.h>
#include <SDL_gfxPrimitives.h>
#include	"SFont.h"	

#define GP2X_BUTTON_UP              (0)
#define GP2X_BUTTON_DOWN            (4)
#define GP2X_BUTTON_LEFT            (2)
#define GP2X_BUTTON_RIGHT           (6)
#define GP2X_BUTTON_UPLEFT          (1)
#define GP2X_BUTTON_UPRIGHT         (7)
#define GP2X_BUTTON_DOWNLEFT        (3)
#define GP2X_BUTTON_DOWNRIGHT       (5)
#define GP2X_BUTTON_CLICK           (18)
#define GP2X_BUTTON_A               (12)
#define GP2X_BUTTON_B               (13)
#define GP2X_BUTTON_X               (14)
#define GP2X_BUTTON_Y               (15)
#define GP2X_BUTTON_L               (10)
#define GP2X_BUTTON_R               (11)
#define GP2X_BUTTON_START           (8)
#define GP2X_BUTTON_SELECT          (9)
#define GP2X_BUTTON_VOLUP           (16)
#define GP2X_BUTTON_VOLDOWN         (17)
#define NUM_BUTTONS  (19) // total number of buttons we'll need to keep track of


#define INPUT_DELAY		(125)	//# ms to ignore input after a button is pressed
#define MENU_DELAY		(10)
#define MAX_CPU_FREQ		(990)
#define MIN_CPU_FREQ		(540)
#define CPU_FREQ_STEP	(10)
#define NUM_DEFS 				(8) //number of button redefinition settings
#define FIRE_IDX				(0)
#define FIRE2_IDX				(1)
#define SPECIAL_IDX			(2)
#define SPECIAL2_IDX			(3)
#define PAUSE_IDX				(4)
#define EXIT_IDX				(5)
#define VOLDOWN_IDX			(6)
#define VOLUP_IDX				(7)

#define NUM_USABLE_BUTTONS	(10)	//number of GP2X buttons that can be assigned 


typedef struct {
	int	laser_on_by_default;
	int	rotated;
	int	cpu_freq;	// if this is 0, overclocking is disabled
	int	fast_ram;
	int	music;
	int	buttons[NUM_DEFS], rbuttons[NUM_DEFS];		//rbuttons is for rotated button definitions
} settings;

settings default_settings = { 
	1, 1, 700, 1, 1,
	// non-rotated button defs:
	{ GP2X_BUTTON_Y, GP2X_BUTTON_R, GP2X_BUTTON_A, GP2X_BUTTON_L,
		GP2X_BUTTON_START, GP2X_BUTTON_SELECT, GP2X_BUTTON_VOLDOWN, GP2X_BUTTON_VOLUP }, 
	// rotated button defs: //19 means disabled
	{ GP2X_BUTTON_START, GP2X_BUTTON_X, GP2X_BUTTON_R, GP2X_BUTTON_A, 
		GP2X_BUTTON_Y, GP2X_BUTTON_SELECT, GP2X_BUTTON_VOLDOWN, GP2X_BUTTON_VOLUP }
};

settings new_settings;

extern int errno;
FILE settings_file;

//global surfaces
SDL_Surface* screen;				// framebuffer
SDL_Surface* background;		// background containing test images, etc

//global fonts
SFont_Font *font_wh; //white

//global joysticks
SDL_Joystick* joy;

//global constants
const int screenw = 320;
const int screenh = 240;
const int screenbpp = 16;
const int screenflags = SDL_SWSURFACE;
 
const char *settings_filename = "./rr.conf";

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

//  Return 1 on success, 0 on error.  Write settings from structure.
int write_new_settings(const char *filename)
{
	FILE *f;
//	char buf[50];

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
	fprintf(f, "cpu_freq=%d\n", new_settings.cpu_freq);
	fprintf(f, "fast_ram=%d\n", new_settings.fast_ram);
	fprintf(f, "music=%d\n", new_settings.music);
	fprintf(f, "buttons_fire=%d\n", new_settings.buttons[FIRE_IDX]);
	fprintf(f, "buttons_fire2=%d\n", new_settings.buttons[FIRE2_IDX]);
	fprintf(f, "buttons_special=%d\n", new_settings.buttons[SPECIAL_IDX]);
	fprintf(f, "buttons_special2=%d\n", new_settings.buttons[SPECIAL2_IDX]);
	fprintf(f, "buttons_pause=%d\n", new_settings.buttons[PAUSE_IDX]);
	fprintf(f, "buttons_exit=%d\n", new_settings.buttons[EXIT_IDX]);
	fprintf(f, "buttons_voldown=%d\n", new_settings.buttons[VOLDOWN_IDX]);
	fprintf(f, "buttons_volup=%d\n", new_settings.buttons[VOLUP_IDX]);
	fprintf(f, "rbuttons_fire=%d\n", new_settings.rbuttons[FIRE_IDX]);
	fprintf(f, "rbuttons_fire2=%d\n", new_settings.rbuttons[FIRE2_IDX]);
	fprintf(f, "rbuttons_special=%d\n", new_settings.rbuttons[SPECIAL_IDX]);
	fprintf(f, "rbuttons_special2=%d\n", new_settings.rbuttons[SPECIAL2_IDX]);
	fprintf(f, "rbuttons_pause=%d\n", new_settings.rbuttons[PAUSE_IDX]);
	fprintf(f, "rbuttons_exit=%d\n", new_settings.rbuttons[EXIT_IDX]);
	fprintf(f, "rbuttons_voldown=%d\n", new_settings.rbuttons[VOLDOWN_IDX]);
	fprintf(f, "rbuttons_volup=%d\n", new_settings.rbuttons[VOLUP_IDX]);
	return (fclose(f) == 0);
}

//  Return 1 on success, 0 on error.  Read settings into new_settings structure
int read_saved_settings(const char *filename)
{
	FILE *f;
	char buf[8192];
	char *str, *param;

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
			new_settings.rotated = clamp(atoi(param), 0, 1);
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
		else if ( strcasecmp(str, "buttons_fire") == 0 )
			new_settings.buttons[FIRE_IDX] = clamp(atoi(param), 0, NUM_BUTTONS);
		else if ( strcasecmp(str, "buttons_fire2") == 0 )
			new_settings.buttons[FIRE2_IDX] = clamp(atoi(param), 0, NUM_BUTTONS);
		else if ( strcasecmp(str, "buttons_special") == 0 )
			new_settings.buttons[SPECIAL_IDX] = clamp(atoi(param), 0, NUM_BUTTONS);
		else if ( strcasecmp(str, "buttons_special2") == 0 )
			new_settings.buttons[SPECIAL2_IDX] = clamp(atoi(param), 0, NUM_BUTTONS);
		else if ( strcasecmp(str, "buttons_pause") == 0 )
			new_settings.buttons[PAUSE_IDX] = clamp(atoi(param), 0, NUM_BUTTONS);
		else if ( strcasecmp(str, "buttons_exit") == 0 )
			new_settings.buttons[EXIT_IDX] = clamp(atoi(param), 0, NUM_BUTTONS);
		else if ( strcasecmp(str, "buttons_voldown") == 0 )
			new_settings.buttons[VOLDOWN_IDX] = clamp(atoi(param), 0, NUM_BUTTONS);
		else if ( strcasecmp(str, "buttons_volup") == 0 )
			new_settings.buttons[VOLUP_IDX] = clamp(atoi(param), 0, NUM_BUTTONS);
		else if ( strcasecmp(str, "rbuttons_fire") == 0 )
			new_settings.rbuttons[FIRE_IDX] = clamp(atoi(param), 0, NUM_BUTTONS);
		else if ( strcasecmp(str, "rbuttons_fire2") == 0 )
			new_settings.rbuttons[FIRE2_IDX] = clamp(atoi(param), 0, NUM_BUTTONS);
		else if ( strcasecmp(str, "rbuttons_special") == 0 )
			new_settings.rbuttons[SPECIAL_IDX] = clamp(atoi(param), 0, NUM_BUTTONS);
		else if ( strcasecmp(str, "rbuttons_special2") == 0 )
			new_settings.rbuttons[SPECIAL2_IDX] = clamp(atoi(param), 0, NUM_BUTTONS);
		else if ( strcasecmp(str, "rbuttons_pause") == 0 )
			new_settings.rbuttons[PAUSE_IDX] = clamp(atoi(param), 0, NUM_BUTTONS);
		else if ( strcasecmp(str, "rbuttons_exit") == 0 )
			new_settings.rbuttons[EXIT_IDX] = clamp(atoi(param), 0, NUM_BUTTONS);
		else if ( strcasecmp(str, "rbuttons_voldown") == 0 )
			new_settings.rbuttons[VOLDOWN_IDX] = clamp(atoi(param), 0, NUM_BUTTONS);
		else if ( strcasecmp(str, "rbuttons_volup") == 0 )
			new_settings.rbuttons[VOLUP_IDX] = clamp(atoi(param), 0, NUM_BUTTONS);
		else
		{
			printf("Ignoring unknown setting: %s\n", str);
		}
	}

	fclose(f);
	return 1;
}

int load_graphics(void)
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

void unload_graphics(void)
{
	SDL_FreeSurface(background);

	if (font_wh) 
	{
		SFont_FreeFont(font_wh);
	}
}

void shutdown(void)
{
	unload_graphics();
	SDL_Quit();
}

void controls_menu(void)
{
#define CONTROLS_MENU_LINES					10
#define CONTROLS_MENU_FIRE_LINE				0
#define CONTROLS_MENU_FIRE2_LINE				1
#define CONTROLS_MENU_SPECIAL_LINE			2
#define CONTROLS_MENU_SPECIAL2_LINE			3
#define CONTROLS_MENU_PAUSE_LINE				4
#define CONTROLS_MENU_EXIT_LINE				5
#define CONTROLS_MENU_VOLDOWN_LINE			6
#define CONTROLS_MENU_VOLUP_LINE				7
#define CONTROLS_MENU_ACCEPT_LINE			8
#define CONTROLS_MENU_CANCEL_LINE			9

	SDL_Rect	srcrect, dstrect;
	const int menu_y = 80;			// top of menu y coord
	const int line_y[] = { menu_y+10, menu_y+20, menu_y+30, menu_y+40, menu_y+50,
  									menu_y+60, menu_y+70, menu_y+80, menu_y+100, menu_y+110 };
	const int line_x = 100;			// x offset of menu lines' text
	const int line_x_col2 = 190;
	const int menu_height = 133;
	const int menu_width = 200;
	
	int quit = 0;
	int current_line = 0;
	int last_input_time = 0;	// What was the ticks value when we last took a button input?
	char tmp_str[50];

	const char* button_strs[] = {
		"A", "B", "X", "Y", "L Trigger", "R Trigger", 
		"Menu", "Select", "Vol -", "Vol+", "DISABLED" };

	const int usable_button_to_gp2x_map[NUM_USABLE_BUTTONS+1] = { GP2X_BUTTON_A, GP2X_BUTTON_B, GP2X_BUTTON_X, GP2X_BUTTON_Y,
			GP2X_BUTTON_L, GP2X_BUTTON_R, GP2X_BUTTON_START, GP2X_BUTTON_SELECT, GP2X_BUTTON_VOLDOWN, GP2X_BUTTON_VOLUP,
			NUM_BUTTONS};

	const int gp2x_to_usable_button_map[NUM_BUTTONS+1] = { NUM_USABLE_BUTTONS, 
																				NUM_USABLE_BUTTONS, 
																				NUM_USABLE_BUTTONS,
																				NUM_USABLE_BUTTONS, 
																				NUM_USABLE_BUTTONS,
																				NUM_USABLE_BUTTONS, 
																				NUM_USABLE_BUTTONS, 
																				NUM_USABLE_BUTTONS, 
																				6, 7, 5, 4, 0, 1, 3, 2, 9, 8,
																				NUM_USABLE_BUTTONS,
																				NUM_USABLE_BUTTONS };

	int tmp_button_map[NUM_DEFS] = { gp2x_to_usable_button_map[new_settings.buttons[0]], 
												gp2x_to_usable_button_map[new_settings.buttons[1]], 
												gp2x_to_usable_button_map[new_settings.buttons[2]], 
												gp2x_to_usable_button_map[new_settings.buttons[3]], 
												gp2x_to_usable_button_map[new_settings.buttons[4]], 
												gp2x_to_usable_button_map[new_settings.buttons[5]], 
												gp2x_to_usable_button_map[new_settings.buttons[6]], 
												gp2x_to_usable_button_map[new_settings.buttons[7]] };

	int i;
	for (i = 0; i < (menu_width>>1); i+=4)
	{
		//blit background
		SDL_BlitSurface(background, NULL, screen, NULL);
		// animated menu border
		rectangleRGBA(screen, ((screenw >> 1) - i), menu_y, ((screenw >> 1) + i), menu_y+menu_height, 
				255, 255, 255, 255);		
		dstrect.x = (screenw >> 1) - i + 1;
		dstrect.y = menu_y + 1;
		dstrect.w = ((screenw >> 1) + i) - ((screenw >> 1) - i) - 2;
		dstrect.h = menu_height-2;
		SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 50, 50, 50));
		SDL_Flip(screen);
//		SDL_Delay(10);
	}


	while (!quit) // defaults menu loop
	{
		//blit background
		SDL_BlitSurface(background, NULL, screen, NULL);

		//blit menu border & BG
		rectangleRGBA(screen, ((screenw >> 1) - (menu_width>>1)), menu_y, (screenw >> 1) + (menu_width>>1), menu_y+menu_height, 
				255, 255, 255, 255);		
		dstrect.x = (screenw >> 1) - (menu_width>>1) + 1;
		dstrect.y = menu_y+1;
		dstrect.w = menu_width-2;
		dstrect.h = menu_height-2;
		SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 50, 50, 50));

		SFont_Write(screen, font_wh, line_x, line_y[CONTROLS_MENU_FIRE_LINE], "Fire:");		
		SFont_Write(screen, font_wh, line_x_col2, line_y[CONTROLS_MENU_FIRE_LINE], button_strs[tmp_button_map[0]]);		
		SFont_Write(screen, font_wh, line_x, line_y[CONTROLS_MENU_FIRE2_LINE], "Fire (alt):");		
		SFont_Write(screen, font_wh, line_x_col2, line_y[CONTROLS_MENU_FIRE2_LINE], button_strs[tmp_button_map[1]]);		
		SFont_Write(screen, font_wh, line_x, line_y[CONTROLS_MENU_SPECIAL_LINE], "Special:");		
		SFont_Write(screen, font_wh, line_x_col2, line_y[CONTROLS_MENU_SPECIAL_LINE], button_strs[tmp_button_map[2]]);		
		SFont_Write(screen, font_wh, line_x, line_y[CONTROLS_MENU_SPECIAL2_LINE], "Special (alt):");		
		SFont_Write(screen, font_wh, line_x_col2, line_y[CONTROLS_MENU_SPECIAL2_LINE], button_strs[tmp_button_map[3]]);		
		SFont_Write(screen, font_wh, line_x, line_y[CONTROLS_MENU_PAUSE_LINE], "Pause:");		
		SFont_Write(screen, font_wh, line_x_col2, line_y[CONTROLS_MENU_PAUSE_LINE], button_strs[tmp_button_map[4]]);		
		SFont_Write(screen, font_wh, line_x, line_y[CONTROLS_MENU_EXIT_LINE], "Quit:");		
		SFont_Write(screen, font_wh, line_x_col2, line_y[CONTROLS_MENU_EXIT_LINE], button_strs[tmp_button_map[5]]);		
		SFont_Write(screen, font_wh, line_x, line_y[CONTROLS_MENU_VOLDOWN_LINE], "Volume down:");		
		SFont_Write(screen, font_wh, line_x_col2, line_y[CONTROLS_MENU_VOLDOWN_LINE], button_strs[tmp_button_map[6]]);		
		SFont_Write(screen, font_wh, line_x, line_y[CONTROLS_MENU_VOLUP_LINE], "Volume up:");		
		SFont_Write(screen, font_wh, line_x_col2, line_y[CONTROLS_MENU_VOLUP_LINE], button_strs[tmp_button_map[7]]);		
		SFont_Write(screen, font_wh, line_x, line_y[CONTROLS_MENU_ACCEPT_LINE], "Accept changes");		
		SFont_Write(screen, font_wh, line_x, line_y[CONTROLS_MENU_CANCEL_LINE], "Cancel changes");		

		// draw cursor
		SFont_Write(screen, font_wh, line_x - 4, line_y[current_line], ">");
		SFont_Write(screen, font_wh, line_x - 6, line_y[current_line] - 1, "-");

		// update screen and wait a bit
		SDL_Flip(screen);
		SDL_Delay(MENU_DELAY);

		if ((SDL_GetTicks() - last_input_time) > INPUT_DELAY)
		{
			SDL_JoystickUpdate();

			if (SDL_JoystickGetButton(joy, GP2X_BUTTON_B))
			{
				switch (current_line)
				{
					case CONTROLS_MENU_ACCEPT_LINE:
						new_settings.buttons[0] = usable_button_to_gp2x_map[tmp_button_map[0]];
						printf("wrote %d to buttons[0]\n", new_settings.buttons[0]);
						new_settings.buttons[1] = usable_button_to_gp2x_map[tmp_button_map[1]];
						new_settings.buttons[2] = usable_button_to_gp2x_map[tmp_button_map[2]];
						new_settings.buttons[3] = usable_button_to_gp2x_map[tmp_button_map[3]];
						new_settings.buttons[4] = usable_button_to_gp2x_map[tmp_button_map[4]];
						new_settings.buttons[5] = usable_button_to_gp2x_map[tmp_button_map[5]];
						new_settings.buttons[6] = usable_button_to_gp2x_map[tmp_button_map[6]];
						new_settings.buttons[7] = usable_button_to_gp2x_map[tmp_button_map[7]];
						quit = 1;
						break;
					case CONTROLS_MENU_CANCEL_LINE:
						quit = 1;
						break;
					default:
						break;
				}
			} else if (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
			{
				switch (current_line)
				{
					case CONTROLS_MENU_FIRE_LINE:
						tmp_button_map[0]--;
						if (tmp_button_map[0] < 0) tmp_button_map[0] = NUM_USABLE_BUTTONS;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case CONTROLS_MENU_FIRE2_LINE:
						tmp_button_map[1]--;
						if (tmp_button_map[1] < 0) tmp_button_map[1] = NUM_USABLE_BUTTONS;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case CONTROLS_MENU_SPECIAL_LINE:
						tmp_button_map[2]--;
						if (tmp_button_map[2] < 0) tmp_button_map[2] = NUM_USABLE_BUTTONS;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case CONTROLS_MENU_SPECIAL2_LINE:
						tmp_button_map[3]--;
						if (tmp_button_map[3] < 0) tmp_button_map[3] = NUM_USABLE_BUTTONS;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case CONTROLS_MENU_PAUSE_LINE:
						tmp_button_map[4]--;
						if (tmp_button_map[4] < 0) tmp_button_map[4] = NUM_USABLE_BUTTONS;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case CONTROLS_MENU_EXIT_LINE:
						tmp_button_map[5]--;
						if (tmp_button_map[5] < 0) tmp_button_map[5] = NUM_USABLE_BUTTONS;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case CONTROLS_MENU_VOLDOWN_LINE:
						tmp_button_map[6]--;
						if (tmp_button_map[6] < 0) tmp_button_map[6] = NUM_USABLE_BUTTONS;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case CONTROLS_MENU_VOLUP_LINE:
						tmp_button_map[7]--;
						if (tmp_button_map[7] < 0) tmp_button_map[7] = NUM_USABLE_BUTTONS;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					default:
						break;
				}
			} else if (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
			{
				switch (current_line)
				{
					case CONTROLS_MENU_FIRE_LINE:
						tmp_button_map[0]++;
						if (tmp_button_map[0] > NUM_USABLE_BUTTONS) tmp_button_map[0] = 0;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case CONTROLS_MENU_FIRE2_LINE:
						tmp_button_map[1]++;
						if (tmp_button_map[1] > NUM_USABLE_BUTTONS) tmp_button_map[1] = 0;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case CONTROLS_MENU_SPECIAL_LINE:
						tmp_button_map[2]++;
						if (tmp_button_map[2] > NUM_USABLE_BUTTONS) tmp_button_map[2] = 0;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case CONTROLS_MENU_SPECIAL2_LINE:
						tmp_button_map[3]++;
						if (tmp_button_map[3] > NUM_USABLE_BUTTONS) tmp_button_map[3] = 0;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case CONTROLS_MENU_PAUSE_LINE:
						tmp_button_map[4]++;
						if (tmp_button_map[4] > NUM_USABLE_BUTTONS) tmp_button_map[4] = 0;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case CONTROLS_MENU_EXIT_LINE:
						tmp_button_map[5]++;
						if (tmp_button_map[5] > NUM_USABLE_BUTTONS) tmp_button_map[5] = 0;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case CONTROLS_MENU_VOLDOWN_LINE:
						tmp_button_map[6]++;
						if (tmp_button_map[6] > NUM_USABLE_BUTTONS) tmp_button_map[6] = 0;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case CONTROLS_MENU_VOLUP_LINE:
						tmp_button_map[7]++;
						if (tmp_button_map[7] > NUM_USABLE_BUTTONS) tmp_button_map[7] = 0;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					default:
						break;
				}
			} else if (SDL_JoystickGetButton(joy, GP2X_BUTTON_UP))
			{
				current_line--;
				if (current_line < 0)
				{
					current_line = 0;
				}
			} else if (SDL_JoystickGetButton(joy, GP2X_BUTTON_DOWN))
			{
				current_line++;
				if (current_line >= CONTROLS_MENU_LINES)
				{
					current_line = CONTROLS_MENU_LINES-1;
				}
			}
						
			last_input_time = SDL_GetTicks();
		}
	}
}

void rcontrols_menu(void)
{
#define RCONTROLS_MENU_LINES					10
#define RCONTROLS_MENU_FIRE_LINE				0
#define RCONTROLS_MENU_FIRE2_LINE				1
#define RCONTROLS_MENU_SPECIAL_LINE			2
#define RCONTROLS_MENU_SPECIAL2_LINE			3
#define RCONTROLS_MENU_PAUSE_LINE				4
#define RCONTROLS_MENU_EXIT_LINE				5
#define RCONTROLS_MENU_VOLDOWN_LINE			6
#define RCONTROLS_MENU_VOLUP_LINE				7
#define RCONTROLS_MENU_ACCEPT_LINE			8
#define RCONTROLS_MENU_CANCEL_LINE			9

	SDL_Rect	srcrect, dstrect;
	const int menu_y = 80;			// top of menu y coord
	const int line_y[] = { menu_y+10, menu_y+20, menu_y+30, menu_y+40, menu_y+50,
  									menu_y+60, menu_y+70, menu_y+80, menu_y+100, menu_y+110 };
	const int line_x = 100;			// x offset of menu lines' text
	const int line_x_col2 = 190;
	const int menu_height = 133;
	const int menu_width = 200;
	
	int quit = 0;
	int current_line = 0;
	int last_input_time = 0;	// What was the ticks value when we last took a button input?
	char tmp_str[50];

	const char* button_strs[] = {
		"A", "B", "X", "Y", "L Trigger", "R Trigger", 
		"Menu", "Select", "Vol -", "Vol+", "DISABLED" };

	const int usable_button_to_gp2x_map[NUM_USABLE_BUTTONS+1] = { GP2X_BUTTON_A, GP2X_BUTTON_B, GP2X_BUTTON_X, GP2X_BUTTON_Y,
			GP2X_BUTTON_L, GP2X_BUTTON_R, GP2X_BUTTON_START, GP2X_BUTTON_SELECT, GP2X_BUTTON_VOLDOWN, GP2X_BUTTON_VOLUP,
			NUM_BUTTONS};
	const int gp2x_to_usable_button_map[NUM_BUTTONS+1] = { NUM_USABLE_BUTTONS, 
																				NUM_USABLE_BUTTONS, 
																				NUM_USABLE_BUTTONS,
																				NUM_USABLE_BUTTONS, 
																				NUM_USABLE_BUTTONS,
																				NUM_USABLE_BUTTONS, 
																				NUM_USABLE_BUTTONS, 
																				NUM_USABLE_BUTTONS, 
																				6, 7, 5, 4, 0, 1, 3, 2, 9, 8, NUM_USABLE_BUTTONS };

	int tmp_button_map[NUM_DEFS] = { gp2x_to_usable_button_map[new_settings.rbuttons[0]], 
												gp2x_to_usable_button_map[new_settings.rbuttons[1]], 
												gp2x_to_usable_button_map[new_settings.rbuttons[2]], 
												gp2x_to_usable_button_map[new_settings.rbuttons[3]], 
												gp2x_to_usable_button_map[new_settings.rbuttons[4]], 
												gp2x_to_usable_button_map[new_settings.rbuttons[5]], 
												gp2x_to_usable_button_map[new_settings.rbuttons[6]], 
												gp2x_to_usable_button_map[new_settings.rbuttons[7]] };

	int i;
	for (i = 0; i < (menu_width>>1); i+=4)
	{
		//blit background
		SDL_BlitSurface(background, NULL, screen, NULL);
		// animated menu border
		rectangleRGBA(screen, ((screenw >> 1) - i), menu_y, ((screenw >> 1) + i), menu_y+menu_height, 
				255, 255, 255, 255);		
		dstrect.x = (screenw >> 1) - i + 1;
		dstrect.y = menu_y + 1;
		dstrect.w = ((screenw >> 1) + i) - ((screenw >> 1) - i) - 2;
		dstrect.h = menu_height-2;
		SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 50, 50, 50));
		SDL_Flip(screen);
//		SDL_Delay(10);
	}


	while (!quit) // defaults menu loop
	{
		//blit background
		SDL_BlitSurface(background, NULL, screen, NULL);

		//blit menu border & BG
		rectangleRGBA(screen, ((screenw >> 1) - (menu_width>>1)), menu_y, (screenw >> 1) + (menu_width>>1), menu_y+menu_height, 
				255, 255, 255, 255);		
		dstrect.x = (screenw >> 1) - (menu_width>>1) + 1;
		dstrect.y = menu_y+1;
		dstrect.w = menu_width-2;
		dstrect.h = menu_height-2;
		SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 50, 50, 50));

		SFont_Write(screen, font_wh, line_x, line_y[RCONTROLS_MENU_FIRE_LINE], "Fire:");		
		SFont_Write(screen, font_wh, line_x_col2, line_y[RCONTROLS_MENU_FIRE_LINE], button_strs[tmp_button_map[0]]);		
		SFont_Write(screen, font_wh, line_x, line_y[RCONTROLS_MENU_FIRE2_LINE], "Fire (alt):");		
		SFont_Write(screen, font_wh, line_x_col2, line_y[RCONTROLS_MENU_FIRE2_LINE], button_strs[tmp_button_map[1]]);		
		SFont_Write(screen, font_wh, line_x, line_y[RCONTROLS_MENU_SPECIAL_LINE], "Special:");		
		SFont_Write(screen, font_wh, line_x_col2, line_y[RCONTROLS_MENU_SPECIAL_LINE], button_strs[tmp_button_map[2]]);		
		SFont_Write(screen, font_wh, line_x, line_y[RCONTROLS_MENU_SPECIAL2_LINE], "Special (alt):");		
		SFont_Write(screen, font_wh, line_x_col2, line_y[RCONTROLS_MENU_SPECIAL2_LINE], button_strs[tmp_button_map[3]]);		
		SFont_Write(screen, font_wh, line_x, line_y[RCONTROLS_MENU_PAUSE_LINE], "Pause:");		
		SFont_Write(screen, font_wh, line_x_col2, line_y[RCONTROLS_MENU_PAUSE_LINE], button_strs[tmp_button_map[4]]);		
		SFont_Write(screen, font_wh, line_x, line_y[RCONTROLS_MENU_EXIT_LINE], "Quit:");		
		SFont_Write(screen, font_wh, line_x_col2, line_y[RCONTROLS_MENU_EXIT_LINE], button_strs[tmp_button_map[5]]);		
		SFont_Write(screen, font_wh, line_x, line_y[RCONTROLS_MENU_VOLDOWN_LINE], "Volume down:");		
		SFont_Write(screen, font_wh, line_x_col2, line_y[RCONTROLS_MENU_VOLDOWN_LINE], button_strs[tmp_button_map[6]]);		
		SFont_Write(screen, font_wh, line_x, line_y[RCONTROLS_MENU_VOLUP_LINE], "Volume up:");		
		SFont_Write(screen, font_wh, line_x_col2, line_y[RCONTROLS_MENU_VOLUP_LINE], button_strs[tmp_button_map[7]]);		
		SFont_Write(screen, font_wh, line_x, line_y[RCONTROLS_MENU_ACCEPT_LINE], "Accept changes");		
		SFont_Write(screen, font_wh, line_x, line_y[RCONTROLS_MENU_CANCEL_LINE], "Cancel changes");		

		// draw cursor
		SFont_Write(screen, font_wh, line_x - 4, line_y[current_line], ">");
		SFont_Write(screen, font_wh, line_x - 6, line_y[current_line] - 1, "-");

		// update screen and wait a bit
		SDL_Flip(screen);
		SDL_Delay(MENU_DELAY);

		if ((SDL_GetTicks() - last_input_time) > INPUT_DELAY)
		{
			SDL_JoystickUpdate();

			if (SDL_JoystickGetButton(joy, GP2X_BUTTON_B))
			{
				switch (current_line)
				{
					case RCONTROLS_MENU_ACCEPT_LINE:
						new_settings.rbuttons[0] = usable_button_to_gp2x_map[tmp_button_map[0]];
						new_settings.rbuttons[1] = usable_button_to_gp2x_map[tmp_button_map[1]];
						new_settings.rbuttons[2] = usable_button_to_gp2x_map[tmp_button_map[2]];
						new_settings.rbuttons[3] = usable_button_to_gp2x_map[tmp_button_map[3]];
						new_settings.rbuttons[4] = usable_button_to_gp2x_map[tmp_button_map[4]];
						new_settings.rbuttons[5] = usable_button_to_gp2x_map[tmp_button_map[5]];
						new_settings.rbuttons[6] = usable_button_to_gp2x_map[tmp_button_map[6]];
						new_settings.rbuttons[7] = usable_button_to_gp2x_map[tmp_button_map[7]];
						quit = 1;
						break;
					case RCONTROLS_MENU_CANCEL_LINE:
						quit = 1;
						break;
					default:
						break;
				}
			} else if (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
			{
				switch (current_line)
				{
					case RCONTROLS_MENU_FIRE_LINE:
						tmp_button_map[0]--;
						if (tmp_button_map[0] < 0) tmp_button_map[0] = NUM_USABLE_BUTTONS;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case RCONTROLS_MENU_FIRE2_LINE:
						tmp_button_map[1]--;
						if (tmp_button_map[1] < 0) tmp_button_map[1] = NUM_USABLE_BUTTONS;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case RCONTROLS_MENU_SPECIAL_LINE:
						tmp_button_map[2]--;
						if (tmp_button_map[2] < 0) tmp_button_map[2] = NUM_USABLE_BUTTONS;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case RCONTROLS_MENU_SPECIAL2_LINE:
						tmp_button_map[3]--;
						if (tmp_button_map[3] < 0) tmp_button_map[3] = NUM_USABLE_BUTTONS;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case RCONTROLS_MENU_PAUSE_LINE:
						tmp_button_map[4]--;
						if (tmp_button_map[4] < 0) tmp_button_map[4] = NUM_USABLE_BUTTONS;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case RCONTROLS_MENU_EXIT_LINE:
						tmp_button_map[5]--;
						if (tmp_button_map[5] < 0) tmp_button_map[5] = NUM_USABLE_BUTTONS;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case RCONTROLS_MENU_VOLDOWN_LINE:
						tmp_button_map[6]--;
						if (tmp_button_map[6] < 0) tmp_button_map[6] = NUM_USABLE_BUTTONS;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case RCONTROLS_MENU_VOLUP_LINE:
						tmp_button_map[7]--;
						if (tmp_button_map[7] < 0) tmp_button_map[7] = NUM_USABLE_BUTTONS;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					default:
						break;
				}
			} else if (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
			{
				switch (current_line)
				{
					case RCONTROLS_MENU_FIRE_LINE:
						tmp_button_map[0]++;
						if (tmp_button_map[0] > NUM_USABLE_BUTTONS) tmp_button_map[0] = 0;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case RCONTROLS_MENU_FIRE2_LINE:
						tmp_button_map[1]++;
						if (tmp_button_map[1] > NUM_USABLE_BUTTONS) tmp_button_map[1] = 0;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case RCONTROLS_MENU_SPECIAL_LINE:
						tmp_button_map[2]++;
						if (tmp_button_map[2] > NUM_USABLE_BUTTONS) tmp_button_map[2] = 0;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case RCONTROLS_MENU_SPECIAL2_LINE:
						tmp_button_map[3]++;
						if (tmp_button_map[3] > NUM_USABLE_BUTTONS) tmp_button_map[3] = 0;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case RCONTROLS_MENU_PAUSE_LINE:
						tmp_button_map[4]++;
						if (tmp_button_map[4] > NUM_USABLE_BUTTONS) tmp_button_map[4] = 0;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case RCONTROLS_MENU_EXIT_LINE:
						tmp_button_map[5]++;
						if (tmp_button_map[5] > NUM_USABLE_BUTTONS) tmp_button_map[5] = 0;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case RCONTROLS_MENU_VOLDOWN_LINE:
						tmp_button_map[6]++;
						if (tmp_button_map[6] > NUM_USABLE_BUTTONS) tmp_button_map[6] = 0;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case RCONTROLS_MENU_VOLUP_LINE:
						tmp_button_map[7]++;
						if (tmp_button_map[7] > NUM_USABLE_BUTTONS) tmp_button_map[7] = 0;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					default:
						break;
				}
			} else if (SDL_JoystickGetButton(joy, GP2X_BUTTON_UP))
			{
				current_line--;
				if (current_line < 0)
				{
					current_line = 0;
				}
			} else if (SDL_JoystickGetButton(joy, GP2X_BUTTON_DOWN))
			{
				current_line++;
				if (current_line >= RCONTROLS_MENU_LINES)
				{
					current_line = RCONTROLS_MENU_LINES-1;
				}
			}
						
			last_input_time = SDL_GetTicks();
		}
	}
}

void defaults_menu(void)
{
#define DEFAULTS_MENU_LINES				2
#define DEFAULTS_MENU_NO_LINE				0
#define DEFAULTS_MENU_YES_LINE			1

	SDL_Rect	srcrect, dstrect;
	const int menu_y = 80;			// top of menu y coord
	const int line_y[] = { menu_y+60, menu_y+70 };
	const int line_x = 110;			// x offset of menu lines' text
	const int menu_height = 133;
	const int menu_width = 200;
	
	int quit = 0;
	int current_line = 0;
	int last_input_time = 0;	// What was the ticks value when we last took a button input?

	int i;
	for (i = 0; i < (menu_width>>1); i+=4)
	{
		//blit background
		SDL_BlitSurface(background, NULL, screen, NULL);
		// animated menu border
		rectangleRGBA(screen, ((screenw >> 1) - i), menu_y, ((screenw >> 1) + i), menu_y+menu_height, 
				255, 255, 255, 255);		
		dstrect.x = (screenw >> 1) - i + 1;
		dstrect.y = menu_y + 1;
		dstrect.w = ((screenw >> 1) + i) - ((screenw >> 1) - i) - 2;
		dstrect.h = menu_height-2;
		SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 50, 50, 50));
		SDL_Flip(screen);
//		SDL_Delay(10);
	}


	while (!quit) // defaults menu loop
	{
		//blit background
		SDL_BlitSurface(background, NULL, screen, NULL);

		//blit menu border & BG
		rectangleRGBA(screen, ((screenw >> 1) - (menu_width>>1)), menu_y, (screenw >> 1) + (menu_width>>1), menu_y+menu_height, 
				255, 255, 255, 255);		
		dstrect.x = (screenw >> 1) - (menu_width>>1) + 1;
		dstrect.y = menu_y+1;
		dstrect.w = menu_width-2;
		dstrect.h = menu_height-2;
		SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 50, 50, 50));

		// draw menu text
		SFont_Write(screen, font_wh, line_x, line_y[0]-10, "Load default settings?");		

		SFont_Write(screen, font_wh, line_x, line_y[DEFAULTS_MENU_NO_LINE], "No");		
		SFont_Write(screen, font_wh, line_x, line_y[DEFAULTS_MENU_YES_LINE], "Yes");		

		// draw cursor
		SFont_Write(screen, font_wh, line_x - 4, line_y[current_line], ">");
		SFont_Write(screen, font_wh, line_x - 6, line_y[current_line] - 1, "-");

		// update screen and wait a bit
		SDL_Flip(screen);
		SDL_Delay(MENU_DELAY);

		if ((SDL_GetTicks() - last_input_time) > INPUT_DELAY)
		{
			SDL_JoystickUpdate();

			if (SDL_JoystickGetButton(joy, GP2X_BUTTON_B))
			{
				switch (current_line)
				{
					case DEFAULTS_MENU_YES_LINE:
						new_settings = default_settings;
						quit = 1;
						break;
					case DEFAULTS_MENU_NO_LINE:
						quit = 1;
						break;
					default:
						break;
				}
			}
			if (SDL_JoystickGetButton(joy, GP2X_BUTTON_UP))
			{
				current_line--;
				if (current_line < 0)
				{
					current_line = 0;
				}
			}
			if (SDL_JoystickGetButton(joy, GP2X_BUTTON_DOWN))
			{
				current_line++;
				if (current_line >= DEFAULTS_MENU_LINES)
				{
					current_line = DEFAULTS_MENU_LINES-1;
				}
			}
						
			last_input_time = SDL_GetTicks();
		}
	}
}
	

int menu(void)
{

#define MENU_LINES		10
#define ROTATED_LINE		0
#define LASER_LINE		1
#define RAM_LINE			2
#define CPU_LINE			3
#define MUSIC_LINE		4
#define RCONTROLS_LINE	5
#define CONTROLS_LINE	6
#define DEFAULTS_LINE	7
#define ABANDON_LINE		8
#define SAVE_LINE			9

	int quit = 0;
	int wants_to_save = 0;

	int last_input_time = 0;	// What was the ticks value when we last took a button input?

	SDL_Rect	srcrect, dstrect;

//	const int line_y[4] = { 13, 23, 33, 43 };
	const int menu_y = 80;			// top of menu y coord
	const int line_y[] = { menu_y+10, menu_y+20, menu_y+30, menu_y+40, menu_y+50, menu_y+60, 
		menu_y+70, menu_y+90, menu_y+100, menu_y+110 };
	const int line_x = 90;			// x offset of menu lines' text
	const int line_col1_x = line_x + 90;
//	const int line_col2_x = line_x + 150;
	const int menu_height = 133;
	const int menu_width = 200;

	int current_line = 0;			// What line of the menu to start on?
	char *tmp_str;
	char tmp_str2[50];
	int i;
	for (i = 0; i < (menu_width>>1); i+=4)
	{
		//blit background
		SDL_BlitSurface(background, NULL, screen, NULL);
		// animated menu border
		rectangleRGBA(screen, ((screenw >> 1) - i), menu_y, ((screenw >> 1) + i), menu_y+menu_height, 
				255, 255, 255, 255);		
		dstrect.x = (screenw >> 1) - i + 1;
		dstrect.y = menu_y + 1;
		dstrect.w = ((screenw >> 1) + i) - ((screenw >> 1) - i) - 2;
		dstrect.h = menu_height-2;
		SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 50, 50, 50));
		SDL_Flip(screen);
//		SDL_Delay(10);
	}

	while (!quit) // menu loop
	{

		//blit background
		SDL_BlitSurface(background, NULL, screen, NULL);

		//blit menu border & BG
		rectangleRGBA(screen, ((screenw >> 1) - (menu_width>>1)), menu_y, (screenw >> 1) + (menu_width>>1), menu_y+menu_height, 
				255, 255, 255, 255);		
		dstrect.x = (screenw >> 1) - (menu_width>>1) + 1;
		dstrect.y = menu_y+1;
		dstrect.w = menu_width-2;
		dstrect.h = menu_height-2;
		SDL_FillRect(screen, &dstrect, SDL_MapRGB(screen->format, 50, 50, 50));

		// draw menu text
		SFont_Write(screen, font_wh, line_x, line_y[ROTATED_LINE], "Screen:");		
		if (new_settings.rotated)
//			sprintf(tmp_str, "%d", current_timings.timing);
			tmp_str = "Rotated";
		else
			tmp_str = "Normal";
		SFont_Write(screen, font_wh, line_col1_x, line_y[ROTATED_LINE], tmp_str);		
//		SFont_Write(screen, font_wh, line_col2_x, line_y[ROTATED_LINE], "(default: rotated)");		

		SFont_Write(screen, font_wh, line_x, line_y[LASER_LINE], "Laser:");		
		if (new_settings.laser_on_by_default)
			tmp_str = "On by default";
		else
			tmp_str = "Off by default";
		SFont_Write(screen, font_wh, line_col1_x, line_y[LASER_LINE], tmp_str);		

		SFont_Write(screen, font_wh, line_x, line_y[RAM_LINE], "RAM timings:");		
		if (new_settings.fast_ram)
			tmp_str = "Fast";
		else
			tmp_str = "Untouched";
		SFont_Write(screen, font_wh, line_col1_x, line_y[RAM_LINE], tmp_str);		

		SFont_Write(screen, font_wh, line_x, line_y[CPU_LINE], "CPU Frequency:");		
		if (new_settings.cpu_freq == 0) {
			tmp_str = "Untouched";
			SFont_Write(screen, font_wh, line_col1_x, line_y[CPU_LINE], tmp_str);		
		} else {
			sprintf(tmp_str2, "%d", new_settings.cpu_freq);
			SFont_Write(screen, font_wh, line_col1_x, line_y[CPU_LINE], tmp_str2);		
		}

		SFont_Write(screen, font_wh, line_x, line_y[MUSIC_LINE], "Music:");		
		if (new_settings.music)
			tmp_str = "Enabled";
		else
			tmp_str = "Disabled";
		SFont_Write(screen, font_wh, line_col1_x, line_y[MUSIC_LINE], tmp_str);		

		SFont_Write(screen, font_wh, line_x, line_y[RCONTROLS_LINE], "Redefine rotated controls");		
		SFont_Write(screen, font_wh, line_x, line_y[CONTROLS_LINE], "Redefine non-rotated controls");		
		SFont_Write(screen, font_wh, line_x, line_y[DEFAULTS_LINE], "Load default settings");		
		SFont_Write(screen, font_wh, line_x, line_y[ABANDON_LINE], "Abandon changes and exit");		
		SFont_Write(screen, font_wh, line_x, line_y[SAVE_LINE], "Save and exit");		

		// draw cursor
		SFont_Write(screen, font_wh, line_x - 4, line_y[current_line], ">");
		SFont_Write(screen, font_wh, line_x - 6, line_y[current_line] - 1, "-");

		// update screen and wait a bit
		SDL_Flip(screen);
		SDL_Delay(MENU_DELAY);

		if ((SDL_GetTicks() - last_input_time) > INPUT_DELAY)
		{
			SDL_JoystickUpdate();

			if (SDL_JoystickGetButton(joy, GP2X_BUTTON_B))
			{
				switch (current_line)
				{
					case ROTATED_LINE:
						new_settings.rotated = !new_settings.rotated;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_B))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case LASER_LINE:
						new_settings.laser_on_by_default = !new_settings.laser_on_by_default;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_B))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case RAM_LINE:
						new_settings.fast_ram = !new_settings.fast_ram;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_B))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case CPU_LINE:
						if (new_settings.cpu_freq == 0) {
							new_settings.cpu_freq = MIN_CPU_FREQ;
						} else {
							new_settings.cpu_freq += CPU_FREQ_STEP;
							if (new_settings.cpu_freq > MAX_CPU_FREQ)
							{
								new_settings.cpu_freq = 0;
							}
						}
						break;
					case MUSIC_LINE:
						new_settings.music = !new_settings.music;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_B))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case RCONTROLS_LINE:
						rcontrols_menu();
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_B))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case CONTROLS_LINE:
						controls_menu();
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_B))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case DEFAULTS_LINE:
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_B))
						{
							SDL_JoystickUpdate();
							// do nothing
						}

						defaults_menu();

						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_B))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case ABANDON_LINE:
						quit = 1;
						break;
					case SAVE_LINE:
						quit = 1;
						wants_to_save = 1;
						break;
					default:
						break;
				}
			}
			if (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
			{
				switch (current_line)
				{
					case ROTATED_LINE:
						new_settings.rotated = !new_settings.rotated;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case LASER_LINE:
						new_settings.laser_on_by_default = !new_settings.laser_on_by_default;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case RAM_LINE:
						new_settings.fast_ram = !new_settings.fast_ram;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case CPU_LINE:
						if (new_settings.cpu_freq == 0) {
							new_settings.cpu_freq = MIN_CPU_FREQ;
						} else {
							new_settings.cpu_freq += CPU_FREQ_STEP;
							if (new_settings.cpu_freq > MAX_CPU_FREQ)
							{
								new_settings.cpu_freq = 0;
							}
						}
						break;
					case MUSIC_LINE:
						new_settings.music = !new_settings.music;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_RIGHT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					default:
						break;
				}
			}
			if (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
			{
				switch (current_line)
				{
					case ROTATED_LINE:
						new_settings.rotated = !new_settings.rotated;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case LASER_LINE:
						new_settings.laser_on_by_default = !new_settings.laser_on_by_default;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case RAM_LINE:
						new_settings.fast_ram = !new_settings.fast_ram;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					case CPU_LINE:
						if (new_settings.cpu_freq == 0) {
							new_settings.cpu_freq = MAX_CPU_FREQ;
						} else {
							new_settings.cpu_freq -= CPU_FREQ_STEP;
							if (new_settings.cpu_freq < MIN_CPU_FREQ)
							{
								new_settings.cpu_freq = 0;
							}
						}
						break;
					case MUSIC_LINE:
						new_settings.music = !new_settings.music;
						while (SDL_JoystickGetButton(joy, GP2X_BUTTON_LEFT))
						{
							SDL_JoystickUpdate();
							// do nothing
						}
						break;
					default:
						break;
				}
			}
			if (SDL_JoystickGetButton(joy, GP2X_BUTTON_UP))
			{
				current_line--;
				if (current_line < 0)
				{
					current_line = MENU_LINES - 1;
				}
			}
			if (SDL_JoystickGetButton(joy, GP2X_BUTTON_DOWN))
			{
				current_line++;
				if (current_line >= MENU_LINES)
				{
					current_line = 0;
				}
			}
						
			last_input_time = SDL_GetTicks();
		}
	}

	return wants_to_save;
}

int initialize(void)
{
	if (!(screen = SDL_SetVideoMode(screenw, screenh, screenbpp, screenflags))) 
	{
		fprintf(stderr, "cannot open %dx%d display: %s\n",
			     screenw, screenh, SDL_GetError());
		return 0;
    } 
	
	SDL_ShowCursor(SDL_DISABLE);
	
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
		fprintf(stderr, "Cannot initialize SDL system: %s\n", SDL_GetError());
		return 0;
	}
	 
	joy = SDL_JoystickOpen(0);
	if (joy == NULL)
	{
		fprintf(stderr, "Cannot initialize GP2X joystick: %s\n", SDL_GetError());
	}

	if (!load_graphics()) {
		fprintf(stderr, "Error loading sprites/fonts.\n");
		return 0;
	}

	return 1;
}

int main(int argc, char *argv[])
{
	// Initialize graphics, load sprits/fonts etc, mmap registers
	if (!initialize())
	{
		fprintf(stderr, "Aborting because of initialization errors.\n");
		return 1;
	}

	//  Return 0 on success, -1 on error.  Read settings into *timings structure.
	if (!read_saved_settings(settings_filename))
	{
		printf("Unable to read settings from %s, using defaults values.\n", settings_filename);
		new_settings = default_settings;
	}

	// RUN MENU LOOP
	if (menu())
	{
		// Returned 1, user wanted to save new values
		printf("User requested to save new settings.\n");
		if (write_new_settings(settings_filename))
		{
			printf("Successfully saved new settings.\n");
		}
		else
		{
			printf("Problem occurred writing new settings.\n");
		}

		sync();
	}
	else
	{
		printf("Abandoning changes per user request.\n");
	}

	shutdown();

	return 0;
}

