#include <cstdint>

#include "src/hal/uart.hpp"
#include "src/hal/pins.hpp"
#include "src/hal/arm_core.hpp"
#include "src/hal/ring_buffer.hpp"

namespace {
    // ── Register addresses ────────────────────────────────────────────
    // Source: RM0090 Rev 22, Table 13
    constexpr uint32_t RCC_AHB1ENR  = 0x40023830u;
    constexpr uint32_t RCC_APB2ENR  = 0x40023844u;
    constexpr uint32_t GPIOA_MODER  = 0x40020000u;
    constexpr uint32_t GPIOA_AFRH   = 0x40020024u;
    constexpr uint32_t USART1_SR    = 0x40011000u;
    constexpr uint32_t USART1_DR    = 0x40011004u;
    constexpr uint32_t USART1_BRR   = 0x40011008u;
    constexpr uint32_t USART1_CR1   = 0x4001100Cu;

    // ── RCC bit positions ─────────────────────────────────────────────
    constexpr uint32_t GPIOAEN      = (1u << 0);   // AHB1ENR
    constexpr uint32_t USART1EN     = (1u << 4);   // APB2ENR

    // ── USART1_CR1 bits ───────────────────────────────────────────────
    constexpr uint32_t CR1_UE       = (1u << 13);  // UART enable
    constexpr uint32_t CR1_TE       = (1u << 3);   // transmitter enable
    constexpr uint32_t CR1_TXEIE    = (1u << 7);   // TX empty interrupt enable

    // ── BRR for 115200 baud at 16MHz HSI ─────────────────────────────
    // DIV = 16000000 / (16 * 115200) = 8.680...
    // Mantissa = 8, Fraction = round(0.680 * 16) = 11
    // BRR = (8 << 4) | 11 = 0x08B
    constexpr uint32_t BRR_115200   = 0x008Bu;

    // In uart::init(), after configuring USART1:
    // NVIC ISER1 — IRQs 32-63, bit position = IRQ - 32
    // IRQ37: bit 37 - 32 = bit 5 of ISER1
    constexpr uint32_t NVIC_ISER1 = 0xE000E104u;

    // TX ring buffer
    RingBuffer<uint8_t, 256> tx_buffer;
} // anonymous namespace

void uart::init() {
    // Enable peripheral clocks before touching any registers
    reg(RCC_AHB1ENR) |= GPIOAEN;
    reg(RCC_APB2ENR) |= USART1EN;

    // GPIO pin configuration for USART1_TX (PA9)
    pins::UART1_TX.configure_af(7u); 

    // Set baud rate and enable UART with TX only
    reg(USART1_BRR) = BRR_115200;
    reg(USART1_CR1) = CR1_UE | CR1_TE;

    // Enable NVIC for IRQ handling
    reg(NVIC_ISER1) |= (1u << 5);
}

void uart::put_new_line(void) {
    uart::put_char('\n');
    uart::put_char('\r');
}

void uart::put_char(char c) {
    uint8_t unsigned_value = static_cast<uint8_t>(c);
    while(!tx_buffer.write(unsigned_value));

    reg(USART1_CR1) |= CR1_TXEIE;
}

void uart::print(const char *str) {
    for(; *str; ++str) {
        uart::put_char(*str);
    }
}

void uart::println(const char *str) {
    uart::print(str);

    uart::put_char('\n');
    uart::put_char('\r');
}

void uart::put_hex(uint8_t value) {
    constexpr const char *hex_str = "0123456789ABCDEF";
    uart::put_char(hex_str[(value >> 4) & 0xFu]);
    uart::put_char(hex_str[value & 0xFu]);
}

void uart::put_u32(uint32_t value) {
    if(value == 0u) {
        put_char('0');
        return;
    }

    char digits[10]{};
    uint8_t digit_count = 0;

    while(value > 0) {
        digits[digit_count++] = static_cast<char>('0' + (value % 10));
        value /= 10;
    }

    for(uint8_t i = digit_count; i > 0; --i) {
        put_char(digits[i - 1]);
    }
}

void uart::isr_tx_handler() {
    uint8_t byte{};

    if(tx_buffer.read(byte)) {
        reg(USART1_DR) = byte;
    }
    else {
        reg(USART1_CR1) &= ~CR1_TXEIE;
    }
}

extern "C" void usart1_irq_handler(void) {
    uart::isr_tx_handler();
}
