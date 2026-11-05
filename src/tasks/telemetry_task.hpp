#ifndef AVIONICS_RTE_TELEMETRY_TASK_HPP
#define AVIONICS_RTE_TELEMETRY_TASK_HPP

#include <cstdint>

namespace telemetry_task {
    constexpr uint32_t PERIOD_MS = 50u;

    void run();
}

#endif // AVIONICS_RTE_TELEMETRY_TASK_HPP
