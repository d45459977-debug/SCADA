/*
 * YFS401 Terkalibrasi - Flow Sensor dengan Faktor Kalibrasi Aktual
 * Slave 1 - TK-101 Flow Control
 *
 * Hasil Kalibrasi Volumetrik:
 * - Volume Aktual: 200 mL
 * - Waktu: 37.63 detik
 * - Total Pulse: ~37.630 pulse
 * - Faktor Kalibrasi: 0.0053 mL/pulse
 * - Pulse per L: ~188.000 (datasheet 98.000)
 *
 * Output: Serial CSV - VOLUME_ML,PULSE_COUNT,FLOW_ML_MIN,TIME_MS
 *
 * CATATAN: Faktor kalibrasi ini spesifik untuk setup:
 * - Pompa 5V yang sama
 * - Flow rate yang sama (~319 mL/menit)
 * - Kalau pompa/flowrate berubah, PERLU KALIBRASI ULANG
 */

// Pin Configuration
const int FLOW_SENSOR_PIN = 2;  // D2 = INT0 on Arduino Uno
const int RELAY_PIN = 3;        // D3 = Relay control (Pompa 5V)

// Relay configuration
const bool RELAY_ACTIVE_LOW = true;  // Ubah ke false jika relay active HIGH

// Pump auto-off safety (ms)
const unsigned long PUMP_TIMEOUT_MS = 60000;  // Auto-off setelah 60 detik

// ============================================
// FAKTOR KALIBRASI - UPDATE DISINI
// ============================================
// Dari datasheet (untuk perbandingan)
const float ML_PER_PULSE_DATASHEET = 1000.0 / 98.0;  // 10.2 mL/pulse
const float PULSE_PER_L_DATASHEET = 98.0;

// Dari kalibrasi volumetrik (200 mL dalam 37.63 detik)
const float ML_PER_PULSE_KALIBRASI = 0.0053;  // 0.0053 mL/pulse
const float PULSE_PER_L_KALIBRASI = 188150.0;  // ~188.150 pulse/L

// Pilih faktor yang dipakai: true = kalibrasi, false = datasheet
const bool PAKAI_FAKTOR_KALIBRASI = true;

// ============================================

// Konstanta untuk sampling
const unsigned long SAMPLING_WINDOW_MS = 1000;  // 1 detik sampling window
const int REPEAT_COUNT = 10;  // Jumlah pengulangan untuk repeatability test

// Variabel global untuk interrupt
volatile unsigned long pulseCount = 0;
volatile unsigned long lastPulseTime = 0;
volatile unsigned long totalPulseAccumulated = 0;  // Akumulasi sejak reset

// Variabel untuk mode continuous
bool continuousMode = false;
unsigned long lastReportTime = 0;

// Variabel untuk pompa
bool isPumpOn = false;
unsigned long pumpStartTime = 0;

void setup() {
  Serial.begin(9600);

  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);

  // Initial relay state OFF
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(RELAY_PIN, HIGH);  // HIGH = OFF untuk active LOW
  } else {
    digitalWrite(RELAY_PIN, LOW);   // LOW = OFF untuk active HIGH
  }

  // Attach interrupt untuk counting pulse
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, FALLING);

  printHeader();
  printMenu();
}

void loop() {
  // Cek auto-off pompa (safety)
  if (isPumpOn && (millis() - pumpStartTime > PUMP_TIMEOUT_MS)) {
    pumpOff();
    Serial.println("AUTO_PUMP_OFF (timeout)");
  }

  // Cek command dari Serial
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    switch (cmd) {
      case 'p':  // Toggle pump ON/OFF
      case 'P':
        if (isPumpOn) {
          pumpOff();
          Serial.println("PUMP_OFF");
        } else {
          turnPumpOn();
          Serial.println("PUMP_ON");
        }
        break;

      case 'n':  // Pump ON (explicit)
      case 'N':
        turnPumpOn();
        Serial.println("PUMP_ON");
        break;

      case 'f':  // Pump OFF (explicit)
      case 'F':
        pumpOff();
        Serial.println("PUMP_OFF");
        break;

      case '1':  // Single reading (1 detik)
        singleReading();
        break;

      case 'c':  // Continuous mode
      case 'C':
        continuousMode = true;
        Serial.println("MODE_CONTINUOUS (tekan 's' untuk stop)");
        Serial.println("VOLUME_ML,PULSE_COUNT,FLOW_ML_MIN,TIME_MS");
        lastReportTime = millis();
        while (Serial.read() != 's') {
          if (millis() - lastReportTime >= SAMPLING_WINDOW_MS) {
            reportReading();
            lastReportTime = millis();
          }
        }
        continuousMode = false;
        Serial.println("\nMODE_STOP");
        printMenu();
        break;

      case 'r':  // Raw pulse info
      case 'R':
        rawPulseInfo();
        break;

      case 't':  // Repeatability test
      case 'T':
        repeatabilityTest();
        break;

      case 'z':  // Reset counter
      case 'Z':
        noInterrupts();
        pulseCount = 0;
        totalPulseAccumulated = 0;
        interrupts();
        Serial.println("RESET_COUNTER");
        break;

      case 'a':  // Akumulasi volume (sampai 's' ditekan)
      case 'A':
        accumulationMode();
        break;

      case 'h':  // Help
      case 'H':
        printMenu();
        break;

      default:
        break;
    }
  }
}

// Interrupt Service Routine
void pulseCounter() {
  pulseCount++;
  totalPulseAccumulated++;
  lastPulseTime = millis();
}

// Pump control functions
void turnPumpOn() {
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(RELAY_PIN, LOW);   // LOW = ON untuk active LOW
  } else {
    digitalWrite(RELAY_PIN, HIGH);  // HIGH = ON untuk active HIGH
  }
  isPumpOn = true;
  pumpStartTime = millis();
}

void pumpOff() {
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(RELAY_PIN, HIGH);  // HIGH = OFF untuk active LOW
  } else {
    digitalWrite(RELAY_PIN, LOW);   // LOW = OFF untuk active HIGH
  }
  isPumpOn = false;
}

// Single reading - ukur selama 1 detik
void singleReading() {
  // Reset counter sementara
  noInterrupts();
  unsigned long startPulse = pulseCount;
  interrupts();

  Serial.println("MEASURING_1_SEC...");
  delay(SAMPLING_WINDOW_MS);

  noInterrupts();
  unsigned long pulses = pulseCount - startPulse;
  pulseCount = 0;  // Reset untuk next reading
  interrupts();

  reportReadingData(pulses);
}

// Report hasil pembacaan
void reportReading() {
  noInterrupts();
  unsigned long pulses = pulseCount;
  pulseCount = 0;  // Reset untuk next reading
  interrupts();

  reportReadingData(pulses);
}

void reportReadingData(unsigned long pulses) {
  // Pilih faktor kalibrasi
  float ml_per_pulse = PAKAI_FAKTOR_KALIBRASI ? ML_PER_PULSE_KALIBRASI : ML_PER_PULSE_DATASHEET;

  // Hitung volume
  float volumeMeasured_ml = pulses * ml_per_pulse;
  float flow_ml_min = volumeMeasured_ml * 60.0;  // mL/detik → mL/menit

  // Output CSV
  Serial.print(volumeMeasured_ml, 2);
  Serial.print(",");
  Serial.print(pulses);
  Serial.print(",");
  Serial.print(flow_ml_min, 1);
  Serial.print(",");
  Serial.println(millis());
}

// Akumulasi volume - untuk kalibrasi
void accumulationMode() {
  Serial.println("MODE_AKUMULASI (tekan 's' untuk stop)");
  Serial.println("PUMP_ON dulu baru mulai akumulasi...");
  Serial.println("VOLUME_ML,TOTAL_PULSE,TIME_SEC,FLOW_ML_MIN");

  noInterrupts();
  unsigned long startPulse = totalPulseAccumulated;
  unsigned long startTime = millis();
  interrupts();

  while (Serial.read() != 's') {
    delay(100);

    noInterrupts();
    unsigned long totalPulses = totalPulseAccumulated - startPulse;
    unsigned long elapsedMs = millis() - startTime;
    interrupts();

    float ml_per_pulse = PAKAI_FAKTOR_KALIBRASI ? ML_PER_PULSE_KALIBRASI : ML_PER_PULSE_DATASHEET;
    float volume_ml = totalPulses * ml_per_pulse;
    float time_sec = elapsedMs / 1000.0;
    float flow_ml_min = (time_sec > 0) ? (volume_ml / time_sec) * 60.0 : 0.0;

    Serial.print(volume_ml, 1);
    Serial.print(",");
    Serial.print(totalPulses);
    Serial.print(",");
    Serial.print(time_sec, 1);
    Serial.print(",");
    Serial.println(flow_ml_min, 1);
  }

  Serial.println("\nMODE_STOP");
  printMenu();
}

// Raw pulse info
void rawPulseInfo() {
  Serial.println("RAW_PULSE_INFO (detak 10 detik)");

  noInterrupts();
  pulseCount = 0;
  interrupts();

  Serial.println("Sampling...");
  delay(10000);

  noInterrupts();
  unsigned long totalPulses = pulseCount;
  interrupts();

  float ml_per_pulse = PAKAI_FAKTOR_KALIBRASI ? ML_PER_PULSE_KALIBRASI : ML_PER_PULSE_DATASHEET;

  float avgInterval = 10000.0 / totalPulses;  // ms
  float volume_ml = totalPulses * ml_per_pulse;
  float flow_ml_min = volume_ml * 6.0;  // /10 detik * 60

  Serial.println("RESULT_RAW");
  Serial.print("TOTAL_PULSE,");
  Serial.println(totalPulses);
  Serial.print("AVG_INTERVAL_MS,");
  Serial.println(avgInterval, 2);
  Serial.print("VOLUME_ML,");
  Serial.println(volume_ml, 1);
  Serial.print("EST_FLOW_ML_MIN,");
  Serial.println(flow_ml_min, 1);
  Serial.print("FAKTOR_YG_DIPAKAI,");
  Serial.println(PAKAI_FAKTOR_KALIBRASI ? "KALIBRASI" : "DATASHEET");

  printMenu();
}

// Repeatability test
void repeatabilityTest() {
  Serial.println("REPEATABILITY_TEST (10 x 1 detik)");
  Serial.println("NO,VOLUME_ML,PULSE_COUNT,FLOW_ML_MIN");

  float ml_per_pulse = PAKAI_FAKTOR_KALIBRASI ? ML_PER_PULSE_KALIBRASI : ML_PER_PULSE_DATASHEET;

  unsigned long pulsesArray[REPEAT_COUNT];
  float volumeArray[REPEAT_COUNT];
  float flowArray[REPEAT_COUNT];

  for (int i = 0; i < REPEAT_COUNT; i++) {
    noInterrupts();
    pulseCount = 0;
    interrupts();

    delay(SAMPLING_WINDOW_MS);

    noInterrupts();
    unsigned long pulses = pulseCount;
    pulseCount = 0;
    interrupts();

    float volume_ml = pulses * ml_per_pulse;
    float flow_ml_min = volume_ml * 60.0;

    pulsesArray[i] = pulses;
    volumeArray[i] = volume_ml;
    flowArray[i] = flow_ml_min;

    Serial.print(i + 1);
    Serial.print(",");
    Serial.print(volume_ml, 2);
    Serial.print(",");
    Serial.print(pulses);
    Serial.print(",");
    Serial.println(flow_ml_min, 1);

    delay(500);
  }

  // Statistik
  float sumVol = 0, sumFlow = 0;
  float minVol = volumeArray[0], maxVol = volumeArray[0];

  for (int i = 0; i < REPEAT_COUNT; i++) {
    sumVol += volumeArray[i];
    sumFlow += flowArray[i];
    if (volumeArray[i] < minVol) minVol = volumeArray[i];
    if (volumeArray[i] > maxVol) maxVol = volumeArray[i];
  }

  float meanVol = sumVol / REPEAT_COUNT;
  float meanFlow = sumFlow / REPEAT_COUNT;
  float rangeVol = maxVol - minVol;

  float sumSqDiff = 0;
  for (int i = 0; i < REPEAT_COUNT; i++) {
    float diff = volumeArray[i] - meanVol;
    sumSqDiff += diff * diff;
  }
  float stdDev = sqrt(sumSqDiff / REPEAT_COUNT);

  Serial.println("\nSTATISTICS");
  Serial.print("MEAN_VOLUME_ML,");
  Serial.println(meanVol, 2);
  Serial.print("MEAN_FLOW_ML_MIN,");
  Serial.println(meanFlow, 1);
  Serial.print("MIN_VOLUME_ML,");
  Serial.println(minVol, 2);
  Serial.print("MAX_VOLUME_ML,");
  Serial.println(maxVol, 2);
  Serial.print("RANGE_VOLUME_ML,");
  Serial.println(rangeVol, 2);
  Serial.print("STD_DEV_ML,");
  Serial.println(stdDev, 3);
  Serial.print("CV_PERCENT,");
  Serial.println((stdDev / meanVol) * 100.0, 2);

  printMenu();
}

void printHeader() {
  Serial.println("=====================================");
  Serial.println("YFS401 TERKALIBRASI");
  Serial.println("Slave 1 - TK-101 Flow Control");
  Serial.println("=====================================");
  Serial.print("Faktor: ");
  if (PAKAI_FAKTOR_KALIBRASI) {
    Serial.print(ML_PER_PULSE_KALIBRASI, 4);
    Serial.println(" mL/pulse (KALIBRASI)");
  } else {
    Serial.print(ML_PER_PULSE_DATASHEET, 2);
    Serial.println(" mL/pulse (DATASHEET)");
  }
  Serial.print("Relay: Active ");
  Serial.println(RELAY_ACTIVE_LOW ? "LOW" : "HIGH");
  Serial.print("Pump timeout: ");
  Serial.print(PUMP_TIMEOUT_MS / 1000);
  Serial.println(" seconds");
  Serial.println("=====================================");
}

void printMenu() {
  Serial.println("\nMENU:");
  Serial.println("=== PUMP CONTROL ===");
  Serial.println("  [p] Toggle pump ON/OFF");
  Serial.println("  [n] Pump ON");
  Serial.println("  [f] Pump OFF");
  Serial.print("  Status: PUMP_");
  Serial.println(isPumpOn ? "ON" : "OFF");

  Serial.println("\n=== SENSOR READING ===");
  Serial.println("  [1] Single reading (1 detik)");
  Serial.println("  [c] Continuous mode");
  Serial.println("  [a] Akumulasi volume (untuk kalibrasi)");
  Serial.println("  [r] Raw pulse info");
  Serial.println("  [t] Repeatability test (10x)");
  Serial.println("  [z] Reset pulse counter");

  Serial.println("\n=== KALIBRASI ===");
  Serial.print("  Faktor: ");
  Serial.println(PAKAI_FAKTOR_KALIBRASI ? "KALIBRASI" : "DATASHEET");
  Serial.println("  Ubah PAKAI_FAKTOR_KALIBRASI di kode");

  Serial.println("\n  [h] Help");
  Serial.println("-------------------------------------");
}
