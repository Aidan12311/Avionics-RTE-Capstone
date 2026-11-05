#include <cstdint>

#include "src/hal/uart.hpp"
#include "src/hal/spi.hpp"
#include "src/hal/gpio.hpp"
#include "src/hal/pins.hpp"
#include "src/hal/arm_core.hpp"

#include "src/drivers/icm42688.hpp"

#include "src/rte_core/task.hpp"
#include "src/rte_core/scheduler.hpp"

#include "src/tasks/sensor_task.hpp"
#include "src/tasks/estimator_task.hpp"
#include "src/tasks/telemetry_task.hpp"

namespace {
    constexpr uint8_t PRIORITY_IDLE       = 0u;
    
    constexpr uint8_t PRIORITY_TELEMETRY  = 1u;
    constexpr uint8_t PRIORITY_CONTROLLER = 2u;
    constexpr uint8_t PRIORITY_ESTIMATOR  = 3u;
    constexpr uint8_t PRIORITY_SENSOR     = 4u;

    constexpr uint32_t STACK_WORDS = 256u;
    constexpr uint32_t IDLE_PERIOD_MS = 1u;
    constexpr uint32_t HEARTBEAT_HALF_PERIOD_MS = 500u;
} // anonymous namespace

alignas(8) __attribute__((section(".ccm"))) static uint32_t idle_stack[STACK_WORDS];

alignas(8) __attribute__((section(".ccm"))) static uint32_t sensor_stack[STACK_WORDS];
alignas(8) __attribute__((section(".ccm"))) static uint32_t estimator_stack[STACK_WORDS];
// alignas(8) __attribute__((section(".ccm"))) static uint32_t controller_stack[STACK_WORDS];
alignas(8) __attribute__((section(".ccm"))) static uint32_t telemetry_stack[STACK_WORDS];

void idle_task_fn() {
    uint32_t last_toggle_ms = scheduler::now();

    while(true) {
        if(scheduler::now() - last_toggle_ms >= HEARTBEAT_HALF_PERIOD_MS) {
            pins::HEARTBEAT.toggle();
            last_toggle_ms += HEARTBEAT_HALF_PERIOD_MS;
        }
    }
}

static rte::Task idle_task {
    idle_task_fn,
    "idle",
    idle_stack,
    sizeof(idle_stack),
    PRIORITY_IDLE,
    IDLE_PERIOD_MS
};

static rte::Task sensor_rt {
    sensor_task::run,
    "sensor",
    sensor_stack,
    sizeof(sensor_stack),
    PRIORITY_SENSOR,
    sensor_task::PERIOD_MS
};

static rte::Task estimator_rt {
    estimator_task::run,
    "estimator",
    estimator_stack,
    sizeof(estimator_stack),
    PRIORITY_ESTIMATOR,
    estimator_task::PERIOD_MS
};

/*
static rte::Task controller_rt {
    controller_task::run,
    "controller",
    controller_stack,
    sizeof(controller_stack),
    PRIORITY_CONTROLLER,
    controller_task::PERIOD_MS
};
*/

static rte::Task telemetry_rt {
    telemetry_task::run,
    "telemetry",
    telemetry_stack,
    sizeof(telemetry_stack),
    PRIORITY_TELEMETRY,
    telemetry_task::PERIOD_MS
};

int main() {
    uart::init();
    spi::init();

    pins::HEARTBEAT.configure_output();
    pins::LED_RED.configure_output();

    pins::HEARTBEAT.set_low();
    pins::LED_RED.set_low();

    uart::println("Booting...");

    if(!icm42688::init()) {
        uart::println("FAILED to initialize ICM-42688 IMU! Check wiring!");
        pins::LED_RED.set_high();

        while(true);
    }

    uart::println("IMU OK! Starting scheduler!");

    scheduler::add_task(&idle_task);
    scheduler::add_task(&sensor_rt);
    //scheduler::add_task(&controller_rt);
    scheduler::add_task(&estimator_rt);
    scheduler::add_task(&telemetry_rt);

    scheduler::start();
}
