#pragma once
#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>AIMS Robot Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family:'Segoe UI',Tahoma,sans-serif; background:#121212; color:#fff; text-align:center; padding:20px; }
    h2 { color:#00ff88; }
    .card { background:#1e1e1e; padding:20px; margin:15px auto; width:90%; max-width:600px; border-radius:10px; box-shadow:0 4px 8px rgba(0,0,0,.5); }
    .slider { -webkit-appearance:none; width:100%; height:10px; border-radius:5px; background:#333; outline:none; margin-top:15px; margin-bottom:5px; }
    .slider::-webkit-slider-thumb { -webkit-appearance:none; width:25px; height:25px; border-radius:50%; background:#00ff88; cursor:pointer; }
    .val { font-weight:bold; color:#00ff88; }
    button { padding:12px 18px; margin:5px; border:none; border-radius:5px; font-weight:bold; cursor:pointer; background:#00ff88; color:#121212; transition:.15s; }
    button:disabled { background:#555; color:#888; cursor:not-allowed; }
    .panel { display:flex; flex-wrap:wrap; justify-content:center; gap:10px; margin-bottom:15px; }
    .labels { display:flex; justify-content:space-between; font-size:.9em; font-weight:bold; color:#aaa; padding:0 5px; }
    .prof { min-width:96px; }
  </style>
</head>
<body>
  <h2>AIMS</h2>

  <div class="card">
    <div class="panel">
      <button onclick="sendCmd('ESTOP')" id="btnEstop" style="background:#ff0033;color:#fff;font-size:1.1em;padding:14px 26px;">&#9632; STOP</button>
      <button onclick="sendCmd('DEFAULT')" id="btnDefault" style="background:#2196F3;color:#fff;">Return to Home Pose</button>
      <button onclick="sendCmd('HOME')" id="btnHome">Home All</button>
    </div>
    <div id="status" style="color:#00ff88;font-size:.9em;margin-top:10px;">Status: READY</div>
  </div>

  <!-- ============================ TRAINING PROFILES ============================ -->
  <div class="card" style="border:1px solid #9c27b0;">
    <h3 style="color:#ce93d8;margin-top:0;">Training Profiles</h3>
    <div class="panel" id="profButtons">
      <button class="prof" id="prof0" onclick="selectProfile(0)">Profile 1</button>
      <button class="prof" id="prof1" onclick="selectProfile(1)">Profile 2</button>
      <button class="prof" id="prof2" onclick="selectProfile(2)">Profile 3</button>
      <button class="prof" id="prof3" onclick="selectProfile(3)">Profile 4</button>
    </div>
    <div class="panel" id="profControls"></div>
    <div id="profStatus" style="color:#aaa;font-size:.9em;height:16px;">Select a profile.</div>
  </div>

  <!-- ============================ GRIPPER ============================ -->
  <div class="card" style="border:1px solid #ffcc00;">
    <h3 style="color:#ffcc00;margin-top:0;">Gripper Control</h3>
    <div class="panel"><button id="btnStartCalib" onclick="startCalibration()" style="background:#2196F3;color:#fff;">Start Calibration</button></div>
    <div id="calibPanel" class="panel" style="display:none;background:#333;padding:10px;border-radius:8px;">
      <button onclick="setOpenPos()" style="background:#4CAF50;color:#fff;">Set Open</button>
      <button onclick="setClosePos()" style="background:#f44336;color:#fff;">Set Closed</button>
      <button onclick="finishCalibration()" style="background:#ffcc00;color:#121212;width:100%;">Finish Calibration</button>
    </div>
    <div class="labels"><span>Open</span><span>Closed</span></div>
    <input type="range" class="slider" id="grip" min="0" max="100" value="0" oninput="streamGripper(this.value)" onchange="sendFinalGripper(this.value)">
    <div id="calibStatus" style="color:#00ff88;font-size:.85em;margin-top:10px;height:15px;"></div>
  </div>

  <!-- ============================ INVERSE KINEMATICS ============================ -->
  <div class="card" style="border:1px solid #2196F3;">
    <h3 style="color:#2196F3;margin-top:0;">Inverse Kinematics</h3>
      <div class="panel" style="gap:18px;align-items:center;">
      <label>X <input type="number" id="ikx" value="300" step="5" style="width:75px;"></label>
      <label>Y <input type="number" id="iky" value="0" step="5" style="width:75px;"></label>
      <label>Z <input type="number" id="ikz" value="500" step="5" style="width:75px;"></label>
    </div>
    <div class="panel">
      <button id="btnElbow" onclick="toggleElbow()" style="background:#555;color:#fff;">Elbow: Up</button>
      <button id="btnTool" onclick="toggleTool()" style="background:#555;color:#fff;">Tool: Auto</button>
      <button id="btnIK" onclick="sendIK()" style="background:#2196F3;color:#fff;">Move to Target</button>
      <button id="btnLine" onclick="sendLine()" style="background:#9c27b0;color:#fff;">Draw line here</button>
    </div>
    <div id="ikStatus" style="color:#aaa;font-size:.85em;height:15px;"></div>
  </div>

  <!-- ============================ JOINT SLIDERS (REAL ANGLES) ============================ -->
  <div class="card" id="sliders">
    <label>J1 (Base): <span class="val" id="val1">0</span>&deg;</label>
    <input type="range" class="slider" id="j1" min="-90" max="90" value="0" step="0.1" oninput="streamJoint(1,this.value)" onchange="sendFinalJoint(1,this.value)">
    <label>J2 (Shoulder): <span class="val" id="val2">0</span>&deg;</label>
    <input type="range" class="slider" id="j2" min="-90" max="90" value="0" step="0.1" oninput="streamJoint(2,this.value)" onchange="sendFinalJoint(2,this.value)">
    <label>J3 (Elbow): <span class="val" id="val3">0</span>&deg;</label>
    <input type="range" class="slider" id="j3" min="-90" max="90" value="0" step="0.1" oninput="streamJoint(3,this.value)" onchange="sendFinalJoint(3,this.value)">
    <label>J4 (Forearm Roll): <span class="val" id="val4">0</span>&deg;</label>
    <input type="range" class="slider" id="j4" min="-180" max="180" value="0" step="0.1" oninput="streamJoint(4,this.value)" onchange="sendFinalJoint(4,this.value)">
    <label>J5 (Wrist Pitch): <span class="val" id="val5">0</span>&deg;</label>
    <input type="range" class="slider" id="j5" min="-120" max="160" value="0" step="0.1" oninput="streamJoint(5,this.value)" onchange="sendFinalJoint(5,this.value)">
    <label>J6 (Gripper Roll): <span class="val" id="val6">0</span>&deg;</label>
    <input type="range" class="slider" id="j6" min="-180" max="180" value="0" step="0.1" oninput="streamJoint(6,this.value)" onchange="sendFinalJoint(6,this.value)">
  </div>

<script>
  var ws = new WebSocket(`ws://${window.location.hostname}/ws`);
  var THROTTLE_MS = 40, lastJointSend = 0, lastGripSend = 0;

  // gripper
  var gripOpenAngle = 0, gripCloseAngle = 60, isCalibrating = false;
  var gripSlider = document.getElementById("grip"), calibStatus = document.getElementById("calibStatus");

  // profiles
  var profilesUsed = [false,false,false,false];
  var activeProfile = -1, isRecording = false, isPlaying = false, isPaused = false, pointCount = 0;

  // ----------------------------------------------------- WS inbound
  ws.onmessage = function(e) {
    var d = e.data;
    if (d.startsWith("POINTS:")) {
      pointCount = parseInt(d.split(":")[1]) || 0;
      document.getElementById("status").innerText = "Status: READY | Path Points: " + pointCount;
      var pc = document.getElementById("ptCount"); if (pc) pc.innerText = "Points: " + pointCount;
    }
    else if (d.startsWith("LIMITS:")) {
      var p = d.split(":");           // LIMITS:min1:max1:...:min6:max6
      for (var i = 0; i < 6; i++) {
        var s = document.getElementById("j" + (i+1));
        s.min = p[1 + i*2]; s.max = p[2 + i*2];
      }
    }
    else if (d.startsWith("SYNC:")) {
      var p = d.split(":");
      gripOpenAngle = parseInt(p[1]); gripCloseAngle = parseInt(p[2]);
      var cg = parseInt(p[3]), pct = 0;
      if (Math.abs(gripCloseAngle - gripOpenAngle) > 0) pct = ((cg - gripOpenAngle)/(gripCloseAngle - gripOpenAngle))*100;
      gripSlider.value = Math.round(pct);
      for (var i = 0; i < 6; i++) {
        var deg = parseFloat(p[4 + i]).toFixed(1);
        document.getElementById("j" + (i+1)).value = deg;
        document.getElementById("val" + (i+1)).innerText = deg;
      }
    }
    else if (d.startsWith("PROFILES:")) {
      var p = d.split(":");           // PROFILES:u0:u1:u2:u3:active
      for (var i = 0; i < 4; i++) profilesUsed[i] = (p[1 + i] === "1");
      activeProfile = parseInt(p[5]);
      renderProfiles();   // occupancy + active slot; record/play flags are local
    }
    else if (d.startsWith("IKRES:")) {
      var p = d.split(":"), st = document.getElementById("ikStatus");
      if (p[1] === "OK") {
        var cl = (p[6] === "1");
        st.style.color = cl ? "#ffcc00" : "#00ff88";
        st.innerText = "Moving \u2192 J1=" + p[2] + " J2=" + p[3] + " J3=" + p[4] + " J5=" + p[5] + (cl ? "  (J5 clamped)" : "");
      } else { st.style.color = "#ff3366"; st.innerText = "Rejected: " + p.slice(2).join(":"); }
    }
    else if (d.startsWith("LINERES:")) {
      var p = d.split(":"), st = document.getElementById("ikStatus");
      if (p[1] === "OK") { st.style.color = "#ce93d8"; st.innerText = "Drawing line (" + p[2] + " points)..."; }
      else { st.style.color = "#ff3366"; st.innerText = "Line rejected: " + p.slice(2).join(":"); }
    }
    else if (d.startsWith("ESTOPPED")) {
      isPlaying = false; isRecording = false; isPaused = false;
      document.getElementById("status").innerText = "Status: STOPPED (emergency) | motors halted";
      renderProfiles();
    }
  };

  // ----------------------------------------------------- profiles UI
  function selectProfile(p) {
    if (isRecording || isPlaying || isPaused) return;   // finish current action first
    activeProfile = p;
    ws.send("CMD:PROF:sel:" + p);
    renderProfiles();
  }
  function renderProfiles() {
    for (var i = 0; i < 4; i++) {
      var b = document.getElementById("prof" + i);
      b.style.background = profilesUsed[i] ? "#00c853" : "#555";
      b.style.color = profilesUsed[i] ? "#121212" : "#ddd";
      b.style.border = (i === activeProfile) ? "3px solid #fff" : "3px solid transparent";
      b.disabled = (isRecording || isPlaying || isPaused) && (i !== activeProfile);
    }
    var ctl = document.getElementById("profControls");
    var st  = document.getElementById("profStatus");
    ctl.innerHTML = "";
    if (activeProfile < 0) { st.innerText = "Select a profile."; return; }
    var n = activeProfile + 1;
    if (isRecording) {
      ctl.innerHTML =
        '<button onclick="profRecordPoint()" style="background:#4CAF50;color:#fff;">Record Point</button>' +
        '<span id="ptCount" style="align-self:center;font-weight:bold;color:#00ff88;">Points: ' + pointCount + '</span>' +
        '<button onclick="profStopSave()" style="background:#ffcc00;color:#121212;">Stop &amp; Save</button>';
      st.innerText = "Recording Profile " + n + " - jog/IK freely, press Record Point to capture each pose.";
    } else if (profilesUsed[activeProfile]) {
      var dPlay = isPlaying ? " disabled" : "", dPause = isPlaying ? "" : " disabled";
      ctl.innerHTML =
        '<button onclick="profPlay()"' + dPlay + '>Play</button>' +
        '<button onclick="profPause()" style="background:#ffcc00;color:#121212;"' + dPause + '>Pause</button>' +
        '<button onclick="profReset()" style="background:#ff3366;color:#fff;">Reset (delete)</button>';
      st.innerText = isPlaying ? ("Playing Profile " + n + " ...")
                    : (isPaused ? ("Paused - press Play to resume.") : ("Profile " + n + " ready."));
    } else {
      ctl.innerHTML = '<button onclick="profRecord()" style="background:#9c27b0;color:#fff;">Record</button>';
      st.innerText = "Profile " + n + " is empty - press Record.";
    }
  }
  function profRecord()   { isRecording = true; pointCount = 0; ws.send("CMD:PROF:rec:" + activeProfile); renderProfiles(); }
  function profRecordPoint(){ ws.send("CMD:PROF:wp"); }
  function profStopSave() { isRecording = false; ws.send("CMD:PROF:save"); renderProfiles(); }
  function profPlay()     { isPlaying = true; isPaused = false; ws.send("CMD:PROF:play:" + activeProfile); renderProfiles(); }
  function profPause()    { isPlaying = false; isPaused = true; ws.send("CMD:PROF:pause"); renderProfiles(); }
  function profReset()    { isPlaying = false; isPaused = false; ws.send("CMD:PROF:del:" + activeProfile); document.getElementById("profStatus").innerText = "Deleting Profile " + (activeProfile + 1) + " ..."; }

  // ----------------------------------------------------- gripper
  function startCalibration() {
    isCalibrating = true;
    document.getElementById("calibPanel").style.display = "flex";
    document.getElementById("btnStartCalib").style.display = "none";
    calibStatus.innerText = "Calibration: slider is raw degrees (0-180).";
    gripSlider.min = 0; gripSlider.max = 180; gripSlider.value = gripOpenAngle;
    ws.send("GRIP:" + gripOpenAngle);
  }
  function setOpenPos()  { gripOpenAngle  = parseInt(gripSlider.value); ws.send("CMD:SET_OPEN:" + gripOpenAngle);  calibStatus.innerText = "Open at " + gripOpenAngle + "\u00b0"; }
  function setClosePos() { gripCloseAngle = parseInt(gripSlider.value); ws.send("CMD:SET_CLOSE:" + gripCloseAngle); calibStatus.innerText = "Closed at " + gripCloseAngle + "\u00b0"; }
  function finishCalibration() {
    isCalibrating = false;
    document.getElementById("calibPanel").style.display = "none";
    document.getElementById("btnStartCalib").style.display = "inline-block";
    calibStatus.innerText = "Calibration finished!";
    gripSlider.min = 0; gripSlider.max = 100; gripSlider.value = 0;
    ws.send("GRIP:" + gripOpenAngle);
  }
  function streamGripper(v) {
    var now = Date.now(); if (now - lastGripSend <= THROTTLE_MS) return;
    if (isCalibrating) ws.send("GRIP:" + v);
    else ws.send("GRIP:" + Math.round(gripOpenAngle + (gripCloseAngle - gripOpenAngle) * (parseInt(v)/100)));
    lastGripSend = now;
  }
  function sendFinalGripper(v) {
    if (isCalibrating) ws.send("GRIP:" + v);
    else ws.send("GRIP:" + Math.round(gripOpenAngle + (gripCloseAngle - gripOpenAngle) * (parseInt(v)/100)));
  }

  // ----------------------------------------------------- joints (real angles)
  function streamJoint(j, v) {
    document.getElementById("val" + j).innerText = v;
    var now = Date.now(); if (now - lastJointSend <= THROTTLE_MS) return;
    ws.send("J:" + j + ":" + v); lastJointSend = now;
  }
  function sendFinalJoint(j, v) {
    ws.send("J:" + j + ":" + v);
  }

  function sendCmd(c) {
    if (c === "ESTOP" || c === "PAUSE" || c === "HOME" || c === "DEFAULT") { isPlaying = false; isPaused = false; renderProfiles(); }
    ws.send("CMD:" + c);
  }

  // ----------------------------------------------------- IK
  var ikElbowUp = true, ikToolDown = false;
  function toggleElbow() { ikElbowUp = !ikElbowUp; document.getElementById("btnElbow").innerText = "Elbow: " + (ikElbowUp ? "Up" : "Down"); }
  function toggleTool()  { ikToolDown = !ikToolDown; document.getElementById("btnTool").innerText = "Tool: " + (ikToolDown ? "Down" : "Auto"); }
  function sendIK() {
    ws.send("IK:" + document.getElementById("ikx").value + ":" + document.getElementById("iky").value + ":" +
            document.getElementById("ikz").value + ":" + (ikElbowUp ? "up" : "down") + ":" + (ikToolDown ? "down" : "auto"));
  }
  function sendLine() {
    ws.send("LINE:" + document.getElementById("ikx").value + ":" + document.getElementById("iky").value + ":" +
            document.getElementById("ikz").value + ":" + (ikElbowUp ? "up" : "down") + ":" + (ikToolDown ? "down" : "auto"));
  }

  renderProfiles();
</script>
</body>
</html>
)rawliteral";