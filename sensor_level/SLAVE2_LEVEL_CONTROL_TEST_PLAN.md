# Test Plan Slave 2 - TK-201 Level Control

**Firmware:** `sensor_level/slave2_level_control/slave2_level_control.ino`  
**Board:** Arduino Uno  
**Baudrate:** 9600  
**Tanggal:** 2026-05-26

## 1. Tujuan

Memverifikasi sketch closed-loop Slave 2 yang menggabungkan HC-SR04, motor M-201, dan driver TB6612FNG untuk kendali posisi plunger texapon.

## 2. Setup Hardware

| Komponen | Pin Arduino | Keterangan |
|----------|-------------|------------|
| HC-SR04 TRIG | D4 | Dipindah dari D9 agar tidak konflik dengan PWM motor |
| HC-SR04 ECHO | D10 | Input echo |
| TB6612FNG STBY | D5 | HIGH = driver aktif |
| TB6612FNG AIN2 | D7 | Direction 2 |
| TB6612FNG AIN1 | D8 | Direction 1 |
| TB6612FNG PWMA | D9 | PWM speed motor |

Pastikan GND Arduino, GND TB6612FNG, dan GND PSU 12V tersambung common ground.

## 3. Compile dan Upload

```bash
arduino-cli compile --fqbn arduino:avr:uno sensor_level/slave2_level_control
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:uno sensor_level/slave2_level_control
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=9600
```

## 4. Menu Test Firmware

| Command | Fungsi | Expected Result |
|---------|--------|-----------------|
| `1` | Tampilkan data aktual | `Jarak_PV`, `Volume_PV`, error, dan status motor tampil |
| `2` | Set `Jarak_SP` | Setpoint jarak berubah dan dikunci di range aman |
| `3` | Set `Volume_SP` | Volume dikonversi otomatis menjadi `Jarak_SP` |
| `4` | Test motor UP | Motor bergerak naik 2 detik lalu stop |
| `5` | Test motor DOWN | Motor bergerak turun 2 detik lalu stop |
| `6` | Start/stop auto control | Closed-loop aktif/nonaktif |
| `7` | Test Kendali Proses | Test otomatis ke `Jarak_SP = 15 cm` |
| `s` | Emergency stop | Motor stop dan auto control OFF |
| `m` | Tampilkan menu | Menu muncul ulang |

## 5. Prosedur Test

### 5.1 Test Sensor HC-SR04

1. Buka Serial Monitor 9600 baud.
2. Kirim command `1` sebanyak 10 kali.
3. Catat `Jarak_PV`.
4. Status lulus jika pembacaan stabil dan tidak sering `INVALID`.

### 5.2 Test Motor Manual

1. Kirim command `4` untuk test motor UP.
2. Kirim command `5` untuk test motor DOWN.
3. Jika arah terbalik, balik logika `AIN1/AIN2` di firmware atau swap kabel motor.
4. Status lulus jika motor bergerak dua arah dan short brake menghentikan motor cepat.

### 5.3 Test Closed-Loop Manual

1. Kirim command `2`, set `Jarak_SP = 20`.
2. Kirim command `6` untuk start auto control.
3. Tunggu sampai error masuk deadband `+-0.3 cm`, lalu kirim `6` atau `s`.
4. Ulangi untuk `Jarak_SP = 15` dan `Jarak_SP = 10`.
5. Status lulus jika motor berhenti saat target tercapai dan tidak hunting berlebihan.

### 5.4 Test Kendali Proses

1. Kirim command `7`.
2. Firmware otomatis set `Jarak_SP = 15.00 cm`.
3. Catat output:

```text
t_ms,SP_cm,PV_cm,Err_cm,VolPV_mL,Motor
```

4. Test berhenti otomatis saat masuk deadband atau timeout 60 detik.
5. Ulangi minimal 3 kali.

## 6. Template Hasil

| Trial | Command | SP cm | PV Akhir cm | Error Akhir cm | Waktu Respon s | Timeout | Status |
|-------|---------|-------|-------------|----------------|----------------|---------|--------|
| 1 | `7` | 15.0 | | | | Tidak/Ya | |
| 2 | `7` | 15.0 | | | | Tidak/Ya | |
| 3 | `7` | 15.0 | | | | Tidak/Ya | |

## 7. Kriteria Lulus

| Parameter | Kriteria |
|-----------|----------|
| Compile | Berhasil untuk `arduino:avr:uno` |
| Sensor | Pembacaan valid pada range kerja |
| Motor | UP/DOWN berfungsi dan stop cepat |
| Closed-loop | Berhenti dalam deadband `+-0.3 cm` |
| Test proses | Command `7` selesai tanpa timeout pada kondisi mekanik normal |

