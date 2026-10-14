#ifndef AVIONICS_RTE_ARM_CORE_HPP
#define AVIONICS_RTE_ARM_CORE_HPP

#include <cstdint>

namespace arm {
    inline void dsb() { __asm__ volatile("dsb" ::: "memory"); }
    inline void isb() { __asm__ volatile("isb" ::: "memory"); }

    inline void enable_irq() { __asm__ volatile("cpsie i" ::: "memory"); }
    inline void disable_irq() { __asm__ volatile("cpsid i" ::: "memory"); }

    inline void nop() { __asm__ volatile("nop"); }
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
