#ifndef MENU_GAME_SETTINGS_H
#define MENU_GAME_SETTINGS_H

//Generic boolean-to-text function:
char* settings_get_text_for_boolean(int setting);

//settings.rotated stuff:
void settings_dec_rotated();
void settings_inc_rotated();
char* settings_get_text_for_rotated();

//settings.no_wait stuff:
void settings_toggle_no_wait();
char* settings_get_text_for_no_wait();

//settings.laser_on_by_default stuff:
void settings_toggle_laser_on_by_default();
char* settings_get_text_for_laser_on_by_default();

//settings.music stuff:
void settings_toggle_music();
char* settings_get_text_for_music();

//settings.draw_outlines stuff:
void settings_inc_draw_outlines();
void settings_dec_draw_outlines();
char* settings_get_text_for_draw_outlines();

//settings.show_fps stuff:
void settings_toggle_show_fps();
char* settings_get_text_for_show_fps();

//settings.extra_lives stuff: 
void settings_inc_extra_lives();
void settings_dec_extra_lives();
char* settings_get_text_for_extra_lives();

//settings.extra_bombs stuff:
void settings_inc_extra_bombs();
void settings_dec_extra_bombs();
char* settings_get_text_for_extra_bombs();

#endif //MENU_GAME_SETTINGS_H
