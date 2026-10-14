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

    uart::println("Boot OK");

    uart::println("== UART Test ==\n");

    uart::println("put_u32: ");
    uart::put_u32(115200u);
    uart::put_new_line();

    uart::println("put_hex: ");
    uart::put_hex(0xABu);
    uart::put_new_line();

    uart::println("UART OK\n");

    uart::println("== GPIO Test ==\n");
    for(int i = 0; i < 3; ++i) {
        pins::HEARTBEAT.set_high();
        for(volatile int j = 0; j < 500000; ++j) { arm::nop(); }

        pins::HEARTBEAT.set_low();
        for(volatile int j = 0; j < 500000; ++j) { arm::nop(); }
    }

    uart::println("GPIO OK\n");

    uart::println("== SPI Test ==\n");
    pins::IMU_CS.set_low();
    spi::transfer(0x75u | 0x80u);
    uint8_t who_am_i = spi::transfer(0x00u);

    pins::IMU_CS.set_high();

    uart::println("WHO_AM_I: ");
    uart::put_hex(who_am_i);
    uart::put_new_line();

    if(who_am_i == 0x47u) {
        uart::println("ICM-42688 detected");
    }
    else {
        uart::println("ICM-42688 NOT detected");
    }
    uart::println("SPI OK\n");

    while(1);
}
