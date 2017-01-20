/* robot_specific.h
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

#ifndef JDH_16_17_RASPI3_ROBOT_SPECIFIC_H_
#define JDH_16_17_RASPI3_ROBOT_SPECIFIC_H_

#include "mbed.h"

extern const double kWheelDiameter;
extern const double kRobotDiameter;

extern const int kStepsPerRotation;
extern const int kAccelDecelSteps;
extern const int kInitialPwmPeriod;
extern const int kPwmPeriodIncrement;
extern DigitalOut right_wheel_direction;
extern DigitalOut left_wheel_direction;
void LeftWheelForward();
void LeftWheelBack();
void RightWheelForward();
void RightWheelBack();

extern const double kStepsPerCentimetre;
extern const double kStepsPerDegree;
extern PwmOut pwm;
extern Serial usb_serial;
extern Ticker ticker;

#endif  // JDH_16_17_RASPI3_ROBOT_SPECIFIC_H_