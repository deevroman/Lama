#ifdef X4
    #define X(code, name, func, args_bytes) X4(code, name, func, args_bytes)
#endif

#ifdef X3
    #define X(code, name, func, args_bytes) X3(code, name, func)
#endif

#ifdef X2
    #define X(code, name, func, args_bytes) X2(code, name)
#endif

#ifdef X1
    #define X(code, name, func, args_bytes) X1(code)
#endif

#define NO_ARGS 0
#define READ_BYTE 1
#define READ_INT 4
#define READ_STRING 4
#define READ_CUSTOM -1

X(0x01, ADD,             op_add,             NO_ARGS                )
X(0x02, SUB,             op_sub,             NO_ARGS                )
X(0x03, MUL,             op_mul,             NO_ARGS                )
X(0x04, DIV,             op_div,             NO_ARGS                )
X(0x05, MOD,             op_mod,             NO_ARGS                )
X(0x06, LT,              op_lt,              NO_ARGS                )
X(0x07, LE,              op_le,              NO_ARGS                )
X(0x08, GT,              op_gt,              NO_ARGS                )
X(0x09, GE,              op_ge,              NO_ARGS                )
X(0x0A, EQ,              op_eq,              NO_ARGS                )
X(0x0B, NE,              op_ne,              NO_ARGS                )
X(0x0C, AND,             op_and,             NO_ARGS                )
X(0x0D, OR,              op_or,              NO_ARGS                )

X(0x10, CONST,           op_const,           READ_INT               )
X(0x11, STRING,          op_string,          READ_STRING            )
X(0x12, SEXP,            op_sexp,            READ_STRING + READ_INT )
X(0x13, STI,             op_sti,             NO_ARGS                )
X(0x14, STA,             op_sta,             NO_ARGS                )
X(0x15, JMP,             op_jmp,             READ_INT               )
X(0x16, END,             op_end,             NO_ARGS                )
X(0x17, RET,             op_ret,             NO_ARGS                )
X(0x18, DROP,            op_drop,            NO_ARGS                )
X(0x19, DUP,             op_dup,             NO_ARGS                )
X(0x1A, SWAP,            op_swap,            NO_ARGS                )
X(0x1B, ELEM,            op_elem,            NO_ARGS                )

X(0x20, LD_G,            op_ld_g,            READ_INT               )
X(0x21, LD_L,            op_ld_l,            READ_INT               )
X(0x22, LD_A,            op_ld_a,            READ_INT               )
X(0x23, LD_C,            op_ld_c,            READ_INT               )

X(0x30, LDA_G,           op_lda_g,           READ_INT               )
X(0x31, LDA_L,           op_lda_l,           READ_INT               )
X(0x32, LDA_A,           op_lda_a,           READ_INT               )
X(0x33, LDA_C,           op_lda_c,           READ_INT               )

X(0x40, ST_G,            op_st_g,            READ_INT               )
X(0x41, ST_L,            op_st_l,            READ_INT               )
X(0x42, ST_A,            op_st_a,            READ_INT               )
X(0x43, ST_C,            op_st_c,            READ_INT               )

X(0x50, CJMPz,           op_cjmpz,           READ_INT               )
X(0x51, CJMPnz,          op_cjmpnz,          READ_INT               )
X(0x52, BEGIN,           op_begin,           READ_INT + READ_INT    )
X(0x53, CBEGIN,          op_cbegin,          READ_INT + READ_INT    )
X(0x54, CLOSURE,         op_closure,         READ_CUSTOM            )
X(0x55, CALLC,           op_callc,           READ_INT               )
X(0x56, CALL,            op_call,            READ_INT + READ_INT    )
X(0x57, TAG,             op_tag,             READ_STRING + READ_INT )
X(0x58, ARRAY,           op_array,           READ_INT               )
X(0x59, FAIL,            op_fail,            READ_INT + READ_INT    )
X(0x5A, LINE,            op_line,            READ_INT               )

X(0x60, PATT_STR,        op_patt_str,        NO_ARGS                )
X(0x61, PATT_STRING_TAG, op_patt_string_tag, NO_ARGS                )
X(0x62, PATT_ARRAY_TAG,  op_patt_array_tag,  NO_ARGS                )
X(0x63, PATT_SEXP_TAG,   op_patt_sexp_tag,   NO_ARGS                )
X(0x64, PATT_REF,        op_patt_ref,        NO_ARGS                )
X(0x65, PATT_VAL,        op_patt_val,        NO_ARGS                )
X(0x66, PATT_FUN,        op_patt_fun,        NO_ARGS                )

X(0x70, LREAD,           op_builtin_lread,   NO_ARGS                )
X(0x71, LWRITE,          op_builtin_lwrite,  NO_ARGS                )
X(0x72, LLENGTH,         op_builtin_llength, NO_ARGS                )
X(0x73, LSTRING,         op_builtin_lstring, NO_ARGS                )
X(0x74, BARRAY,          op_builtin_barray,  READ_INT               )

#undef X
#undef X1
#undef X2
#undef X3
#undef X4