# CLAUDE.md

File ini memberikan panduan untuk Claude Code (claude.ai/code) saat bekerja dengan kode di repositori ini.

**Selalu gunakan bahasa Indonesia** dalam komunikasi dan dokumentasi, kecuali untuk istilah teknis yang ditulis dengan *italic*.

---

## Project Overview

**SOLOPHOBIA** - Heating & Mixing Miniplant adalah sistem automasi proses pembuatan sabun cair dengan arsitektur *distributed control*:

| Komponen | Deskripsi |
|----------|-----------|
| **Master** | ESP32 sebagai *Modbus Master* / *Gateway* TCP/IP |
| **Slave 1** | Arduino Uno - *Flow Control* TK-101 (Pompa Air) |
| **Slave 2** | Arduino Uno - *Level Control* TK-201 (Dosing Texapon) |
| **Slave 3** | Arduino Uno - *Temperature Control* TK-301 (Heater + Mixer) |
| **Komunikasi** | *Modbus RTU* over RS-485 atau TCP/IP |

**Tahap B400** adalah fase Implementasi & Verifikasi Eksperimental.

---

## Struktur Direktori

```
B400/
├── KOMDAT_TCPIP/       # Implementasi komunikasi TCP/IP (ESP32 Gateway)
│   └── esp32_gateway/esp32_simulasi_dummy/  # Sketch ESP32 Master
├── sensor_flow/        # Karakterisasi sensor YFS401 (hall-effect flow sensor)
│   └── slave1_flow_control/  # Sketch Slave 1 Aktif
├── sensor_level/       # Karakterisasi sensor HC-SR04 (ultrasonic distance)
│   └── slave2_level_control/  # Sketch Slave 2 Aktif
├── sensor_temp/        # Implementasi sensor DS18B20 (1-Wire temperature)
│   └── (Slave 3 belum diimplementasi)
└── wiring/             # Diagram wiring dan dokumentasi hardware
```

---

## Perintah Development

### Arduino Uno (Semua Slave)

```bash
# Compile sketch
arduino-cli compile --fqbn arduino:avr:uno <sketch>.ino

# CEK PORT dulu sebelum upload
ls -la /dev/tty* | grep -E "(USB|ACM)"

# Identifikasi slave dengan PING dari ESP32 sebelum upload:
# S1:PING → Slave 1
# S2:PING → Slave 2
# S3:PING → Slave 3

# Upload ke Arduino (GANTI PORT dengan port yang benar)
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:uno <sketch>.ino

# Serial monitor (9600 baud untuk kebanyakan sketch)
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=9600
# atau
screen /dev/ttyUSB0 9600
```

**PERINGATAN:** Port sering berubah (`USB0`, `USB1`, `ACM0`, dll). SELALU cek port dan identifikasi slave sebelum upload.

### ESP32 Gateway (PlatformIO)

```bash
cd KOMDAT_TCPIP/esp32_gateway
pio run                    # Build
pio run --target upload    # Upload
pio device monitor         # Serial monitor (115200 baud)
```

### Python Data Logger

```bash
cd sensor_flow
python3 logger.py          # Logging data sensor ke CSV
```

---

## Pin Assignments Utama

| Sensor/Actuator | Pin Arduino | Catatan |
|-----------------|-------------|---------|
| YFS401 Flow | D3 (INT1) | Interrupt untuk *pulse counting* |
| HC-SR04 TRIG | D4 | Ultrasonic trigger |
| HC-SR04 ECHO | D12 | Ultrasonic echo |
| DS18B20 DATA | D2 | 1-Wire dengan *pull-up* 1kΩ ke 5V |
| **TB6612FNG AIN1** | **D8** | **Motor direction 1** |
| **TB6612FNG AIN2** | **D7** | **Motor direction 2** |
| **TB6612FNG PWMA** | **D9** | **Motor PWM speed** |
| **TB6612FNG STBY** | **D5** | **Motor standby enable** |

### RS485 Pin (SEMUA Slave - Konsisten)

| Pin | Fungsi | Catatan |
|-----|--------|---------|
| D2 | DE/RE | RS485 direction control |
| D10 | RO | Receiver output |
| D11 | DI | Driver input |

**PENTING:** Pin RS485 SAMA di semua slave. Jangan ubah D2, D10, D11 untuk keperluan lain.

---

## Parameter Kunci Sistem Kendali

| Parameter | Slave 1 (Flow) | Slave 2 (Level) | Slave 3 (Temp) |
|-----------|----------------|-----------------|----------------|
| *Sensor* | YFS401 | HC-SR04 | DS18B20 |
| *Sampling rate* | 1 Hz (1000ms) | 2 Hz (500ms) | 1 Hz (1000ms) |
| *Actuator* | Relay FY-101 | M-201 + MOV-202 | SSR TY-301 + M-301 |
| *Setpoint* | Volume (mL) | Jarak (cm) → Volume (mL) | Suhu (°C) |
| *Deadband* | - | ±0.3 cm | ±1°C |
| *Range* | 0-5000 mL | **0-5590 mL** | 25-60°C |
| *Timeout* | 15 menit | 5 menit | TBD |

---

## Modbus TCP Register Map (ESP32 Gateway)

| Address | Nama | Deskripsi | Default |
|---------|------|-----------|---------|
| 1 (40001) | Volume_SP | Setpoint Volume (mL) | 1000 |
| 2 (40002) | Volume_PV | Volume Aktual (mL) | 0 |
| 3 (40003) | Status_Pompa | 0=OFF, 1=ON | 0 |
| 4 (40004) | Jarak_SP | Setpoint Jarak (cm) | 15 |
| 5 (40005) | Jarak_PV | Jarak Aktual (cm) | 20 |
| 6 (40006) | Status_Motor | 0=OFF, 1=ON | 0 |
| 7 (40007) | Suhu_SP | Setpoint Suhu (×0.1 °C) | 600 |
| 8 (40008) | Suhu_PV | Suhu Aktual (×0.1 °C) | 250 |
| 9 (40009) | Status_Heater | 0=OFF, 1=ON | 0 |
| 10 (40010) | Mixing_Duration | Durasi Mixing (s) | 300 |
| 11 (40011) | Mixing_Timer | Timer Mixing (s) | 0 |
| 12 (40012) | Status_Mixer | 0=OFF, 1=ON | 0 |

**Catatan:** Suhu disimpan ×10 (600 = 60.0°C), scaling dikerjakan di KEPServerEX.

---

## Konvensi Penulisan Kode

1. **Bahasa:** Bahasa Indonesia untuk komentar, istilah teknis dalam *italic*
2. **Nama variabel:** *camelCase* untuk variabel, *CAPITAL_SNAKE* untuk konstanta
3. **Validasi data:** Selalu cek batas aman (min/max) sebelum eksekusi actuator
4. **Struktur menu serial:** Gunakan format konsisten `d` (display), `c` (continuous), `s` (stop), `r` (reset)

---

## Referensi Dokumentasi Lengkap

| Dokumen | Lokasi |
|---------|--------|
| Overview wiring & hardware | `wiring/CLAUDE.md` |
| Detail implementasi TCP/IP | `KOMDAT_TCPIP/CLAUDE.md` |
| Karakterisasi YFS401 | `sensor_flow/README.md` |
| **Test Report YFS401** | `sensor_flow/YFS401_TEST_REPORT.md` |
| **Data Kalibrasi YFS401** | `sensor_flow/DATA_KALIBRASI_YFS401.md` |
| **Karaterisasi HC-SR04** | `sensor_level/README.md` |
| **Test Report HC-SR04** | `sensor_level/HC-SR04_TEST_REPORT.md` |
| **Implementasi HC-SR04** | `sensor_level/LAPORAN_IMPLEMENTASI_HC-SR04.md` |
| **Data Kalibrasi HC-SR04** | `sensor_level/DATA_KALIBRASI_HC-SR04.md` |
| **Test Report DS18B20** | `sensor_temp/DS18B20_TEST_REPORT.md` |
| **Implementasi DS18B20** | `sensor_temp/LAPORAN_IMPLEMENTASI_DS18B20.md` |
| **Data Kalibrasi DS18B20** | `sensor_temp/DATA_KALIBRASI_DS18B20.md` |
| **Pembahasan Kendali Slave** | `PEMBAHASAN_KENDALI_SLAVE.md` |
| **Wiring Motor M-201** | `wiring/motor_leadscrew/README.md` |
| **Implementasi Motor M-201** | `wiring/motor_leadscrew/LAPORAN_IMPLEMENTASI_M-201.md` |

---

## Sistem Plunger TK-201 (Slave 2)

TK-201 menggunakan mekanisme **plunger** (penekan) untuk dosing texapon:

| Komponen | Nilai |
|----------|-------|
| Bentuk wadah | Balok (tidak simetris) |
| Panjang | 22.0 cm |
| Lebar rata-rata | 11.55 cm |
| **Luas penampang** | **254.1 cm²** |
| Tinggi maksimal texapon | 22.0 cm |
| **Volume maksimal** | **5590 mL** |

**Kalibrasi Aktif (per 2026-05-27):**
- `TINGGI_SENSOR_CM = 24.2 cm` (sensor ke dasar, setelah pindah)
- `FAKTOR_KALIBRASI_SENSOR = 1.00` (sensor sudah akurat)
- `SP_FIXED_TEXAPON_ML = 100.0 mL`

**Prinsip Kerja:**
- Sensor HC-SR04 membaca jarak ke plunger
- Tinggi texapon = TINGGI_SENSOR_CM - jarak_sensor_ke_plunger
- Volume = Tinggi × Luas Penampang
- Perlu kalibrasi ulang setiap ganti holder/posisi sensor

**Sketch Kalibrasi:** `sensor_level/hcsr04_kalibrasi_volume/hcsr04_kalibrasi_volume.ino`

Menu:
- `k` - Kalibrasi jarak penuh (plunger di atas)
- `m` - Kalibrasi manual (input jarak)
- `u` - Ukur volume saat ini
- `c` - Continuous mode (monitoring)
- `t` - Tabel konversi tinggi → volume

---

## Aktuator Motor M-201 (Slave 2)

TK-201 menggunakan **motor DC 12V geared** dengan driver **TB6612FNG** untuk menggerakkan lead screw:

| Komponen | Nilai |
|----------|-------|
| Motor | 12V DC geared |
| Driver | TB6612FNG (1.2A continuous, 3.2A peak) |
| PSU | 12V DC, minimal 2A |
| Lead Screw | Acme/Trapesoid dengan nut |

**Wiring TB6612FNG:**
```
TB6612FNG → Arduino:
  AIN1 → D8, AIN2 → D7, PWMA → D9, STBY → D5, VCC → 5V, GND → GND
TB6612FNG → PSU 12V:
  VM → +12V, GND → V- (common ground!)
TB6612FNG → Motor:
  AO1 → Lead 1, AO2 → Lead 2
```

**Kontrol Motor:**
- Short brake: AIN1=HIGH, AIN2=HIGH
- Motor UP: AIN1=HIGH, AIN2=LOW
- Motor DOWN: AIN1=LOW, AIN2=HIGH
- Speed: PWM 0-255 di PWMA
- Enable: STBY=HIGH

**Sketch Test:** `wiring/motor_leadscrew/test_motor_basic/test_motor_basic.ino`

Menu:
- `1` - Tes arah UP (NAIK)
- `2` - Tes arah DOWN (TURUN)
- `3` - Tes speed variasi
- `s` - Stop motor

---

## Port Mapping Dinamis

Port Arduino sering berubah setelah reconnect/USB. **SELALU cek port sebelum upload:**

```bash
ls -la /dev/tty* | grep -E "(USB|ACM)"
```

Identifikasi slave dengan perintah PING dari ESP32 Master sebelum upload.

## Protokol Komunikasi Master-Slave

Semua slave menggunakan protokol ASCII via SoftwareSerial (RS485):

| Format | Contoh | Response |
|--------|--------|----------|
| `S1:PING` | `S2:PING` | `S1:OK` / `S2:OK` |
| `S1:READ` | `S2:READ` | `S1:DATA,...` / `S2:DATA,...` |
| `S1:START:FIXED` | `S2:START:FIXED` | `S1:OK,START,FIXED,...` |
| `S1:SETSP:1000` | `S2:SETSP:15` | `S1:OK,SETSP=...` |

**Wajib:** Cek slave mana yang tercolok dengan PING sebelum upload sketch apapun.

## SP FIXED vs SP VARIABLE

Setiap slave punya 2 mode setpoint:

| Mode | Slave 1 | Slave 2 | Slave 3 |
|------|---------|---------|---------|
| **FIXED** (produksi) | 900 mL air | 100 mL texapon | TBD |
| **VARIABLE** (dev) | Input manual | Input manual | Input manual |

Perintah `START:FIXED` otomatis pakai SP produksi tanpa input user.

---

## Terakhir Diperbarui

- Tanggal: 2026-05-27
- Versi: 1.5
- Update: Menambah port mapping dinamis, protokol komunikasi ASCII, SP FIXED concept
