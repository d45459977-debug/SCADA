# CLAUDE.md

File ini memberikan panduan untuk Claude Code (claude.ai/code) saat bekerja dengan kode di repositori ini.

**Selalu gunakan bahasa Indonesia** dalam komunikasi dan dokumentasi, kecuali untuk istilah teknis yang ditulis dengan *italic*.

## Konteks Proyek

**SOLOPHOBIA** adalah sistem automasi proses pembuatan sabun cair dengan 3 tahap:
1. **Slave 1 (TK-101):** Flow Control - Pengisian air ke tangki reaktor
2. **Slave 2 (TK-201):** Level Control - Dosing texapon ke tangki reaktor
3. **Slave 3 (TK-301):** Temperature Control - Pemanasan dan mixing campuran

## Direktori KOMDAT_TCPIP

Subdirektori ini untuk implementasi protokol komunikasi **KOMDAT** (Komunikasi Data) via TCP/IP sebagai alternatif atau pengembangan dari sistem *Modbus RTU* over RS-485 yang sudah didokumentasikan.

### Arsitektur Referensi (RS-485/Modbus)

Dari dokumen B300 dan `01_documents/komdat.docx`:
- **MTU (*Master Terminal Unit*):** ESP32 sebagai *Modbus Master*
- **RTU (*Remote Terminal Unit*):** 3x Arduino Uno sebagai *Modbus Slave* (0x01, 0x02, 0x03)
- **Protokol:** *Modbus RTU* over RS-485 (FC03 *read*, FC06 *write*)
- **Topologi:** *Bus* (RS-485) + *Star* (Wi-Fi ke HMI/Database)

### Alur Komunikasi Data

Alur komunikasi dari `komdat.docx`:
1. *Sensor sampling* → Arduino RTU (GPIO/*pulse*/ADC/1-Wire)
2. ESP32 MTU *polling* RTU via *Modbus RTU* (500ms per RTU)
3. Validasi CRC-16, konversi ke JSON
4. *MQTT Publish* ke *broker* lokal
5. *Push* ke HMI (AVEVA InTouch) dan *database* (InfluxDB)

## Pseudocode Referensi

Pseudocode lengkap untuk setiap slave tersedia di `01_documents/pseudocode_sisdal_b300.docx`:

**Slave 1 - *Flow Control* (TK-101):**
- *Sensor*: YF-S201 *flow sensor* via *interrupt pulse counting*
- *Actuator*: Pompa DC PMP-101 via *relay*
- *Loop*: *Bang-bang control* berdasarkan *volume setpoint*

**Slave 2 - *Level Control* (TK-201):**
- *Sensor*: HC-SR04 *ultrasonic* untuk jarak plunger
- *Actuator*: Motor DC M-201 (*lead screw*) + *valve* MOV-202
- *Loop*: *Position control* dengan toleransi ±0.3 cm

**Slave 3 - *Temperature Control* (TK-301):**
- *Sensor*: DS18B20 via 1-Wire
- *Actuator*: SSR-25DA untuk *heater* + TB6612FNG untuk *mixer*
- *Loop*: *Bang-bang control* suhu + *open-loop timer mixing*

## Parameter Kunci dari B300

| Parameter | Slave 1 | Slave 2 | Slave 3 |
|-----------|---------|---------|---------|
| *Sampling rate* | 2 Hz (500ms) | 2 Hz (500ms) | 1 Hz (1000ms) |
| *Sensor* | YF-S201 | HC-SR04 | DS18B20 |
| *Actuator* | *Relay* FY-101 | M-201 + MOV-202 | SSR TY-301 + M-301 |
| *Setpoint* | volume (mL) | jarak (cm) | suhu (°C) + waktu (s) |

## Aturan Penulisan Kode

1. **Bahasa:** Bahasa Indonesia untuk komentar, istilah teknis dalam *italic*
2. **Konvensi nama:** *camelCase* untuk variabel, *CAPITAL_SNAKE* untuk konstanta
3. **Struktur *loop*:** ATUR → MULAI → ULANGI SELAMANYA (jika pseudocode)
4. **Validasi data:** Selalu cek batas aman (min/max) sebelum eksekusi

## Dokumen Terkait

- `../CLAUDE.md` - Instruksi utama proyek SOLOPHOBIA
- `../01_documents/komdat.docx` - Desain komunikasi data *SCADA*
- `../01_documents/pseudocode_sisdal_b300.docx` - Pseudocode lengkap semua slave
- `../01_documents/SOLOPHOBIA_B300.docx` - Desain *hardware & software* lengkap
- `../B300_ANALISIS_3POIN_SEMUA_SLAVE.md` - Analisis sistem kendali

## Status Implementasi

**Modbus TCP Simulator sudah berjalan!**

ESP32 sebagai Modbus TCP Server yang mengekspose 12 *holding registers* untuk simulasi data 3 slave. Terhubung ke KEPServerEX via WiFi.

---

## Perintah Development

### Compile & Upload (Arduino CLI)

```bash
# Compile
arduino-cli compile --fqbn esp32:esp32:esp32 esp32_gateway/esp32_simulasi_dummy/esp32_simulasi_dummy.ino

# Upload
arduino-cli upload -p /dev/ttyACM0 --fqbn esp32:esp32:esp32 esp32_gateway/esp32_simulasi_dummy/esp32_simulasi_dummy.ino

# Serial Monitor
screen /dev/ttyACM0 115200
# atau
arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```

### PlatformIO (Alternatif)

```bash
cd esp32_gateway
pio run --target upload
pio device monitor
```

---

## Modbus TCP Register Map

### Slave 1 - Flow Control (TK-101)

| Address (0-based) | KEPware Address | Name | Description | Scaling | Default |
|-------------------|-----------------|------|-------------|---------|---------|
| 1 | 40001 | S1_VOLUME_SP | Setpoint Volume (mL) | 1.0 | 900 |
| 2 | 40002 | S1_VOLUME_PV | Volume Aktual (mL) | 1.0 | 0 |
| 3 | 40003 | S1_PUMP_STATUS | 0=OFF, 1=ON | - | 0 |
| 4 | 40004 | S1_FLOW_RATE | Debit (mL/min) | ×0.1 | 0 |
| 5 | 40005 | S1_PULSE_COUNT | Total pulse | 1.0 | 0 |
| 6 | 40006 | S1_PROCESS_ACTIVE | 0=STOP, 1=RUN | - | 0 |

### Slave 2 - Level Control (TK-201)

| Address (0-based) | KEPware Address | Name | Description | Scaling | Default |
|-------------------|-----------------|------|-------------|---------|---------|
| 7 | 40007 | S2_DISTANCE_SP | Setpoint Jarak (cm) | ÷100 | 2381 (23.81cm) |
| 8 | 40008 | S2_DISTANCE_PV | Jarak Aktual (cm) | ÷100 | 2420 (24.20cm) |
| 9 | 40009 | S2_DISTANCE_ERR | Error (cm) signed | ÷100 | 0 |
| 10 | 40010 | S2_VOLUME_SP | Setpoint Volume (mL) | 1.0 | 100 |
| 11 | 40011 | S2_VOLUME_PV | Volume Aktual (mL) | 1.0 | 0 |
| 12 | 40012 | S2_MOTOR_DIR | 0=STOP, 1=UP, 2=DOWN | - | 0 |
| 13 | 40013 | S2_AUTO_STATUS | 0=OFF, 1=ON | - | 0 |
| 14 | 40014 | S2_SYS_STATUS | 0=STOP, 1=RUN, 2=FAULT | - | 0 |

### Slave 3 - Temperature Control (TK-301)

| Address (0-based) | KEPware Address | Name | Description | Scaling | Default |
|-------------------|-----------------|------|-------------|---------|---------|
| 15 | 40015 | S3_TEMP_SP | Setpoint Suhu (°C) | ÷10 | 600 (60.0°C) |
| 16 | 40016 | S3_TEMP_PV | Suhu Aktual (°C) | ÷10 | 250 (25.0°C) |
| 17 | 40017 | S3_HEATER_STATUS | 0=OFF, 1=ON | - | 0 |
| 18 | 40018 | S3_MIXER_STATUS | 0=OFF, 1=ON | - | 0 |
| 19 | 40019 | S3_MIXER_SPEED | 0=STOP, 1=NORMAL, 2=CEPAT | - | 0 |
| 20 | 40020 | S3_PROCESS_TIMER | Timer countdown (s) | 1.0 | 420 |
| 21 | 40021 | S3_PROCESS_STATUS | 0=STOP, 1=RUN | - | 0 |

**Catatan Penting:**
- Register 0 tidak dipakai (dummy)
- **Polling otomatis**: ESP32 polling semua 3 slave setiap 500ms
- **Scaling**: Sebagian register perlu scaling di KEPServerEX
- KEPware mengirim **Unit ID = 0** (Modbus TCP default), ESP32 merespon Unit ID 0/1

---

## Konfigurasi KEPServerEX

### Channel Properties
- **Driver:** Modbus TCP/IP Ethernet
- **Network Interface:** Pilih adapter yang terhubung ke WiFi yang sama dengan ESP32

### Device Properties
| Setting | Value |
|---------|-------|
| IP Address | `[IP ESP32]` |
| Port | `502` |
| Device ID | `1` |
| Zero Based Addressing | **UNCHECK** (gunakan 40001-based) |

### Tag Scaling (Suhu)
| Setting | Value |
|---------|-------|
| Scale Type | Linear |
| Raw Low | `0` |
| Raw High | `900` |
| Scaled Low | `0.0` |
| Scaled High | `90.0` |

---

## Struktur File

```
KOMDAT_TCPIP/
├── esp32_gateway/
│   ├── esp32_simulasi_dummy/
│   │   └── esp32_simulasi_dummy.ino    # Sketch simulator (menu Serial)
│   ├── platformio.ini                  # Config PlatformIO
│   └── README.md                       # Cara pakai
├── kepserver/
│   ├── kepware_setup_guide.md          # Panduan KEPware
│   └── solophobia_tags.csv             # Template import tags
└── CLAUDE.md                           # File ini
```

---

## Menu Serial Simulator

Setelah upload, buka Serial Monitor (115200 baud):

| Perintah | Fungsi |
|----------|--------|
| `d` | Display semua data |
| `s` | Toggle simulasi otomatis |
| `1` | Menu Slave 1 (atur volume, toggle pompa) |
| `2` | Menu Slave 2 (atur jarak, toggle motor) |
| `3` | Menu Slave 3 (atur suhu, toggle heater/mixer) |
| `r` | Reset semua data |
| `m` | Tampilkan menu |
