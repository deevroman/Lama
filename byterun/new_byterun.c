#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../runtime/runtime.h"
#include "../interp/disasm_format.h"

void* __start_custom_data;
void* __stop_custom_data;

typedef struct
{
    char* string_ptr;
    int32_t* public_ptr;
    char* code_ptr;
    int32_t* global_ptr;
    size_t code_size;
    uint32_t stringtab_size;
    uint32_t global_area_size;
    uint32_t public_symbols_number;
    char buffer[0];
} bytefile;

static char* get_string(bytefile* f, uint32_t pos)
{
    if (pos >= f->stringtab_size)
    {
        failure("invalid string table offset %u (table size=%u)\n", pos, f->stringtab_size);
    }
    return &f->string_ptr[pos];
}

static char* get_public_name(bytefile* f, uint32_t i)
{
    return get_string(f, (uint32_t)f->public_ptr[i * 2]);
}

static uint32_t get_public_offset(bytefile* f, uint32_t i)
{
    return (uint32_t)f->public_ptr[i * 2 + 1];
}

static bytefile* read_file(const char* fname)
{
    FILE* f = fopen(fname, "rb");
    if (!f)
    {
        failure("%s\n", strerror(errno));
    }

    if (fseek(f, 0, SEEK_END) != 0)
    {
        failure("%s\n", strerror(errno));
    }

    const long end_pos = ftell(f);
    if (end_pos < 0)
    {
        failure("%s\n", strerror(errno));
    }
    const size_t size = (size_t)end_pos;

    if (fseek(f, 0, SEEK_SET) != 0)
    {
        failure("%s\n", strerror(errno));
    }

    if (size < 3 * sizeof(uint32_t))
    {
        failure("invalid bytecode: file is too small\n");
    }

    bytefile* file = (bytefile*)malloc(sizeof(*file) + size);
    if (!file)
    {
        failure("*** FAILURE: unable to allocate memory.\n");
    }

    const size_t read_bytes = fread(&file->stringtab_size, 1, size, f);
    if (read_bytes != size)
    {
        failure("%s\n", strerror(errno));
    }
    fclose(f);

    const size_t payload_bytes = size - 3 * sizeof(uint32_t);
    const size_t public_bytes = (size_t)file->public_symbols_number * 2 * sizeof(int32_t);
    const size_t string_bytes = file->stringtab_size;

    if (public_bytes > payload_bytes || public_bytes + string_bytes > payload_bytes)
    {
        failure("invalid bytecode: malformed public/string tables\n");
    }

    file->public_ptr = (int32_t*)file->buffer;
    file->string_ptr = file->buffer + public_bytes;
    file->code_ptr = file->string_ptr + string_bytes;
    file->code_size = payload_bytes - public_bytes - string_bytes;
    file->global_ptr = (int32_t*)calloc(file->global_area_size, sizeof(int32_t));
    if (!file->global_ptr)
    {
        failure("*** FAILURE: unable to allocate global area.\n");
    }

    return file;
}

static void print_instruction(FILE* f, uint8_t opcode, disasm_context* context)
{
#pragma push_macro("TAG")
#undef TAG
    switch (opcode)
    {
#define X(code, name, func, args_bytes, disasm_format, disasm_mode, stack_pop, stack_push, verify_kind) \
    case code:                                                       \
        disasm_print(f, context, disasm_format, disasm_mode);         \
        break;
#include "../interp/opcodes.inc"
    default:
        failure("invalid opcode 0x%02x at code offset %zu\n", opcode, disasm_code_pos(context, context->ip - 1));
    }
#pragma pop_macro("TAG")
}

static void disassemble(FILE* f, bytefile* bf)
{
    disasm_context context = {
        .code_begin = bf->code_ptr,
        .code_end = bf->code_ptr + bf->code_size,
        .ip = bf->code_ptr,
        .string_ptr = bf->string_ptr,
        .stringtab_size = bf->stringtab_size,
    };

    while (context.ip < context.code_end)
    {
        const size_t offset = disasm_code_pos(&context, context.ip);
        const uint8_t opcode = disasm_read_u8(&context);

        fprintf(f, "0x%.8zx:\t", offset);

        if ((opcode & 0xF0) == 0xF0)
        {
            fprintf(f, "<end>\n");
            return;
        }

        print_instruction(f, opcode, &context);
        fputc('\n', f);
    }

    fprintf(f, "<eof>\n");
}

static void dump_file(FILE* f, bytefile* bf)
{
    fprintf(f, "String table size       : %u\n", bf->stringtab_size);
    fprintf(f, "Global area size        : %u\n", bf->global_area_size);
    fprintf(f, "Number of public symbols: %u\n", bf->public_symbols_number);
    fprintf(f, "Public symbols          :\n");

    for (uint32_t i = 0; i < bf->public_symbols_number; i++)
    {
        fprintf(f, "   0x%.8x: %s\n", get_public_offset(bf, i), get_public_name(bf, i));
    }

    fprintf(f, "Code:\n");
    disassemble(f, bf);
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s file.bc\n", argv[0]);
        return 1;
    }

    bytefile* f = read_file(argv[1]);
    dump_file(stdout, f);
    free(f->global_ptr);
    free(f);
    return 0;
}
