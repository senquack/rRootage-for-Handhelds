#ifndef MENU_CONTROL_SETTINGS_H
#define MENU_CONTROL_SETTINGS_H

//generic overall get-text-for-mapping helper function for this stuff
char* settings_get_text_for_mapping(int mapping);

//settings.map.move stuff:
void settings_dec_move();
void settings_inc_move();
char* settings_get_text_for_move();

//settings.map.btn1 stuff:
void settings_dec_btn1();
void settings_inc_btn1();
char* settings_get_text_for_btn1();

//settings.map.btn1_alt stuff:
void settings_dec_btn1_alt();
void settings_inc_btn1_alt();
char* settings_get_text_for_btn1_alt();

//settings.map.btn2 stuff:
void settings_dec_btn2();
void settings_inc_btn2();
char* settings_get_text_for_btn2();

//settings.map.btn2_alt stuff:
void settings_dec_btn2_alt();
void settings_inc_btn2_alt();
char* settings_get_text_for_btn2_alt();

//settings.map.pause stuff:
void settings_dec_pause();
void settings_inc_pause();
char* settings_get_text_for_pause();

//settings.map.exit stuff:
void settings_dec_exit();
void settings_inc_exit();
char* settings_get_text_for_exit();

//settings.map.analog_deadzone stuff:
void settings_dec_analog_deadzone();
void settings_inc_analog_deadzone();
char* settings_get_text_for_analog_deadzone();

#endif // MENU_CONTROL_SETTINGS_H
