/// IFJ24
/// @brief STDIN handler

#ifndef FILE_H
#define FILE_H

#include <stdio.h>

void save_input_to_file();
FILE* open_temp_file();
void rewind_file(FILE *file);
void cleanup_temp_file();
void debug_temp_file();

#endif 