#ifndef GLOBALS_H
#define GLOBALS_H

static int running = 0;
static char* ip = NULL;
static bytefile* bytecode;

static char* stack_pointer = NULL;
static size_t stack_capacity = 0;
static size_t frame_position = 0;
static size_t stack_frames_counter = 0;

#endif //GLOBALS_H
