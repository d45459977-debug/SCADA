# PEMBAHASAN_KENDALI_SLAVE.md

**Nomor Dokumen:** B400-CTRL-001
**Judul:** Pembahasan Sistem Kendali untuk Slave 1, 2, dan 3
**Proyek:** SOLOPHOBIA - Heating & Mixing Miniplant (Tahap B400)
**Tanggal:** 2026-05-11

---

## Daftar Isi

1. [Overview Sistem Kendali](#1-overview-sistem-kendali)
2. [Slave 1: Flow Control (TK-101)](#2-slave-1-flow-control-tk-101)
3. [Slave 2: Level Control (TK-201)](#3-slave-2-level-control-tk-201)
4. [Slave 3: Temperature Control (TK-301)](#4-slave-3-temperature-control-tk-301)
5. [Komunikasi Modbus & Integrasi](#5-komunikasi-modbus--integrasi)
6. [Kesimpulan & Rekomendasi](#6-kesimpulan--rekomendasi)

---

## 1. Overview Sistem Kendali

### 1.1 Arsitektur Distributed Control

```
┌─────────────────────────────────────────────────────────────────────┐
│                         MASTER (ESP32)                              │
│                    Modbus TCP/IP Gateway                            │
└───────┬─────────────────┬─────────────────┬────────────────────────┘
        │                 │                 │
    RS-485/RTU        RS-485/RTU        RS-485/RTU
        │                 │                 │
        ▼                 ▼                 ▼
┌───────────────┐  ┌───────────────┐  ┌───────────────┐
│   SLAVE 1     │  │   SLAVE 2     │  │   SLAVE 3     │
│  Flow Control │  │  Level Control│  │  Temp Control │
│  (TK-101)     │  │  (TK-201)     │  │  (TK-301)     │
├───────────────┤  ├───────────────┤  ├───────────────┤
│ Sensor: YFS401│  │Sensor: HC-SR04│  │Sensor: DS18B20│
│ Actuator:Relay│  │Actuator:M-201 │  │Actuator:SSR   │
│ Control: ON/OF│  │Control: Position│ │Control: PID  │
└───────────────┘  └───────────────┘  └───────────────┘
```

### 1.2 Tabel Parameter Kendali

| Slave | Proses | Sensor | Actuator | Setpoint | Deadband | Sampling |
|-------|--------|--------|----------|----------|----------|----------|
| 1 | Flow (mL) | YFS401 | Relay FY-101 | Volume (mL) | - | 500 ms |
| 2 | Level (mL) | HC-SR04 | Motor M-201 | Volume (mL) | ±0.3 cm | 500 ms |
| 3 | Temp (°C) | DS18B20 | SSR TY-301 | Suhu (°C) | ±1°C | 1000 ms |

---

## 2. Slave 1: Flow Control (TK-101)

### 2.1 Deskripsi Proses

**Tujuan:** Mengontrol volume air yang mengalir melalui pompa FY-101.

**Komponen:**
- **Sensor:** YFS401 Hall-Effect Flow Sensor
  - Output: Pulse (50% duty cycle)
  - Range: 1-5 L/min
  - K_pulse: 98 pulse/L (datasheet)
- **Actuator:** Relay FY-101 (kontrol pompa 12V)
- **Proses:** Integrasi flow rate → volume

### 2.2 Plant Model

#### 2.2.1 Persamaan Matematika

```
Flow Rate (mL/min) = (Pulse Count / Sampling Time) × (60000 / K_pulse)
Volume (mL) = ∫ Flow Rate dt = Σ (Flow Rate × Δt)

Dengan:
- Pulse Count: jumlah pulse dalam sampling window
- Sampling Time: 1000 ms (1 detik)
- K_pulse: 98 pulse/L (datasheet) atau nilai kalibrasi
```

#### 2.2.2 Transfer Function

**Domain waktu (kontinu):**
```
G(s) = K_p / s
```
Dimana G(s) adalah plant dari flow rate ke volume (integrator).

**Domain-z (diskrit):**
```
G(z) = K_p × Ts × z^(-1) / (1 - z^(-1))
```
Dengan Ts = 0.5 detik (sampling time).

### 2.3 Strategi Kendali

#### 2.3.1 Pendekatan: Volume Control dengan Feedback

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  Setpoint   │     │ Controller  │     │   Plant     │
│  Volume     │────▶│  (Integral) │────▶│  Pompa +    │
│  (mL)       │     │             │     │  Sensor     │
└─────────────┘     └─────────────┘     └─────────────┘
       │                                     │
       │          ┌─────────────┐           │
       └──────────│   Feedback  │───────────┘
                  │  Volume PV  │
                  └─────────────┘
```

#### 2.3.2 Algoritma Kendali

**Metode: ON/OFF Control dengan Hysteresis**

```cpp
// Parameter
#define VOLUME_SETPOINT 1000    // mL
#define VOLUME_TOLERANCE 50     // mL

// Variabel
float volume_ml = 0;
int pulse_count = 0;
bool pump_active = false;

// Loop (setiap 500 ms)
void loop() {
    // Baca sensor
    pulse_count = read_pulse_count();      // Reset tiap sampling
    flow_ml_min = (pulse_count / 1.0) * (60000.0 / K_PULSE);
    volume_ml += flow_ml_min * (0.5 / 60.0);  // Akumulasi volume

    // Kendali ON/OFF
    if (!pump_active && volume_ml < (VOLUME_SETPOINT - VOLUME_TOLERANCE)) {
        pump_active = true;
        digitalWrite(RELAY_PIN, HIGH);
    }
    else if (pump_active && volume_ml >= VOLUME_SETPOINT) {
        pump_active = false;
        digitalWrite(RELAY_PIN, LOW);
    }

    delay(500);
}
```

#### 2.3.3 Alternatif: PID Control (opsional)

Untuk kontrol presisi flow rate (bukan volume):

```cpp
// PID Parameters
#define KP 2.0
#define KI 0.5
#define KD 0.0

float flow_setpoint = 1000;  // mL/min
float flow_pv = 0;
float error = 0;
float integral = 0;
float last_error = 0;
float output = 0;

void loop() {
    flow_pv = get_flow_rate();  // mL/min
    error = flow_setpoint - flow_pv;

    integral += error * Ts;
    float derivative = (error - last_error) / Ts;

    output = KP * error + KI * integral + KD * derivative;
    output = constrain(output, 0, 100);  // PWM 0-100%

    // Control pump (jika pakai PWM)
    analogWrite(PUMP_PIN, output * 2.55);

    last_error = error;
    delay(500);
}
```

### 2.4 Implementasi Arduino

#### 2.4.1 Pin Assignment

| Pin | Fungsi | Keterangan |
|-----|--------|-----------|
| D2  | INT0   | Input pulse YFS401 |
| D3  | OUTPUT | Kontrol Relay FY-101 |

#### 2.4.2 Flowchart

```
┌─────────┐
│  START  │
└────┬────┘
     │
     ▼
┌─────────────────┐
│ Inisialisasi    │
│ - Setup interrupt│
│ - Reset counter │
└────┬────────────┘
     │
     ▼
┌─────────────────┐     Ya     ┌──────────────┐
│ Baca Pulse Count│────────────▶│ Update Volume│
│ (setiap 500ms)  │             └──────┬───────┘
└─────────────────┘                    │
     │                                │
     │                                ▼
     │ No                    ┌─────────────────┐
     └──────────────────────▶│ Cek Setpoint   │
                              └────┬────────────┘
                                   │
                    ┌──────────────┴──────────────┐
                    │                             │
              Volume < SP-Δ                  Volume >= SP
                    │                             │
                    ▼                             ▼
            ┌───────────────┐             ┌───────────────┐
            │ Relay ON      │             │ Relay OFF     │
            │ (Pompa Jalan) │             │ (Pompa Stop)  │
            └───────┬───────┘             └───────┬───────┘
                    │                             │
                    └──────────────┬──────────────┘
                                   │
                                   ▼
                            ┌─────────────┐
                            │ Kirim ke Modbus│
                            └──────┬──────┘
                                   │
                                   ▼
                            ┌─────────────┐
                            │   Loop      │
                            └─────────────┘
```

### 2.5 Tuning Parameter

| Parameter | Nilai Awal | Cara Tuning |
|-----------|------------|-------------|
| K_pulse | 98 pulse/L | Kalibrasi dengan beban actual |
| Sampling time | 500 ms | Sesuai flow rate min |
| Tolerance | ±50 mL | Sesuai requirement proses |

---

## 3. Slave 2: Level Control (TK-201)

### 3.1 Deskripsi Proses

**Tujuan:** Mengontrol volume texapon yang didosis melalui gerakan plunger.

**Komponen:**
- **Sensor:** HC-SR04 Ultrasonic Distance Sensor
  - Range: 2-400 cm
  - Akurasi: ±3mm
  - Output: Jarak (cm)
- **Actuator:** Motor DC 12V dengan driver TB6612FNG
  - PWM speed control
  - Direction: CW/CCW
- **Mekanisme:** Lead screw menggerakkan plunger naik-turun

### 3.2 Plant Model

#### 3.2.1 Persamaan Matematika

```
Volume (mL) = Luas_Penampang × Tinggi_Texapon
Tinggi_Texapon = Jarak_Dasar - Jarak_Sensor

Dengan:
- Luas_Penampang = 254.1 cm²
- Jarak_Dasar = 21.70 cm (kalibrasi)
- Jarak_Sensor = bacaan HC-SR04
```

#### 3.2.2 Transfer Function Motor

**Model Motor DC (First Order):**
```
G_motor(s) = K / (τs + 1)
```

Dimana:
- K = kecepatan steady state (cm/s per % duty)
- τ = time constant motor (detik)

**Diskrit:**
```
G_motor(z) = K(1-a) × z^(-1) / (1 - a×z^(-1))
```
Dengan a = exp(-Ts/τ).

### 3.3 Strategi Kendali

#### 3.3.1 Pendekatan: Position Control dengan Feedback

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  Setpoint   │     │ Controller  │     │   Plant     │
│  Volume     │────▶│  (Position) │────▶│  Motor +     │
│  (mL)       │     │  + Deadband │     │  Sensor     │
└─────────────┘     └─────────────┘     └─────────────┘
       │                                     │
       │          ┌─────────────┐           │
       └──────────│   Feedback  │───────────┘
                  │  Volume PV  │
                  └─────────────┘
```

#### 3.3.2 Algoritma Kendali

**Metode: Position Control dengan Deadband**

```cpp
// Parameter
#define VOLUME_SETPOINT 3000    // mL
#define DEADBAND 0.3            // cm (±76 mL)
#define MOTOR_SPEED 150         // PWM (0-255)
#define LUAS_PENAMPANG 254.1    // cm²
#define JARAK_DASAR 21.70       // cm

// Variabel
float jarak_sensor = 0;
float volume_ml = 0;
float volume_error = 0;
bool motor_running = false;
int motor_direction = 0;  // 1=UP, -1=DOWN

void loop() {
    // Baca sensor
    jarak_sensor = read_hcsr04_filtered();  // Dengan moving average
    tinggi_texapon = JARAK_DASAR - jarak_sensor;
    volume_ml = tinggi_texapon * LUAS_PENAMPANG;

    // Konversi setpoint volume ke jarak
    float target_tinggi = VOLUME_SETPOINT / LUAS_PENAMPANG;
    float target_jarak = JARAK_DASAR - target_tinggi;

    // Error dalam jarak (cm)
    float jarak_error = jarak_sensor - target_jarak;

    // Kendali dengan deadband
    if (jarak_error > DEADBAND) {
        // Plunger terlalu bawah → naikkan (turunkan jarak sensor)
        motor_direction = 1;  // UP
        motor_running = true;
    }
    else if (jarak_error < -DEADBAND) {
        // Plunger terlalu atas → turunkan (naikkan jarak sensor)
        motor_direction = -1;  // DOWN
        motor_running = true;
    }
    else {
        // Dalam deadband → stop
        motor_running = false;
        motor_direction = 0;
    }

    // Eksekusi motor
    if (motor_running) {
        if (motor_direction == 1) {
            // Motor UP
            digitalWrite(AIN1, HIGH);
            digitalWrite(AIN2, LOW);
            analogWrite(PWMA, MOTOR_SPEED);
        }
        else {
            // Motor DOWN
            digitalWrite(AIN1, LOW);
            digitalWrite(AIN2, HIGH);
            analogWrite(PWMA, MOTOR_SPEED);
        }
        digitalWrite(STBY, HIGH);
    }
    else {
        // Stop dengan short brake
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, HIGH);
        digitalWrite(STBY, LOW);
    }

    delay(500);
}
```

#### 3.3.3 Moving Average Filter untuk HC-SR04

```cpp
#define SAMPLE_COUNT 5

float read_hcsr04_filtered() {
    float samples[SAMPLE_COUNT];
    float sum = 0;

    for (int i = 0; i < SAMPLE_COUNT; i++) {
        samples[i] = read_hcsr04();
        sum += samples[i];
        delay(10);
    }

    return sum / SAMPLE_COUNT;
}
```

### 3.4 Implementasi Arduino

#### 3.4.1 Pin Assignment TB6612FNG

| Pin Arduino | TB6612FNG | Fungsi |
|-------------|-----------|--------|
| D5 | STBY | Standby enable |
| D7 | AIN2 | Motor direction 2 |
| D8 | AIN1 | Motor direction 1 |
| D9 | PWMA | Motor PWM speed |
| 5V | VCC | Logic power |
| GND | GND | Common ground |

| Pin Arduino | HC-SR04 | Fungsi |
|-------------|---------|--------|
| D9 | TRIG | Trigger pulse |
| D10 | ECHO | Echo input |

#### 3.4.2 Flowchart

```
┌─────────┐
│  START  │
└────┬────┘
     │
     ▼
┌─────────────────┐
│ Inisialisasi    │
│ - Setup pins    │
│ - Kalibrasi     │
└────┬────────────┘
     │
     ▼
┌─────────────────┐
│ Baca Jarak      │
│ (dengan filter) │
└────┬────────────┘
     │
     ▼
┌─────────────────┐
│ Hitung Volume   │
│ V = A × h       │
└────┬────────────┘
     │
     ▼
┌─────────────────┐
│ Error = PV - SP  │
└────┬────────────┘
     │
     ▼
      ┌────┴────┐
      │         │
  Error > +Δ  Error < -Δ  │Error│ ≤ Δ
      │         │         │
      ▼         ▼         ▼
┌─────────┐ ┌─────────┐ ┌─────────┐
│Motor UP │ │Motor DWN│ │Motor STOP│
│(Plunger │ │(Plunger │ │(Deadband)│
│ NAIK)   │ │ TURUN)  │ │         │
└────┬────┘ └────┬────┘ └────┬────┘
     │           │           │
     └───────────┴───────────┘
                 │
                 ▼
          ┌─────────────┐
          │ Kirim Modbus│
          └──────┬──────┘
                 │
                 ▼
          ┌─────────────┐
          │   Loop      │
          └─────────────┘
```

### 3.5 Tuning Parameter

| Parameter | Nilai Awal | Cara Tuning |
|-----------|------------|-------------|
| Deadband | ±0.3 cm | Sesuai akurasi sensor |
| Motor speed | 150 PWM | Sesuai lead screw pitch |
| Filter window | 5 sampel | Sesuai noise level |
| Sampling time | 500 ms | Trade-off respon vs noise |

---

## 4. Slave 3: Temperature Control (TK-301)

### 4.1 Deskripsi Proses

**Tujuan:** Mengontrol suhu campuran dengan heater dan mixer.

**Komponen:**
- **Sensor:** DS18B20 Waterproof Temperature Sensor
  - Range: -55°C s/d +125°C
  - Akurasi: ±0.5°C
  - Output: Digital (1-Wire)
- **Actuator 1:** SSR TY-301 (kontrol heater)
- **Actuator 2:** Motor M-301 (mixer)

### 4.2 Plant Model

#### 4.2.1 Persamaan Matematika

**Model Termal (First Order):**
```
dT/dt = (Q_in - Q_out) / (m × Cp)

Dimana:
- Q_in = Power_heater × duty_cycle
- Q_out = UA × (T - T_ambient)
- m = massa fluida (kg)
- Cp = specific heat (J/kg·K)
```

#### 4.2.2 Transfer Function

```
G(s) = K / (τs + 1)

Dimana:
- K = Power_heater / (m × Cp × UA)  [°C per % duty]
- τ = (m × Cp) / UA  [detik]
```

**Contoh untuk 1L air:**
- m = 1 kg, Cp = 4186 J/kg·K
- Power_heater = 500 W
- UA = 5 W/K (heat loss)
- τ ≈ 836 detik (~14 menit)

### 4.3 Strategi Kendali

#### 4.3.1 Pendekatan: Temperature Control dengan PID

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  Setpoint   │     │ Controller  │     │   Plant     │
│  Suhu (°C)  │────▶│     PID     │────▶│  Heater +   │
└─────────────┘     └─────────────┘     │  Sensor     │
       │                                 └─────────────┘
       │          ┌─────────────┐
       └──────────│   Feedback  │───────────┐
                  │  Suhu PV    │           │
                  └─────────────┘           │
                                             ▼
                                      ┌─────────────┐
                                      │   Mixer     │
                                      │  (Timer)    │
                                      └─────────────┘
```

#### 4.3.2 Algoritma Kendali

**Metode: PID Control dengan ON/OFF Heater**

```cpp
// Parameter
#define TEMP_SETPOINT 50.0      // °C
#define TEMP_DEADBAND 1.0       // °C
#define HEATER_PIN 6
#define MIXER_PIN 7
#define MIXING_DURATION 300     // detik

// PID Parameters
#define KP 15.0
#define KI 0.8
#define KD 0.0

// Variabel
float temp_pv = 0;
float temp_error = 0;
float integral = 0;
float last_error = 0;
float pid_output = 0;
bool heater_active = false;
unsigned long mixing_timer = 0;
bool mixing_complete = false;

// Konstanta
const float Ts = 1.0;  // Sampling time (detik)

void loop() {
    // Baca sensor
    sensors.requestTemperatures();
    temp_pv = sensors.getTempCByIndex(0);

    // PID Controller
    temp_error = TEMP_SETPOINT - temp_pv;

    // Anti-windup: reset integral jika error besar
    if (abs(temp_error) > 10.0) {
        integral = 0;
    } else {
        integral += temp_error * Ts;
        // Clamp integral
        integral = constrain(integral, -50, 50);
    }

    float derivative = (temp_error - last_error) / Ts;

    pid_output = KP * temp_error + KI * integral + KD * derivative;
    pid_output = constrain(pid_output, 0, 100);  // %

    // ON/OFF Control untuk heater (SSR tidak bisa PWM cepat)
    if (temp_pv < (TEMP_SETPOINT - TEMP_DEADBAND/2)) {
        heater_active = true;
        digitalWrite(HEATER_PIN, HIGH);
    }
    else if (temp_pv > (TEMP_SETPOINT + TEMP_DEADBAND/2)) {
        heater_active = false;
        digitalWrite(HEATER_PIN, LOW);
    }
    // Dalam deadband, maintain state

    // Mixer control (timer based)
    if (mixing_timer < MIXING_DURATION * 1000UL) {
        digitalWrite(MIXER_PIN, HIGH);  // Mixer ON
        mixing_timer += Ts * 1000UL;
    }
    else {
        digitalWrite(MIXER_PIN, LOW);   // Mixer OFF
        mixing_complete = true;
    }

    last_error = temp_error;
    delay(1000);  // 1 detik sampling
}
```

#### 4.3.3 Alternatif: PWM Control (jika SSR mendukung)

```cpp
// Untuk SSR yang bisa PWM (zero-crossing detection)
void control_heater_pwm() {
    int pwm_value = (int)pid_output;  // 0-100%
    analogWrite(HEATER_PWM_PIN, pwm_value * 2.55);
}
```

### 4.4 Implementasi Arduino

#### 4.4.1 Pin Assignment

| Pin Arduino | Komponen | Fungsi |
|-------------|----------|--------|
| D2 | DS18B20 DATA | 1-Wire data |
| D6 | SSR TY-301 | Heater control |
| D7 | Relay M-301 | Mixer control |

#### 4.4.2 Flowchart

```
┌─────────┐
│  START  │
└────┬────┘
     │
     ▼
┌─────────────────┐
│ Inisialisasi    │
│ - Setup 1-Wire  │
│ - Setup pins    │
└────┬────────────┘
     │
     ▼
┌─────────────────┐
│ Baca Suhu       │
│ (DS18B20)       │
└────┬────────────┘
     │
     ▼
┌─────────────────┐
│ Hitung Error    │
│ e = SP - PV     │
└────┬────────────┘
     │
     ▼
┌─────────────────┐
│ PID Calculate   │
│ u = Kp·e + Ki·∫e │
└────┬────────────┘
     │
     ▼
      ┌────┴────┐
      │         │
  T < SP-Δ/2  T > SP+Δ/2  │Lainnya│
      │         │         │
      ▼         ▼         ▼
┌─────────┐ ┌─────────┐ ┌─────────┐
│Heater ON│ │Heater OFF│ │Maintain │
└────┬────┘ └─────────┘ └────┬────┘
     │                      │
     ▼                      │
┌─────────┐                 │
│Cek Mixer│                 │
│Timer    │                 │
└────┬────┘                 │
     │                      │
     ▼                      │
┌────────────────────────────┘
│
▼
┌─────────────┐
│ Kirim Modbus│
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   Loop      │
└─────────────┘
```

### 4.5 Tuning Parameter

| Parameter | Nilai Awal | Cara Tuning |
|-----------|------------|-------------|
| Kp | 15.0 | Mulai dari kecil, naikkan sampai oscillasi |
| Ki | 0.8 | Tambah untuk eliminasi steady-state error |
| Kd | 0.0 | Biasanya tidak perlu untuk temperature |
| Deadband | ±1°C | Sesuai akurasi sensor |
| Sampling time | 1 s | Trade-off respon vs noise |

**Metode Ziegler-Nichols (jika perlu):**
1. Set Ki = 0, Kd = 0
2. Naikkan Kp sampai oscillasi steady (Ku)
3. Catat periode oscillation (Pu)
4. Hitung:
   - Kp = 0.6 × Ku
   - Ki = 1.2 × Ku / Pu
   - Kd = 0.075 × Ku × Pu

---

## 5. Komunikasi Modbus & Integrasi

### 5.1 Modbus Register Map

| Address | Nama | Slave | Tipe | Deskripsi |
|---------|------|-------|------|-----------|
| 40001 | Volume_SP | 1 | Holding | Setpoint Volume (mL) |
| 40002 | Volume_PV | 1 | Input | Volume Aktual (mL) |
| 40003 | Status_Pompa | 1 | Coil | 0=OFF, 1=ON |
| 40004 | Jarak_SP | 2 | Holding | Setpoint Jarak (cm) |
| 40005 | Jarak_PV | 2 | Input | Jarak Aktual (cm) |
| 40006 | Status_Motor | 2 | Coil | 0=OFF, 1=ON |
| 40007 | Suhu_SP | 3 | Holding | Setpoint Suhu (×0.1°C) |
| 40008 | Suhu_PV | 3 | Input | Suhu Aktual (×0.1°C) |
| 40009 | Status_Heater | 3 | Coil | 0=OFF, 1=ON |
| 40010 | Mixing_Duration | 3 | Holding | Durasi Mixing (s) |
| 40011 | Mixing_Timer | 3 | Input | Timer Mixing (s) |
| 40012 | Status_Mixer | 3 | Coil | 0=OFF, 1=ON |

### 5.2 Komunikasi Master-Slave

```
ESP32 (Master)                 Arduino (Slave 1-3)
─────────────────────────────────────────────────
    │                                  │
    │  ┌─────────────────────────┐     │
    │  │ Query Slave 1           │     │
    │  │ Read Input Register 2   │     │
    │  │ (Volume_PV)             │     │
    │  └─────────────────────────┘     │
    │─────────── Request ──────────────▶│
    │◀────────── Response ──────────────│
    │                                  │
    │  ┌─────────────────────────┐     │
    │  │ Write Coil 3            │     │
    │  │ (Status_Pompa = ON)     │     │
    │  └─────────────────────────┘     │
    │─────────── Request ──────────────▶│
    │◀────────── Response ──────────────│
    │                                  │
    │  [Ulangi untuk Slave 2, 3]       │
```

### 5.3 Timing Komunikasi

| Slave | Polling Interval | Timeout | Retry |
|-------|------------------|---------|-------|
| 1 | 500 ms | 100 ms | 3 |
| 2 | 500 ms | 100 ms | 3 |
| 3 | 1000 ms | 100 ms | 3 |

**Total cycle time:** ~2 detik untuk semua slave

---

## 6. Kesimpulan & Rekomendasi

### 6.1 Ringkasan Strategi Kendali

| Slave | Metode | Alasan |
|-------|--------|---------|
| 1 (Flow) | ON/OFF dengan hysteresis | Relay tidak bisa PWM, sederhana |
| 2 (Level) | Position control + deadband | Motor DC dengan posisi, deadband mengurangi cycling |
| 3 (Temp) | PID + ON/OFF output | Proses thermal lambat, perlu eliminasi error |

### 6.2 Rekomendasi Implementasi

1. **Slave 1 (Flow):**
   - Gunakan ON/OFF control sederhana
   - Tambahkan timeout protection
   - Kalibrasi K_pulse sebelum operasi

2. **Slave 2 (Level):**
   - Implementasi moving average filter (5 sampel)
   - Gunakan short brake saat stop motor
   - Kalibrasi jarak dasar setiap ganti holder

3. **Slave 3 (Temperature):**
   - Gunakan PID dengan anti-windup
   - Implementasi deadband ±1°C
   - Mixer independent dengan timer

### 6.3 Tantangan & Solusi

| Tantangan | Solusi |
|-----------|--------|
| Noise sensor HC-SR04 | Moving average filter |
| Thermal lag (temp) | PID dengan integral action |
| Motor overshoot (level) | Deadband + brake |
| Flow sensor drift | Kalibrasi rutin |
| Modbus latency | Optimize polling interval |

### 6.4 Future Improvements

1. **Adaptive Control:** PID parameter tuning otomatis
2. **Feedforward Control:** Komputasi delay untuk temperature
3. **Fault Detection:** Diagnosa error sensor/actuator
4. **Data Logging:** SD card untuk history
5. **HMI Interface:** Display LCD + tombol

---

## 7. Lampiran

### 7.1 Kode MATLAB Lengkap

Lihat dokumen terpisah:
- `sensor_flow/DATA_KALIBRASI_YFS401.md` — Plant model & simulasi
- `sensor_level/DATA_KALIBRASI_HC-SR04.md` — Motor control & simulasi
- `sensor_temp/DATA_KALIBRASI_DS18B20.md` — Thermal model & simulasi

### 7.2 Referensi

- Modbus RTU Specification
- Arduino PID Library
- TB6612FNG Datasheet
- DS18B20 Datasheet
- YFS401 Flow Sensor Datasheet
- HC-SR04 Ultrasonic Sensor Datasheet

---

**Dibuat oleh:** Hifzhudin
**Tanggal:** 2026-05-11
**Versi:** 1.0
**Status:** FINAL
