#include <cstdint>

extern uint32_t _estack;

extern "C" void reset_handler(void);
extern "C" void default_handler(void) { while(1); }

int main(void);

__attribute__((section(".isr_vector"), used))
const uint32_t vector_table[] = {
    (uint32_t)&_estack,
    (uint32_t)&reset_handler,
    (uint32_t)&default_handler, // NMI Handler
    (uint32_t)&default_handler, // HardFault Handler
};

extern "C" void reset_handler(void) {
    main();
    while(1);
}
