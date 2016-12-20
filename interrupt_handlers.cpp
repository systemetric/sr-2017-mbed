/* interrupt_handlers.cpp
 *
 * Copyright 2016 Josh Holland
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "interrupt_handlers.h"

#include "mbed.h"

#include "robot_specific.h"
#include "pwm_constants.h"

enum State {
    kReadyForCommand,
    // k*Received means "waiting for argument(s)", i.e. how far to move
    kForwardReceived,
    kBackwardReceived,
    kLeftReceived,
    kRightReceived
};

volatile int steps_gone = 0;
volatile int steps_left = 0;
volatile int pwm_period = 0;
volatile int robot_state = kReadyForCommand;

// DigitalOuts are inherently "volatile", so they don't need to be explicitly
// declared as such. If they are, things break.
DigitalOut led_scale_speed(LED1);
DigitalOut led_pwm_interrupt(p11);
DigitalOut led_accelerate(p12);
DigitalOut led_decelerate(p13);
DigitalOut led_left(LED2);
DigitalOut led_zero_distance_received(LED3);
DigitalOut led_right(LED4);

void PwmHandler() {
    // Check whether a match 4 interrupt is pending (we're only interested if it is).
    if (LPC_PWM1->IR & PWM_IR_MR4) {
        led_pwm_interrupt = !led_pwm_interrupt;
        steps_gone++;
        steps_left--;
        if (steps_left <= 0) {
            pwm.pulsewidth_ms(0);  // disable PWM by setting duty cycle to 0
            usb_serial.putc('d');  // say we're done driving
        }
        LPC_PWM1->IR = PWM_IR_MR4;  // clear the interrupt flag (yes, by writing a 1 to it)
    } else {
        // Clear all interrupts, just in case one we aren't interested in fired.
        LPC_PWM1->IR = PWM_IR_ALL;
    }
}

// Alters the current speed of the robot to perform acceleration and
// deceleration. This should be called frequently (perhaps every 0.05 seconds).
// The more often it is called, the faster the robot will change speed.
void ScaleSpeed() {
    led_scale_speed = !led_scale_speed;  // Toggle an LED every timer tick.

    if (steps_left == 0) {
        led_accelerate = 0;
        led_decelerate = 0;
        return;  // No point scaling speed when not moving.
    }

    if (steps_gone < kAccelDecelSteps) {
        // not gone far, probably accelerate
        if (steps_left < steps_gone) {
            // more than halfway there (but still close to start), decelerate
            led_accelerate = 0;
            led_decelerate = 1;
            pwm_period += kPwmPeriodIncrement;
        } else {
            // less than halfway there (and not gone far), accelerate
            led_accelerate = 1;
            led_decelerate = 0;
            pwm_period -= kPwmPeriodIncrement;
        }
    } else if (steps_left < kAccelDecelSteps) {
        // almost done moving, decelerate
        led_accelerate = 0;
        led_decelerate = 1;
        pwm_period += kPwmPeriodIncrement;
    } else {
        // In the middle of movement, don't change speed.
        led_accelerate = 1;
        led_decelerate = 1;
    }

    pwm.period_us(pwm_period);
    // Make sure there is both a rising and falling edge at some point in the
    // cycle.
    pwm = 0.5;
}

// Consumes one character from the serial connection (if possible), and acts
// upon it.
void SerialHandler() {
    if (!usb_serial.readable()) {
        // There's not actually anything available to read(!), abort.
        return;
    }
    char character = usb_serial.getc();
    switch (robot_state) {
        case kReadyForCommand:
            switch (character) {
                case 'f':
                    robot_state = kForwardReceived;
                    break;
                case 'b':
                    robot_state = kBackwardReceived;
                    break;
                case 'l':
                    led_left = 1;
                    led_right = 0;
                    robot_state = kLeftReceived;
                    break;
                case 'r':
                    led_right = 1;
                    led_left = 0;
                    robot_state = kRightReceived;
                    break;
            }
            break;
        // To the next guy: yes, this sucks. The important thing is it works.
        case kForwardReceived:
            if (character == 0) {
                led_zero_distance_received = 1;
            }
            steps_left = character * kStepsPerCentimetre;
            steps_gone = 0;
            LeftWheelForward();
            RightWheelForward();
            pwm_period = kInitialPwmPeriod;
            pwm.period_us(pwm_period);
            pwm = 0.5;
            robot_state = kReadyForCommand;
            break;
        case kBackwardReceived:
            if (character == 0) {
                led_zero_distance_received = 1;
            }
            steps_left = character * kStepsPerCentimetre;
            steps_gone = 0;
            LeftWheelBack();
            RightWheelBack();
            pwm_period = kInitialPwmPeriod;
            pwm.period_us(pwm_period);
            pwm = 0.5;
            robot_state = kReadyForCommand;
            break;
        case kLeftReceived:
            if (character == 0) {
                led_zero_distance_received = 1;
            }
            steps_left = character * kStepsPerDegree;
            steps_gone = 0;
            LeftWheelBack();
            RightWheelForward();
            pwm_period = kInitialPwmPeriod;
            pwm.period_us(pwm_period);
            pwm = 0.5;
            robot_state = kReadyForCommand;
            break;
        case kRightReceived:
            if (character == 0) {
                led_zero_distance_received = 1;
            }
            steps_left = character * kStepsPerDegree;
            steps_gone = 0;
            LeftWheelForward();
            RightWheelBack();
            pwm_period = kInitialPwmPeriod;
            pwm.period_us(pwm_period);
            pwm = 0.5;
            robot_state = kReadyForCommand;
            break;
    }
}