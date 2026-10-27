#include "src/hal/uart.hpp"
#include "src/hal/spi.hpp"
#include "src/hal/gpio.hpp"
#include "src/hal/pins.hpp"
#include "src/drivers/icm42688.hpp"

int main() {
    uart::init();
    spi::init();
    pins::HEARTBEAT.configure_output();
    pins::LED_RED.configure_output();
    pins::HEARTBEAT.set_low();
    pins::LED_RED.set_low();

    if (!icm42688::init()) {
        uart::println("IMU init failed");
        pins::LED_RED.set_high();
        while (1);
    }

    uart::println("IMU OK — streaming data");
    pins::HEARTBEAT.set_high();

    while (1) {
        icm42688::SensorData data = icm42688::read_all();

        uart::print("ax="); uart::put_float(data.accel.x);
        uart::print(" ay="); uart::put_float(data.accel.y);
        uart::print(" az="); uart::put_float(data.accel.z);
        uart::print(" gx="); uart::put_float(data.gyro.x);
        uart::print(" gy="); uart::put_float(data.gyro.y);
        uart::print(" gz="); uart::put_float(data.gyro.z);
        uart::put_new_line();

        // ~100ms delay between prints so output is readable
        for (volatile int i = 0; i < 1600000; ++i);
    }
}
