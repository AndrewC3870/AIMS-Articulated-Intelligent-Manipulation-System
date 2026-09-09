#pragma once

struct IKSolution {
  bool  ok;          // reachable AND within joint limits
  bool  reachable;   // target within geometric reach
  bool  inLimits;    // J1/J2/J3 within limits
  float j1, j2, j3;  // real joint angles (deg)
  float j5;          // real wrist-pitch angle (deg)
  float a2;          // forearm effective angle from vertical (rad), internal
  bool  j5Clamped;   // J5 had to be clamped to its limit
  const char* error; // reason when !ok
};

IKSolution solveIK(float x, float y, float z, bool elbowUp);

IKSolution solveIKTip(float x, float y, float z, bool elbowUp, bool toolDown);

void forwardKinematics(float j1, float j2, float j3, float j5, float &x, float &y, float &z);
