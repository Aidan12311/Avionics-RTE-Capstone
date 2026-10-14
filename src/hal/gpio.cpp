#include <cstdint>

#include "src/hal/gpio.hpp"
#include "src/hal/arm_core.hpp"

using gpio::Pin;

// ─────────────────────────────────────────────────────────────────────
// GPIO register offsets from port base address
// Source: RM0090 Rev 22, Section 8.4
// ─────────────────────────────────────────────────────────────────────
namespace {
    constexpr uint32_t MODER_OFFSET   = 0x00u;  // mode register
    constexpr uint32_t OTYPER_OFFSET  = 0x04u;  // output type
    constexpr uint32_t OSPEEDR_OFFSET = 0x08u;  // output speed
    constexpr uint32_t PUPDR_OFFSET   = 0x0Cu;  // pull-up/pull-down
    constexpr uint32_t ODR_OFFSET     = 0x14u;  // output data register
    constexpr uint32_t BSRR_OFFSET    = 0x18u;  // bit set/reset register
    constexpr uint32_t AFRL_OFFSET    = 0x20u;  // AF low  (pins 0–7)
    constexpr uint32_t AFRH_OFFSET    = 0x24u;  // AF high (pins 8–15)

    // GPIO port base addresses — 0x400 apart from each other
    // Source: RM0090 Table 13
    constexpr uint32_t GPIOA_BASE = 0x40020000u;
    constexpr uint32_t PORT_STRIDE = 0x400u;

    // MODER field values — 2 bits per pin
    constexpr uint32_t MODER_OUTPUT = 0x1u;
    constexpr uint32_t MODER_AF     = 0x2u;

    // OTYPER — push-pull = 0, open-drain = 1
    constexpr uint32_t OTYPE_PUSH_PULL = 0x0u;

    // OSPEEDR — medium speed (50MHz) = 0b10
    constexpr uint32_t OSPEED_MEDIUM = 0x2u;

    // PUPDR — no pull = 0b00
    constexpr uint32_t PUPDR_NONE = 0x0u;
}

volatile uint32_t *Pin::base() const {
    const uint32_t offset = static_cast<uint32_t>(_port) * PORT_STRIDE;
    volatile uint32_t *addr = reinterpret_cast<volatile uint32_t *>(GPIOA_BASE + offset);

    return addr;
}

void Pin::configure_af(uint8_t af) const {
    uint32_t base_addr = reinterpret_cast<uint32_t>(base());
    uint32_t casted_number = static_cast<uint32_t>(_number);

    uint32_t shift = casted_number < 8 
        ? casted_number * 4u 
        : (casted_number - 8) * 4u;
    uint32_t afr_offset = casted_number < 8 ? AFRL_OFFSET : AFRH_OFFSET;

    reg(base_addr + afr_offset) &= ~(0xFu << shift);
    reg(base_addr + afr_offset) |=  (static_cast<uint32_t>(af) << shift);

    reg(base_addr + MODER_OFFSET) &= ~(0x3u << (casted_number * 2u));
    reg(base_addr + MODER_OFFSET) |=  (MODER_AF << (casted_number * 2u));
}

void Pin::configure_output() const {
    uint32_t base_addr = reinterpret_cast<uint32_t>(base());
    uint32_t casted_number = static_cast<uint32_t>(_number);

    reg(base_addr + OTYPER_OFFSET)  &= ~(0x1u << casted_number);
    reg(base_addr + OTYPER_OFFSET)  |=  (OTYPE_PUSH_PULL << casted_number);

    reg(base_addr + OSPEEDR_OFFSET) &= ~(0x3u << (casted_number * 2u));
    reg(base_addr + OSPEEDR_OFFSET) |=  (OSPEED_MEDIUM << (casted_number * 2u));

    reg(base_addr + PUPDR_OFFSET)   &= ~(0x3u << (casted_number * 2u));
    reg(base_addr + PUPDR_OFFSET)   |=  (PUPDR_NONE << (casted_number * 2u));

    reg(base_addr + MODER_OFFSET)   &= ~(0x3u << (casted_number * 2u));
    reg(base_addr + MODER_OFFSET)   |=  (MODER_OUTPUT << (casted_number * 2u));
}

void Pin::set_high() const {
    uint32_t base_addr = reinterpret_cast<uint32_t>(base());
    uint32_t casted_number = static_cast<uint32_t>(_number);
    
    reg(base_addr + BSRR_OFFSET) = (1u << casted_number);
}

void Pin::set_low() const {
    uint32_t base_addr = reinterpret_cast<uint32_t>(base());
    uint32_t casted_number = static_cast<uint32_t>(_number);
    
    reg(base_addr + BSRR_OFFSET) = (1u << (casted_number + 16u));
}

void Pin::toggle() const {
    uint32_t base_addr = reinterpret_cast<uint32_t>(base());
    uint32_t casted_number = static_cast<uint32_t>(_number);

    reg(base_addr + ODR_OFFSET) ^= (1u << casted_number);
}

