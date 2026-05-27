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

---

## Direktori B400

B400 adalah tahap **Implementasi & Verifikasi Eksperimental**. Berisi:

- `KOMDAT_TCPIP/` - Implementasi komunikasi data via TCP/IP
- `sensor_flow/` - Karakterisasi dan implementasi sensor YFS401
- `sensor_level/` - Karakterisasi dan implementasi sensor HC-SR04
- `sensor_temp/` - Implementasi sensor DS18B20
- `wiring/` - Diagram wiring dan dokumentasi hardware (direktori ini)

---

## Perintah Development

### Arduino CLI

```bash
# Compile sketch
arduino-cli compile --fqbn arduino:avr:uno <sketch>.ino

# Upload ke Arduino
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:uno <sketch>.ino

# Serial monitor
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=9600
# atau
screen /dev/ttyUSB0 9600
```

### ESP32 (PlatformIO)

```bash
cd KOMDAT_TCPIP/esp32_gateway
pio run --target upload
pio device monitor
```

### Python Data Logger

```bash
# Untuk logging data sensor ke CSV
python3 logger.py
# Default: /dev/ttyUSB0, 9600 baud
```

---

## Struktur Subdirektori B400

### KOMDAT_TCPIP/

Berisi implementasi ESP32 sebagai *Modbus TCP Server* untuk simulasi dan integrasi dengan KEPServerEX.

**Register Map** (12 *holding registers*):
- Register 1-3: Slave 1 (Volume SP/PV, Status Pompa)
- Register 4-6: Slave 2 (Jarak SP/PV, Status Motor)
- Register 7-12: Slave 3 (Suhu SP/PV, Heater, Mixer)

Lihat: `KOMDAT_TCPIP/CLAUDE.md`

### sensor_flow/

Karakterisasi sensor YFS401 (*hall-effect flow sensor*, 1-5 L/min).

**Sketch:**
- `yfs401_karakteristik.ino` - Testing lengkap dengan menu serial
- `yfs401_simple.ino` - Implementasi sederhana
- `yfs401_terkalibrasi.ino` - Dengan faktor kalibrasi

**Perintah Serial Menu:**
- `d` - Display data
- `c` - Continuous mode
- `s` - Stop
- `z` - Reset counter
- `t` - Repeatability test

### sensor_level/

Karakterisasi sensor HC-SR04 (*ultrasonic distance sensor*).

**Sketch:**
- `hcsr04_karakteristik.ino` - Testing dengan menu serial

**Pin Assignment:**
- TRIG → D9
- ECHO → D10

### sensor_temp/

Implementasi sensor DS18B20 (*temperature sensor* via 1-Wire).

**Pin Assignment:**
- DATA → D2 (dengan *pull-up* resistor 1kΩ ke 5V)

---

## Parameter Kunci dari B300

| Parameter | Slave 1 (Flow) | Slave 2 (Level) | Slave 3 (Temp) |
|-----------|----------------|-----------------|----------------|
| *Sensor* | YF-S201/YFS401 | HC-SR04 | DS18B20 |
| *Sampling rate* | 2 Hz (500ms) | 2 Hz (500ms) | 1 Hz (1000ms) |
| *Actuator* | Relay FY-101 | M-201 + MOV-202 | SSR TY-301 + M-301 |
| *Setpoint* | Volume (mL) | Jarak (cm) | Suhu (°C) |
| *Deadband* | - | ±0.3 cm | ±1°C |
| *Range* | 0-1500 mL | 0-30 cm | 25-60°C |

---

## Konvensi Penulisan Kode

1. **Bahasa:** Bahasa Indonesia untuk komentar, istilah teknis dalam *italic*
2. **Nama variabel:** *camelCase* untuk variabel, *CAPITAL_SNAKE* untuk konstanta
3. **Struktur loop:** ATUR → MULAI → ULANGI SELAMANYA (untuk pseudocode)
4. **Validasi data:** Selalu cek batas aman (min/max) sebelum eksekusi actuator

---

## Dokumen Terkait

- `../B300/CLAUDE.md` - Desain sistem kendali (B300)
- `../B400/KOMDAT_TCPIP/CLAUDE.md` - Detail implementasi komunikasi
- `../B400/sensor_flow/README.md` - Karakterisasi YFS401
- `../B400/sensor_level/README.md` - Karakterisasi HC-SR04
- `../B400/sensor_temp/LAPORAN_IMPLEMENTASI_DS18B20.md` - Implementasi DS18B20

---

## Wiring Diagram Standar

### YFS401 Flow Sensor

```
Arduino Uno               YFS401
─────────────────────────────────────
5V  ───────────────────────→ RED (VCC)
D2  ───────────────────────→ YELLOW (Signal)
GND ───────────────────────→ BLACK (GND)
```

### HC-SR04 Ultrasonic

```
Arduino Uno               HC-SR04
─────────────────────────────────────
5V  ───────────────────────→ VCC
D9  ───────────────────────→ TRIG
D10 ───────────────────────→ ECHO
GND ───────────────────────→ GND
```

### DS18B20 Temperature

```
Arduino Uno               DS18B20
─────────────────────────────────────
5V  ───┬───────────────────→ RED (VCC)
      │
     1kΩ ← Pull-up resistor
      │
D2  ───┴───────────────────→ BLUE (DATA)
GND ───────────────────────→ BLACK (GND)
```

---

## Terakhir Diperbarui

- Tanggal: 2026-05-05
- Versi: 1.0
