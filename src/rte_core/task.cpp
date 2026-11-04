#include "src/rte_core/task.hpp"

namespace {
    constexpr uint32_t EXC_RETURN_THREAD_PSP = 0xFFFFFFFDu; 
    constexpr uint32_t XPSR_THUMB_BIT        = 0x01000000u;

    void trap() {
        while(true);
    }
} // anonymous namespace

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
        _state(TaskState::Ready),
        _stack_base(stack) 
    {
        _stack_base[0] = CANARY;

        const uint32_t top_addr = (reinterpret_cast<uint32_t>(stack) + stack_size_bytes) & ~7u;
        uint32_t *frame = reinterpret_cast<uint32_t*>(top_addr);

        // Hardware-restored frame (popped by the CPU on exception return)
        *--frame = XPSR_THUMB_BIT;                                  // xPSR
        *--frame = reinterpret_cast<uint32_t>(task_fn);             // PC
        *--frame = reinterpret_cast<uint32_t>(trap);                // LR (the task's own LR)
        *--frame = 0u;                                              // R12
        *--frame = 0u;                                              // R3
        *--frame = 0u;                                              // R2
        *--frame = 0u;                                              // R1
        *--frame = 0u;                                              // R0

        // Software-restored by PendSV: EXC_RETURN sits just above R11
        *--frame = EXC_RETURN_THREAD_PSP;                           // saved LR / EXC_RETURN
        *--frame = 0u;                                              // R11
        *--frame = 0u;                                              // R10
        *--frame = 0u;                                              // R9
        *--frame = 0u;                                              // R8
        *--frame = 0u;                                              // R7
        *--frame = 0u;                                              // R6
        *--frame = 0u;                                              // R5
        *--frame = 0u;                                              // R4

        _sp = frame;
    }
}
