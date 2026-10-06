# Preface
**HEAVY WIP**

This document is **NOT** a project reference. This is a simple collection of notes that **I** think are useful for understanding the underlying structures that make this project what it is. The reason I emphasize that **I** think these are useful is because I might omit some information that I take for granted. This means it might omit critical information the reader could reasonably not know.

# Introduction
This project is a bare-metal real-time executive/partial RTOS (real-time operating system) microkernel for simulated avionics hardware for an in-flight aircraft. It is written in C++17 and ARM Thumb-2 assembly on a STM32F49 discovery board with a Cortex-M4F MCU. 

This project runs directly on the bare-metal hardware and built completely from scratch with **zero** abstraction layers, standard libraries, or any underlying operating system.

# Project Structure
The project is composed of these various components:
* Startup code/bootloader (vector table setup, reset handler configuration, and linker, etc).

* Custom hardware abstraction library for GPIO, UART, SPI, etc, written completely in C++17.

* Core partial microkernel/real-time executive that implements a rate-monotonic scheduler, context switcher, etc, all written completely in ARM assembly.

* SPI driver for the IMU (inertial measurement unit) sensor. The driver handles the reads integers from the ICM-242688-P hardwares accelerometer and gyroscope at 1KhZ with SPI. Additionally, we handle WHO_AM_I to authenticate the bytes.

* Sensor fusion calculations to produce clean roll and pitch estimations from the noisy SPI driver output ran through a complementary filter/kalman filter clean up the data to send to the PID controller.

* PID (proportional-integral-derivative) controller for roll and pitch to compute errors from our sensor fusion calculations which tries to reach our desired state by sending commands to actuators (ailerons, elevators, etc) to fix the errors.

* Telemetry transmission via an interrupt-driven UART transmitter which streams frames of information we've received from our control loop. Information includes rol, pitch, commands, per-task worst-case execution time, deadline miss count, etc to my work laptop.

* A concise PC dashboard that simulates a live artifical horizon, sensor graph comparisons, and other information.

# Content
(links to individual sections)

# Domain Primer
**TOC HERE**

## Avionics
Avionics (aviation + electronics) is the collection of systems that run on airplanes (measuring, computing, communicating, controlling, etc). Aivonics is one of the most strict domains of software engineering. It is considered DAL-A (design assurance level A) software and it is governed by various rigorous development standards (``DO-178C DAL``, ``DO-254``, etc) that the code must adhere to, without question.

## Mission-critical code
Avionics software is considered "mission-critical" (DAL-A software) which means that bugs and failures in the code have irreversible real-world impacts that can possibly cause physical harm, so it absolutely needs to be deterministically and verifably correct. Mission-critical code requires things like:

* **Determinism**: Every execution path must run in a bounded frame of time. This **prohibits** ``recursion, garbage collection, and most applications of dynamic memory allocation``. This code runs on components that run in a control loop, each step requires the last step to be fully completed before moving on, **there CANNOT be ANY fluxation** in how long an execution path takes.

* **Provability**: The code must be written in a way that can make assertions on the code. You need to be able to **PROVE** that an execution path can be completed in a frame of time, that function ``F`` can work for **ALL** inputs. Standards **REQUIRE** every single piece of code be analyzed and proven to work.

* **Fault Isolation**: Failures **must not** propagate to other components in the system. The entire system **must remain online**, even if a component has a minor failure or slight miscalculation.

## Sensors
Sensors are **physical devices that take real-world data and converts them into electrical signals that the computer can understand**. This real-world data could be things like temperatures, pressure readings, motion readings, voltage, etc, or in our case it will quantities of acceleration and angular rate read from the IMU sensors.

The project contains two physical IMUs (inertial measurement unit) sensors: the ``accelerometer`` and the ``gyroscope``.

## Actuators
Actuators are the **reverse** of sensors. They convert electrical signals from the computer and turn them into physical actions. Some avionic actuators include ailerons, elevators, and rudders. 

This project actually simulates these aileron and elevator commands by transmitting values (via UART) to a dashboard on the main PC. This project mainly cares about simulating in-flight situations, so implementing commands for rudders (yaw) aren't particularly useful. This is because yaw/rudders are used for nose driving on the runway, it's not used in the actual flight process.

## Control loop
A control loop is continuously taking measurements, comparing it to some desired state or goal, and adjusts parameters, then closes the gap. This is the heart of the actual project, it is simply a control loop running in real-time on a microcontroller. 

For example, a simple control loop would be something like a thermostat; you set some setpoint (some desired state) that your thermostat is trying to reach, you compare some measurements to the thermostat, you do some work to move towards the setpoint, you compare, and the cycle continues. The loop looks like this: measure room temperature, compares it to the setpoint, turns heater/AC on/off, measures it again, continue. 

There will be a more fruitful introduction in the **concept deep-dive** below.

## Flight State Variables
A major portion of flight instruments, control surface commands, navigation commands utilize a small set of variables that describe the orientation and location of the aircraft. The set of variables are rotational angles that describe the orientation of the aircraft (aka "attitude") relative to some reference frame. The standard reference frame is known as the **NED** frame (north-east-down). North is forward, east is right, and town is towards the center of the Earth. The set of variables are known as **roll**, **pitch**, and **yaw**.

### Roll
**Roll** is the rotation around the **forward/north axis** (colloquially known as "nose-to-tail"). Essentially, this rotation describes the diagonal orientation of the aircraft (how it's tilting).

Here's what roll values means:
* **Zero roll**: Wings are level, the tips of the wings are at the **same altitude**.

* **Positive roll**: Right wing is down, the left wing is up, the aircraft is **turning right**.

* **Negative roll**: Left wing is down, right wing is up, the aircraft is **turning left**.

The ``aileron`` actuator is what controls the **roll** of our aircraft. Our IMU sensor is what gives us information about our roll orientation.

### Pitch
**Pitch** is the rotation around the **east/right axis**. Pitch being rotation about  the ``east`` axis (you might imagine it would be the *vertical* axis since it's up and down, but remember we're talking about **rotation**) is a little unintuitive at first, but makes more sense once you look at visualizations of it. Essentially, this rotation describes the vertical orientation of the aircraft (climbing/descending). 

Here's what pitch values mean:
* **Zero pitch**: The aircraft is **level and flying straight**.

* **Positive pitch**: The nose is up, the aircraft is **climbing** altitude.

* **Negative pitch**: The nose is down, the aircraft is **descending** altitude.

The ``elevator`` acutator is what controls the **pitch** of our aircraft. Our IMU also gives us information about our pitch orentation.

### Yaw
**Yaw** is the rotation around the **vertical axis**, once again, we're strictly talking **rotation** (think about how your hand moves when you rotate it vertically).

Here's what the yaw values mean:
* **Zero yaw**: Default reference direction (usually north or just "straight").
* **Positive yaw**: Nose is clockwise (right) relative to the reference direction.
* **Negative yaw**: Nose is counterclockwise (left) relative to the reference direction.

The ``rudder`` actuator is what controls the yaw. Our IMU can give a rough estiminate of yaw, but it's much more accurate for pitch and roll. We can integrate the raw yaw rate from the ``gyroscope``, but you with constants you drift from the actual correct yaw reading. You can't just use gravity and compute angles, you need some sort of GPS reference or a ``magnetometer`` to have proper readings.

**Yaw won't be included** as a part of our MVP (minimum viable project) because this project is mostly focusing on **in-air flight**, not simply driving around the runway.

# Process concurrency (context switching)
The MCU only comes with a **singular CPU core**, so we'll need code that allows tasks to interleave each one another to make progress in an optimized way. At any given moment (every second), our **task scheduler** will need to check the task list, select the highest priority task, save the current CPU stack, jump to the task, do some work, and jump back and forth.

The tasks we plan on completing and switching back and forth from would be tasks from our ``control loop``, which are as follows:

* **Sensor Task (1ms @ 1kHz) - takes 20µs-50µs**: Reads raw ``accelerometer`` and ``gyroscope`` data values and writes the data into a shared memory buffer.

* **Estimator/Filter Task (10ms @ 100Hz) - takes 5µs-10µs**: Reads the shared buffer (shared between the sensor task and the filter task) and runs the filter to produce clean data we can pass to the PID task. 

* **PID Controller Task (50ms @ 20Hz) - takes 5µ-10µs**: Reads the cleaned up data, calculates the difference of the current values with the setpoint, runs the correction calculations, write the results to the dynamic model (generates actuator commands).

* **Telemetry Streaming Task (100ms @ 10Hz) - takes (200µ-500sµ)**: Formats the data with a snapshot of the current RTE data (roll, pitch, actuator commands, per-task WCET, deadline count) over UART to another PC.

## Flight Controller (PID controller/loop)
The "flight controller" is a PID feedback loop, which essentially is a correction loop. The PID controller reads the orientation, computes how far it is from the goal (setpoint 0° for roll and pitch), and computes the adjustments accordingly, and sends updates the actual altitude changes to the dynamics model.

## Sensor Graph


## Deadlines


# Concept Deep-dive
**TOC HERE**
## Accelerometer
The accelerometer measures the net total force that's acting on the sensor, then reads the force into three discrete axes [X, Y, and Z] (where **x=front**, **y=right**, and **z=up**). You can see the parallels from this system and ``NED`` frames coordinates. Each variable is expressed in units of **gravitational acceleration**. 

Gravity is **a constant 1g force** pulling everything towards the center of gravity (the Earth) at all times. The only thing the sensor knows is force caused by gravity. We can compute certain information (specifically, orientation) about the sensor from it's readings on the axes being projected upon by gravity.

This is why when the sensor is still and level, the reading will produce ``[0g, 0g, 1g]``. Since it is still and level, gravity is not projecting upon ``x`` or ``y``, just ``z``, which is ``1g`` of force to combat the gravitational pull of the Earth (the internal spring needs to exert 1g of force to get to it's resting state). These readings become more interesting when change the orientation of the sensor.

### Tilting
When strictly *just* tilting, that ``1g`` of force gets distributed across the terms in our reading. *Before*, it was strictly the ``z`` variable that was taking the reading, but now we're tilting which affects the ``x`` variable.

Let's say we tilt the nose of the board up 30°. This causes that ``1g`` force to be distributed among the ``x`` variable. The reading will look like this: ``[-sin(30°)g, 0g, cos(30°)g] -> [-0.5g, 0, 0.866g]``, which will yield ``1g`` once we take the magnitude of the directional vector (``sqrt(0.5^2 + 0^2 + 0.866^2) = 1``). The gravitational force did not change, it just redistributed in a different form. We can compute the angle we're at when we tilt by using simple trigonometry from this information. 


### Capstone control loop
