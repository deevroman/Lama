#ifndef DISASM_FORMAT_H
#define DISASM_FORMAT_H

#include "../runtime/runtime.h"
#include "opcodes.inc"

typedef struct
{
    const char* code_begin;
    const char* code_end;
    const char* ip;
    char* string_ptr;
    uint32_t stringtab_size;
} disasm_context;

static inline size_t disasm_code_pos(const disasm_context* ctx, const char* ptr)
{
    return (size_t)(ptr - ctx->code_begin);
}

static inline uint8_t disasm_read_u8(disasm_context* ctx)
{
    if (ctx->ip >= ctx->code_end)
    {
        failure("out of bytecode while reading byte at code offset=%zu\n", disasm_code_pos(ctx, ctx->ip));
    }
    return (uint8_t)*ctx->ip++;
}

static inline uint32_t disasm_read_u32(disasm_context* ctx)
{
    if (ctx->ip + sizeof(uint32_t) > ctx->code_end)
    {
        failure("out of bytecode while reading uint32 at code offset=%zu\n", disasm_code_pos(ctx, ctx->ip));
    }
    uint32_t value;
    memcpy(&value, ctx->ip, sizeof(value));
    ctx->ip += sizeof(value);
    return value;
}

static inline int32_t disasm_read_i32(disasm_context* ctx)
{
    return (int32_t)disasm_read_u32(ctx);
}

static inline char* disasm_read_string(disasm_context* ctx)
{
    const uint32_t pos = disasm_read_u32(ctx);
    if (pos >= ctx->stringtab_size)
    {
        failure("invalid string table offset %u (table size=%u)\n", pos, ctx->stringtab_size);
    }
    return &ctx->string_ptr[pos];
}

static inline void disasm_print_varspec(FILE* out, uint8_t kind, uint32_t index)
{
    switch (kind)
    {
    case 0:
        fprintf(out, "G(%u)", index);
        break;
    case 1:
        fprintf(out, "L(%u)", index);
        break;
    case 2:
        fprintf(out, "A(%u)", index);
        break;
    case 3:
        fprintf(out, "C(%u)", index);
        break;
    default:
        failure("invalid varspec kind %u in closure\n", kind);
    }
}

static inline void disasm_print(FILE* out, disasm_context* ctx, const char* format, int mode)
{
    switch (mode)
    {
    case DISASM_MODE_LITERAL:
        fprintf(out, "%s", format);
        break;
    case DISASM_MODE_I32:
        fprintf(out, format, disasm_read_i32(ctx));
        break;
    case DISASM_MODE_HEX32:
        fprintf(out, format, disasm_read_u32(ctx));
        break;
    case DISASM_MODE_I32_I32:
        fprintf(out, format, disasm_read_i32(ctx), disasm_read_i32(ctx));
        break;
    case DISASM_MODE_I32_I32_NOSPACE:
        fprintf(out, format, disasm_read_i32(ctx), disasm_read_i32(ctx));
        break;
    case DISASM_MODE_STRING:
        fprintf(out, format, disasm_read_string(ctx));
        break;
    case DISASM_MODE_STRING_I32:
        fprintf(out, format, disasm_read_string(ctx), disasm_read_i32(ctx));
        break;
    case DISASM_MODE_HEX32_I32:
        fprintf(out, format, disasm_read_u32(ctx), disasm_read_i32(ctx));
        break;
    case DISASM_MODE_CLOSURE:
    {
        const uint32_t closure_offset = disasm_read_u32(ctx);
        const uint32_t captures = disasm_read_u32(ctx);
        fprintf(out, format, closure_offset);
        for (uint32_t i = 0; i < captures; i++)
        {
            const uint8_t kind = disasm_read_u8(ctx);
            const uint32_t index = disasm_read_u32(ctx);
            disasm_print_varspec(out, kind, index);
        }
        break;
    }
    default:
        failure("unknown disasm mode %d for format \"%s\"\n", mode, format);
    }
}

#endif
