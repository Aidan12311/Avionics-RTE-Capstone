#ifndef AVIONICS_RTE_UART_HPP
#define AVIONICS_RTE_UART_HPP

#include <cstdint>

namespace uart {
    void init();

    void put_next_line();

    void print(const char *str);
    void println(const char *str);

    void put_char(char c);
    void put_hex(uint8_t value);
    void put_u32(uint32_t value);

    void isr_tx_handler();
} // namespace uart

#endif // AVIONICS_RTE_UART_HPP
