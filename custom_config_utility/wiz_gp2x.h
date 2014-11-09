#ifndef WIZ_GP2X_H
#define WIZ_GP2X_H

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
#define MAX_CPU_FREQ		(990)
#define MIN_CPU_FREQ		(540)
#define CPU_FREQ_STEP	(10)

#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   240
#define SCREEN_BPP      16
#define SCREEN_FLAGS    (SDL_FULLSCREEN)


// Each platform must provide a version of this to set the location of the port-specific configuration settings filename
int set_portcfg_filename();

//  Return 1 on success, 0 on error.  Write settings from structure.
int write_new_portcfg(const char *filename);

//  Return 1 on success, 0 on error.  Read settings into new_settings structure
int read_portcfg_settings(const char *filename);

// Returns 0 if an SDL_QUIT event is received
int update_control_state();

#endif //WIZ_GP2X_H
