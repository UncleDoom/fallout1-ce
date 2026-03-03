#pragma once


#include "plib/db/db.h"

namespace fallout {

int do_options();
int PauseWindow(bool is_world_map);
int init_options_menu();
int save_options(DB_FILE* stream);
int load_options(DB_FILE* stream);
void IncGamma();
void DecGamma();

} // namespace fallout
