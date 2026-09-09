// ==============================================================================
//  File map:
//    Config.h         pins, gear ratios, offsets, limits, Wi-Fi, shared types
//    Globals.h/.cpp   all shared mutable state (steppers, servo, path, flags...)
//    WebPage.h        the dashboard HTML
//    WebInterface     Wi-Fi AP, HTTP route, WebSocket parsing
//    Homing           limit-switch homing + post-home offsets
//    Training         waypoint recording
//    Motion           main-loop helpers (flags, playback, stepper run)
//    AIMS_Robot.cpp   this file - setup() and loop() only
// ==============================================================================

#include <Arduino.h>

#include "Config.h"
#include "Globals.h"
#include "Homing.h"
#include "WebInterface.h"
#include "Motion.h"

void setup() {
  Serial.begin(115200);
  Serial.println("\n[System] Booting up AIMS Robot Controller...");

  pinMode(SW1_PIN, INPUT_PULLUP);
  pinMode(SW2_PIN, INPUT_PULLUP);
  pinMode(SW3_PIN, INPUT_PULLUP);
  pinMode(SW4_PIN, INPUT_PULLUP);
  pinMode(SW5_PIN, INPUT_PULLUP);
  pinMode(SW6_PIN, INPUT_PULLUP);

  gripperServo.setPeriodHertz(50);
  gripperServo.attach(GRIPPER_PIN, 500, 2500);

  for (int i = 0; i < 6; i++) {
    steppers[i]->setMaxSpeed(6000);
    steppers[i]->setAcceleration(1200);
    steppers[i]->setMinPulseWidth(20);
  }


  stepperJ1.setPinsInverted(true, false, false);
  stepperJ3.setPinsInverted(true, false, false);
  stepperJ4.setPinsInverted(true, false, false);

  // Register all axes with the coordinated mover used for smooth line/draw motion.
  for (int i = 0; i < 6; i++) {
    multiDraw.addStepper(*steppers[i]);
  }

  loadCalibration();   // restore gripper angles + training profiles from flash

  Serial.println("Starting Wi-Fi Access Point...");
  setupWebServer();   // WiFi.softAP + routes + websocket + server.begin()

  Serial.println("Web Server Started Successfully!");

  delay(1000);
  executeHardwareHome();
}


void loop() {
  ws.cleanupClients();

  processCommandFlags();   // HOME / RESET / STOP_TRAIN / PLAY / PAUSE + sliders
  updatePlayback();        // PLAYING state machine
  runAllSteppers();        // step every motor once
}
