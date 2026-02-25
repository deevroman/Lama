#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "../runtime/runtime.h"
#include "../runtime/gc.h"
#include "external.h"
#include "globals.h"
#include "callstack.h"
#include "opcodes.h"
#include <stdio.h>

void interpret_bytecode(bytefile* bf)
{
    DEBUG_LOG("[DEBUG] Calling __gc_init...\n");
    __gc_init();
    DEBUG_LOG("[DEBUG] __gc_init completed\n");

    bytecode = bf;
    ip = bf->code_ptr;

    DEBUG_LOG("[DEBUG] Interpreter starting...\n");
    DEBUG_LOG("[DEBUG] Code size: %zu bytes\n", bf->code_size);
    DEBUG_LOG("[DEBUG] Global area size: %d\n", bf->global_area_size);

    stack_capacity = 16 * 1024;
    stack_pointer = malloc(stack_capacity);
    if (!stack_pointer)
    {
        failure("Failed to allocate call stack\n");
    }

    frame_position = bf->global_area_size;
    __gc_stack_top = (size_t)stack_pointer;
    __gc_stack_bottom = __gc_stack_top + frame_position * sizeof(stack_value);

    stack_frames_counter = 0;
    if (push_frame(0, 2) != OK)
    {
        failure("Failed to push initial call frame");
    }

    int instr_count = 0;
    running = 1;
    while (running)
    {
        unsigned char x = read_byte();
        DEBUG_LOG("[DEBUG] Instr #%d: 0x%02x\n", instr_count++, x);

        const opcode_handler op_handler = opcodes[x];
        if (op_handler)
        {
            error_t res = op_handler();
            if (res != OK)
            {
                failure("Error executing opcode: 0x%02x %s\n", x, res);
            }
        }
        else
        {
            failure("No op_handler for opcode: 0x%02x\n", x);
        }
    }

    free(stack_pointer);
}

#endif
