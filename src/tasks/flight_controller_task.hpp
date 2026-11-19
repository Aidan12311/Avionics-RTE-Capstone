#ifndef AVIONICS_RTE_CONTROLLER_TASK_HPP
#define AVIONICS_RTE_CONTROLLER_TASK_HPP

#include <cstdint>

#include "src/rte_core/latest.hpp"

namespace controller_task {
    constexpr uint32_t PERIOD_MS = 50u;

    enum class FlightMode : uint8_t {
        FreeFly   = 0,
        AutoPilot = 1,
    };

    struct FlightState {
        float roll{};
        float pitch{};

        FlightMode mode{};

        float aileron{};
        float elevator{};
    };

    void run();

    const rte::Latest<FlightState> &output();
} // namespace controller_task

#endif // AVIONICS_RTE_CONTROLLER_TASK
