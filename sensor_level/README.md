# Kalibrasi HC-SR04 - Slave 2 TK-201

## Tujuan

Menentukan karakteristik sensor HC-SR04 untuk memutuskan apakah perlu *signal conditioning* atau tidak.

---

## Persiapan Hardware

### Komponen
- HC-SR04 ultrasonic sensor
- Arduino Uno/Nano
- Kabel jumper
- Solid target (karton/kertas putih, minimal 15x15 cm)
- Penggaris/ruler (minimal 50 cm)

### Wiring

| HC-SR04 Pin | Arduino Pin |
|-------------|-------------|
| VCC | 5V |
| GND | GND |
| TRIG | D9 |
| ECHO | D10 |

---

## Prosedur Kalibrasi

### 1. Setup Fisik

```
┌─────────────────────────────────────┐
│                                     │
│         HC-SR04 (FIX)               │
│            ↓↓↓                      │
│                                     │
│    ┌─────────────────────────┐      │
│    │                         │      │
│    │   TARGET (karton)       │ ←───┼─── Geser sesuai jarak
│    │                         │      │     yang diukur penggaris
│    └─────────────────────────┘      │
│                                     │
│    ←───────────→                    │
│      Jarak Referensi                │
│      (ukur dengan ruler)            │
└─────────────────────────────────────┘
```

**Langkah:**
1. Mount HC-SR04 di posisi fix (bisa dengan holder atau gboard)
2. Letakkan target solid SEJAJAR dengan sensor
3. Pastikan target tegak lurus dengan arah beam sensor
4. Siapkan penggaris untuk mengukur jarak referensi

**Tips:**
- Target warna putih matte (bukan mengkilap) untuk hasil terbaik
- Pastikan tidak ada objek lain di sekitar beam path
- Sensor dan target harus sejajar (tidak miring)

### 2. Upload Sketch

1. Buka Arduino IDE
2. Load `hcsr04_karakteristik.ino`
3. Pilih board dan port yang sesuai
4. Upload

### 3. Serial Monitor Setup

- Buka Serial Monitor (Ctrl+Shift+M)
- Set baudrate: **9600**
- Set line ending: **No line ending**

### 4. Pengambilan Data

#### Mode Single Reading (Untuk presisi)

1. Geser target ke jarak referensi pertama (misal: 5 cm)
2. Ukur dengan penggaris dari FACE sensor ke permukaan target
3. Kirim karakter `1` ke Serial Monitor
4. Catat pembacaan di template_data.csv
5. Ulangi 10x untuk jarak yang sama
6. Geser ke jarak berikutnya (increment 2.5 cm atau 5 cm)

#### Mode Continuous (Untuk quick check)

1. Kirim karakter `c`
2. Sensor akan membaca terus-menerus setiap 100ms
3. Geser target dan lihat respon real-time
4. Tekan `s` untuk stop

#### Mode Test Repeatability

1. Letakkan target di jarak tertentu
2. Kirim karakter `t`
3. Serial akan output 10 pembacaan + statistik (mean, min, max, range)

### 5. Range Pengukuran

**Rekomendasi jarak referensi:**

| Jarak (cm) | Catatan |
|------------|---------|
| 5 - 10 | Dekat (batas bawah) |
| 10 - 30 | Range optimal |
| 30 - 50 | Jauh (cek linearity) |

**Increment:** 2.5 cm atau 5 cm (sesuai preferensi)

---

## Analisis Data

### 1. Hitung Error

```
Error = Jarak_Sensor - Jarak_Referensi
```

Untuk setiap titik, hitung:
- **Mean error** → bias sensor
- **Std dev** → presisi
- **Min/Max** → variasi

### 2. Linearity Check

Plot **Error vs Jarak Referensi**:
- Jika garis lurus horizontal → sensor linear
- Jika ada pola → perlu koreksi/linearization

### 3. Precision Check

Lihat **Std Dev** di setiap jarak:
- Std dev kecil (< 0.5 cm) → presisi bagus
- Std dev besar (> 1 cm) → ada noise, mungkin perlu filter

---

## Kesimpulan

### Apakah perlu signal conditioning?

| Kondisi | Kesimpulan |
|---------|------------|
| Mean error < 0.5 cm, std dev < 0.3 cm | **Tidak perlu** - sensor siap pakai |
| Mean error > 1 cm (konstan) | **Perlu offset correction** (di software) |
| Std dev > 0.5 cm | **Perlu filter** (moving average / median) |
| Error non-linear terhadap jarak | **Perlu linearization table** |

### Jenis Signal Conditioning (jika diperlukan)

1. **Software** (disarankan dulu):
   - Offset correction
   - Moving average filter
   - Linearization table

2. **Hardware** (jika software tidak cukup):
   - Low-pass filter analog (jika output analog)
   - Shielding dari noise EMI
   - Voltage regulator stabil untuk sensor

---

## Catatan Penting

### Limitasi HC-SR04

- **Minimum distance:** 2 cm (blind spot)
- **Maximum distance:** 400 cm (teoritis, ~200 cm praktis)
- **Beam angle:** ~15 derajat
- **Refresh rate:** maks 20 Hz

### Sumber Error

1. **Target surface:** Warna gelap/mengkilap mengurangi akurasi
2. **Temperature:** Kecepatan suara berubah (0.6% per 10°C)
3. **Humidity:** Sedikit mempengaruhi kecepatan suara
4. **Alignment:** Target tidak tegak lurus
5. **Noise:** EMI dari motor/switching supply

---

## Contoh Hasil (Referensi)

```
Jarak Ref | Mean Sensor | Error | Std Dev
---------|-------------|-------|--------
5.0 cm   | 5.2 cm      | +0.2  | 0.08
10.0 cm  | 10.1 cm     | +0.1  | 0.05
15.0 cm  | 15.3 cm     | +0.3  | 0.12
20.0 cm  | 20.2 cm     | +0.2  | 0.15
25.0 cm  | 25.4 cm     | +0.4  | 0.18
```

**Analisis:**
- Mean error rata-rata: +0.24 cm (positif bias)
- Std dev maksimal: 0.18 cm (presisi acceptable)
- **Kesimpulan:** Sensor linear, perlu offset correction -0.24 cm di software

---

## Troubleshooting

### Masalah: Pembacaan "ERROR,OUT_OF_RANGE"
- Target terlalu dekat (< 2 cm) atau terlalu jauh (> 200 cm)
- Target tidak reflektif (warna gelap/kasar)
- Kabel ECHO putus/loose

### Masalah: Pembacaan tidak stabil (fluktuasi besar)
- Cek koneksi kabel
- Jauhkan dari sumber noise (motor, switching adapter)
- Pastikan target fix (tidak goyang)
- Gunakan mode moving average (default aktif)

### Masalah: Error linear terhadap jarak
- Bisa jadi faktor kecepatan suara (cek temperature)
- Atur konstanta `KECEPATAN_SUARA` jika perlu:
  - 343 m/s pada 20°C
  - 349 m/s pada 30°C

---

## File Terkait

- `hcsr04_karakteristik.ino` - Sketch Arduino
- `template_data.csv` - Template record data
- `perhitungan_volume_tk201.md` - Konversi jarak ke volume
- `sketsa_teknis_tk201.md` - Desain tangki TK-201
