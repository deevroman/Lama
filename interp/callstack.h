#ifndef CALLSTACK_H
#define CALLSTACK_H

#include <stddef.h>
#include "runtime_common.h"

typedef char* error_t;
extern char* OK;

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

size_t stack_values_count(void);
void write_stack_value(size_t pos, stack_value v);
stack_value read_stack_value(size_t pos);
void write_raw_stack_value(size_t pos, uint32_t value);
[[nodiscard]] error_t read_raw_stack_value(size_t pos, uint32_t* res);

size_t get_closures_start_position(void);
aint get_closure(void);
size_t get_args_start_position(void);
size_t get_locals_start_position(void);
size_t get_operands_count_pos(void);
void check_stack_capacity();

[[nodiscard]] error_t push_value(stack_value v);
[[nodiscard]] error_t push_raw_value(uint32_t v);
[[nodiscard]] error_t pop_stack_value(stack_value* ret);
stack_value stack_value_from_aint(aint value);
[[nodiscard]] error_t stack_value_to_raw_aint(stack_value v, aint* res);

[[nodiscard]] error_t push_frame(uint32_t ret_addr, uint32_t args_count);
[[nodiscard]] error_t push_closure_frame(aint closure, uint32_t ret_addr, uint32_t args_count);
[[nodiscard]] error_t alloc_locals(uint32_t n);
[[nodiscard]] error_t pop_frame(uint32_t* ret);
[[nodiscard]] error_t push_operand(stack_value value);
[[nodiscard]] error_t push_operand_and_box(aint value);
[[nodiscard]] error_t push_heap_operand(aint* ptr);
[[nodiscard]] error_t pop_operand(stack_value* ret);
[[nodiscard]] error_t pop_n_operands(uint32_t n);
[[nodiscard]] stack_value get_last_operand_ref(uint32_t n);

[[nodiscard]] error_t get_local(uint32_t index, stack_value* ret);
[[nodiscard]] error_t set_local(uint32_t index, stack_value value)
;
[[nodiscard]] error_t get_arg(uint32_t index, stack_value* ret);
[[nodiscard]] error_t set_arg(uint32_t index, stack_value value);

[[nodiscard]] error_t get_glob(uint32_t index, stack_value* ret);
[[nodiscard]] error_t set_glob(uint32_t index, stack_value value);

[[nodiscard]] error_t get_local_addr(uint32_t index, stack_value* ret);
[[nodiscard]] error_t get_arg_addr(uint32_t index, stack_value* ret);
[[nodiscard]] error_t get_glob_addr(uint32_t index, stack_value* ret);

[[nodiscard]] error_t stack_value_to_ref_ptr(stack_value v, void** ret);

void* get_closure_content_ptr(aint c);
aint get_closure_capture_ref(aint clos, uint32_t index);
aint* get_closure_capture_value(aint clos, uint32_t index);
error_t write_by_ref(stack_value ref, stack_value value);

#endif // CALLSTACK_H
