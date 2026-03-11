#ifndef X
#define X(...)
#endif

#define NO_ARGS 0
#define READ_BYTE 1
#define READ_INT 4
#define READ_STRING 4
#define READ_CUSTOM -1

#define DISASM_MODE_LITERAL 0
#define DISASM_MODE_I32 1
#define DISASM_MODE_HEX32 2
#define DISASM_MODE_I32_I32 3
#define DISASM_MODE_STRING 4
#define DISASM_MODE_STRING_I32 5
#define DISASM_MODE_HEX32_I32 6
#define DISASM_MODE_CLOSURE 7
#define DISASM_MODE_I32_I32_NOSPACE 8

#define VFY_FIXED 0
#define VFY_SEXP 1
#define VFY_CALL 2
#define VFY_CALLC 3
#define VFY_BARRAY 4
#define VFY_STA 5

//code, name,            func,               args_bytes,             disasm_text,             disasm_fmt,               stack_pop, stack_push, verify_kind)
X(0x01, ADD,             op_add,             NO_ARGS,                "BINOP\t+",              DISASM_MODE_LITERAL,      2, 1, VFY_FIXED)
X(0x02, SUB,             op_sub,             NO_ARGS,                "BINOP\t-",              DISASM_MODE_LITERAL,      2, 1, VFY_FIXED)
X(0x03, MUL,             op_mul,             NO_ARGS,                "BINOP\t*",              DISASM_MODE_LITERAL,      2, 1, VFY_FIXED)
X(0x04, DIV,             op_div,             NO_ARGS,                "BINOP\t/",              DISASM_MODE_LITERAL,      2, 1, VFY_FIXED)
X(0x05, MOD,             op_mod,             NO_ARGS,                "BINOP\t%",              DISASM_MODE_LITERAL,      2, 1, VFY_FIXED)
X(0x06, LT,              op_lt,              NO_ARGS,                "BINOP\t<",              DISASM_MODE_LITERAL,      2, 1, VFY_FIXED)
X(0x07, LE,              op_le,              NO_ARGS,                "BINOP\t<=",             DISASM_MODE_LITERAL,      2, 1, VFY_FIXED)
X(0x08, GT,              op_gt,              NO_ARGS,                "BINOP\t>",              DISASM_MODE_LITERAL,      2, 1, VFY_FIXED)
X(0x09, GE,              op_ge,              NO_ARGS,                "BINOP\t>=",             DISASM_MODE_LITERAL,      2, 1, VFY_FIXED)
X(0x0A, EQ,              op_eq,              NO_ARGS,                "BINOP\t==",             DISASM_MODE_LITERAL,      2, 1, VFY_FIXED)
X(0x0B, NE,              op_ne,              NO_ARGS,                "BINOP\t!=",             DISASM_MODE_LITERAL,      2, 1, VFY_FIXED)
X(0x0C, AND,             op_and,             NO_ARGS,                "BINOP\t&&",             DISASM_MODE_LITERAL,      2, 1, VFY_FIXED)
X(0x0D, OR,              op_or,              NO_ARGS,                "BINOP\t!!",             DISASM_MODE_LITERAL,      2, 1, VFY_FIXED)

X(0x10, CONST,           op_const,           READ_INT,               "CONST\t%d",             DISASM_MODE_I32,          0, 1, VFY_FIXED)
X(0x11, STRING,          op_string,          READ_STRING,            "STRING\t%s",            DISASM_MODE_STRING,       0, 1, VFY_FIXED)
X(0x12, SEXP,            op_sexp,            READ_STRING + READ_INT, "SEXP\t%s %d",           DISASM_MODE_STRING_I32,   0, 1, VFY_SEXP)
X(0x13, STI,             op_sti,             NO_ARGS,                "STI",                   DISASM_MODE_LITERAL,      2, 1, VFY_FIXED)
X(0x14, STA,             op_sta,             NO_ARGS,                "STA",                   DISASM_MODE_LITERAL,      0, 0, VFY_STA)
X(0x15, JMP,             op_jmp,             READ_INT,               "JMP\t0x%.8x",           DISASM_MODE_HEX32,        0, 0, VFY_FIXED)
X(0x16, END,             op_end,             NO_ARGS,                "END",                   DISASM_MODE_LITERAL,      1, 1, VFY_FIXED)
X(0x17, RET,             op_ret,             NO_ARGS,                "RET",                   DISASM_MODE_LITERAL,      1, 1, VFY_FIXED)
X(0x18, DROP,            op_drop,            NO_ARGS,                "DROP",                  DISASM_MODE_LITERAL,      1, 0, VFY_FIXED)
X(0x19, DUP,             op_dup,             NO_ARGS,                "DUP",                   DISASM_MODE_LITERAL,      1, 2, VFY_FIXED)
X(0x1A, SWAP,            op_swap,            NO_ARGS,                "SWAP",                  DISASM_MODE_LITERAL,      2, 2, VFY_FIXED)
X(0x1B, ELEM,            op_elem,            NO_ARGS,                "ELEM",                  DISASM_MODE_LITERAL,      2, 1, VFY_FIXED)

X(0x20, LD_G,            op_ld_g,            READ_INT,               "LD\tG(%d)",             DISASM_MODE_I32,          0, 1, VFY_FIXED)
X(0x21, LD_L,            op_ld_l,            READ_INT,               "LD\tL(%d)",             DISASM_MODE_I32,          0, 1, VFY_FIXED)
X(0x22, LD_A,            op_ld_a,            READ_INT,               "LD\tA(%d)",             DISASM_MODE_I32,          0, 1, VFY_FIXED)
X(0x23, LD_C,            op_ld_c,            READ_INT,               "LD\tC(%d)",             DISASM_MODE_I32,          0, 1, VFY_FIXED)

X(0x30, LDA_G,           op_lda_g,           READ_INT,               "LDA\tG(%d)",            DISASM_MODE_I32,          0, 1, VFY_FIXED)
X(0x31, LDA_L,           op_lda_l,           READ_INT,               "LDA\tL(%d)",            DISASM_MODE_I32,          0, 1, VFY_FIXED)
X(0x32, LDA_A,           op_lda_a,           READ_INT,               "LDA\tA(%d)",            DISASM_MODE_I32,          0, 1, VFY_FIXED)
X(0x33, LDA_C,           op_lda_c,           READ_INT,               "LDA\tC(%d)",            DISASM_MODE_I32,          0, 1, VFY_FIXED)

X(0x40, ST_G,            op_st_g,            READ_INT,               "ST\tG(%d)",             DISASM_MODE_I32,          1, 1, VFY_FIXED)
X(0x41, ST_L,            op_st_l,            READ_INT,               "ST\tL(%d)",             DISASM_MODE_I32,          1, 1, VFY_FIXED)
X(0x42, ST_A,            op_st_a,            READ_INT,               "ST\tA(%d)",             DISASM_MODE_I32,          1, 1, VFY_FIXED)
X(0x43, ST_C,            op_st_c,            READ_INT,               "ST\tC(%d)",             DISASM_MODE_I32,          1, 1, VFY_FIXED)

X(0x50, CJMPz,           op_cjmpz,           READ_INT,               "CJMPz\t0x%.8x",         DISASM_MODE_HEX32,        1, 0, VFY_FIXED)
X(0x51, CJMPnz,          op_cjmpnz,          READ_INT,               "CJMPnz\t0x%.8x",        DISASM_MODE_HEX32,        1, 0, VFY_FIXED)
X(0x52, BEGIN,           op_begin,           READ_INT + READ_INT,    "BEGIN\t%d %d",          DISASM_MODE_I32_I32,      0, 0, VFY_FIXED)
X(0x53, CBEGIN,          op_cbegin,          READ_INT + READ_INT,    "CBEGIN\t%d %d",         DISASM_MODE_I32_I32,      0, 0, VFY_FIXED)
X(0x54, CLOSURE,         op_closure,         READ_CUSTOM,            "CLOSURE\t0x%.8x",       DISASM_MODE_CLOSURE,      0, 1, VFY_FIXED)
X(0x55, CALLC,           op_callc,           READ_INT,               "CALLC\t%d",             DISASM_MODE_I32,          0, 1, VFY_CALLC)
X(0x56, CALL,            op_call,            READ_INT + READ_INT,    "CALL\t0x%.8x %d",       DISASM_MODE_HEX32_I32,    0, 1, VFY_CALL)
X(0x57, TAG,             op_tag,             READ_STRING + READ_INT, "TAG\t%s %d",            DISASM_MODE_STRING_I32,   1, 1, VFY_FIXED)
X(0x58, ARRAY,           op_array,           READ_INT,               "ARRAY\t%d",             DISASM_MODE_I32,          1, 1, VFY_FIXED)
X(0x59, FAIL,            op_fail,            READ_INT + READ_INT,    "FAIL\t%d%d",            DISASM_MODE_I32_I32_NOSPACE, 1, 0, VFY_FIXED)
X(0x5A, LINE,            op_line,            READ_INT,               "LINE\t%d",              DISASM_MODE_I32,          0, 0, VFY_FIXED)

X(0x60, PATT_STR,        op_patt_str,        NO_ARGS,                "PATT\t=str",            DISASM_MODE_LITERAL,      2, 1, VFY_FIXED)
X(0x61, PATT_STRING_TAG, op_patt_string_tag, NO_ARGS,                "PATT\t#string",         DISASM_MODE_LITERAL,      1, 1, VFY_FIXED)
X(0x62, PATT_ARRAY_TAG,  op_patt_array_tag,  NO_ARGS,                "PATT\t#array",          DISASM_MODE_LITERAL,      1, 1, VFY_FIXED)
X(0x63, PATT_SEXP_TAG,   op_patt_sexp_tag,   NO_ARGS,                "PATT\t#sexp",           DISASM_MODE_LITERAL,      1, 1, VFY_FIXED)
X(0x64, PATT_REF,        op_patt_ref,        NO_ARGS,                "PATT\t#ref",            DISASM_MODE_LITERAL,      1, 1, VFY_FIXED)
X(0x65, PATT_VAL,        op_patt_val,        NO_ARGS,                "PATT\t#val",            DISASM_MODE_LITERAL,      1, 1, VFY_FIXED)
X(0x66, PATT_FUN,        op_patt_fun,        NO_ARGS,                "PATT\t#fun",            DISASM_MODE_LITERAL,      1, 1, VFY_FIXED)

X(0x70, LREAD,           op_builtin_lread,   NO_ARGS,                "CALL\tLread",           DISASM_MODE_LITERAL,      0, 1, VFY_FIXED)
X(0x71, LWRITE,          op_builtin_lwrite,  NO_ARGS,                "CALL\tLwrite",          DISASM_MODE_LITERAL,      1, 1, VFY_FIXED)
X(0x72, LLENGTH,         op_builtin_llength, NO_ARGS,                "CALL\tLlength",         DISASM_MODE_LITERAL,      1, 1, VFY_FIXED)
X(0x73, LSTRING,         op_builtin_lstring, NO_ARGS,                "CALL\tLstring",         DISASM_MODE_LITERAL,      1, 1, VFY_FIXED)
X(0x74, BARRAY,          op_builtin_barray,  READ_INT,               "CALL\tBarray\t%d",      DISASM_MODE_I32,          0, 1, VFY_BARRAY)

#undef X