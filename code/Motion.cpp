#include "Motion.h"
#include <Arduino.h>
#include "Config.h"
#include "Globals.h"
#include "Homing.h"
#include "WebInterface.h" 

// Playback Timers (local to the playback state machine)
static unsigned long waypointPauseTime = 0;
static bool isPausing = false;
static void restoreNormalSpeed();
static void resyncSteppers();
static void setDrawRampSpeed(int segIdx);
static void writeGripper(int angle);

//  COMMAND FLAGS 
void processCommandFlags() {
  // EMERGENCY STOP: handled first, halts everything immediately
  if (flagTriggerEstop) {
    flagTriggerEstop = false;
    currentState = IDLE;
    playOnce = false;
    recording = false;
    resyncSteppers();
    restoreNormalSpeed();
    for (int i = 0; i < 6; i++) 
      flagUpdateSteppers[i] = false;

    Serial.println("EMERGENCY STOP");
    ws.textAll("ESTOPPED");
    broadcastSync();
    return;
  }

  // drive every joint back to the home pose
  if (flagTriggerDefault) {
    flagTriggerDefault = false;
    currentState = IDLE;
    playOnce = false;
    resyncSteppers();
    restoreNormalSpeed();
    Serial.println("Returning to home pose.");
    for (int i = 0; i < 6; i++) {
      long hs = (long)(DIR_SIGN[i] * HOME_ANGLE_DEG[i] * STEPS_DEG[i]);
      uiTargetSteps[i] = hs;
      steppers[i]->moveTo(hs);
    }
    writeGripper(gripOpenAngle);
    broadcastSync();
  }

  if (flagTriggerHome) {
    flagTriggerHome = false;
    executeHardwareHome();
  }

  // Reset logic
  if (flagTriggerReset) {
    flagTriggerReset = false;
    pathLength = 0;
    playbackIndex = 0;
    ws.textAll("POINTS:0");

    Serial.println("Reset and drive home");
    currentState = IDLE;

    for (int i = 0; i < 6; i++) {
      long hs = (long)(DIR_SIGN[i] * HOME_ANGLE_DEG[i] * STEPS_DEG[i]);
      uiTargetSteps[i] = hs;
      steppers[i]->moveTo(hs);
    }
    writeGripper(gripOpenAngle);
  }

  if (flagTriggerStopTrain) {
    flagTriggerStopTrain = false;
    Serial.println("Training stopped, returning to Start Point.");
    currentState = IDLE;
    for (int i = 0; i < 6; i++) {
      uiTargetSteps[i] = path[0].positions[i];
      steppers[i]->moveTo(path[0].positions[i]);
    }
    writeGripper(path[0].gripperAngle);
  }

  if (flagTriggerPlay) {
    flagTriggerPlay = false;
    if (pathLength > 0) {
      currentState = PLAYING;
      if (playOnce) {
        Serial.println("DRAW");
        setDrawRampSpeed(playbackIndex);
        long pos[6];

        for (int i = 0; i < 6; i++) 
          pos[i] = path[playbackIndex].positions[i];

        multiDraw.moveTo(pos);
        writeGripper(path[playbackIndex].gripperAngle);
      } else {
        Serial.println("PLAYING");

        for (int i = 0; i < 6; i++) 
          steppers[i]->moveTo(path[playbackIndex].positions[i]);
        writeGripper(path[playbackIndex].gripperAngle);
      }
    }
  }

  if (flagTriggerPause) {
    flagTriggerPause = false;
    Serial.println("PAUSED");
    currentState = PAUSED;
    playOnce = false;
    resyncSteppers();
    restoreNormalSpeed();
  }

  // Slider Movements
  for (int i = 0; i < 6; i++) {
    if (flagUpdateSteppers[i]) {
      flagUpdateSteppers[i] = false;
      steppers[i]->moveTo(uiTargetSteps[i]);
    }
  }
}


static void restoreNormalSpeed() {
  for (int i = 0; i < 6; i++) {
    steppers[i]->setMaxSpeed(6000);
    steppers[i]->setAcceleration(1200);
  }
}

static void resyncSteppers() {
  for (int i = 0; i < 6; i++) {
    long p = steppers[i]->currentPosition();
    steppers[i]->setCurrentPosition(p); // resets internal speed state, target = p
    steppers[i]->moveTo(p);
    uiTargetSteps[i] = p;
  }
}


static void setDrawRampSpeed(int segIdx) {
  float up = (float)(segIdx + 1) / DRAW_RAMP_SEGS;
  float down = (float)(pathLength - segIdx) / DRAW_RAMP_SEGS;
  float f = up;
  if (down < f)
    f = down;
  if (f > 1.0f)
    f = 1.0f;
  if (f < DRAW_MIN_FACTOR)
    f = DRAW_MIN_FACTOR;
  for (int i = 0; i < 6; i++)
    steppers[i]->setMaxSpeed(DRAW_SPEED * f);
}


static void writeGripper(int angle) {
  if (angle < 0)
    angle = 0;
  if (angle > 180)
    angle = 180;
  gripperAngleCmd = angle;
  gripperServo.write(angle);
}

static void updateDraw() {
  if (!multiDraw.run()) {
    playbackIndex++;
    if (playbackIndex >= pathLength) {
      playOnce = false;
      currentState = IDLE;
      resyncSteppers();
      restoreNormalSpeed();
      Serial.println("Draw complete.");
      broadcastSync();
      return;
    }
    setDrawRampSpeed(playbackIndex);
    long pos[6];
    for (int i = 0; i < 6; i++)
      pos[i] = path[playbackIndex].positions[i];
    multiDraw.moveTo(pos);
    writeGripper(path[playbackIndex].gripperAngle);
  }
}

void updatePlayback() {
  if (currentState != PLAYING) return;

  if (playOnce) { 
    updateDraw();
    return;
  }

  bool allReached = true;
  for (int i = 0; i < 6; i++) {
    if (steppers[i]->distanceToGo() != 0) { allReached = false; break; }
  }
  if (allReached) {
    if (!isPausing) { 
     isPausing = true; 
      waypointPauseTime = millis();
    }

    if (millis() - waypointPauseTime >= (unsigned long)SERVO_WAIT_TIME) {
      playbackIndex++;
      if (playbackIndex >= pathLength) {
        playbackIndex = 0;
        Serial.println(" Loop complete. Restarting from Waypoint 0.");
      }
      for (int i = 0; i < 6; i++) 
        steppers[i]->moveTo(path[playbackIndex].positions[i]);

      writeGripper(path[playbackIndex].gripperAngle);
      isPausing = false;
    }
  } else {
    isPausing = false;
  }
}

void runAllSteppers() {
  if (currentState == PLAYING && playOnce)
    return;
  for (int i = 0; i < 6; i++) {
    steppers[i]->run();
  }
}