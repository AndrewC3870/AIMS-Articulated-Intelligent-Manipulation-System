#pragma once
#include <AccelStepper.h>

// Drive one joint into its limit switch, back off, and zero its position.
void homeJoint(AccelStepper* stepper, int limitPin, int direction);

// After homing, move by the SAFE_HOME_ANGLES offset and call THAT the new zero.
void moveOffsetAfterHoming(AccelStepper* stepper, int jointIndex, float angleOffset);

void executeHardwareHome();
