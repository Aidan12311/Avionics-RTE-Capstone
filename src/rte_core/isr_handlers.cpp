#include "src/rte_core/scheduler.hpp"

extern "C" void systick_handler() {
    scheduler::tick();
}
