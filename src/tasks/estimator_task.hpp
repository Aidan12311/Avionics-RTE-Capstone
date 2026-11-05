#ifndef AVIONICS_RTE_ESTIMATOR_TASK_HPP
#define AVIONICS_RTE_ESTIMATOR_TASK_HPP

#include <cstdint>

#include "src/rte_core/latest.hpp"
#include "src/filters/complementary.hpp"

namespace estimator_task {
    constexpr uint32_t PERIOD_MS = 10u;

    void run();

    const rte::Latest<filter::Attitude> &output();

    uint32_t run_count();
} // namespace estimator_task

#endif
