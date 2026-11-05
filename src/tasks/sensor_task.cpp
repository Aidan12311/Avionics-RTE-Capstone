#include "src/tasks/sensor_task.hpp"
#include "src/rte_core/scheduler.hpp"

namespace {
    volatile uint32_t runs = 0u;
    rte::Latest<sensor_task::DataSample> latest_sample;

    sensor_task::DataSample to_board_frame(const sensor_task::DataSample &raw) {
        return {
            .accel = {
                .x = raw.accel.x,
                .y = -raw.accel.y,
                .z = -raw.accel.z
            },
            .gyro = {
                .x = raw.gyro.x,
                .y = -raw.gyro.y,
                .z = -raw.gyro.z
            }
        };
    }
} // anonymous namespace

void sensor_task::run() {
    while(true) {
        latest_sample.publish(to_board_frame(icm42688::read_all()));

        ++runs;

        scheduler::task_block_until_next_period();
    }
}

const rte::Latest<sensor_task::DataSample> &sensor_task::output() {
    return latest_sample;
}

uint32_t sensor_task::run_count() {
    return runs;
}