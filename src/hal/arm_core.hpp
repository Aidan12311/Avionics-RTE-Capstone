#ifndef AVIONICS_RTE_ARM_CORE_HPP
#define AVIONICS_RTE_ARM_CORE_HPP

#include <cstdint>

namespace arm {
    inline void dsb() { __asm__ volatile("dsb" ::: "memory"); }
    inline void isb() { __asm__ volatile("isb" ::: "memory"); }

    inline void nop() { __asm__ volatile("nop"); }

    class IrqLock {
    public:
        IrqLock() {
            __asm__ volatile("mrs %[s], primask" : [s] "=r" (_saved_state));
            __asm__ volatile("cpsid i" ::: "memory");
        }
        ~IrqLock() {
            __asm__ volatile("msr primask, %[s]" : : [s] "r" (_saved_state) : "memory");
        }

        IrqLock(const IrqLock&) = delete;
        IrqLock& operator=(const IrqLock&) = delete;

    private:
        uint32_t _saved_state{};
    };
} // namespace arm

template<typename T=uint32_t>
inline volatile T& as_reg(uint32_t addr) {
    return *reinterpret_cast<volatile T*>(addr);
}

template<typename T=uint32_t>
inline volatile T& reg(uint32_t addr) {
    return as_reg<T>(addr);
}

#endif // AVIONICS_RTE_ARM_CORE_HPP
