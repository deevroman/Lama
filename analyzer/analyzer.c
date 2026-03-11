#include "analyzer.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "runtime.h"
#include "../interp/interpreter.h"
#include "../interp/disasm_format.h"

static int is_jump(uint8_t op)
{
    return op == OPC_JMP
        || op == OPC_CJMPz
        || op == OPC_CJMPnz
        || op == OPC_CALL;
}

static int is_call(uint8_t op)
{
    return op == OPC_CALLC
        || op == OPC_CALL;
}

static int is_terminal(uint8_t op)
{
    return op == OPC_JMP
        || op == OPC_RET
        || op == OPC_END
        || op == OPC_FAIL;
}

static int is_sequence_break(uint8_t op)
{
    return op == OPC_JMP
        || op == OPC_CALL
        || op == OPC_CALLC
        || op == OPC_RET
        || op == OPC_END
        || op == OPC_FAIL;
}

static disasm_context make_context_at(uint32_t offset)
{
    disasm_context context = {
        .code_begin = bytecode->code_ptr,
        .code_end = bytecode->code_ptr + bytecode->code_size,
        .ip = bytecode->code_ptr + offset,
        .string_ptr = bytecode->string_ptr,
        .stringtab_size = bytecode->data->stringtab_size,
    };
    return context;
}

static uint32_t instruction_args_length(uint32_t offset)
{
    const size_t code_size = bytecode->code_size;
    if (offset >= code_size)
    {
        failure("instruction offset out of bytecode range: %u (code_size=%zu)\n", offset, code_size);
    }

    disasm_context context = make_context_at(offset);
    const uint8_t op = disasm_read_u8(&context);

    DEBUG_LOG("[ANALYZER] instruction_length opcode=0x%02X (%s)\n", op, opcode_names[op]);

    if (!opcodes[op])
    {
        failure("unknown opcode in instruction_length: 0x%02x\n", op);
    }

    const int args_len = opcode_args_count[op];
    if (args_len != READ_CUSTOM)
    {
        DEBUG_LOG("[ANALYZER] -> args length=%d\n", args_len);
        return (uint32_t)args_len;
    }

    switch (opcode_disasm_mode[op])
    {
    case DISASM_MODE_CLOSURE:
    {
        (void)disasm_read_u32(&context);
        const uint32_t n = disasm_read_u32(&context);
        const uint32_t len = (uint32_t)(2 * sizeof(uint32_t) + n * (sizeof(uint32_t) + sizeof(uint8_t)));
        DEBUG_LOG("[ANALYZER] -> args length=%u\n", len);
        return len;
    }
    default:
        failure("unsupported variable-length opcode 0x%02X (mode=%d)\n", op, opcode_disasm_mode[op]);
    }
}

static uint32_t instruction_length(uint32_t offset)
{
    return 1 + instruction_args_length(offset);
}

static uint32_t get_jump_target(uint32_t offset)
{
    disasm_context context = make_context_at(offset);
    (void)disasm_read_u8(&context);
    return disasm_read_u32(&context);
}

static void walk_bytecode(bytefile* bytefile, bool* visited, bool* has_label)
{
    uint32_t code_size = bytefile->code_size;
    DEBUG_LOG("[ANALYZER] code_size=%u\n", code_size);

    uint32_t* stack = malloc(code_size * sizeof(uint32_t));
    uint32_t sp = 0;

    for (uint32_t i = 0; i < bytefile->data->public_symbols_number; ++i)
    {
        uint32_t offset = ((public_symbol_t*)bytefile->public_ptr)[i].offset;
        DEBUG_LOG("[ANALYZER] public entry: offset=%u\n", offset);

        if (!has_label[offset])
        {
            has_label[offset] = 1;
            stack[sp++] = offset;
        }
    }

    while (sp)
    {
        uint32_t pos = stack[--sp];
        DEBUG_LOG("[ANALYZER] pop offset=%u (sp=%u)\n", pos, sp);
        if (visited[pos])
        {
            DEBUG_LOG("[ANALYZER] already reachable, skip\n");
            continue;
        }
        visited[pos] = true;

        uint8_t op = bytefile->code_ptr[pos];
        uint32_t len = instruction_length(pos);
        DEBUG_LOG("[ANALYZER] opcode=0x%02X len=%u at %u\n", op, len, pos);

        if (is_jump(op))
        {
            uint32_t target = get_jump_target(pos);
            has_label[target] = true;
            DEBUG_LOG("[ANALYZER] jump detected -> target=%u\n", target);
            if (!visited[target])
            {
                stack[sp++] = target;
                DEBUG_LOG("[ANALYZER] push target %u\n", target);
            }
        }

        if (!is_terminal(op))
        {
            uint32_t next = pos + len;
            DEBUG_LOG("[ANALYZER] next=%u\n", next);
            if (next < code_size)
            {
                if (is_call(op))
                {
                    has_label[next] = true;
                    DEBUG_LOG("[ANALYZER] call -> label fallthrough %u\n", next);
                }
                if (!visited[next])
                {
                    stack[sp++] = next;
                    DEBUG_LOG("[ANALYZER] push next %u\n", next);
                }
            }
        }
    }

    free(stack);
    DEBUG_LOG("[ANALYZER] === Preprocess end ===\n");
}

static int compare_idiom(const idiom_info_t* a, const idiom_info_t* b)
{
    return memcmp(bytecode->code_ptr + a->offset, bytecode->code_ptr + b->offset, a->size);
}

static int cmp_qsort(const void* pa, const void* pb) { return compare_idiom(pa, pb); }

static uint32_t collect_frequencies(idiom_info_t* idioms, uint32_t count, idiom_stat_t* out, uint32_t instruction_count)
{
    if (count == 0)
    {
        return 0;
    }
    qsort(idioms, count, sizeof(idiom_info_t), cmp_qsort);

    uint32_t freq = 1, out_len = 0;
    for (uint32_t i = 1; i < count; ++i)
    {
        if (compare_idiom(&idioms[i - 1], &idioms[i]) == 0)
        {
            freq++;
        }
        else
        {
            out[out_len++] = (idiom_stat_t){idioms[i - 1].offset, freq, instruction_count};
            freq = 1;
        }
    }
    out[out_len++] = (idiom_stat_t){idioms[count - 1].offset, freq, instruction_count};
    return out_len;
}

static void print_instruction(uint32_t offset)
{
    disasm_context context = make_context_at(offset);
    const uint8_t op = disasm_read_u8(&context);

    if (!opcodes[op])
    {
        printf("invalid opcode 0x%02X", op);
        return;
    }

    const char* disasm_text = opcode_disasm_text[op];
    if (!disasm_text)
    {
        printf("%s", opcode_names[op]);
        return;
    }

    disasm_print(stdout, &context, disasm_text, opcode_disasm_mode[op]);
}

static void print_idiom(idiom_stat_t idiom)
{
    uint32_t len1 = instruction_length(idiom.offset);
    print_instruction(idiom.offset);
    if (idiom.instruction_count == 2)
    {
        printf("\t->\t");
        print_instruction(idiom.offset + len1);
    }
}

static int cmp_idiom_result_desc(const void* a, const void* b)
{
    const idiom_stat_t* ia = a;
    const idiom_stat_t* ib = b;

    if (ib->freq > ia->freq) return 1;
    if (ib->freq < ia->freq) return -1;
    const uint8_t op_a = (uint8_t)bytecode->code_ptr[ia->offset];
    const uint8_t op_b = (uint8_t)bytecode->code_ptr[ib->offset];
    return strcmp(
        opcode_names[op_a],
        opcode_names[op_b]
    );
}

void analyze_bytecode(bytefile* bytefile)
{
    bytecode = bytefile;

    bool* visited = calloc(bytefile->code_size, 1);
    bool* has_label = calloc(bytefile->code_size, 1);
    if (visited == NULL || has_label == NULL)
    {
        perror("calloc");
        exit(1);
    }

    walk_bytecode(bytefile, visited, has_label);

    idiom_info_t* idioms1 = malloc(bytefile->code_size * sizeof(idiom_info_t));
    idiom_info_t* idioms2 = malloc(bytefile->code_size * sizeof(idiom_info_t));
    uint32_t idioms1cnt = 0, idioms2cnt = 0;

    uint32_t cur_offset = 0;
    while (cur_offset < bytefile->code_size)
    {
        if (!visited[cur_offset])
        {
            cur_offset++;
            continue;
        }
        opcode_t op = bytefile->code_ptr[cur_offset];
        uint32_t len = instruction_length(cur_offset);
        DEBUG_LOG("[ANALYZER] instruction at %u len=%u\n", cur_offset, len);

        idioms1[idioms1cnt++] = (idiom_info_t){cur_offset, len};
        DEBUG_LOG("[ANALYZER] add idiom1 offset=%u size=%u\n", cur_offset, len);

        uint32_t next = cur_offset + len;
        if (next < bytefile->code_size && !is_sequence_break(op) && visited[next] && !has_label[next])
        {
            uint32_t len2 = instruction_length(next);
            idioms2[idioms2cnt++] = (idiom_info_t){cur_offset, len + len2};
            DEBUG_LOG("[ANALYZER] add idiom2 offset=%u size=%u\n", cur_offset, len + len2);
        }

        cur_offset += len;
    }

    DEBUG_LOG("[ANALYZER] collected: idioms1=%u idioms2=%u\n", idioms1cnt, idioms2cnt);

    idiom_stat_t* res1 = malloc(idioms1cnt * sizeof(idiom_stat_t));
    idiom_stat_t* res2 = malloc(idioms2cnt * sizeof(idiom_stat_t));
    if (res1 == NULL || res2 == NULL)
    {
        perror("malloc");
        exit(1);
    }

    uint32_t r1 = collect_frequencies(idioms1, idioms1cnt, res1, 1);
    uint32_t r2 = collect_frequencies(idioms2, idioms2cnt, res2, 2);

    uint32_t total = r1 + r2;
    idiom_stat_t* merged = malloc(total * sizeof(idiom_stat_t));
    if (!merged)
    {
        perror("malloc");
        exit(1);
    }
    memcpy(merged, res1, r1 * sizeof(idiom_stat_t));
    memcpy(merged + r1, res2, r2 * sizeof(idiom_stat_t));

    free(res1);
    free(res2);

    qsort(merged, total, sizeof(idiom_stat_t), cmp_idiom_result_desc);

    printf("Idioms stat: \n");
    for (uint32_t i = 0; i < total; ++i)
    {
        printf("cnt=%u ", merged[i].freq);
        print_idiom(merged[i]);
        printf("\n");
    }

    free(visited);
    free(has_label);
    free(idioms1);
    free(idioms2);
}
