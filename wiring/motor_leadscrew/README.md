# Wiring Motor Lead Screw - Slave 2 TK-201

**Project:** SOLOPHOBIA - Heating & Mixing Miniplant
**Lokasi:** `/B400/wiring/motor_leadscrew/`
**Tanggal:** 2026-05-05

---

## Overview

Motor lead screw M-201 digunakan pada Slave 2 (TK-201) untuk *position control* plunger sistem *dosing* texapon. Motor menggerakkan *lead screw* yang menggerakkan plunger naik-turun untuk mengontrol level cairan.

---

## Komponen

| Komponen | Spesifikasi | Qty |
|----------|-------------|-----|
| Motor DC | 12V DC, *geared* (reducer) | 1 |
| Motor Driver | **TB6612FNG** | 1 |
| Power Supply | 12V DC, minimal 2A | 1 |
| Arduino Uno | Microcontroller | 1 |
| Lead Screw | Acme/Trapesoid, dengan *nut* | 1 set |

---

## Wiring Diagram (TB6612FNG Driver)

```
┌─────────────────────────────────────────────────────────────────┐
│                         POWER SUPPLY 12V                        │
│                            (PSU 12V)                            │
└───────────────────────────┬─────────────────────────────────────┘
                            │
                ┌───────────┴───────────┐
                │                       │
                ▼                       ▼
         ┌─────────────┐         ┌─────────────┐
         │   L298N     │         │   Arduino   │
         │   DRIVER    │         │    UNO      │
         └─────────────┘         └─────────────┘
```

### Koneksi Lengkap

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         POWER SUPPLY (12V DC)                           │
│                                                                         │
│   (+) POSITIVE ────────────────────────────┐                           │
│                                            │                           │
│   (-) GND ────────────────────────────────┼───────────────────────┐   │
│                                            │                       │   │
└────────────────────────────────────────────┼───────────────────────┼───┘
                                             │                       │
                                             ▼                       ▼
                                      ┌─────────────┐         ┌─────────────┐
                                      │  TB6612FNG  │         │   Arduino   │
                                      │   DRIVER    │         │    UNO      │
                                      └─────────────┘         └─────────────┘
```

### Detail Koneksi TB6612FNG

```
┌─────────────────────────────────────────────────────────────────────────┐
│                            TB6612FNG DRIVER                             │
│                                                                         │
│   ┌───────────────────────────────────────────────────────────────┐    │
│   │                                                               │    │
│   │   POWER SIDE              LOGIC SIDE           MOTOR SIDE     │    │
│   │   ┌─────────┐            ┌─────────┐           ┌─────────┐   │    │
│   │   │ VM      │◄───────────┤  12V    │           │         │   │    │
│   │   │         │            │         │           │ AO1 ────┼───┐──┐   │    │
│   │   │ VCC     │◄───────────┤  5V     │           │ AO2 ────┼───┼──┼───┐│    │
│   │   │ GND     │◄───────────┤  GND    ├──┬────────│         │   │  │  ││   │    │
│   │   │         │            │         │  │        └─────────┘   │  │  ││   │    │
│   │   │ AIN1    │────────────┼─────────┼──┘                       │  │  ││   │    │
│   │   │ AIN2    │────────────┼─────────┼──┐        ┌───────────▼──▼──▼──▼──┐ │    │
│   │   │ PWMA    │────────────┼─────────┼──┼────────┤   MOTOR DC  (M-201) │ │    │
│   │   │ BIN1    │────────────┼─────────┼──┘        │                       │ │    │
│   │   │ BIN2    │────────────┼─────────┤           │                       │ │    │
│   │   │ PWMB    │────────────┼─────────┤           └───────────────────────┘ │    │
│   │   │ STBY    │────────────┼─────────┤                                       │    │
│   │   └─────────┘            └────┬────┘                                       │    │
│   └─────────────────────────────────┴────────────────────────────────────────┘    │
│                                      │                                            │
└──────────────────────────────────────┼────────────────────────────────────────────┘
                                       │
                                       │ (Ke Arduino)
                                       │
         ┌─────────────────────────────┴─────────────────────────────────────────┐
         │                         ARDUINO UNO                                   │
         │                                                                     │
         │   D9  ─────────────────────────────────────► PWMA  (Speed Motor A) │
         │   D8  ─────────────────────────────────────► AIN1  (Direction 1)   │
         │   D7  ─────────────────────────────────────► AIN2  (Direction 2)   │
         │   D5  ─────────────────────────────────────► STBY  (Standby)       │
         │                                                                     │
         │   5V  ─────────────────────────────────────► VCC   (Logic Power)   │
         │   GND ─────────────────────────────────────► GND   (Common GND)    │
         │                                                                     │
         └─────────────────────────────────────────────────────────────────────┘
```

---

## Tabel Koneksi

### TB6612FNG → Arduino

| TB6612FNG Pin | Arduino Pin | Keterangan |
|---------------|-------------|------------|
| AIN1 | D8 | *Direction* Control 1 |
| AIN2 | D7 | *Direction* Control 2 |
| PWMA | D9 | PWM Speed Control |
| STBY | D5 | *Standby* (HIGH = enable) |
| VCC | 5V | Logic power (2.7-5.5V) |
| GND | GND | Ground common |

### TB6612FNG → Power Supply

| TB6612FNG Pin | PSU 12V | Keterangan |
|---------------|---------|------------|
| VM | +12V (Positive) | Motor power (2.5-13.5V) |
| GND | - (GND) | Common ground |

### TB6612FNG → Motor DC (M-201)

| TB6612FNG Pin | Motor Lead | Keterangan |
|---------------|------------|------------|
| AO1 | Lead 1 (Merah/+) | Motor terminal 1 |
| AO2 | Lead 2 (Hitam/-) | Motor terminal 2 |

> **Catatan:** Polaritas motor menentukan arah putaran. Jika arah terbalik, *swap* kabel motor.

> **Catatan:** Polaritas motor menentukan arah putaran. Jika arah terbalik, *swap* kabel motor.

---

## Pin Assignment Arduino

| Pin | Fungsi | Tipe |
|-----|--------|------|
| D9 | PWMA (Speed/PWM) | Output *Analog* |
| D8 | AIN1 (Direction) | Output *Digital* |
| D7 | AIN2 (Direction) | Output *Digital* |
| D5 | STBY (*Standby Enable*) | Output *Digital* |
| GND | Common Ground | Power |

---

## Kode Arduino Dasar

### Kontrol Direction dan Speed (TB6612FNG)

```cpp
// Pin Assignment TB6612FNG
#define PIN_PWMA  9   // PWM speed control
#define PIN_AIN1  8   // Direction 1
#define PIN_AIN2  7   // Direction 2
#define PIN_STBY  5   // Standby enable

void setup() {
  pinMode(PIN_PWMA, OUTPUT);
  pinMode(PIN_AIN1, OUTPUT);
  pinMode(PIN_AIN2, OUTPUT);
  pinMode(PIN_STBY, OUTPUT);

  // Enable driver (keluar dari standby)
  digitalWrite(PIN_STBY, HIGH);
}

void motorStop() {
  // Brake mode (short brake)
  digitalWrite(PIN_AIN1, HIGH);
  digitalWrite(PIN_AIN2, HIGH);
  analogWrite(PIN_PWMA, 0);
}

void motorUp(int speed) {
  // ARAH ATAS (sesuaikan dengan wiring)
  digitalWrite(PIN_AIN1, HIGH);
  digitalWrite(PIN_AIN2, LOW);
  analogWrite(PIN_PWMA, speed);  // 0-255
}

void motorDown(int speed) {
  // ARAH BAWAH (sesuaikan dengan wiring)
  digitalWrite(PIN_AIN1, LOW);
  digitalWrite(PIN_AIN2, HIGH);
  analogWrite(PIN_PWMA, speed);  // 0-255
}

void loop() {
  // Contoh: motor naik dengan speed 50% (127/255)
  motorUp(127);
  delay(2000);

  motorStop();
  delay(1000);

  // Contoh: motor turun dengan speed 50%
  motorDown(127);
  delay(2000);

  motorStop();
  delay(1000);
}
```

### Catatan TB6612FNG

- **STBY (Standby):** Harus HIGH agar driver aktif. Jika LOW, motor dalam *mode standby*
- **Short Brake:** AIN1=HIGH, AIN2=HIGH → motor berhenti cepat (*brake*, bukan *coast*)
- **Coast Stop:** PWMA=0 → motor berhenti perlahan (*coast*)

---

## Logika Kontrol (Berdasarkan B300)

### *Position Control* Loop

```
ATUR
  LUAS_PENAMPANG = 314.16   // cm² (diameter tangki 20 cm)
  FAKTOR_KONVERSI = 3141.6  // konversi cm ke mL
  toleransi = 0.3           // cm
  posisi_sp = 15.0          // cm (contoh setpoint)

MULAI
  baca posisi_aktual dari sensor LT-201 (HC-SR04)

  selisih = posisi_sp - posisi_aktual

  JIKA selisih > toleransi MAKA
    motor_atur(NAIK, speed_map(selisih))
  ATAU JIKA selisih < -toleransi MAKA
    motor_atur(TURUN, speed_map(abs(selisih)))
  ATAU
    motor_stop()

ULANGI SELAMA 500 ms  // sampling 2 Hz
```

---

## Pengujian

### 1. Tes Koneksi Power

1. Hubungkan PSU 12V ke L298N
2. Cek voltage multimeter:
   - Antara +12V dan GND pada L298N: harus ~12V
   - Jika ada *on-board* 5V regulator: cek antara +5V dan GND: harus ~5V

### 2. Tes Motor Direction

Upload kode tes sederhana:
```cpp
#define PIN_PWMA  9
#define PIN_AIN1  8
#define PIN_AIN2  7
#define PIN_STBY  5

void setup() {
  pinMode(PIN_PWMA, OUTPUT);
  pinMode(PIN_AIN1, OUTPUT);
  pinMode(PIN_AIN2, OUTPUT);
  pinMode(PIN_STBY, OUTPUT);

  digitalWrite(PIN_STBY, HIGH);  // Enable driver!
}

void loop() {
  // Tes arah 1 (NAIK)
  digitalWrite(PIN_AIN1, HIGH);
  digitalWrite(PIN_AIN2, LOW);
  analogWrite(PIN_PWMA, 150);
  delay(2000);

  // STOP (Short brake)
  digitalWrite(PIN_AIN1, HIGH);
  digitalWrite(PIN_AIN2, HIGH);
  analogWrite(PIN_PWMA, 0);
  delay(1000);

  // Tes arah 2 (TURUN)
  digitalWrite(PIN_AIN1, LOW);
  digitalWrite(PIN_AIN2, HIGH);
  analogWrite(PIN_PWMA, 150);
  delay(2000);

  // STOP
  digitalWrite(PIN_AIN1, HIGH);
  digitalWrite(PIN_AIN2, HIGH);
  analogWrite(PIN_PWMA, 0);
  delay(2000);
}
```

**Yang diperhatikan:**
- STBY harus HIGH agar driver aktif!
- Arah putaran motor sesuai dengan label "NAIK" dan "TURUN"
- Jika terbalik, *swap* kabel motor AO1 dan AO2

### 3. Tes Speed Control

Gunakan PWM (0-255) untuk mengontrol kecepatan:
- 0 = stop
- 127 = 50% speed
- 255 = 100% speed

---

## Troubleshooting

### Motor tidak berputar

**Cek:**
1. Koneksi power PSU ke L298N (+12V, GND)
2. Koneksi motor ke OUT1, OUT2
3. *Enable* pin (ENA) - harus HIGH atau nilai PWM > 0
4. IN1 dan IN2 tidak sama-sama LOW atau HIGH

### Motor berputar terbalik

**Solusi:** Swap kabel motor di AO1 dan AO2, atau ubah logika AIN1/AIN2 di kode.

### Motor tidak jalan sama sekali (padahal wiring benar)

**Cek:**
1. Pin STBY - harus HIGH untuk enable driver
2. VCC terhubung ke 5V Arduino (logic power)
3. Voltage antara VCC dan GND harus ~5V

### Motor berputar tapi *lead screw* tidak bergerak

**Cek:**
1. *Coupling* antara motor dan *lead screw* - apakah kencang?
2. *Set screw* pada coupling - apakah terkencang?
3. *Lead screw nut* - apakah terpasang benar?

### TB6612FNG panas berlebih

**Penyebab:**
- Arus motor melebihi rating (1.2A per channel *continuous*, 3.2A *peak*)
- PWM duty cycle tinggi dalam waktu lama
- Tidak ada *heat sink* (untuk arus > 1A)

**Solusi:**
- Tambah *heat sink* pada TB6612FNG
- Pastikan *current rating* motor sesuai
- Pertimbangkan driver lain (DRV8871, BTS7960) untuk arus lebih besar

---

## Catatan Penting

1. **Common Ground:** Pastikan GND Arduino, GND TB6612FNG, dan GND PSU terhubung bersama
2. **STBY Pin:** Jangan lupa set STBY = HIGH untuk enable driver
3. **Arus Motor:** TB6612FNG max 1.2A *continuous* per channel - pastikan PSU cukup
4. **Logic Level:** TB6612FNG bekerja pada 3.3V atau 5V logic
5. **Arah Putaran:** Kalibrasi arah "NAIK" dan "TURUN" setelah wiring selesai

---

## Catatan Penting

1. **Common Ground:** Pastikan GND Arduino, GND L298N, dan GND PSU terhubung bersama
2. **Arus Motor:** Motor DC *geared* bisa *draw* arus tinggi saat *stall* - pastikan PSU cukup (minimal 2A)
3. **Back EMF:** L298N sudah memiliki *flyback diode* internal untuk proteksi
4. **Arah Putaran:** Kalibrasi arah "NAIK" dan "TURUN" setelah wiring selesai

---

## Dokumen Terkait

- `../../../B300/CLAUDE.md` - Desain sistem Slave 2
- `../../../B400/sensor_level/README.md` - Sensor HC-SR04 untuk *feedback* posisi
- `../../../B400/sensor_level/perhitungan_volume_tk201.md` - Konversi jarak ke volume

---

## Riwayat Revisi

| Tanggal | Versi | Perubahan |
|---------|-------|-----------|
| 2026-05-05 | 1.0 | Dokumentasi awal wiring motor lead screw |
| 2026-05-05 | 1.1 | Update ke TB6612FNG driver (sebelumnya L298N) |

---

## Pinout TB6612FNG

Referensi cepat pinout TB6612FNG (16-pin SOP):

```
        ┌─────────────────────┐
    AIN1 │ 1              16 │ VCC
    AIN2 │ 2              15 │ AO2
    PWMA │ 3              14 │ AO1
 STBY(┤) 4              13 │ VM
    BIN1 │ 5              12 │ GND
    BIN2 │ 6              11 │ GND
    PWMB │ 7              10 │ GND
    GND  │ 8               9 │ GND
        └─────────────────────┘
```

**Keterangan Pin:**
- **VM:** Motor power supply (2.5V - 13.5V)
- **VCC:** Logic power supply (2.7V - 5.5V)
- **GND:** Ground
- **AIN1, AIN2:** Motor A direction control
- **BIN1, BIN2:** Motor B direction control (tidak dipakai untuk single motor)
- **PWMA, PWMB:** PWM speed control
- **STBY:** Standby (LOW = standby, HIGH = enable)
- **AO1, AO2:** Motor A output
- **BO1, BO2:** Motor B output (tidak dipakai untuk single motor)
