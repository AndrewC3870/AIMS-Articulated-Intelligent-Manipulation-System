#include "Homing.h"
#include <Arduino.h>
#include "Config.h"
#include "Globals.h"

// ==============================================================================
//  HARDWARE HOMING & OFFSETS
// ==============================================================================
void homeJoint(AccelStepper* stepper, int limitPin, int direction) {
  pinMode(limitPin, INPUT_PULLUP);

  stepper->setMaxSpeed(1000);
  stepper->setAcceleration(10000);
  stepper->move(1000000 * direction);

  while (digitalRead(limitPin) == LOW) {
    stepper->run();
    yield();
  }

  stepper->stop();
  while (stepper->distanceToGo() != 0) {
    stepper->run();
    yield();
  }

  stepper->setAcceleration(1000);

  stepper->move(-1200 * direction);
  while (stepper->distanceToGo() != 0) {
    stepper->run();
    yield();
  }

  stepper->setCurrentPosition(0);

  stepper->setMaxSpeed(6000);
  stepper->setAcceleration(1200);
}

void moveOffsetAfterHoming(AccelStepper* stepper, int jointIndex, float angleOffset) {
  if (angleOffset != 0.0) {
    Serial.printf("Applying offset: Moving J%d by %.2f degrees...\n", jointIndex + 1, angleOffset);

    long targetStep = (long)(angleOffset * STEPS_DEG[jointIndex]);
    stepper->moveTo(targetStep);

    while (stepper->distanceToGo() != 0) {
      stepper->run();
      yield();
    }
  }

  long homeSteps = (long)(DIR_SIGN[jointIndex] * HOME_ANGLE_DEG[jointIndex] * STEPS_DEG[jointIndex]);
  stepper->setCurrentPosition(homeSteps);
  uiTargetSteps[jointIndex] = homeSteps;
}

void executeHardwareHome() {
  currentState = HOMING;
  Serial.println("Starting Homing Sequence");

  // Notice we pull the offsets directly from the SAFE_HOME_ANGLES array now!
  Serial.println("Homing J1...");
  homeJoint(&stepperJ1, SW1_PIN, -1);
  moveOffsetAfterHoming(&stepperJ1, 0, SAFE_HOME_ANGLES[0]);

  Serial.println("Homing J2...");
  homeJoint(&stepperJ2, SW2_PIN, 1);
  moveOffsetAfterHoming(&stepperJ2, 1, SAFE_HOME_ANGLES[1]);

  Serial.println("Homing J3...");
  homeJoint(&stepperJ3, SW3_PIN, -1);
  moveOffsetAfterHoming(&stepperJ3, 2, SAFE_HOME_ANGLES[2]);

  Serial.println("Homing J4...");
  homeJoint(&stepperJ4, SW4_PIN, 1);
  moveOffsetAfterHoming(&stepperJ4, 3, SAFE_HOME_ANGLES[3]);

  Serial.println("Homing J5...");
  homeJoint(&stepperJ5, SW5_PIN, 1);
  moveOffsetAfterHoming(&stepperJ5, 4, SAFE_HOME_ANGLES[4]);

  Serial.println("Homing J6...");
  homeJoint(&stepperJ6, SW6_PIN, -1);
  moveOffsetAfterHoming(&stepperJ6, 5, SAFE_HOME_ANGLES[5]);

  Serial.println("Limit switches found, all Ready!");

  // Update the UI via WebSockets after Homing completes
  String syncData = "SYNC:" + String(gripOpenAngle) + ":" + String(gripCloseAngle) + ":" + String(gripperServo.read());
  for (int i = 0; i < 6; i++) {
    float deg = (float)uiTargetSteps[i] / STEPS_DEG[i] * DIR_SIGN[i]; // real angle
    syncData += ":" + String(deg, 2);
  }
  ws.textAll(syncData);

  currentState = IDLE;
}
