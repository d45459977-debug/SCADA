/*
 * Slave 3 Temperature Control - TK-301
 * SOLOPHOBIA - Heating & Mixing Miniplant
 *
 * RS485 master-slave:
 *   S3:PING
 *   S3:READ
 *   S3:SETSP:<C>
 *   S3:START[:<C>]
 *   S3:STOP
 *   S3:RESET
 *
 * Wiring RS485 disamakan dengan Slave 1:
 *   DE/RE = D2
 *   RO    = D10
 *   DI    = D11
 *
 * Pin sensor dipindah supaya tidak bentrok:
 *   DS18B20 DATA = D4
 */

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const int RS485_DE_RE_PIN = 2;
const int RS485_RO_PIN = 10;
const int RS485_DI_PIN = 11;
const int ONE_WIRE_BUS = 4;

SoftwareSerial SlaveBus(RS485_RO_PIN, RS485_DI_PIN);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const float SUHU_SP_DEFAULT = 35.0;
const unsigned long SENSOR_INTERVAL_MS = 1000;
const unsigned long PROCESS_TIMEOUT_MS = 300000;

float suhuSpC = SUHU_SP_DEFAULT;
float suhuPvC = DEVICE_DISCONNECTED_C;
float errorC = 0.0;
bool processRunning = false;
bool sensorFault = false;
unsigned long processStartMs = 0;
unsigned long lastSensorMs = 0;
String inputLine = "";

void setup() {
  Serial.begin(9600);
  SlaveBus.begin(9600);
  SlaveBus.listen();

  pinMode(RS485_DE_RE_PIN, OUTPUT);
  digitalWrite(RS485_DE_RE_PIN, LOW);

  pinMode(ONE_WIRE_BUS, INPUT_PULLUP);
  sensors.begin();

  Serial.println(F("SLAVE 3 READY - DS18B20 TEMP"));
}

void loop() {
  handleSlaveBus();

  if (processRunning && millis() - lastSensorMs >= SENSOR_INTERVAL_MS) {
    readTemperature();
    if (!sensorFault && suhuPvC != DEVICE_DISCONNECTED_C && suhuPvC >= suhuSpC) {
      processRunning = false;
    }
    lastSensorMs = millis();
  }

  if (processRunning && millis() - processStartMs > PROCESS_TIMEOUT_MS) {
    processRunning = false;
    sensorFault = true;
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
  if (!cmd.startsWith("S3:")) {
    return;
  }

  if (cmd == "S3:PING") {
    sendLine(F("S3:OK"));
  } else if (cmd == "S3:READ") {
    readTemperature();
    sendData();
  } else if (cmd.startsWith("S3:SETSP:")) {
    float sp = cmd.substring(9).toFloat();
    if (sp > -50.0 && sp < 125.0) {
      suhuSpC = sp;
      errorC = suhuPvC - suhuSpC;
      sendText(F("S3:OK,SETSP="));
      sendFloatLine(suhuSpC, 1);
    } else {
      sendLine(F("S3:ERR,SETSP"));
    }
  } else if (cmd == "S3:START" || cmd.startsWith("S3:START:")) {
    if (cmd.startsWith("S3:START:") && cmd.length() > 9) {
      float sp = cmd.substring(9).toFloat();
      if (sp > -50.0 && sp < 125.0) {
        suhuSpC = sp;
      }
    }

    processRunning = true;
    sensorFault = false;
    processStartMs = millis();
    lastSensorMs = 0;
    readTemperature();

    sendText(F("S3:OK,START,SP="));
    sendFloatLine(suhuSpC, 1);
  } else if (cmd == "S3:STOP") {
    processRunning = false;
    sensorFault = false;
    sendLine(F("S3:OK,STOP"));
  } else if (cmd == "S3:RESET") {
    processRunning = false;
    sensorFault = false;
    suhuSpC = SUHU_SP_DEFAULT;
    suhuPvC = DEVICE_DISCONNECTED_C;
    errorC = 0.0;
    sendLine(F("S3:OK,RESET"));
  } else {
    sendLine(F("S3:ERR,UNKNOWN_CMD"));
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
  SlaveBus.print(F("S3:DATA,SP="));
  SlaveBus.print(suhuSpC, 1);
  SlaveBus.print(F(",PV="));
  if (suhuPvC == DEVICE_DISCONNECTED_C) {
    SlaveBus.print(F("-127.0"));
  } else {
    SlaveBus.print(suhuPvC, 1);
  }
  SlaveBus.print(F(",ERR="));
  SlaveBus.print(errorC, 1);
  SlaveBus.print(F(",AUTO="));
  SlaveBus.print(processRunning ? F("ON") : F("OFF"));
  SlaveBus.print(F(",STATUS="));
  SlaveBus.println(getStatusText());
  SlaveBus.flush();
  setTransmitMode(false);
  SlaveBus.listen();
}

bool readTemperature() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  if (tempC == DEVICE_DISCONNECTED_C || tempC < -55.0 || tempC > 125.0) {
    suhuPvC = DEVICE_DISCONNECTED_C;
    errorC = 0.0;
    sensorFault = true;
    return false;
  }

  suhuPvC = tempC;
  errorC = suhuPvC - suhuSpC;
  sensorFault = false;
  return true;
}

String getStatusText() {
  if (sensorFault) {
    return F("FAULT");
  }
  if (processRunning) {
    return F("RUNNING");
  }
  return F("STOP");
}
