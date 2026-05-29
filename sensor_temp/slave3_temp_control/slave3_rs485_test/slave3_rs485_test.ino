/*
 * SLAVE 1/3 - RS485 MINIMAL TEST
 * HANYA test komunikasi RS485, tanpa sensor/actuator apapun
 *
 * Ganti nomor slave di bawah ini:
 * - Untuk Slave 1: ganti SLAVE_ID menjadi 1
 * - Untuk Slave 3: ganti SLAVE_ID menjadi 3
 */

#include <Arduino.h>
#include <SoftwareSerial.h>

const int SLAVE_ID = 1;  // GANTI INI: 1 untuk Slave 1, 3 untuk Slave 3

const int RS485_DE_RE_PIN = 2;
const int RS485_RO_PIN = 10;
const int RS485_DI_PIN = 11;

SoftwareSerial SlaveBus(RS485_RO_PIN, RS485_DI_PIN);

unsigned long lastBlinkMs = 0;
bool ledState = false;

void setup() {
  Serial.begin(9600);
  delay(500);

  // RS485 setup
  pinMode(RS485_DE_RE_PIN, OUTPUT);
  digitalWrite(RS485_DE_RE_PIN, LOW);  // Receive mode

  SlaveBus.begin(9600);

  // LED indicator
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.println("========================================");
  Serial.print(" SLAVE ");
  Serial.print(SLAVE_ID);
  Serial.println(" - RS485 MINIMAL TEST");
  Serial.println("========================================");
  Serial.print("DE/RE: D");
  Serial.println(RS485_DE_RE_PIN);
  Serial.print("RO: D");
  Serial.println(RS485_RO_PIN);
  Serial.print("DI: D");
  Serial.println(RS485_DI_PIN);
  Serial.println("========================================");
  Serial.println("Menunggu command dari Master...");
  Serial.println();
}

void loop() {
  // Blink LED untuk indikator hidup
  if (millis() - lastBlinkMs >= 1000) {
    lastBlinkMs = millis();
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);
  }

  // Handle Serial dari Master (RS485)
  if (SlaveBus.available()) {
    String command = SlaveBus.readStringUntil('\n');
    command.trim();

    Serial.print("[RS485 RX] ");
    Serial.println(command);

    // Process command
    String prefix = "S" + String(SLAVE_ID) + ":";

    if (command == prefix + "PING") {
      sendResponse(prefix + "OK");
      Serial.println("[RS485 TX] PING OK");
    }
    else if (command == prefix + "READ") {
      // Kirim data dummy
      sendResponse(prefix + "DATA,SP=100.0,PV=50.0,STATUS=OK");
      Serial.println("[RS485 TX] READ OK");
    }
    else if (command.startsWith("S")) {
      // Command untuk slave lain, abaikan
      Serial.println("[RS485 RX] Command untuk slave lain, abaikan");
    }
    else {
      Serial.println("[RS485 RX] Unknown command");
    }
  }

  // Handle Serial lokal (USB)
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "test") {
      Serial.println("Mengirim test response...");
      sendResponse(prefix + "OK,TEST");
    }
    else if (cmd == "ping") {
      Serial.println("Simulasi PING response...");
      sendResponse(prefix + "OK");
    }
    else if (cmd == "?") {
      Serial.println();
      Serial.println("========== MENU ==========");
      Serial.println("test - Kirim test response");
      Serial.println("ping - Kirim PING response");
      Serial.println("?   - Menu ini");
      Serial.println("=========================");
      Serial.println();
    }
  }
}

void sendResponse(String response) {
  digitalWrite(RS485_DE_RE_PIN, HIGH);  // Transmit mode
  delayMicroseconds(100);  // Longer delay for safety

  SlaveBus.print(response);
  SlaveBus.print('\n');
  SlaveBus.flush();

  delay(10);  // Wait for transmission to complete

  digitalWrite(RS485_DE_RE_PIN, LOW);  // Receive mode
  delayMicroseconds(100);

  SlaveBus.listen();  // Reset SoftwareSerial
}
