# YFS401 Flow Sensor - Karakteristik & Test Report

**Nomor Dokumen:** B400-SENS-002
**Tanggal:** [ISI TANGGAL]
**Sensor:** YFS401 Hall-Effect Flow Sensor
**Firmware:** Arduino Uno + Hardware Interrupt

---

## 1. Spesifikasi Sensor (Datasheet)

| Parameter | Nilai |
|-----------|-------|
| Tipe | Hall-effect flow sensor (PVC body) |
| Flow Rate Range | 1~5 L/min |
| Working Voltage | DC 5V~24V (min 4.5V) |
| Max Current | 15mA @ 5V |
| Water Pressure | ≤1.75MPa |
| Error (datasheet) | ±2% |
| Pinout | RED=VCC, YELLOW=Signal, BLACK=GND |
| **K_pulse (datasheet)** | **F = 98 × Q** (Q = L/min) |
| Internal diameter | 1.2mm |
| Output | Pulse 50% duty cycle |

**Konversi Datasheet:**
- F = 98 × Q (pulses/min ketika Q = L/min)
- 1 pulse ≈ 10.2 mL (1000 mL / 98 pulse)

---

## 2. Setup Testing

### 2.1 Hardware

- **MCU:** Arduino Uno (ATmega328P)
- **Interrupt Pin:** D2 (INT0)
- **Baudrate:** 9600
- **Sampling Window:** 1000 ms (1 detik)

### 2.2 Wiring

```
Arduino Uno               YFS401 Sensor
─────────────────────────────────────────────

5V  ───────────────────────→ RED (VCC)

D2  ───────────────────────→ YELLOW (Signal)

GND ───────────────────────→ BLACK (GND)
```

### 2.3 Software

- **Sketch:** `yfs401_karakteristik.ino`
- **Method:** Hardware interrupt pulse counting
- **Output:** CSV format (VOLUME_ML,PULSE_COUNT,FLOW_ML_MIN,PULSE_PER_L,TIME_MS)

---

## 3. Hasil Karakterisasi

### 3.1 Raw Data

| Target (mL) | Actual (mL) | Time (s) | Pulse Count | Flow (mL/min) | Pulse/L | Error % |
|-------------|-------------|----------|-------------|---------------|---------|---------|
| [ISI DATA]  |             |          |             |               |         |         |

### 3.2 Grafik Respon

[Plot: Volume vs Pulse Count]

- Jika linear → garis lurus
- Jika non-linear → ada kurva

### 3.3 Repeatability Analysis

Dari repeatability test (10x reading):

| Parameter | Nilai |
|-----------|-------|
| Mean Volume (mL) | [ISI] |
| Min Volume (mL) | [ISI] |
| Max Volume (mL) | [ISI] |
| Range (mL) | [ISI] |
| Std Dev (mL) | [ISI] |
| CV (%) | [ISI] |

### 3.4 K_pulse Aktual

| Metode | K_pulse (pulse/L) | Error vs Datasheet |
|--------|-------------------|-------------------|
| Datasheet | 98.0 | - |
| Rata-rata Pengukuran | [ISI] | [ISI] % |

### 3.5 Signal Quality

Dari raw pulse info test:

| Parameter | Nilai | Status |
|-----------|-------|--------|
| Total Pulse (10s) | [ISI] | - |
| Avg Interval (ms) | [ISI] | - |
| Bounce Detected | [YA/TIDAK] | - |

---

## 4. Analisis

### 4.1 Linearity

- [ ] Linear (pulse ∝ volume)
- [ ] Non-linear (butuh lookup table)

### 4.2 Accuracy

| vs Datasheet | Hasil |
|--------------|-------|
| K_pulse Error | [ISI] % |
| Dalam toleransi ±2%? | [YA/TIDAK] |

### 4.3 Precision

| Parameter | Hasil | Keterangan |
|-----------|-------|------------|
| Std Dev | [ISI] mL | < 5% = bagus |
| CV | [ISI] % | < 10% = acceptable |

### 4.4 Signal Quality

- [ ] Sinyal bersih (tidak ada bounce)
- [ ] Ada bounce (perlu debounce)
- [ ] Noise tinggi (perlu filter)

---

## 5. Keputusan: Signal Conditioning yang Diperlukan

Berdasarkan hasil karakteristik:

| Jenis Signal Conditioning | Diperlukan? | Implementasi |
|---------------------------|-------------|--------------|
| Debounce | [YA/TIDAK] | [Jika YA: software/hardware] |
| Moving Average Filter | [YA/TIDAK] | [Jika YA: berapa window?] |
| Scale Correction | [YA/TIDAK] | [Jika YA: faktor berapa?] |
| Offset Correction | [YA/TIDAK] | [Jika YA: berapa?] |
| Lookup Table | [YA/TIDAK] | [Jika YA: range apa?] |

---

## 6. Kesimpulan

| Item | Verdict |
|------|---------|
| Fungsi sensor | [NORMAL/RUSAK] |
| Linearity | [LINEAR/NON-LINEAR] |
| Accuracy vs Datasheet | [MEMENUHI/TIDAK] |
| Precision (Repeatability) | [BAGUS/KURANG] |
| Signal Quality | [BERSIH/NOISY] |
| **Perlu Kalibrasi?** | **[YA/TIDAK]** |

---

## 7. Rekomendasi

### Jika TIDAK PERLU Kalibrasi:

- Pakai nilai datasheet: K_pulse = 98.0
- Sensor siap digunakan untuk Slave 1

### Jika PERLU Kalibrasi:

1. **Scale Correction:**
   - K_pulse aktual = [ISI]
   - Update code Slave 1: `volume_mL = pulse_count * ([ISI])`

2. **Filter (jika perlu):**
   - [ISI detail implementasi]

3. **Update B300:**
   - Ganti nilai faktor_kalibrasi di dokumen B300

---

## 8. Lampiran

- Test code: `yfs401_karakteristik.ino`
- Raw data: `template_data.csv` (terisi)
- Foto setup: *(optional)*
- Datasheet: YFS401 Manufacturer

---

**Dibuat oleh:** [ISI NAMA]
**Disetujui:** *(pending verification)*
