#include "Kinematics.h"
#include <Arduino.h>
#include <math.h>
#include "Config.h"

// Derived once from the link geometry.
static const float L2EFF = sqrtf(KIN_L2*KIN_L2 + KIN_D_SHOULDER*KIN_D_SHOULDER);
static const float PHI2 = atan2f(KIN_D_SHOULDER, KIN_L2);
static const float LF = sqrtf(KIN_L3*KIN_L3 + KIN_D_ELBOW*KIN_D_ELBOW);
static const float PHI = atan2f(KIN_D_ELBOW, KIN_L3);
static const float R2D = 57.29577951f;
static const float D2R = 0.01745329252f;

IKSolution solveIK(float x, float y, float z, bool elbowUp) {
  IKSolution s = {false, false, false, 0, 0, 0, 0, 0, false, nullptr};

  float th1 = atan2f(y, x);
  float dr  = sqrtf(x*x + y*y);
  float dh  = z - KIN_L1;
  float D   = sqrtf(dr*dr + dh*dh);

  if (D > L2EFF + LF || D < fabsf(L2EFF - LF)) { s.error = "unreachable"; return s; }
  s.reachable = true;

  float gamma = atan2f(dr, dh);
  float cb = constrain((D*D + L2EFF*L2EFF - LF*LF) / (2.0f*D*L2EFF), -1.0f, 1.0f);
  float cp = constrain((L2EFF*L2EFF + LF*LF - D*D) / (2.0f*L2EFF*LF), -1.0f, 1.0f);
  float beta = acosf(cb);
  float psi  = acosf(cp);

  float uaEff, a2eff;
  if (elbowUp) { uaEff = gamma - beta; a2eff = uaEff + ((float)PI - psi); }
  else         { uaEff = gamma + beta; a2eff = uaEff - ((float)PI - psi); }

  float a1 = uaEff - PHI2;
  float a2 = a2eff;
  s.a2 = a2;

  s.j1 = th1 * R2D;
  s.j2 = a1  * R2D;
  s.j3 = (a2 - a1) * R2D - PHI * R2D;

  float jj[3] = {s.j1, s.j2, s.j3};
  s.inLimits = true;
  for (int i = 0; i < 3; i++) if (jj[i] < jointMin(i) || jj[i] > jointMax(i)) s.inLimits = false;
  if (!s.inLimits) { s.error = "joint limit"; return s; }

  s.ok = true;
  return s;
}


IKSolution solveIKTip(float x, float y, float z, bool elbowUp, bool toolDown) {
  if (COMP_ENABLE) {
    float rr = sqrtf(x*x + y*y);
    if (rr > 50.0f) {
      float dz = COMP_Z_SLOPE * (rr - COMP_Z_R0);
      float dr = COMP_R_OFFSET + COMP_R_SLOPE * (rr - COMP_R_R0);
      dz = constrain(dz, -COMP_MAX_MM, COMP_MAX_MM);
      dr = constrain(dr, -COMP_MAX_MM, COMP_MAX_MM);
      float sc = (rr + dr) / rr;
      x *= sc; y *= sc; z += dz;
    }
  }

  float r  = sqrtf(x*x + y*y);
  float th = atan2f(y, x);

  if (toolDown) {
    float ta = 180.0f * D2R;
    float wr = r - KIN_L4 * sinf(ta);
    IKSolution s = solveIK(wr*cosf(th), wr*sinf(th), z - KIN_L4*cosf(ta), elbowUp);
    if (!s.ok) return s;
    float j5 = 180.0f - s.a2 * R2D;
    s.j5Clamped = false;
    if (j5 < jointMin(4)) {
      j5 = jointMin(4);
      s.j5Clamped = true;
    }
    if (j5 > jointMax(4)) {
      j5 = jointMax(4);
      s.j5Clamped = true;
    }
    s.j5 = j5;
    return s;
  }

  IKSolution best = {false, false, false, 0, 0, 0, 0, 0, false, "unreachable"};
  for (float toolAng = 180.0f; toolAng >= 0.0f; toolAng -= 2.0f) {
    float ta = toolAng * D2R;
    float wr = r - KIN_L4 * sinf(ta);
    IKSolution s = solveIK(wr*cosf(th), wr*sinf(th), z - KIN_L4*cosf(ta), elbowUp);

    if (!s.ok) {
      if (s.reachable) best.error = "joint limit";
      continue;
    }

    float j5 = toolAng - s.a2 * R2D;
    if (j5 < jointMin(4) || j5 > jointMax(4)) { best.error = "joint limit"; continue; }

    s.j5 = j5;
    s.j5Clamped = false;
    return s;
  }
  return best;
}

void forwardKinematics(float j1, float j2, float j3, float j5,
                       float &x, float &y, float &z) {
  float th1 = j1 * D2R, a1 = j2 * D2R;
  float a2  = a1 + j3 * D2R + PHI;       // forearm effective from vertical
  float Er = L2EFF*sinf(a1 + PHI2),  Ez = KIN_L1 + L2EFF*cosf(a1 + PHI2);
  float Wr = Er + LF*sinf(a2),       Wz = Ez + LF*cosf(a2);
  float tool = a2 + j5 * D2R;
  float tr = Wr + KIN_L4*sinf(tool), tz = Wz + KIN_L4*cosf(tool);
  x = tr*cosf(th1); y = tr*sinf(th1); z = tz;
}