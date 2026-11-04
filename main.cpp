#include <cstdint>
#include "src/hal/uart.hpp"
#include "src/rte_core/task.hpp"
#include "src/rte_core/scheduler.hpp"

__attribute__((always_inline)) inline void set_fpu_regs(uint32_t bits) {
    __asm__ volatile("vmov s16, %[v]\n vmov s23, %[v]\n vmov s31, %[v]\n"
                     : : [v] "r" (bits) : "s16", "s23", "s31");
}

__attribute__((always_inline)) inline bool fpu_regs_equal(uint32_t bits) {
    uint32_t a, b, c;
    __asm__ volatile("vmov %[a], s16\n vmov %[b], s23\n vmov %[c], s31\n"
                     : [a] "=r" (a), [b] "=r" (b), [c] "=r" (c));
    return a == bits && b == bits && c == bits;
}

constexpr uint32_t H_PAT = 0x11111111u;
constexpr uint32_t L_PAT = 0x22222222u;

volatile uint32_t h_iters, h_errs, l_checks, l_errs;

alignas(8) static uint32_t h_stack[256];
alignas(8) static uint32_t l_stack[256];
alignas(8) static uint32_t n_stack[256];

void h_task() {
    while(true) {
        set_fpu_regs(H_PAT);
        scheduler::task_block_until_next_period();
        if(!fpu_regs_equal(H_PAT)) { ++h_errs; }
        ++h_iters;
    }
}

void l_task() {
    while(true) {
        set_fpu_regs(L_PAT);
        volatile float a = 1.25f;
        float r = a * 4.0f + 0.5f;      // 5.5
        float s = r * r - 2.25f;        // 28.0
        if(s != 28.0f)             { ++l_errs; }
        if(!fpu_regs_equal(L_PAT)) { ++l_errs; }
        ++l_checks;
    }
}

void n_task() {
    uint32_t n = 0u;
    while(true) {
        if(++n % 100u == 0u) {
            uart::print("H=");     uart::put_u32(h_iters);
            uart::print(" Herr="); uart::put_u32(h_errs);
            uart::print(" L=");    uart::put_u32(l_checks);
            uart::print(" Lerr="); uart::put_u32(l_errs);
            uart::print(" N=");    uart::put_u32(n);
            uart::println("");
        }
        scheduler::task_block_until_next_period();
    }
}

static rte::Task h { h_task, "H", h_stack, sizeof(h_stack), 3, 3 };
static rte::Task l { l_task, "L", l_stack, sizeof(l_stack), 2, 1 };
static rte::Task n { n_task, "N", n_stack, sizeof(n_stack), 4, 7 };

int main() {
    uart::init();
    uart::println("fpu switch test start");
    scheduler::add_task(&l);
    scheduler::add_task(&h);
    scheduler::add_task(&n);
    scheduler::start();
}
