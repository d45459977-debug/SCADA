# SOLOPHOBIA - Heating & Mixing Miniplant

**Automasi proses pembuatan sabun cair dengan arsitektur *Distributed Control System* (DCS)**

---

## 📋 Overview

SOLOPHOBIA adalah sistem automasi proses batch untuk pembuatan sabun cair dengan 3 tahap proses utama:

| Tahap | Slave | Fungsi | Lokasi | Kode |
|-------|-------|--------|--------|------|
| **1** | **TK-101** | Flow Control - Pengisian air | Tangki Air | `sensor_flow/slave1_flow_control.ino` |
| **2** | **TK-201** | Level Control - Dosing texapon | Tangki Texapon | `sensor_level/slave2_level_control.ino` |
| **3** | **TK-301** | Temp Control - Pemanasan & Mixing | Tangki Reaktor | `sensor_temp/slave3_temp_control.ino` |

### Arsitektur Sistem

```
┌─────────────────┐     RS485      ┌──────────────┐
│   HMI/SCADA     │◄──────────────►│  ESP32 Master │
│   (AVEVA/KEPServer)              │   Gateway    │
└─────────────────┘                └──────┬───────┘
                                           │
                                    ┌──────┴───────┐
                    RS485 Bus     │              │
                    (ASCII)      ▼              ▼
                               ┌──────┐      ┌──────┐
                               │ S1   │      │ S2   │
                               │ Uno  │      │ Uno  │
                               └──────┘      └──────┘
                                    │
                                    ▼
                               ┌──────┐
                               │ S3   │
                               │ Uno  │
                               └──────┘
```

---

## 🏗️ Struktur Project

```
B400/
├── KOMDAT_TCPIP/           # Komunikasi TCP/IP & ESP32 Gateway
│   ├── esp32_gateway/
│   │   ├── esp32_simulasi_dummy/   # Master RS485-only (testing)
│   │   └── esp32_kepware/          # Master Modbus TCP (KEPServer)
│   └── CLAUDE.md                    # Dokumentasi ESP32
│
├── sensor_flow/             # Slave 1 - Flow Control
│   ├── slave1_flow_control/        # Sketch Slave 1 Aktif
│   ├── YFS401_TEST_REPORT.md       # Test report sensor
│   ├── DATA_KALIBRASI_YFS401.md    # Data kalibrasi
│   └── README.md                    # Overview flow sensor
│
├── sensor_level/            # Slave 2 - Level Control
│   ├── slave2_level_control/        # Sketch Slave 2 Aktif
│   ├── HC-SR04_TEST_REPORT.md      # Test report sensor
│   ├── DATA_KALIBRASI_HC-SR04.md   # Data kalibrasi
│   └── README.md                    # Overview ultrasonic
│
├── sensor_temp/             # Slave 3 - Temperature Control
│   ├── slave3_temp_control/        # Sketch Slave 3 Aktif
│   ├── DS18B20_TEST_REPORT.md      # Test report sensor
│   ├── DATA_KALIBRASI_DS18B20.md   # Data kalibrasi
│   └── README.md                    # Overview temp sensor
│
├── wiring/                  # Dokumentasi hardware
│   ├── motor_leadscrew/             # Motor M-201 wiring
│   └── CLAUDE.md                    # Overview wiring
│
├── B500/                    # Implementasi Kendali & Pengujian
│   ├── slave1/, slave2/, slave3/    # Kode cloned
│   ├── master_dummy/, master_kepware/
│   ├── 2.1.1_PENGUJIAN_SUB_SISTEM_KENDALI.md  # 68 test items
│   └── README.md                    # Overview B500
│
├── CLAUDE.md                # Panduan utama project
├── HMI_TAG_LIST.md         # Tag list untuk HMI/SCADA
└── PEMBAHASAN_KENDALI_SLAVE.md  # Analisis sistem kendali
```

---

## 🔌 Hardware Specification

### Slave 1 - TK-101 (Flow Control)

| Komponen | Tipe | Spec | Pin |
|----------|-----|------|-----|
| **Flow Sensor** | YF-S201 | 0-5000 mL/min, pulse output | D3 (INT1) |
| **Pompa** | DC Submersible 12V | 300-500 mA | D7 (Relay) |
| **Microcontroller** | Arduino Uno | ATmega328P | - |

**Wiring Flow Sensor:**
```
YF-S201 → Arduino:
  RED   → +5V
  BLACK → GND
  YELLOW → D3 (INT1)
```

### Slave 2 - TK-201 (Level Control)

| Komponen | Tipe | Spec | Pin |
|----------|-----|------|-----|
| **Ultrasonic** | HC-SR04 | 2-400 cm | D4 (TRIG), D12 (ECHO) |
| **Motor DC** | Geared 12V | Lead screw actuator | D8, D7, D9 (TB6612FNG) |
| **Valve** | Solenoid 12V | MOV-202 | - |
| **Microcontroller** | Arduino Uno | ATmega328P | - |

**Wiring Motor (TB6612FNG):**
```
TB6612FNG → Arduino:
  STBY → D5
  AIN1 → D8
  AIN2 → D7
  PWMA → D9
  VM   → +12V PSU
  GND  → GND (common!)
```

### Slave 3 - TK-301 (Temp Control)

| Komponen | Tipe | Spec | Pin |
|----------|-----|------|-----|
| **Temp Sensor** | DS18B20 | -55 to +125°C | D3 (1-Wire) |
| **Heater** | 220V AC element | SSR-25DA control | D4 (SSR) |
| **Mixer** | DC Motor 12V | TB6612FNG driver | D6, D7, D9 |
| **Valve** | Solenoid 12V | MOV-302 | - |
| **Microcontroller** | Arduino Uno | ATmega328P | - |

**Wiring DS18B20:**
```
DS18B20 → Arduino:
  VCC  → +5V (atau 3.3V)
  GND  → GND
  DATA → D3 (with 4.7kΩ pull-up to VCC)
```

### RS485 Communication (SEMUA Slave)

| Pin | Fungsi | Catatan |
|-----|--------|---------|
| D2 | DE/RE | RS485 direction control |
| D10 | RO | Receiver output |
| D11 | DI | Driver input |

**PENTING:** Pin RS485 SAMA di semua slave. Jangan ubah D2, D10, D11.

---

## 📡 Komunikasi & Protocol

### ASCII Protocol (Master → Slave)

| Format | Contoh | Response |
|--------|--------|----------|
| `S1:PING` | `S2:PING` | `S1:OK` / `S2:OK` |
| `S1:READ` | `S2:READ` | `S1:DATA,...` / `S2:DATA,...` |
| `S1:START:900` | `S2:START:100` | `S1:OK,START,...` / `S2:OK,START,...` |
| `S1:SETSP:1000` | `S2:SETSP:15` | `S1:OK,SETSP=...` / `S2:OK,SETSP=...` |

### Modbus TCP Register Map (ESP32 → HMI)

| Address | Tag | Deskripsi | Scaling |
|---------|-----|----------|---------|
| 40001 | S1_VOLUME_SP | Setpoint Volume S1 | 1.0 (1 = 1 mL) |
| 40002 | S1_VOLUME_PV | Volume Aktual S1 | 1.0 |
| 40003 | S1_PUMP_STATUS | Status Pompa S1 | 0=OFF, 1=ON |
| 40007 | S2_DISTANCE_SP | Setpoint Jarak S2 | ÷100 (2381 = 23.81cm) |
| 40008 | S2_DISTANCE_PV | Jarak Aktual S2 | ÷100 |
| 40015 | S3_TEMP_SP | Setpoint Suhu S3 | ÷10 (600 = 60.0°C) |
| 40016 | S3_TEMP_PV | Suhu Aktual S3 | ÷10 |

**Lengkap:** Lihat `HMI_TAG_LIST.md`

---

## 🚀 Quick Start

### 1. Upload ke Arduino (Slave)

```bash
# Cek port dulu!
ls -la /dev/tty* | grep -E "(USB|ACM)"

# Identifikasi slave dengan PING dari ESP32 sebelum upload:
# S1:PING → Slave 1
# S2:PING → Slave 2
# S3:PING → Slave 3

# Compile & Upload
arduino-cli compile --fqbn arduino:avr:uno <sketch>.ino
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:uno <sketch>.ino

# Serial monitor
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=9600
```

### 2. Upload ke ESP32 (Master)

```bash
cd KOMDAT_TCPIP/esp32_gateway

# Master RS485-only (testing)
pio run --target upload
pio device monitor

# Atau dengan arduino-cli
arduino-cli compile --fqbn esp32:esp32:esp32 esp32_simulasi_dummy/esp32_simulasi_dummy.ino
arduino-cli upload -p /dev/ttyACM0 --fqbn esp32:esp32:esp32 esp32_simulasi_dummy/esp32_simulasi_dummy.ino
```

### 3. Test Komunikasi

Buka Serial Monitor ESP32 (115200 baud):

```
========== MENU MASTER ==========
1 - Test Komunikasi Slave 1
2 - Test Komunikasi Slave 2
3 - Test Komunikasi Slave 3
4 - Test Semua Slave
d - Display Semua Slave
m - Menu Ini
=================================
```

---

## 📊 Parameter Operasi

| Parameter | Slave 1 | Slave 2 | Slave 3 |
|-----------|---------|---------|---------|
| **Sensor** | YF-S201 | HC-SR04 | DS18B20 |
| **Sampling** | 1 Hz | 2 Hz | 1 Hz |
| **Actuator** | Relay FY-101 | M-201 + MOV-202 | SSR TY-301 + M-301 |
| **Setpoint (FIXED)** | 900 mL | 100 mL | 60.0 °C |
| **Deadband** | ±10 mL | ±0.3 cm | 59-61 °C |
| **Timeout** | 15 menit | 5 menit | 7 menit |

---

## 📖 Dokumen Penting

| Dokumen | Deskripsi |
|---------|-----------|
| `CLAUDE.md` | Panduan utama project (WAJIB baca) |
| `HMI_TAG_LIST.md` | Tag list lengkap untuk HMI/SCADA |
| `PEMBAHASAN_KENDALI_SLAVE.md` | Analisis sistem kendali |
| `B500/2.1.1_PENGUJIAN_SUB_SISTEM_KENDALI.md` | 68 test items untuk validasi |
| `KOMDAT_TCPIP/CLAUDE.md` | Dokumentasi ESP32 & komunikasi |
| `wiring/CLAUDE.md` | Overview wiring & hardware |

---

## 🔧 Troubleshooting

### Port Arduino Berubah-ubah?

Port sering berubah (`USB0`, `USB1`, `ACM0`, dll). **SELALU:**

```bash
# 1. Cek port
ls -la /dev/tty* | grep -E "(USB|ACM)"

# 2. Identifikasi slave dengan PING dari ESP32
# Kirim: S1:PING → Slave mana yang jawab OK?

# 3. Upload ke port yang benar
```

### Slave TIMEOUT?

1. Cek wiring RS485 (D2, D10, D11)
2. Cek common ground
3. Test dengan PING dulu
4. Cek DE/RE timing di ESP32 (`delayMicroseconds(50)`)

### Slave 3 Intermittent?

Slave 3 butuh delay lebih lama:
- Delay 800ms sebelum komunikasi
- 2x retry logic
- Cek pull-up resistor 4.7kΩ di DS18B20

---

## 📝 Development Guide

### Konvensi Kode

1. **Bahasa:** Bahasa Indonesia untuk komentar, istilah teknis dalam *italic*
2. **Nama variabel:** *camelCase* untuk variabel, *CAPITAL_SNAKE* untuk konstanta
3. **Validasi:** Selalu cek batas aman (min/max) sebelum eksekusi actuator

### Git Workflow

```bash
# Branch utama: main
# Untuk fitur baru, buat branch dari main
git checkout -b feature/nama-fitur

# Commit dengan format:
# docs: update dokumentasi
# fix: perbaiki bug
# feat: tambah fitur baru

git push origin feature/nama-fitur
```

---

## 📞 Team Contact

Untuk pertanyaan teknis, cek dokumentasi di folder masing-masing:
- Flow Sensor: `sensor_flow/`
- Level Sensor: `sensor_level/`
- Temp Sensor: `sensor_temp/`
- ESP32 Gateway: `KOMDAT_TCPIP/`
- Wiring: `wiring/`

---

## 📜 Changelog

| Versi | Tanggal | Update |
|-------|--------|--------|
| 1.0 | 2026-05-25 | Initial release |
| 1.5 | 2026-05-27 | Tambah port mapping dinamis, protokol ASCII |
| 2.0 | 2026-05-30 | Dokumentasi lengkap, B500, HMI tags |

---

**Project:** SOLOPHOBIA - Heating & Mixing Miniplant
**Fase:** B400 - Implementasi & Verifikasi Eksperimental
**Platform:** Arduino Uno + ESP32
**License:**
