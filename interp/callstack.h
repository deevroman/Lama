#ifndef CALLSTACK_H
#define CALLSTACK_H

typedef char* error_t;
char* OK = "";

#define TRY(expr)                 \
    do {                          \
        error_t err = (expr);     \
        if (err != OK) {          \
            return err;           \
        }                         \
    } while (0)

typedef struct
{
    union
    {
        enum stack_value_tag
        {
            RAW_VALUE = 0,
            HEAP_PTR = 1,
            STACK_REF = 2
        } tag;

        aint __dummy;
    };

    aint value;
} stack_value;

static size_t stack_values_count(void)
{
    return (__gc_stack_bottom - __gc_stack_top) / sizeof(stack_value);
}

static void write_stack_value(size_t pos, stack_value v)
{
    ((stack_value*)__gc_stack_top)[pos] = v;
}

static stack_value read_stack_value(size_t pos)
{
    return ((stack_value*)__gc_stack_top)[pos];
}

static void write_raw_stack_value(size_t pos, uint32_t value)
{
    write_stack_value(pos, (stack_value){.tag = RAW_VALUE, .value = BOX(value)});
}

[[nodiscard]] static error_t read_raw_stack_value(size_t pos, uint32_t* res)
{
    stack_value v = read_stack_value(pos);
    if (v.tag != RAW_VALUE)
    {
        return "Not raw value";
    }
    *res = (uint32_t)UNBOX(v.value);
    return OK;
}

static size_t get_locals_count(void)
{
    uint32_t n;
    if (!stack_frames_counter)
    {
        return 0;
    }
    if (read_raw_stack_value(frame_position, &n) != OK)
    {
        return 0;
    }
    return n;
}

static size_t get_args_count(void)
{
    if (!stack_frames_counter)
    {
        return 0;
    }
    uint32_t args_count;
    if (read_raw_stack_value(frame_position - 2, &args_count) != OK)
    {
        return 0;
    }
    return args_count;
}

static size_t get_closures_start_position(void)
{
    return frame_position - 4;
}

static aint get_closure(void)
{
    if (!stack_frames_counter)
    {
        return BOX(0);
    }
    return read_stack_value(get_closures_start_position()).value;
}

static size_t get_args_start_position(void)
{
    return get_closures_start_position() - get_args_count();
}

static size_t get_locals_start_position(void)
{
    return frame_position + 1;
}

static size_t get_operands_count_pos(void)
{
    return get_locals_start_position() + get_locals_count();
}

static size_t get_operands_count(void)
{
    uint32_t n;
    if (!stack_frames_counter)
    {
        return 0;
    }
    if (read_raw_stack_value(get_operands_count_pos(), &n) != OK)
    {
        return 0;
    }
    return n;
}

static void check_stack_capacity()
{
    size_t used_bytes = __gc_stack_bottom - __gc_stack_top;
    if (used_bytes + sizeof(stack_value) <= stack_capacity)
    {
        return;
    }
    size_t new_capacity = stack_capacity * 2;
    aint* new_stack = realloc((void*)__gc_stack_top, new_capacity);
    if (!new_stack)
    {
        failure("Failed to realloc(..., %zu) call stack\n", new_capacity);
    }

    stack_pointer = (char*)new_stack;
    __gc_stack_top = (size_t)new_stack;
    __gc_stack_bottom = __gc_stack_top + used_bytes;
    stack_capacity = new_capacity;
}

[[nodiscard]] static error_t push_value(stack_value v)
{
    check_stack_capacity();
    write_stack_value(stack_values_count(), v);
    __gc_stack_bottom += sizeof(stack_value);
    return OK;
}

[[nodiscard]] static error_t push_raw_value(uint32_t v)
{
    check_stack_capacity();
    write_stack_value(stack_values_count(), (stack_value){.tag = RAW_VALUE, .value = BOX(v)});
    __gc_stack_bottom += sizeof(stack_value);
    return OK;
}

[[nodiscard]] static error_t pop_stack_value(stack_value* ret)
{
    if (stack_values_count() == 0)
    {
        return "Operand stack underflow";
    }
    __gc_stack_bottom -= sizeof(stack_value);
    if (ret)
    {
        *ret = read_stack_value(stack_values_count());
    }
    return OK;
}


static stack_value stack_value_from_aint(aint value)
{
    if (UNBOXED(value))
    {
        return (stack_value){.tag = RAW_VALUE, .value = value};
    }
    else
    {
        return (stack_value){.tag = HEAP_PTR, .value = value};
    }
}

[[nodiscard]] static error_t stack_value_to_raw_aint(stack_value v, aint* res)
{
    if (v.tag != RAW_VALUE)
    {
        return "Value is not raw value";
    }
    *res = v.value;
    return OK;
}

[[nodiscard]] static error_t push_frame(uint32_t ret_addr, uint32_t args_count)
{
    TRY(push_value(stack_value_from_aint(BOX(0))));
    TRY(push_raw_value(ret_addr));
    TRY(push_raw_value(args_count));
    TRY(push_raw_value(frame_position));
    frame_position = stack_values_count();
    stack_frames_counter++;
    return OK;
}

[[nodiscard]] static error_t push_closure_frame(aint closure, uint32_t ret_addr, uint32_t args_count)
{
    TRY(push_value(stack_value_from_aint(closure)));
    TRY(push_raw_value(ret_addr));
    TRY(push_raw_value(args_count));
    TRY(push_raw_value(frame_position));
    frame_position = stack_values_count();
    stack_frames_counter++;
    return OK;
}

[[nodiscard]] static error_t alloc_locals(uint32_t n)
{
    TRY(push_raw_value(n));
    __gc_stack_bottom += n * sizeof(stack_value);
    TRY(push_raw_value(0));
    return OK;
}


/*
 Stack frame:
    arguments
    closure          fp - 4
    return address   fp - 3
    arguments count  fp - 2
    prev_frame       fp - 1
    locals           fp
    ...
*/

[[nodiscard]] static error_t pop_frame(uint32_t* ret)
{
    if (!stack_frames_counter)
    {
        return "Stack underflow";
    }
    __gc_stack_bottom = __gc_stack_top + frame_position * sizeof(stack_value);
    stack_value prev_fp;
    TRY(pop_stack_value(&prev_fp));
    aint prev_fp_aint;
    TRY(stack_value_to_raw_aint(prev_fp, &prev_fp_aint));
    frame_position = (size_t)UNBOX(prev_fp_aint);
    stack_frames_counter--;

    stack_value args_count;
    TRY(pop_stack_value(&args_count));

    stack_value ret_addr;
    TRY(pop_stack_value(&ret_addr));
    aint ret_addr_aint;
    TRY(stack_value_to_raw_aint(ret_addr, &ret_addr_aint));
    *ret = (uint32_t)UNBOX(ret_addr_aint);

    stack_value closure;
    TRY(pop_stack_value(&closure));

    return OK;
}

[[nodiscard]] static error_t push_operand(stack_value value)
{
    uint32_t n = get_operands_count();
    TRY(push_value(value));
    write_raw_stack_value(get_operands_count_pos(), n + 1);
    return OK;
}

[[nodiscard]] static error_t push_operand_and_box(aint value)
{
    uint32_t n = get_operands_count();
    TRY(push_value((stack_value){.tag = RAW_VALUE, .value = BOX(value)}));
    write_raw_stack_value(get_operands_count_pos(), n + 1);
    return OK;
}

[[nodiscard]] static error_t push_heap_operand(aint* ptr)
{
    uint32_t n = get_operands_count();
    TRY(push_value((stack_value){.tag = HEAP_PTR, .value = (aint)ptr}));
    write_raw_stack_value(get_operands_count_pos(), n + 1);
    return OK;
}

[[nodiscard]] static error_t pop_operand(stack_value* ret)
{
    uint32_t n = get_operands_count();
    if (n == 0)
    {
        return "Operand stack underflow";
    }

    TRY(pop_stack_value(ret));
    write_raw_stack_value(get_operands_count_pos(), n - 1);
    return OK;
}

[[nodiscard]] static error_t pop_n_operands(uint32_t n)
{
    uint32_t operands_count = get_operands_count();
    if (operands_count < n)
    {
        return "Operand stack underflow";
    }

    write_raw_stack_value(get_operands_count_pos(), operands_count - n);
    __gc_stack_bottom -= n * sizeof(stack_value);
    return OK;
}

[[nodiscard]] static stack_value get_last_operand_ref(uint32_t n)
{
    size_t pos = get_operands_count_pos() + 1 + (get_operands_count() - n);
    return (stack_value){.tag = STACK_REF, .value = BOX(pos)};
}

[[nodiscard]] static error_t get_local(uint32_t index, stack_value* ret)
{
    if (index >= get_locals_count())
    {
        return "Local variable index out of range";
    }
    *ret = read_stack_value(get_locals_start_position() + index);
    return OK;
}

[[nodiscard]] static error_t set_local(uint32_t index, stack_value value)
{
    if (index >= get_locals_count())
    {
        return "Local variable index out of range";
    }
    write_stack_value(get_locals_start_position() + index, value);
    return OK;
}

[[nodiscard]] static error_t get_arg(uint32_t index, stack_value* ret)
{
    if (index >= get_args_count())
    {
        return "Argument index out of range";
    }
    *ret = read_stack_value(get_args_start_position() + index);
    return OK;
}

[[nodiscard]] static error_t set_arg(uint32_t index, stack_value value)
{
    if (index >= get_args_count())
    {
        return "Argument index out of range";
    }
    write_stack_value(get_args_start_position() + index, value);
    return OK;
}

[[nodiscard]] static error_t get_glob(uint32_t index, stack_value* ret)
{
    if (index >= bytecode->data->global_area_size)
    {
        return "Global variable index out of range";
    }
    *ret = read_stack_value(index);
    return OK;
}

[[nodiscard]] static error_t set_glob(uint32_t index, stack_value value)
{
    if (index >= bytecode->data->global_area_size)
    {
        return "Global variable index out of range";
    }
    write_stack_value(index, value);
    return OK;
}

[[nodiscard]] static error_t get_local_addr(uint32_t index, stack_value* ret)
{
    if (index >= get_locals_count())
    {
        return "Local variable index out of range";
    }
    *ret = (stack_value){.tag = STACK_REF, .value = BOX(get_locals_start_position() + index)};
    return OK;
}

[[nodiscard]] static error_t get_arg_addr(uint32_t index,
                                          stack_value* ret)
{
    if (index >= get_args_count())
    {
        return "Argument index out of range";
    }
    *ret = (stack_value){.tag = STACK_REF, .value = BOX(get_args_start_position() + index)};
    return OK;
}

[[nodiscard]] static error_t get_glob_addr(uint32_t index, stack_value* ret)
{
    if (index >= bytecode->data->global_area_size)
    {
        return "Global variable index out of range";
    }
    *ret = (stack_value){.tag = STACK_REF, .value = BOX(index)};
    return OK;
}

[[nodiscard]] static error_t stack_value_to_ref_ptr(stack_value v, void** ret)
{
    switch (v.tag)
    {
    case HEAP_PTR:
        *ret = (void*)v.value;
        return OK;
    case STACK_REF:
        size_t pos = (size_t)UNBOX(v.value);
        if (pos * sizeof(stack_value) >= stack_capacity)
        {
            return "Stack overflow";
        }
        *ret = &((aint*)__gc_stack_top)[pos * 2];
        return OK;
    default:
        return "Value is not a reference";
    }
}

static void* get_closure_content_ptr(aint c)
{
    if (UNBOXED(c))
    {
        failure("expected closure, got unboxed\n");
    }
    data* d = TO_DATA((void *)c);
    if (TAG(d->data_header) != CLOSURE_TAG)
    {
        failure("expected closure, got tag=%ld\n", TAG(d->data_header));
    }
    return ((void**)d->contents)[0];
}

static aint get_closure_capture_ref(aint clos, uint32_t index)
{
    if (UNBOXED(clos))
    {
        failure("closure_capture_ref: expected closure, got unboxed\n");
    }
    data* d = TO_DATA((void *)clos);
    if (TAG(d->data_header) != CLOSURE_TAG)
    {
        failure("closure_capture_ref: expected closure, got tag=%ld\n",
                TAG(d->data_header));
    }
    aint len = LEN(d->data_header);
    if ((aint)(index + 1) >= len)
    {
        failure("closure_capture_ref: index %u out of range\n", index);
    }
    return ((aint*)d->contents)[index + 1];
}

static aint* get_closure_capture_value(aint clos, uint32_t index)
{
    if (UNBOXED(clos))
    {
        failure("get_closure_capture_value: expected closure, got unboxed\n");
    }
    data* d = TO_DATA((void *)clos);
    if (TAG(d->data_header) != CLOSURE_TAG)
    {
        failure("get_closure_capture_value: expected closure, got tag=%ld\n", TAG(d->data_header));
    }
    if (index + 1 >= LEN(d->data_header))
    {
        failure("get_closure_capture_value: index %u out of range\n", index);
    }
    return ((aint*)d->contents) + (index + 1);
}

static error_t write_by_ref(stack_value ref, stack_value value)
{
    if (ref.tag == STACK_REF)
    {
        stack_value* v = NULL;
        TRY(stack_value_to_ref_ptr(ref, (void**)&v));
        *v = value;
        return OK;
    }
    aint* p = NULL;
    TRY(stack_value_to_ref_ptr(ref, (void**)&p));
    aint v;
    TRY(stack_value_to_raw_aint(value, &v));
    Bsta(p, (aint)p, (void*)v);
    return OK;
}

#endif // CALLSTACK_H
