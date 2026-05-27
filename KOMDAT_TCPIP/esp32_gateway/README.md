# ESP32 Modbus TCP Simulator - SOLOPHOBIA

Simulasi data dummy untuk testing koneksi ESP32 ke KEPServerEX via Modbus TCP.

## Fitur

- Expose 12 holding registers via Modbus TCP (port 502)
- Menu Serial Monitor untuk mengubah data dummy
- Simulasi otomatis (flow, level, temperature, mixing)
- Cocok untuk testing KEPware sebelum hardware siap

## Setup WiFi

Edit file `esp32_simulasi_dummy.ino`, bagian:

```cpp
const char* ssid = "NAMA_WIFI";      // Ganti dengan WiFi Anda
const char* password = "PASSWORD_WIFI";  // Ganti dengan password WiFi
```

## Compile & Upload

### Menggunakan PlatformIO (Rekomendasi)

```bash
cd KOMDAT_TCPIP/esp32_gateway
pio run --target upload
```

### Menggunakan Arduino IDE

1. Install library: **ModbusTCP** by Jacob Jarick
2. Board: ESP32 Dev Module
3. Upload sketch

## Cara Pakai

### 1. Buka Serial Monitor

- Baudrate: **115200**
- Line ending: **No line ending**

### 2. Tunggu Koneksi

Setelah boot, akan muncul:

```
========================================
 ESP32 MODBUS TCP SIMULATOR - SOLOPHOBIA
========================================

WiFi connected!
IP: 192.168.1.100

Modbus TCP Server started on port 502
IP Address: 192.168.1.100

========== MENU ==========
...
```

**Catat IP address ini untuk konfigurasi KEPware!**

### 3. Menu Serial Monitor

| Command | Fungsi |
|---------|--------|
| `1` | Menu Slave 1 (Flow Control) - ubah volume, toggle pompa |
| `2` | Menu Slave 2 (Level Control) - ubah jarak, toggle motor |
| `3` | Menu Slave 3 (Temperature) - ubah suhu, toggle heater/mixer |
| `s` | Toggle simulasi otomatis ON/OFF |
| `r` | Reset semua data ke default |
| `d` | Display semua data saat ini |
| `m` | Print menu ini |

### 4. Contoh Penggunaan

```
> 1
--- SLAVE 1 - TK-101 (Flow Control) ---
1. Set Volume SP (mL)
2. Set Volume PV (mL)
3. Toggle Pompa (0/1)
Pilih: 1
Masukkan Volume SP (0-1500 mL): 1500

> d
========== DATA DUMMY ==========

--- Slave 1 - TK-101 (Flow) ---
Volume SP:   1500 mL
Volume PV:   0 mL
Pompa:       OFF

> s
Simulasi: ON

> 3
--- SLAVE 3 - TK-301 (Temperature) ---
...
Pilih: 3
Status Heater: ON
```

## Register Map

| Address | Name | Type | KEPware Address |
|---------|------|------|-----------------|
| 0 | Volume_SP | uint16 | 40001 |
| 1 | Volume_PV | uint16 | 40002 |
| 2 | Status_Pompa | uint8 | 40003 |
| 3 | Jarak_SP | uint16 | 40004 |
| 4 | Jarak_PV | uint16 | 40005 |
| 5 | Status_Motor | uint8 | 40006 |
| 6 | Suhu_SP | uint16 | 40007 |
| 7 | Suhu_PV | int16 | 40008 |
| 8 | Status_Heater | uint8 | 40009 |
| 9 | Mixing_Duration | uint16 | 40010 |
| 10 | Mixing_Timer | uint16 | 40011 |
| 11 | Status_Mixer | uint8 | 40012 |

**Catatan Suhu:** Disimpan x10 (60.5°C = 605). Scaling dikerjakan di KEPware (scale 0.1).

## Testing dengan KEPware

Lihat panduan lengkap di: `../kepserver/kepware_setup_guide.md`

Singkatnya:
1. Buat Channel: Modbus TCP/IP Ethernet
2. Buat Device: IP ESP32, Port 502, Device ID 1
3. Buat Tags sesuai CSV atau manual
4. Quality check: harus "Good"
5. Quick Client: coba baca/tulis data

## Troubleshooting

### WiFi tidak connect
- Cek nama WiFi dan password
- Pastikan ESP32 dalam range WiFi
- Coba restart ESP32

### KEPware tidak connect
- Cek IP ESP32 di Serial Monitor
- Ping dari PC: `ping 192.168.1.100`
- Cek firewall Windows (port 502)

### Data di KEPware salah/0
- Cek Address: 0-based (ESP32) vs 40001-based (KEPware)
- Cek Data Type: UINT16 vs INT16 untuk suhu
- Cek scaling untuk suhu (x0.1)
