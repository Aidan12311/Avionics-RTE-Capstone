#ifndef AVIONICS_RTE_TASKS_SENSOR_TASK_HPP
#define AVIONICS_RTE_TASKS_SENSOR_TASK_HPP

#include <cstdint>

#include "src/rte_core/latest.hpp"
#include "src/drivers/icm42688.hpp"

namespace sensor_task {
    constexpr uint32_t PERIOD_MS = 1u;

    using DataSample = icm42688::SensorData;

    void run();

    
}

#endif // AVIONICS_RTE_TASKS_SENSOR_TASK_HPP
