#include <cstdint>

#include "src/hal/spi.hpp"
#include "src/hal/pins.hpp"
#include "src/hal/arm_core.hpp"
#include "src/drivers/icm42688.hpp"

namespace {
        // ── Register addresses ────────────────────────────────────────────
    // Source: DS-000347 Rev 1.6, Register Map
    constexpr uint8_t REG_WHO_AM_I    = 0x75u;  // identity register
    constexpr uint8_t REG_PWR_MGMT0   = 0x4Eu;  // power management
    constexpr uint8_t REG_GYRO_CFG0   = 0x4Fu;  // gyro ODR and range
    constexpr uint8_t REG_ACCEL_CFG0  = 0x50u;  // accel ODR and range
    constexpr uint8_t REG_ACCEL_DATA  = 0x1Fu;  // first data byte

    // ── Expected identity value ───────────────────────────────────────
    // Source: DS-000347 Rev 1.6, Register 0x75
    constexpr uint8_t WHO_AM_I_VALUE    = 0x47u;

    // ── PWR_MGMT0 configuration ───────────────────────────────────────
    // Bits 3:2 = ACCEL_MODE: 0b11 = low noise mode
    // Bits 1:0 = GYRO_MODE:  0b11 = low noise mode
    constexpr uint8_t PWR_ACCEL_GYRO_ON = 0x0Fu;

    // ── GYRO_CONFIG0 ──────────────────────────────────────────────────
    // Bits 7:5 = FS_SEL: 0b111 = ±250°/s → sensitivity 131 LSB/°/s
    // Bits 3:0 = ODR:    0b0110 = 1kHz
    constexpr uint8_t GYRO_250DPS_1KHZ = 0b01100110u;

    // ── ACCEL_CONFIG0 ─────────────────────────────────────────────────
    // Bits 7:5 = FS_SEL: 0b111 = ±2g → sensitivity 16384 LSB/g
    // Bits 3:0 = ODR:    0b0110 = 1kHz
    constexpr uint8_t ACCEL_2G_1KHZ    = 0b01100110u;

    // ── Gyroscope startup time ────────────────────────────────────────
    // Source: DS-000347 Rev 1.6, Table 2 — 30ms minimum after power-on
    constexpr uint32_t GYRO_STARTUP_DELAY_CYCLES = 480000u;  // ~30ms at 16MHz

    uint8_t read_reg(uint8_t reg_addr) {
        pins::IMU_CS.set_low();

        spi::transfer(reg_addr | 0x80u);
        uint8_t read_value = spi::transfer(0x00u);

        while(spi::is_busy());
        pins::IMU_CS.set_high();

        return read_value;
    }

    void write_reg(uint8_t reg_addr, uint8_t value) {
        pins::IMU_CS.set_low();

        spi::transfer(reg_addr & 0x7Fu);
        spi::transfer(value);

        while(spi::is_busy());
        pins::IMU_CS.set_high();
    }

    void read_consecutive_regs(
        uint8_t start_addr,
        uint8_t *buffer,
        uint8_t buffer_len
    ) {
        pins::IMU_CS.set_low();

        spi::transfer(start_addr | 0x80u);
        for(uint8_t i = 0u; i < buffer_len; ++i) {
            buffer[i] = spi::transfer(0x00u);
        }

        while(spi::is_busy());
        pins::IMU_CS.set_high();
    }

    void delay_cycles(uint32_t cycles) {
        for(volatile uint32_t i = 0u; i < cycles; ++i) {
            arm::nop();
        }
    }
} // anonymous namespace


bool icm42688::init() {
    if(read_reg(REG_WHO_AM_I) != WHO_AM_I_VALUE) {
        return false;
    }

    delay_cycles(GYRO_STARTUP_DELAY_CYCLES);

    write_reg(REG_GYRO_CFG0, GYRO_250DPS_1KHZ);
    write_reg(REG_ACCEL_CFG0, ACCEL_2G_1KHZ);

    write_reg(REG_PWR_MGMT0, PWR_ACCEL_GYRO_ON);

    delay_cycles(GYRO_STARTUP_DELAY_CYCLES);

    return true;
}

icm42688::SensorData icm42688::read_all() {
    constexpr uint8_t data_buffer_len = 12; 
    uint8_t data_buffer[data_buffer_len]{};

    read_consecutive_regs(REG_ACCEL_DATA, data_buffer, data_buffer_len);

    auto coalesce_bytes = [](uint8_t high, uint8_t low) -> int16_t {
        return static_cast<int16_t>(static_cast<uint16_t>(high) << 8u | low);
    };

    int16_t ax_raw = coalesce_bytes(data_buffer[0], data_buffer[1]);
    int16_t ay_raw = coalesce_bytes(data_buffer[2], data_buffer[3]);
    int16_t az_raw = coalesce_bytes(data_buffer[4], data_buffer[5]);
    int16_t gx_raw = coalesce_bytes(data_buffer[6], data_buffer[7]);
    int16_t gy_raw = coalesce_bytes(data_buffer[8], data_buffer[9]);
    int16_t gz_raw = coalesce_bytes(data_buffer[10], data_buffer[11]);

    auto normalize = [](int16_t raw_axis_value, float sensitivity) -> float {
        return static_cast<float>(raw_axis_value) / sensitivity;
    };

    return {
        .accel = {
            .x = normalize(ax_raw, ACCEL_SENSITIVITY),
            .y = normalize(ay_raw, ACCEL_SENSITIVITY),
            .z = normalize(az_raw, ACCEL_SENSITIVITY),
        },
        .gyro = {
            .x = normalize(gx_raw, GYRO_SENSITIVITY),
            .y = normalize(gy_raw, GYRO_SENSITIVITY),
            .z = normalize(gz_raw, GYRO_SENSITIVITY),
        },
    };
}
