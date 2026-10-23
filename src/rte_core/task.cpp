#include "src/rte_core/task.hpp"

namespace rte {
    Task::Task(
        TaskFn task_fn,
        const char *name, 
        uint32_t *stack,
        uint32_t stack_size_bytes,
        uint8_t priority,
        uint32_t period_ms
    ) : 
        _sp(nullptr),
        _name(name),
        _priority(priority),
        _period_ms(period_ms),
        _next_run_time_ms(0u),
        _wcet_us(0u),
        _state(TaskState::Ready),
        _stack_base(stack) 
    {
        _stack_base[0] = CANARY;

        uint32_t stack_top_idx = stack_size_bytes / sizeof(uint32_t);
        uint32_t *stack_frame_ptr = stack + stack_top_idx;

        *--stack_frame_ptr = 0x01000000u; // xPSR
        *--stack_frame_ptr = reinterpret_cast<uint32_t>(task_fn); // PC
        *--stack_frame_ptr = 0xFFFFFFFDu; // LR
        *--stack_frame_ptr = 0x00000000u; // R12
        *--stack_frame_ptr = 0x00000000u; // R3
        *--stack_frame_ptr = 0x00000000u; // R2
        *--stack_frame_ptr = 0x00000000u; // R1
        *--stack_frame_ptr = 0x00000000u; // R0

        *--stack_frame_ptr = 0x00000000u; // R11
        *--stack_frame_ptr = 0x00000000u; // R10
        *--stack_frame_ptr = 0x00000000u; // R9
        *--stack_frame_ptr = 0x00000000u; // R8
        *--stack_frame_ptr = 0x00000000u; // R7
        *--stack_frame_ptr = 0x00000000u; // R6
        *--stack_frame_ptr = 0x00000000u; // R5
        *--stack_frame_ptr = 0x00000000u; // R4

        _sp = stack_frame_ptr;
    }
}
