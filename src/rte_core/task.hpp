#ifndef AVIONICS_RTE_CORE_TASK_HPP
#define AVIONICS_RTE_CORE_TASK_HPP

#include <cstdint>
#include <cstddef>

namespace rte {
    enum class TaskState : uint8_t {
        Ready,
        Running,
        Blocked,
    };

    class Task {
    public:
        using TaskFn = void (*)();

        Task(
            TaskFn task_fn,
            const char *name, 
            uint32_t *stack,
            uint32_t stack_size_bytes,
            uint8_t priority,
            uint32_t period_ms
        );
    
        bool is_canary_ok() const {
            return _stack_base[0] == CANARY;
        }

        /* === Scheduler public interface === */
        bool is_ready() const {
            return _state == TaskState::Ready;
        }

        bool is_running() const {
            return _state == TaskState::Running;
        }

        bool is_blocked() const {
            return _state == TaskState::Blocked;
        }

        uint8_t get_priority() const {
            return _priority;
        }

        uint32_t get_period_ms() const {
            return _period_ms;
        }

        uint32_t get_next_run_time_ms() const {
            return _next_run_time_ms;
        }

        const char *get_name() const {
            return _name;
        }

        void set_next_run_time_ms(uint32_t next_run_time_ms) {
            _next_run_time_ms = next_run_time_ms;
        }

        void set_state(TaskState state) {
            _state = state;
        }

        void update_wcet(uint32_t elapsed) {
            _wcet_us = (_wcet_us > elapsed) ? _wcet_us : elapsed;
        }

        uint32_t *_sp{};

    private:
        const char *_name{};
        uint8_t _priority{};
        uint32_t _period_ms{};
        uint32_t _next_run_time_ms{};
        uint32_t _wcet_us{};
        TaskState _state{};
        uint32_t *_stack_base{};

        static constexpr uint32_t CANARY{0xDEADBEEFu};
    };

    static_assert(offsetof(Task, _sp) == 0, "Task::_sp must be the first member of Task");
} // namespace rte

#endif // AVIONICS_RTE_CORE_TASK_HPP_
