const int FLOW_SENSOR_PIN = 2;
const int RELAY_PIN = 3;

const bool RELAY_ACTIVE_LOW = true;

const unsigned long PUMP_TIMEOUT_MS = 60000;

const float PULSE_PER_L_DATASHEET = 98.0;
const float ML_PER_PULSE_DATASHEET = 1000.0 / 98.0;

const unsigned long SAMPLING_WINDOW_MS = 1000;
const int REPEAT_COUNT = 10;

volatile unsigned long pulseCount = 0;
volatile unsigned long lastPulseTime = 0;

bool continuousMode = false;
unsigned long lastReportTime = 0;

bool isPumpOn = false;
unsigned long pumpStartTime = 0;

void setup() {
  Serial.begin(9600);

  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);

  if (RELAY_ACTIVE_LOW) {
    digitalWrite(RELAY_PIN, HIGH);
  } else {
    digitalWrite(RELAY_PIN, LOW);
  }

  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, FALLING);

  printHeader();
  printMenu();
}

void loop() {
  if (isPumpOn && (millis() - pumpStartTime > PUMP_TIMEOUT_MS)) {
    pumpOff();
    Serial.println("AUTO_PUMP_OFF (timeout)");
  }

  if (Serial.available() > 0) {
    char cmd = Serial.read();

    switch (cmd) {
      case 'p':
      case 'P':
        if (isPumpOn) {
          pumpOff();
          Serial.println("PUMP_OFF");
        } else {
          turnPumpOn();
          Serial.println("PUMP_ON");
        }
        break;

      case 'n':
      case 'N':
        turnPumpOn();
        Serial.println("PUMP_ON");
        break;

      case 'f':
      case 'F':
        pumpOff();
        Serial.println("PUMP_OFF");
        break;

      case '1':
        singleReading();
        break;

      case 'c':
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

      case 'r':
      case 'R':
        rawPulseInfo();
        break;

      case 't':
      case 'T':
        repeatabilityTest();
        break;

      case 'z':
      case 'Z':
        noInterrupts();
        pulseCount = 0;
        interrupts();
        Serial.println("RESET_COUNTER");
        break;

      case 'h':
      case 'H':
        printMenu();
        break;

      default:
        break;
    }
  }
}

void pulseCounter() {
  pulseCount++;
  lastPulseTime = millis();
}

void turnPumpOn() {
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(RELAY_PIN, LOW);
  } else {
    digitalWrite(RELAY_PIN, HIGH);
  }
  isPumpOn = true;
  pumpStartTime = millis();
}

void pumpOff() {
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(RELAY_PIN, HIGH);
  } else {
    digitalWrite(RELAY_PIN, LOW);
  }
  isPumpOn = false;
}

void singleReading() {
  noInterrupts();
  pulseCount = 0;
  interrupts();

  Serial.println("MEASURING_1_SEC...");

  delay(SAMPLING_WINDOW_MS);

  reportReading();
}

void reportReading() {
  noInterrupts();
  unsigned long pulses = pulseCount;
  pulseCount = 0;
  interrupts();

  float volumeMeasured_ml = pulses * ML_PER_PULSE_DATASHEET;
  float flow_ml_min = volumeMeasured_ml * 60.0;
  float pulse_per_L = 0.0;

  if (volumeMeasured_ml > 0) {
    pulse_per_L = (pulses / volumeMeasured_ml) * 1000.0;
  }

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

void rawPulseInfo() {
  Serial.println("RAW_PULSE_INFO (detak 10 detik)");

  noInterrupts();
  pulseCount = 0;
  interrupts();

  unsigned long startTime = millis();

  Serial.println("Sampling...");

  while (millis() - startTime < 10000) {
    delay(100);
  }

  noInterrupts();
  unsigned long totalPulses = pulseCount;
  interrupts();

  float avgInterval = 10000.0 / totalPulses;

  Serial.println("RESULT_RAW");
  Serial.print("TOTAL_PULSE,");
  Serial.println(totalPulses);
  Serial.print("AVG_INTERVAL_MS,");
  Serial.println(avgInterval, 2);
  Serial.print("EST_FLOW_ML_MIN,");
  Serial.println((totalPulses * ML_PER_PULSE_DATASHEET * 60.0 / 10.0), 1);

  printMenu();
}

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

    delay(500);
  }

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
  Serial.print("Relay: Active ");
  Serial.println(RELAY_ACTIVE_LOW ? "LOW (default)" : "HIGH");
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
  Serial.println("  [f] Pump OFF (force)");
  Serial.print("  Status: PUMP_");
  Serial.println(isPumpOn ? "ON" : "OFF");
  Serial.println("\n=== SENSOR READING ===");
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
