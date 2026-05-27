/*
 * HC-SR04 Karakterisasi Sensor
 * Slave 2 - TK-201 Level Control
 *
 * Untuk menentukan karakteristik sensor HC-SR04:
 * - Akurasi vs jarak referensi
 * - Presisi (repeatability)
 * - Linearity
 * - Noise
 *
 * Output: Serial CSV - jarak_cm, jarak_us, raw_pulse
 */

// Pin Configuration
const int TRIG_PIN = 9;
const int ECHO_PIN = 10;

// Konstanta
const float KECEPATAN_SUARA = 0.0343;  // cm/us (suara 343 m/s)
const int SAMPEL_MOVING_AVG = 5;       // Jumlah sampel untuk filter
const int DELAY_BACA = 100;            // ms antar pembacaan (continuous mode)

// Variabel global
float bufferJarak[SAMPEL_MOVING_AVG];
int indexBuffer = 0;
bool bufferFull = false;

void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Inisialisasi buffer
  for (int i = 0; i < SAMPEL_MOVING_AVG; i++) {
    bufferJarak[i] = 0.0;
  }

  printHeader();
  printMenu();
}

void loop() {
  // Cek command dari Serial
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    switch (cmd) {
      case '1':  // Single reading
        bacaDanTampilkan();
        break;

      case 'c':  // Continuous mode
      case 'C':
        Serial.println("Mode CONTINUOUS (tekan 's' untuk stop)");
        delay(500);
        while (Serial.read() != 's') {
          bacaDanTampilkan();
          delay(DELAY_BACA);
        }
        Serial.println("\nMode STOP");
        printMenu();
        break;

      case 'r':  // Raw data (tanpa filter)
      case 'R':
        Serial.println("Mode RAW (tanpa moving average)");
        bacaRaw();
        break;

      case 't':  // Test - 10 pembacaan berturut-turut
      case 'T':
        testRepeatability();
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

// Baca jarak dengan HC-SR04
float bacaJarak() {
  // Trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Baca pulse duration (timeout 30ms = max 5m)
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return -1.0;  // Timeout / out of range
  }

  // Konversi ke cm
  float jarak = (duration * KECEPATAN_SUARA) / 2.0;
  return jarak;
}

// Baca dengan moving average filter
float bacaJarakFiltered() {
  float jarak = bacaJarak();

  if (jarak < 0) {
    return jarak;  // Lewatkan error reading
  }

  // Update buffer
  bufferJarak[indexBuffer] = jarak;
  indexBuffer = (indexBuffer + 1) % SAMPEL_MOVING_AVG;

  if (indexBuffer == 0) {
    bufferFull = true;
  }

  // Hitung average
  float sum = 0.0;
  int count = bufferFull ? SAMPEL_MOVING_AVG : indexBuffer;

  for (int i = 0; i < count; i++) {
    sum += bufferJarak[i];
  }

  return sum / count;
}

// Baca raw pulse width (untuk analisis sinyal)
unsigned long bacaRawPulse() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  return pulseIn(ECHO_PIN, HIGH, 30000);
}

// Baca dan tampilkan satu reading
void bacaDanTampilkan() {
  float jarak = bacaJarakFiltered();

  if (jarak < 0) {
    Serial.println("ERROR,OUT_OF_RANGE");
  } else {
    unsigned long rawPulse = bacaRawPulse();
    Serial.print("DATA,");
    Serial.print(jarak, 2);      // 2 desimal
    Serial.print(",");
    Serial.print(rawPulse);       // Raw pulse dalam us
    Serial.print(",");
    Serial.println((unsigned long)(jarak * 100));  // Jarak dalam 0.01mm unit
  }
}

// Baca raw tanpa filter
void bacaRaw() {
  for (int i = 0; i < 10; i++) {
    unsigned long rawPulse = bacaRawPulse();
    float jarak = (rawPulse * KECEPATAN_SUARA) / 2.0;

    Serial.print("RAW,");
    Serial.print(i + 1);
    Serial.print(",");
    Serial.print(jarak, 3);
    Serial.print(",");
    Serial.println(rawPulse);

    delay(50);
  }
}

// Test repeatability - 10 pembacaan cepat
void testRepeatability() {
  Serial.println("TEST_REPEATABILITAS (10 sampel)");

  float readings[10];
  unsigned long rawReadings[10];

  // Ambil 10 sampel
  for (int i = 0; i < 10; i++) {
    rawReadings[i] = bacaRawPulse();
    readings[i] = (rawReadings[i] * KECEPATAN_SUARA) / 2.0;
    delay(50);
  }

  // Hitung statistik
  float sum = 0;
  float minVal = readings[0];
  float maxVal = readings[0];

  for (int i = 0; i < 10; i++) {
    sum += readings[i];
    if (readings[i] < minVal) minVal = readings[i];
    if (readings[i] > maxVal) maxVal = readings[i];
  }

  float mean = sum / 10;
  float range = maxVal - minVal;

  // Tampilkan hasil
  Serial.println("SAMPLES,VALUE_CM,RAW_US");
  for (int i = 0; i < 10; i++) {
    Serial.print("S,");
    Serial.print(i + 1);
    Serial.print(",");
    Serial.print(readings[i], 3);
    Serial.print(",");
    Serial.println(rawReadings[i]);
  }

  Serial.println("STATISTICS");
  Serial.print("MEAN,");
  Serial.println(mean, 3);
  Serial.print("MIN,");
  Serial.println(minVal, 3);
  Serial.print("MAX,");
  Serial.println(maxVal, 3);
  Serial.print("RANGE,");
  Serial.println(range, 3);

  printMenu();
}

void printHeader() {
  Serial.println("=====================================");
  Serial.println("HC-SR04 KARAKTERISASI SENSOR");
  Serial.println("Slave 2 - TK-201 Level Control");
  Serial.println("=====================================");
}

void printMenu() {
  Serial.println("\nMENU:");
  Serial.println("  [1] Single reading");
  Serial.println("  [c] Continuous mode");
  Serial.println("  [r] Raw data (10x)");
  Serial.println("  [t] Test repeatability");
  Serial.println("  [h] Help");
  Serial.println("\nFormat DATA: jarak_cm, raw_pulse_us, jarak_001mm");
  Serial.println("-------------------------------------");
}
