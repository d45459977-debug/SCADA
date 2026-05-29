/*
 * ESP32 Master RS485 ONLY - FULL VERSION
 * Tanpa WiFi/Modbus/KEPServer - hanya RS485 communication
 */

#include <Arduino.h>

// Konstanta
const uint32_t SLAVE_BAUD = 9600;
const int SLAVE_RX_PIN = 16;
const int SLAVE_TX_PIN = 17;
const int RS485_DE_RE_PIN = 4;
const unsigned long RESPONSE_TIMEOUT_MS = 3500;
const unsigned long INTER_SLAVE_DELAY_MS = 400;
const unsigned long PROCESS_POLL_INTERVAL_MS = 1000;
const unsigned long PROCESS_TEST_TIMEOUT_MS = 900000;

// Serial2 untuk RS485
HardwareSerial SlaveBus(2);

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(RS485_DE_RE_PIN, OUTPUT);
  digitalWrite(RS485_DE_RE_PIN, LOW);
  SlaveBus.begin(SLAVE_BAUD, SERIAL_8N1, SLAVE_RX_PIN, SLAVE_TX_PIN);

  Serial.println();
  Serial.println("========================================");
  Serial.println(" ESP32 MASTER - RS485 ONLY");
  Serial.println(" SOLOPHOBIA - Heating Mixing Miniplant");
  Serial.println("========================================");
  Serial.println("RX2=GPIO16, TX2=GPIO17, DE/RE=GPIO4");
  Serial.println("Mode: TANPA WiFi/Modbus - RS485 murni");
  Serial.println("========================================");
  printMenu();
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    while (Serial.available()) Serial.read();
    processCommand(cmd);
  }
}

void processCommand(char cmd) {
  switch (cmd) {
    case '1': testKomunikasiSlave(1); break;
    case '2': testKomunikasiSlave(2); break;
    case '3': testKomunikasiSlave(3); break;
    case '4': testKomunikasiSemuaSlave(); break;
    case '5': menuDataSlave1(); break;
    case '6': menuDataSlave2(); break;
    case '7': menuDataSlave3(); break;
    case '8': testKendaliProsesSlave1(); break;
    case '9': runSequentialProcess(); break;
    case 'd': case 'D': testKomunikasiSemuaSlave(); break;
    case 'm': case 'M': case '?': printMenu(); break;
    case '\n': case '\r': break;
    default:
      Serial.println("Perintah tidak dikenali!");
      printMenu();
      break;
  }
}

void printMenu() {
  Serial.println();
  Serial.println("========== MENU MASTER ==========");
  Serial.println("1 - Test Komunikasi Slave 1");
  Serial.println("2 - Test Komunikasi Slave 2");
  Serial.println("3 - Test Komunikasi Slave 3");
  Serial.println("4 - Test Semua Slave");
  Serial.println("5 - Menu Data Slave 1");
  Serial.println("6 - Menu Data Slave 2");
  Serial.println("7 - Menu Data Slave 3");
  Serial.println("8 - Test Kendali Proses Slave 1");
  Serial.println("9 - Proses Sekuensial");
  Serial.println("d - Display Semua Slave");
  Serial.println("m - Menu Ini");
  Serial.println("=================================");
}

void testKomunikasiSlave(int slaveId) {
  Serial.print("\n--- TEST SLAVE ");
  Serial.print(slaveId);
  Serial.println(" ---");

  String response;

  // Slave 3 dengan retry logic
  if (slaveId == 3) {
    delay(500);  // Delay awal untuk Slave 3
    bool success = false;
    for (int retry = 0; retry < 2 && !success; retry++) {
      if (requestSlave(slaveId, "READ", response)) {
        Serial.println(response);
        Serial.println("Status: OK");
        success = true;
      } else if (retry < 1) {
        Serial.println("Retry...");
        delay(500);
      }
    }
    if (!success) {
      Serial.print("Slave ");
      Serial.print(slaveId);
      Serial.println(": TIMEOUT");
    }
  } else {
    // Slave 1 & 2 normal
    if (requestSlave(slaveId, "READ", response)) {
      Serial.println(response);
      Serial.println("Status: OK");
    } else {
      Serial.print("Slave ");
      Serial.print(slaveId);
      Serial.println(": TIMEOUT");
    }
  }
}

void testKomunikasiSemuaSlave() {
  Serial.println("\n=== TEST SEMUA SLAVE ===");
  for (int slaveId = 1; slaveId <= 3; slaveId++) {
    String response;
    Serial.print("Slave ");
    Serial.print(slaveId);
    Serial.print(": ");

    // Slave 3 butuh delay lebih lama dan retry
    if (slaveId == 3) {
      delay(800);  // Delay khusus untuk Slave 3
      bool success = false;
      for (int retry = 0; retry < 2 && !success; retry++) {
        if (requestSlave(slaveId, "READ", response)) {
          Serial.println(response);
          success = true;
        } else if (retry < 1) {
          delay(500);  // Delay sebelum retry
        }
      }
      if (!success) Serial.println("TIMEOUT");
    } else {
      // Slave 1 & 2 normal
      if (requestSlave(slaveId, "READ", response)) {
        Serial.println(response);
      } else {
        Serial.println("TIMEOUT");
      }
      delay(INTER_SLAVE_DELAY_MS);
    }
  }
}

void menuDataSlave1() {
  while (true) {
    Serial.println("\n--- MENU SLAVE 1 (FLOW) ---");
    Serial.println("1 - Baca data");
    Serial.println("2 - Set Volume_SP");
    Serial.println("3 - Reset volume");
    Serial.println("4 - Relay ON");
    Serial.println("5 - Relay OFF");
    Serial.println("6 - Start sesuai SP");
    Serial.println("s - Stop");
    Serial.println("m - Kembali");
    Serial.print("Pilih: ");
    char sub = readConsoleChar();
    String response;
    switch (sub) {
      case '1': printRequestResult(1, "READ"); break;
      case '2': {
        Serial.print("Volume_SP (mL): ");
        String sp = readConsoleLine();
        printRequestResult(1, "SETSP:" + sp);
        break;
      }
      case '3': printRequestResult(1, "RESET"); break;
      case '4': printRequestResult(1, "RELAY:ON"); break;
      case '5': printRequestResult(1, "RELAY:OFF"); break;
      case '6': {
        if (requestSlave(1, "READ", response)) {
          String sp = getField(response, "SP=");
          if (sp.length() == 0) sp = "900";
          printRequestResult(1, "START:" + sp);
        } else Serial.println("Slave 1: TIMEOUT");
        break;
      }
      case 's': case 'S': printRequestResult(1, "STOP"); break;
      case 'm': case 'M': printMenu(); return;
      default: Serial.println("Perintah tidak dikenali!"); break;
    }
  }
}

void menuDataSlave2() {
  while (true) {
    Serial.println("\n--- MENU SLAVE 2 (LEVEL) ---");
    Serial.println("1 - Baca data");
    Serial.println("2 - Set Jarak_SP");
    Serial.println("3 - Start proses");
    Serial.println("4 - Motor UP");
    Serial.println("5 - Motor DOWN");
    Serial.println("6 - Motor STOP");
    Serial.println("7 - Set Speed");
    Serial.println("s - Stop");
    Serial.println("m - Kembali");
    Serial.print("Pilih: ");
    char sub = readConsoleChar();
    switch (sub) {
      case '1': printRequestResult(2, "READ"); break;
      case '2': {
        Serial.print("Jarak_SP (cm): ");
        String sp = readConsoleLine();
        printRequestResult(2, "SETSP:" + sp);
        break;
      }
      case '3': printRequestResult(2, "START"); break;
      case '4': printRequestResult(2, "MOTOR:UP"); break;
      case '5': printRequestResult(2, "MOTOR:DOWN"); break;
      case '6': printRequestResult(2, "MOTOR:STOP"); break;
      case '7': {
        Serial.print("Speed (0-255): ");
        String spd = readConsoleLine();
        printRequestResult(2, "MOTOR:SPEED:" + spd);
        break;
      }
      case 's': case 'S': printRequestResult(2, "STOP"); break;
      case 'm': case 'M': printMenu(); return;
      default: Serial.println("Perintah tidak dikenali!"); break;
    }
  }
}

void menuDataSlave3() {
  while (true) {
    Serial.println("\n--- MENU SLAVE 3 (TK-301 TEMP) ---");
    Serial.println("1 - Baca data sensor/status");
    Serial.println("2 - Set Suhu_SP");
    Serial.println("3 - START proses (60 C, 7 menit)");
    Serial.println("4 - STOP proses");
    Serial.println("5 - RESET semua");
    Serial.println("6 - Heater ON");
    Serial.println("7 - Heater OFF");
    Serial.println("8 - Mixer NORMAL (40%)");
    Serial.println("9 - Mixer CEPAT (60%)");
    Serial.println("a - Mixer STOP");
    Serial.println("m - Kembali ke Menu Utama");
    Serial.print("Pilih: ");
    char sub = readConsoleChar();
    switch (sub) {
      case '1': printRequestResult(3, "READ"); break;
      case '2': {
        Serial.print("Suhu_SP (C): ");
        String sp = readConsoleLine();
        printRequestResult(3, "SETSP:" + sp);
        break;
      }
      case '3': printRequestResult(3, "START"); break;
      case '4': printRequestResult(3, "STOP"); break;
      case '5': printRequestResult(3, "RESET"); break;
      case '6': printRequestResult(3, "HEATER:ON"); break;
      case '7': printRequestResult(3, "HEATER:OFF"); break;
      case '8': printRequestResult(3, "MIXER:NORMAL"); break;
      case '9': printRequestResult(3, "MIXER:CEPAT"); break;
      case 'a': case 'A': printRequestResult(3, "MIXER:STOP"); break;
      case 'm': case 'M': printMenu(); return;
      default: Serial.println("Perintah tidak dikenali!"); break;
    }
  }
}

void testKendaliProsesSlave1() {
  Serial.println("\n=== TEST KENDALI SLAVE 1 ===");
  Serial.println("Mengirim START 900 mL...");

  String response;
  if (!requestSlave(1, "START:900", response)) {
    Serial.println("Slave 1: TIMEOUT saat START");
    return;
  }
  Serial.println(response);
  Serial.print("Polling tiap 1 detik, timeout ");
  Serial.print(PROCESS_TEST_TIMEOUT_MS / 60000);
  Serial.println(" menit.");

  unsigned long startMs = millis();
  while (millis() - startMs < PROCESS_TEST_TIMEOUT_MS) {
    delay(PROCESS_POLL_INTERVAL_MS);
    if (!requestSlave(1, "READ", response)) {
      Serial.println("Slave 1: TIMEOUT saat READ");
      return;
    }
    Serial.println(response);
    String process = getField(response, "PROCESS=");
    String pv = getField(response, "PV=");
    String sp = getField(response, "SP=");
    if (process == "STOP" && pv.toFloat() >= sp.toFloat()) {
      Serial.println("PROCESS_DONE: target tercapai.");
      return;
    }
  }
  printRequestResult(1, "STOP");
  Serial.print("PROCESS_TIMEOUT: target tidak tercapai dalam ");
  Serial.print(PROCESS_TEST_TIMEOUT_MS / 60000);
  Serial.println(" menit.");
}

void runSequentialProcess() {
  Serial.println("\n========================================");
  Serial.println("   PROSES SEKUENSIAL (STEP 1-2)");
  Serial.println("   Menggunakan SP FIXED di Slave");
  Serial.println("========================================");

  // STEP 1: Slave 1
  Serial.println("\n=== STEP 1: ISI AIR (SLAVE 1) ===");
  Serial.println("SP FIXED: 900 mL");
  String response;

  if (!requestSlave(1, "START:FIXED", response)) {
    Serial.println("Slave 1: TIMEOUT saat START!");
    return;
  }
  Serial.println(response);

  bool step1Done = false;
  unsigned long step1Start = millis();
  while (millis() - step1Start < PROCESS_TEST_TIMEOUT_MS) {
    delay(PROCESS_POLL_INTERVAL_MS);
    if (!requestSlave(1, "READ", response)) {
      Serial.println("Slave 1: TIMEOUT saat READ");
      return;
    }
    Serial.println(response);
    String process = getField(response, "PROCESS=");
    String pv = getField(response, "PV=");
    String sp = getField(response, "SP=");
    if (process == "STOP" && pv.toFloat() >= sp.toFloat()) {
      Serial.println("\n*** STEP 1 SELESAI! ***");
      step1Done = true;
      break;
    }
  }

  if (!step1Done) {
    Serial.println("\n*** STEP 1 GAGAL/timeout! ***");
    printRequestResult(1, "STOP");
    return;
  }

  // STEP 2: Slave 2
  Serial.println("\n=== STEP 2: DOSING TEXAPON (SLAVE 2) ===");
  Serial.println("SP FIXED: 100 mL");

  if (!requestSlave(2, "START:FIXED", response)) {
    Serial.println("Slave 2: TIMEOUT saat START!");
    return;
  }
  Serial.println(response);

  bool step2Done = false;
  unsigned long step2Start = millis();
  while (millis() - step2Start < PROCESS_TEST_TIMEOUT_MS) {
    delay(500);
    if (!requestSlave(2, "READ", response)) {
      Serial.println("Slave 2: TIMEOUT saat READ");
      return;
    }
    Serial.println(response);
    String process = getField(response, "PROCESS=");
    String err = getField(response, "ERR=");
    String motor = getField(response, "MOTOR=");
    if ((process == "STOP" || motor == "STOP") && abs(err.toFloat()) <= 0.5) {
      Serial.println("\n*** STEP 2 SELESAI! ***");
      step2Done = true;
      break;
    }
  }

  if (!step2Done) {
    Serial.println("\n*** STEP 2 GAGAL/timeout! ***");
    printRequestResult(2, "STOP");
    return;
  }

  Serial.println("\n========================================");
  Serial.println("   PROSES SEKUENSIAL SELESAI!");
  Serial.println("========================================");
}

void printRequestResult(int slaveId, const String &command) {
  String response;
  if (requestSlave(slaveId, command, response)) {
    Serial.println(response);
  } else {
    Serial.print("Slave ");
    Serial.print(slaveId);
    Serial.println(": TIMEOUT");
  }
}

bool requestSlave(int slaveId, const String &command, String &response) {
  while (SlaveBus.available() > 0) SlaveBus.read();

  String frame = "S" + String(slaveId) + ":" + command;

  digitalWrite(RS485_DE_RE_PIN, HIGH);
  delayMicroseconds(50);

  SlaveBus.print(frame);
  SlaveBus.print('\n');
  SlaveBus.flush();

  digitalWrite(RS485_DE_RE_PIN, LOW);
  delayMicroseconds(50);
  delay(20);

  unsigned long startMs = millis();
  response = "";
  while (millis() - startMs < RESPONSE_TIMEOUT_MS) {
    while (SlaveBus.available() > 0) {
      char c = SlaveBus.read();
      if (c == '\n' || c == '\r') {
        response.trim();
        if (response.length() > 0 && response.startsWith("S" + String(slaveId) + ":")) {
          return true;
        }
        response = "";
      } else if (response.length() < 160) {
        response += c;
      }
    }
  }
  return false;
}

char readConsoleChar() {
  while (!Serial.available()) delay(10);
  char c = Serial.read();
  while (Serial.available()) Serial.read();
  Serial.println(c);
  return c;
}

String readConsoleLine() {
  while (!Serial.available()) delay(10);
  String value = Serial.readStringUntil('\n');
  value.trim();
  while (Serial.available()) Serial.read();
  Serial.println(value);
  return value;
}

String getField(const String &line, const String &key) {
  int start = line.indexOf(key);
  if (start < 0) return "";
  start += key.length();
  int end = line.indexOf(',', start);
  if (end < 0) end = line.length();
  return line.substring(start, end);
}
