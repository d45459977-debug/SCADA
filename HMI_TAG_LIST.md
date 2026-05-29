# HMI Tag List - SOLOPHOBIA B400

**Tanggal:** 2026-05-30
**Versi:** 2.0 - Command Registers Lengkap & KEPServer Integration

---

## Overview

Dokumen ini berisi daftar lengkap semua *tag* variable untuk komunikasi HMI ⇄ ESP32 Gateway ⇄ 3 Slave Arduino.

---

## Register Map Lengkap (ESP32 Gateway → HMI)

### Slave 1 - Flow Control (TK-101)

| Addr (KEP) | Addr (0) | Tag | Deskripsi | Tipe | Scaling | RW |
|------------|----------|-----|-----------|------|---------|-----|
| 40001 | 1 | `S1_VOLUME_SP` | Setpoint Volume | Uint16 | 1.0 (1 = 1 mL) | RW |
| 40002 | 2 | `S1_VOLUME_PV` | Volume Aktual | Uint16 | 1.0 | R |
| 40003 | 3 | `S1_PUMP_STATUS` | Status Pompa | Uint16 | 0=OFF, 1=ON | RW |
| 40004 | 4 | `S1_FLOW_RATE` | Debit Air | Uint16 | ×0.1 (10 = 1.0 mL/min) | R |
| 40005 | 5 | `S1_PULSE_COUNT` | Total Pulse | Uint16 | 1.0 | R |
| 40006 | 6 | `S1_PROCESS_ACTIVE` | Proses Aktif | Uint16 | 0=STOP, 1=RUN | R |
| 40022 | 22 | `S1_CONNECTED` | Slave Terhubung | Uint16 | 0=NO, 1=YES | R |

### Slave 2 - Level Control (TK-201)

| Addr (KEP) | Addr (0) | Tag | Deskripsi | Tipe | Scaling | RW |
|------------|----------|-----|-----------|------|---------|-----|
| 40007 | 7 | `S2_DISTANCE_SP` | Setpoint Jarak | Uint16 | ÷100 (2381 = 23.81 cm) | RW |
| 40008 | 8 | `S2_DISTANCE_PV` | Jarak Aktual | Uint16 | ÷100 | R |
| 40009 | 9 | `S2_DISTANCE_ERR` | Error Jarak | Int16* | ÷100, signed | R |
| 40010 | 10 | `S2_VOLUME_SP` | Setpoint Volume | Uint16 | 1.0 (1 = 1 mL) | R |
| 40011 | 11 | `S2_VOLUME_PV` | Volume Aktual | Uint16 | 1.0 | R |
| 40012 | 12 | `S2_MOTOR_DIR` | Arah Motor | Uint16 | 0=STOP, 1=UP, 2=DOWN | RW |
| 40013 | 13 | `S2_AUTO_STATUS` | Auto Control | Uint16 | 0=OFF, 1=ON | R |
| 40014 | 14 | `S2_SYS_STATUS` | Status Sistem | Uint16 | 0=STOP, 1=RUN, 2=FAULT | R |
| 40023 | 23 | `S2_CONNECTED` | Slave Terhubung | Uint16 | 0=NO, 1=YES | R |

### Slave 3 - Temperature Control (TK-301)

| Addr (KEP) | Addr (0) | Tag | Deskripsi | Tipe | Scaling | RW |
|------------|----------|-----|-----------|------|---------|-----|
| 40015 | 15 | `S3_TEMP_SP` | Setpoint Suhu | Uint16 | ÷10 (600 = 60.0 °C) | RW |
| 40016 | 16 | `S3_TEMP_PV` | Suhu Aktual | Uint16 | ÷10 | R |
| 40017 | 17 | `S3_HEATER_STATUS` | Status Heater | Uint16 | 0=OFF, 1=ON | RW |
| 40018 | 18 | `S3_MIXER_STATUS` | Status Mixer | Uint16 | 0=OFF, 1=ON | R |
| 40019 | 19 | `S3_MIXER_SPEED` | Kecepatan Mixer | Uint16 | 0=STOP, 1=NORMAL, 2=CEPAT | RW |
| 40020 | 20 | `S3_PROCESS_TIMER` | Timer Countdown | Uint16 | 1.0 (s) | R |
| 40021 | 21 | `S3_PROCESS_STATUS` | Proses Aktif | Uint16 | 0=STOP, 1=RUN | R |
| 40024 | 24 | `S3_CONNECTED` | Slave Terhubung | Uint16 | 0=NO, 1=YES | R |

---

## Command Registers (Control & Actuator Commands)

### Slave 1 Commands (40100-40106)

| Addr (KEP) | Addr (0) | Tag | Deskripsi | Write Value | Hasil |
|------------|----------|-----|-----------|-------------|-------|
| 40100 | 100 | `S1_SET_VOLUME_SP` | Set Setpoint Volume | 0-5000 | SP = nilai (mL) |
| 40101 | 101 | `S1_START_STOP` | Start/Stop Process | 1=START, 0=STOP | Toggle proses |
| 40102 | 102 | `S1_RELAY_CTRL` | Control Relay Pompa | 1=ON, 0=OFF | Pompa ON/OFF |
| 40103 | 103 | `S1_RESET` | Reset Volume Counter | 1 | Reset ke 0 |
| 40104 | 104 | `S1_START_FIXED` | Start FIXED Mode | 1 | Start 900mL |
| 40105 | 105 | `S1_RESERVED` | Reserved | - | - |
| 40106 | 106 | `S1_RESERVED` | Reserved | - | - |

### Slave 2 Commands (40107-40114)

| Addr (KEP) | Addr (0) | Tag | Deskripsi | Write Value | Hasil |
|------------|----------|-----|-----------|-------------|-------|
| 40107 | 107 | `S2_SET_DISTANCE_SP` | Set Setpoint Jarak | 200-2420 | SP = nilai÷100 (cm) |
| 40108 | 108 | `S2_START_STOP` | Start/Stop Process | 1=START, 0=STOP | Toggle proses |
| 40109 | 109 | `S2_MOTOR_DIR` | Control Arah Motor | 0=STOP, 1=UP, 2=DOWN | Motor control |
| 40110 | 110 | `S2_MOTOR_SPEED` | Set Motor Speed PWM | 0-255 | Speed = nilai |
| 40111 | 111 | `S2_START_FIXED` | Start FIXED Mode | 1 | Start 100mL |
| 40112 | 112 | `S2_RESERVED` | Reserved | - | - |
| 40113 | 113 | `S2_RESERVED` | Reserved | - | - |
| 40114 | 114 | `S2_RESERVED` | Reserved | - | - |

### Slave 3 Commands (40115-40122)

| Addr (KEP) | Addr (0) | Tag | Deskripsi | Write Value | Hasil |
|------------|----------|-----|-----------|-------------|-------|
| 40115 | 115 | `S3_SET_TEMP_SP` | Set Setpoint Suhu | 250-900 | SP = nilai÷10 (°C) |
| 40116 | 116 | `S3_START_STOP` | Start/Stop Process | 1=START, 0=STOP | Toggle proses |
| 40117 | 117 | `S3_HEATER_CTRL` | Control Heater | 1=ON, 0=OFF | Heater ON/OFF |
| 40118 | 118 | `S3_MIXER_SPEED` | Set Kecepatan Mixer | 0=STOP, 1=NORMAL, 2=CEPAT | Mixer control |
| 40119 | 119 | `S3_RESERVED` | Reserved | - | - |
| 40120 | 120 | `S3_RESERVED` | Reserved | - | - |
| 40121 | 121 | `S3_RESERVED` | Reserved | - | - |
| 40122 | 122 | `S3_RESERVED` | Reserved | - | - |

**Cara Pakai Command Registers:**
- Write **1** untuk START/ON/FIXED mode
- Write **0** untuk STOP/OFF
- Write **nilai spesifik** untuk SETSP/SETPOINT

---

## Actuator Control Details

### Slave 1 - Pompa Air FY-101

| Tag | Register | Write Value | Hasil |
|-----|----------|-------------|-------|
| `S1_PUMP_STATUS` | 40003 | 1 | Pompa ON |
| `S1_PUMP_STATUS` | 40003 | 0 | Pompa OFF |

### Slave 2 - Motor M-201 (Plunger)

| Tag | Register | Write Value | Hasil |
|-----|----------|-------------|-------|
| `S2_MOTOR_DIR` | 40012 | 1 | Motor UP (Naik) |
| `S2_MOTOR_DIR` | 40012 | 2 | Motor DOWN (Turun) |
| `S2_MOTOR_DIR` | 40012 | 0 | Motor STOP |

### Slave 3 - Heater TY-301 & Mixer M-301

| Tag | Register | Write Value | Hasil |
|-----|----------|-------------|-------|
| `S3_HEATER_STATUS` | 40017 | 1 | Heater ON |
| `S3_HEATER_STATUS` | 40017 | 0 | Heater OFF |
| `S3_MIXER_SPEED` | 40019 | 1 | Mixer NORMAL (40% PWM) |
| `S3_MIXER_SPEED` | 40019 | 2 | Mixer CEPAT (60% PWM) |
| `S3_MIXER_SPEED` | 40019 | 0 | Mixer STOP |

---

## Setpoint Control (Write untuk Ubah SP)

| Slave | Tag | Register | Range | Unit | Scaling |
|-------|-----|----------|-------|------|---------|
| S1 | `S1_VOLUME_SP` | 40001 | 0-5000 | mL | 1.0 |
| S2 | `S2_DISTANCE_SP` | 40007 | 200-2420 | cm | ÷100 |
| S3 | `S3_TEMP_SP` | 40015 | 250-900 | °C | ÷10 |

**Contoh Write:**
- Set Volume SP 1500 mL → Write **1500** ke 40001
- Set Jarak SP 20.00 cm → Write **2000** ke 40007
- Set Suhu SP 55.0 °C → Write **550** ke 40015

---

## Scaling Guide untuk KEPServerEX

| Register | Raw Min | Raw Max | Eng Min | Eng Max | Formula |
|----------|---------|---------|---------|---------|---------|
| 40004 | 0 | 5000 | 0.0 | 500.0 | Raw × 0.1 |
| 40007-40008 | 200 | 2420 | 2.00 | 24.20 | Raw ÷ 100 |
| 40009 | 0 | 1000 | 0.00 | 10.00 | (Raw<32768?Raw:32768-Raw)÷100 |
| 40015-40016 | 250 | 900 | 25.0 | 90.0 | Raw ÷ 10 |

---

## Status Koneksi Slave

| Register | Tag | Timeout | Indikator |
|----------|-----|---------|-----------|
| 40022 | `S1_CONNECTED` | 5 detik | 1=OK, 0=Timeout |
| 40023 | `S2_CONNECTED` | 5 detik | 1=OK, 0=Timeout |
| 40024 | `S3_CONNECTED` | 5 detik | 1=OK, 0=Timeout |

**HMI Alarm:** Jika `CONNECTED = 0` lebih dari 10 detik → tampilkan "Slave X Offline"

---

## Setpoint FIXED (Mode Produksi)

| Parameter | Slave 1 | Slave 2 | Slave 3 |
|-----------|---------|---------|---------|
| Nilai SP | 900 mL | 100 mL | 60.0 °C |
| Command | `CMD_S1_START` (40100) | `CMD_S2_START` (40110) | `CMD_S3_START` (40120) |
| Timeout | 15 menit | 5 menit | 7 menit |

---

## Protocol ASCII (ESP32 → Slave)

### Slave 1
```
→ S1:READ
← S1:DATA,SP=xxx.x,PV=xxx.x,FLOW=xxx.x,PULSE=xxxxx,RELAY=ON/OFF,PROCESS=RUNNING/STOP
```

### Slave 2
```
→ S2:READ
← S2:DATA,SP=xx.xx,PV=xx.xx,ERR=±x.xx,VOLSP=xxxx,VOLPV=xxxx,MOTOR=UP/DOWN/STOP,AUTO=ON/OFF,STATUS=RUNNING/STOP/FAULT
```

### Slave 3
```
→ S3:READ
← S3:OK,SP=xx.x,PV=xx.x,HEATER=ON/OFF,MIXER=ON/OFF,SPEED=NORMAL/CEPAT,TIMER=xxx,PROCESS=RUN/STOP
```

---

## Quick Reference Matrix

| Fungsi | Register | Type | Slave |
|--------|----------|------|-------|
| **Setpoint** | 40001, 40007, 40015 | RW | S1, S2, S3 |
| **PV Value** | 40002, 40008, 40016 | R | S1, S2, S3 |
| **Actuator** | 40003, 40012, 40017, 40019 | RW | S1, S2, S3 |
| **Process Status** | 40006, 40014, 40021 | R | S1, S2, S3 |
| **Connection** | 40022, 40023, 40024 | R | S1, S2, S3 |
| **Command Trigger** | 40100-40122 | W | All |

---

## File Terkait

- `HMI_TAG_LIST.md` - File ini
- `KOMDAT_TCPIP/esp32_gateway/esp32_kepware/esp32_kepware.ino` - ESP32 Gateway untuk KEPServerEX
- `KOMDAT_TCPIP/esp32_gateway/esp32_simulasi_dummy/esp32_simulasi_dummy.ino` - ESP32 Gateway RS485-only
- `init/` - Backup kode yang berfungsi
- `KOMDAT_TCPIP/CLAUDE.md` - Dokumentasi ESP32

---

## Changelog

**v1.0 → v2.0:**
- v1.1: Tambah register lengkap semua slave (40001-40021)
- v1.2: Tambah command registers (40100-40122), status koneksi (40022-40024), actuator control
- v2.0: Perbaiki command registers lengkap untuk semua aktuator control, siap integrasi KEPServerEX

---

**EOF**
