#ifndef ANALYZER_H
#define ANALYZER_H

#include "loader.h"

typedef struct
{
    uint32_t offset;
    uint32_t size;
} idiom_info_t;

typedef struct
{
    uint32_t offset;
    uint32_t freq;
    uint32_t instruction_count;
} idiom_stat_t;


void analyze_bytecode(bytefile* bytefile);

#endif
