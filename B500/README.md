# B500 - Implementasi Kendali

**Project:** SOLOPHOBIA - Heating & Mixing Miniplant  
**Fase:** B500 - Implementasi Kendali  
**Tanggal:** 2026-05-30

---

## Struktur Folder

```
B500/
├── 2.1.1_PENGUJIAN_SUB_SISTEM_KENDALI.md  # Dokumen pengujian
├── slave1/                                 # Kode Slave 1
│   └── slave1_flow_control.ino
├── slave2/                                 # Kode Slave 2
│   └── slave2_level_control.ino
├── slave3/                                 # Kode Slave 3
│   └── slave3_temp_control.ino
├── master_dummy/                           # Master RS485-only (tanpa WiFi)
│   └── esp32_simulasi_dummy.ino
└── master_kepware/                         # Master untuk KEPServerEX (Modbus TCP)
    └── esp32_kepware.ino
```

---

## Deskripsi

### Slave 1 — TK-101 Tangki Air
- **Sensor:** Flow Sensor YF-S201 (FT-101)
- **Actuator:** Pompa DC Submersible PMP-101 (P-101)
- **Fungsi:** Kendali volume air dengan feedback flow rate
- **Kode:** `slave1_flow_control.ino`

### Slave 2 — TK-201 Tangki Texapon
- **Sensor:** Ultrasonik HC-SR04 (LT-201)
- **Actuator:** Motor DC MTR-201 (lead screw), Valve MOV-202
- **Fungsi:** Kendali level dengan posisioning plunger
- **Kode:** `slave2_level_control.ino`

### Slave 3 — TK-301 Tangki Reaktor
- **Sensor:** DS18B20 (TT-301)
- **Actuator:** Heater SSR HTR-301, Mixer MTR-301, Valve MOV-302
- **Fungsi:** Kendali suhu dengan mixing
- **Kode:** `slave3_temp_control.ino`

---

## Master Gateway

### Master Dummy (RS485-only)
- **File:** `master_dummy/esp32_simulasi_dummy.ino`
- **Fungsi:** RS485 Master tanpa WiFi/Modbus
- **Penggunaan:** Testing, development, local operation
- **Menu:** Serial menu untuk test tiap slave

### Master KEPServer (Modbus TCP)
- **File:** `master_kepware/esp32_kepware.ino`
- **Fungsi:** Gateway untuk KEPServerEX integration
- **Protokol:** Modbus TCP (Port 502) + RS485 Master
- **Registers:** 40001-40024 (data), 40100-40122 (commands)

---

## Quick Reference

### Upload ke Arduino (Slave)
```bash
arduino-cli compile --fqbn arduino:avr:uno slave1/slave1_flow_control.ino
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:uno slave1/slave1_flow_control.ino
```

### Upload ke ESP32 (Master)
```bash
# Master Dummy
arduino-cli compile --fqbn esp32:esp32:esp32 master_dummy/esp32_simulasi_dummy.ino
arduino-cli upload -p /dev/ttyACM0 --fqbn esp32:esp32:esp32 master_dummy/esp32_simulasi_dummy.ino

# Master KEPServer
arduino-cli compile --fqbn esp32:esp32:esp32 master_kepware/esp32_kepware.ino
arduino-cli upload -p /dev/ttyACM0 --fqbn esp32:esp32:esp32 master_kepware/esp32_kepware.ino
```

### Port Assignment
- **Slave 1:** PING → `S1:OK`
- **Slave 2:** PING → `S2:OK`
- **Slave 3:** PING → `S3:OK`

---

## Pengujian

Lihat dokumen lengkap di: `2.1.1_PENGUJIAN_SUB_SISTEM_KENDALI.md`

Total test items: **68** (17 Slave 1, 21 Slave 2, 30 Slave 3)

---

## Status Implementasi

| Komponen | Status | Catatan |
|----------|--------|---------|
| Slave 1 Flow Control | ✅ Tested | Sensor & aktuator OK |
| Slave 2 Level Control | ✅ Tested | Sensor & motor OK |
| Slave 3 Temp Control | ✅ Tested | Sensor, heater, mixer OK |
| Master RS485-only | ✅ Tested | All slave communication OK |
| Master KEPServer | ✅ Ready | Siap integrasi |

---

EOF
