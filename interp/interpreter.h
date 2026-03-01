#ifndef OPCODES_H
#define OPCODES_H

#include "globals.h"
#include "callstack.h"

/* Bytecode reading helpers */

size_t file_position_from_ip(const char* ip);
unsigned char read_byte();
uint32_t read_uint(void);
uint32_t read_int(void);
char* read_string(void);

/* Opcode handlers */
#define DEFINE_BINOP_DECL(NAME) error_t NAME(void);

DEFINE_BINOP_DECL(op_add)
DEFINE_BINOP_DECL(op_sub)
DEFINE_BINOP_DECL(op_mul)

DEFINE_BINOP_DECL(op_lt)
DEFINE_BINOP_DECL(op_le)
DEFINE_BINOP_DECL(op_gt)
DEFINE_BINOP_DECL(op_ge)
DEFINE_BINOP_DECL(op_eq)
DEFINE_BINOP_DECL(op_ne)

DEFINE_BINOP_DECL(op_and)
DEFINE_BINOP_DECL(op_or)

DEFINE_BINOP_DECL(op_div)
DEFINE_BINOP_DECL(op_mod)

error_t op_const(void);
error_t op_ld_g(void);

error_t op_st_g(void);

error_t op_drop(void);
error_t op_dup(void);

error_t op_swap(void);
[[nodiscard]] error_t safe_jmp(uint32_t offset);

error_t op_jmp(void);

error_t op_end(void);
error_t op_builtin_lread(void);

[[nodiscard]] error_t op_builtin_lwrite(void);

error_t op_line(void);
error_t op_begin(void);

#define DEFINE_CJMP(name) error_t name(void);

DEFINE_CJMP(op_cjmpz)
DEFINE_CJMP(op_cjmpnz)

error_t op_patt_str(void);
error_t op_patt_string_tag(void);
error_t op_patt_array_tag(void);
error_t op_patt_sexp_tag(void);
error_t op_patt_fun(void);
error_t op_patt_ref(void);
error_t op_patt_val(void);

error_t op_string(void);
error_t op_sexp(void);
error_t op_sti(void);
error_t op_sta(void);
error_t op_elem(void);
error_t op_ret(void);
error_t op_ld_l(void);
error_t op_ld_a(void);
error_t op_ld_c(void);
error_t op_lda_g(void);
error_t op_lda_l(void);
error_t op_lda_a(void);
error_t op_lda_c(void);
error_t op_st_l(void);
error_t op_st_a(void);
error_t op_st_c(void);
error_t op_cbegin(void);
error_t op_closure(void);
error_t op_callc(void);
error_t op_call(void);
error_t op_tag(void);
error_t op_array(void);
error_t op_fail(void);
error_t op_builtin_llength(void);
error_t op_builtin_lstring(void);
error_t op_builtin_barray(void);
error_t op_stop(void);

typedef error_t (*opcode_handler)(void);

#pragma push_macro("TAG")
#undef TAG

#define OPCODE_LIST(X)                                                                           \
    X(0x01, ADD,             op_add,             0, 1)                                           \
    X(0x02, SUB,             op_sub,             0, 1)                                           \
    X(0x03, MUL,             op_mul,             0, 1)                                           \
    X(0x04, DIV,             op_div,             0, 1)                                           \
    X(0x05, MOD,             op_mod,             0, 1)                                           \
    X(0x06, LT,              op_lt,              0, 1)                                           \
    X(0x07, LE,              op_le,              0, 1)                                           \
    X(0x08, GT,              op_gt,              0, 1)                                           \
    X(0x09, GE,              op_ge,              0, 1)                                           \
    X(0x0A, EQ,              op_eq,              0, 1)                                           \
    X(0x0B, NE,              op_ne,              0, 1)                                           \
    X(0x0C, AND,             op_and,             0, 1)                                           \
    X(0x0D, OR,              op_or,              0, 1)                                           \
                                                                                                 \
    X(0x10, CONST,           op_const,           4, 1)                                           \
    X(0x11, STRING,          op_string,          4, 1)                                           \
    X(0x12, SEXP,            op_sexp,            8, 1)                                           \
    X(0x13, STI,             op_sti,             0, 1)                                           \
    X(0x14, STA,             op_sta,             0, 1)                                           \
    X(0x15, JMP,             op_jmp,             4, 1)                                           \
    X(0x16, END,             op_end,             0, 1)                                           \
    X(0x17, RET,             op_ret,             0, 1)                                           \
    X(0x18, DROP,            op_drop,            0, 1)                                           \
    X(0x19, DUP,             op_dup,             0, 1)                                           \
    X(0x1A, SWAP,            op_swap,            0, 1)                                           \
    X(0x1B, ELEM,            op_elem,            0, 1)                                           \
                                                                                                 \
    X(0x20, LD_G,            op_ld_g,            4, 1)                                           \
    X(0x21, LD_L,            op_ld_l,            4, 1)                                           \
    X(0x22, LD_A,            op_ld_a,            4, 1)                                           \
    X(0x23, LD_C,            op_ld_c,            4, 1)                                           \
                                                                                                 \
    X(0x30, LDA_G,           op_lda_g,           4, 1)                                           \
    X(0x31, LDA_L,           op_lda_l,           4, 1)                                           \
    X(0x32, LDA_A,           op_lda_a,           4, 1)                                           \
    X(0x33, LDA_C,           op_lda_c,           4, 1)                                           \
                                                                                                 \
    X(0x40, ST_G,            op_st_g,            4, 1)                                           \
    X(0x41, ST_L,            op_st_l,            4, 1)                                           \
    X(0x42, ST_A,            op_st_a,            4, 1)                                           \
    X(0x43, ST_C,            op_st_c,            4, 1)                                           \
                                                                                                 \
    X(0x50, CJMPz,           op_cjmpz,           4, 1)                                           \
    X(0x51, CJMPnz,          op_cjmpnz,          4, 1)                                           \
    X(0x52, BEGIN,           op_begin,           8, 1)                                           \
    X(0x53, CBEGIN,          op_cbegin,          8, 1)                                           \
    X(0x54, CLOSURE,         op_closure,        -1, 1)                                           \
    X(0x55, CALLC,           op_callc,           4, 1)                                           \
    X(0x56, CALL,            op_call,            8, 1)                                           \
    X(0x57, TAG,             op_tag,             8, 1)                                           \
    X(0x58, ARRAY,           op_array,           4, 1)                                           \
    X(0x59, FAIL,            op_fail,            8, 1)                                           \
    X(0x5A, LINE,            op_line,            4, 1)                                           \
                                                                                                 \
    X(0x60, PATT_STR,        op_patt_str,        0, 1)                                           \
    X(0x61, PATT_STRING_TAG, op_patt_string_tag, 0, 1)                                           \
    X(0x62, PATT_ARRAY_TAG,  op_patt_array_tag,  0, 1)                                           \
    X(0x63, PATT_SEXP_TAG,   op_patt_sexp_tag,   0, 1)                                           \
    X(0x64, PATT_REF,        op_patt_ref,        0, 1)                                           \
    X(0x65, PATT_VAL,        op_patt_val,        0, 1)                                           \
    X(0x66, PATT_FUN,        op_patt_fun,        0, 1)                                           \
                                                                                                 \
    X(0x70, LREAD,           op_builtin_lread,   0, 1)                                           \
    X(0x71, LWRITE,          op_builtin_lwrite,  0, 1)                                           \
    X(0x72, LLENGTH,         op_builtin_llength, 0, 1)                                           \
    X(0x73, LSTRING,         op_builtin_lstring, 0, 1)                                           \
    X(0x74, BARRAY,          op_builtin_barray,  4, 1)                                           \
                                                                                                 \
    X(0xF0, STOP,            op_stop,            0, 1)                                           \
    X(0xF1, STOP,            op_stop,            0, 0)                                           \
    X(0xF2, STOP,            op_stop,            0, 0)                                           \
    X(0xF3, STOP,            op_stop,            0, 0)                                           \
    X(0xF4, STOP,            op_stop,            0, 0)                                           \
    X(0xF5, STOP,            op_stop,            0, 0)                                           \
    X(0xF6, STOP,            op_stop,            0, 0)                                           \
    X(0xF7, STOP,            op_stop,            0, 0)                                           \
    X(0xF8, STOP,            op_stop,            0, 0)                                           \
    X(0xF9, STOP,            op_stop,            0, 0)                                           \
    X(0xFA, STOP,            op_stop,            0, 0)                                           \
    X(0xFB, STOP,            op_stop,            0, 0)                                           \
    X(0xFC, STOP,            op_stop,            0, 0)                                           \
    X(0xFD, STOP,            op_stop,            0, 0)                                           \
    X(0xFE, STOP,            op_stop,            0, 0)                                           \
    X(0xFF, STOP,            op_stop,            0, 0)                                           \

#define GEN_HANDLER(code, name, func, args_bytes, gen_enum) [code] = func,
const static opcode_handler opcodes[256] = {
    OPCODE_LIST(GEN_HANDLER)
};
#undef GEN_HANDLER

#define GEN_NAME(code, name, func, args_bytes, gen_enum) [code] = #name,
static const char* opcode_names[256] = {
    OPCODE_LIST(GEN_NAME)
};
#undef GEN_NAME

#define GEN_ARGS_COUNT(code, name, func, args_bytes, gen_enum) [code] = args_bytes,
static const int opcode_args_count[256] = {
    OPCODE_LIST(GEN_ARGS_COUNT)
};
#undef GEN_ARGS_COUNT


#define ENUM_IF_1(code, name) OPC_##name = code,
#define ENUM_IF_0(code, name)

#define GEN_ENUM(code, name, func, args_bytes, gen_enum) ENUM_IF_##gen_enum(code, name)
typedef enum {
    OPCODE_LIST(GEN_ENUM)
} opcode_t;
#undef GEN_ENUM

#define GEN_SWITCH_CASE(code, name, func, args_bytes, gen_enum) \
    case code:                                                  \
        res = func();                                           \
        break;

#define MAIN_SWITCH(x)                                                   \
    switch (x) {                                                         \
        OPCODE_LIST(GEN_SWITCH_CASE)                                     \
    default:                                                             \
        failure("No op_handler for opcode: 0x%02x file_position=%zu\n",  \
                x, file_position_from_ip(ip));                           \
    }                                                                    \

#pragma pop_macro("TAG")

void interpret_bytecode(bytefile* bf);

#endif //OPCODES_H
