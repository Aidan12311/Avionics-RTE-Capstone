# Avionics RTE

A from-scratch real-time executive for an ARM Cortex-M4F that reads a live IMU, estimates the board's attitude, and streams it to a flight simulator, so tilting the board in your hand flies a simulated aircraft.

Written bare-metal in C++17 and Thumb-2 assembly on an STM32F429I-DISC1, with no vendor HAL, no CMSIS, no heap and no off-the-shelf RTOS. The whole firmware is under 10 KB of flash.

## How it works

```
ICM-42688 --SPI--> sensor_task --> estimator_task --> telemetry_task --UART--> PC bridge --UDP--> X-Plane
                    (1 ms)          (10 ms)            (50 ms)                 (Python)
```

A preemptive, priority-based scheduler runs three periodic tasks plus an idle task. The sensor task reads the IMU at 1 kHz. The estimator fuses accelerometer and gyro data into roll and pitch. The telemetry task streams the result over UART. A Python bridge turns that into UDP packets that position the aircraft in X-Plane.

## Status

| Component | State |
|---|---|
| Toolchain, startup code, linker script, vector table | Verified on hardware |
| Preemptive scheduler and FPU-aware context switcher | Verified on hardware (FPU stress test, 20,000+ task wakeups, 0 errors) |
| Interrupt-driven UART, SPI, ICM-42688 driver | Verified on hardware |
| Sensor → estimator → telemetry pipeline, complementary filter | Written and run in an emulator; hardware verification pending |
| X-Plane link | Spike verified (UDP read/write, attitude override, forward motion); PC bridge in progress |
| Autopilot mode (PID + dynamics model) | Planned |

## Tech stack

- **Languages and toolchain:** C++17 (`-ffreestanding -fno-exceptions -fno-rtti -nostdlib`), ARM Thumb-2 assembly, GNU Arm Embedded Toolchain 13.2 (`arm-none-eabi-g++`), GNU Make
- **Hardware:** STM32F429I-DISC1 (Cortex-M4F, currently on the 16 MHz internal oscillator), ICM-42688-P 6-axis IMU over SPI Mode 3
- **Debug and flash:** OpenOCD, GDB over ST-Link/SWD, picocom, WSL2 with usbipd-win
- **Host side:** Python 3 UDP bridge, X-Plane 12 (DREF/RREF interface)

## Design decisions

| Decision | Why |
|---|---|
| Fixed-priority preemptive scheduling, priorities by rate (shorter period = higher priority) | The standard approach for periodic tasks (Liu & Layland, 1973) and easy to reason about. |
| SysTick decides, PendSV switches, both at the lowest priority | A switch can never land in the middle of another ISR, and the tick and the switch can never interrupt each other. |
| Every new task starts with a fabricated exception frame | First run and every later resume take one code path, so there is no special case in the switcher. |
| PendSV saves each task's `EXC_RETURN` and the FPU's `s16`-`s31` only when the task used the FPU | A switcher that ignores the FPU corrupts registers once tasks mix float and non-float code. The first version failed most checks in an emulator stress test, and the fixed one had 0 errors there and on hardware. |
| Task stacks in CCM RAM, 8-byte aligned, with canaries checked every tick | Fast CPU-only RAM, and stack overflow is detected instead of silently corrupting memory. |
| UART TX is interrupt-driven through a lock-free single-producer ring buffer | Tasks never wait on a wire that takes 87 µs per byte. |
| `Latest<T>` mailbox with an RAII interrupt-masking lock | A 6-float struct isn't copied atomically, so a preempted reader could see half-old, half-new data. The masked window is about a microsecond. |
| Measured before putting `std::optional` in the UART ISR | I traced the ISR instruction by instruction and converted it to cycles with ARM's published timings. That showed +1.4% CPU at `-O0` and almost nothing at `-O1` and above, so the change was approved by numbers. |
| Register-level HAL with `constexpr` pin objects defined once in `pins.hpp` | No magic addresses scattered through the code, and pin configuration is checked at compile time. |
| Complementary filter first, Kalman filter as a stretch goal | It's explainable in one sentence and adequate for hand-tilting. Its time step comes from the task period, so the two can't disagree. |
| Moved from an emulator to real hardware once the board arrived | The emulator modeled neither the IMU nor the clock tree. Hardware exposed real issues, for example that the ST-Link serial port is on USART1/PA9, not USART2. |
| X-Plane is driven kinematically (physics overridden, roll/pitch/position written) | The IMU gives attitude but not heading or speed, so bank angle drives a coordinated-turn heading and the aircraft visibly flies. |

## Hardware wiring

| IMU pin | STM32 pin |
|---|---|
| VCC | 3V |
| GND | GND |
| ADO / MISO | PA6 |
| SDA / MOSI | PA7 |
| SCL / SCLK | PA5 |
| CS | PA8 |
| INT1, INT2 | unconnected |

## Build and run

```bash
make                                    # builds bin/firmware.elf
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
        -c "program bin/firmware.elf verify reset exit"
picocom -b 115200 /dev/ttyACM0          # telemetry: ATT,<ms>,<roll>,<pitch>
```

Under WSL2, attach the board first with `usbipd attach --wsl --busid <id>`.

## Layout

```
src/startup/    vector table, reset handler
src/hal/        registers, GPIO, UART, SPI, ring buffer
src/drivers/    ICM-42688 driver
src/filters/    complementary filter
src/rte_core/   scheduler, tasks, context switcher (assembly), Latest<T>
src/tasks/      sensor, estimator, telemetry tasks
```

## What this demonstrates

- **It builds the layers most projects import:** startup code, linker script, vector table, scheduler, context switcher and peripheral drivers, written against the datasheets and reference manual.
- **It works at the hardware/software boundary:** Thumb-2 assembly, exception frames, FPU stacking, NVIC priorities and memory-mapped registers.
- **It uses modern C++ in a constrained environment:** `constexpr`, RAII, templates and `std::optional`, with no exceptions, RTTI or heap.
- **It treats correctness as something to measure:** an FPU stress test, emulator-based checks, instruction-level ISR cost analysis, and a hardware-in-the-loop link to a real flight simulator.
- **It's end to end:** a physical sensor feeds real-time estimation and drives a live simulation.
