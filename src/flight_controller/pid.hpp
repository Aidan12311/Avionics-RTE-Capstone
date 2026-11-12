#ifndef AVIONICS_RTE_PID_HPP
#define AVIONICS_RTE_PID_HPP

namespace pid {
    struct Gains {
        double kp{};
        double ki{};
        double kd{};
    };

    class PIDController {
    public:
        PIDController(const Gains& gains, float output_limit, float integral_limit=1.0e9f)
            : _gains(gains), _output_limit(output_limit), _integral_limit(integral_limit) {}

        float update(
            float setpoint,
            float angle,
            float rate,
            float dt
        ) {
            const float error = setpoint - angle;
            const float output = _gains.kp * error + _gains.ki * _integral - _gains.kd * rate;

            const float clamped_output = clamp(
                output,
                -_output_limit,
                _output_limit
            );

            const bool at_limit = clamped_output != output;
            const bool pushing_further =
                (clamped_output >= _output_limit && error > 0.0f) ||
                (clamped_output <= -_output_limit && error < 0.0f);
            
            const bool within_integral_limit =
                (error < _integral_limit) && (error > -_integral_limit);

            if(within_integral_limit && !(at_limit && pushing_further)) {
                _integral = clamp(
                    _integral + error * dt,
                    -_output_limit,
                    _output_limit
                );
            }

            return clamped_output;
        }

        void reset() { _integral = 0.0f; }
        
    private:
        Gains _gains{};
        float _output_limit{};
        float _integral_limit{};
        float _integral{0.0f};

        static float clamp(float value, float min, float max) {
            if (value < min) return min;
            if (value > max) return max;

            return value;
        }
    };
} // namespace pid

#endif // AVIONICS_RTE_PID_HPP
