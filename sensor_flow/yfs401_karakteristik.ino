/*
 * YFS401 Karakteristik Flow Sensor
 * Slave 1 - TK-101 Flow Control
 *
 * Untuk menentukan karakteristik sensor YFS401:
 * - Akurasi vs volume referensi (volumetric method)
 * - Presisi (repeatability)
 * - Linearity (pulse per L)
 * - Noise/bounce detection
 *
 * Output: Serial CSV - VOLUME_ML,PULSE_COUNT,FLOW_ML_MIN,PULSE_PER_L,TIME_MS
 *
 * CATATAN: TIDAK ADA KALIBRASI yang di-apply
 *          Ini hanya KARAKTERISTIK untuk dapat data RAW
 */

// Pin Configuration
const int FLOW_SENSOR_PIN = 2;  // D2 = INT0 on Arduino Uno

// Konstanta dari datasheet (untuk perbandingan saja)
const float PULSE_PER_L_DATASHEET = 98.0;  // F = 98 x Q (Q = L/min)
const float ML_PER_PULSE_DATASHEET = 1000.0 / 98.0;  // ~10.2 mL/pulse

// Konstanta untuk sampling
const unsigned long SAMPLING_WINDOW_MS = 1000;  // 1 detik sampling window
const int REPEAT_COUNT = 10;  // Jumlah pengulangan untuk repeatability test

// Variabel global untuk interrupt
volatile unsigned long pulseCount = 0;
volatile unsigned long lastPulseTime = 0;

// Variabel untuk mode continuous
bool continuousMode = false;
unsigned long lastReportTime = 0;

void setup() {
  Serial.begin(9600);

  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);

  // Attach interrupt untuk counting pulse
  // FALLING = detect saat pulsa dari HIGH ke LOW
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, FALLING);

  printHeader();
  printMenu();
}

void loop() {
  // Cek command dari Serial
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    // Reset pulse count sebelum setiap measurement
    switch (cmd) {
      case '1':  // Single reading (1 detik)
        singleReading();
        break;

      case 'c':  // Continuous mode
      case 'C':
        continuousMode = true;
        Serial.println("MODE_CONTINUOUS (tekan 's' untuk stop)");
        Serial.println("VOLUME_ML,PULSE_COUNT,FLOW_ML_MIN,PULSE_PER_L,TIME_MS");
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

      case 'r':  // Raw pulse info (untuk cek bounce)
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
        interrupts();
        Serial.println("RESET_COUNTER");
        break;

      case 'h':  // Help
      case 'H':
        printMenu();
        break;

      default:
        // Abaikan karakter lain
        break;
    }
  }
}

// Interrupt Service Routine - dipanggil setiap kali pulse detected
void pulseCounter() {
  pulseCount++;
  lastPulseTime = millis();
}

// Single reading - ukur selama 1 detik
void singleReading() {
  // Reset counter
  noInterrupts();
  pulseCount = 0;
  interrupts();

  Serial.println("MEASURING_1_SEC...");

  // Tunggu 1 detik
  delay(SAMPLING_WINDOW_MS);

  // Ambil hasil
  reportReading();
}

// Report hasil pembacaan dalam format CSV
void reportReading() {
  noInterrupts();
  unsigned long pulses = pulseCount;
  pulseCount = 0;  // Reset untuk next reading
  interrupts();

  // Hitung berdasarkan datasheet (hanya untuk comparison)
  float volumeMeasured_ml = pulses * ML_PER_PULSE_DATASHEET;
  float flow_ml_min = volumeMeasured_ml * 60.0;  // mL/detik → mL/menit
  float pulse_per_L = 0.0;

  if (volumeMeasured_ml > 0) {
    pulse_per_L = (pulses / volumeMeasured_ml) * 1000.0;
  }

  // Output CSV
  Serial.print(volumeMeasured_ml, 1);
  Serial.print(",");
  Serial.print(pulses);
  Serial.print(",");
  Serial.print(flow_ml_min, 1);
  Serial.print(",");
  Serial.print(pulse_per_L, 1);
  Serial.print(",");
  Serial.println(millis());
}

// Raw pulse info - untuk deteksi bounce
void rawPulseInfo() {
  Serial.println("RAW_PULSE_INFO (detak 10 detik)");

  noInterrupts();
  pulseCount = 0;
  interrupts();

  unsigned long startTime = millis();
  unsigned long lastPulse = lastPulseTime;
  unsigned long minInterval = 0xFFFFFFFF;
  unsigned long maxInterval = 0;

  Serial.println("Sampling...");

  while (millis() - startTime < 10000) {
    delay(100);
  }

  noInterrupts();
  unsigned long totalPulses = pulseCount;
  interrupts();

  float avgInterval = 10000.0 / totalPulses;  // ms

  Serial.println("RESULT_RAW");
  Serial.print("TOTAL_PULSE,");
  Serial.println(totalPulses);
  Serial.print("AVG_INTERVAL_MS,");
  Serial.println(avgInterval, 2);
  Serial.print("EST_FLOW_ML_MIN,");
  Serial.println((totalPulses * ML_PER_PULSE_DATASHEET * 60.0 / 10.0), 1);

  printMenu();
}

// Repeatability test - 10 pembacaan berturut-turut
void repeatabilityTest() {
  Serial.println("REPEATABILITY_TEST (10 x 1 detik)");
  Serial.println("NO,VOLUME_ML,PULSE_COUNT,FLOW_ML_MIN,PULSE_PER_L");

  unsigned long pulsesArray[REPEAT_COUNT];
  float volumeArray[REPEAT_COUNT];
  float flowArray[REPEAT_COUNT];
  float pulsePerLArray[REPEAT_COUNT];

  for (int i = 0; i < REPEAT_COUNT; i++) {
    noInterrupts();
    pulseCount = 0;
    interrupts();

    delay(SAMPLING_WINDOW_MS);

    noInterrupts();
    unsigned long pulses = pulseCount;
    pulseCount = 0;
    interrupts();

    float volume_ml = pulses * ML_PER_PULSE_DATASHEET;
    float flow_ml_min = volume_ml * 60.0;
    float pulse_per_L = (volume_ml > 0) ? (pulses / volume_ml) * 1000.0 : 0.0;

    pulsesArray[i] = pulses;
    volumeArray[i] = volume_ml;
    flowArray[i] = flow_ml_min;
    pulsePerLArray[i] = pulse_per_L;

    Serial.print(i + 1);
    Serial.print(",");
    Serial.print(volume_ml, 2);
    Serial.print(",");
    Serial.print(pulses);
    Serial.print(",");
    Serial.print(flow_ml_min, 1);
    Serial.print(",");
    Serial.println(pulse_per_L, 2);

    delay(500);  // Jeda antar reading
  }

  // Hitung statistik
  float sumVol = 0, sumFlow = 0, sumPulseL = 0;
  float minVol = volumeArray[0], maxVol = volumeArray[0];

  for (int i = 0; i < REPEAT_COUNT; i++) {
    sumVol += volumeArray[i];
    sumFlow += flowArray[i];
    sumPulseL += pulsePerLArray[i];
    if (volumeArray[i] < minVol) minVol = volumeArray[i];
    if (volumeArray[i] > maxVol) maxVol = volumeArray[i];
  }

  float meanVol = sumVol / REPEAT_COUNT;
  float meanFlow = sumFlow / REPEAT_COUNT;
  float meanPulseL = sumPulseL / REPEAT_COUNT;
  float rangeVol = maxVol - minVol;

  // Hitung standard deviation
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
  Serial.print("MEAN_PULSE_PER_L,");
  Serial.println(meanPulseL, 2);
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
  Serial.println("YFS401 KARAKTERISTIK FLOW SENSOR");
  Serial.println("Slave 1 - TK-101 Flow Control");
  Serial.println("=====================================");
}

void printMenu() {
  Serial.println("\nMENU:");
  Serial.println("  [1] Single reading (1 detik)");
  Serial.println("  [c] Continuous mode");
  Serial.println("  [r] Raw pulse info (deteksi bounce)");
  Serial.println("  [t] Repeatability test (10x)");
  Serial.println("  [z] Reset pulse counter");
  Serial.println("  [h] Help");
  Serial.println("\nCATATAN: Nilai berdasarkan datasheet (98 pulse/L)");
  Serial.println("         Ini untuk KARAKTERISTIK, bukan kalibrasi");
  Serial.println("-------------------------------------");
}
