/*
 * Slave 1 Flow Control - TK-101
 * SOLOPHOBIA - Heating & Mixing Miniplant
 *
 * Protokol master-slave via Serial 9600:
 *   S1:PING
 *   S1:READ
 *   S1:SETSP:<mL>
 *   S1:START:<mL>
 *   S1:STOP
 *   S1:RESET
 *   S1:RELAY:ON
 *   S1:RELAY:OFF
 */

#include <Arduino.h>
#include <SoftwareSerial.h>

const int RS485_DE_RE_PIN = 2;
const int FLOW_SENSOR_PIN = 3;
const int RELAY_PIN = 4;
const int RS485_RO_PIN = 10;
const int RS485_DI_PIN = 11;
const bool RELAY_ACTIVE_LOW = true;

SoftwareSerial SlaveBus(RS485_RO_PIN, RS485_DI_PIN);

const float ML_PER_PULSE_KALIBRASI = 0.0053;
const float ML_PER_PULSE_DATASHEET = 1000.0 / 98.0;
const bool PAKAI_FAKTOR_KALIBRASI = true;

// SP FIXED untuk produksi (900 mL air)
const float SP_FIXED_VOLUME_ML = 900.0;
const float DEFAULT_VOLUME_SP_ML = 900.0;
const float VOLUME_TOLERANCE_ML = 0.0;
const unsigned long SAMPLE_INTERVAL_MS = 1000;
// const unsigned long PROCESS_TIMEOUT_MS = 300000;  // 5 menit (lama)
const unsigned long PROCESS_TIMEOUT_MS = 900000;  // 15 menit (baru)

volatile unsigned long pulseCount = 0;
volatile unsigned long totalPulseAccumulated = 0;
volatile unsigned long lastPulseTime = 0;

float volumeSpMl = DEFAULT_VOLUME_SP_ML;
float volumePvMl = 0.0;
float flowMlMin = 0.0;
unsigned long processStartPulse = 0;
unsigned long processStartMs = 0;
unsigned long lastSampleMs = 0;
unsigned long lastFlowPulseSnapshot = 0;
unsigned long lastFlowMs = 0;
bool pumpRunning = false;
bool processRunning = false;
String inputLine = "";

void setup() {
  Serial.begin(9600);
  SlaveBus.begin(9600);
  SlaveBus.listen();

  pinMode(RS485_DE_RE_PIN, OUTPUT);
  digitalWrite(RS485_DE_RE_PIN, LOW);
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  pumpOff();
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, FALLING);
}

void loop() {
  if (processRunning && millis() - lastSampleMs >= SAMPLE_INTERVAL_MS) {
    updateProcess();
    lastSampleMs = millis();
  }

  while (SlaveBus.available() > 0) {
    char c = SlaveBus.read();
    if (c == '\n' || c == '\r') {
      if (inputLine.length() > 0) {
        handleCommand(inputLine);
        inputLine = "";
      }
    } else if (inputLine.length() < 80) {
      inputLine += c;
    }
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

void handleCommand(String cmd) {
  cmd.trim();
  if (!cmd.startsWith("S1:")) {
    return;
  }

  if (cmd == "S1:PING") {
    sendLine(F("S1:OK"));
  } else if (cmd == "S1:READ") {
    updateVolumeSnapshot();
    sendData();
  } else if (cmd.startsWith("S1:SETSP:")) {
    float sp = cmd.substring(9).toFloat();
    if (sp > 0.0 && sp <= 5000.0) {
      volumeSpMl = sp;
      sendText(F("S1:OK,SETSP="));
      sendFloatLine(volumeSpMl, 1);
    } else {
      sendLine(F("S1:ERR,SETSP"));
    }
  } else if (cmd == "S1:START:FIXED") {
    volumeSpMl = SP_FIXED_VOLUME_ML;
    startProcess();
    sendText(F("S1:OK,START,FIXED,SP="));
    sendFloatLine(volumeSpMl, 1);
  } else if (cmd.startsWith("S1:START:")) {
    float sp = cmd.substring(9).toFloat();
    if (sp > 0.0 && sp <= 5000.0) {
      volumeSpMl = sp;
    }
    startProcess();
    sendText(F("S1:OK,START,SP="));
    sendFloatLine(volumeSpMl, 1);
  } else if (cmd == "S1:STOP") {
    emergencyStop();
    sendLine(F("S1:OK,STOP"));
  } else if (cmd == "S1:RESET") {
    resetVolume();
    sendLine(F("S1:OK,RESET"));
  } else if (cmd == "S1:RELAY:ON") {
    pumpOn();
    sendLine(F("S1:OK,RELAY=ON"));
  } else if (cmd == "S1:RELAY:OFF") {
    processRunning = false;
    pumpOff();
    sendLine(F("S1:OK,RELAY=OFF"));
  } else {
    sendLine(F("S1:ERR,UNKNOWN_CMD"));
  }
}

void startProcess() {
  resetVolume();
  processRunning = true;
  processStartMs = millis();
  lastSampleMs = 0;
  pumpOn();
}

void updateProcess() {
  updateVolumeSnapshot();

  if (volumePvMl >= volumeSpMl - VOLUME_TOLERANCE_ML) {
    pumpOff();
    processRunning = false;
    return;
  }

  if (millis() - processStartMs >= PROCESS_TIMEOUT_MS) {
    pumpOff();
    processRunning = false;
    return;
  }

  if (!pumpRunning) {
    pumpOn();
  }
}

void updateVolumeSnapshot() {
  noInterrupts();
  unsigned long totalPulse = totalPulseAccumulated - processStartPulse;
  unsigned long totalNow = totalPulseAccumulated;
  interrupts();

  float mlPerPulse = getMlPerPulse();
  volumePvMl = totalPulse * mlPerPulse;

  unsigned long now = millis();
  unsigned long deltaPulse = totalNow - lastFlowPulseSnapshot;
  unsigned long deltaMs = now - lastFlowMs;
  if (deltaMs > 0) {
    flowMlMin = (deltaPulse * mlPerPulse) * (60000.0 / deltaMs);
  }

  lastFlowPulseSnapshot = totalNow;
  lastFlowMs = now;
}

void resetVolume() {
  noInterrupts();
  processStartPulse = totalPulseAccumulated;
  pulseCount = 0;
  interrupts();

  lastFlowPulseSnapshot = totalPulseAccumulated;
  lastFlowMs = millis();
  volumePvMl = 0.0;
  flowMlMin = 0.0;
}

void sendData() {
  noInterrupts();
  unsigned long totalPulse = totalPulseAccumulated - processStartPulse;
  interrupts();

  setTransmitMode(true);
  SlaveBus.print(F("S1:DATA,SP="));
  SlaveBus.print(volumeSpMl, 1);
  SlaveBus.print(F(",PV="));
  SlaveBus.print(volumePvMl, 1);
  SlaveBus.print(F(",FLOW="));
  SlaveBus.print(flowMlMin, 1);
  SlaveBus.print(F(",PULSE="));
  SlaveBus.print(totalPulse);
  SlaveBus.print(F(",RELAY="));
  SlaveBus.print(pumpRunning ? F("ON") : F("OFF"));
  SlaveBus.print(F(",PROCESS="));
  SlaveBus.println(processRunning ? F("RUNNING") : F("STOP"));
  SlaveBus.flush();
  setTransmitMode(false);
  SlaveBus.listen();
}

void pulseCounter() {
  pulseCount++;
  totalPulseAccumulated++;
  lastPulseTime = millis();
}

void pumpOn() {
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? LOW : HIGH);
  pumpRunning = true;
}

void pumpOff() {
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
  pumpRunning = false;
}

void emergencyStop() {
  processRunning = false;
  pumpOff();
}

float getMlPerPulse() {
  return PAKAI_FAKTOR_KALIBRASI ? ML_PER_PULSE_KALIBRASI : ML_PER_PULSE_DATASHEET;
}
