#ifndef AVIONICS_RTE_CORE_LATEST_HPP
#define AVIONICS_RTE_CORE_LATEST_HPP

#include <optional>
#include <type_traits>

#include "src/hal/arm_core.hpp"

namespace rte {
template<typename T>
class Latest {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable!");

public:
    void publish(const T& value) {
        arm::IrqLock lock{};
        _value = value;
    }

    std::optional<T> read() const {
        arm::IrqLock lock{};
        return _value;
    }
private:
    std::optional<T> _value{};
};
} // namespace rte

#endif // AVIONICS_RTE_CORE_LATEST_HPP
