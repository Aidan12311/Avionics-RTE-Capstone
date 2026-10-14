#ifndef AVIONICS_RTE_PINS_HPP
#define AVIONICS_RTE_PINS_HPP

#include "src/hal/gpio.hpp"

namespace pins {
    // ── LEDs ──────────────────────────────────────────────────────────
    // Source: UM1670 Table 4 — LD3 green PG13, LD4 red PG14
    constexpr gpio::Pin HEARTBEAT { gpio::Port::G, 13u };  // green LD3
    constexpr gpio::Pin LED_RED   { gpio::Port::G, 14u };  // red   LD4

    // ── USART1 — ST-Link VCP ──────────────────────────────────────────
    // PA9 = USART1_TX AF7
    // Source: DS9405 Table 11 (AF mapping), UM1670 (SB11/SB15 closed)
    constexpr gpio::Pin UART1_TX  { gpio::Port::A, 9u  };

    // ── SPI1 — IMU ICM-42688 ──────────────────────────────────────────
    // PA5 = SPI1_SCK  AF5
    // PA6 = SPI1_MISO AF5
    // PA7 = SPI1_MOSI AF5
    // PA8 = IMU CS    GPIO output (software chip select)
    // Source: DS9405 Table 11 (AF mapping)
    constexpr gpio::Pin SPI1_SCK  { gpio::Port::A, 5u  };
    constexpr gpio::Pin SPI1_MISO { gpio::Port::A, 6u  };
    constexpr gpio::Pin SPI1_MOSI { gpio::Port::A, 7u  };
    constexpr gpio::Pin IMU_CS    { gpio::Port::A, 8u  };
}

#endif
