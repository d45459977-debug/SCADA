/*
 * HC-SR04 Kalibrasi Volume - TK-201 (Sistem Plunger)
 *
 * Sketch untuk kalibrasi sensor ultrasonik HC-SR04
 * untuk mengukur volume texapon dengan sistem plunger.
 *
 * KONFIGURASI:
 * - Tinggi maksimal plunger: 22.6 cm (dari pengukuran manual)
 * - Luas penampang: 254.1 cm²
 * - Volume maksimal: ~5742 mL
 *
 * CARA KERJA:
 * - Kalibrasi: Baca jarak sensor saat plunger di posisi ATAS (penuh)
 * - Saat pakai: Hitung tinggi texapon dari selisih jarak
 *
 * Wiring:
 * HC-SR04 TRIG → Arduino D9
 * HC-SR04 ECHO → Arduino D10
 * HC-SR04 VCC  → 5V
 * HC-SR04 GND  → GND
 *
 * Serial: 9600 baud
 */

#include <Arduino.h>

const int TRIG_PIN = 9;
const int ECHO_PIN = 10;

const float KECEPATAN_SUARA = 0.0343;  // cm/us (343 m/s pada 20°C)
const float LUAS_PENAMPANG = 254.1;    // cm²
const float TINGGI_MAX_CM = 22.6;      // Tinggi maksimal dari MANUAL
const float VOLUME_MAX_ML = LUAS_PENAMPANG * TINGGI_MAX_CM;

const int SAMPEL_KALIBRASI = 20;  // Tambah sampel
const int DELAY_BACA = 100;
const int MOVING_AVG_SIZE = 5;     // Moving average filter

// Variabel kalibrasi
float jarakPenuh = 0.0;    // Jarak sensor ke plunger saat PENUH (di batas 22.6 cm)
bool terkalibrasi = false;
bool continuousMode = false;
unsigned long lastBacaTime = 0;

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);

  tampilkanMenu();
}

void loop() {
  if (continuousMode && (millis() - lastBacaTime >= DELAY_BACA)) {
    bacaDanTampilkanVolume();
    lastBacaTime = millis();
  }

  if (Serial.available() > 0) {
    char perintah = Serial.read();
    prosesPerintah(perintah);

    while (Serial.available() > 0) {
      Serial.read();
    }
  }
}

// ============================================================================
// FUNGSI UTAMA
// ============================================================================

void prosesPerintah(char cmd) {
  switch (cmd) {
    case 'k':
    case 'K':
      kalibrasiJarakPenuh();
      break;

    case 'm':
    case 'M':
      kalibrasiManual();
      break;

    case 'u':
    case 'U':
      bacaDanTampilkanVolume();
      break;

    case 'c':
    case 'C':
      if (terkalibrasi) {
        continuousMode = true;
        Serial.println(F("\n=== CONTINUOUS MODE ==="));
        Serial.println(F("Tekan [s] untuk berhenti\n"));
      } else {
        Serial.println(F("\nBELUM KALIBRASI! Ketik [k] dulu.\n"));
      }
      break;

    case 's':
    case 'S':
      continuousMode = false;
      Serial.println(F("\n=== STOP ===\n"));
      tampilkanMenu();
      break;

    case 't':
    case 'T':
      tampilkanTabelVolume();
      break;

    case 'r':
    case 'R':
      resetKalibrasi();
      break;

    case 'h':
    case 'H':
    case '?':
      tampilkanMenu();
      break;

    default:
      Serial.println(F("Perintah tidak dikenali. Ketik [h] untuk help."));
  }
}

// ============================================================================
// KALIBRASI
// ============================================================================

void kalibrasiJarakPenuh() {
  Serial.println(F("\n=== KALIBRASI JARAK PENUH ==="));
  Serial.println(F("Pastikan plunger di posisi PALING ATAS (mentok batas 22.6 cm)!"));
  Serial.println(F("Mengambil pembacaan..."));

  float totalJarak = 0.0;
  int sampelValid = 0;

  for (int i = 0; i < SAMPEL_KALIBRASI; i++) {
    float jarak = bacaJarak();

    if (jarak > 0 && jarak < 50) {
      totalJarak += jarak;
      sampelValid++;
      Serial.print(F("Sampel "));
      Serial.print(i + 1);
      Serial.print(F(": "));
      Serial.print(jarak, 2);
      Serial.println(F(" cm"));
    } else {
      Serial.print(F("Sampel "));
      Serial.print(i + 1);
      Serial.println(F(": ERROR (abaikan)"));
    }

    delay(100);
  }

  if (sampelValid > 0) {
    jarakPenuh = totalJarak / sampelValid;
    terkalibrasi = true;

    Serial.println(F("\n--- HASIL KALIBRASI ---"));
    Serial.print(F("Jarak Penuh (sensor→plunger): "));
    Serial.print(jarakPenuh, 2);
    Serial.println(F(" cm"));
    Serial.print(F("Tinggi Maksimal (manual): "));
    Serial.print(TINGGI_MAX_CM, 2);
    Serial.println(F(" cm"));
    Serial.print(F("Volume Maksimal: "));
    Serial.print(VOLUME_MAX_ML, 0);
    Serial.println(F(" mL"));
    Serial.println(F("\nKALIBRASI SELESAI! Siap mengukur volume.\n"));
  } else {
    Serial.println(F("\nKalibrasi GAGAL! Periksa sensor.\n"));
  }
}

void kalibrasiManual() {
  Serial.println(F("\n=== KALIBRASI MANUAL ==="));
  Serial.println(F("Masukkan jarak dari sensor ke plunger saat PENUH (cm)."));
  Serial.println(F("Contoh: untuk 2.5 cm, ketik: 2.5"));

  Serial.setTimeout(10000);
  float inputJarak = Serial.parseFloat();

  if (inputJarak > 0 && inputJarak < 50) {
    jarakPenuh = inputJarak;
    terkalibrasi = true;

    Serial.print(F("\nJarak Penuh diset: "));
    Serial.print(jarakPenuh, 2);
    Serial.println(F(" cm"));
    Serial.print(F("Volume Maksimal: "));
    Serial.print(VOLUME_MAX_ML, 0);
    Serial.println(F(" mL"));
    Serial.println(F("\nKalibrasi BERHASIL!\n"));
  } else {
    Serial.println(F("\nInput tidak valid. Gunakan format angka.\n"));
  }

  Serial.setTimeout(1000);
}

void resetKalibrasi() {
  jarakPenuh = 0.0;
  terkalibrasi = false;
  continuousMode = false;
  Serial.println(F("\nKalibrasi di RESET.\n"));
  tampilkanMenu();
}

// ============================================================================
// PENGUKURAN VOLUME
// ============================================================================

void bacaDanTampilkanVolume() {
  if (!terkalibrasi) {
    Serial.println(F("\nBELUM KALIBRASI! Ketik [k] atau [m] dulu.\n"));
    continuousMode = false;
    return;
  }

  float jarakSekarang = bacaJarak();
  float selisihJarak = jarakSekarang - jarakPenuh;
  float tinggiTexapon = TINGGI_MAX_CM - selisihJarak;
  float volume = LUAS_PENAMPANG * tinggiTexapon;
  float persen = (volume / VOLUME_MAX_ML) * 100.0;

  Serial.print(F("Jarak: "));
  Serial.print(jarakSekarang, 2);
  Serial.print(F(" cm | "));

  Serial.print(F("Selisih: "));
  Serial.print(selisihJarak, 2);
  Serial.print(F(" cm | "));

  Serial.print(F("Tinggi: "));
  Serial.print(tinggiTexapon, 2);
  Serial.print(F(" cm | "));

  Serial.print(F("Volume: "));
  Serial.print(volume, 0);
  Serial.print(F(" / "));
  Serial.print(VOLUME_MAX_ML, 0);
  Serial.print(F(" mL ("));
  Serial.print(persen, 1);
  Serial.println(F("%))"));
}

// ============================================================================
// SENSOR HC-SR04
// ============================================================================

float bacaJarak() {
  // Ambil multiple reading dan rata-rata untuk filter noise
  float total = 0.0;
  int count = 0;

  for (int i = 0; i < MOVING_AVG_SIZE; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000);

    if (duration > 0) {
      float jarak = (duration * KECEPATAN_SUARA) / 2.0;
      if (jarak > 0 && jarak < 100) {  // Filter nilai tidak valid
        total += jarak;
        count++;
      }
    }
    delayMicroseconds(10000);  // 10ms delay antar reading
  }

  if (count > 0) {
    return total / count;
  }
  return -1;  // Semua reading gagal
}

// ============================================================================
// TABEL & MENU
// ============================================================================

void tampilkanTabelVolume() {
  if (!terkalibrasi) {
    Serial.println(F("\nSELESAIKAN KALIBRASI DULU [k]!\n"));
    return;
  }

  Serial.println(F("\n=== TABEL KONVERSI TINGGI → VOLUME ==="));
  Serial.println(F(" Tinggi (cm) | Volume (mL) | Persen"));
  Serial.println(F("-------------|--------------|--------"));

  for (float h = 0.5; h <= TINGGI_MAX_CM; h += 0.5) {
    float vol = LUAS_PENAMPANG * h;
    float pct = (vol / VOLUME_MAX_ML) * 100.0;

    Serial.print(F("     "));
    Serial.print(h, 1);
    Serial.print(F("    | "));
    Serial.print(vol, 1);
    Serial.print(F("     | "));
    Serial.print(pct, 1);
    Serial.println(F("%"));
  }

  Serial.println();
}

void tampilkanMenu() {
  Serial.println(F("\n╔════════════════════════════════════════╗"));
  Serial.println(F("║  HC-SR04 KALIBRASI VOLUME - TK-201    ║"));
  Serial.println(F("║       (Sistem Plunger)                ║"));
  Serial.println(F("╠════════════════════════════════════════╣"));
  Serial.println(F("║ Tinggi Max: 22.6 cm (manual)          ║"));
  Serial.println(F("║ Volume Max: 5742 mL                   ║"));
  Serial.println(F("╠════════════════════════════════════════╣"));
  Serial.println(F("║ [k] Kalibrasi (plunger di atas!)      ║"));
  Serial.println(F("║ [m] Kalibrasi Manual                  ║"));
  Serial.println(F("║ [u] Ukur Volume Sekarang              ║"));
  Serial.println(F("║ [c] Continuous Mode                   ║"));
  Serial.println(F("║ [s] Stop                              ║"));
  Serial.println(F("║ [t] Tampilkan Tabel Volume            ║"));
  Serial.println(F("║ [r] Reset Kalibrasi                   ║"));
  Serial.println(F("║ [h] Help / Menu                       ║"));
  Serial.println(F("╚════════════════════════════════════════╝"));

  if (terkalibrasi) {
    Serial.print(F("Status: TERKALIBRASI (Ref: "));
    Serial.print(jarakPenuh, 2);
    Serial.println(F(" cm)"));
  } else {
    Serial.println(F("Status: BELUM KALIBRASI"));
  }

  Serial.println();
}
