#include "WebInterface.h"
#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "Config.h"
#include "Globals.h"
#include "WebPage.h"
#include "Training.h"
#include "Kinematics.h"

// GRIPPER CALIBRATION
static Preferences gripPrefs;

void loadCalibration() {
  gripPrefs.begin("gripper", false);
  gripOpenAngle  = gripPrefs.getInt("open", gripOpenAngle);
  gripCloseAngle = gripPrefs.getInt("close", gripCloseAngle);
  Serial.printf("Gripper cal: open=%d close=%d\n", gripOpenAngle, gripCloseAngle);
  loadProfilesMeta();
}

static void saveCalibration() {
  gripPrefs.putInt("open", gripOpenAngle);
  gripPrefs.putInt("close", gripCloseAngle);
}

// PACKETS
static String syncString() {
  String s = "SYNC:" + String(gripOpenAngle) + ":" + String(gripCloseAngle) + ":" + String(gripperAngleCmd);
  for (int i = 0; i < 6; i++) {
    float deg = (float)uiTargetSteps[i] / STEPS_DEG[i] * DIR_SIGN[i];
    s += ":" + String(deg, 2);
  }
  return s;
}
static String limitsString() {
  String s = "LIMITS";
  for (int i = 0; i < 6; i++) s += ":" + String(jointMin(i), 1) + ":" + String(jointMax(i), 1);
  return s;
}
static String profilesString() {
  String s = "PROFILES";
  for (int i = 0; i < NUM_PROFILES; i++) s += ":" + String(profileUsed[i] ? 1 : 0);
  s += ":" + String(activeProfile);
  return s;
}
void broadcastSync() { 
  ws.textAll(syncString());
}

static void broadcastProfiles(){ 
  ws.textAll(profilesString());
}

//  WEBSOCKET PARSING
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (!(info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)) 
    return;

  String msg = "";
  for (size_t i = 0; i < len; i++) msg += (char)data[i];
  Serial.println("[WS RAW] " + msg);

  // CMD:
  if (msg.startsWith("CMD:")) {
    String cmd = msg.substring(4);

    if (cmd == "HOME")
      flagTriggerHome = true;
    else if (cmd == "ESTOP")
      flagTriggerEstop = true;
    else if (cmd == "DEFAULT")
      flagTriggerDefault = true;
    else if (cmd == "PAUSE")
      flagTriggerPause = true;

    else if (cmd.startsWith("SET_OPEN:")) {
      gripOpenAngle  = cmd.substring(9).toInt();
      saveCalibration();
    }
    else if (cmd.startsWith("SET_CLOSE:")) {
      gripCloseAngle = cmd.substring(10).toInt();
      saveCalibration();
    }

    // Training profiles
    else if (cmd.startsWith("PROF:")) {
      String rest = cmd.substring(5);
      int c = rest.indexOf(':');
      String action = (c < 0) ? rest : rest.substring(0, c);
      int p = (c < 0) ? -1 : rest.substring(c + 1).toInt();

      if (action == "sel" && currentState == IDLE && !recording) {
        activeProfile = p;

        if (profileUsed[p]) 
          loadProfileToPath(p); 
        else { pathLength = 0; 
          playbackIndex = 0;
        }
        ws.textAll("POINTS:" + String(pathLength));
        broadcastProfiles();
      }
      else if (action == "rec" && currentState == IDLE && !recording) {
        recording = true;
        activeProfile = p;
        pathLength = 0; playbackIndex = 0;
        ws.textAll("POINTS:0");
        broadcastProfiles();
        Serial.printf("recording into slot %d\n", p);
      }
      else if (action == "wp" && recording) {
        recordWaypoint();
      }
      else if (action == "save" && recording) {
        recording = false;
        saveActiveProfile(activeProfile);
        broadcastProfiles();
        Serial.printf("STOP&SAVE");
      }
      else if (action == "play" && !recording && (currentState == IDLE || currentState == PAUSED)) {
        activeProfile = p;
        bool ready = true;
        if (currentState == IDLE)
          ready = loadProfileToPath(p);
        if (ready && pathLength > 0) {
          ws.textAll("POINTS:" + String(pathLength));
          flagTriggerPlay = true;
        }
        broadcastProfiles();
      }
      else if (action == "pause" && currentState == PLAYING) {
        flagTriggerPause = true;
      }
      else if (action == "del" && !recording) {
        currentState = IDLE;
        deleteProfile(p);
        if (activeProfile == p) {
          pathLength = 0; playbackIndex = 0;
          ws.textAll("POINTS:0");
        }
        broadcastProfiles();
      }
    }
  }
  // GRIP:
  else if (msg.startsWith("GRIP:") && currentState == IDLE) {
    int a = msg.substring(5).toInt();
    if (a < 0)
      a = 0;
    if (a > 180)
      a = 180;
    gripperAngleCmd = a;
    gripperServo.write(a);
  }
  // J:joint:angle
  else if (msg.startsWith("J:") && currentState == IDLE) {
    int c1 = msg.indexOf(':');
    int c2 = msg.indexOf(':', c1 + 1);
    int joint = msg.substring(c1 + 1, c2).toInt();
    float angle = msg.substring(c2 + 1).toFloat();
    long step = (long)(DIR_SIGN[joint - 1] * angle * STEPS_DEG[joint - 1]);
    uiTargetSteps[joint - 1] = step;
    flagUpdateSteppers[joint - 1] = true;
  }
  // IK:x:y:z:elbow:tool
  else if (msg.startsWith("IK:") && currentState == IDLE) {
    int c1 = msg.indexOf(':');
    int c2 = msg.indexOf(':', c1 + 1);
    int c3 = msg.indexOf(':', c2 + 1);
    int c4 = msg.indexOf(':', c3 + 1);
    int c5 = msg.indexOf(':', c4 + 1);
    float x = msg.substring(c1 + 1, c2).toFloat();
    float y = msg.substring(c2 + 1, c3).toFloat();
    float z = msg.substring(c3 + 1, c4).toFloat();
    String elbowStr, toolStr;
    if (c5 < 0) {
      elbowStr = msg.substring(c4 + 1);
      toolStr = "auto";
    }
    else {
      elbowStr = msg.substring(c4 + 1, c5);
      toolStr = msg.substring(c5 + 1);
    }
    bool elbowUp = (elbowStr != "down");
    bool toolDown = (toolStr == "down");

    IKSolution sol = solveIKTip(x, y, z, elbowUp, toolDown);
    if (sol.ok) {

      float jdeg[3] = {sol.j1, sol.j2, sol.j3};
      for (int i = 0; i < 3; i++) {
        uiTargetSteps[i] = (long)(DIR_SIGN[i] * jdeg[i] * STEPS_DEG[i]);
        flagUpdateSteppers[i] = true;
      }
      uiTargetSteps[4] = (long)(DIR_SIGN[4] * sol.j5 * STEPS_DEG[4]);
      flagUpdateSteppers[4] = true;

      char buf[110];
      snprintf(buf, sizeof(buf), "IKRES:OK:%.1f:%.1f:%.1f:%.1f:%d", sol.j1, sol.j2, sol.j3, sol.j5, sol.j5Clamped ? 1 : 0);
      ws.textAll(buf);
      Serial.printf("[IK] tip(%.0f,%.0f,%.0f) -> J1=%.1f J2=%.1f J3=%.1f J5=%.1f%s\n", x, y, z, sol.j1, sol.j2, sol.j3, sol.j5, sol.j5Clamped ? " (J5 clamped)" : "");
      broadcastSync();
    } else {
      ws.textAll(String("IKRES:ERR:") + (sol.error ? sol.error : "error"));
    }
  }
  // LINE:x:y:z  (draw straight line from current tip)
  else if (msg.startsWith("LINE:") && currentState == IDLE) {
    int c1 = msg.indexOf(':');
    int c2 = msg.indexOf(':', c1 + 1);
    int c3 = msg.indexOf(':', c2 + 1);
    int c4 = msg.indexOf(':', c3 + 1);
    int c5 = msg.indexOf(':', c4 + 1);
    float tx = msg.substring(c1 + 1, c2).toFloat();
    float ty = msg.substring(c2 + 1, c3).toFloat();
    float tz;
    bool elbowUp = true, toolDown = true;
    if (c4 < 0) {
      tz = msg.substring(c3 + 1).toFloat();
    } else {
      tz = msg.substring(c3 + 1, c4).toFloat();
      String elbowStr, toolStr;
      if (c5 < 0) { elbowStr = msg.substring(c4 + 1); toolStr = "down"; }
      else        { elbowStr = msg.substring(c4 + 1, c5); toolStr = msg.substring(c5 + 1); }
      elbowUp  = (elbowStr != "down");
      toolDown = (toolStr == "down");
    }

    // current tip from current real joint angles
    float cj1 = (float)steppers[0]->currentPosition() / STEPS_DEG[0] * DIR_SIGN[0];
    float cj2 = (float)steppers[1]->currentPosition() / STEPS_DEG[1] * DIR_SIGN[1];
    float cj3 = (float)steppers[2]->currentPosition() / STEPS_DEG[2] * DIR_SIGN[2];
    float cj5 = (float)steppers[4]->currentPosition() / STEPS_DEG[4] * DIR_SIGN[4];
    float cx, cy, cz;
    forwardKinematics(cj1, cj2, cj3, cj5, cx, cy, cz);

    float dist = sqrtf((tx-cx)*(tx-cx) + (ty-cy)*(ty-cy) + (tz-cz)*(tz-cz));
    int N = (int)(dist / 4.0f); // 4 mm spacing
    if (N < 2) N = 2;
    if (N > MAX_WAYPOINTS) N = MAX_WAYPOINTS;

    int failAt = -1; const char* failReason = "";
    long J4hold = steppers[3]->currentPosition();
    long J6hold = steppers[5]->currentPosition();
    int  grip   = gripperAngleCmd;

    
    for (int k = 1; k <= N; k++) {
      float t = (float)k / N;
      float px = cx + (tx-cx)*t, py = cy + (ty-cy)*t, pz = cz + (tz-cz)*t;
      IKSolution s = solveIKTip(px, py, pz, elbowUp, toolDown);
      if (!s.ok) {
        Serial.printf("  pt %2d/%d (%.0f,%.0f,%.0f)  UNREACHABLE: %s\n", k, N, px,py,pz, s.error ? s.error : "?");
        failAt = k; failReason = s.error ? s.error : "?";
        break;
      }
      Serial.printf("  pt %2d/%d (%.0f,%.0f,%.0f)  OK  J1=%.1f J2=%.1f J3=%.1f J5=%.1f%s\n", k, N, px,py,pz, s.j1, s.j2, s.j3, s.j5, s.j5Clamped ? "  [J5 clamped]" : "");
      int idx = k - 1;
      path[idx].positions[0] = (long)(DIR_SIGN[0] * s.j1 * STEPS_DEG[0]);
      path[idx].positions[1] = (long)(DIR_SIGN[1] * s.j2 * STEPS_DEG[1]);
      path[idx].positions[2] = (long)(DIR_SIGN[2] * s.j3 * STEPS_DEG[2]);
      path[idx].positions[3] = J4hold;
      path[idx].positions[4] = (long)(DIR_SIGN[4] * s.j5 * STEPS_DEG[4]);
      path[idx].positions[5] = J6hold;
      path[idx].gripperAngle = grip;
    }

    if (failAt > 0) {
      ws.textAll(String("ERR:point ") + failAt + " of " + N + " (" + failReason + ")");
      Serial.printf("ABORTED: %d of %d points reachable; stopped at point %d (%s)\n", failAt - 1, N, failAt, failReason);
    } else {
      pathLength = N;
      playbackIndex = 0;
      playOnce = true;
      activeProfile = -1;
      flagTriggerPlay = true;
      ws.textAll(String("OK:") + N);
      Serial.printf("ALL %d points reachable -> drawing.\n", N);
    }
  }
}

//  CONNECTION HANDLER
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_DATA) {
    handleWebSocketMessage(arg, data, len);
  } else if (type == WS_EVT_CONNECT) {
    Serial.println("Client connected. Syncing UI...");
    client->text(limitsString());
    client->text(syncString());
    client->text(profilesString());
    client->text("POINTS:" + String(pathLength));
  }
}

//  SERVER BRING-UP
void setupWebServer() {
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html);
    response->addHeader("Cache-Control", "no-store");
    request->send(response);
  });
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  server.begin();
}