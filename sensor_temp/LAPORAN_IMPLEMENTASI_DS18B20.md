# Laporan Implementasi DS18B20 Temperature Sensor
## Slave 3 - TK-301 Temperature Control

**Project:** SOLOPHOBIA - Heating & Mixing Miniplant
**Tanggal:** 29 April 2026
**Sensor:** DS18B20 Waterproof Probe (Stainless Steel 6x50mm)
**Lokasi:** `/b400/sensor_temp/`

---

# BAB 1
## IMPLEMENTASI

Implementasi sensor suhu DS18B20 pada Slave 3 (TK-301) dilakukan untuk mengukur suhu cairan dalam tangki reaktor sebagai feedback untuk sistem kendali temperature.

### Setup Hardware

Sensor DS18B20 menggunakan protokol OneWire dengan konfigurasi pull-up resistor 1kΩ antara DATA dan VCC. Pin assignment menggunakan D2 sebagai DATA, dengan kabel sensor 1 meter (Red=VCC, Blue=DATA, Black=GND). Library yang digunakan adalah OneWire v2.3.8 dan DallasTemperature v4.0.6.

```
Arduino Uno               DS18B20 Sensor
─────────────────────────────────────────────

5V  ───┬───────────────────→ Red (VCC)
      │
      │
     1kΩ  ← Pull-up resistor
      │
      │
D2  ───┴───────────────────→ Blue (DATA)

GND ───────────────────────→ Black (GND)
```

### Hasil Testing

Pengujian sensor dilakukan dengan tiga kondisi: suhu ruang sebagai baseline, pendinginan dengan air dan es, serta monitoring respon termal. Pada kondisi suhu ruang, sensor menunjukkan bacaan 26.94°C dengan referensi sekitar 27°C, serta stabilitas ±0.35°C selama 10 menit.

Pada uji pendinginan, sensor menunjukkan respon linear dan halus dari 26.94°C menuju 17°C ketika dicelupkan ke campuran air dan es. Tidak terdapat jump atau fluktuasi abnormal pada pembacaan sensor.

**Spesifikasi Sensor:**

| Parameter | Nilai |
|-----------|-------|
| Range Operasi | -55°C s/d +125°C |
| Akurasi | ±0.5°C @ -10°C s/d +85°C |
| Resolusi | 9-12 bit (programmable) |
| Supply Voltage | 3.0V - 5.5V |
| Waterproof | Ya (stainless steel sheath) |

Sensor tidak memerlukan kalibrasi tambahan karena akurasi pabrik ±0.5°C telah memenuhi requirement sistem. Deadband controller ±1°C lebih lebar dibandingkan akurasi sensor, sehingga penggunaan nilai pabrik sudah mencukupi.

---

# BAB 2
## PERMASALAHAN DAN SOLUSI

### 2.1 Bacaan Sensor Tidak Stabil

Pada awal pengujian, ditemukan bacaan sensor yang melompat secara abnormal (22→24→28°C). Penyebab masalah ini adalah koneksi wiring yang longgar pada breadboard, sehingga sinyal OneWire tidak tertransmisi dengan baik.

Solusi yang diterapkan adalah perbaikan koneksi kabel dan memastikan semua kontak pada breadboard terpasang dengan kuat. Setelah perbaikan, bacaan sensor menjadi stabil dan menunjukkan respon yang linear.

### 2.2 Nilai Resistor Pull-up Tidak Standar

Penggunaan resistor pull-up 1kΩ berbeda dengan nilai standar 4.7kΩ yang umum digunakan pada sensor DS18B20. Nilai ini dipilih berdasarkan ketersediaan komponen dan pengujian menunjukkan fungsi normal untuk jarak kabel 1 meter.

Untuk implementasi dengan jarak kabel yang lebih jauh (>5m), disarankan menggunakan resistor pull-up 2.2kΩ atau 4.7kΩ untuk menjaga integritas sinyal OneWire.

### 2.3 Warna Kabel Non-Standard

Warna kabel pada sensor DS18B20 yang digunakan (Red=VCC, Blue=DATA, Black=GND) berbeda dengan standar umum yang biasanya menggunakan Yellow atau White untuk DATA. Hal ini dapat menyebabkan kebingungan saat wiring atau troubleshoot.

Solusi yang diterapkan adalah dokumentasi dan label yang jelas pada setiap kabel serta pada schematic diagram. Dengan dokumentasi yang baik, risiko salah wiring dapat diminimalkan.

### 2.4 Respon Termal Lambat

Probe stainless steel dengan panjang 50mm memiliki thermal mass yang signifikan, menyebabkan respon sensor tidak instan terhadap perubahan suhu. Hal ini merupakan karakteristik fisik dari waterproof probe yang tidak dapat dihindari.

Namun demikian, respon yang lambat ini memberikan efek filtering natural yang bermanfaat untuk aplikasi temperature control. Sensor tidak menunjukkan osilasi atau fluktuasi cepat, sehingga stabil sebagai feedback untuk sistem kendali.

### 2.5 Keterbatasan Akurasi untuk Aplikasi Presisi Tinggi

Akurasi sensor ±0.5°C sudah mencukupi untuk aplikasi mixing pada SOLOPHOBIA dengan deadband controller ±1°C. Namun untuk aplikasi yang memerlukan akurasi lebih tinggi (<0.1°C), sensor DS18B20 mungkin tidak memenuhi requirement.

Untuk pengembangan selanjutnya, jika diperlukan akurasi yang lebih tinggi, pertimbangan penggunaan sensor lain seperti PT100 atau thermocouple dengan kalibrasi khusus dapat dilakukan.

---

# REFERENSI

- Datasheet Maxim Integrated DS18B20
- Library OneWire v2.3.8, DallasTemperature v4.0.6
- Test Report: B400-SENS-001
- Kode test: `/b400/sensor_temp/ds18b20_test.ino`
