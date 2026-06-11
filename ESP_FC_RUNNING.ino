#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <MPU6050.h>
#include <Adafruit_BMP280.h>

MPU6050 mpu;
Adafruit_BMP280 bmp;
WebServer server(80);

// -------- MODE --------
bool autoMode = true;

// -------- PHONE INPUT --------
float phoneRoll = 0;
float phonePitch = 0;
float phoneThrottle = 1300;

// -------- SENSOR --------
float roll, pitch;
float altitude, baseAltitude = 0;
bool calibrated = false;

// -------- PID --------
float Kp_angle = 1.5;
float Kp_alt = 2.0;

// -------- MOTOR OUTPUT --------
int m1, m2, m3, m4;

void handleControl() {

  Serial.println("---- REQUEST ----");

  // Print all received data
  for (int i = 0; i < server.args(); i++) {
    Serial.print(server.argName(i));
    Serial.print(" = ");
    Serial.println(server.arg(i));
  }

  // MODE SWITCH
  if (server.hasArg("mode")) {
    String mode = server.arg("mode");

    if (mode == "manual") {
      autoMode = false;
      Serial.println("Switched to MANUAL");
    } 
    else if (mode == "auto") {
      autoMode = true;
      Serial.println("Switched to AUTO");
    }
  }

  // CONTROL VALUES
  if (server.hasArg("roll")) {
    phoneRoll = server.arg("roll").toFloat();
  }

  if (server.hasArg("pitch")) {
    phonePitch = server.arg("pitch").toFloat();
  }

  if (server.hasArg("throttle")) {
    phoneThrottle = server.arg("throttle").toFloat();
  }

  Serial.print("Stored → ");
  Serial.print(phoneRoll); Serial.print(" ");
  Serial.print(phonePitch); Serial.print(" ");
  Serial.println(phoneThrottle);

  server.send(200, "text/plain", "OK");

}

const char* html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Drone Remote</title>
</head>
<body>
  <h2>Drone Controller</h2>

  <button onclick="setMode('auto')">AUTO</button>
  <button onclick="setMode('manual')">MANUAL</button>
  <br><br>

  Throttle:<br>
  <input type="range" min="1000" max="1600" value="1200" id="throttle"><br>

  Roll:<br>
  <input type="range" min="-30" max="30" value="0" id="roll"><br>

  Pitch:<br>
  <input type="range" min="-30" max="30" value="0" id="pitch"><br><br>

  <script>
    function setMode(m) {
      fetch(`/control?mode=${m}`);
    }

    function sendData() {
      let t = document.getElementById("throttle").value;
      let r = document.getElementById("roll").value;
      let p = document.getElementById("pitch").value;

      fetch(`/control?roll=${r}&pitch=${p}&throttle=${t}`);
    }

    setInterval(sendData, 150); // send every 150ms
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // -------- INIT MPU --------
  mpu.initialize();

  // -------- INIT BMP --------
  if (!bmp.begin(0x76)) {
    Serial.println("BMP FAIL");
    while (1);
  }

  bmp.setSampling(
    Adafruit_BMP280::MODE_NORMAL,
    Adafruit_BMP280::SAMPLING_X2,
    Adafruit_BMP280::SAMPLING_X16,
    Adafruit_BMP280::FILTER_X16,
    Adafruit_BMP280::STANDBY_MS_500
  );

  // -------- WIFI --------
  WiFi.softAP("DroneFC", "12345678");
  server.on("/control", handleControl);
  server.begin();

  Serial.println("System Ready");

server.on("/", []() {
  server.send(200, "text/html", html);
});

}

// -------- READ IMU --------
void readIMU() {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  roll = atan2(ay, az) * 180 / PI;
  pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180 / PI;
}

// -------- READ ALTITUDE --------
void readAltitude() {
  float rawAlt = bmp.readAltitude(1013.25);

  if (!calibrated) {
    baseAltitude = rawAlt;
    calibrated = true;
  }

  altitude = rawAlt - baseAltitude;
}

// -------- PID --------
float pid(float target, float current, float Kp) {
  float error = target - current;
  return Kp * error;
}

void loop() {
  server.handleClient();

  readIMU();
  readAltitude();

  float rollCmd, pitchCmd, throttleCmd;

  // -------- MODE SWITCH --------
if (autoMode) {
      rollCmd = pid(0, roll, Kp_angle);
    pitchCmd = pid(0, pitch, Kp_angle);
    throttleCmd = 1300 + pid(0, altitude, Kp_alt);
} else {
  float manualGain = 1.5; // or 2.0 max

  rollCmd  = phoneRoll  * manualGain;
  pitchCmd = phonePitch * manualGain;
  throttleCmd = phoneThrottle;
  throttleCmd = constrain(throttleCmd, 1000, 1400);
}
rollCmd  = constrain(rollCmd,  -80, 80);
pitchCmd = constrain(pitchCmd, -80, 80);

  // -------- MOTOR MIXING --------
  m1 = throttleCmd + pitchCmd + rollCmd;
  m2 = throttleCmd + pitchCmd - rollCmd;
  m3 = throttleCmd - pitchCmd + rollCmd;
  m4 = throttleCmd - pitchCmd - rollCmd;

  m1 = constrain(m1, 1000, 2000);
  m2 = constrain(m2, 1000, 2000);
  m3 = constrain(m3, 1000, 2000);
  m4 = constrain(m4, 1000, 2000);

  // -------- DEBUG --------
  Serial.print("Mode: ");
  Serial.print(autoMode ? "AUTO" : "MANUAL");
  Serial.print(" Roll: "); Serial.print(roll);
  Serial.print(" Pitch: "); Serial.print(pitch);
  Serial.print(" Alt: "); Serial.println(altitude);

  Serial.print(" | M1: "); Serial.print(m1);
Serial.print(" M2: "); Serial.print(m2);
Serial.print(" M3: "); Serial.print(m3);
Serial.print(" M4: "); Serial.println(m4);

// Serial.print("Cmd Roll: "); Serial.print(rollCmd);
// Serial.print(" Cmd Pitch: "); Serial.print(pitchCmd);
// Serial.print(" Cmd Throttle: "); Serial.println(throttleCmd);

  delay(10);
}