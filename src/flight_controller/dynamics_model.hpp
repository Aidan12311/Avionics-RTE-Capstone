#ifndef AVIONICS_RTE_DYNAMICS_MODEL_HPP
#define AVIONICS_RTE_DYNAMICS_MODEL_HPP

#include <algorithm>

namespace aircraft {
    struct AxisParams {
        // NOTE: Rate gain
        float k{};

        // NOTE: Time constant
        float tau{};
    };

    class AxisModel {
    public:
        explicit AxisModel(const AxisParams& params) :
            _params(params) {}

        void set_disturbance(float disturbance) {
            _disturbance = disturbance;
        }

        void reset(float angle, float rate=0.0f) {
            _angle = angle;
            _rate = rate;
        }

        void step(float stick, float dt) {
            const float clamped_stick = std::clamp(stick, -1.0f, 1.0f);
            const float target_rate = _params.k * clamped_stick + _disturbance;

            _rate += dt * (target_rate - _rate) / _params.tau;
            _angle += dt * _rate;
        }

        float get_angle() const {
            return _angle;
        }

        float get_rate() const {
            return _rate;
        }

    private:
        AxisParams _params{};
        
        float _angle{0.0f};
        float _rate{0.0f};
        
        // NOTE: Optional disturbance
        float _disturbance{0.0f};
    };
} // namespace aircraft

#endif // AVIONICS_RTE_DYNAMICS_MODEL_HPP
