#include "Globals.h"

AccelStepper stepperJ1(AccelStepper::DRIVER, J1_STEP, J1_DIR);
AccelStepper stepperJ2(AccelStepper::DRIVER, J2_STEP, J2_DIR);
AccelStepper stepperJ3(AccelStepper::DRIVER, J3_STEP, J3_DIR);
AccelStepper stepperJ4(AccelStepper::DRIVER, J4_STEP, J4_DIR);
AccelStepper stepperJ5(AccelStepper::DRIVER, J5_STEP, J5_DIR);
AccelStepper stepperJ6(AccelStepper::DRIVER, J6_STEP, J6_DIR);

AccelStepper* steppers[6] = {
  &stepperJ1, &stepperJ2, &stepperJ3, &stepperJ4, &stepperJ5, &stepperJ6
};

MultiStepper multiDraw;

// Gripper
Servo gripperServo;
int gripOpenAngle  = 100;
int gripCloseAngle = 30;

// Status
volatile RobotState currentState = IDLE;

// Path memory
Waypoint path[MAX_WAYPOINTS];
int pathLength = 0;
int playbackIndex = 0;

// Training profiles
volatile bool playOnce = false;
bool recording = false;
int gripperAngleCmd = 0;
int activeProfile = -1;
bool profileUsed[NUM_PROFILES] = { false, false, false, false };

// flags
volatile bool flagTriggerHome  = false;
volatile bool flagTriggerReset = false;
volatile bool flagTriggerPlay = false;
volatile bool flagTriggerPause = false;
volatile bool flagTriggerStopTrain = false;
volatile bool flagTriggerEstop = false;
volatile bool flagTriggerDefault = false;

volatile long uiTargetSteps[6] = {0, 0, 0, 0, 0, 0};
volatile bool flagUpdateSteppers[6] = {false, false, false, false, false, false};

// Networking
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");