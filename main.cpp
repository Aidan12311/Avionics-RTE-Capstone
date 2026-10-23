#include "src/hal/uart.hpp"
#include "src/hal/gpio.hpp"
#include "src/hal/pins.hpp"
#include "src/hal/arm_core.hpp"
#include "src/rte_core/task.hpp"
#include "src/rte_core/scheduler.hpp"

__attribute__((section(".ccm")))
static uint32_t task_a_stack[256]{};

__attribute__((section(".ccm")))
static uint32_t task_b_stack[256]{};

__attribute__((section(".ccm")))
static uint32_t idle_task_stack[128]{};

void idle_task_fn() {
    while(true) {
        arm::nop();
    }
}

static rte::Task idle_task {
    idle_task_fn,
    "Idle Task",
    idle_task_stack,
    sizeof(idle_task_stack),
    0,
    1
};

void task_a_fn() {
    while(true) {
        uart::println("Task A is running");
        scheduler::task_block_until_next_period();
    }
}

void task_b_fn() {
    while(true) {
        uart::println("Task B is running");
        scheduler::task_block_until_next_period();
    }
}

static rte::Task task_a {
    task_a_fn,
    "Task A",
    task_a_stack,
    sizeof(task_a_stack),
    1,
    500
};

static rte::Task task_b {
    task_b_fn,
    "Task B",
    task_b_stack,
    sizeof(task_b_stack),
    2,
    1000
};

int main() {
    uart::init();
    pins::HEARTBEAT.configure_output();
    pins::LED_RED.configure_output();

    pins::HEARTBEAT.set_low();
    pins::LED_RED.set_low();

    uart::println("BOOT OK! Starting Scheduler...");

    scheduler::add_task(&idle_task);
    scheduler::add_task(&task_a);
    scheduler::add_task(&task_b);

    scheduler::start();
}
