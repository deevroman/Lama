#ifndef LOADER_H
#define LOADER_H

#include <stdint.h>
#include <stdio.h>

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
} PACKED bytefile_data;
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

bytefile* load_bytecode_file(const char* filename);

#endif
