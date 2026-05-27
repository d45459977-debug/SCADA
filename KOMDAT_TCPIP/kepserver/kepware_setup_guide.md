# Konfigurasi KEPServerEX untuk SOLOPHOBIA

## Persiapan

1. Pastikan ESP32 sudah running dan terhubung ke WiFi
2. Catat IP address ESP32 (lihat di Serial Monitor)
3. Pastikan PC dan ESP32 di jaringan yang sama

## Langkah-langkah Konfigurasi

### 1. Buat Channel Baru

1. Buka KEPServerEX
2. Klik kanan pada "Channels" → New Channel
3. Pilih: **Modbus TCP/IP Ethernet**
4. Beri nama: `ESP32_Gateway`
5. Klik Next → Finish

### 2. Buat Device

1. Klik kanan pada channel `ESP32_Gateway` → New Device
2. Pilih: **Modbus TCP/IP Ethernet Device**
3. Konfigurasi:
   - **Device ID:** `1` (sesuai MODBUS_SLAVE_ID di ESP32)
   - **IP Address:** `[Masukkan IP ESP32]`
   - **Port:** `502`
4. Klik Next → Apply

### 3. Buat Tags (Manual)

Untuk setiap tag, lakukan:
1. Klik kanan pada device → New Tag
2. Konfigurasi sesuai tabel di bawah

#### Tag Configuration Table

| Tag Name | Address | Data Type | Scan Rate | Description |
|----------|---------|-----------|-----------|-------------|
| **Slave 1 - TK-101** |
| Volume_SP | 40001 | UINT16 | 500 ms | Setpoint Volume Air (mL) |
| Volume_PV | 40002 | UINT16 | 500 ms | Volume Air Aktual (mL) |
| Status_Pompa | 40003 | UINT8 | 500 ms | Status PMP-101 (0=OFF, 1=ON) |
| **Slave 2 - TK-201** |
| Jarak_SP | 40004 | UINT16 | 500 ms | Setpoint Jarak (cm) |
| Jarak_PV | 40005 | UINT16 | 500 ms | Jarak Aktual (cm) |
| Status_Motor | 40006 | UINT8 | 500 ms | Status MTR-201 (0=OFF, 1=ON) |
| **Slave 3 - TK-301** |
| Suhu_SP | 40007 | UINT16 | 500 ms | Setpoint Suhu (°C, raw x10) |
| Suhu_PV | 40008 | INT16 | 500 ms | Suhu Aktual (°C, raw x10) |
| Status_Heater | 40009 | UINT8 | 500 ms | Status HTR-301 (0=OFF, 1=ON) |
| Mixing_Duration | 40010 | UINT16 | 500 ms | Durasi Mixing (detik) |
| Mixing_Timer | 40011 | UINT16 | 500 ms | Timer Mixing (detik) |
| Status_Mixer | 40012 | UINT8 | 500 ms | Status MTR-301 (0=OFF, 1=ON) |

### 3a. Import Tags dari CSV (Alternatif)

1. Klik File → Import Tags
2. Pilih file `solophobia_tags.csv`
3. Mapping kolom sesuai prompt
4. Finish

### 4. Scaling untuk Suhu

Untuk tag Suhu (Suhu_SP, Suhu_PV):
1. Klik dua kali tag → Advanced
2. Pada "Linear Scaling":
   - **Raw Low:** 0
   - **Raw High:** 1000
   - **Scaled Low:** 0.0
   - **Scaled High:** 100.0
3. Klik OK

Atau dengan scale factor:
- **Scale:** 0.1
- **Offset:** 0

### 5. Quality Check

1. Klik kanan pada device → Quality
2. Pastikan status "Good" dan ada koneksi
3. Coba ubah nilai setpoint dari Quick Client
4. Lihat perubahan di Serial Monitor ESP32

## Troubleshooting

### Tidak bisa connect
- Cek IP ESP32 (Serial Monitor)
- Cek firewall PC (port 502)
- Ping dari PC ke ESP32

### Data selalu 0
- Cek address (40001-based vs 0-based)
- KEPware biasanya pakai 40001-based untuk holding register
- ESP32 pakai 0-based (0 = 40001)

### Quality "Bad"
- ESP32 mati atau WiFi disconnect
- Port salah (bukan 502)
- Device ID salah
