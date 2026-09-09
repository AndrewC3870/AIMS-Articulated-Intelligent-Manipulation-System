#include "Training.h"
#include <Arduino.h>
#include <Preferences.h>
#include "Config.h"
#include "Globals.h"

// ==============================================================================
//  LIVE WAYPOINT CAPTURE
// ==============================================================================
void recordWaypoint() {
  if (pathLength < MAX_WAYPOINTS) {
    for (int i = 0; i < 6; i++)
      path[pathLength].positions[i] = steppers[i]->targetPosition();

    path[pathLength].gripperAngle = gripperAngleCmd;
    pathLength++;
    ws.textAll("POINTS:" + String(pathLength));
    Serial.printf("Waypoint %d saved | Gripper %d\n", pathLength, path[pathLength-1].gripperAngle);
  } else {
    Serial.println("WARNING: waypoint limit reached (100).");
  }
}

void recordWaypointFromTargets() {
  if (pathLength < MAX_WAYPOINTS) {
    for (int i = 0; i < 6; i++) 
      path[pathLength].positions[i] = uiTargetSteps[i];

    path[pathLength].gripperAngle = gripperAngleCmd;
    pathLength++;
    ws.textAll("POINTS:" + String(pathLength));
    Serial.printf("IK waypoint %d saved\n", pathLength);
  } else {
    Serial.println("WARNING: waypoint limit reached (100).");
  }
}

static Preferences profPrefs;

static String kLen(int p) { return "len" + String(p); }
static String kWp (int p) { return "wp"  + String(p); }

void loadProfilesMeta() {
  profPrefs.begin("profiles", false);   // RW namespace in flash (NVS)
  for (int p = 0; p < NUM_PROFILES; p++) {
    int len = profPrefs.getInt(kLen(p).c_str(), 0);
    profileUsed[p] = (len > 0);
  }
  Serial.printf("loaded: %d %d %d %d\n", profileUsed[0], profileUsed[1], profileUsed[2], profileUsed[3]);
}

bool loadProfileToPath(int p) {
  if (p < 0 || p >= NUM_PROFILES) 
    return false;
  int len = profPrefs.getInt(kLen(p).c_str(), 0);

  if (len <= 0) { 
    pathLength = 0;
    playbackIndex = 0;
    return false;
  }
  if (len > MAX_WAYPOINTS) len = MAX_WAYPOINTS;
  profPrefs.getBytes(kWp(p).c_str(), path, len * sizeof(Waypoint));
  pathLength = len;
  playbackIndex = 0;
  Serial.printf("loaded slot %d (%d points)\n", p, len);
  return true;
}

void saveActiveProfile(int p) {
  if (p < 0 || p >= NUM_PROFILES) 
    return;
  profPrefs.putInt(kLen(p).c_str(), pathLength);
  profPrefs.putBytes(kWp(p).c_str(), path, pathLength * sizeof(Waypoint));
  profileUsed[p] = (pathLength > 0);
  Serial.printf("saved %d points to slot %d\n", pathLength, p);
}

void deleteProfile(int p) {
  if (p < 0 || p >= NUM_PROFILES)
    return;
  profPrefs.putInt(kLen(p).c_str(), 0);
  profPrefs.remove(kWp(p).c_str());
  profileUsed[p] = false;
  int back = profPrefs.getInt(kLen(p).c_str(), -1);
  Serial.printf("DELETED slot %d  (flash len now = %d -> %s)\n", p, back, back == 0 ? "OK, erased" : "WARNING: not persisted!");
}