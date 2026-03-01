#include "globals.h"
#include "callstack.h"
#include "external.h"
#include "gc.h"
#include "runtime.h"
#include "interpreter.h"

/* Bytecode reading helpers */

size_t file_position_from_ip(const char *ip)
{
    return ip - (char*)bytecode->data;
}

unsigned char read_byte()
{
    if (ip >= bytecode->code_ptr + bytecode->code_size)
    {
        failure("out of bytecode while reading byte at ip=%zu, file_position=%zu\n",
                (void*)(ip - bytecode->code_ptr),
                file_position_from_ip(ip)
        );
    }
    return *ip++;
}

uint32_t read_uint(void)
{
    size_t bytes_to_read = sizeof(uint32_t);
    if (ip + bytes_to_read > bytecode->code_ptr + bytecode->code_size)
    {
        failure("out of bytecode while reading byte at ip=%p, file_position=%p\n",
                (void*)(ip - bytecode->code_ptr),
                file_position_from_ip(ip)
        );
    }
    uint32_t res;
    memcpy(&res, ip, bytes_to_read);
    ip += bytes_to_read;
    return res;
}

uint32_t read_int(void)
{
    size_t bytes_to_read = sizeof(int32_t);
    if (ip + bytes_to_read > bytecode->code_ptr + bytecode->code_size)
    {
        failure("out of bytecode while reading byte at ip=%p, file_position=%p\n",
                (void*)(ip - bytecode->code_ptr),
                file_position_from_ip(ip)
        );
    }
    int32_t res;
    memcpy(&res, ip, bytes_to_read);
    ip += bytes_to_read;
    return res;
}

char* read_string(void)
{
    uint32_t pos = read_uint();
    if (pos >= bytecode->data->stringtab_size)
    {
        failure("invalid string table index: %d (table_size=%d), file_position_of_index=%p\n",
                pos,
                bytecode->data->stringtab_size,
                file_position_from_ip(ip - sizeof(uint32_t))
        );
    }
    return &bytecode->string_ptr[pos];
}

/* Opcode handlers */
#define DEFINE_BINOP(NAME, OP_SYMBOL, OP_EXPR)          \
error_t NAME(void)                               \
{                                                       \
    stack_value b_val, a_val;                           \
    TRY(pop_operand(&b_val));                           \
    TRY(pop_operand(&a_val));                           \
                                                        \
    aint b = UNBOX(b_val.value);                        \
    aint a = UNBOX(a_val.value);                        \
                                                        \
    DEBUG_LOG("[EXEC] BINOP %s: %ld %s %ld\n",          \
              OP_SYMBOL, a, OP_SYMBOL, b);              \
                                                        \
    aint res = (OP_EXPR);                               \
                                                        \
    DEBUG_LOG("[EXEC]   -> result: %ld\n", res);        \
    TRY(push_operand_and_box(res));                     \
    return OK;                                          \
}

DEFINE_BINOP(op_add, "+", a + b)
DEFINE_BINOP(op_sub, "-", a - b)
DEFINE_BINOP(op_mul, "*", a * b)

DEFINE_BINOP(op_lt, "<", a < b)
DEFINE_BINOP(op_le, "<=", a <= b)
DEFINE_BINOP(op_gt, ">", a > b)
DEFINE_BINOP(op_ge, ">=", a >= b)
DEFINE_BINOP(op_eq, "==", a == b)
DEFINE_BINOP(op_ne, "!=", a != b)

DEFINE_BINOP(op_and, "&&", a && b)
DEFINE_BINOP(op_or, "||", a || b)

#define DEFINE_BINOP_CHECK(NAME, OP_SYMBOL, CHECK, OP_EXPR) \
error_t NAME(void)                                    \
{                                                            \
    stack_value b_val, a_val;                                \
    TRY(pop_operand(&b_val));                                \
    TRY(pop_operand(&a_val));                                \
                                                             \
    aint b = UNBOX(b_val.value);                             \
    aint a = UNBOX(a_val.value);                             \
                                                             \
    DEBUG_LOG("[EXEC] BINOP %s: %ld %s %ld\n",               \
              OP_SYMBOL, a, OP_SYMBOL, b);                   \
                                                             \
    CHECK;                                                   \
                                                             \
    aint res = (OP_EXPR);                                    \
                                                             \
    DEBUG_LOG("[EXEC]   -> result: %ld\n", res);             \
    TRY(push_operand_and_box(res));                          \
    return OK;                                               \
}

DEFINE_BINOP_CHECK(op_div, "/",
                   if (b == 0) failure("Division by zero\n");,
                   a / b)

DEFINE_BINOP_CHECK(op_mod, "%%",
                   if (b == 0) failure("Modulo by zero\n");,
                   a % b)

error_t op_const(void)
{
    int32_t value = read_int();
    DEBUG_LOG("[EXEC] CONST %d\n", value);
    TRY(push_operand((stack_value){.tag = RAW_VALUE, .value = BOX(value)}));
    return OK;
}

error_t op_ld_g(void)
{
    uint32_t index = read_uint();
    DEBUG_LOG("[EXEC] LD GLOBAL [%d]\n", index);
    stack_value val;
    TRY(get_glob(index, &val));
    DEBUG_LOG("[EXEC]   -> value: %ld\n", UNBOX(val.value));
    TRY(push_operand(val));
    return OK;
}

error_t op_st_g(void)
{
    uint32_t index = read_uint();
    DEBUG_LOG("[EXEC] ST GLOBAL [%d]\n", index);
    stack_value value;
    TRY(pop_operand(&value));
    DEBUG_LOG("[EXEC]   -> storing: %ld\n", UNBOX(value.value));
    TRY(set_glob(index, value));
    TRY(push_operand(value));
    return OK;
}

error_t op_drop(void)
{
    DEBUG_LOG("[EXEC] DROP\n");
    stack_value val;
    TRY(pop_operand(&val));
    return OK;
}

error_t op_dup(void)
{
    DEBUG_LOG("[EXEC] DUP\n");
    stack_value value;
    TRY(pop_operand(&value));
    TRY(push_operand(value));
    TRY(push_operand(value));
    return OK;
}

error_t op_swap(void)
{
    DEBUG_LOG("[EXEC] SWAP\n");
    stack_value a, b;
    TRY(pop_operand(&a));
    TRY(pop_operand(&b));
    TRY(push_operand(a));
    TRY(push_operand(b));
    return OK;
}

[[nodiscard]] error_t safe_jmp(uint32_t offset)
{
    if (offset >= bytecode->code_size)
    {
        fprintf(stderr, "jump offset out of range: %d(code size=%lu) at ip=%p\n",
                offset,
                bytecode->code_size,
                (void*)(ip - bytecode->code_ptr));
        return "Jump offset out of range";
    }
    ip = bytecode->code_ptr + offset;
    return OK;
}

error_t op_jmp(void)
{
    uint32_t offset = read_uint();
    DEBUG_LOG("[EXEC] OP1 JMP offset=%d\n", offset);
    TRY(safe_jmp(offset));
    return OK;
}

error_t op_end(void)
{
    DEBUG_LOG("[EXEC] END\n");
    size_t args_count = current_args_count;
    stack_value callee_ret;
    TRY(pop_operand(&callee_ret));
    uint32_t ret_addr;
    TRY(pop_frame(&ret_addr));
    if (stack_frames_counter == 0)
    {
        running = 0;
        return OK;
    }

    TRY(pop_n_operands(args_count));
    TRY(push_operand(callee_ret));
    ip = bytecode->code_ptr + ret_addr;
    return OK;
}

error_t op_builtin_lread(void)
{
    DEBUG_LOG("[EXEC] BUILTIN LREAD\n");
    aint value = Lread();
    // aint value = BOX(0);
    TRY(push_operand(stack_value_from_aint(value)));
    return OK;
}

[[nodiscard]] error_t op_builtin_lwrite(void)
{
    DEBUG_LOG("[EXEC] BUILTIN LWRITE\n");
    stack_value value;
    TRY(pop_operand(&value));
    Lwrite(value.value);
    TRY(push_operand(value));
    return OK;
}

error_t op_line(void)
{
    uint32_t line_num = read_uint();
    DEBUG_LOG("[EXEC] OP5 LINE %d\n", line_num);
    return OK;
}

error_t op_begin(void)
{
    uint32_t args_count = read_uint();
    uint32_t locals_count = read_uint();
    DEBUG_LOG("[EXEC] OP5 BEGIN args_count=%d locals_count=%d\n", args_count, locals_count);
    if (args_count != current_args_count)
    {
        failure("BEGIN: args_count mismatch\n");
    }
    TRY(alloc_locals(locals_count));
    return OK;
}

#define DEFINE_CJMP(name, jump_cond)                             \
error_t name(void)                                        \
{                                                                \
    uint32_t offset = read_uint();                               \
    stack_value cond_val;                                        \
    TRY(pop_operand(&cond_val));                                 \
    aint cond = cond_val.value;                                  \
                                                                 \
    DEBUG_LOG("[EXEC] OP5 %s offset=%d, condition=%ld\n",        \
              #name, offset, UNBOX(cond));                       \
                                                                 \
    if (jump_cond)                                               \
    {                                                            \
        DEBUG_LOG("[EXEC]   -> jumping to offset %d\n", offset); \
        TRY(safe_jmp(offset));                                   \
    }                                                            \
    else                                                         \
    {                                                            \
        DEBUG_LOG("[EXEC]   -> not jumping\n");                  \
    }                                                            \
                                                                 \
    return OK;                                                   \
}

DEFINE_CJMP(op_cjmpz, !UNBOX(cond))
DEFINE_CJMP(op_cjmpnz, UNBOX(cond))

error_t op_patt_str(void)
{
    DEBUG_LOG("[EXEC] PATT STR\n");

    stack_value rhs_val, lhs_val;
    TRY(pop_operand(&rhs_val));
    TRY(pop_operand(&lhs_val));

    aint rhs = rhs_val.value;
    aint lhs = lhs_val.value;

    aint r = Bstring_patt((void*)lhs, (void*)rhs);

    DEBUG_LOG("[EXEC]   -> result: %ld\n", UNBOX(r));

    TRY(push_operand(stack_value_from_aint(r)));
    return OK;
}

error_t op_patt_string_tag(void)
{
    DEBUG_LOG("[EXEC] PATT STRING_TAG\n");

    stack_value v_val;
    TRY(pop_operand(&v_val));

    aint v = v_val.value;
    aint r = Bstring_tag_patt((void*)v);

    DEBUG_LOG("[EXEC]   -> result: %ld\n", UNBOX(r));

    TRY(push_operand(stack_value_from_aint(r)));
    return OK;
}

error_t op_patt_array_tag(void)
{
    DEBUG_LOG("[EXEC] PATT ARRAY_TAG\n");

    stack_value v_val;
    TRY(pop_operand(&v_val));

    aint v = v_val.value;
    aint r = Barray_tag_patt((void*)v);

    DEBUG_LOG("[EXEC]   -> result: %ld\n", UNBOX(r));

    TRY(push_operand(stack_value_from_aint(r)));
    return OK;
}

error_t op_patt_sexp_tag(void)
{
    DEBUG_LOG("[EXEC] PATT SEXP_TAG\n");

    stack_value v_val;
    TRY(pop_operand(&v_val));

    aint v = v_val.value;
    aint r = Bsexp_tag_patt((void*)v);

    DEBUG_LOG("[EXEC]   -> result: %ld\n", UNBOX(r));

    TRY(push_operand(stack_value_from_aint(r)));
    return OK;
}

error_t op_patt_fun(void)
{
    DEBUG_LOG("[EXEC] PATT FUN\n");

    stack_value v_val;
    TRY(pop_operand(&v_val));

    aint v = v_val.value;
    aint r = Bclosure_tag_patt((void*)v);

    DEBUG_LOG("[EXEC]   -> result: %ld\n", UNBOX(r));

    TRY(push_operand(stack_value_from_aint(r)));
    return OK;
}

error_t op_patt_ref(void)
{
    DEBUG_LOG("[EXEC] PATT REF\n");

    stack_value v_val;
    TRY(pop_operand(&v_val));

    aint r = (v_val.tag == STACK_REF) ? BOX(1) : BOX(0);

    DEBUG_LOG("[EXEC]   -> result: %ld\n", UNBOX(r));

    TRY(push_operand(stack_value_from_aint(r)));
    return OK;
}

error_t op_patt_val(void)
{
    DEBUG_LOG("[EXEC] PATT VAL\n");

    stack_value v_val;
    TRY(pop_operand(&v_val));

    aint r = (v_val.tag == RAW_VALUE) ? BOX(1) : BOX(0);

    DEBUG_LOG("[EXEC]   -> result: %ld\n", UNBOX(r));

    TRY(push_operand(stack_value_from_aint(r)));
    return OK;
}


error_t op_string(void)
{
    char* str = read_string();
    DEBUG_LOG("[EXEC] STRING\n");
    TRY(push_heap_operand((aint*)Bstring((aint*)&str)));
    return OK;
}

error_t op_sexp(void)
{
    char* tag = read_string();
    uint32_t arity = read_uint();
    DEBUG_LOG("[EXEC] SEXP tag=%s arity=%u\n", tag, arity);

    aint t = LtagHash(tag);
    stack_value args_ref = get_last_operand_ref(arity);
    stack_value* values = NULL;
    TRY(stack_value_to_ref_ptr(args_ref, (void**)&values));

    sexp* r = alloc_sexp(arity);
    for (int i = 0; i < arity; i++)
    {
        ((auint*)r->contents)[i] = values[i].value;
    }
    r->tag = UNBOX(t);

    TRY(pop_n_operands(arity));
    TRY(push_heap_operand((aint*)((data*)r)->contents));
    return OK;
}

error_t op_sti(void)
{
    DEBUG_LOG("[EXEC] STI\n");

    stack_value val, ref;
    TRY(pop_operand(&val));
    TRY(pop_operand(&ref));

    write_by_ref(ref, val);
    TRY(push_operand(val));
    return OK;
}

error_t op_sta(void)
{
    DEBUG_LOG("[EXEC] STA\n");

    stack_value val, sec_op;
    TRY(pop_operand(&val));
    TRY(pop_operand(&sec_op));

    if (sec_op.tag == RAW_VALUE)
    {
        stack_value agg;
        TRY(pop_operand(&agg));
        aint* agg_val;
        aint index_val;
        aint val_aint;
        TRY(stack_value_to_ref_ptr(agg, (void**)&agg_val));
        TRY(stack_value_to_raw_aint(sec_op, &index_val));
        TRY(stack_value_to_raw_aint(val, &val_aint));
        Bsta(agg_val, index_val, (void*)val_aint);
    }
    else
    {
        write_by_ref(sec_op, val);
    }

    TRY(push_operand(val));
    return OK;
}

error_t op_elem(void)
{
    DEBUG_LOG("[EXEC] ELEM\n");

    stack_value index_val, agg_val;
    TRY(pop_operand(&index_val));
    TRY(pop_operand(&agg_val));

    aint index, agg;
    TRY(stack_value_to_raw_aint(index_val, &index));
    agg = agg_val.value;

    void* r = Belem((void*)agg, index);
    TRY(push_operand(stack_value_from_aint((aint)r)));
    return OK;
}

error_t op_ret(void)
{
    DEBUG_LOG("[EXEC] RET\n");
    stack_value callee_ret;
    size_t args_count = current_args_count;
    TRY(pop_operand(&callee_ret));
    uint32_t ret_addr;
    TRY(pop_frame(&ret_addr));

    if (stack_frames_counter == 0)
    {
        return "RET with no frames";
    }

    TRY(pop_n_operands(args_count));

    TRY(push_operand(callee_ret));
    ip = bytecode->code_ptr + ret_addr;
    return OK;
}

error_t op_ld_l(void)
{
    uint32_t index = read_uint();
    DEBUG_LOG("[EXEC] LD LOCAL [%d]\n", index);
    stack_value val;
    TRY(get_local(index, &val));
    TRY(push_operand(val));
    return OK;
}

error_t op_ld_a(void)
{
    uint32_t index = read_uint();
    DEBUG_LOG("[EXEC] LD ARG [%d]\n", index);
    stack_value val;
    TRY(get_arg(index, &val));
    TRY(push_operand(val));
    return OK;
}

error_t op_ld_c(void)
{
    uint32_t index = read_uint();
    DEBUG_LOG("[EXEC] LD CLOSURE [%d]\n", index);
    aint clos = get_closure();
    if (UNBOXED(clos) && UNBOX(clos) == 0)
    {
        failure("LD C: no closure in current frame\n");
    }
    TRY(push_operand(stack_value_from_aint(get_closure_capture_ref(clos, (uint32_t)index))));
    return OK;
}

error_t op_lda_g(void)
{
    uint32_t index = read_uint();
    DEBUG_LOG("[EXEC] LDA GLOBAL [%d]\n", index);
    stack_value ref;
    TRY(get_glob_addr(index, &ref));
    TRY(push_operand(ref));
    return OK;
}

error_t op_lda_l(void)
{
    uint32_t index = read_uint();
    DEBUG_LOG("[EXEC] LDA LOCAL [%d]\n", index);
    stack_value ref;
    TRY(get_local_addr(index, &ref));
    TRY(push_operand(ref));
    return OK;
}

error_t op_lda_a(void)
{
    uint32_t index = read_uint();
    DEBUG_LOG("[EXEC] LDA ARG [%d]\n", index);
    stack_value ref;
    TRY(get_arg_addr(index, &ref));
    TRY(push_operand(ref));
    return OK;
}

error_t op_lda_c(void)
{
    uint32_t index = read_uint();
    DEBUG_LOG("[EXEC] LDA CLOSURE [%d]\n", index);
    aint clos = get_closure();
    if (UNBOXED(clos) && UNBOX(clos) == 0)
    {
        failure("LDA C: no closure in current frame\n");
    }
    TRY(push_heap_operand(get_closure_capture_value(clos, (uint32_t)index)));
    return OK;
}

error_t op_st_l(void)
{
    uint32_t index = read_uint();
    DEBUG_LOG("[EXEC] ST LOCAL [%d]\n", index);
    stack_value val;
    TRY(pop_operand(&val));
    TRY(set_local(index, val));
    TRY(push_operand(val));
    return OK;
}

error_t op_st_a(void)
{
    uint32_t index = read_uint();
    DEBUG_LOG("[EXEC] ST ARG [%d]\n", index);
    stack_value val;
    TRY(pop_operand(&val));
    TRY(set_arg(index, val));
    TRY(push_operand(val));
    return OK;
}

error_t op_st_c(void)
{
    uint32_t index = read_uint();
    DEBUG_LOG("[EXEC] ST CLOSURE [%d]\n", index);
    stack_value val;
    TRY(pop_operand(&val));
    aint a = val.value;

    aint clos = get_closure();
    if (UNBOXED(clos) && UNBOX(clos) == 0)
    {
        failure("ST C: no closure in current frame\n");
    }

    *get_closure_capture_value(clos, (uint32_t)index) = a;
    TRY(push_operand(val));
    return OK;
}

error_t op_cbegin(void)
{
    uint32_t args_count = read_uint();
    uint32_t locals_count = read_uint();
    DEBUG_LOG("[EXEC] OP5 CBEGIN args_count=%d locals_count=%d\n", args_count, locals_count);
    if (args_count != current_args_count)
    {
        failure("CBEGIN: args_count mismatch\n");
    }
    TRY(alloc_locals(locals_count));
    return OK;
}

error_t op_closure(void)
{
    int32_t offset = read_uint();
    uint32_t n = read_uint();
    DEBUG_LOG("[EXEC] OP5 CLOSURE offset=%d n=%d\n", offset, n);

    data* r = alloc_closure(n + 1);
    ((void**)r->contents)[0] = (void*)(bytecode->code_ptr + offset);

    for (int i = 0; i < n; i++)
    {
        char capture_type = read_byte();
        uint32_t capture_index = read_uint();

        stack_value v;
        switch (capture_type)
        {
        case 0:
            TRY(get_glob(capture_index, &v));
            ((aint*)r->contents)[i + 1] = v.value;
            break;
        case 1:
            TRY(get_local(capture_index, &v));
            ((aint*)r->contents)[i + 1] = v.value;
            break;
        case 2:
            TRY(get_arg(capture_index, &v));
            ((aint*)r->contents)[i + 1] = v.value;
            break;
        case 3:
            aint clos = get_closure();
            if (UNBOXED(clos) && UNBOX(clos) == 0)
            {
                failure("CLOSURE: capture C but no closure in current frame\n");
            }
            ((aint*)r->contents)[i + 1] = get_closure_capture_ref(clos, capture_index);
            break;
        default:
            failure("Invalid closure capture type: %d\n", capture_type);
        }
    }
    TRY(push_heap_operand((aint*)r->contents));
    return OK;
}

error_t op_callc(void)
{
    uint32_t n = read_uint();
    DEBUG_LOG("[EXEC] OP5 CALLC n=%d\n", n);

    uint32_t ret_addr = ip - bytecode->code_ptr;
    stack_value clos_ref = get_last_operand_ref(n + 1);
    stack_value* clos_ptr = NULL;
    TRY(stack_value_to_ref_ptr(clos_ref, (void**)&clos_ptr));
    aint clos = clos_ptr->value;
    memmove(clos_ptr, clos_ptr + 1, n * sizeof(stack_value));
    TRY(pop_operand(NULL));
    void* entry = get_closure_content_ptr(clos);
    TRY(push_closure_frame(clos, ret_addr, (uint32_t)n));
    ip = (char*)entry;
    return OK;
}

error_t op_call(void)
{
    uint32_t offset = read_uint();
    uint32_t args_count = read_uint();
    DEBUG_LOG("[EXEC] OP5 CALL offset=%d args_count=%d\n", offset, args_count);
    TRY(push_frame(ip - bytecode->code_ptr, args_count));
    TRY(safe_jmp(offset));
    return OK;
}

error_t op_tag(void)
{
    char* tag = read_string();
    uint32_t arity = read_uint();
    DEBUG_LOG("[EXEC] OP5 TAG tag=%s arity=%u\n", tag, arity);

    stack_value p_val;
    TRY(pop_operand(&p_val));
    aint p = p_val.value;
    aint t = LtagHash(tag);
    aint an = BOX(arity);

    if (arity == 0 && UNBOXED(p))
    {
        if ((UNBOX(p) == UNBOX(t)))
        {
            TRY(push_operand(stack_value_from_aint(BOX(1))));
        }
        else
        {
            TRY(push_operand(stack_value_from_aint(BOX(0))));
        }
    }
    else
    {
        TRY(push_operand(stack_value_from_aint(Btag((void*)p, t, an))));
    }
    return OK;
}

error_t op_array(void)
{
    uint32_t size = read_uint();
    DEBUG_LOG("[EXEC] OP5 ARRAY size=%u\n", size);

    stack_value p_val;
    TRY(pop_operand(&p_val));
    aint r = Barray_patt((void*)p_val.value, BOX(size));
    TRY(push_operand(stack_value_from_aint(r)));
    return OK;
}

error_t op_fail(void)
{
    uint32_t line = read_uint();
    uint32_t col = read_uint();
    DEBUG_LOG("[EXEC] OP5 FAIL line=%u col=%d\n", line, col);

    stack_value p_val;
    TRY(pop_operand(&p_val));
    Bmatch_failure((void*)p_val.value, "main", BOX(line), BOX(col));
    return OK;
}

error_t op_builtin_llength(void)
{
    DEBUG_LOG("[EXEC] BUILTIN LLENGTH\n");

    stack_value p_val;
    TRY(pop_operand(&p_val));
    aint p = p_val.value;
    if (UNBOXED(p))
    {
        failure("Llength: expected reference, got unboxed value (%p)\n", p);
    }
    TRY(push_operand(stack_value_from_aint(Llength((void*)p))));
    return OK;
}

error_t op_builtin_lstring(void)
{
    DEBUG_LOG("[EXEC] BUILTIN LSTRING\n");

    stack_value p_val;
    TRY(pop_operand(&p_val));
    TRY(push_heap_operand(Lstring(&p_val.value)));
    return OK;
}

error_t op_builtin_barray(void)
{
    uint32_t size = read_uint();
    DEBUG_LOG("[EXEC] BUILTIN BARRAY size=%d\n", size);

    if (size == 0)
    {
        TRY(push_heap_operand(Barray(NULL, BOX(0))));
        return OK;
    }

    stack_value args_ref = get_last_operand_ref(size);
    stack_value* values = NULL;
    TRY(stack_value_to_ref_ptr(args_ref, (void**)&values));

    data* r = alloc_array(size);
    for (uint32_t i = 0; i < size; i++)
    {
        ((aint*)r->contents)[i] = values[i].value;
    }

    TRY(pop_n_operands(size));
    TRY(push_heap_operand((aint*)r->contents));
    return OK;
}

error_t op_stop(void)
{
    running = 0;
    return OK;
}


void loop_with_table()
{
    int instr_count = 0;
    running = 1;
    while (running)
    {
        unsigned char x = read_byte();
        DEBUG_LOG("[DEBUG] Instr #%d: 0x%02x\n", instr_count++, x);

        const opcode_handler op_handler = opcodes[x];
        if (op_handler)
        {
            error_t res = op_handler();
            if (res != OK)
            {
                failure("Error executing opcode: 0x%02x file_position=%zu. Error: %s\n", x, file_position_from_ip(ip), res);
            }
        }
        else
        {
            failure("No op_handler for opcode: 0x%02x file_position=%zu\n", x, file_position_from_ip(ip));
        }
    }

}

void loop_with_switch()
{
    int instr_count = 0;
    running = 1;
    while (running)
    {
        unsigned char x = read_byte();
        DEBUG_LOG("[DEBUG] Instr #%d: 0x%02x\n", instr_count++, x);

        error_t res;

        switch (x)
        {
        case 0x01: res = op_add();
            break;
        case 0x02: res = op_sub();
            break;
        case 0x03: res = op_mul();
            break;
        case 0x04: res = op_div();
            break;
        case 0x05: res = op_mod();
            break;
        case 0x06: res = op_lt();
            break;
        case 0x07: res = op_le();
            break;
        case 0x08: res = op_gt();
            break;
        case 0x09: res = op_ge();
            break;
        case 0x0A: res = op_eq();
            break;
        case 0x0B: res = op_ne();
            break;
        case 0x0C: res = op_and();
            break;
        case 0x0D: res = op_or();
            break;

        case 0x10: res = op_const();
            break;
        case 0x11: res = op_string();
            break;
        case 0x12: res = op_sexp();
            break;
        case 0x13: res = op_sti();
            break;
        case 0x14: res = op_sta();
            break;
        case 0x15: res = op_jmp();
            break;
        case 0x16: res = op_end();
            break;
        case 0x17: res = op_ret();
            break;
        case 0x18: res = op_drop();
            break;
        case 0x19: res = op_dup();
            break;
        case 0x1A: res = op_swap();
            break;
        case 0x1B: res = op_elem();
            break;

        case 0x20: res = op_ld_g();
            break;
        case 0x21: res = op_ld_l();
            break;
        case 0x22: res = op_ld_a();
            break;
        case 0x23: res = op_ld_c();
            break;

        case 0x30: res = op_lda_g();
            break;
        case 0x31: res = op_lda_l();
            break;
        case 0x32: res = op_lda_a();
            break;
        case 0x33: res = op_lda_c();
            break;

        case 0x40: res = op_st_g();
            break;
        case 0x41: res = op_st_l();
            break;
        case 0x42: res = op_st_a();
            break;
        case 0x43: res = op_st_c();
            break;

        case 0x50: res = op_cjmpz();
            break;
        case 0x51: res = op_cjmpnz();
            break;
        case 0x52: res = op_begin();
            break;
        case 0x53: res = op_cbegin();
            break;
        case 0x54: res = op_closure();
            break;
        case 0x55: res = op_callc();
            break;
        case 0x56: res = op_call();
            break;
        case 0x57: res = op_tag();
            break;
        case 0x58: res = op_array();
            break;
        case 0x59: res = op_fail();
            break;
        case 0x5A: res = op_line();
            break;

        case 0x60: res = op_patt_str();
            break;
        case 0x61: res = op_patt_string_tag();
            break;
        case 0x62: res = op_patt_array_tag();
            break;
        case 0x63: res = op_patt_sexp_tag();
            break;
        case 0x64: res = op_patt_ref();
            break;
        case 0x65: res = op_patt_val();
            break;
        case 0x66: res = op_patt_fun();
            break;

        case 0x70: res = op_builtin_lread();
            break;
        case 0x71: res = op_builtin_lwrite();
            break;
        case 0x72: res = op_builtin_llength();
            break;
        case 0x73: res = op_builtin_lstring();
            break;
        case 0x74: res = op_builtin_barray();
            break;

        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:
            res = op_stop();
            break;

        default:
            failure("No op_handler for opcode: 0x%02x file_position=%zu\n",
                    x, file_position_from_ip(ip));
        }

        if (res != OK)
        {
            failure("Error executing opcode: 0x%02x file_position=%zu. Error: %s\n",
                    x, file_position_from_ip(ip), res);
        }
    }
}

void interpret_bytecode(bytefile* bf)
{
    DEBUG_LOG("[DEBUG] Calling __gc_init...\n");
    __gc_init();
    DEBUG_LOG("[DEBUG] __gc_init completed\n");

    bytecode = bf;
    ip = bf->code_ptr;

    DEBUG_LOG("[DEBUG] Interpreter starting...\n");
    DEBUG_LOG("[DEBUG] Code size: %zu bytes\n", bf->code_size);
    DEBUG_LOG("[DEBUG] Global area size: %d\n", bf->data->global_area_size);

    stack_capacity = 16 * 1024;
    stack_pointer = malloc(stack_capacity);
    if (!stack_pointer)
    {
        failure("Failed to allocate call stack\n");
    }

    frame_position = bf->data->global_area_size;
    __gc_stack_top = (size_t)stack_pointer;
    __gc_stack_bottom = __gc_stack_top + frame_position * sizeof(stack_value);

    stack_frames_counter = 0;
    if (push_frame(0, 2) != OK)
    {
        failure("Failed to push initial call frame");
    }

    loop_with_table();
    //loop_with_switch();

    free(stack_pointer);
}
