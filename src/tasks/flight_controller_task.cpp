#include "src/hal/pins.hpp"
#include "src/rte_core/scheduler.hpp"

#include "src/tasks/estimator_task.hpp"
#include "src/tasks/flight_controller_task.hpp"

#include "src/flight_controller/pid_controller.hpp"
#include "src/flight_controller/dynamics_model.hpp"

namespace {
    constexpr float DT_SEC = static_cast<float>(controller_task::PERIOD_MS) / 1000.0f;

    constexpr aircraft::AxisParams ROLL_DYNAMICS  { 60.0f, 0.3f };
    constexpr aircraft::AxisParams PITCH_DYNAMICS { 30.0f, 0.4f };

    constexpr float NATURAL_FREQ = 1.5f; // rads/sec
    constexpr float DAMPING = 0.9f;

    constexpr float STICK_LIMIT = 1.0f; // [-1, 1]
    constexpr float INTEGRAL_ZONE_DEG = 0.5f;

    constexpr float SET_POINT = 0.0f; // level altitude

    // AI Citation: I had claude fine tune the PID gain parameters for time sake
    // The link between the two classes at design time: gains computed from the dynamics model's numbers.
    //   Kp = tau * wn^2 / K        Kd = (2 * zeta * wn * tau - 1) / K  (0 if negative)        Ki = 0.1 * Kp * wn
    constexpr pid_controller::Gains design_gains(const aircraft::AxisParams &dynamics) {
        const float kp = dynamics.time_constant * NATURAL_FREQ * NATURAL_FREQ / dynamics.rate_gain;
        const float kd = (2.0f * DAMPING * NATURAL_FREQ * dynamics.time_constant - 1.0f) / dynamics.rate_gain;
        
        return { kp, 0.1f * kp * NATURAL_FREQ, kd > 0.0f ? kd : 0.0f };
    }

    rte::Latest<controller_task::FlightState> latest_state;

    bool is_button_pressed() {
        return pins::USER_BUTTON.is_high() == pins::USER_BUTTON_PRESSED_IS_HIGH;
    }
} // anonymous namespace

void controller_task::run() {
    aircraft::AxisModel roll_model(ROLL_DYNAMICS);
    aircraft::AxisModel pitch_model(PITCH_DYNAMICS);

    pid_controller::PIDController roll_pid(design_gains(ROLL_DYNAMICS), STICK_LIMIT, INTEGRAL_ZONE_DEG);
    pid_controller::PIDController pitch_pid(design_gains(PITCH_DYNAMICS), STICK_LIMIT, INTEGRAL_ZONE_DEG);

    // NOTE: Start off in freefly mode
    FlightMode mode = FlightMode::FreeFly;
    bool was_button_pressed = is_button_pressed();

    while(true) {
        const bool just_pressed = is_button_pressed();
        if(just_pressed && !was_button_pressed) {
            mode = (mode == FlightMode::FreeFly) ? FlightMode::AutoPilot : FlightMode::FreeFly;
        }
        
        was_button_pressed = just_pressed;

        if(mode == FlightMode::FreeFly) {
            if(const auto board = estimator_task::output().read()) {
                roll_model.reset(board->roll);
                pitch_model.reset(board->pitch);

                roll_pid.reset();
                pitch_pid.reset();

                latest_state.publish({ board->roll, board->pitch, FlightMode::FreeFly, 0.0f, 0.0f });
            }
        }
        else {
            const float aileron = roll_pid.update(SET_POINT, roll_model.get_angle(), roll_model.get_rate(), DT_SEC);
            const float elevator = pitch_pid.update(SET_POINT, pitch_model.get_angle(), pitch_model.get_rate(), DT_SEC);

            roll_model.step(aileron, DT_SEC);
            pitch_model.step(elevator, DT_SEC);

            latest_state.publish({ roll_model.get_angle(), pitch_model.get_angle(), FlightMode::AutoPilot, aileron, elevator });
        }

        scheduler::task_block_until_next_period();
    }
}

const rte::Latest<controller_task::FlightState> &controller_task::output() {
    return latest_state;
}
