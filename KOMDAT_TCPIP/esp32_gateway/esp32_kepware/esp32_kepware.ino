/*
 * ESP32 Gateway untuk KEPServerEX - SOLOPHOBIA
 *
 * Modbus TCP Server + RS485 Master untuk 3 Slave
 * Integrasi dengan KEPServerEX untuk HMI/SCADA
 *
 * Wiring Serial2 ESP32:
 *   ESP32 TX2 GPIO17 -> RX slave / DI RS485
 *   ESP32 RX2 GPIO16 <- TX slave / RO RS485
 *   ESP32 GPIO4 -> DE/RE RS485
 */

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

// WiFi credentials (akan diinput via Serial)
char ssid[32] = "";
char password[64] = "";

// Preferences untuk menyimpan WiFi
Preferences wifiCreds;

// RS485 Serial
const uint32_t SLAVE_BAUD = 9600;
const int SLAVE_RX_PIN = 16;
const int SLAVE_TX_PIN = 17;
const int RS485_DE_RE_PIN = 4;

// Timeout
const unsigned long RESPONSE_TIMEOUT_MS = 3500;
const unsigned long SLAVE_POLL_INTERVAL_MS = 500;
const unsigned long WIFI_RECONNECT_INTERVAL_MS = 30000;

// Modbus TCP Server
WiFiServer mbServer(502);
WiFiClient mbClient;

// Register Map - Holding Registers (40001-based)
// 40001-40006: Slave 1 Data
// 40007-40014: Slave 2 Data
// 40015-40021: Slave 3 Data
// 40022: System Status
// 40100-40122: Command Registers

#define NUM_HOLDING_REGISTERS 150
uint16_t holdingRegisters[NUM_HOLDING_REGISTERS];

// Slave Data Variables
bool slave1Connected = false;
bool slave2Connected = false;
bool slave3Connected = false;

// Slave 1 Data
float s1_volumeSp = 900.0;
float s1_volumePv = 0.0;
float s1_flowRate = 0.0;
uint32_t s1_pulseCount = 0;
bool s1_relayOn = false;
bool s1_processActive = false;

// Slave 2 Data
float s2_distanceSp = 15.0;
float s2_distancePv = 0.0;
float s2_volumeSp = 100.0;
float s2_volumePv = 0.0;
int s2_motorDir = 0;  // 0=STOP, 1=UP, 2=DOWN
bool s2_autoOn = false;
int s2_status = 0;  // 0=STOP, 1=RUN, 2=FAULT

// Slave 3 Data
float s3_tempSp = 60.0;
float s3_tempPv = 0.0;
bool s3_heaterOn = false;
bool s3_mixerOn = false;
int s3_mixerSpeed = 0;  // 0=STOP, 1=NORMAL, 2=CEPAT
uint32_t s3_timer = 0;
bool s3_processActive = false;

HardwareSerial SlaveBus(2);

unsigned long lastSlavePollMs = 0;
unsigned long lastWifiReconnectMs = 0;

void setup() {
  Serial.begin(115200);
  delay(500);

  // Init RS485
  pinMode(RS485_DE_RE_PIN, OUTPUT);
  digitalWrite(RS485_DE_RE_PIN, LOW);
  SlaveBus.begin(SLAVE_BAUD, SERIAL_8N1, SLAVE_RX_PIN, SLAVE_TX_PIN);

  // Init registers
  initRegisters();

  // Setup WiFi atau load dari preferences
  setupWiFi();

  // Start Modbus TCP Server
  mbServer.begin();

  Serial.println();
  Serial.println("========================================");
  Serial.println(" ESP32 GATEWAY - KEPServer Integration");
  Serial.println(" SOLOPHOBIA - Heating Mixing Miniplant");
  Serial.println("========================================");
  printMenu();
}

void loop() {
  // Poll slaves setiap 500ms
  if (millis() - lastSlavePollMs >= SLAVE_POLL_INTERVAL_MS) {
    lastSlavePollMs = millis();
    pollSlaves();
    updateRegisters();
  }

  // Handle Modbus client
  handleModbusClient();

  // WiFi reconnect check
  if (WiFi.status() != WL_CONNECTED && millis() - lastWifiReconnectMs >= WIFI_RECONNECT_INTERVAL_MS) {
    lastWifiReconnectMs = millis();
    reconnectWiFi();
  }

  // Handle Serial commands
  if (Serial.available()) {
    char cmd = Serial.read();
    while (Serial.available()) Serial.read();
    processCommand(cmd);
  }
}

void initRegisters() {
  for (int i = 0; i < NUM_HOLDING_REGISTERS; i++) {
    holdingRegisters[i] = 0;
  }

  // Slave 1 default values
  holdingRegisters[0] = 900;   // 40001: S1_VOLUME_SP
  holdingRegisters[5] = 1;     // 40006: S1_PULSE_COUNT (0-65535)
  holdingRegisters[20] = 1;    // 40022: SYSTEM_STATUS (1=Online)

  // Slave 2 default values
  holdingRegisters[6] = 1500;  // 40007: S2_DISTANCE_SP (15.00 cm * 100)
  holdingRegisters[10] = 1000; // 40011: S2_VOLUME_SP (100 mL)

  // Slave 3 default values
  holdingRegisters[14] = 600;  // 40015: S3_TEMP_SP (60.0 C * 10)
}

void pollSlaves() {
  String response;

  // Poll Slave 1
  if (requestSlave(1, "READ", response)) {
    slave1Connected = true;
    parseSlave1Response(response);
  } else {
    slave1Connected = false;
  }

  delay(100);

  // Poll Slave 2
  if (requestSlave(2, "READ", response)) {
    slave2Connected = true;
    parseSlave2Response(response);
  } else {
    slave2Connected = false;
  }

  delay(100);

  // Poll Slave 3
  if (requestSlave(3, "READ", response)) {
    slave3Connected = true;
    parseSlave3Response(response);
  } else {
    slave3Connected = false;
  }
}

void parseSlave1Response(const String &response) {
  s1_volumeSp = getField(response, "SP=").toFloat();
  s1_volumePv = getField(response, "PV=").toFloat();
  s1_flowRate = getField(response, "FLOW=").toFloat();
  s1_pulseCount = getField(response, "PULSE=").toInt();
  s1_relayOn = (getField(response, "RELAY=") == "ON");
  s1_processActive = (getField(response, "PROCESS=") == "RUN");
}

void parseSlave2Response(const String &response) {
  s2_distanceSp = getField(response, "SP=").toFloat();
  s2_distancePv = getField(response, "PV=").toFloat();
  s2_volumeSp = getField(response, "VOLSP=").toFloat();
  s2_volumePv = getField(response, "VOLPV=").toFloat();
  String motor = getField(response, "MOTOR=");
  s2_motorDir = (motor == "UP") ? 1 : (motor == "DOWN") ? 2 : 0;
  s2_autoOn = (getField(response, "AUTO=") == "ON");
  String status = getField(response, "STATUS=");
  s2_status = (status == "RUN") ? 1 : (status == "FAULT") ? 2 : 0;
}

void parseSlave3Response(const String &response) {
  s3_tempSp = getField(response, "SP=").toFloat();
  s3_tempPv = getField(response, "PV=").toFloat();
  s3_heaterOn = (getField(response, "HEATER=") == "ON");
  s3_mixerOn = (getField(response, "MIXER=") == "ON");
  String speed = getField(response, "SPEED=");
  s3_mixerSpeed = (speed == "NORMAL") ? 1 : (speed == "CEPAT") ? 2 : 0;
  s3_timer = getField(response, "TIMER=").toInt();
  s3_processActive = (getField(response, "PROCESS=") == "RUN");
}

void updateRegisters() {
  // Slave 1 Registers (40001-40006)
  holdingRegisters[0] = (uint16_t)s1_volumeSp;          // 40001: S1_VOLUME_SP
  holdingRegisters[1] = (uint16_t)s1_volumePv;          // 40002: S1_VOLUME_PV
  holdingRegisters[2] = s1_relayOn ? 1 : 0;             // 40003: S1_RELAY_STATUS
  holdingRegisters[3] = (uint16_t)(s1_flowRate * 10);   // 40004: S1_FLOW_RATE (×0.1)
  holdingRegisters[4] = (uint16_t)(s1_pulseCount & 0xFFFF); // 40005: S1_PULSE_COUNT
  holdingRegisters[5] = s1_processActive ? 1 : 0;      // 40006: S1_PROCESS_STATUS

  // Slave 2 Registers (40007-40014)
  holdingRegisters[6] = (uint16_t)(s2_distanceSp * 100); // 40007: S2_DISTANCE_SP (×0.01)
  holdingRegisters[7] = (uint16_t)(s2_distancePv * 100); // 40008: S2_DISTANCE_PV
  holdingRegisters[8] = (uint16_t)((s2_distancePv - s2_distanceSp) * 100); // 40009: S2_DISTANCE_ERR
  holdingRegisters[9] = (uint16_t)s2_volumeSp;          // 40010: S2_VOLUME_SP
  holdingRegisters[10] = (uint16_t)s2_volumePv;         // 40011: S2_VOLUME_PV
  holdingRegisters[11] = s2_motorDir;                  // 40012: S2_MOTOR_DIR
  holdingRegisters[12] = s2_autoOn ? 1 : 0;             // 40013: S2_AUTO_STATUS
  holdingRegisters[13] = s2_status;                     // 40014: S2_SYS_STATUS

  // Slave 3 Registers (40015-40021)
  holdingRegisters[14] = (uint16_t)(s3_tempSp * 10);    // 40015: S3_TEMP_SP (×0.1)
  holdingRegisters[15] = (uint16_t)(s3_tempPv * 10);    // 40016: S3_TEMP_PV (×0.1)
  holdingRegisters[16] = s3_heaterOn ? 1 : 0;           // 40017: S3_HEATER_STATUS
  holdingRegisters[17] = s3_mixerOn ? 1 : 0;            // 40018: S3_MIXER_STATUS
  holdingRegisters[18] = s3_mixerSpeed;                 // 40019: S3_MIXER_SPEED
  holdingRegisters[19] = s3_timer;                     // 40020: S3_PROCESS_TIMER
  holdingRegisters[20] = s3_processActive ? 1 : 0;       // 40021: S3_PROCESS_STATUS

  // System Status (40022)
  holdingRegisters[21] = 1; // System Online
  holdingRegisters[22] = slave1Connected ? 1 : 0; // S1 Connected
  holdingRegisters[23] = slave2Connected ? 1 : 0; // S2 Connected
  holdingRegisters[24] = slave3Connected ? 1 : 0; // S3 Connected
}

void handleModbusClient() {
  if (WiFi.status() != WL_CONNECTED) return;

  if (!mbClient || !mbClient.connected()) {
    mbClient = mbServer.available();
    if (mbClient && mbClient.connected()) {
      Serial.println("Modbus Client connected!");
    }
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

  // Read Holding Registers (FC=03)
  if (funcCode == 0x03 && dataLen >= 4) {
    uint16_t addr = (data[0] << 8) | data[1];
    uint16_t count = (data[2] << 8) | data[3];
    if (count > 60) count = 60;
    if (addr >= NUM_HOLDING_REGISTERS) addr = 0;

    byte resp[128];
    resp[0] = 0x03;
    resp[1] = count * 2;
    for (int i = 0; i < count; i++) {
      uint16_t val = (addr + i < NUM_HOLDING_REGISTERS) ? holdingRegisters[addr + i] : 0;
      resp[2 + i * 2] = highByte(val);
      resp[3 + i * 2] = lowByte(val);
    }

    byte mbap[7] = {txIdH, txIdL, protoH, protoL, highByte(3 + count * 2), lowByte(3 + count * 2), unitId};
    mbClient.write(mbap, 7);
    mbClient.write(resp, 2 + count * 2);
    mbClient.flush();
  }
  // Write Single Register (FC=06)
  else if (funcCode == 0x06 && dataLen >= 4) {
    uint16_t addr = (data[0] << 8) | data[1];
    uint16_t val = (data[2] << 8) | data[3];

    if (addr < NUM_HOLDING_REGISTERS) {
      holdingRegisters[addr] = val;
      processWriteCommand(addr, val);
    }

    byte resp[12] = {txIdH, txIdL, protoH, protoL, 0, 6, unitId, 0x06, highByte(addr), lowByte(addr), highByte(val), lowByte(val)};
    mbClient.write(resp, 12);
    mbClient.flush();
  }
}

void processWriteCommand(uint16_t addr, uint16_t val) {
  // Command registers start at 40100 (index 99)
  if (addr < 99) return;

  String cmd = "";
  int slaveId = 0;

  // Slave 1 Commands (40100-40106)
  if (addr >= 99 && addr <= 105) {
    slaveId = 1;
    switch(addr) {
      case 99: cmd = "SETSP:" + String(val); break;              // 40100: S1_SET_VOLUME_SP
      case 100: cmd = val ? "START:" + String(holdingRegisters[0]) : "STOP"; break; // 40101: S1_START_STOP
      case 101: cmd = val ? "RELAY:ON" : "RELAY:OFF"; break;    // 40102: S1_RELAY_CTRL
      case 102: cmd = "RESET"; break;                            // 40103: S1_RESET
      case 103: // 40104: S1_START_FIXED
        if (val) cmd = "START:FIXED";
        break;
    }
  }
  // Slave 2 Commands (40107-40114)
  else if (addr >= 106 && addr <= 113) {
    slaveId = 2;
    switch(addr) {
      case 106: cmd = "SETSP:" + String(val / 100.0, 2); break; // 40107: S2_SET_DISTANCE_SP
      case 107: cmd = val ? "START" : "STOP"; break;             // 40108: S2_START_STOP
      case 108: // 40109: S2_MOTOR_DIR
        if (val == 1) cmd = "MOTOR:UP";
        else if (val == 2) cmd = "MOTOR:DOWN";
        else if (val == 0) cmd = "MOTOR:STOP";
        break;
      case 109: cmd = "MOTOR:SPEED:" + String(val); break;       // 40110: S2_MOTOR_SPEED
      case 110: // 40111: S2_START_FIXED
        if (val) cmd = "START:FIXED";
        break;
    }
  }
  // Slave 3 Commands (40115-40122)
  else if (addr >= 114 && addr <= 121) {
    slaveId = 3;
    switch(addr) {
      case 114: cmd = "SETSP:" + String(val / 10.0, 1); break;  // 40115: S3_SET_TEMP_SP
      case 115: cmd = val ? "START" : "STOP"; break;             // 40116: S3_START_STOP
      case 116: cmd = val ? "HEATER:ON" : "HEATER:OFF"; break;  // 40117: S3_HEATER_CTRL
      case 117: // 40118: S3_MIXER_SPEED
        if (val == 1) cmd = "MIXER:NORMAL";
        else if (val == 2) cmd = "MIXER:CEPAT";
        else if (val == 0) cmd = "MIXER:STOP";
        break;
    }
  }

  // Kirim command ke slave
  if (slaveId > 0 && cmd.length() > 0) {
    String response;
    requestSlave(slaveId, cmd, response);
    Serial.println("CMD -> S" + String(slaveId) + ":" + cmd + " => " + response);
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

void setupWiFi() {
  wifiCreds.begin("wifi-creds", false);
  String savedSSID = wifiCreds.getString("ssid", "");
  String savedPass = wifiCreds.getString("pass", "");
  wifiCreds.end();

  if (savedSSID.length() > 0 && savedPass.length() > 0) {
    savedSSID.toCharArray(ssid, sizeof(ssid));
    savedPass.toCharArray(password, sizeof(password));
    connectWiFi();
  } else {
    Serial.println("\n=== SETUP WIFI ===");
    Serial.print("SSID: ");
    readSerialLine(ssid, sizeof(ssid));
    Serial.print("Password: ");
    readSerialLine(password, sizeof(password));
    connectWiFi();
  }
}

void connectWiFi() {
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

    wifiCreds.begin("wifi-creds", false);
    wifiCreds.putString("ssid", String(ssid));
    wifiCreds.putString("pass", String(password));
    wifiCreds.end();
  } else {
    Serial.println("\nWiFi TIMEOUT!");
  }
}

void reconnectWiFi() {
  Serial.println("Attempting WiFi reconnect...");
  WiFi.disconnect();
  delay(500);
  WiFi.begin(ssid, password);
  delay(5000);
}

void printMenu() {
  Serial.println();
  Serial.println("========== MENU GATEWAY ==========");
  Serial.println("1 - Test Slave 1");
  Serial.println("2 - Test Slave 2");
  Serial.println("3 - Test Slave 3");
  Serial.println("4 - Test Semua Slave");
  Serial.println("s - Status System");
  Serial.println("i - Info WiFi");
  Serial.println("r - Reset WiFi");
  Serial.println("m - Menu Ini");
  Serial.println("==================================");
}

void processCommand(char cmd) {
  String response;
  switch (cmd) {
    case '1':
      if (requestSlave(1, "READ", response)) Serial.println(response);
      else Serial.println("Slave 1: TIMEOUT");
      break;
    case '2':
      if (requestSlave(2, "READ", response)) Serial.println(response);
      else Serial.println("Slave 2: TIMEOUT");
      break;
    case '3':
      if (requestSlave(3, "READ", response)) Serial.println(response);
      else Serial.println("Slave 3: TIMEOUT");
      break;
    case '4':
      Serial.println("Slave 1: " + String(slave1Connected ? "OK" : "TIMEOUT"));
      Serial.println("Slave 2: " + String(slave2Connected ? "OK" : "TIMEOUT"));
      Serial.println("Slave 3: " + String(slave3Connected ? "OK" : "TIMEOUT"));
      break;
    case 's': case 'S':
      printSystemStatus();
      break;
    case 'i': case 'I':
      if (WiFi.status() == WL_CONNECTED) {
        Serial.print("WiFi: Connected (");
        Serial.print(WiFi.localIP());
        Serial.println(")");
      } else {
        Serial.println("WiFi: Not connected");
      }
      Serial.print("Modbus TCP: Port 502 ");
      Serial.println(mbServer ? "Active" : "Inactive");
      break;
    case 'r': case 'R':
      wifiCreds.begin("wifi-creds", false);
      wifiCreds.clear();
      wifiCreds.end();
      Serial.println("WiFi credentials cleared. Restart to reconfigure.");
      break;
    case 'm': case 'M': case '?':
      printMenu();
      break;
  }
}

void printSystemStatus() {
  Serial.println("\n=== SYSTEM STATUS ===");
  Serial.print("WiFi: ");
  Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
  Serial.print("Modbus TCP: ");
  Serial.println(mbServer ? "Active" : "Inactive");
  Serial.print("Slave 1: ");
  Serial.println(slave1Connected ? "OK" : "TIMEOUT");
  Serial.print("Slave 2: ");
  Serial.println(slave2Connected ? "OK" : "TIMEOUT");
  Serial.print("Slave 3: ");
  Serial.println(slave3Connected ? "OK" : "TIMEOUT");
  Serial.println("======================\n");
}

void readSerialLine(char* buffer, size_t maxSize) {
  size_t idx = 0;
  buffer[0] = '\0';
  unsigned long timeout = millis() + 30000;
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

String getField(const String &line, const String &key) {
  int start = line.indexOf(key);
  if (start < 0) return "";
  start += key.length();
  int end = line.indexOf(',', start);
  if (end < 0) end = line.length();
  return line.substring(start, end);
}
