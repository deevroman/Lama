#include "globals.h"
#include "loader.h"

int running = 0;
char* ip = NULL;
bytefile* bytecode;

char* stack_pointer = NULL;
size_t stack_capacity = 0;
size_t frame_position = 0;
size_t stack_frames_counter = 0;