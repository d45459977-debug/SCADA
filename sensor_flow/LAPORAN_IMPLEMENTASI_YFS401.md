# Laporan Implementasi YF-S401 Flow Sensor
## Slave 1 - TK-101 Flow Control

**Project:** SOLOPHOBIA - Heating & Mixing Miniplant
**Tanggal:** 3 Mei 2026
**Sensor:** YF-S401 Hall Effect Flow Sensor
**Lokasi:** `/b400/sensor_flow/`

---

# BAB 1
## IMPLEMENTASI

Implementasi flow sensor YF-S401 pada Slave 1 (TK-101) dilakukan untuk mengukur volume air yang masuk ke tangki reaktor dan mengendalikan pompa P-101 berdasarkan setpoint volume.

### Setup Power Supply

Supply tegangan 5V yang stabil diperlukan untuk akurasi sensor. Penggunaan pompa submersible 5V sebagai sumber tegangan menyebabkan data sensor tidak stabil. Solusi yang diterapkan adalah menggunakan Arduino 2 sebagai sumber 5V stabil dari USB laptop, sedangkan Arduino 1 berfungsi sebagai kontrol relay dan pembaca pulse.

```
Laptop → USB → Arduino 2 → 5V → YF-S401 VCC
                              ↓
                          Pompa 5V
                              ↓
Arduino 1 → Relay → Kontrol ON/OFF pompa
YF-S401 Signal → D2 Arduino 1 (Interrupt)
```

### Hasil Kalibrasi Volumetrik

Kalibrasi dilakukan dengan metode volumetrik menggunakan volume referensi 200 mL. Prosedur kalibrasi meliputi priming selang hingga tidak ada gelembung, reset pulse counter, pengukuran volume 200 mL, dan pencatatan total pulse.

**Data Kalibrasi:**
- Volume referensi: 200 mL
- Waktu pengisian: 37.63 detik
- Total pulse: 39.263 pulse
- Flow rate aktual: 318.8 mL/menit

**Faktor Kalibrasi:**

| Parameter | Nilai Datasheet | Nilai Kalibrasi |
|-----------|-----------------|-----------------|
| mL per pulse | 10.2 | **0.0053** |
| Pulse per Liter | 98 | **188.150** |
| Akurasi | - | **±4%** |

Faktor kalibrasi 0.0053 mL/pulse diperoleh dari perhitungan volume aktual terhadap pulse count. Faktor ini spesifik untuk kondisi pompa 5V, flow rate konstan ~319 mL/menit, setup selang yang sama, dan kondisi selang penuh (primed). Perubahan pada salah satu parameter memerlukan kalibrasi ulang.

---

# BAB 2
## PERMASALAHAN DAN SOLUSI

### 2.1 Data Karakteristik Tidak Stabil

Data pembacaan sensor sangat fluktuatif dengan pulse count mencapai 1000+ pulse/detik dan volume terbaca jauh dari target. Penyebab utama adalah supply tegangan tidak stabil dari pompa submersible 5V dan flowrate yang terlalu tinggi di luar range sensor (0.3-6 L/menit).

Solusi yang diterapkan adalah stabilisasi supply tegangan menggunakan Arduino 2 sebagai PSU 5V stabil dari USB laptop, serta kalibrasi volumetrik untuk mendapatkan faktor koreksi empiris. Hasil yang diperoleh adalah faktor kalibrasi 0.0053 mL/pulse dengan akurasi ±4% pada flow rate aktual 318.8 mL/menit.

### 2.2 Pulse Count Tidak Sesuai Datasheet

Pulse per L dari hasil pengukuran (~188.000) jauh lebih besar dibandingkan datasheet (98.000). Hal ini disebabkan oleh contact bouncing pada optocoupler hall effect, sensor yang terlalu sensitif mendeteksi kedua edge, serta flow rate tinggi yang menyebabkan frekuensi pulse tinggi.

Solusi yang diterapkan adalah kalibrasi empiris dengan mengukur volume referensi menggunakan gelas ukur, menghitung faktor koreksi, dan menggunakannya dalam implementasi kode. Metode kalibrasi memberikan hasil dengan error ±4% dibandingkan error sangat besar jika menggunakan nilai datasheet.

### 2.3 Dead Volume di Selang

Selang dari pompa ke sensor (1m) dan dari sensor ke output (1m) mengandung volume air yang mempengaruhi akurasi pengukuran. Pada kondisi selang kosong, pulse terhitung sejak air lewat sensor namun air baru keluar setelah selang penuh, sehingga volume terbaca tidak sama dengan volume aktual di gelas.

Solusi yang diterapkan adalah priming sebelum kalibrasi atau operasi dengan menyalakan pompa hingga tidak ada gelembung di selang, kemudian baru memulai penghitungan volume. Dead volume untuk selang 6mm ID dengan panjang 2m adalah sekitar 56.5 mL.

### 2.4 Sensor Bouncing dan Noise

Sensor mendeteksi 6 pulse dalam 10 detik meskipun tidak ada aliran (noise). Penyebabnya adalah debu atau kotoran pada optocoupler, electrical noise dari kabel, dan EMI dari pompa.

Solusi yang dapat diterapkan adalah filtering di software dengan debounce filter (minimum 10ms antar pulse) atau filtering di hardware dengan penambahan capacitor 0.1µF pada pin signal, penggunaan kabel shielded, dan pemisahan kabel signal dari kabel power.

### 2.5 Keterbatasan Kalibrasi Volumetrik

Pendekatan kalibrasi volumetrik memiliki akurasi ±4% yang sudah cukup untuk aplikasi mixing dengan flow rate tinggi untuk waktu isi yang lebih cepat. Namun faktor kalibrasi hanya valid untuk kondisi spesifik: pompa yang sama, tekanan air yang sama, dan setup selang yang sama.

Perubahan pada salah satu parameter memerlukan kalibrasi ulang. Untuk pengembangan selanjutnya, pertimbangan penggunaan flow sensor dengan range lebih tinggi atau implementasi safety tambahan seperti timeout protection dan maximum volume limit dapat dilakukan.

---

# REFERENSI

- Dokumen B300: Desain Hardware & Software
- Datasheet YF-S401
- Hasil kalibrasi: 3 Mei 2026
- Lokasi kode: `/b400/sensor_flow/yfs401_terkalibrasi/`
