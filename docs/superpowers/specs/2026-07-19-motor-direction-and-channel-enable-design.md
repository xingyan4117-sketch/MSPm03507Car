# Motor Direction and Per-Channel Enable Design

## Goal

Define motor direction in vehicle coordinates, normalize encoder feedback, and
allow A/B/C/D to be enabled independently with direct channel names throughout.

## Channel and Direction Contract

- A, B, C, and D always name their corresponding software and D24A channels.
- Positive target RPM means vehicle forward for every channel.
- The A/C and B/D motor shafts rotate in opposite physical directions because
  the motors are mounted as mirrored sides of the vehicle. This is expected.
- The existing output polarity that drives the vehicle forward is retained.
- B/D encoder deltas and RPM values are sign-normalized so vehicle-forward
  motion is positive on all four channels. A/C feedback remains unchanged.

## Root Cause

All four PWM outputs use the same 16 MHz timer clock, 800-count period, 20 kHz
frequency, and compare conversion. The observed A/C-versus-B/D speed grouping
is therefore not caused by PWM frequency grouping. The current speed controller
uses proportional gain 1 with zero integral and derivative gains. A negative
forward encoder measurement on the mirrored side increases error instead of
closing the loop, which explains the faster B/D group. Pure proportional control
can still leave smaller steady-state differences after sign normalization.

## Per-Channel Enable Model

- `AppMotorCommand` owns a four-bit enable mask with one bit per A/B/C/D channel.
- The default mask enables all four channels. Startup remains in READY, so this
  does not start any motor automatically.
- The motor submenu contains `RUN`, `SPEED`, `A:ON`, `B:ON`, `C:ON`, `D:ON`,
  and `BACK`.
- In a channel item, KEY1 enables, KEY2 disables, and KEY3 exits edit mode.
- A disabled channel has zero duty and its PID state is reset every control
  cycle. Other channels continue normally.
- Re-enabling a channel while RUN is active resumes it at the current common
  target speed from a reset PID state.
- Emergency stop, fault, timeout, and READY behavior continue to stop all four
  channels regardless of the enable mask.

## Data Flow

The command task edits the mask and publishes it with the existing command.
MotorControl copies the mask into status for display, skips PID update for each
disabled channel, and emits zero duty for it. The PWM and GPIO BSP interfaces do
not gain a second enable state; zero duty remains the single hardware stop path.

## Verification

- Command host tests cover default all-enabled state and independent KEY1/KEY2
  changes without affecting neighboring bits.
- Application host tests prove disabled channels reset PID and emit zero duty
  while enabled channels continue.
- Encoder host tests prove B/D feedback is sign-normalized and A/C is unchanged.
- Screen tests prove all four `A/B/C/D:ON/OFF` states render in the motor menu.
- Documentation tests enforce direct A/B/C/D naming.
- Full verification must pass all PowerShell tests and a Keil rebuild with zero
  errors and zero warnings.

## Hardware Acceptance

After flashing, test one enabled channel at a time at low speed. Vehicle-forward
motion must report positive feedback for every channel. A/C and B/D shafts are
expected to rotate oppositely because of mirrored installation. Final PID gain
tuning is separate from this direction and enable correction.
