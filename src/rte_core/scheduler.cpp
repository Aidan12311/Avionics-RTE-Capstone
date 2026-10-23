#include <cstdint>

#include "src/hal/uart.hpp"
#include "src/hal/arm_core.hpp"
#include "src/rte_core/task.hpp"
#include "src/rte_core/scheduler.hpp"

using rte::Task;

extern "C" {
    Task *current_task = nullptr;
    Task *next_task = nullptr;
}

namespace {
    // ── SysTick registers ─────────────────────────────────────────────
    // Same on every Cortex-M chip — ARM core registers, not STM32-specific
    // Source: ARM Cortex-M4 TRM (DDI0439C)
    constexpr uint32_t SYST_CSR   = 0xE000E010u;  // control and status
    constexpr uint32_t SYST_RVR   = 0xE000E014u;  // reload value
    constexpr uint32_t SYST_CVR   = 0xE000E018u;  // current value

    // ── NVIC ──────────────────────────────────────────────────────────
    constexpr uint32_t NVIC_SHPR3 = 0xE000ED20u;  // system handler priority 3

    // ── SCB ICSR ──────────────────────────────────────────────────────
    constexpr uint32_t SCB_ICSR   = 0xE000ED04u;  // interrupt control state
    constexpr uint32_t PENDSVSET  = (1u << 28);   // bit 28 pends PendSV

    // ── SysTick CSR bits ──────────────────────────────────────────────
    constexpr uint32_t SYST_ENABLE    = (1u << 0);
    constexpr uint32_t SYST_TICKINT   = (1u << 1);
    constexpr uint32_t SYST_CLKSOURCE = (1u << 2);  // 1 = processor clock

    // ── SysTick reload for 1ms tick at 16MHz HSI ──────────────────────
    // reload = (clock / tick_rate) - 1 = (16,000,000 / 1,000) - 1 = 15,999
    constexpr uint32_t SYSTICK_RELOAD = 15999u;

    Task *tasks[scheduler::MAX_TASKS]{};
    uint8_t task_count = 0u;

    volatile uint32_t systick_count = 0u;

    void check_canaries() {
        for(uint8_t i = 0; i < task_count; ++i) {
            if(!tasks[i]->is_canary_ok()) {
                uart::println("Stack overflow detected in task: ");
                uart::println(tasks[i]->get_name());
                uart::put_char('\n');
                
                while (true);
            }
        }
    }

    Task *get_next_task() {
        Task *next_task_candidate = nullptr;
        
        for(uint8_t i = 0u; i < task_count; ++i) {
            Task *curr_task = tasks[i];
            if(curr_task->is_ready()) {
                if(!next_task_candidate || curr_task->get_priority() > next_task_candidate->get_priority()) {
                    next_task_candidate = curr_task;
                }
            }
        }

        return next_task_candidate;
    }
} // anonymous namespace

void scheduler::add_task(Task *task) {
    if(task_count >= scheduler::MAX_TASKS) {
        uart::println("Error: Maximum number of tasks reached.");
        return;
    }

    tasks[task_count++] = task;
}

void scheduler::task_block_until_next_period() {
    arm::disable_irq();

    current_task->set_state(rte::TaskState::Blocked);
    current_task->set_next_run_time_ms(
        current_task->get_next_run_time_ms() + current_task->get_period_ms()
    );

    Task *next_task_candidate = get_next_task();
    if(next_task_candidate && next_task_candidate != current_task) {
        next_task_candidate->set_state(rte::TaskState::Running);
        next_task = next_task_candidate;
    }

    reg(SCB_ICSR) |= PENDSVSET;

    arm::enable_irq();
}

void scheduler::start() {
    reg(NVIC_SHPR3) |= (0xFFU << 16);
    reg(SYST_RVR) = SYSTICK_RELOAD;
    reg(SYST_CVR) = 0u;
    reg(SYST_CSR) = SYST_CLKSOURCE | SYST_TICKINT | SYST_ENABLE;

    Task *first_task = get_next_task();
    if(!first_task) {
        uart::println("Error: No tasks to schedule! Make sure to add tasks before starting the scheduler!");
        while(1);
    }

    first_task->set_state(rte::TaskState::Running);
    current_task = nullptr;
    next_task = first_task;


    __asm__ volatile (
        // Set PSP to any valid stack location (we use MSP value for now)
        // The actual PSP will be set by pendsv_handler from next_task->_sp
        "mrs r0, msp        \n"
        "msr psp, r0        \n"

        // Switch Thread mode to PSP
        "mrs r0, control    \n"
        "orr r0, r0, #2     \n"
        "msr control, r0    \n"
        "isb                \n"

        // Pend PendSV and enable interrupts
        "ldr r0, =%[icsr]   \n"
        "ldr r1, =%[psvset] \n"
        "str r1, [r0]       \n"
        "cpsie i            \n"
        "b .                \n"

        :
        : [icsr]   "i" (SCB_ICSR),
          [psvset] "i" (PENDSVSET)
        : "r0", "r1"
    );

    while(1);
}

void scheduler::tick() {
    ++systick_count;

    check_canaries();

    for(uint8_t i = 0u; i < task_count; ++i) {
        Task *curr_task = tasks[i];

        if(curr_task->is_blocked() && curr_task->get_next_run_time_ms() <= systick_count) {
            curr_task->set_state(rte::TaskState::Ready);
        }
    }

    Task *next_task_candidate = get_next_task();
    if(next_task_candidate && next_task_candidate != current_task) {
        if(current_task && current_task->is_running()) {
            current_task->set_state(rte::TaskState::Ready);
        }

        next_task_candidate->set_state(rte::TaskState::Running);
        next_task = next_task_candidate;

        reg(SCB_ICSR) |= PENDSVSET;
    }
}

uint32_t scheduler::now() {
    return systick_count;
}
