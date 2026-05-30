/*
 * Slave 3 Temperature Control - TK-301
 * SOLOPHOBIA - Heating & Mixing Miniplant
 *
 * RS485 master-slave:
 *   S3:PING
 *   S3:READ
 *   S3:SETSP:<suhu>
 *   S3:START
 *   S3:STOP
 *   S3:RESET
 *
 * Wiring RS485 disamakan dengan Slave 1 & 2:
 *   DE/RE = D2
 *   RO    = D10
 *   DI    = D11
 *
 * Pin lain:
 *   DS18B20 DATA  = D3 (1-Wire dengan pull-up 4.7kΩ ke 5V)
 *   SSR TY-301 DC+ = D4 (HIGH = ON)
 *   TB6612FNG STBY = D5
 *   TB6612FNG AIN1 = D6
 *   TB6612FNG AIN2 = D7
 *   TB6612FNG PWMA = D9
 *
 * Kendali: HANYA FEEDBACK (ON/OFF deadband 59-61°C)
 * Mixer: 2 speed (40% saat heater ON, 60% saat heater OFF)
 */

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// RS485 Pin
const int RS485_DE_RE_PIN = 2;
const int RS485_RO_PIN = 10;
const int RS485_DI_PIN = 11;

// Sensor & Actuator Pin
const int DS18B20_PIN = 3;
const int SSR_HEATER_PIN = 4;
const int PIN_STBY = 5;
const int PIN_AIN1 = 6;
const int PIN_AIN2 = 7;
const int PIN_PWMA = 9;

const bool RS485_ACTIVE_HIGH = true;

SoftwareSerial SlaveBus(RS485_RO_PIN, RS485_DI_PIN);
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

// Parameter Sistem
const float SP_FIXED_SUHU_C = 60.0;
const float DEFAULT_SUHU_SP_C = 60.0;
const float SUHU_DEADBAND_LOW = 59.0;
const float SUHU_DEADBAND_HIGH = 61.0;
const float SUHU_MIN_C = 25.0;
const float SUHU_MAX_C = 90.0;
const unsigned long PROCESS_DURATION_SEC = 420; // 7 menit
const unsigned long SAMPLE_INTERVAL_MS = 1000;

// Motor Mixer (2 Speed)
const int MOTOR_SPEED_NORMAL = 102; // 40% PWM (0-255)
const int MOTOR_SPEED_CEPAT = 153;  // 60% PWM (0-255)

// Variabel Sistem
float suhuSpC = DEFAULT_SUHU_SP_C;
float suhuPvC = 0.0;
bool heaterActive = false;
bool mixerActive = false;
bool processActive = false;
unsigned long processTimerSec = 0; // Countdown timer
unsigned long lastSampleMs = 0;

DeviceAddress tempDeviceAddress;

void setup() {
  Serial.begin(9600);
  delay(500);

  // RS485
  pinMode(RS485_DE_RE_PIN, OUTPUT);
  digitalWrite(RS485_DE_RE_PIN, LOW);

  // SSR Heater
  pinMode(SSR_HEATER_PIN, OUTPUT);
  digitalWrite(SSR_HEATER_PIN, LOW);

  // TB6612FNG Motor Driver
  pinMode(PIN_STBY, OUTPUT);
  pinMode(PIN_AIN1, OUTPUT);
  pinMode(PIN_AIN2, OUTPUT);
  pinMode(PIN_PWMA, OUTPUT);
  stopMotor();

  // Slave bus
  SlaveBus.begin(9600);

  // DS18B20
  sensors.begin();
  if (sensors.getAddress(tempDeviceAddress, 0)) {
    sensors.setResolution(tempDeviceAddress, 10);
  }

  // Baca suhu awal
  sensors.requestTemperatures();
  suhuPvC = sensors.getTempC(tempDeviceAddress);

  Serial.println("=== SLAVE 3 - TK-301 TEMP CONTROL ===");
  Serial.println("S3:PING, S3:READ, S3:SETSP:<C>, S3:START, S3:STOP, S3:RESET");
  Serial.println("SP FIXED: " + String(SP_FIXED_SUHU_C) + " C");
  Serial.println("PROCESS DURATION: " + String(PROCESS_DURATION_SEC) + " sec");
  Serial.println("======================================");
}

void loop() {
  unsigned long now = millis();

  // Sampling suhu setiap 1 detik
  if (now - lastSampleMs >= SAMPLE_INTERVAL_MS) {
    lastSampleMs = now;

    // Baca suhu
    if (sensors.requestTemperatures()) {
      float tempC = sensors.getTempC(tempDeviceAddress);
      if (tempC != DEVICE_DISCONNECTED_C && tempC >= SUHU_MIN_C && tempC <= SUHU_MAX_C) {
        suhuPvC = tempC;
      }
    }

    // Kendali Heater (Hysteresis: ON di 59, OFF di 61)
    if (processActive) {
      if (suhuPvC < SUHU_DEADBAND_LOW) {
        heaterActive = true;
      } else if (suhuPvC >= SUHU_DEADBAND_HIGH) {
        heaterActive = false;
      }
    } else {
      heaterActive = false;
    }
    digitalWrite(SSR_HEATER_PIN, heaterActive ? HIGH : LOW);

    // Kendali Mixer (2 speed berdasarkan heater state)
    if (processActive) {
      mixerActive = true;
      int motorSpeed;
      String speedLabel;

      if (heaterActive) {
        motorSpeed = MOTOR_SPEED_NORMAL; // 40%
        speedLabel = "NORMAL";
      } else {
        motorSpeed = MOTOR_SPEED_CEPAT;  // 60%
        speedLabel = "CEPAT";
      }

      runMotor(motorSpeed);
    } else {
      mixerActive = false;
      stopMotor();
    }

    // Timer countdown
    if (processActive) {
      if (processTimerSec > 0) {
        processTimerSec--;
      } else {
        // Timer habis
        processActive = false;
        heaterActive = false;
        mixerActive = false;
        stopMotor();
        digitalWrite(SSR_HEATER_PIN, LOW);
      }
    }
  }

  // Handle Serial dari Master
  if (SlaveBus.available()) {
    String command = SlaveBus.readStringUntil('\n');
    command.trim();
    handleCommand(command);
  }

  // Handle Serial lokal (debug)
  if (Serial.available()) {
    char cmd = Serial.read();
    while (Serial.available()) Serial.read();
    switch (cmd) {
      case 'd': case 'D': printStatus(); break;
      case 'c': case 'C':
        Serial.print("Suhu PV: "); Serial.print(suhuPvC, 1); Serial.println(" C");
        break;
      case 'h':
        heaterActive = true;
        digitalWrite(SSR_HEATER_PIN, HIGH);
        Serial.println("Heater: ON");
        break;
      case 'H':
        heaterActive = false;
        digitalWrite(SSR_HEATER_PIN, LOW);
        Serial.println("Heater: OFF");
        break;
      case '1':
        Serial.println("Mixer Speed: NORMAL (40%)");
        mixerActive = true;
        runMotor(MOTOR_SPEED_NORMAL);
        break;
      case '2':
        Serial.println("Mixer Speed: CEPAT (60%)");
        mixerActive = true;
        runMotor(MOTOR_SPEED_CEPAT);
        break;
      case 's': case 'S':
        mixerActive = false;
        stopMotor();
        Serial.println("Mixer: STOP");
        break;
      case '?': printMenu(); break;
      case '\n': case '\r': break;
      default: break;
    }
  }
}

void handleCommand(String command) {
  if (command.length() < 4) return;

  String prefix = command.substring(0, 3);
  if (prefix != "S3:") return;

  String cmd = command.substring(3);

  if (cmd == "PING") {
    sendResponse("S3:OK");
  }
  else if (cmd == "READ") {
    String speedLabel = (heaterActive) ? "NORMAL" : "CEPAT";
    sendResponse("S3:OK,SP=" + String(suhuSpC, 1) + ",PV=" + String(suhuPvC, 1) +
                ",HEATER=" + (heaterActive ? "ON" : "OFF") +
                ",MIXER=" + (mixerActive ? "ON" : "OFF") +
                ",SPEED=" + speedLabel +
                ",TIMER=" + String(processTimerSec) +
                ",PROCESS=" + (processActive ? "RUN" : "STOP"));
  }
  else if (cmd.startsWith("SETSP:")) {
    float newSp = cmd.substring(6).toFloat();
    if (newSp >= SUHU_MIN_C && newSp <= SUHU_MAX_C) {
      suhuSpC = newSp;
      sendResponse("S3:OK,SETSP=" + String(suhuSpC, 1));
    } else {
      sendResponse("S3:ERR,INVALID_SP");
    }
  }
  else if (cmd == "START") {
    // Start dengan SP FIXED 60°C
    suhuSpC = SP_FIXED_SUHU_C;
    processActive = true;
    processTimerSec = PROCESS_DURATION_SEC;
    heaterActive = true;
    mixerActive = true;
    sendResponse("S3:OK,START,SP=" + String(suhuSpC, 1) + ",DURATION=" + String(PROCESS_DURATION_SEC) + ",PROCESS=RUN");
  }
  else if (cmd == "STOP") {
    processActive = false;
    heaterActive = false;
    mixerActive = false;
    stopMotor();
    digitalWrite(SSR_HEATER_PIN, LOW);
    sendResponse("S3:OK,STOP,HEATER=OFF,MIXER=OFF,PROCESS=STOP");
  }
  else if (cmd == "RESET") {
    processActive = false;
    heaterActive = false;
    mixerActive = false;
    processTimerSec = 0;
    suhuSpC = DEFAULT_SUHU_SP_C;
    stopMotor();
    digitalWrite(SSR_HEATER_PIN, LOW);
    sendResponse("S3:OK,RESET");
  }
  else if (cmd == "HEATER:ON") {
    heaterActive = true;
    digitalWrite(SSR_HEATER_PIN, HIGH);
    sendResponse("S3:OK,HEATER=ON");
  }
  else if (cmd == "HEATER:OFF") {
    heaterActive = false;
    digitalWrite(SSR_HEATER_PIN, LOW);
    sendResponse("S3:OK,HEATER=OFF");
  }
  else if (cmd == "MIXER:NORMAL") {
    mixerActive = true;
    runMotor(MOTOR_SPEED_NORMAL);
    sendResponse("S3:OK,MIXER=ON,SPEED=NORMAL");
  }
  else if (cmd == "MIXER:CEPAT") {
    mixerActive = true;
    runMotor(MOTOR_SPEED_CEPAT);
    sendResponse("S3:OK,MIXER=ON,SPEED=CEPAT");
  }
  else if (cmd == "MIXER:STOP") {
    mixerActive = false;
    stopMotor();
    sendResponse("S3:OK,MIXER=OFF,SPEED=STOP");
  }
  else {
    sendResponse("S3:ERR,UNKNOWN_CMD");
  }
}

void sendResponse(String response) {
  setTransmitMode(true);
  SlaveBus.print(response);
  SlaveBus.print('\n');
  SlaveBus.flush();
  delay(10);
  setTransmitMode(false);
}

void setTransmitMode(bool tx) {
  digitalWrite(RS485_DE_RE_PIN, tx ? HIGH : LOW);
  delayMicroseconds(50);
}

void runMotor(int speed) {
  digitalWrite(PIN_STBY, HIGH);
  digitalWrite(PIN_AIN1, HIGH);
  digitalWrite(PIN_AIN2, LOW);
  analogWrite(PIN_PWMA, speed);
}

void stopMotor() {
  digitalWrite(PIN_STBY, LOW);
  digitalWrite(PIN_AIN1, LOW);
  digitalWrite(PIN_AIN2, LOW);
  analogWrite(PIN_PWMA, 0);
}

void printStatus() {
  String speedLabel = (heaterActive) ? "NORMAL" : "CEPAT";
  Serial.println("\n=== STATUS SLAVE 3 ===");
  Serial.print("Suhu SP: "); Serial.print(suhuSpC, 1); Serial.println(" C");
  Serial.print("Suhu PV: "); Serial.print(suhuPvC, 1); Serial.println(" C");
  Serial.print("Heater: "); Serial.println(heaterActive ? "ON" : "OFF");
  Serial.print("Mixer: "); Serial.println(mixerActive ? "ON" : "OFF");
  Serial.print("Mixer Speed: "); Serial.println(speedLabel);
  Serial.print("Timer: "); Serial.print(processTimerSec); Serial.println(" / ");
  Serial.println(PROCESS_DURATION_SEC);
  Serial.print("Process: "); Serial.println(processActive ? "RUN" : "STOP");
  Serial.println("=====================\n");
}

void printMenu() {
  Serial.println("\n=== MENU SLAVE 3 (DEBUG) ===");
  Serial.println("d - Display status");
  Serial.println("c - Continuous temp display");
  Serial.println("h - Heater ON manual");
  Serial.println("H - Heater OFF manual");
  Serial.println("1 - Mixer NORMAL (40%)");
  Serial.println("2 - Mixer CEPAT (60%)");
  Serial.println("s - Mixer STOP");
  Serial.println("? - Menu ini");
  Serial.println("=====================\n");
}
