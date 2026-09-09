#pragma once
#include <Arduino.h>

// Stepper pins
#define J1_STEP 4
#define J1_DIR  5
#define J2_STEP 6
#define J2_DIR  7
#define J3_STEP 15
#define J3_DIR  16
#define J4_STEP 17
#define J4_DIR  18
#define J5_STEP 8
#define J5_DIR  3
#define J6_STEP 46
#define J6_DIR  9

// Limit switches
#define SW6_PIN 1
#define SW3_PIN 2
#define SW5_PIN 42
#define SW4_PIN 41
#define SW2_PIN 40
#define SW1_PIN 39

// Gripper servo
#define GRIPPER_PIN 12

const float STEPS_DEG[] = {
  (3200.0 * 10.0) / 360.0,
  (3200.0 * 40.0) / 360.0,
  (3200.0 * 20.0) / 360.0,
  (3200.0 * 3.0)  / 360.0,
  (3200.0 * 20.0) / 360.0,
  (3200.0 * 4.0)  / 360.0
};

// Home offsets
const float SAFE_HOME_ANGLES[6] = {86.0, 0.0, 0.0, -146.0, -40.0, 155.0};


// Path memory
const int MAX_WAYPOINTS = 100;
const int SERVO_WAIT_TIME = 500;
const int NUM_PROFILES = 4;
const float DRAW_SPEED = 2000.0;
const int   DRAW_RAMP_SEGS = 8;
const float DRAW_MIN_FACTOR = 0.25;

// Wi-Fi AP
const char* const WIFI_SSID = "AIMS_ROBOT";
const char* const WIFI_PASSWORD = "password123";

// Kinematics
const float KIN_L1         = 199.0;
const float KIN_L2         = 250.4;
const float KIN_D_SHOULDER = 11.0;
const float KIN_L3         = 251.6;
const float KIN_D_ELBOW    = 41.0;
const float KIN_L4         = 251.0;
const float KIN_D_TIP      = 17.0;

//  SAG / DROOP COMPENSATION
const bool  COMP_ENABLE = true;
const float COMP_Z_SLOPE  = 0.23f;
const float COMP_Z_R0 = 324.0f;
const float COMP_R_OFFSET = 13.0f;
const float COMP_R_SLOPE = 0.0f;
const float COMP_R_R0 = 300.0f;
const float COMP_MAX_MM = 35.0f;


const float DIR_SIGN[6]       = {  1.0, -1.0,  -1.0,    1.0,   1.0,    1.0 };
const float HOME_ANGLE_DEG[6] = {  0.0,-53.0, 102.7,    0.0,  76.0,    0.0 };


const float TRAVEL_MIN_DEG[6] = { -90.0,   0.0, -81.0, -140.0, -110.0, -180.0 };
const float TRAVEL_MAX_DEG[6] = {  90.0, 100.0,   0.0,  180.0,   40.0,  150.0 };

inline float jointMin(int i) { return HOME_ANGLE_DEG[i] + TRAVEL_MIN_DEG[i]; }
inline float jointMax(int i) { return HOME_ANGLE_DEG[i] + TRAVEL_MAX_DEG[i]; }


//  SHARED TYPES
enum RobotState { IDLE, HOMING, TRAINING, PLAYING, PAUSED };

struct Waypoint {
  long positions[6];
  int  gripperAngle;
};