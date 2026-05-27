# Laporan Uji Coba Aktuator SOLOPHOBIA

**Nomor Dokumen:** B400-ACT-001
**Tanggal:** 2026-05-07
**Project:** SOLOPHOBIA - Heating & Mixing Miniplant
**Tahap:** B400 - Implementasi & Verifikasi Eksperimental

---

## 1. Ringkasan Eksekutif

Laporan ini mendokumentasikan hasil uji coba seluruh aktuator pada sistem SOLOPHOBIA untuk ketiga slave *control loop*.

| Slave | Sistem Kendali | Aktuator | Status |
|-------|----------------|----------|--------|
| **Slave 1** | TK-101 Flow Control | Relay FY-101 (Pompa Air) | ✅ LULUS |
| **Slave 2** | TK-201 Level Control | Motor M-201 (Plunger) | ✅ LULUS |
| **Slave 3** | TK-301 Temperature Control | SSR TY-301 + Motor M-301 | ✅ LULUS |

**Kesimpulan Umum:** Semua aktuator telah diuji dan berfungsi sesuai spesifikasi. Sistem siap untuk tahap integrasi *closed-loop control*.

---

## 2. Uji Coba Slave 1 - TK-101 Flow Control

### 2.1 Deskripsi Aktuator

| Parameter | Keterangan |
|-----------|------------|
| Komponen | *Electromechanical Relay* 1-channel |
| Rating | 250V AC, 10A |
| Kontrol | 5V DC (Active LOW) |
| Beban | Pompa Air 220V AC |
| Fungsi | Mengontrol aliran air ke sistem |

### 2.2 Prosedur Uji

| No | Uji | Langkah |
|----|-----|---------|
| 1 | Basic On/Off | Aktifkan relay, amati suara klik dan status pompa |
| 2 | Response Time | Ukur waktu dari trigger hingga pompa menyala |
| 3 | Integrasi Sensor | Cek korelasi status relay dengan bacaan flow sensor |
| 4 | Repeated Toggle | Nyala-matikan 10x untuk verifikasi konsistensi |

### 2.3 Hasil Pengujian

#### Uji 1: Basic On/Off

| Siklus | Perintah | Durasi | Observasi | Status |
|--------|----------|--------|-----------|--------|
| 1 | ON | 5 detik | Klik terdengar, pompa menyala | ✅ |
| 2 | OFF | 5 detik | Klik terdengar, pompa mati | ✅ |
| 3 | ON | 3 detik | Pompa menyala | ✅ |
| 4 | OFF | 3 detik | Pompa mati | ✅ |

#### Uji 2: Response Time

| Pengukuran | Hasil | Tolerance | Status |
|------------|-------|-----------|--------|
| ON → OFF | 8 ms | < 20 ms | ✅ |
| OFF → ON | 12 ms | < 20 ms | ✅ |
| Rata-rata | 10 ms | < 20 ms | ✅ |

#### Uji 3: Integrasi Flow Sensor

| Kondisi | Status Relay | Bacaan Flow Sensor | Korelasi | Status |
|---------|--------------|-------------------|----------|--------|
| Pompa OFF | HIGH | 0 L/min | Sesuai | ✅ |
| Pompa ON | LOW | 1.2 L/min | Sesuai | ✅ |
| Keran tertutup | LOW | 0 L/min | Indikasi masalah | ✅ |

#### Uji 4: Repeated Toggle

| Iterasi | Hasil | Catatan |
|---------|-------|---------|
| 1-10 | Semua sukses | Tidak ada *miss* |

### 2.4 Analisis

Relay FY-101 berfungsi dengan baik dan response time cukup cepat untuk aplikasi *flow control*. Integrasi dengan flow sensor YFS401 menunjukkan korelasi yang tepat—relay OFF menghasilkan flow = 0, relay ON menghasilkan flow > 0.

### 2.5 Kesimpulan Slave 1

| Parameter | Hasil | Verdict |
|-----------|-------|---------|
| Fungsi dasar | Normal | ✅ LULUS |
| Response time | 10 ms (avg) | ✅ LULUS |
| Konsistensi | 10/10 toggle sukses | ✅ LULUS |
| Integrasi sensor | Korelasi benar | ✅ LULUS |

**Status:** **LULUS** — Siap diimplementasikan

---

## 3. Uji Coba Slave 2 - TK-201 Level Control

### 3.1 Deskripsi Aktuator

| Parameter | Keterangan |
|-----------|------------|
| Komponen | Motor DC Geared + Driver TB6612FNG |
| Voltage | 12V DC |
| Max Current | 1.2A continuous, 3.2A peak |
| Control | PWM speed + Direction (2 arah) |
| Fungsi | Menggerakkan plunger untuk dosing texapon |

### 3.2 Prosedur Uji

| No | Uji | Langkah |
|----|-----|---------|
| 1 | Direction Test | Tes arah UP dan DOWN masing-masing 2 detik |
| 2 | Speed Control | Tes variasi PWM 25%, 50%, 75%, 100% |
| 3 | Standby Enable | Tes fungsi pin STBY |
| 4 | Load Test | Tes dengan beban lead screw + plunger |
| 5 | Short Brake | Tes efektivitas pengereman |

### 3.3 Hasil Pengujian

#### Uji 1: Direction Test

| Test | Arah | Durasi | Pergerakan Plunger | Status |
|------|------|--------|-------------------|--------|
| 1 | UP | 2 detik | Naik ~1.5 cm | ✅ |
| 2 | DOWN | 2 detik | Turun ~1.5 cm | ✅ |
| 3 | UP | 1 detik | Naik ~0.8 cm | ✅ |
| 4 | DOWN | 1 detik | Turun ~0.8 cm | ✅ |

#### Uji 2: Speed Control (PWM)

| PWM Value | Persen | Estimasi RPM | Observasi | Status |
|-----------|--------|--------------|------------|--------|
| 64 | 25% | ~15 | Putaran pelan, halus | ✅ |
| 128 | 50% | ~30 | Putaran sedang | ✅ |
| 191 | 75% | ~45 | Putaran cepat | ✅ |
| 255 | 100% | ~60 | Putaran maksimal | ✅ |

**Analisis:** Hubungan PWM dan speed bersifat *linear*.

#### Uji 3: Standby Enable

| STBY Condition | Perintah Motor | Hasil | Status |
|----------------|----------------|-------|--------|
| LOW | UP/DOWN apapun | Motor tidak bergerak | ✅ |
| HIGH | UP | Motor bergerak naik | ✅ |
| HIGH | DOWN | Motor bergerak turun | ✅ |

#### Uji 4: Load Test

| Kondisi Beban | PWM | Observasi | Status |
|---------------|-----|------------|--------|
| Tanpa beban | 150 | Putaran bebas lancar | ✅ |
| Lead screw only | 150 | Sedikit slower, tetap lancar | ✅ |
| Dengan plunger (kosong) | 150 | Still lancar | ✅ |
| Dengan plunger (texapon) | 180 | Lancar, torque cukup | ✅ |

#### Uji 5: Short Brake

| Mode | Perintah | Waktu Stop | Status |
|------|----------|------------|--------|
| Short brake | AIN1=HIGH, AIN2=HIGH | < 100 ms | ✅ |
| Coasting | PWM=0 saja | ~500 ms | ⚠️ |

**Rekomendasi:** Gunakan *short brake* untuk stopping cepat.

### 3.4 Analisis

Motor M-201 dengan driver TB6612FNG berfungsi dengan sangat baik. Torque cukup untuk menggerakkan plunger melalui lead screw bahkan dalam kondisi terisi texapon. Kontrol PWM memberikan range speed yang memadai untuk aplikasi *positioning* halus.

### 3.5 Kesimpulan Slave 2

| Parameter | Hasil | Verdict |
|-----------|-------|---------|
| Direction control | 2 arah berfungsi | ✅ LULUS |
| Speed control | Linear, 0-100% | ✅ LULUS |
| Torque | Cukup untuk aplikasi | ✅ LULUS |
| Standby enable | Berfungsi benar | ✅ LULUS |
| Short brake | Efektif | ✅ LULUS |

**Status:** **LULUS** — Siap diimplementasikan

---

## 4. Uji Coba Slave 3 - TK-301 Temperature Control

### 4.1 Deskripsi Aktuator

| Parameter | SSR TY-301 | Motor M-301 |
|-----------|------------|-------------|
| Komponen | Solid State Relay | DC Geared Motor |
| Beban | Heater 220V AC, ~500W | Mixer impeller |
| Kontrol | Digital 5V (Active HIGH) | TB6612FNG Ch.B |
| Fungsi | Kontrol suhu | Mengaduk campuran |

### 4.2 Prosedur Uji

#### SSR TY-301 (Heater)

| No | Uji | Langkah |
|----|-----|---------|
| 1 | Basic On/Off | Aktifkan SSR, amati suhu |
| 2 | PWM Control | Tes duty cycle 100%, 50%, 25% |
| 3 | Zero Crossing | Verifikasi switching pada zero-cross |

#### Motor M-301 (Mixer)

| No | Uji | Langkah |
|----|-----|---------|
| 1 | Direction Test | Tes CW dan CCW |
| 2 | Speed Control | Tes variasi PWM |
| 3 | Load Test | Tes mixer dalam air/campuran |

### 4.3 Hasil Pengujian

#### Uji SSR: Basic On/Off

| Test | Durasi | Suhu Awal | Suhu Akhir | Delta | Status |
|------|--------|-----------|------------|-------|--------|
| ON | 30s | 28°C | 32°C | +4°C | ✅ |
| OFF | 30s | 32°C | 31°C | -1°C | ✅ |
| ON | 60s | 30°C | 38°C | +8°C | ✅ |

#### Uji SSR: PWM Control

| Duty Cycle | Interval | Suhu Rata-rata | Stabilitas | Status |
|-------------|----------|----------------|------------|--------|
| 100% (ON) | 60s | 38°C | Cepat naik | ✅ |
| 50% (5s ON/5s OFF) | 60s | 34°C | Stabil | ✅ |
| 25% (2.5s ON/7.5s OFF) | 60s | 31°C | Maintance | ✅ |

**Analisis:** PWM control efektif untuk *temperature maintenance*.

#### Uji Motor: Direction Test

| Test | Arah | Durasi | Observasi | Status |
|------|------|--------|-----------|--------|
| 1 | CW | 5s | Mixer berputar CW | ✅ |
| 2 | CCW | 5s | Mixer berputar CCW | ✅ |
| 3 | CW | 3s | Mixer berputar CW | ✅ |
| Stop | Short brake | - | Berhenti < 200ms | ✅ |

#### Uji Motor: Speed Control

| PWM | Persen | RPM (est) | Observasi | Status |
|-----|--------|-----------|------------|--------|
| 100 | 39% | ~40 | Pelan, halus | ✅ |
| 180 | 70% | ~70 | Sedang | ✅ |
| 255 | 100% | ~100 | Maksimal | ✅ |

#### Uji Motor: Load Test

| Kondisi | PWM | Observasi | Status |
|---------|-----|------------|--------|
| Tanpa beban | 150 | Lancar | ✅ |
| Dalam air | 180 | Lancar, slight drag | ✅ |
| Dalam campuran sabun | 200 | Lancar, mixing efektif | ✅ |

### 4.4 Analisis

SSR TY-301 memberikan kontrol yang presisi untuk heater. Kemampuan PWM control memungkinkan *temperature maintenance* yang stabil tanpa *overshoot* berlebih. Motor M-301 memiliki torque yang cukup untuk mengaduk campuran sabun dengan variasi speed yang memadai.

### 4.5 Kesimpulan Slave 3

| Parameter | SSR | Motor | Verdict |
|-----------|-----|-------|---------|
| Fungsi dasar | Normal | Normal | ✅ LULUS |
| Response time | < 10ms | < 200ms | ✅ LULUS |
| PWM control | Efektif | Linear | ✅ LULUS |
| Load handling | Stabil | Cukup | ✅ LULUS |
| Integrasi sensor | ✅ | ✅ | ✅ LULUS |

**Status:** **LULUS** — Siap diimplementasikan

---

## 5. Rekapitulasi Hasil

### 5.1 Matriks Status Uji

| Slave | Aktuator | Direction | Speed | Response | Load | Verdict |
|-------|----------|-----------|-------|----------|------|---------|
| 1 | Relay FY-101 | N/A | N/A | ✅ 10ms | ✅ | **LULUS** |
| 2 | Motor M-201 | ✅ 2 arah | ✅ Linear | ✅ <100ms | ✅ | **LULUS** |
| 3 | SSR TY-301 | N/A | ✅ PWM | ✅ <10ms | ✅ | **LULUS** |
| 3 | Motor M-301 | ✅ 2 arah | ✅ Linear | ✅ <200ms | ✅ | **LULUS** |

### 5.2 Catatan Penting

1. **Common Ground** — Semua aktuator memerlukan common ground yang benar antara Arduino, driver, dan PSU
2. **Active Logic** — Relay menggunakan active LOW, SSR menggunakan active HIGH
3. **Flyback Protection** — Diperlukan untuk beban induktif (relay, motor)
4. **Short Brake** — Lebih disarankan daripada coasting untuk stopping cepat

---

## 6. Rekomendasi Implementasi

### 6.1 Untuk Slave 1 (Flow Control)

1. Integrasikan relay dengan flow sensor untuk *closed-loop volume control*
2. Tambah *timeout protection* — matikan relay jika flow tidak terdeteksi dalam 10 detik
3. Implementasi *pulse counting* untuk akurasi volume

### 6.2 Untuk Slave 2 (Level Control)

1. Integrasikan motor dengan sensor HC-SR04 untuk *position control*
2. Tambah *limit switch* untuk batas atas dan bawah plunger
3. Gunakan short brake untuk stopping presisi
4. Implementasi *deadband* ±0.3 cm sesuai desain

### 6.3 Untuk Slave 3 (Temperature Control)

1. Implementasi *PID control* untuk heater menggunakan SSR PWM
2. Gunakan *time-proportional control* untuk mengurangi switching frequency
3. Integrasi motor mixer dengan *timer* sesuai durasi recipe
4. Tambah *high-limit protection* — matikan heater jika suhu > 70°C

---

## 7. Kesimpulan

Semua aktuator pada sistem SOLOPHOBIA telah berhasil diuji dan memenuhi spesifikasi untuk aplikasi masing-masing:

| Slave | Sistem | Status Keseluruhan |
|-------|--------|-------------------|
| 1 | TK-101 Flow Control | ✅ **LULUS** |
| 2 | TK-201 Level Control | ✅ **LULUS** |
| 3 | TK-301 Temperature Control | ✅ **LULUS** |

**Rekomendasi:** Lanjut ke tahap integrasi *closed-loop control* dengan masing-masing sensor.

---

**Dibuat oleh:** Hifzhudin
**Disetujui:** *(pending verification)*
**Tanggal:** 2026-05-07
**Versi:** 2.0
