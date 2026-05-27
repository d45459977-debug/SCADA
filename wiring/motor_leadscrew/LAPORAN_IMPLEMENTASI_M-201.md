# Laporan Implementasi Motor Lead Screw M-201
## Slave 2 - TK-201 Level Control

**Project:** SOLOPHOBIA - Heating & Mixing Miniplant
**Tanggal:** 6 Mei 2026
**Aktuator:** Motor DC 12V Geared + TB6612FNG Driver
**Lokasi:** `/b400/wiring/motor_leadscrew/`

---

# BAB 1
## IMPLEMENTASI

Implementasi motor lead screw M-201 pada Slave 2 (TK-201) dilakukan untuk menggerakkan plunger sistem dosing texapan. Motor DC dengan *gearbox* memutar *lead screw* yang menggerakkan plunger naik-turun untuk mengontrol volume texapon yang keluar.

### Spesifikasi Hardware

| Komponen | Spesifikasi |
|----------|-------------|
| Motor DC | 12V DC, geared (reducer) |
| Motor Driver | TB6612FNG (dual motor driver) |
| Max Current (per channel) | 1.2A continuous, 3.2A peak |
| Logic Voltage | 2.7V - 5.5V |
| Motor Voltage | 2.5V - 13.5V |
| PSU | 12V DC, minimal 2A |
| Lead Screw | Acme/Trapesoid dengan nut |

### Wiring Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                         PSU 12V DC                               │
│                                                                         │
│   V+ (+12V) ────────────────────────────────► VM (TB6612FNG)    │
│                                                                         │
│   V- (GND)  ──┬───────────────────────────────► GND (TB6612FNG)   │
│               │                                                        │
│               └───────────────────────────────► GND (Arduino)     │
└─────────────────────────────────────────────────────────────────────┘

TB6612FNG Driver:
  AIN1 → D8 (Arduino) - Direction control 1
  AIN2 → D7 (Arduino) - Direction control 2
  PWMA → D9 (Arduino) - PWM speed control
  STBY → D5 (Arduino) - Standby enable (HIGH = on)
  VCC  → 5V (Arduino) - Logic power
  AO1  → Motor lead 1
  AO2  → Motor lead 2
```

### Pin Assignment Arduino

| Pin | Fungsi | Tipe |
|-----|--------|------|
| D9 | PWMA (Speed/PWM) | Output Analog |
| D8 | AIN1 (Direction) | Output Digital |
| D7 | AIN2 (Direction) | Output Digital |
| D5 | STBY (Standby Enable) | Output Digital |
| GND | Common Ground | Power |

### Algoritma Kontrol Motor

**Short Brake (Stop):**
```cpp
digitalWrite(PIN_AIN1, HIGH);
digitalWrite(PIN_AIN2, HIGH);
analogWrite(PIN_PWMA, 0);
```

**Motor UP (Naik):**
```cpp
digitalWrite(PIN_AIN1, HIGH);
digitalWrite(PIN_AIN2, LOW);
analogWrite(PIN_PWMA, speed);  // 0-255
```

**Motor DOWN (Turun):**
```cpp
digitalWrite(PIN_AIN1, LOW);
digitalWrite(PIN_AIN2, HIGH);
analogWrite(PIN_PWMA, speed);  // 0-255
```

---

# BAB 2
## HASIL PENGUJIAN

### 2.1 Tes Direction

Pengujian arah putaran motor dengan speed 150/255:

| Test | Arah | Durasi | Hasil |
|------|------|--------|-------|
| 1 | UP (NAIK) | 2 detik | ✅ Motor berputar, plunger naik |
| 2 | DOWN (TURUN) | 2 detik | ✅ Motor berputar, plunger turun |
| Stop | Short brake | - | ✅ Motor berhenti cepat |

**Status:** ✅ **PASS** — Arah putaran sesuai ekspektasi

### 2.2 Tes Speed Control

Pengujian variasi speed (PWM):

| Speed (PWM) | Persen | Hasil |
|-------------|--------|-------|
| 64 | 25% | ✅ Motor berputar pelan |
| 128 | 50% | ✅ Motor berputar sedang |
| 191 | 75% | ✅ Motor berputar cepat |
| 255 | 100% | ✅ Motor berputar maksimal |

**Status:** ✅ **PASS** — PWM control berfungsi baik

### 2.3 Tes Standby Enable

Pengujian pin STBY:

| Kondisi STBY | Hasil Motor |
|--------------|-------------|
| LOW | ❌ Motor tidak bergerak (standby) |
| HIGH | ✅ Motor aktif |

**Status:** ✅ **PASS** — Pin STBY berfungsi sebagai enable

---

# BAB 3
## PERMASALAHAN DAN SOLUSI

### 3.1 Motor Tidak Berputar Saat Pertama Kali

**Masalah:** Saat sketch pertama diupload, motor tidak berputar padahal wiring sudah benar.

**Penyebab:** Pin STBY belum diset HIGH, sehingga driver dalam mode standby.

**Solusi:** Tambahkan `digitalWrite(PIN_STBY, HIGH);` di setup() untuk enable driver.

### 3.2 Motor Berputar Terbalik

**Masalah:** Arah putaran motor berlawanan dengan yang diharapkan (UP justru turunkan plunger).

**Penyebab:** Polaritas kabel motor terbalik.

**Solusi:** Tukar kabel motor di AO1 dan AO2, atau ubah logika AIN1/AIN2 di kode. Solusi yang dipilih: **tukar kabel motor**.

### 3.3 Common Ground

**Masalah:** Motor berperilaku tidak stabil (kadang jalan kadang tidak).

**Penyebab:** GND Arduino, GND TB6612FNG, dan V- PSU tidak terhubung (tidak common ground).

**Solusi:** Hubungkan V- PSU ke GND TB6612FNG dan GND Arduino dalam satu rangkaian common ground.

---

# BAB 4
## KESIMPULAN

Implementasi motor lead screw M-201 dengan driver TB6612FNG telah berhasil dilakukan. Motor dapat dikontrol arah (UP/DOWN) dan speed (0-100%) melalui Arduino.

**Hal-hal yang telah dicapai:**
1. ✅ Wiring TB6612FNG ke Arduino
2. ✅ Wiring TB6612FNG ke PSU 12V
3. ✅ Wiring TB6612FNG ke motor DC
4. ✅ Common ground antara semua komponen
5. ✅ Sketch test motor dengan menu serial
6. ✅ Kontrol direction (AIN1, AIN2)
7. ✅ Kontrol speed (PWM PWMA)
8. ✅ Standby enable (STBY)
9. ✅ Short brake untuk stop cepat

**Keterbatasan:**
1. Driver hanya support sampai 1.2A continuous per channel
2. Perlu heat sink untuk operasi arus tinggi
3. belum terintegrasi dengan sensor HC-SR04 untuk feedback

**Rekomendasi untuk Implementasi Slave 2:**
1. Gunakan sketch `test_motor_basic.ino` sebagai basis kontrol motor
2. Integrasikan dengan sensor HC-SR04 untuk position control loop
3. Tambah safety limit (maximum travel, current limit)
4. Implementasi *deadband* ±0.3 cm sesuai B300

---

# REFERENSI

- Dokumen B300: Desain Hardware & Software
- Datasheet TB6612FNG: Toshiba Dual Motor Driver
- Lokasi kode: `/b400/wiring/motor_leadscrew/test_motor_basic/`
- Wiring diagram: `/b400/wiring/motor_leadscrew/README.md`

---

**Dibuat oleh:** Hifzhudin
**Disetujui:** *(pending verification)*
**Tanggal:** 2026-05-06
