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

typedef error_t (*opcode_handler)(void);

#pragma push_macro("TAG")
#undef TAG

const static opcode_handler opcodes[256] = {
#define X(code, name, func, ...) [code] = func,
#include "opcodes.inc"
};

static const char* opcode_names[256] = {
#define X(code, name, ...) [code] = #name,
#include "opcodes.inc"
};

static const int opcode_args_count[256] = {
#define X(code, name, func, args_bytes, ...) [code] = args_bytes,
#include "opcodes.inc"
};

static const char* opcode_disasm_text[256] = {
#define X(code, name, func, args_bytes, disasm_text, ...) [code] = disasm_text,
#include "opcodes.inc"
};

static const int opcode_disasm_mode[256] = {
#define X(code, name, func, args_bytes, disasm_text, disasm_mode, ...) [code] = disasm_mode,
#include "opcodes.inc"
};

static const int opcode_stack_pop[256] = {
#define X(code, name, func, args_bytes, disasm_text, disasm_mode, stack_pop, ...) [code] = stack_pop,
#include "opcodes.inc"
};

static const int opcode_stack_push[256] = {
#define X(code, name, func, args_bytes, disasm_text, disasm_mode, stack_pop, stack_push, ...) [code] = stack_push,
#include "opcodes.inc"
};

static const int opcode_verify_kind[256] = {
#define X(code, name, func, args_bytes, disasm_text, disasm_mode, stack_pop, stack_push, verify_kind) [code] = verify_kind,
#include "opcodes.inc"
};

typedef enum {
#define X(code, name, ...) OPC_##name = code,
#include "opcodes.inc"
} opcode_t;

#pragma pop_macro("TAG")

void interpret_bytecode(bytefile* bf);

#endif //OPCODES_H
