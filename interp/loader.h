#ifndef LOADER_H
#define LOADER_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

#define ERROR_LOG(fmt, ...) do { fprintf(stderr, fmt, ##__VA_ARGS__); fflush(stderr); } while (0)

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif

#pragma pack(push, 1)
typedef struct
{
    uint32_t stringtab_size;
    uint32_t global_area_size;
    uint32_t public_symbols_number;

    char buffer[0];
} bytefile_data PACKED;
#pragma pack(pop)

typedef struct
{
    char* string_ptr;
    int* public_ptr;
    char* code_ptr;
    size_t code_size;
    int* global_ptr;

    bytefile_data* data;
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
        ERROR_LOG("Cannot open file: %s: %s\n", filename, strerror(errno));
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) != 0)
    {
        perror("Failed to seek file");
        fclose(f);
        return NULL;
    }

    const long end_pos = ftell(f);
    if (end_pos < 0)
    {
        perror("Failed to tell file size\n");
        fclose(f);
        return NULL;
    }

    const size_t size = (size_t)end_pos;
    if (fseek(f, 0, SEEK_SET) != 0)
    {
        perror("Failed to seek file\n");
        fclose(f);
        return NULL;
    }

    bytefile* file = malloc(sizeof(bytefile));
    file->data = malloc(sizeof(bytefile_data) + size);
    if (!file->data)
    {
        ERROR_LOG("Failed to alloc %zu\n", sizeof(bytefile_data) + size);
        fclose(f);
        return NULL;
    }

    const size_t read_bytes = fread(&file->data->stringtab_size, 1, size, f);
    if (read_bytes != size)
    {
        perror("Failed to read file\n");
        free(file->data);
        return NULL;
    }
    fclose(f);

    if (size < sizeof(file->data->stringtab_size) + sizeof(file->data->global_area_size) + sizeof(file->data->public_symbols_number))
    {
        ERROR_LOG("Invalid bytecode size: %zu\n", size);
        free(file->data);
        return NULL;
    }

    DEBUG_LOG("[PARSE] Read %zu bytes\n", read_bytes);
    DEBUG_LOG("[PARSE] stringtab_size=%d (0x%x)\n", file->stringtab_size, file->stringtab_size);
    DEBUG_LOG("[PARSE] global_area_size=%d (0x%x)\n", file->global_area_size, file->global_area_size);
    DEBUG_LOG("[PARSE] public_symbols_number=%d (0x%x)\n", file->public_symbols_number, file->public_symbols_number);

    const size_t payload_bytes = size - sizeof(file->data->stringtab_size) + sizeof(file->data->global_area_size) + sizeof(file->data->public_symbols_number);
    const size_t public_bytes = (size_t)file->data->public_symbols_number * sizeof(public_symbol_t);
    const size_t string_bytes = (size_t)file->data->stringtab_size;

    DEBUG_LOG("[PARSE] size=%zu, payload=%zu\n", size, payload_bytes);
    DEBUG_LOG("[PARSE] public_symbols_number=%d, public_bytes=%zu\n", file->public_symbols_number, public_bytes);
    DEBUG_LOG("[PARSE] stringtab_size=%d, string_bytes=%zu\n", file->stringtab_size, string_bytes);

    if (public_bytes > payload_bytes || public_bytes + string_bytes > payload_bytes)
    {
        ERROR_LOG("Invalid bytecode: tables out of range\n");
        free(file->data);
        return NULL;
    }

    file->public_ptr = (int32_t*)file->data->buffer;
    file->string_ptr = file->data->buffer + public_bytes;
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

    return file;
}

#endif
