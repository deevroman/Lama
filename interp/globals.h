#ifndef GLOBALS_H
#define GLOBALS_H
#include "loader.h"

extern int running;
extern char* ip;
extern bytefile* bytecode;

extern char* stack_pointer;
extern size_t stack_capacity;
extern size_t frame_position;
extern size_t stack_frames_counter;
extern size_t current_operands_count;
extern size_t current_locals_count;
extern size_t current_args_count;

#endif //GLOBALS_H
