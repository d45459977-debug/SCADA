/*
 * ESP32 Master Komunikasi Data - SOLOPHOBIA
 *
 * Master membaca data slave nyata lewat Serial2 dan menampilkan TIMEOUT
 * jika slave tidak terhubung/tidak merespons. Untuk tahap test awal,
 * Slave 1 sudah memakai protokol ASCII request-response.
 *
 * Wiring default Serial2 ESP32:
 *   ESP32 TX2 GPIO17 -> RX slave / DI RS485
 *   ESP32 RX2 GPIO16 <- TX slave / RO RS485
 *   GND master dan slave harus common ground.
 *
 * Jika memakai RS-485 half duplex dengan pin DE/RE, set RS485_DE_RE_PIN
 * ke pin yang dipakai. Default -1 berarti tidak dipakai.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

// WiFi selection variables
const char* ssid;
const char* password;
char selectedSsid[32];
char enteredPassword[64];

// Preferences untuk nyimpan WiFi
Preferences wifiCreds;

const uint32_t SLAVE_BAUD = 9600;
const int SLAVE_RX_PIN = 16;
const int SLAVE_TX_PIN = 17;
const int RS485_DE_RE_PIN = 4;
const unsigned long RESPONSE_TIMEOUT_MS = 3500;
const unsigned long INTER_SLAVE_DELAY_MS = 400;
const unsigned long PROCESS_POLL_INTERVAL_MS = 1000;
const unsigned long PROCESS_TEST_TIMEOUT_MS = 900000;  // 15 menit (sinkron dengan Slave 1)

HardwareSerial SlaveBus(2);
WiFiServer mbServer(502);
WiFiClient mbClient;

#define NUM_REGISTERS 16
uint16_t holdingRegisters[NUM_REGISTERS];
#define REG_VOLUME_SP      1
#define REG_VOLUME_PV      2
#define REG_STATUS_POMPA   3
#define REG_JARAK_SP       4
#define REG_JARAK_PV       5
#define REG_STATUS_MOTOR   6
#define REG_SUHU_SP        7
#define REG_SUHU_PV        8
#define REG_STATUS_HEATER  9
#define REG_MIXING_DUR    10
#define REG_MIXING_TIMER  11
#define REG_STATUS_MIXER  12

void setup() {
  Serial.begin(115200);
  delay(500);

  if (RS485_DE_RE_PIN >= 0) {
    pinMode(RS485_DE_RE_PIN, OUTPUT);
    digitalWrite(RS485_DE_RE_PIN, LOW);
  }

  SlaveBus.begin(SLAVE_BAUD, SERIAL_8N1, SLAVE_RX_PIN, SLAVE_TX_PIN);

  Serial.println();
  Serial.println("========================================");
  Serial.println(" ESP32 MASTER KOMUNIKASI DATA");
  Serial.println(" SOLOPHOBIA - Heating Mixing Miniplant");
  Serial.println("========================================");
  Serial.println("Bus: Serial2 9600 baud");
  Serial.println("RX2=GPIO16, TX2=GPIO17");

  pilihWiFi();
  connectWiFiAndStartTcp();
  initRegisters();
  printMenu();
}

void loop() {
  handleModbusClient();

  if (Serial.available()) {
    char cmd = Serial.read();
    while (Serial.available()) Serial.read();
    processCommand(cmd);
  }
}


void pilihWiFi() {
  // Buka Preferences
  wifiCreds.begin("wifi-creds", false);

  // Cek apakah ada WiFi tersimpan
  String savedSSID = wifiCreds.getString("ssid", "");
  String savedPass = wifiCreds.getString("pass", "");

  wifiCreds.end();

  if (savedSSID.length() > 0 && savedPass.length() > 0) {
    // Ada WiFi tersimpan
    Serial.println("\n=== WIFI TERSIMPAN DITEMUKAN ===");
    Serial.println("SSID: " + savedSSID);
    Serial.println("1 - Connect ke WiFi tersimpan");
    Serial.println("2 - Scan WiFi baru");
    Serial.print("Pilih (1-2): ");

    while (!Serial.available()) {
      delay(10);
    }
    int pilihan = Serial.parseInt();
    while (Serial.available()) Serial.read();

    Serial.println(String(pilihan));

    if (pilihan == 1) {
      // Pakai WiFi tersimpan
      savedSSID.toCharArray(selectedSsid, sizeof(selectedSsid));
      savedPass.toCharArray(enteredPassword, sizeof(enteredPassword));
      ssid = selectedSsid;
      password = enteredPassword;

      Serial.println("Menghubungkan ke: " + String(ssid));
      return;
    }
  }

  // Scan WiFi baru
  Serial.println("\n=== SCAN WIFI OTOMATIS ===");
  Serial.println("Scanning...");

  // Set WiFi ke station mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Scan WiFi
  int n = WiFi.scanNetworks();
  Serial.println("Scan selesai!");

  if (n == 0) {
    Serial.println("Tidak ada WiFi ditemukan!");
    Serial.println("Gunakan mode manual...");
    inputManualWifi();
    return;
  }

  Serial.println(String(n) + " WiFi ditemukan:");
  for (int i = 0; i < n; i++) {
    Serial.print(String(i + 1) + " - ");
    Serial.print(WiFi.SSID(i));
    Serial.print(" (");
    Serial.print(WiFi.RSSI(i));
    Serial.println(" dBm)");
    delay(10);
  }

  Serial.print("\nPilih WiFi (1-" + String(n) + "): ");

  // Tunggu input user
  while (!Serial.available()) {
    delay(10);
  }
  int pilihan = Serial.parseInt();
  while (Serial.available()) Serial.read();

  if (pilihan < 1 || pilihan > n) {
    Serial.println("Pilihan tidak valid!");
    inputManualWifi();
    return;
  }

  Serial.println(String(pilihan));

  // Simpan SSID yang dipilih
  String ssidStr = WiFi.SSID(pilihan - 1);
  ssidStr.toCharArray(selectedSsid, sizeof(selectedSsid));
  ssid = selectedSsid;

  Serial.print("SSID: ");
  Serial.println(ssid);

  // Input password
  Serial.print("Password: ");
  readSerialLine(enteredPassword, sizeof(enteredPassword));
  password = enteredPassword;

  Serial.println("WiFi terpilih: " + String(ssid));
}

void inputManualWifi() {
  Serial.println("\n=== INPUT WIFI MANUAL ===");
  Serial.print("SSID: ");
  readSerialLine(selectedSsid, sizeof(selectedSsid));
  ssid = selectedSsid;

  Serial.print("Password: ");
  readSerialLine(enteredPassword, sizeof(enteredPassword));
  password = enteredPassword;

  Serial.println("WiFi terpilih: " + String(ssid));
}

void connectWiFiAndStartTcp() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  unsigned long startMs = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startMs < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    // Simpan WiFi credentials ke Preferences
    wifiCreds.begin("wifi-creds", false);
    wifiCreds.putString("ssid", String(ssid));
    wifiCreds.putString("pass", String(password));
    wifiCreds.end();
    Serial.println("WiFi tersimpan untuk next boot!");

    mbServer.begin();
    Serial.println("Modbus TCP Server on port 502");
  } else {
    Serial.println("\nWiFi TIMEOUT: Modbus TCP belum aktif.");

    // Hapus credentials tersimpan jika gagal connect
    wifiCreds.begin("wifi-creds", false);
    wifiCreds.clear();
    wifiCreds.end();
    Serial.println("WiFi tersimpan dihapus (gagal connect).");
  }
}

void initRegisters() {
  for (int i = 0; i < NUM_REGISTERS; i++) holdingRegisters[i] = 0;
  holdingRegisters[REG_VOLUME_SP] = 900;
}

void handleModbusClient() {
  if (WiFi.status() != WL_CONNECTED) return;

  if (!mbClient || !mbClient.connected()) {
    mbClient = mbServer.available();
    return;
  }

  if (mbClient.available() < 8) return;

  byte txIdH = mbClient.read();
  byte txIdL = mbClient.read();
  byte protoH = mbClient.read();
  byte protoL = mbClient.read();
  byte lenH = mbClient.read();
  byte lenL = mbClient.read();
  byte unitId = mbClient.read();
  uint16_t length = (lenH << 8) | lenL;
  byte funcCode = mbClient.read();

  byte data[32];
  int dataLen = min((int)sizeof(data), (int)length - 2);
  for (int i = 0; i < dataLen; i++) data[i] = mbClient.read();
  while (mbClient.available() && dataLen < (int)length - 2) mbClient.read();

  if (unitId != 0 && unitId != 1) return;

  if (funcCode == 0x03 && dataLen >= 4) {
    pollSlave1ToRegisters();
    uint16_t addr = (data[0] << 8) | data[1];
    uint16_t count = (data[2] << 8) | data[3];
    if (count > 60) count = 60;

    byte resp[128];
    resp[0] = 0x03;
    resp[1] = count * 2;
    for (int i = 0; i < count; i++) {
      uint16_t val = (addr + i < NUM_REGISTERS) ? holdingRegisters[addr + i] : 0;
      resp[2 + i * 2] = highByte(val);
      resp[3 + i * 2] = lowByte(val);
    }

    byte mbap[7] = {txIdH, txIdL, protoH, protoL, highByte(3 + count * 2), lowByte(3 + count * 2), unitId};
    mbClient.write(mbap, 7);
    mbClient.write(resp, 2 + count * 2);
    mbClient.flush();
  } else if (funcCode == 0x06 && dataLen >= 4) {
    uint16_t addr = (data[0] << 8) | data[1];
    uint16_t val = (data[2] << 8) | data[3];
    if (addr < NUM_REGISTERS) holdingRegisters[addr] = val;
    if (addr == REG_VOLUME_SP) {
      String response;
      requestSlave(1, "SETSP:" + String(val), response);
    }

    byte resp[12] = {txIdH, txIdL, protoH, protoL, 0, 6, unitId, 0x06, highByte(addr), lowByte(addr), highByte(val), lowByte(val)};
    mbClient.write(resp, 12);
    mbClient.flush();
  }
}

void pollSlave1ToRegisters() {
  String response;
  if (!requestSlave(1, "READ", response)) return;
  updateSlave1RegistersFromResponse(response);
}

void updateSlave1RegistersFromResponse(const String &response) {
  String sp = getField(response, "SP=");
  String pv = getField(response, "PV=");
  String relay = getField(response, "RELAY=");
  if (sp.length()) holdingRegisters[REG_VOLUME_SP] = (uint16_t)sp.toFloat();
  if (pv.length()) holdingRegisters[REG_VOLUME_PV] = (uint16_t)pv.toFloat();
  if (relay.length()) holdingRegisters[REG_STATUS_POMPA] = (relay == "ON") ? 1 : 0;
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
    case 'r': case 'R': resetWiFiCreds(); break;
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
  Serial.println("1 - Test Komunikasi Data Slave 1");
  Serial.println("2 - Test Komunikasi Data Slave 2");
  Serial.println("3 - Test Komunikasi Data Slave 3");
  Serial.println("4 - Test Komunikasi Data Semua Slave");
  Serial.println("5 - Menu Data Slave 1");
  Serial.println("6 - Menu Data Slave 2");
  Serial.println("7 - Menu Data Slave 3");
  Serial.println("8 - Test Kendali Proses Slave 1");
  Serial.println("9 - Proses Sekuensial (S1->S2)");
  Serial.println("r - Reset WiFi tersimpan");
  Serial.println("d - Display/Test semua data slave");
  Serial.println("m - Menu Ini");
  Serial.println("=================================");
}

void testKomunikasiSlave(int slaveId) {
  Serial.print("\n--- TEST KOMUNIKASI DATA SLAVE ");
  Serial.print(slaveId);
  Serial.println(" ---");

  String response;
  if (requestSlave(slaveId, "READ", response)) {
    if (slaveId == 1) updateSlave1RegistersFromResponse(response);
    Serial.println(response);
    Serial.println("Status: OK");
  } else {
    Serial.print("Slave ");
    Serial.print(slaveId);
    Serial.println(": TIMEOUT");
  }
  // Menu tidak auto-print setelah command. Tekan m untuk tampilkan menu.
}

void testKomunikasiSemuaSlave() {
  Serial.println("\n=== TEST KOMUNIKASI DATA SEMUA SLAVE ===");
  for (int slaveId = 1; slaveId <= 3; slaveId++) {
    String response;
    Serial.print("Slave ");
    Serial.print(slaveId);
    Serial.print(": ");
    if (requestSlave(slaveId, "READ", response)) {
      if (slaveId == 1) updateSlave1RegistersFromResponse(response);
      Serial.println(response);
    } else {
      Serial.println("TIMEOUT");
    }
    delay(INTER_SLAVE_DELAY_MS);
  }
  // Menu tidak auto-print setelah command. Tekan m untuk tampilkan menu.
}

void menuDataSlave1() {
  while (true) {
    Serial.println("\n--- MENU DATA SLAVE 1 (TK-101 Flow) ---");
    Serial.println("1 - Baca data sensor/status");
    Serial.println("2 - Set Volume_SP");
    Serial.println("3 - Reset volume");
    Serial.println("4 - Relay output ON");
    Serial.println("5 - Relay output OFF");
    Serial.println("6 - Start proses sesuai Volume_SP");
    Serial.println("7 - Test Kendali Proses 900 mL");
    Serial.println("s - Stop proses/relay output");
    Serial.println("m - Kembali ke Menu Utama");
    Serial.print("Pilih: ");

    char sub = readConsoleChar();
    String response;
    switch (sub) {
      case '1':
        printRequestResult(1, "READ");
        break;
      case '2': {
        Serial.print("Volume_SP (mL): ");
        String sp = readConsoleLine();
        printRequestResult(1, "SETSP:" + sp);
        break;
      }
      case '3':
        printRequestResult(1, "RESET");
        break;
      case '4':
        printRequestResult(1, "RELAY:ON");
        break;
      case '5':
        printRequestResult(1, "RELAY:OFF");
        break;
      case '6': {
        if (requestSlave(1, "READ", response)) {
          String sp = getField(response, "SP=");
          if (sp.length() == 0) sp = "900";
          printRequestResult(1, "START:" + sp);
        } else {
          Serial.println("Slave 1: TIMEOUT");
        }
        break;
      }
      case '7':
        testKendaliProsesSlave1();
        break;
      case 's': case 'S':
        printRequestResult(1, "STOP");
        break;
      case 'm': case 'M':
        printMenu();
        return;
      default:
        Serial.println("Perintah tidak dikenali!");
        break;
    }
  }
}

void menuDataSlave2() {
  while (true) {
    Serial.println("\n--- MENU DATA SLAVE 2 (TK-201 Level) ---");
    Serial.println("1 - Baca data sensor/status");
    Serial.println("2 - Set Jarak_SP");
    Serial.println("3 - Start/Test proses");
    Serial.println("4 - Motor UP (Naik)");
    Serial.println("5 - Motor DOWN (Turun)");
    Serial.println("6 - Motor STOP");
    Serial.println("7 - Set Motor Speed");
    Serial.println("s - Stop proses");
    Serial.println("m - Kembali ke Menu Utama");
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
        Serial.print("Motor Speed (0-255): ");
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
    Serial.println("\n--- MENU DATA SLAVE 3 (TK-301 Temp) ---");
    Serial.println("1 - Baca data sensor/status");
    Serial.println("2 - Set Suhu_SP");
    Serial.println("3 - Toggle heater/start proses");
    Serial.println("s - Stop proses");
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
      case 's': case 'S': printRequestResult(3, "STOP"); break;
      case 'm': case 'M': printMenu(); return;
      default: Serial.println("Perintah tidak dikenali!"); break;
    }
  }
}

void testKendaliProsesSlave1() {
  Serial.println("\n=== TEST KENDALI PROSES SLAVE 1 ===");
  Serial.println("Master kirim START 900 mL ke Slave 1.");

  String response;
  if (!requestSlave(1, "START:900", response)) {
    Serial.println("Slave 1: TIMEOUT saat START");
    return;
  }
  Serial.println(response);
  Serial.print("Polling data tiap 1 detik. Stop otomatis saat PROCESS=STOP atau timeout ");
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
      Serial.println("PROCESS_DONE: target volume tercapai.");
      return;
    }
  }

  printRequestResult(1, "STOP");
  Serial.print("PROCESS_TIMEOUT: target belum tercapai dalam ");
  Serial.print(PROCESS_TEST_TIMEOUT_MS / 60000);
  Serial.println(" menit.");
  // Menu tidak auto-print setelah command. Tekan m untuk tampilkan menu.
}

void runSequentialProcess() {
  Serial.println("\n========================================");
  Serial.println("   PROSES SEKUENSIAL (STEP 1-2)");
  Serial.println("   Menggunakan SP FIXED di Slave");
  Serial.println("========================================");

  // STEP 1: Slave 1 - Isi Air (SP FIXED: 900 mL)
  Serial.println("\n=== STEP 1: ISI AIR (SLAVE 1) ===");
  Serial.println("Menggunakan SP FIXED: 900 mL");
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
      Serial.println("\n*** STEP 1 SELESAI! Air terisi. ***");
      step1Done = true;
      break;
    }
  }

  // Validasi Step 1 benar-benar selesai
  if (!step1Done) {
    Serial.println("\n*** STEP 1 GAGAL/timeout! Proses dibatalkan. ***");
    printRequestResult(1, "STOP");
    return;
  }

  // STEP 2: Slave 2 - Dosing Texapon (SP FIXED: 100 mL)
  Serial.println("\n=== STEP 2: DOSING TEXAPON (SLAVE 2) ===");
  Serial.println("Menggunakan SP FIXED: 100 mL");

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

    // Cek apakah sudah selesai (motor STOP dan error kecil)
    if (process == "STOP" || motor == "STOP") {
      float errorVal = err.toFloat();
      if (abs(errorVal) <= 0.5) {
        Serial.println("\n*** STEP 2 SELESAI! Texapon terdosing. ***");
        step2Done = true;
        break;
      }
    }
  }

  if (!step2Done) {
    Serial.println("\n*** STEP 2 GAGAL/timeout! ***");
    printRequestResult(2, "STOP");
    return;
  }

  Serial.println("\n========================================");
  Serial.println("   PROSES SEKUENSIAL SELESAI (STEP 1-2)");
  Serial.println("========================================");
  Serial.println("Siap untuk step berikutnya (Slave 3: Heating + Mixing)");
  // Menu tidak auto-print setelah command. Tekan m untuk tampilkan menu.
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
  clearSlaveInput();

  String frame = "S" + String(slaveId) + ":" + command;
  setTransmitMode(true);
  SlaveBus.print(frame);
  SlaveBus.print('\n');
  SlaveBus.flush();
  setTransmitMode(false);
  delay(20);

  unsigned long startMs = millis();
  response = "";
  while (millis() - startMs < RESPONSE_TIMEOUT_MS) {
    while (SlaveBus.available() > 0) {
      char c = SlaveBus.read();
      if (c == '\n' || c == '\r') {
        response.trim();
        if (response.length() == 0) {
          continue;
        }
        if (response.startsWith("S" + String(slaveId) + ":")) {
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

void clearSlaveInput() {
  while (SlaveBus.available() > 0) {
    SlaveBus.read();
  }
}

void setTransmitMode(bool tx) {
  if (RS485_DE_RE_PIN >= 0) {
    digitalWrite(RS485_DE_RE_PIN, tx ? HIGH : LOW);
    delayMicroseconds(50);
  }
}

char readConsoleChar() {
  while (!Serial.available()) {
    delay(10);
  }
  char c = Serial.read();
  while (Serial.available()) Serial.read();
  Serial.println(c);
  return c;
}

String readConsoleLine() {
  while (!Serial.available()) {
    delay(10);
  }
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

void readSerialLine(char* buffer, size_t maxSize) {
  size_t idx = 0;
  buffer[0] = '\0';

  unsigned long timeout = millis() + 30000; // 30 detik timeout
  while (millis() < timeout) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        buffer[idx] = '\0';
        Serial.println(buffer);
        return;
      } else if (idx < maxSize - 1) {
        buffer[idx++] = c;
      }
    }
    delay(10);
  }
  buffer[idx] = '\0';
  Serial.println(buffer);
}

void resetWiFiCreds() {
  Serial.println("\n=== RESET WIFI TERSIMPAN ===");

  wifiCreds.begin("wifi-creds", false);
  String savedSSID = wifiCreds.getString("ssid", "");

  if (savedSSID.length() > 0) {
    Serial.println("WiFi tersimpan: " + savedSSID);
    Serial.print("Yakin hapus? (y/n): ");

    while (!Serial.available()) {
      delay(10);
    }
    char confirm = Serial.read();
    while (Serial.available()) Serial.read();
    Serial.println(confirm);

    if (confirm == 'y' || confirm == 'Y') {
      wifiCreds.clear();
      Serial.println("WiFi tersimpan telah DIHAPUS.");
      Serial.println("Restart untuk scan WiFi baru.");
    } else {
      Serial.println("Batal menghapus.");
    }
  } else {
    Serial.println("Tidak ada WiFi tersimpan.");
  }

  wifiCreds.end();
  Serial.println();
  printMenu();
}
