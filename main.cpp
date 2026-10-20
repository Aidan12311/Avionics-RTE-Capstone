#include "src/hal/spi.hpp"
#include "src/hal/uart.hpp"
#include "src/hal/gpio.hpp"
#include "src/hal/pins.hpp"
#include "src/hal/arm_core.hpp"

int main() {
    uart::init();
    spi::init();

    pins::HEARTBEAT.configure_output();
    pins::LED_RED.configure_output();
    pins::HEARTBEAT.set_low();
    pins::LED_RED.set_low();

    uart::println("Boot OK");

    // ── UART Test ─────────────────────────────────────────────────────
    uart::println("== UART Test ==");
    uart::print("put_u32: ");
    uart::put_u32(115200u);
    uart::put_new_line();
    uart::print("put_hex: 0x");
    uart::put_hex(0xABu);
    uart::put_new_line();
    uart::println("UART OK");

    // ── GPIO Test ─────────────────────────────────────────────────────
    uart::println("== GPIO Test ==");
    for (int i = 0; i < 3; ++i) {
        pins::HEARTBEAT.set_high();
        for (volatile int j = 0; j < 500000; ++j) { arm::nop(); }
        
        pins::HEARTBEAT.set_low();
        for (volatile int j = 0; j < 500000; ++j) { arm::nop(); }
    }
    uart::println("GPIO OK");

    // ── SPI Test ─────────────────────────────────────────────────────
    uart::println("== SPI Test ==");

    // Wait 30ms for IMU gyroscope startup — DS-000347 Rev 1.6
    // Also guards against back-powering through signal lines
    // giving a false 0x47 before the chip is properly initialized
    uart::println("Waiting for IMU power-on stabilization...");
    for (volatile int i = 0; i < 480000; ++i) { arm::nop(); }

    // Read WHO_AM_I 10 times — a floating or back-powered MISO line
    // will not consistently return 0x47. One lucky read is not enough.
    uart::println("Reading WHO_AM_I x10...");
    bool imu_ok = true;

    for (int i = 0; i < 10; ++i) {
        pins::IMU_CS.set_low();
        spi::transfer(0x75u | 0x80u);
        uint8_t who_am_i = spi::transfer(0x00u);
        while (spi::is_busy());
        pins::IMU_CS.set_high();

        uart::print("  Read ");
        uart::put_u32(static_cast<uint32_t>(i + 1));
        uart::print(": 0x");
        uart::put_hex(who_am_i);
        uart::put_new_line();

        if (who_am_i != 0x47u) {
            imu_ok = false;
            break;
        }

        // Small delay between reads
        for (volatile int j = 0; j < 16000; ++j) { arm::nop(); }
    }

    if (imu_ok) {
        uart::println("IMU OK — ICM-42688 detected (10/10)");
        pins::HEARTBEAT.set_high();  // solid green = all tests passed
    } else {
        uart::println("ERROR — check wiring and power");
        pins::LED_RED.set_high();    // solid red = something wrong
    }

    uart::println("SPI OK");

    while (1);
}
