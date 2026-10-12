// main.cpp — IMU WHO_AM_I smoke test
// Expected output: "WHO_AM_I = 0x47" over UART1 at 115200 baud
#include <cstdint>

// ── Register access helper ────────────────────────────────────────────
static volatile uint32_t& reg(uint32_t addr) {
    return *reinterpret_cast<volatile uint32_t*>(addr);
}

// ── RCC addresses (RM0090 Table 13) ──────────────────────────────────
constexpr uint32_t RCC_AHB1ENR  = 0x40023830u;  // GPIO clocks
constexpr uint32_t RCC_APB2ENR  = 0x40023844u;  // SPI1, USART1
constexpr uint32_t RCC_APB1ENR  = 0x40023840u;  // USART2 (unused now)

// ── GPIO A addresses ──────────────────────────────────────────────────
constexpr uint32_t GPIOA_MODER  = 0x40020000u;
constexpr uint32_t GPIOA_AFRL   = 0x40020020u;
constexpr uint32_t GPIOA_AFRH   = 0x40020024u;
constexpr uint32_t GPIOA_BSRR   = 0x40020018u;

// ── USART1 addresses (APB2, base 0x40011000) ─────────────────────────
// Source: RM0090 Table 13 — USART1 at 0x40011000
constexpr uint32_t USART1_SR    = 0x40011000u;
constexpr uint32_t USART1_DR    = 0x40011004u;
constexpr uint32_t USART1_BRR   = 0x40011008u;
constexpr uint32_t USART1_CR1   = 0x4001100Cu;

// ── SPI1 addresses (APB2, base 0x40013000) ───────────────────────────
constexpr uint32_t SPI1_CR1     = 0x40013000u;
constexpr uint32_t SPI1_SR      = 0x40013008u;
constexpr uint32_t SPI1_DR      = 0x4001300Cu;

// ── SPI1 status bits ─────────────────────────────────────────────────
constexpr uint32_t SPI_SR_RXNE  = (1u << 0);
constexpr uint32_t SPI_SR_TXE   = (1u << 1);
constexpr uint32_t SPI_SR_BSY   = (1u << 7);

// ── USART1 status bits ───────────────────────────────────────────────
constexpr uint32_t USART_SR_TXE = (1u << 7);

// ── PA8 chip select helpers ───────────────────────────────────────────
static void cs_low()  { reg(GPIOA_BSRR) = (1u << (8 + 16)); }
static void cs_high() { reg(GPIOA_BSRR) = (1u << 8); }

// ── UART1 init ────────────────────────────────────────────────────────
// USART1 is connected to ST-Link VCP via SB11/SB15 (default closed)
// PA9 = USART1_TX, AF7
// USART1 is on APB2 — same bus as SPI1, runs at 16MHz HSI at reset
static void uart1_init() {
    reg(RCC_AHB1ENR) |= (1u << 0);    // GPIOAEN
    reg(RCC_APB2ENR) |= (1u << 4);    // USART1EN — APB2 bit 4

    // PA9 = USART1_TX, AF7
    // PA9 is in AFRH (pins 8–15), offset = (9 - 8) * 4 = 4
    reg(GPIOA_MODER) &= ~(3u << (9 * 2));
    reg(GPIOA_MODER) |=  (2u << (9 * 2));   // alternate function
    reg(GPIOA_AFRH)  &= ~(0xFu << ((9 - 8) * 4));
    reg(GPIOA_AFRH)  |=  (7u   << ((9 - 8) * 4));  // AF7 = USART1

    // 115200 baud at 16MHz HSI
    // DIV = 16000000 / (16 * 115200) = 8.680...
    // Mantissa = 8, Fraction = 0.680 * 16 = 10.88 → 11 = 0xB
    // BRR = (8 << 4) | 11 = 0x08B
    reg(USART1_BRR)  = 0x008Bu;
    reg(USART1_CR1)  = (1u << 13) | (1u << 3);  // UE | TE
}

// ── UART1 send one character ──────────────────────────────────────────
static void uart1_putc(char c) {
    while (!(reg(USART1_SR) & USART_SR_TXE));
    reg(USART1_DR) = (uint32_t)c;
}

// ── UART1 send string ─────────────────────────────────────────────────
static void uart1_puts(const char* s) {
    while (*s) uart1_putc(*s++);
}

// ── UART1 print hex byte ──────────────────────────────────────────────
static void uart1_print_hex(uint8_t val) {
    const char hex[] = "0123456789ABCDEF";
    uart1_putc('0');
    uart1_putc('x');
    uart1_putc(hex[(val >> 4) & 0xF]);
    uart1_putc(hex[val & 0xF]);
}

// ── SPI1 init ─────────────────────────────────────────────────────────
static void spi1_init() {
    reg(RCC_AHB1ENR) |= (1u << 0);    // GPIOAEN
    reg(RCC_APB2ENR) |= (1u << 12);   // SPI1EN

    // PA5 = SPI1_SCK AF5
    reg(GPIOA_MODER) &= ~(3u << (5 * 2));
    reg(GPIOA_MODER) |=  (2u << (5 * 2));
    reg(GPIOA_AFRL)  &= ~(0xFu << (5 * 4));
    reg(GPIOA_AFRL)  |=  (5u   << (5 * 4));

    // PA6 = SPI1_MISO AF5
    reg(GPIOA_MODER) &= ~(3u << (6 * 2));
    reg(GPIOA_MODER) |=  (2u << (6 * 2));
    reg(GPIOA_AFRL)  &= ~(0xFu << (6 * 4));
    reg(GPIOA_AFRL)  |=  (5u   << (6 * 4));

    // PA7 = SPI1_MOSI AF5
    reg(GPIOA_MODER) &= ~(3u << (7 * 2));
    reg(GPIOA_MODER) |=  (2u << (7 * 2));
    reg(GPIOA_AFRL)  &= ~(0xFu << (7 * 4));
    reg(GPIOA_AFRL)  |=  (5u   << (7 * 4));

    // PA8 = CS, GPIO output
    reg(GPIOA_MODER) &= ~(3u << (8 * 2));
    reg(GPIOA_MODER) |=  (1u << (8 * 2));
    cs_high();

    // SPI1_CR1: Mode 3 (CPOL=1 CPHA=1), master, BR=fPCLK/8, SSM+SSI, SPE
    constexpr uint32_t CPOL    = (1u << 1);
    constexpr uint32_t CPHA    = (1u << 0);
    constexpr uint32_t MSTR    = (1u << 2);
    constexpr uint32_t BR_DIV8 = (0b010u << 3);
    constexpr uint32_t SSM     = (1u << 9);
    constexpr uint32_t SSI     = (1u << 8);
    constexpr uint32_t SPE     = (1u << 6);

    reg(SPI1_CR1) = CPOL | CPHA | MSTR | BR_DIV8 | SSM | SSI | SPE;
}

// ── SPI1 transfer one byte ────────────────────────────────────────────
static uint8_t spi1_transfer(uint8_t data) {
    while (!(reg(SPI1_SR) & SPI_SR_TXE));
    reg(SPI1_DR) = data;
    while (!(reg(SPI1_SR) & SPI_SR_RXNE));
    return (uint8_t)reg(SPI1_DR);
}

// ── Read one IMU register ─────────────────────────────────────────────
static uint8_t imu_read_reg(uint8_t addr) {
    cs_low();
    spi1_transfer(addr | 0x80u);
    uint8_t val = spi1_transfer(0x00u);
    while (reg(SPI1_SR) & SPI_SR_BSY);
    cs_high();
    return val;
}

// ── Simple busy-wait delay ────────────────────────────────────────────
static void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms * 16000u; i++) {
        __asm__ volatile("nop");
    }
}

int main() {
    uart1_init();
    spi1_init();

    uart1_puts("IMU WHO_AM_I test\r\n");

    delay_ms(30);

    uint8_t who_am_i = imu_read_reg(0x75u);

    uart1_puts("WHO_AM_I = ");
    uart1_print_hex(who_am_i);
    uart1_puts("\r\n");

    if (who_am_i == 0x47u) {
        uart1_puts("IMU OK — ICM-42688 detected\r\n");
    } else {
        uart1_puts("ERROR — check wiring and solder joints\r\n");
    }

    while (1);
}