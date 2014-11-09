#ifndef CFG_H
#define CFG_H

// Control mapping for use inside the configuration utility:
enum { 
   MAP_UTIL_UP,
   MAP_UTIL_DOWN,
   MAP_UTIL_LEFT,
   MAP_UTIL_RIGHT,
   MAP_UTIL_SELECT,
   MAP_UTIL_STARTBUTTON,
   MAP_UTIL_NUMMAPPINGS
};
   
typedef struct {
   int is_special;               // Is this a menu title or a blank separator line?
   char *text;                   // Text to print for entry in meny. If NULL it is a separator entry
   char *desc_text;              // Additional text that can be printed below menu
   char* (*func_get_val_text)(); // Function to call to get entry's value text
   void (*func_select)();        // Function to call if button is pressed
   void (*func_right)();         // Function to call if right-directional is pressed
   void (*func_left)();          // Function to call if left-directional is pressed
} menu_entry;

typedef struct {
   int num_entries;              // Total number of entries in menu
   int cur_entry;                // Current entry line selected
   menu_entry *entries;          // Pointer to array of menu_entry
} menu;

// Menu data:
extern menu_entry control_settings_menu_entries[];
extern menu control_settings_menu;
extern menu_entry game_settings_menu_entries[];
extern menu game_settings_menu;
extern menu_entry defaults_menu_entries[];
extern menu defaults_menu;
extern menu_entry reset_menu_entries[];
extern menu reset_menu;
extern menu_entry main_menu_entries[];
extern menu main_menu;

int clamp(int x, int min, int max);
char *trim_string(char *buf);
int create_dir(const char *dir);
int load_graphics();
void unload_graphics();
void shutdown();
void shutdown_and_exit();
void display_first_run_message();
void run_menu(menu *m);
int initialize(void);
void run_rrootage();
void load_defaults_for_rotated_left();
void load_defaults_for_rotated_right();
void load_defaults_for_horizontal();
void settings_save_changes();
void settings_cancel_changes();
void run_game_settings_menu();
void run_control_settings_menu();
void run_defaults_menu();
void run_reset_menu();
void reset_scores();
void exit_current_menu();


#endif // CFG_H

