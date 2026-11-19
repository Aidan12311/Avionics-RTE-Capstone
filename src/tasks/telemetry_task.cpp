#include "src/hal/uart.hpp"

#include "src/tasks/sensor_task.hpp"
#include "src/tasks/telemetry_task.hpp"
#include "src/tasks/estimator_task.hpp"
#include "src/tasks/flight_controller_task.hpp"

#include "src/rte_core/scheduler.hpp"

namespace {
    constexpr uint32_t RUNS_PER_SECOND = 1000u / telemetry_task::PERIOD_MS;

    void send_attitude(const controller_task::FlightState& state) {
        uart::print("ATT,");
        
        uart::put_u32(scheduler::now());
        uart::put_char(',');

        uart::put_float(state.roll);
        uart::put_char(',');

        uart::put_float(state.pitch);
        uart::put_new_line();
    }

    void send_control(const controller_task::FlightState& state) {
        uart::print("CTRL,");
        
        uart::put_u32(scheduler::now());
        uart::put_char(',');

        uart::put_u32(static_cast<uint32_t>(state.mode));
        uart::put_char(',');

        uart::put_float(state.aileron);
        uart::put_char(',');

        uart::put_float(state.elevator);
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
        if(const auto state = controller_task::output().read()) {
            send_attitude(*state);
            send_control(*state);
        }

        if(++runs % RUNS_PER_SECOND == 0u) {
            send_statistics();
        }

        scheduler::task_block_until_next_period();
    }
}
