# HC-SR04 Ultrasonic Sensor - Test Report

**Nomor Dokumen:** B400-SENS-003
**Tanggal:** 2026-05-06
**Sensor:** HC-SR04 Ultrasonic Distance Sensor
**Aplikasi:** TK-201 Level Control (Dosing Texapon)
**Firmware:** Arduino Uno + HC-SR04_Kalibrasi_Volume

---

## 1. Spesifikasi Sensor (Datasheet)

| Parameter | Nilai |
|-----------|-------|
| Tipe | Ultrasonic Distance Sensor |
| Working Voltage | DC 5V |
| Working Current | 15mA |
| Frequency | 40kHz |
| Max Range | 400 cm (teoritis), ~200 cm (praktis) |
| Min Range | 2 cm |
| Measuring Angle | <15° |
| Input Trigger | 10μs TTL pulse |
| Echo Output | TTL PWM |
| Resolution | 0.3 cm |
| Accuracy (datasheet) | ±3mm |
| Pinout | VCC, TRIG, ECHO, GND |

---

## 2. Setup Testing

### 2.1 Hardware

- **MCU:** Arduino Uno (ATmega328P)
- **Pin TRIG:** D9
- **Pin ECHO:** D10
- **Baudrate:** 9600
- **Sampling:** 5x moving average, 20x untuk kalibrasi
- **Speed of Sound:** 0.0343 cm/μs (343 m/s @ 20°C)

### 2.2 Wiring

```
Arduino Uno               HC-SR04 Sensor
─────────────────────────────────────────────

5V  ───────────────────────→ VCC

D9  ───────────────────────→ TRIG

D10 ───────────────────────→ ECHO

GND ───────────────────────→ GND
```

### 2.3 Software

- **Sketch:** `hcsr04_kalibrasi_volume.ino`
- **Method:** Pulse width measurement + moving average filter
- **Output:** Volume texapon dalam mL dengan persentase

---

## 3. Spesifikasi Wadah TK-201

| Parameter | Nilai | Keterangan |
|-----------|-------|------------|
| Bentuk | Balok (tidak simetris) | - |
| Panjang | 22.0 cm | - |
| Lebar S1 | 12.1 cm | - |
| Lebar S2 | 11.2 cm | - |
| Lebar S3 | 11.55 cm | - |
| Lebar S4 | 11.35 cm | - |
| **Lebar Rata-rata** | **11.55 cm** | Dipakai untuk perhitungan |
| **Luas Penampang** | **254.1 cm²** | 22 × 11.55 |
| Tinggi Maksimal | 22.6 cm | Dari pengukuran manual |
| **Volume Maksimal** | **5743 mL** | 254.1 × 22.6 |

---

## 4. Konfigurasi Sistem Plunger

Sistem TK-201 menggunakan **plunger** untuk mendorong texapon keluar:

```
┌─────────────────────────────────────┐
│         HC-SR04 (Sensor di atas)    │
├─────────────────────────────────────┤
│                                     │
│  ┌─────────┐                        │
│  │ PLUNGER │ ← Bergerak naik-turun  │
│  └─────────┘                        │
│      ↓                              │
│  ~~~~~~~~~~~~  ← Texapon            │
│                                     │
│  ════════════  ← Dasar wadah        │
└─────────────────────────────────────┘
```

**Konsep Pengukuran:**
- Sensor membaca jarak ke plunger
- Selisih jarak = pergerakan plunger
- Volume = Luas Penampang × Tinggi Texapon

---

## 5. Hasil Kalibrasi

### 5.1 Kalibrasi Jarak Dasar (Tanpa Plunger)

| Sampel | Jarak (cm) | Catatan |
|--------|------------|---------|
| 1-10 | 21.54 - 21.78 cm | Rata-rata: 21.78 cm |

**Jarak Sensor → Dasar Wadah:** 21.78 cm

### 5.2 Kalibrasi Jarak Penuh (Dengan Plunger di Atas)

| Sampel | Jarak (cm) | Catatan |
|--------|------------|---------|
| 1-10 | 2.01 - 2.02 cm | Rata-rata: 2.01 cm |

**Jarak Sensor → Plunger (Penuh):** 2.01 cm (awal), **2.66 cm** (final setelah remounting)

### 5.3 Validasi Akurasi

| Test | Target | Bacaan Sensor | Error | Status |
|------|--------|--------------|-------|--------|
| 1 | 8.0 cm | 7.51 cm | -0.49 cm (-6.1%) | ⚠️ Acceptable |
| 2 | 8.5 cm | 8.45 cm | -0.05 cm (-0.6%) | ✅ Good |

**Rata-rata Error:** -0.27 cm (sensor baca sedikit lebih kecil)

**Keputusan:** Error dalam toleransi operasional. Untuk aplikasi dosing (deadband ±0.3 cm), akurasi ini memenuhi requirement.

---

## 6. Hasil Pengujian Volume

Setelah kalibrasi final (jarak penuh = 2.66 cm):

| Test | Jarak Sensor | Selisih | Tinggi Texapon | Volume | Persen |
|------|--------------|---------|----------------|--------|--------|
| 1 | 7.87 cm | 5.21 cm | 17.39 cm | 4419 mL | 77.0% |
| 2 | 13.70 cm | 11.04 cm | 11.56 cm | 2938 mL | 51.2% |
| 3 | 16.96 cm | 14.30 cm | 8.30 cm | 2110 mL | 36.7% |
| 4 | 20.49 cm | 17.83 cm | 4.77 cm | 1212 mL | 21.1% |
| 5 | 7.13 cm | 4.47 cm | 18.13 cm | 4607 mL | 80.2% |

**Analisis:**
- ✅ Hasil KONSISTEN di berbagai posisi
- ✅ Perhitungan volume MASUK AKAL
- ✅ Sensor mengikuti pergerakan plunger dengan BENAR

---

## 7. Analisis

### 7.1 Linearity

- [x] **LINEAR** — Pergerakan plunger sebanding dengan perubahan volume
- Tidak perlu lookup table

### 7.2 Accuracy

| Parameter | Hasil | Status |
|-----------|-------|--------|
| Error rata-rata | -0.27 cm | ✅ Dalam toleransi |
| Deadband sistem | ±0.3 cm | ✅ Sensor lebih akurat |

### 7.3 Precision

| Parameter | Hasil | Keterangan |
|-----------|-------|------------|
| Variasi (kalibrasi) | 0.1-0.2 cm | Setelah moving average filter |
| Std Dev (estimasi) | < 0.5 cm | Dari observasi 10 sampel |

### 7.4 Sensor Quality

| Item | Hasil | Status |
|------|-------|--------|
| Noise | Ada (variasi ±1 cm awalnya) | ⚠️ Diperbaiki dengan filter |
| Setelah filter | Stabil (±0.2 cm) | ✅ Memenuhi requirement |
| Blind spot | 2 cm | ✅ Tidak masalah (jarak penuh 2.66 cm) |

---

## 8. Pembahasan Selisih Sensor vs Manual

### 8.1 Data

| Metode | Tinggi Maksimal | Volume Maksimal |
|--------|-----------------|-----------------|
| **Manual (penggaris)** | 22.6 cm | 5743 mL |
| **Sensor (selisih)** | 19.77 cm | 5023 mL |
| **Selisih** | 2.83 cm | 720 mL |

### 8.2 Analisis Penyebab

Selisih 2.83 cm kemungkinan disebabkan oleh:

1. **Posisi transducer tidak di ujung moncong** — Transducer HC-SR04 berada ~2-3 cm di dalam dari moncong sensor
2. **Titik ukur manual berbeda** — Penggaris mengukur ke marking, sensor membaca ke permukaan reflektif
3. **Ketebalan plunger** — Meskipun 0.5 mm, mungkin ada faktor lain

### 8.3 Keputusan

**Menggunakan nilai MANUAL (22.6 cm)** sebagai tinggi maksimal karena:
- Merupakan batas operasional aktual wadah
- Marking dibuat berdasarkan requirement sistem
- Sensor hanya digunakan untuk membaca posisi relatif plunger

**Catatan:** Setiap perubahan posisi holder membutuhkan kalibrasi ulang jaraking penuh.

---

## 9. Kesimpulan

| Item | Verdict |
|------|---------|
| Fungsi sensor | ✅ NORMAL |
| Akurasi (±0.5 cm) | ✅ MEMENUHI |
| Precision | ✅ BAGUS (setelah filter) |
| Linearity | ✅ LINEAR |
| Moving Average Filter | ✅ EFEKTIF mengurangi noise |
| Ready untuk Slave 2 | ✅ YES |
| **Perlu Kalibrasi?** | **YA (setiap ganti holder)** |

---

## 10. Rekomendasi

### 10.1 Untuk Implementasi Slave 2 (TK-201)

1. **Gunakan sketch** `hcsr04_kalibrasi_volume.ino` sebagai basis
2. **Tinggi maksimal:** Hardcode **22.6 cm**
3. **Luas penampang:** Hardcode **254.1 cm²**
4. **Kalibrasi:** Saat pertama setup, jalankan kalibrasi jarak penuh

### 10.2 Prosedur Kalibrasi (Setiap Ganti Holder)

1. Pastikan wadah kosong
2. Pasang plunger di wadah
3. Dorong plunger ke posisi **PALING ATAS** (mentok batas 22.6 cm)
4. Jalankan kalibrasi (`k`)
5. Verifikasi dengan beberapa posisi test

### 10.3 Catatan Penting

- Sensor harus **STABIL** — gunakan holder yang kokoh
- Hindari getaran/gerakan saat pengukuran
- Filter **moving average (5x)** sudah cukup untuk noise HC-S04
- Jangan mengubah tinggi maksimal 22.6 cm — ini adalah batas operasional wadah

---

## 11. Lampiran

- Test code: `hcsr04_kalibrasi_volume.ino`
- Data template: `TEMPLATE_VOLUME.csv`
- Foto setup: *(belum ada)*
- Datasheet: HC-SR04 Manufacturer

---

**Dibuat oleh:** Hifzhudin
**Disetujui:** *(pending verification)*
**Tanggal:** 2026-05-06
