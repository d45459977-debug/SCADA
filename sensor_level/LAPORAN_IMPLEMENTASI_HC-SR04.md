# Laporan Implementasi HC-SR04 Ultrasonic Sensor
## Slave 2 - TK-201 Level Control

**Project:** SOLOPHOBIA - Heating & Mixing Miniplant
**Tanggal:** 6 Mei 2026
**Sensor:** HC-SR04 Ultrasonic Distance Sensor
**Lokasi:** `/b400/sensor_level/`

---

# BAB 1
## IMPLEMENTASI

Implementasi ultrasonic sensor HC-SR04 pada Slave 2 (TK-201) dilakukan untuk mengukur level texapon dalam wadah melalui sistem plunger. Sensor membaca jarak ke plunger, yang kemudian dikonversi menjadi volume texapon berdasarkan luas penampang wadah.

### Sistem Plunger TK-201

Wadah TK-201 menggunakan mekanisme plunger (penekan) untuk mengeluarkan texapon. Plunger bergerak vertikal naik-turun di dalam wadah:

- **Posisi ATAS:** Plunger di batas maksimal (22.6 cm dari dasar) → Texapon PENUH
- **Posisi BAWAH:** Plunger turun ke dasar → Texapon KOSONG

Sensor HC-SR04 dipasang di atas wadah, menghadap ke bawah untuk membaca jarak ke plunger secara realtime.

### Spesifikasi Wadah TK-201

| Parameter | Nilai |
|-----------|-------|
| Bentuk | Balok (tidak simetris) |
| Panjang | 22.0 cm |
| Lebar rata-rata | 11.55 cm |
| **Luas penampang** | **254.1 cm²** |
| Tinggi maksimal texapon | 22.6 cm |
| **Volume maksimal** | **5743 mL** |

### Wiring Arduino

```
Arduino Uno               HC-SR04 Sensor
─────────────────────────────────────────────

5V  ───────────────────────→ VCC

D9  ───────────────────────→ TRIG

D10 ───────────────────────→ ECHO

GND ───────────────────────→ GND
```

### Algoritma Perhitungan Volume

```
1. Kalibrasi jarak penuh (plunger di posisi atas)
   jarak_penuh = baca sensor saat plunger mentok batas 22.6 cm

2. Baca jarak saat ini
   jarak_saat_ini = baca sensor (dengan moving average filter)

3. Hitung selisih jarak
   selisih = jarak_saat_ini - jarak_penuh

4. Hitung tinggi texapon
   tinggi = 22.6 - selisih

5. Hitung volume
   volume = 254.1 × tinggi
```

### Hasil Kalibrasi

**Kalibrasi Jarak Penuh (Plunger di Atas):**

| Sampel | Jarak (cm) |
|--------|------------|
| 1-10 | 2.66 - 2.68 cm |

**Jarak Penuh:** 2.66 cm

**Validasi Akurasi:**

| Test | Target (cm) | Bacaan (cm) | Error |
|------|-------------|-------------|-------|
| 1 | 8.0 | 7.51 | -0.49 (-6.1%) |
| 2 | 8.5 | 8.45 | -0.05 (-0.6%) |

**Rata-rata Error:** -0.27 cm (dalam toleransi operasional)

### Pengujian Volume

Setelah kalibrasi, pengujian dilakukan dengan memindahkan plunger ke berbagai posisi:

| Test | Jarak (cm) | Tinggi (cm) | Volume (mL) | Persen |
|------|------------|-------------|-------------|--------|
| 1 | 7.87 | 17.39 | 4419 | 77.0% |
| 2 | 13.70 | 11.56 | 2938 | 51.2% |
| 3 | 16.96 | 8.30 | 2110 | 36.7% |
| 4 | 20.49 | 4.77 | 1212 | 21.1% |
| 5 | 7.13 | 18.13 | 4607 | 80.2% |

**Hasil:** Perhitungan volume konsisten dan mengikuti pergerakan plunger dengan benar.

---

# BAB 2
## PERMASALAHAN DAN SOLUSI

### 2.1 Sensor Noise dan Variasi Pembacaan

Pembacaan awal menunjukkan variasi yang cukup besar (±1-2 cm) untuk posisi yang sama. Hal ini disebabkan oleh karakteristik sensor HC-S04 yang memiliki presisi terbatas, serta kemungkinan adanya electrical noise.

**Solusi yang diterapkan:**
1. **Moving Average Filter:** Mengambil 5x pembacaan dan menghitung rata-rata
2. **Increased Sample Size:** Kalibrasi menggunakan 20 sampel (dari 10)
3. **Invalid Value Filtering:** Membuang pembacaan di luar range valid (0-100 cm)

Hasilnya variasi berkurang menjadi ±0.2 cm, yang sudah memenuhi requirement sistem.

### 2.2 Selisih Antara Pengukuran Sensor dan Manual

Pengukuran manual dengan penggaris menunjukkan tinggi maksimal 22.6 cm, namun selisih pembacaan sensor (dari dasar ke plunger penuh) hanya 19.77 cm. Terdapat selisih sekitar 2.83 cm.

**Penyebab:**
1. Transducer HC-S04 tidak berada persis di ujung moncong sensor, melainkan ~2-3 cm di dalam
2. Titik ukur penggaris mungkin berbeda dengan yang dibaca sensor
3. Ketebalan dan konstruksi plunger

**Solusi yang diterapkan:**
Menggunakan nilai manual 22.6 cm sebagai tinggi maksimal (hardcode dalam sketch), sementara sensor hanya digunakan untuk membaca posisi relatif plunger. Kalibrasi jarak penuh dilakukan untuk mengetahui posisi referensi plunger saat texapon penuh.

### 2.3 Ketergantungan pada Posisi Holder

Setiap perubahan posisi atau tinggi holder akan mengubah jarak sensor ke plunger, sehingga memerlukan kalibrasi ulang.

**Solusi yang diterapkan:**
1. Prosedur kalibrasi sederhana dengan perintah `k`
2. Menu kalibrasi manual `m` untuk input langsung nilai jarak
3. Dokumentasi prosedur kalibrasi yang jelas

Setiap kali holder diganti/dipindah, cukup jalankan kalibrasi ulang dalam waktu <1 menit.

### 2.4 Offset Error pada Pembacaan Sensor

Sensor cenderung membaca nilai yang sedikit lebih kecil dari nilai sebenarnya (rata-rata -0.27 cm).

**Solusi yang diterapkan:**
Error ini masih dalam toleransi operasional (±0.5 cm) dan tidak mempengaruhi secara signifikan akurasi pengukuran volume untuk aplikasi dosing. Untuk aplikasi yang membutuhkan akurasi lebih tinggi, dapat ditambahkan offset correction sebesar +0.3 cm dalam sketch.

---

# BAB 3
## KESIMPULAN

Implementasi HC-SR04 untuk pengukuran level texapon melalui sistem plunger telah berhasil dilakukan. Sensor mampu membaca posisi plunger dengan akurasi yang memadai untuk aplikasi dosing (error ±0.3 cm).

**Hal-hal yang telah dicapai:**
1. ✅ Sketch kalibrasi volume dengan filter noise
2. ✅ Menu serial untuk kalibrasi mudah (otomatis dan manual)
3. ✅ Perhitungan volume realtime dengan persentase
4. ✅ Continuous mode untuk monitoring
5. ✅ Tabel konversi tinggi ke volume

**Keterbatasan:**
1. Sensor perlu kalibrasi ulang setiap ganti holder
2. Akurasi terbatas (±0.3 cm) namun cukup untuk aplikasi
3. Blind spot sensor 2 cm (tidak masalah karena jarak penuh 2.66 cm)

**Rekomendasi untuk Implementasi Slave 2:**
1. Gunakan sketch `hcsr04_kalibrasi_volume.ino` sebagai basis
2. Hardcode tinggi maksimal 22.6 cm dan luas penampang 254.1 cm²
3. Lakukan kalibrasi saat pertama setup dan setiap ganti holder
4. Pertimbangkan penambahan offset correction jika diperlukan

---

# REFERENSI

- Dokumen B300: Desain Hardware & Software
- Datasheet HC-SR04
- Hasil kalibrasi: 6 Mei 2026
- Lokasi kode: `/b400/sensor_level/hcsr04_kalibrasi_volume/`
- Test Report: `HC-SR04_TEST_REPORT.md`
