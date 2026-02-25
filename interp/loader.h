#ifndef LOADER_H
#define LOADER_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#define debug(x)                        \
do {                                    \
    printf("%s: ", #x);                 \
    _Generic((x),                       \
        int: printf("%d", (x)),         \
        size_t: printf("%zu", (x)),     \
        long: printf("%ld", (x)),       \
        double: printf("%f", (x)),      \
        char*: printf("%s", (x)),       \
        default: printf("unknown type") \
    );                                  \
    printf("\n");                       \
} while (0)
#define DEBUG_LOG(fmt, ...) do { fprintf(stderr, fmt, ##__VA_ARGS__); fflush(stderr); } while (0)
#else
#define debug(...) do {} while (0)
#define DEBUG_LOG(...) do {} while (0)
#endif

typedef struct
{
    char* string_ptr;
    int* public_ptr;
    char* code_ptr;
    size_t code_size;
    int* global_ptr;

    unsigned int stringtab_size;
    unsigned int global_area_size;
    unsigned int public_symbols_number;

    char buffer[0];
} bytefile;

typedef struct
{
    uint32_t name;
    uint32_t offset;
} public_symbol_t;

bytefile* load_bytecode_file(const char* filename)
{
    DEBUG_LOG("[DEBUG] load_bytecode_file: opening %s\n", filename);

    FILE* f = fopen(filename, "rb");
    if (!f)
    {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) != 0)
    {
        fprintf(stderr, "Failed to seek file\n");
        fclose(f);
        return NULL;
    }

    const long end_pos = ftell(f);
    if (end_pos < 0)
    {
        fprintf(stderr, "Failed to tell file size\n");
        fclose(f);
        return NULL;
    }

    const size_t size = (size_t)end_pos;
    if (fseek(f, 0, SEEK_SET) != 0)
    {
        fprintf(stderr, "Failed to seek file\n");
        fclose(f);
        return NULL;
    }

    bytefile* file = malloc(sizeof(bytefile) + size);
    if (!file)
    {
        fprintf(stderr, "Failed to alloc %zu\n", sizeof(bytefile) + size);
        fclose(f);
        return NULL;
    }

    const size_t read_bytes = fread(&file->stringtab_size, 1, size, f);
    if (read_bytes != size)
    {
        fprintf(stderr, "Failed to read file\n");
        free(file);
        return NULL;
    }
    fclose(f);

    if (size < sizeof(file->stringtab_size) + sizeof(file->global_area_size) + sizeof(file->public_symbols_number))
    {
        fprintf(stderr, "Invalid bytecode size: %zu\n", size);
        free(file);
        return NULL;
    }

    DEBUG_LOG("[PARSE] Read %zu bytes\n", read_bytes);
    DEBUG_LOG("[PARSE] stringtab_size=%d (0x%x)\n", file->stringtab_size, file->stringtab_size);
    DEBUG_LOG("[PARSE] global_area_size=%d (0x%x)\n", file->global_area_size, file->global_area_size);
    DEBUG_LOG("[PARSE] public_symbols_number=%d (0x%x)\n", file->public_symbols_number, file->public_symbols_number);

    const size_t payload_bytes = size - sizeof(file->stringtab_size) + sizeof(file->global_area_size) + sizeof(file->public_symbols_number);
    const size_t public_bytes = (size_t)file->public_symbols_number * sizeof(public_symbol_t);
    const size_t string_bytes = (size_t)file->stringtab_size;

    DEBUG_LOG("[PARSE] size=%zu, payload=%zu\n", size, payload_bytes);
    DEBUG_LOG("[PARSE] public_symbols_number=%d, public_bytes=%zu\n", file->public_symbols_number, public_bytes);
    DEBUG_LOG("[PARSE] stringtab_size=%d, string_bytes=%zu\n", file->stringtab_size, string_bytes);

    if (public_bytes > payload_bytes || public_bytes + string_bytes > payload_bytes)
    {
        fprintf(stderr, "Invalid bytecode: tables out of range\n");
        free(file);
        return NULL;
    }

    file->public_ptr = (int32_t*)file->buffer;
    file->string_ptr = file->buffer + public_bytes;
    file->code_ptr = file->string_ptr + string_bytes;
    file->code_size = payload_bytes - public_bytes - string_bytes;

    DEBUG_LOG("[DEBUG] First 20 bytes of buffer: ");
    for (int i = 0; i < 20 && i < (int)payload_bytes; i++)
    {
        DEBUG_LOG("%02x ", (unsigned char)file->buffer[i]);
    }
    DEBUG_LOG("\n");
    DEBUG_LOG("[DEBUG] buffer addr=%p, string_ptr offset=%ld, code_ptr offset=%ld\n",
              (void*)file->buffer,
              (long)(file->string_ptr - file->buffer),
              (long)(file->code_ptr - file->buffer));

    DEBUG_LOG("[DEBUG] Bytecode structure:\n");
    DEBUG_LOG("[DEBUG]   stringtab_size=%d\n", file->stringtab_size);
    DEBUG_LOG("[DEBUG]   global_area_size=%d\n", file->global_area_size);
    DEBUG_LOG("[DEBUG]   public_symbols_number=%d\n", file->public_symbols_number);
    DEBUG_LOG("[DEBUG]   public_bytes=%zu\n", public_bytes);
    DEBUG_LOG("[DEBUG]   string_bytes=%zu\n", string_bytes);
    DEBUG_LOG("[DEBUG]   code_size=%zu\n", file->code_size);
    DEBUG_LOG("[DEBUG]   First code bytes: %02x %02x %02x %02x\n",
              (unsigned char)file->code_ptr[0],
              (unsigned char)file->code_ptr[1],
              (unsigned char)file->code_ptr[2],
              (unsigned char)file->code_ptr[3]);

    file->global_ptr = calloc((size_t)file->global_area_size, sizeof(int32_t));
    if (!file->global_ptr && file->global_area_size != 0)
    {
        fprintf(stderr, "Failed to allocate global area\n");
        free(file);
        return NULL;
    }

    return file;
}

#endif
