# Karakterisasi YFS401 - Slave 1 TK-101

## Tujuan

Menentukan karakteristik sensor YFS401 untuk memutuskan:
1. Apakah perlu *signal conditioning* atau tidak
2. Jika perlu, jenis *signal conditioning* apa yang dibutuhkan
3. Nilai K_pulse aktual (pulse per liter) untuk kalibrasi (jika diperlukan)

---

## Persiapan Hardware

### Komponen

| Item | Spesifikasi |
|------|-------------|
| YFS401 Flow Sensor | Hall-effect, 1-5 L/min |
| Arduino Uno/Nano | - |
| Kabel jumper | Male-to-male |
| Wadah ukur | Graduated, minimal 2000 mL (gelas ukur lab) |
| Stopwatch | HP atau stopwatch |
| Sumber air | Keran atau pompa dengan valve |

### Wiring Diagram

```
Arduino Uno               YFS401 Sensor
─────────────────────────────────────────────

5V  ───────────────────────→ RED (VCC)

D2  ───────────────────────→ YELLOW (Signal)
                             (INT0 interrupt)

GND ───────────────────────→ BLACK (GND)
```

| YFS401 Pin | Arduino Pin | Keterangan |
|------------|-------------|------------|
| RED | 5V | VCC (4.5-24V) |
| YELLOW | D2 | Signal output (interrupt) |
| BLACK | GND | Ground |

---

## Prosedur Karakterisasi

### 1. Setup Fisik

```
┌─────────────────────────────────────┐
│                                     │
│   Sumber Air                        │
│      │                              │
│      ▼                              │
│   [VALVE] → atur flow rate         │
│      │                              │
│      ▼                              │
│   ┌─────────┐                       │
│   │ YFS401  │ ← Sensor flow         │
│   └─────────┘                       │
│      │                              │
│      ▼                              │
│   ┌─────────────────┐               │
│   │   WADAH UKUR    │ ← Gelas ukur  │
│   │  (Graduated)    │   graduated   │
│   └─────────────────┘               │
│                                     │
└─────────────────────────────────────┘
```

**Langkah:**
1. Mount YFS401 dengan arah flow sesuai panah di body
2. Pastikan instalasi vertikal atau mendekati vertikal (maksimal kemiringan 5°)
3. Siapkan wadah ukur di outlet sensor
4. Siapkan stopwatch (opsional, code sudah ada timer internal)

**Tips:**
- Pastikan tidak ada kebocoran di koneksi
- Valve harus bisa diatur untuk variasi flow rate
- Wadah ukur harus bersih dan skala terbaca jelas

---

### 2. Upload Sketch

1. Buka Arduino IDE
2. Load `yfs401_karakteristik.ino`
3. Pilih board: Arduino Uno (atau Nano)
4. Pilih port yang sesuai
5. Upload

---

### 3. Serial Monitor Setup

- Buka Serial Monitor (Ctrl+Shift+M)
- Set baudrate: **9600**
- Set line ending: **No line ending**

---

### 4. Pengambilan Data (Karakteristik)

#### Metode A: Fixed Volume (Rekomendasi)

Untuk setiap target volume (100, 250, 500, 1000, 1500 mL):

1. Kirim `z` untuk reset counter
2. Buka valve, arahkan air ke wadah ukur
3. Kirim `c` untuk mulai continuous mode
4. Tutup valve saat volume target tercapai
5. Kirim `s` untuk stop
6. Catat data dari Serial Monitor

**Format output CSV:**
```
VOLUME_ML,PULSE_COUNT,FLOW_ML_MIN,PULSE_PER_L,TIME_MS
102.0,10,612.0,98.0,12345
```

#### Metode B: Single Reading (Per Titik)

1. Atur valve untuk flow rate yang diinginkan
2. Kirim `1` untuk single reading (1 detik)
3. Kumpulkan air ke wadah ukur selama 1 detik
4. Catat volume terkumpul dari wadah
5. Bandingkan dengan output Serial Monitor

**Catatan:** Metode ini kurang akurat untuk volume kecil

#### Metode C: Repeatability Test

Untuk cek presisi di satu flow rate:

1. Atur valve ke posisi tertentu
2. Kirim `t` untuk repeatability test (10x reading)
3. Serial akan output:
   - 10 pembacaan
   - Statistik: mean, min, max, range, std dev, CV%

#### Metode D: Raw Pulse Info (Deteksi Bounce)

Untuk cek kualitas sinyal:

1. Kirim `r` untuk raw pulse info
2. Sensor akan sampling selama 10 detik
3. Output: total pulse, average interval, estimated flow

**Gunakan ini untuk:**
- Deteksi bouncing pulse
- Cek kebersihan sinyal

---

### 5. Range Pengukuran

**Rekomendasi test point:**

| Target Volume | Flow Rate (estimasi) | Trial |
|---------------|---------------------|-------|
| 100 mL | ~1 L/min | 5-10x |
| 250 mL | ~1-2 L/min | 5-10x |
| 500 mL | ~2-3 L/min | 5-10x |
| 1000 mL | ~3-4 L/min | 5-10x |
| 1500 mL | ~4-5 L/min | 5-10x |

**Tips:**
- Lakukan variasi valve opening untuk dapat range flow rate berbeda
- Ulangi setiap titik 5-10x untuk repeatability

---

## Analisis Data

### 1. Hitung K_pulse Aktual

Dari data yang terkumpul:

```
K_pulse_aktual = (Total_Pulse / Volume_Aktual_L) = pulse/L
```

Bandingkan dengan datasheet (98 pulse/L):

```
Error_% = ((K_pulse_aktual - 98) / 98) × 100%
```

### 2. Cek Linearity

Plot **Volume_Aktual (x-axis) vs Pulse_Count (y-axis)**:
- Jika garis lurus melalui origin → sensor linear
- Jika ada kurva → non-linear, perlu lookup table

### 3. Cek Repeatability

Dari repeatability test (`t` command):
- **Std Dev** kecil (< 5% dari mean) → presisi bagus
- **Std Dev** besar (> 10%) → ada noise, perlu filter

### 4. Cek Signal Quality

Dari raw pulse info (`r` command):
- Interval antar pulse konsisten → sinyal bersih
- Interval erratic → ada bounce/noise, perlu debounce/filter

---

## Keputusan Signal Conditioning

Berdasarkan hasil analisis, tentukan:

| Kondisi | Signal Conditioning yang Diperlukan |
|---------|-------------------------------------|
| Error < 5%, Linear, Std Dev kecil | **TIDAK PERLU** - sensor siap pakai |
| Error 5-10%, Konstan | **Scale Correction** - update faktor K_pulse |
| Error non-linear vs volume | **Lookup Table** - koreksi per range |
| Std Dev besar (noisy) | **Moving Average Filter** - di code |
| Bounce pulse terdeteksi | **Debounce** - software atau hardware |
| Offset error | **Offset Correction** - tambah/kurangi nilai |

---

## Hasil Karakteristik

Setelah selesai, isi template data dan buat report:

1. **template_data.csv** - Record semua pengukuran
2. **YFS401_TEST_REPORT.md** - Dokumentasi lengkap hasil

---

## Troubleshooting

### Masalah: Tidak ada pulse terbaca (0)

**Penyebab:**
- Wiring salah
- Pin bukan D2 (untuk INT0)
- Sensor tidak dapat supply voltage

**Solusi:**
- Cek wiring: RED→5V, BLACK→GND, YELLOW→D2
- Cek voltage di pin YELLOW (harus berkedip saat air mengalir)
- Cek arah flow (ada panah di body sensor)

---

### Masalah: Pulse count sangat rendah

**Penyebab:**
- Flow rate terlalu rendah (< 1 L/min adalah min range)
- Ada kabelan/penyumbatan

**Solusi:**
- Tingkatkan flow rate
- Cek ada sumbatan di jalur air

---

### Masalah: Reading tidak stabil (fluktuasi besar)

**Penyebab:**
- Flow rate tidak stabil ( valve goyang)
- Ada bubble di air
- Noise elektrik

**Solusi:**
- Fix posisi valve
- Buang bubble dari sistem
- Tambah moving average filter di code

---

### Masalah: Error sangat besar vs datasheet

**Penyebab:**
- Salah unit atau konversi
- Sensor rusak
- Datasheet tidak akurat untuk batch ini

**Solusi:**
- Cek perhitungan
- Cek dengan sensor lain (jika ada)
- Gunakan nilai K_pulse aktual (abaikan datasheet)

---

## Catatan Penting

### Limitasi YFS401

- **Minimum flow:** ~1 L/min (di bawah ini tidak akurat)
- **Maximum flow:** 5 L/min
- **Orientation:** Install vertikal, kemiringan maksimal 5°
- **Medium:** Air bersih (bukan kental/berminyak)
- **Temperature:** Max 120°C

### Sumber Error

1. **Flow rate tidak stabil** → variasi reading
2. **Bubble/udara** → pulse counting salah
3. **Instalasi miring** → rotor tidak berputar proper
4. **Debris/kotoran** → rotor tersangkut
5. **Voltage drop** → sensor tidak bekerja proper

---

## File Terkait

- `yfs401_karakteristik.ino` - Sketch Arduino
- `template_data.csv` - Template record data
- `YFS401_TEST_REPORT.md` - Template report

---

## Referensi

- Datasheet YFS401: F = 98 × Q (Q = L/min)
- Document ID: B400-SENS-002
