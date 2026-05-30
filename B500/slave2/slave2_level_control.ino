/*
 * Slave 2 Level Control - TK-201
 * SOLOPHOBIA - Heating & Mixing Miniplant
 *
 * RS485 master-slave:
 *   S2:PING
 *   S2:READ
 *   S2:SETSP:<cm>
 *   S2:START[:<cm>]
 *   S2:STOP
 *   S2:RESET
 *
 * Wiring RS485 disamakan dengan Slave 1:
 *   DE/RE = D2
 *   RO    = D10
 *   DI    = D11
 *
 * Pin lain disesuaikan supaya tidak bentrok:
 *   HC-SR04 TRIG = D4
 *   HC-SR04 ECHO = D12
 *   TB6612FNG STBY = D5
 *   TB6612FNG AIN2 = D7
 *   TB6612FNG AIN1 = D8
 *   TB6612FNG PWMA = D9
 */

#include <Arduino.h>
#include <SoftwareSerial.h>

const int RS485_DE_RE_PIN = 2;
const int RS485_RO_PIN = 10;
const int RS485_DI_PIN = 11;

const int TRIG_PIN = 4;
const int ECHO_PIN = 12;

const int PIN_STBY = 5;
const int PIN_AIN2 = 7;
const int PIN_AIN1 = 8;
const int PIN_PWMA = 9;

const bool RS485_ACTIVE_HIGH = true;

SoftwareSerial SlaveBus(RS485_RO_PIN, RS485_DI_PIN);

const float KECEPATAN_SUARA = 0.0343;   // cm/us pada 20 C
const float FAKTOR_KALIBRASI_SENSOR = 1.00;  // Tidak perlu koreksi, sensor sudah akurat
const float TINGGI_SENSOR_CM = 24.2;     // Jarak sensor ke dasar (setelah pindah)
const float LUAS_PENAMPANG_CM2 = 254.1;  // Luas penampang (tetap)
const float TINGGI_MAX_CM = 22.0;        // Batas maksimum tinggi texapon
const float VOLUME_MAX_ML = 5590;        // 254.1 x 22 = volume maksimal

// SP FIXED untuk produksi (100 mL texapon)
const float SP_FIXED_TEXAPON_ML = 100.0;
const float SP_FIXED_TINGGI_CM = SP_FIXED_TEXAPON_ML / LUAS_PENAMPANG_CM2;  // 0.39 cm
const float SP_FIXED_JARAK_CM = TINGGI_SENSOR_CM - SP_FIXED_TINGGI_CM;      // 23.81 cm

const float DEADBAND_CM = 0.30;
const int MOTOR_SPEED_DEFAULT = 150;
const unsigned long CONTROL_INTERVAL_MS = 500;
const unsigned long PROCESS_TIMEOUT_MS = 300000;
const byte MOVING_AVG_SIZE = 5;

float jarakSpCm = 15.0;
float jarakPvCm = -1.0;
float volumeSpMl = 0.0;
float volumePvMl = 0.0;
float errorCm = 0.0;

int motorSpeed = MOTOR_SPEED_DEFAULT;
int motorDirection = 0;  // 1=UP, -1=DOWN, 0=STOP
bool motorRunning = false;
bool autoControl = false;
bool sensorFault = false;
unsigned long lastControlMs = 0;
unsigned long autoStartMs = 0;
String inputLine = "";

void setup() {
  Serial.begin(9600);
  SlaveBus.begin(9600);
  SlaveBus.listen();

  pinMode(RS485_DE_RE_PIN, OUTPUT);
  digitalWrite(RS485_DE_RE_PIN, LOW);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(PIN_STBY, OUTPUT);
  pinMode(PIN_AIN1, OUTPUT);
  pinMode(PIN_AIN2, OUTPUT);
  pinMode(PIN_PWMA, OUTPUT);

  digitalWrite(TRIG_PIN, LOW);
  motorStop();
  volumeSpMl = jarakKeVolume(jarakSpCm);

  Serial.println(F("SLAVE 2 READY - TK-201 LEVEL CONTROL"));
}

void loop() {
  handleSlaveBus();

  if (autoControl && millis() - lastControlMs >= CONTROL_INTERVAL_MS) {
    runControlStep();
    lastControlMs = millis();
  }

  if (autoControl && millis() - autoStartMs > PROCESS_TIMEOUT_MS) {
    autoControl = false;
    sensorFault = true;
    motorStop();
  }
}

void handleSlaveBus() {
  while (SlaveBus.available() > 0) {
    char c = SlaveBus.read();
    if (c == '\n' || c == '\r') {
      if (inputLine.length() > 0) {
        handleCommand(inputLine);
        inputLine = "";
      }
    } else if (inputLine.length() < 120) {
      inputLine += c;
    }
  }
}

void handleCommand(String cmd) {
  cmd.trim();
  if (!cmd.startsWith("S2:")) {
    return;
  }

  if (cmd == "S2:PING") {
    sendLine(F("S2:OK"));
  } else if (cmd == "S2:READ") {
    readProcessValues();
    sendData();
  } else if (cmd.startsWith("S2:SETSP:")) {
    float sp = cmd.substring(9).toFloat();
    if (sp > 0.0) {
      jarakSpCm = clampJarak(sp);
      volumeSpMl = jarakKeVolume(jarakSpCm);
      sendText(F("S2:OK,SETSP="));
      sendFloatLine(jarakSpCm, 2);
    } else {
      sendLine(F("S2:ERR,SETSP"));
    }
  } else if (cmd == "S2:START:FIXED") {
    jarakSpCm = SP_FIXED_JARAK_CM;
    volumeSpMl = jarakKeVolume(jarakSpCm);
    autoControl = true;
    sensorFault = false;
    autoStartMs = millis();
    lastControlMs = 0;
    runControlStep();
    sendText(F("S2:OK,START,FIXED,SP="));
    sendFloatLine(jarakSpCm, 2);
  } else if (cmd == "S2:START" || cmd.startsWith("S2:START:")) {
    if (cmd.startsWith("S2:START:") && cmd.length() > 9) {
      float sp = cmd.substring(9).toFloat();
      if (sp > 0.0) {
        jarakSpCm = clampJarak(sp);
        volumeSpMl = jarakKeVolume(jarakSpCm);
      }
    }

    autoControl = true;
    sensorFault = false;
    autoStartMs = millis();
    lastControlMs = 0;
    runControlStep();

    sendText(F("S2:OK,START,SP="));
    sendFloatLine(jarakSpCm, 2);
  } else if (cmd == "S2:STOP") {
    autoControl = false;
    sensorFault = false;
    motorStop();
    sendLine(F("S2:OK,STOP"));
  } else if (cmd == "S2:RESET") {
    autoControl = false;
    sensorFault = false;
    motorStop();
    readProcessValues();
    sendLine(F("S2:OK,RESET"));
  } else if (cmd == "S2:MOTOR:UP") {
    motorUp(motorSpeed);
    sendLine(F("S2:OK,MOTOR=UP"));
  } else if (cmd == "S2:MOTOR:DOWN") {
    motorDown(motorSpeed);
    sendLine(F("S2:OK,MOTOR=DOWN"));
  } else if (cmd == "S2:MOTOR:STOP") {
    motorStop();
    sendLine(F("S2:OK,MOTOR=STOP"));
  } else if (cmd.startsWith("S2:MOTOR:SPEED:")) {
    int speed = cmd.substring(16).toInt();
    if (speed >= 0 && speed <= 255) {
      motorSpeed = speed;
      sendText(F("S2:OK,SPEED="));
      sendFloatLine(motorSpeed, 0);
    } else {
      sendLine(F("S2:ERR,SPEED"));
    }
  } else {
    sendLine(F("S2:ERR,UNKNOWN_CMD"));
  }
}

void setTransmitMode(bool tx) {
  digitalWrite(RS485_DE_RE_PIN, tx ? HIGH : LOW);
  delayMicroseconds(50);
}

void sendLine(const __FlashStringHelper *msg) {
  setTransmitMode(true);
  SlaveBus.println(msg);
  SlaveBus.flush();
  setTransmitMode(false);
  SlaveBus.listen();
}

void sendText(const __FlashStringHelper *msg) {
  setTransmitMode(true);
  SlaveBus.print(msg);
}

void sendFloatLine(float value, int digits) {
  SlaveBus.println(value, digits);
  SlaveBus.flush();
  setTransmitMode(false);
  SlaveBus.listen();
}

void sendData() {
  setTransmitMode(true);
  SlaveBus.print(F("S2:DATA,SP="));
  SlaveBus.print(jarakSpCm, 2);
  SlaveBus.print(F(",PV="));
  SlaveBus.print(jarakPvCm, 2);
  SlaveBus.print(F(",ERR="));
  SlaveBus.print(errorCm, 2);
  SlaveBus.print(F(",VOLSP="));
  SlaveBus.print(volumeSpMl, 0);
  SlaveBus.print(F(",VOLPV="));
  SlaveBus.print(volumePvMl, 0);
  SlaveBus.print(F(",MOTOR="));
  printMotorStateBus();
  SlaveBus.print(F(",AUTO="));
  SlaveBus.print(autoControl ? F("ON") : F("OFF"));
  SlaveBus.print(F(",STATUS="));
  SlaveBus.println(getStatusText());
  SlaveBus.flush();
  setTransmitMode(false);
  SlaveBus.listen();
}

void runControlStep() {
  if (!readProcessValues()) {
    motorStop();
    sensorFault = true;
    return;
  }

  sensorFault = false;
  errorCm = jarakPvCm - jarakSpCm;

  if (errorCm > DEADBAND_CM) {
    motorUp(motorSpeed);
  } else if (errorCm < -DEADBAND_CM) {
    motorDown(motorSpeed);
  } else {
    motorStop();
  }
}

bool readProcessValues() {
  float jarak = readDistanceCm();
  float jarakMin = TINGGI_SENSOR_CM - TINGGI_MAX_CM;  // Plunger di atas (penuh)
  float jarakMax = TINGGI_SENSOR_CM + 2.0;             // Toleransi di bawah dasar

  if (jarak < jarakMin - 0.5 || jarak > jarakMax) {
    jarakPvCm = -1.0;
    volumePvMl = 0.0;
    errorCm = 0.0;
    return false;
  }

  jarakPvCm = jarak;
  volumePvMl = jarakKeVolume(jarakPvCm);
  errorCm = jarakPvCm - jarakSpCm;
  return true;
}

float readDistanceCm() {
  float total = 0.0;
  byte validCount = 0;

  for (byte i = 0; i < MOVING_AVG_SIZE; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    if (duration > 0) {
      float distance = (duration * KECEPATAN_SUARA) / 2.0;
      if (distance > 0.0 && distance < 100.0) {
        total += distance;
        validCount++;
      }
    }
    delay(10);
  }

  if (validCount == 0) {
    return -1.0;
  }
  return (total / validCount) * FAKTOR_KALIBRASI_SENSOR;  // Terapkan faktor kalibrasi
}

void motorUp(int speed) {
  digitalWrite(PIN_STBY, HIGH);
  digitalWrite(PIN_AIN1, HIGH);
  digitalWrite(PIN_AIN2, LOW);
  analogWrite(PIN_PWMA, constrain(speed, 0, 255));
  motorDirection = 1;
  motorRunning = true;
}

void motorDown(int speed) {
  digitalWrite(PIN_STBY, HIGH);
  digitalWrite(PIN_AIN1, LOW);
  digitalWrite(PIN_AIN2, HIGH);
  analogWrite(PIN_PWMA, constrain(speed, 0, 255));
  motorDirection = -1;
  motorRunning = true;
}

void motorStop() {
  digitalWrite(PIN_STBY, HIGH);
  digitalWrite(PIN_AIN1, HIGH);
  digitalWrite(PIN_AIN2, HIGH);
  analogWrite(PIN_PWMA, 0);
  motorDirection = 0;
  motorRunning = false;
}

void printMotorStateBus() {
  if (!motorRunning) {
    SlaveBus.print(F("STOP"));
  } else if (motorDirection > 0) {
    SlaveBus.print(F("UP"));
  } else if (motorDirection < 0) {
    SlaveBus.print(F("DOWN"));
  } else {
    SlaveBus.print(F("UNKNOWN"));
  }
}

String getStatusText() {
  if (sensorFault) {
    return F("FAULT");
  }
  if (autoControl) {
    return F("RUNNING");
  }
  return F("STOP");
}

float jarakKeVolume(float jarakCm) {
  float tinggi = TINGGI_SENSOR_CM - jarakCm;
  tinggi = constrain(tinggi, 0.0, TINGGI_MAX_CM);
  return LUAS_PENAMPANG_CM2 * tinggi;
}

float volumeKeJarak(float volumeMl) {
  float tinggi = constrain(volumeMl / LUAS_PENAMPANG_CM2, 0.0, TINGGI_MAX_CM);
  return clampJarak(TINGGI_SENSOR_CM - tinggi);
}

float clampJarak(float jarakCm) {
  float jarakMin = TINGGI_SENSOR_CM - TINGGI_MAX_CM;  // Plunger di atas (penuh)
  float jarakMax = TINGGI_SENSOR_CM;                   // Plunger di dasar (kosong)
  return constrain(jarakCm, jarakMin, jarakMax);
}
