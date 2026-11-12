#include "src/filters/complementary.hpp"

namespace {
    constexpr float PI         = 3.14159265358979323846f;
    constexpr float RAD_TO_DEG = 180.0f / PI;

    float fsqrtf(float x) {
        float result{};
        __asm__ volatile("vsqrt.f32 %0, %1" : "=w"(result) : "w"(x));
    
        return result;
    }

    float fatan2f(float y, float x) {
        constexpr float HALF_PI = PI / 2.0f;

        const float abs_x = (x < 0.0f) ? -x : x;
        const float abs_y = (y < 0.0f) ? -y : y;

        if(abs_x == 0.0f && abs_y == 0.0f) {
            return 0.0f;
        }

        const bool  swapped = abs_y > abs_x;
        const float z       = swapped ? abs_x / abs_y : abs_y / abs_x;

        float angle = z / (1.0f + 0.28f * z * z);

        if(swapped)  { angle = HALF_PI - angle; }
        if(x < 0.0f) { angle = PI - angle; }
        if(y < 0.0f) { angle = -angle; }

        return angle;
    }
    
    constexpr float to_degrees(float radians) {
        return radians * RAD_TO_DEG;
    }

    constexpr float predict(float prev_angle, float rate, float dt) {
        return prev_angle + rate * dt;
    }

    constexpr float fuse(float gyro, float accel_angle, float alpha) {
        return alpha * gyro + (1.0f - alpha) * accel_angle;
    }
} // anonymous namespace

namespace filter {
    Attitude ComplementaryFilter::update(
        float ax, float ay, float az,
        float gx, float gy,
        float dt
    ) {
        float accel_roll  = to_degrees(fatan2f(ay, az));
        float accel_pitch = to_degrees(fatan2f(ax, fsqrtf(ay * ay + az * az)));

        if(!_is_initialized) {
            _attitude.roll  = accel_roll;
            _attitude.pitch = accel_pitch;

            _is_initialized = true;

            return _attitude;
        }

        _attitude.roll  = fuse(predict(_attitude.roll, gx, dt), accel_roll, ALPHA);
        _attitude.pitch = fuse(predict(_attitude.pitch, -gy, dt), accel_pitch, ALPHA);

        return _attitude;
    }
} // namespace filter
