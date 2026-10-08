#ifndef ARM_CORE_HPP
#define ARM_CORE_HPP

namespace arm {
    inline void dsb() { __asm__ volatile("dsb" ::: "memory"); }
    inline void isb() { __asm__ volatile("isb" ::: "memory"); }

    inline void enable_irq() { __asm__ volatile("cpsie i" ::: "memory"); }
    inline void disable_irq() { __asm__ volatile("cpsid i" ::: "memory"); }

    inline void nop() { __asm__ volatile("nop"); }
} // namespace arm

#endif // ARM_CORE_HPP
