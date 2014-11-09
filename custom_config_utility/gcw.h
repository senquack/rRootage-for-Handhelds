#ifndef GCW_H
#define GCW_H

#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   240
#define SCREEN_BPP      16
#define SCREEN_FLAGS    (SDL_DOUBLEBUF)

extern int control_state[MAP_UTIL_NUMMAPPINGS];
extern const char *base_portcfg_filename;
extern const char *base_rrbin_filename;
extern char *full_portcfg_filename;
extern char *full_rrbin_filename;

extern portcfg_settings default_rotated_right_settings;
extern portcfg_settings default_rotated_left_settings;
extern portcfg_settings default_horizontal_settings;
extern portcfg_settings settings;

// Each platform must provide a version of this to set the location of the port-specific filenames
int set_filenames();

//  Return 1 on success, 0 on error.  Write settings from structure.
int write_new_portcfg(const char *filename);

//  Return 1 on success, 0 on error.  Read settings into new_settings structure
int read_portcfg_settings(const char *filename);

//  Reset control state array to zeroes
void reset_control_state();

// Returns 0 if an SDL_QUIT event is received
int update_control_state();

#endif //GCW_H
