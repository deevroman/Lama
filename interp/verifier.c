#include "verifier.h"

#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interpreter.h"

typedef struct
{
    uint8_t op;
    uint32_t offset;
    uint32_t length;
    uint32_t arg[2];
    uint32_t closure_capture_count;
    uint32_t closure_captures_offset;
} decoded_instruction;

typedef struct
{
    uint32_t begin_offset;
    uint32_t args_count;
    uint32_t locals_count;
    uint32_t captures_count;
    uint8_t begin_opcode;
    uint32_t max_operand_depth;
    int32_t* depths;
} frame_context;

typedef struct
{
    uint32_t ctx_id;
    uint32_t offset;
} cfg_stack_item;

typedef struct
{
    bytefile* bf;
    decoded_instruction* instr;
    uint8_t* is_instr_start;

    frame_context* ctxs;
    size_t ctx_count;
    size_t ctx_capacity;

    cfg_stack_item* cfg_stack;
    size_t cfg_stack_size;
    size_t cfg_stack_capacity;
} verifier_state;

static verifier_state verifier;

_Noreturn static void verify_fail(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "Bytecode verification failed: ");
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    va_end(args);
    exit(1);
}

static uint8_t read_byte_checked(uint32_t offset, size_t* ip)
{
    if (*ip >= verifier.bf->code_size)
    {
        verify_fail("0x%08x: unexpected end of code while reading byte", offset);
    }
    const uint8_t value = (uint8_t)verifier.bf->code_ptr[*ip];
    *ip += 1;
    return value;
}

static uint32_t read_u32_checked(uint32_t offset, size_t* ip)
{
    if (*ip + sizeof(uint32_t) > verifier.bf->code_size)
    {
        verify_fail("0x%08x: unexpected end of code while reading uint32", offset);
    }
    const char* ptr = verifier.bf->code_ptr + *ip;
    uint32_t value;
    memcpy(&value, ptr, sizeof(value));
    *ip += sizeof(uint32_t);
    return value;
}

static uint32_t read_string_offset_checked(uint32_t offset, size_t* ip)
{
    const uint32_t value = read_u32_checked(offset, ip);
    if (value >= verifier.bf->data->stringtab_size)
    {
        verify_fail("0x%08x: invalid string table index %u (size=%u)", offset, value,
                    verifier.bf->data->stringtab_size);
    }
    return value;
}

static void decode_instruction(uint32_t offset, size_t* ip)
{
    decoded_instruction inst = {
        .offset = offset,
        .closure_captures_offset = UINT32_MAX
    };
    uint8_t op = (uint8_t)verifier.bf->code_ptr[offset];
    inst.op = op;
    if ((op & 0xF0) == 0xF0)
    {
        inst.length = 1;
        verifier.instr[offset] = inst;
        verifier.is_instr_start[offset] = 1;
        return;
    }

    if (!opcodes[op])
    {
        verify_fail("0x%08x: unknown opcode 0x%02x", offset, op);
    }

    switch (opcode_disasm_mode[op])
    {
    case DISASM_MODE_LITERAL:
        break;
    case DISASM_MODE_I32:
    case DISASM_MODE_HEX32:
        inst.arg[0] = read_u32_checked(offset, ip);
        break;
    case DISASM_MODE_I32_I32:
    case DISASM_MODE_I32_I32_NOSPACE:
        inst.arg[0] = read_u32_checked(offset, ip);
        inst.arg[1] = read_u32_checked(offset, ip);
        break;
    case DISASM_MODE_STRING:
        inst.arg[0] = read_string_offset_checked(offset, ip);
        break;
    case DISASM_MODE_STRING_I32:
        inst.arg[0] = read_string_offset_checked(offset, ip);
        inst.arg[1] = read_u32_checked(offset, ip);
        break;
    case DISASM_MODE_HEX32_I32:
        inst.arg[0] = read_u32_checked(offset, ip);
        inst.arg[1] = read_u32_checked(offset, ip);
        break;
    case DISASM_MODE_CLOSURE:
        inst.arg[0] = read_u32_checked(offset, ip);
        inst.arg[1] = read_u32_checked(offset, ip);
        inst.closure_capture_count = inst.arg[1];
        inst.closure_captures_offset = (uint32_t)*ip;

        if (inst.arg[1] > 0 && (size_t)inst.arg[1] > (SIZE_MAX - *ip) / (READ_BYTE + READ_INT))
        {
            verify_fail("0x%08x: closure capture list is too large", offset);
        }
        if (*ip + (size_t)inst.arg[1] * 5 > verifier.bf->code_size)
        {
            verify_fail("0x%08x: closure capture list overflows code section (captures=%u)", offset, inst.arg[1]);
        }

        for (uint32_t i = 0; i < inst.arg[1]; ++i)
        {
            uint8_t kind = read_byte_checked(offset, ip);
            read_u32_checked(offset, ip);
            if (kind > 3)
            {
                verify_fail("0x%08x: invalid closure capture kind %u", offset, kind);
            }
        }
        break;
    default:
        verify_fail("0x%08x: unsupported disasm mode %d", offset, opcode_disasm_mode[op]);
    }

    inst.length = *ip - offset;

    if (opcode_args_count[op] != READ_CUSTOM && (int)(inst.length - 1) != opcode_args_count[op])
    {
        verify_fail("0x%08x: opcode metadata mismatch for 0x%02x: expected %d args bytes, decoded %u",
                    offset, op, opcode_args_count[op], inst.length - 1);
    }

    verifier.instr[offset] = inst;
    verifier.is_instr_start[offset] = 1;
}

static void decode_all_instructions(void)
{
    size_t ip = 0;
    while (ip < verifier.bf->code_size)
    {
        const uint32_t offset = (uint32_t)ip;
        ip += 1;
        decode_instruction(offset, &ip);
    }
}

static void ensure_cfg_stack_capacity(void)
{
    if (verifier.cfg_stack_size < verifier.cfg_stack_capacity)
    {
        return;
    }
    size_t new_capacity = verifier.cfg_stack_capacity ? verifier.cfg_stack_capacity * 2 : 64;
    cfg_stack_item* new_stack = realloc(verifier.cfg_stack, new_capacity * sizeof(*new_stack));
    if (!new_stack)
    {
        verify_fail("out of memory while growing verifier cfg stack");
    }
    verifier.cfg_stack = new_stack;
    verifier.cfg_stack_capacity = new_capacity;
}

static void push_cfg_stack(uint32_t ctx_id, uint32_t offset)
{
    ensure_cfg_stack_capacity();
    verifier.cfg_stack[verifier.cfg_stack_size++] = (cfg_stack_item){.ctx_id = ctx_id, .offset = offset};
}

static uint32_t get_or_create_context(uint32_t begin_offset, uint8_t expected_begin_op, uint32_t captures_count)
{
    if (begin_offset >= verifier.bf->code_size)
    {
        verify_fail("0x%08x: function entry is outside code section", begin_offset);
    }
    if (!verifier.is_instr_start[begin_offset])
    {
        verify_fail("0x%08x: function entry is not instruction-aligned", begin_offset);
    }

    const decoded_instruction* begin_inst = &verifier.instr[begin_offset];
    const uint8_t begin_op = begin_inst->op;
    if (expected_begin_op != 0 && begin_op != expected_begin_op)
    {
        verify_fail("0x%08x: expected %s at function entry, got %s", begin_offset,
                    opcode_names[expected_begin_op], opcode_names[begin_op] ? opcode_names[begin_op] : "UNKNOWN");
    }
    if (expected_begin_op == 0 && begin_op != OPC_BEGIN && begin_op != OPC_CBEGIN)
    {
        verify_fail("0x%08x: public entry must point to BEGIN/CBEGIN, got %s", begin_offset,
                    opcode_names[begin_op] ? opcode_names[begin_op] : "UNKNOWN");
    }
    if (begin_op == OPC_BEGIN && captures_count != 0)
    {
        verify_fail("0x%08x: BEGIN entry cannot have captures (captures=%u)", begin_offset, captures_count);
    }

    const uint32_t args_count = begin_inst->arg[0];
    const uint32_t locals_count = begin_inst->arg[1];

    for (size_t i = 0; i < verifier.ctx_count; ++i)
    {
        frame_context* ctx = &verifier.ctxs[i];
        if (ctx->begin_offset == begin_offset && ctx->captures_count == captures_count)
        {
            if (ctx->begin_opcode != begin_op || ctx->args_count != args_count || ctx->locals_count != locals_count)
            {
                verify_fail("0x%08x: conflicting context metadata for function entry", begin_offset);
            }
            return (uint32_t)i;
        }
    }

    frame_context* new_ctx = &verifier.ctxs[verifier.ctx_count];
    *new_ctx = (frame_context){
        .begin_offset = begin_offset,
        .args_count = args_count,
        .locals_count = locals_count,
        .captures_count = captures_count,
        .begin_opcode = begin_op,
        .max_operand_depth = 0,
        .depths = malloc(verifier.bf->code_size * sizeof(*new_ctx->depths))
    };
    if (!new_ctx->depths)
    {
        verify_fail("out of memory while creating depth map for context at 0x%08x", begin_offset);
    }
    for (uint32_t i = 0; i < verifier.bf->code_size; ++i)
    {
        new_ctx->depths[i] = -1;
    }

    uint32_t new_ctx_id = (uint32_t)verifier.ctx_count;
    verifier.ctx_count++;
    return new_ctx_id;
}

static void check_target(uint32_t source_offset, uint32_t target, const char* kind)
{
    if (target >= verifier.bf->code_size)
    {
        verify_fail("0x%08x: %s target 0x%08x is outside code (size=0x%08x)", source_offset, kind, target,
                    verifier.bf->code_size);
    }
    if (!verifier.is_instr_start[target])
    {
        verify_fail("0x%08x: %s target 0x%08x is not instruction-aligned", source_offset, kind, target);
    }
}

static void check_index(uint32_t source_offset, uint32_t index, uint32_t limit, const char* kind)
{
    if (index >= limit)
    {
        verify_fail("0x%08x: %s index %u is out of range (limit=%u)", source_offset, kind, index, limit);
    }
}

static void enqueue_join(uint32_t ctx_id, uint32_t offset, uint32_t depth)
{
    if (offset >= verifier.bf->code_size)
    {
        verify_fail("0x%08x: control-flow edge leaves code section", offset);
    }
    if (!verifier.is_instr_start[offset])
    {
        verify_fail("0x%08x: control-flow edge targets non-instruction byte", offset);
    }

    frame_context* ctx = &verifier.ctxs[ctx_id];
    const int32_t prev_depth = ctx->depths[offset];
    if (prev_depth == -1)
    {
        ctx->depths[offset] = (int32_t)depth;
        push_cfg_stack(ctx_id, offset);
        return;
    }

    if ((uint32_t)prev_depth != depth)
    {
        verify_fail("0x%08x: stack depth mismatch at merge (%u != %u)", offset, (uint32_t)prev_depth, depth);
    }
}

static void seed_entrypoints(void)
{
    const uint32_t public_count = verifier.bf->data->public_symbols_number;
    const public_symbol_t* publics = (const public_symbol_t*)verifier.bf->public_ptr;

    if (public_count == 0)
    {
        if (verifier.bf->code_size == 0)
        {
            verify_fail("bytecode has no code");
        }
        uint32_t ctx_id = get_or_create_context(0, 0, 0);
        enqueue_join(ctx_id, 0, 0);
    }

    for (uint32_t i = 0; i < public_count; ++i)
    {
        const uint32_t name_idx = publics[i].name;
        const uint32_t offset = publics[i].offset;

        if (name_idx >= verifier.bf->data->stringtab_size)
        {
            verify_fail("public symbol #%u has invalid name index %u (string table size=%u)",
                        i, name_idx, verifier.bf->data->stringtab_size);
        }
        if (offset >= verifier.bf->code_size)
        {
            verify_fail("public symbol #%u has invalid entry offset 0x%08x (code size=0x%08x)",
                        i, offset, verifier.bf->code_size);
        }
        if (!verifier.is_instr_start[offset])
        {
            verify_fail("public symbol #%u points to non-instruction offset 0x%08x", i, offset);
        }

        uint32_t ctx_id = get_or_create_context(offset, 0, 0);
        enqueue_join(ctx_id, offset, 0);
    }
}

static void verify_closure_captures(const frame_context* ctx, const decoded_instruction* inst)
{
    uint32_t pos = inst->closure_captures_offset;
    for (uint32_t i = 0; i < inst->closure_capture_count; ++i)
    {
        if (pos + READ_BYTE + READ_INT > verifier.bf->code_size)
        {
            verify_fail("0x%08x: malformed closure capture list", inst->offset);
        }
        const uint8_t kind = (uint8_t)verifier.bf->code_ptr[pos];
        uint32_t index;
        memcpy(&index, verifier.bf->code_ptr + pos + 1, sizeof(index));

        switch (kind)
        {
        case 0:
            check_index(inst->offset, index, verifier.bf->data->global_area_size, "closure G");
            break;
        case 1:
            check_index(inst->offset, index, ctx->locals_count, "closure L");
            break;
        case 2:
            check_index(inst->offset, index, ctx->args_count, "closure A");
            break;
        case 3:
            check_index(inst->offset, index, ctx->captures_count, "closure C");
            break;
        default:
            verify_fail("0x%08x: invalid closure capture kind %u", inst->offset, kind);
        }
        pos += READ_BYTE + READ_INT;
    }
}

static void compute_stack_effect(const decoded_instruction* inst, uint32_t* pop_count, uint32_t* push_count)
{
    const uint8_t op = inst->op;
    const int verify_kind = opcode_verify_kind[op];

    if (verify_kind == VFY_STA)
    {
        *pop_count = 0;
        *push_count = 0;
        return;
    }

    if (verify_kind == VFY_FIXED)
    {
        *pop_count = opcode_stack_pop[op];
        *push_count = opcode_stack_push[op];
        return;
    }

    *push_count = opcode_stack_push[op];

    switch (verify_kind)
    {
    case VFY_SEXP:
        *pop_count = inst->arg[1];
        break;
    case VFY_CALL:
        *pop_count = inst->arg[1];
        break;
    case VFY_CALLC:
        if (inst->arg[0] == UINT32_MAX)
        {
            verify_fail("0x%08x: CALLC argument count overflow", inst->offset);
        }
        *pop_count = inst->arg[0] + 1;
        break;
    case VFY_BARRAY:
        *pop_count = inst->arg[0];
        break;
    default:
        verify_fail("0x%08x: unknown verifier stack rule %d", inst->offset,
                    verify_kind);
    }
}

static uint32_t verify_instruction(uint32_t ctx_id, const decoded_instruction* inst,
                               uint32_t depth_entry)
{
    frame_context* ctx = &verifier.ctxs[ctx_id];
    const uint8_t op = inst->op;
    const uint32_t index = inst->arg[0];

    switch (op)
    {
    case OPC_LD_G:
    case OPC_LDA_G:
    case OPC_ST_G:
        check_index(inst->offset, index, verifier.bf->data->global_area_size, "global");
        break;
    case OPC_LD_L:
    case OPC_LDA_L:
    case OPC_ST_L:
        check_index(inst->offset, index, ctx->locals_count, "local");
        break;
    case OPC_LD_A:
    case OPC_LDA_A:
    case OPC_ST_A:
        check_index(inst->offset, index, ctx->args_count, "arg");
        break;
    case OPC_LD_C:
    case OPC_LDA_C:
    case OPC_ST_C:
        check_index(inst->offset, index, ctx->captures_count, "capture");
        break;
    case OPC_JMP:
    case OPC_CJMPz:
    case OPC_CJMPnz:
        check_target(inst->offset, inst->arg[0], "jump");
        break;
    case OPC_BEGIN:
    case OPC_CBEGIN:
        if (inst->offset != ctx->begin_offset)
        {
            verify_fail("0x%08x: reached %s in the middle of a block", inst->offset, opcode_names[op]);
        }
        if (op != ctx->begin_opcode)
        {
            verify_fail("0x%08x: BEGIN/CBEGIN opcode does not match context", inst->offset);
        }
        if (inst->arg[0] != ctx->args_count || inst->arg[1] != ctx->locals_count)
        {
            verify_fail("0x%08x: BEGIN/CBEGIN header changed in context", inst->offset);
        }
        break;
    case OPC_CALL:
        {
            check_target(inst->offset, inst->arg[0], "call");
            const decoded_instruction* callee_begin = &verifier.instr[inst->arg[0]];
            if (callee_begin->op != OPC_BEGIN)
            {
                verify_fail("0x%08x: CALL target 0x%08x must be BEGIN, got %s",
                            inst->offset, inst->arg[0], opcode_names[callee_begin->op]);
            }
            if (callee_begin->arg[0] != inst->arg[1])
            {
                verify_fail("0x%08x: CALL args mismatch: call passes %u, BEGIN expects %u",
                            inst->offset, inst->arg[1], callee_begin->arg[0]);
            }
            uint32_t callee_ctx_id = get_or_create_context(inst->arg[0], OPC_BEGIN, 0);
            enqueue_join(callee_ctx_id, inst->arg[0], 0);
            break;
        }
    case OPC_CLOSURE:
        {
            check_target(inst->offset, inst->arg[0], "closure");
            const decoded_instruction* closure_begin = &verifier.instr[inst->arg[0]];
            if (closure_begin->op != OPC_CBEGIN && closure_begin->op != OPC_BEGIN)
            {
                verify_fail("0x%08x: CLOSURE target 0x%08x must be BEGIN/CBEGIN, got %s",
                            inst->offset, inst->arg[0], opcode_names[closure_begin->op]);
            }
            verify_closure_captures(ctx, inst);
            uint32_t closure_ctx_id = get_or_create_context(inst->arg[0], 0, inst->arg[1]);
            enqueue_join(closure_ctx_id, inst->arg[0], 0);
            break;
        }
    default:
        break;
    }

    uint32_t pop_count;
    uint32_t push_count;
    compute_stack_effect(inst, &pop_count, &push_count);

    if (depth_entry > ctx->max_operand_depth)
    {
        ctx->max_operand_depth = depth_entry;
    }

    if (depth_entry < pop_count)
    {
        verify_fail("0x%08x: stack underflow (need=%u, have=%u)", inst->offset, pop_count, depth_entry);
    }

    const int64_t next_depth = (int64_t)depth_entry - pop_count + push_count;

    if (next_depth > ctx->max_operand_depth)
    {
        ctx->max_operand_depth = next_depth;
    }
    return next_depth;
}

static int is_terminal(uint8_t op)
{
    if ((op & 0xF0) == 0xF0)
    {
        return 1;
    }
    return op == OPC_JMP || op == OPC_RET || op == OPC_END || op == OPC_FAIL;
}

static void propagate_successors(uint32_t ctx_id, const decoded_instruction* inst, uint32_t depth_next)
{
    const uint8_t op = inst->op;
    const uint32_t next = inst->offset + inst->length;

    if (op == OPC_JMP)
    {
        enqueue_join(ctx_id, inst->arg[0], depth_next);
    }

    if (op == OPC_CJMPz || op == OPC_CJMPnz)
    {
        enqueue_join(ctx_id, inst->arg[0], depth_next);
        if (next >= verifier.bf->code_size)
        {
            verify_fail("0x%08x: conditional jump fallthrough goes outside code", inst->offset);
        }
        enqueue_join(ctx_id, next, depth_next);
    }

    if (is_terminal(op))
    {
        return;
    }

    if (next >= verifier.bf->code_size)
    {
        verify_fail("0x%08x: instruction fallthrough goes outside code",
                    inst->offset);
    }

    enqueue_join(ctx_id, next, depth_next);
}

static void free_state(void)
{
    if (verifier.ctxs)
    {
        for (size_t i = 0; i < verifier.ctx_count; ++i)
        {
            free(verifier.ctxs[i].depths);
        }
    }
    free(verifier.ctxs);
    free(verifier.cfg_stack);
    free(verifier.instr);
    free(verifier.is_instr_start);
    verifier = (verifier_state){0};
}

void verify_bytecode(bytefile* bf)
{
    if (bf->code_size == 0)
    {
        verify_fail("empty code section");
    }
    verifier.bf = bf;
    verifier.ctx_capacity = verifier.bf->code_size;

    verifier.instr = calloc(verifier.bf->code_size, sizeof(*verifier.instr));
    verifier.is_instr_start = calloc(verifier.bf->code_size, sizeof(*verifier.is_instr_start));
    verifier.ctxs = calloc(verifier.ctx_capacity, sizeof(*verifier.ctxs));
    if (!verifier.instr || !verifier.is_instr_start || !verifier.ctxs)
    {
        verify_fail("out of memory");
    }

    decode_all_instructions();
    seed_entrypoints();

    while (verifier.cfg_stack_size)
    {
        const cfg_stack_item i = verifier.cfg_stack[--verifier.cfg_stack_size];
        frame_context* ctx = &verifier.ctxs[i.ctx_id];
        const decoded_instruction* inst = &verifier.instr[i.offset];
        const int32_t depth_mark = ctx->depths[i.offset];
        if (depth_mark < 0)
        {
            continue;
        }

        uint32_t depth_next = verify_instruction(i.ctx_id, inst, (uint32_t)depth_mark);

        const uint32_t next = inst->offset + inst->length;
        if (inst->op == OPC_JMP)
        {
            enqueue_join(i.ctx_id, inst->arg[0], depth_next);
        }
        if (inst->op == OPC_CJMPz || inst->op == OPC_CJMPnz)
        {
            enqueue_join(i.ctx_id, inst->arg[0], depth_next);
            if (next >= verifier.bf->code_size)
            {
                verify_fail("0x%08x: conditional jump fallthrough goes outside code", inst->offset);
            }
            enqueue_join(i.ctx_id, next, depth_next);
        }
        if (is_terminal(inst->op))
        {
            return;
        }
        if (next >= verifier.bf->code_size)
        {
            verify_fail("0x%08x: instruction fallthrough goes outside code",
                        inst->offset);
        }
        enqueue_join(i.ctx_id, next, depth_next);
    }

    free_state();
}
