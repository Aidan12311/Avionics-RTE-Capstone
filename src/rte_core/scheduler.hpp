#ifndef AVIONICS_RTE_CORE_SCHEDULER_HPP
#define AVIONICS_RTE_CORE_SCHEDULER_HPP

#include <cstdint>
#include "src/rte_core/task.hpp"

using rte::Task;

namespace scheduler {
    constexpr uint8_t MAX_TASKS = 8u;

    void add_task(Task *task);

    void task_block_until_next_period();

    void start();

    void tick();

    uint32_t now();
} // namespace scheduler

extern "C" {
    extern Task *current_task;
    extern Task *next_task;
}

#endif // AVIONICS_RTE_CORE_SCHEDULER_HPP_ 
