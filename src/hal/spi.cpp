#include <cstdint>

#include "src/hal/spi.hpp"
#include "src/hal/pins.hpp"
#include "src/hal/arm_core.hpp"

namespace {
    // ── Register addresses ────────────────────────────────────────────
    // SPI1 is on APB2, base address 0x40013000
    // Source: RM0090 Rev 22, Table 13
    constexpr uint32_t RCC_APB2ENR  = 0x40023844u;
    constexpr uint32_t SPI1_CR1     = 0x40013000u;
    constexpr uint32_t SPI1_CR2     = 0x40013004u;
    constexpr uint32_t SPI1_SR      = 0x40013008u;
    constexpr uint32_t SPI1_DR      = 0x4001300Cu;

    // ── RCC bit positions ─────────────────────────────────────────────
    constexpr uint32_t SPI1EN       = (1u << 12);  // APB2ENR bit 12

    // ── SPI1_CR1 bits ─────────────────────────────────────────────────
    // CPOL and CPHA together define the SPI mode.
    // Mode 3: CPOL=1 (clock idles HIGH), CPHA=1 (sample on second edge)
    // This is a hard requirement of the ICM-42688 — wrong mode = garbage data.
    constexpr uint32_t CR1_CPHA     = (1u << 0);   // clock phase
    constexpr uint32_t CR1_CPOL     = (1u << 1);   // clock polarity
    constexpr uint32_t CR1_MSTR     = (1u << 2);   // master mode
    constexpr uint32_t CR1_SPE      = (1u << 6);   // SPI enable
    constexpr uint32_t CR1_SSM      = (1u << 9);   // software slave management
    constexpr uint32_t CR1_SSI      = (1u << 8);   // internal slave select

    // BR[2:0] = bits 5:3 — baud rate divider
    // fPCLK/4 at 16MHz HSI = 4MHz — well within ICM-42688's 24MHz limit
    // On real hardware at 90MHz APB2: 90MHz/4 = 22.5MHz — still within limit
    constexpr uint32_t CR1_BR_DIV4  = (0b001u << 3);

    // ── SPI1_SR bits ──────────────────────────────────────────────────
    constexpr uint32_t SR_RXNE      = (1u << 0);   // receive buffer not empty
    constexpr uint32_t SR_TXE       = (1u << 1);   // transmit buffer empty
    constexpr uint32_t SR_BSY       = (1u << 7);   // bus busy
} // anonymous namespace

void spi::init() {
    // Enable peripheral clocks before touching any registers
    reg(RCC_APB2ENR) |= SPI1EN;

    // GPIO pin configuration for SPI1 (PA5=SCK, PA6=MISO, PA7=MOSI)
    pins::SPI1_SCK.configure_af(5u);
    pins::SPI1_MISO.configure_af(5u);
    pins::SPI1_MOSI.configure_af(5u);

    pins::IMU_CS.configure_output();
    pins::IMU_CS.set_high();

    reg(SPI1_CR1) = CR1_CPOL
        | CR1_CPHA
        | CR1_MSTR
        | CR1_SSM
        | CR1_SSI
        | CR1_BR_DIV4
        | CR1_SPE;
}

uint8_t spi::transfer(uint8_t data) {
    while(!(reg(SPI1_SR) & SR_TXE));
    reg(SPI1_DR) = data;

    while(!(reg(SPI1_SR) & SR_RXNE));
    return static_cast<uint8_t>(reg(SPI1_DR));
}

bool spi::is_busy() {
    return reg(SPI1_SR) & SR_BSY;
}
