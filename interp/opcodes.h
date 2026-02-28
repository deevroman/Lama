#ifndef OPCODES_H
#define OPCODES_H

#include "globals.h"
#include "callstack.h"

/* Bytecode reading helpers */

size_t file_position_from_ip(const char *ip);
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

static error_t op_const(void);
static error_t op_ld_g(void);

static error_t op_st_g(void);

static error_t op_drop(void);
static error_t op_dup(void);

static error_t op_swap(void);
[[nodiscard]] error_t safe_jmp(uint32_t offset);

static error_t op_jmp(void);

static error_t op_end(void);
static error_t op_builtin_lread(void);

[[nodiscard]] static error_t op_builtin_lwrite(void);

static error_t op_line(void);
static error_t op_begin(void);

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

const static opcode_handler opcodes[256] = {
    [0x01] = op_add,
    [0x02] = op_sub,
    [0x03] = op_mul,
    [0x04] = op_div,
    [0x05] = op_mod,
    [0x06] = op_lt,
    [0x07] = op_le,
    [0x08] = op_gt,
    [0x09] = op_ge,
    [0x0A] = op_eq,
    [0x0B] = op_ne,
    [0x0C] = op_and,
    [0x0D] = op_or,

    [0x10] = op_const,
    [0x11] = op_string,
    [0x12] = op_sexp,
    [0x13] = op_sti,
    [0x14] = op_sta,
    [0x15] = op_jmp,
    [0x16] = op_end,
    [0x17] = op_ret,
    [0x18] = op_drop,
    [0x19] = op_dup,
    [0x1A] = op_swap,
    [0x1B] = op_elem,

    [0x20] = op_ld_g,
    [0x21] = op_ld_l,
    [0x22] = op_ld_a,
    [0x23] = op_ld_c,

    [0x30] = op_lda_g,
    [0x31] = op_lda_l,
    [0x32] = op_lda_a,
    [0x33] = op_lda_c,

    [0x40] = op_st_g,
    [0x41] = op_st_l,
    [0x42] = op_st_a,
    [0x43] = op_st_c,

    [0x50] = op_cjmpz,
    [0x51] = op_cjmpnz,
    [0x52] = op_begin,
    [0x53] = op_cbegin,
    [0x54] = op_closure,
    [0x55] = op_callc,
    [0x56] = op_call,
    [0x57] = op_tag,
    [0x58] = op_array,
    [0x59] = op_fail,
    [0x5A] = op_line,

    [0x60] = op_patt_str,
    [0x61] = op_patt_string_tag,
    [0x62] = op_patt_array_tag,
    [0x63] = op_patt_sexp_tag,
    [0x64] = op_patt_ref,
    [0x65] = op_patt_val,
    [0x66] = op_patt_fun,

    [0x70] = op_builtin_lread,
    [0x71] = op_builtin_lwrite,
    [0x72] = op_builtin_llength,
    [0x73] = op_builtin_lstring,
    [0x74] = op_builtin_barray,

    [0xF0] = op_stop,
    [0xF1] = op_stop,
    [0xF2] = op_stop,
    [0xF3] = op_stop,
    [0xF4] = op_stop,
    [0xF5] = op_stop,
    [0xF6] = op_stop,
    [0xF7] = op_stop,
    [0xF8] = op_stop,
    [0xF9] = op_stop,
    [0xFA] = op_stop,
    [0xFB] = op_stop,
    [0xFC] = op_stop,
    [0xFD] = op_stop,
    [0xFE] = op_stop,
    [0xFF] = op_stop,
};


#endif //OPCODES_H
