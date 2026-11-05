#include "src/tasks/sensor_task.hpp"
#include "src/tasks/estimator_task.hpp"

#include "src/rte_core/scheduler.hpp"

namespace {
    constexpr float DT_SECONDS = static_cast<float>(estimator_task::PERIOD_MS) / 1000.0f;

    rte::Latest<filter::Attitude> latest_attitude;
    volatile uint32_t runs = 0u;
}

void estimator_task::run() {
    filter::ComplementaryFilter complementary_filter{};

    while(true) {
        if(const auto sample = sensor_task::output().read()) {
            latest_attitude.publish(complementary_filter.update(
                sample->accel.x, sample->accel.y, sample->accel.z,
                sample->gyro.x, sample->gyro.y,
                DT_SECONDS
            ));
        }

        ++runs;

        scheduler::task_block_until_next_period();
    }
}

const rte::Latest<filter::Attitude> &estimator_task::output() {
    return latest_attitude;
}

uint32_t estimator_task::run_count() {
    return runs;
}
