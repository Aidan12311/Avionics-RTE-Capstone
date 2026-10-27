#ifndef AVIONICS_RTE_ICM42688_DRIVER_HPP
#define AVIONICS_RTE_ICM42688_DRIVER_HPP

namespace icm42688 {
    constexpr float ACCEL_SENSITIVITY = 16384.0f; // LSB/g for ±2g
    constexpr float GYRO_SENSITIVITY  = 131.0f;   // LSB/(°/s) for ±250°/s

    struct AccelData {
        float x;
        float y;
        float z;
    };

    struct GyroData {
        float x;
        float y;
        float z;
    };

    struct SensorData {
        AccelData accel;
        GyroData gyro;
    };

    bool init();

    SensorData read_all();
} // namespace icm42688

#endif // AVIONICS_RTE_ICM42688_DRIVER_HPP
