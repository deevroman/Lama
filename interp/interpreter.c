#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "../runtime/runtime.h"
#include "../runtime/gc.h"
#include "external.h"
#include "globals.h"
#include "callstack.h"
#include "opcodes.h"

#include "loader.h"

void loop_with_table()
{
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
                failure("Error executing opcode: 0x%02x file_position=%zu. Error: %s\n", x, file_position_from_ip(ip), res);
            }
        }
        else
        {
            failure("No op_handler for opcode: 0x%02x file_position=%zu\n", x, file_position_from_ip(ip));
        }
    }

}

void loop_with_switch()
{
    while (running)
    {
        unsigned char x = read_byte();
        DEBUG_LOG("[DEBUG] Instr #%d: 0x%02x\n", instr_count++, x);

        error_t res;

        switch (x)
        {
        case 0x01: res = op_add();
            break;
        case 0x02: res = op_sub();
            break;
        case 0x03: res = op_mul();
            break;
        case 0x04: res = op_div();
            break;
        case 0x05: res = op_mod();
            break;
        case 0x06: res = op_lt();
            break;
        case 0x07: res = op_le();
            break;
        case 0x08: res = op_gt();
            break;
        case 0x09: res = op_ge();
            break;
        case 0x0A: res = op_eq();
            break;
        case 0x0B: res = op_ne();
            break;
        case 0x0C: res = op_and();
            break;
        case 0x0D: res = op_or();
            break;

        case 0x10: res = op_const();
            break;
        case 0x11: res = op_string();
            break;
        case 0x12: res = op_sexp();
            break;
        case 0x13: res = op_sti();
            break;
        case 0x14: res = op_sta();
            break;
        case 0x15: res = op_jmp();
            break;
        case 0x16: res = op_end();
            break;
        case 0x17: res = op_ret();
            break;
        case 0x18: res = op_drop();
            break;
        case 0x19: res = op_dup();
            break;
        case 0x1A: res = op_swap();
            break;
        case 0x1B: res = op_elem();
            break;

        case 0x20: res = op_ld_g();
            break;
        case 0x21: res = op_ld_l();
            break;
        case 0x22: res = op_ld_a();
            break;
        case 0x23: res = op_ld_c();
            break;

        case 0x30: res = op_lda_g();
            break;
        case 0x31: res = op_lda_l();
            break;
        case 0x32: res = op_lda_a();
            break;
        case 0x33: res = op_lda_c();
            break;

        case 0x40: res = op_st_g();
            break;
        case 0x41: res = op_st_l();
            break;
        case 0x42: res = op_st_a();
            break;
        case 0x43: res = op_st_c();
            break;

        case 0x50: res = op_cjmpz();
            break;
        case 0x51: res = op_cjmpnz();
            break;
        case 0x52: res = op_begin();
            break;
        case 0x53: res = op_cbegin();
            break;
        case 0x54: res = op_closure();
            break;
        case 0x55: res = op_callc();
            break;
        case 0x56: res = op_call();
            break;
        case 0x57: res = op_tag();
            break;
        case 0x58: res = op_array();
            break;
        case 0x59: res = op_fail();
            break;
        case 0x5A: res = op_line();
            break;

        case 0x60: res = op_patt_str();
            break;
        case 0x61: res = op_patt_string_tag();
            break;
        case 0x62: res = op_patt_array_tag();
            break;
        case 0x63: res = op_patt_sexp_tag();
            break;
        case 0x64: res = op_patt_ref();
            break;
        case 0x65: res = op_patt_val();
            break;
        case 0x66: res = op_patt_fun();
            break;

        case 0x70: res = op_builtin_lread();
            break;
        case 0x71: res = op_builtin_lwrite();
            break;
        case 0x72: res = op_builtin_llength();
            break;
        case 0x73: res = op_builtin_lstring();
            break;
        case 0x74: res = op_builtin_barray();
            break;

        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:
            res = op_stop();
            break;

        default:
            failure("No op_handler for opcode: 0x%02x file_position=%zu\n",
                    x, file_position_from_ip(ip));
        }

        if (res != OK)
        {
            failure("Error executing opcode: 0x%02x file_position=%zu. Error: %s\n",
                    x, file_position_from_ip(ip), res);
        }
    }
}

void interpret_bytecode(bytefile* bf)
{
    DEBUG_LOG("[DEBUG] Calling __gc_init...\n");
    __gc_init();
    DEBUG_LOG("[DEBUG] __gc_init completed\n");

    bytecode = bf;
    ip = bf->code_ptr;

    DEBUG_LOG("[DEBUG] Interpreter starting...\n");
    DEBUG_LOG("[DEBUG] Code size: %zu bytes\n", bf->code_size);
    DEBUG_LOG("[DEBUG] Global area size: %d\n", bf->data->global_area_size);

    stack_capacity = 16 * 1024;
    stack_pointer = malloc(stack_capacity);
    if (!stack_pointer)
    {
        failure("Failed to allocate call stack\n");
    }

    frame_position = bf->data->global_area_size;
    __gc_stack_top = (size_t)stack_pointer;
    __gc_stack_bottom = __gc_stack_top + frame_position * sizeof(stack_value);

    stack_frames_counter = 0;
    if (push_frame(0, 2) != OK)
    {
        failure("Failed to push initial call frame");
    }

    int instr_count = 0;
    running = 1;

    loop_with_table();
    // loop_with_switch();

    free(stack_pointer);
}

#endif
