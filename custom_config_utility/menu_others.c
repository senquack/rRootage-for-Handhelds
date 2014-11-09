#include <stdlib.h>
#include "../portcfg.h"
#include "cfg.h"
#include "gcw.h"

char credits_str[] = "Game by Kenta Cho / GLES port & configuration utility by senquack      (v1.0)";

// Load defaults menu:
menu_entry defaults_menu_entries[] = {
   {  .is_special = 0,
      .text = "Cancel",
      .desc_text = "",
      .func_get_val_text = NULL,
      .func_select = exit_current_menu,
      .func_right = NULL,
      .func_left = NULL  },
   {  .is_special = 1,              // Blank separator line
      .text = "",
      .desc_text = "",
      .func_get_val_text = NULL,
      .func_select = NULL,
      .func_right = NULL,
      .func_left = NULL  },
   {  .is_special = 0,
      .text = "Load rotated-right screen defaults (recommended)",
      .desc_text = "Load defaults for a rotated-right screen orientation",
      .func_get_val_text = NULL,
      .func_select = load_defaults_for_rotated_right,
      .func_right = NULL,
      .func_left = NULL  },
   {  .is_special = 0,
      .text = "Load rotated-left screen defaults",
      .desc_text = "Load defaults for a rotated-left screen orientation",
      .func_get_val_text = NULL,
      .func_select = load_defaults_for_rotated_left,
      .func_right = NULL,
      .func_left = NULL  },
   {  .is_special = 0,
      .text = "Load horizontal screen defaults",
      .desc_text = "Load defaults for a non-rotated screen orientation",
      .func_get_val_text = NULL,
      .func_select = load_defaults_for_horizontal,
      .func_right = NULL,
      .func_left = NULL  },
};

menu defaults_menu = {
   .num_entries = 5,
   .cur_entry = 0,
   .entries = defaults_menu_entries
};



// Reset high scores menu:
menu_entry reset_menu_entries[] = {
   {  .is_special = 1,
      .text = "Confirm reset of high scores and level completions",
      .desc_text = "",
      .func_get_val_text = NULL,
      .func_select = NULL,
      .func_right = NULL,
      .func_left = NULL  },
   {  .is_special = 1,              // Blank separator line
      .text = "",
      .desc_text = "",
      .func_get_val_text = NULL,
      .func_select = NULL,
      .func_right = NULL,
      .func_left = NULL  },
   {  .is_special = 0,
      .text = "No",
      .desc_text = "",
      .func_get_val_text = NULL,
      .func_select = exit_current_menu,
      .func_right = NULL,
      .func_left = NULL  },
   {  .is_special = 0,
      .text = "Yes",
      .desc_text = "",
      .func_get_val_text = NULL,
      .func_select = reset_scores,
      .func_right = NULL,
      .func_left = NULL  },
};

menu reset_menu = {
   .num_entries = 4,
   .cur_entry = 0,
   .entries = reset_menu_entries
};



//Main menu:
menu_entry main_menu_entries[] = {
   {  .is_special = 0,
      .text = "Play rRootage",
      .desc_text = credits_str,
      .func_get_val_text = NULL,
      .func_select = run_rrootage,
      .func_right = NULL,
      .func_left = NULL  },
   {  .is_special = 1,                 // Blank separator line
      .text = "",
      .desc_text = credits_str,
      .desc_text = "",
      .func_get_val_text = NULL,
      .func_select = NULL,
      .func_right = NULL,
      .func_left = NULL  },
   {  .is_special = 0,
      .desc_text = credits_str,
      .text = "Configure game settings",
      .func_get_val_text = NULL,
      .func_select = run_game_settings_menu,
      .func_right = NULL,
      .func_left = NULL  },
   {  .is_special = 0,
      .desc_text = credits_str,
      .text = "Configure control settings",
      .func_get_val_text = NULL,
      .func_select = run_control_settings_menu,
      .func_right = NULL,
      .func_left = NULL  },
   {  .is_special = 0,
      .desc_text = credits_str,
      .text = "Load default settings",
      .func_get_val_text = NULL,
      .func_select = run_defaults_menu,
      .func_right = NULL,
      .func_left = NULL  },
   {  .is_special = 0,
      .desc_text = credits_str,
      .text = "Reset game scores",
      .func_get_val_text = NULL,
      .func_select = run_reset_menu,
      .func_right = NULL,
      .func_left = NULL  },
   {  .is_special = 0,
      .desc_text = credits_str,
      .text = "Exit",
      .func_get_val_text = NULL,
      .func_select = shutdown_and_exit,
      .func_right = NULL,
      .func_left = NULL  }
};

menu main_menu = {
   .num_entries = 7,
   .cur_entry = 0,
   .entries = main_menu_entries
};
      
