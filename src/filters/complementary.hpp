#ifndef AVIONICS_RTE_COMPLEMENTARY_FILTER_HPP
#define AVIONICS_RTE_COMPLEMENTARY_FILTER_HPP

#include <cstdint>

namespace filter {
    struct Attitude {
        float roll;
        float pitch;
    };

    class ComplementaryFilter {
    public:
        Attitude update(
            float ax, float ay, float az,
            float gx, float gy,
            float dt
        );

        Attitude attitude() const {
            return _attitude;
        }

        void reset() {
            _attitude = {};
            _is_initialize = false;
        }

    private:
        bool _is_initialize{};
        Attitude _attitude{};

        // NOTE: Tunable parameter
        static constexpr float ALPHA = 0.98f;
    };
} // namespace filter

#endif // AVIONICS_RTE_COMPLEMENTARY_FILTER_HPP
