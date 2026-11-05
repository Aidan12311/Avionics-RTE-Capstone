#include "src/hal/uart.hpp"

#include "src/tasks/sensor_task.hpp"
#include "src/tasks/estimator_task.hpp"
#include "src/tasks/telemetry_task.hpp"

#include "src/rte_core/scheduler.hpp"

namespace {
    constexpr uint32_t RUNS_PER_SECOND = 1000u / telemetry_task::PERIOD_MS;

    void send_attitude(const filter::Attitude& attitude) {
        uart::print("ATT,");
        
        uart::put_u32(scheduler::now());
        uart::put_char(',');

        uart::put_float(attitude.roll);
        uart::put_char(',');

        uart::put_float(attitude.pitch);
        uart::put_new_line();
    }

    void send_statistics() {
        uart::print("STAT,");
        
        uart::put_u32(scheduler::now());
        uart::put_char(',');

        uart::put_u32(sensor_task::run_count());
        uart::put_char(',');

        uart::put_u32(estimator_task::run_count());
        uart::put_new_line();
    }
} // anonymous namespace

void telemetry_task::run() {
    uint32_t runs = 0u;

    while(true) {
        if(const auto attitude = estimator_task::output().read()) {
            send_attitude(*attitude);
        }

        if(++runs % RUNS_PER_SECOND == 0u) {
            send_statistics();
        }

        scheduler::task_block_until_next_period();
    }
}
