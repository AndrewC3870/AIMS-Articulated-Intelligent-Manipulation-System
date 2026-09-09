#pragma once
#include <Arduino.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <ESP32Servo.h>
#include <ESPAsyncWebServer.h>
#include "Config.h"

// Steppers
extern AccelStepper stepperJ1;
extern AccelStepper stepperJ2;
extern AccelStepper stepperJ3;
extern AccelStepper stepperJ4;
extern AccelStepper stepperJ5;
extern AccelStepper stepperJ6;
extern AccelStepper* steppers[6];
extern MultiStepper multiDraw;

// Gripper
extern Servo gripperServo;
extern int gripOpenAngle;
extern int gripCloseAngle;

// State machine
extern volatile RobotState currentState;

// Path memory
extern Waypoint path[MAX_WAYPOINTS];
extern int pathLength;
extern int playbackIndex;

// Training profiles
extern volatile bool playOnce;
extern bool recording;
extern int  gripperAngleCmd;
extern int  activeProfile;
extern bool profileUsed[NUM_PROFILES];

// flags
extern volatile bool flagTriggerHome;
extern volatile bool flagTriggerReset;
extern volatile bool flagTriggerPlay;
extern volatile bool flagTriggerPause;
extern volatile bool flagTriggerStopTrain;
extern volatile bool flagTriggerEstop;
extern volatile bool flagTriggerDefault;

// Slider targets set by the Wi-Fi task
extern volatile long uiTargetSteps[6];
extern volatile bool flagUpdateSteppers[6];

// Networking
extern AsyncWebServer server;
extern AsyncWebSocket ws;