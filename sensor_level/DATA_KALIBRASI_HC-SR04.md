# DATA_KALIBRASI_HC-SR04.md

**Nomor Dokumen:** B400-SENS-003-KAL
**Sensor:** HC-SR04 Ultrasonic Distance Sensor
**Aplikasi:** TK-201 Level Control (Dosing Texapon)
**Tanggal Kalibrasi:** 2026-05-06

---

## 1. Data Kalibrasi Mentah

### 1.1 Kalibrasi Jarak Dasar (Tanpa Plunger)

| Sampel | Jarak (cm) | Catatan |
|--------|------------|---------|
| 1 | 21.54 | |
| 2 | 21.78 | |
| 3 | 21.65 | |
| 4 | 21.72 | |
| 5 | 21.69 | |
| 6 | 21.75 | |
| 7 | 21.71 | |
| 8 | 21.68 | |
| 9 | 21.73 | |
| 10 | 21.76 | |
| **Mean** | **21.70** | |
| **Std Dev** | **0.07** | |

**Jarak Sensor → Dasar Wadah:** 21.70 cm

### 1.2 Kalibrasi Jarak Penuh (Dengan Plunger di Atas)

| Sampel | Jarak (cm) | Catatan |
|--------|------------|---------|
| 1 | 2.01 | Awal |
| 2 | 2.02 | |
| 3 | 2.01 | |
| 4 | 2.02 | |
| 5 | 2.66 | Final (setelah remounting) |
| 6 | 2.65 | |
| 7 | 2.67 | |
| 8 | 2.66 | |
| 9 | 2.65 | |
| 10 | 2.66 | |
| **Mean (Final)** | **2.66** | |
| **Std Dev** | **0.01** | |

**Jarak Sensor → Plunger (Penuh):** 2.66 cm (setelah remounting)

### 1.3 Pengujian Validasi Akurasi

| Test | Target Jarak (cm) | Bacaan Sensor (cm) | Error (cm) | Error (%) | Status |
|------|-------------------|--------------------|------------|-----------|--------|
| 1 | 8.0 | 7.51 | -0.49 | -6.1 | ⚠️ Acceptable |
| 2 | 8.5 | 8.45 | -0.05 | -0.6 | ✅ Good |

### 1.4 Pengujian Volume (Setelah Kalibrasi)

| Test | Jarak Sensor (cm) | Selisih Jarak (cm) | Tinggi Texapon (cm) | Volume (mL) | Persen (%) |
|------|-------------------|--------------------|---------------------|-------------|------------|
| 1 | 7.87 | 5.21 | 17.39 | 4419 | 77.0 |
| 2 | 13.70 | 11.04 | 11.56 | 2938 | 51.2 |
| 3 | 16.96 | 14.30 | 8.30 | 2110 | 36.7 |
| 4 | 20.49 | 17.83 | 4.77 | 1212 | 21.1 |
| 5 | 7.13 | 4.47 | 18.13 | 4607 | 80.2 |

---

## 2. Parameter Wadah TK-201

| Parameter | Nilai | Satuan |
|-----------|-------|--------|
| Panjang | 22.0 | cm |
| Lebar rata-rata | 11.55 | cm |
| **Luas Penampang** | **254.1** | cm² |
| Tinggi maksimal | [ISI] | cm |
| **Volume maksimal** | **[ISI]** | mL |
| Jarak sensor ke dasar | 21.70 | cm |
| Jarak sensor ke plunger (penuh) | 2.66 | cm |

---

## 3. Hasil Kalibrasi

### 3.1 Persamaan Konversi

```
Volume (mL) = Luas_Penampang × Tinggi_Texapon
Volume (mL) = 254.1 × (Jarak_Penuh - Jarak_Sensor + Offset)

Dimana:
- Jarak_Penuh = 21.70 cm (jarak sensor ke dasar)
- Jarak_Sensor = bacaan HC-SR04 (cm)
- Offset = 2.66 cm (kalibrasi posisi plunger penuh)

Contoh perhitungan:
  Tinggi_Texapon = 21.70 - 7.87 = 13.83 cm (salah, harusnya 21.70 - 2.66 = 19.04 cm range)
  
  Koreksi:
  Tinggi_Texapon = Jarak_Dasar - Jarak_Sensor
  Volume = 254.1 × Tinggi_Texapon
  
  Untuk Jarak_Sensor = 7.87 cm:
  Tinggi_Texapon = 21.70 - 7.87 = 13.83 cm
  Volume = 254.1 × 13.83 = 3514 mL
  
  Catatan: Dalam praktik, dipakai referensi jarak plunger penuh
```

### 3.2 Akurasi Sensor

| Parameter | Nilai | Status |
|-----------|-------|--------|
| Error rata-rata | -0.27 | cm |
| Error maksimum | -0.49 | cm |
| Std Dev (kalibrasi) | 0.07 | cm |
| Deadband sistem | ±0.3 | cm |
| Memenuhi toleransi | YA | ✅ |

---

## 4. Kode MATLAB - Analisis Kalibrasi

### 4.1 Plot Data Kalibrasi

```matlab
%% HC-SR04 Ultrasonic Sensor - Kalibrasi Data Analysis
% Dokumen: B400-SENS-003-KAL
% Sensor: HC-SR04 Ultrasonic Distance Sensor
% Aplikasi: TK-201 Level Control (Dosing Texapon)

clear all; close all; clc;

%% 1. Parameter Wadah TK-201
panjang = 22.0;           % cm
lebar_rata = 11.55;       % cm
luas_penampang = panjang * lebar_rata;  % cm²
tinggi_maksimal = 22.6;   % cm (manual measurement)
volume_maksimal = luas_penampang * tinggi_maksimal;  % mL

fprintf('=== PARAMETER WADAH TK-201 ===\n');
fprintf('Panjang: %.2f cm\n', panjang);
fprintf('Lebar rata-rata: %.2f cm\n', lebar_rata);
fprintf('Luas Penampang: %.2f cm²\n', luas_penampang);
fprintf('Tinggi Maksimal: %.2f cm\n', tinggi_maksimal);
fprintf('Volume Maksimal: %.2f mL\n\n', volume_maksimal);

%% 2. Input Data Kalibrasi
% Data dari pengukuran aktual 2026-05-06

% Jarak dasar (tanpa plunger) - cm
jarak_dasar = [21.54, 21.78, 21.65, 21.72, 21.69, 21.75, 21.71, 21.68, 21.73, 21.76];

% Jarak penuh (dengan plunger di atas) - cm
jarak_penuh = [2.01, 2.02, 2.01, 2.02, 2.66, 2.65, 2.67, 2.66, 2.65, 2.66];

% Validasi akurasi - [Target, Bacaan]
validasi_target = [8.0, 8.5];  % cm
validasi_bacaan = [7.51, 8.45];  % cm

% Pengujian volume - [Jarak Sensor dalam cm]
jarak_sensor_test = [7.87, 13.70, 16.96, 20.49, 7.13];

%% 3. Analisis Kalibrasi Dasar
mean_dasar = mean(jarak_dasar);
std_dasar = std(jarak_dasar);
mean_penuh = mean(jarak_penuh);
std_penuh = std(jarak_penuh);

fprintf('=== HASIL KALIBRASI JARAK ===\n');
fprintf('Jarak Dasar: %.2f ± %.2f cm\n', mean_dasar, std_dasar);
fprintf('Jarak Penuh: %.2f ± %.2f cm\n', mean_penuh, std_penuh);
fprintf('Stroke Plunger: %.2f cm\n', mean_dasar - mean_penuh);

%% 4. Analisis Validasi Akurasi
error_validasi = validasi_bacaan - validasi_target;
error_persen = error_validasi ./ validasi_target * 100;

fprintf('\n=== VALIDASI AKURASI ===\n');
fprintf('Target\tBacaan\tError\tError %%\n');
for i = 1:length(validasi_target)
    fprintf('%.1f\t%.2f\t%.2f\t%.1f%%\n', ...
        validasi_target(i), validasi_bacaan(i), error_validasi(i), error_persen(i));
end
fprintf('Error rata-rata: %.2f cm\n', mean(abs(error_validasi)));

%% 5. Konversi Jarak ke Volume
% Volume = Luas × Tinggi
% Tinggi = Jarak_Penuh - Jarak_Sensor + Offset

offset = mean_penuh;  % Offset kalibrasi

for i = 1:length(jarak_sensor_test)
    tinggi_texapon = mean_dasar - jarak_sensor_test(i);
    volume_texapon = luas_penampang * tinggi_texapon;
    persen = volume_texapon / volume_maksimal * 100;
    fprintf('Jarak: %.2f cm → Volume: %.2f mL (%.1f%%)\n', ...
        jarak_sensor_test(i), volume_texapon, persen);
end

%% 6. Plot Hasil Kalibrasi
figure(1);

% Plot 1: Distribusi Jarak Dasar
subplot(2,3,1);
histogram(jarak_dasar, 5, 'FaceAlpha', 0.7);
hold on;
xline(mean_dasar, 'r--', 'Mean', 'LineWidth', 2);
xline(mean_dasar + std_dasar, 'g--', '+1σ', 'LineWidth', 1.5);
xline(mean_dasar - std_dasar, 'g--', '-1σ', 'LineWidth', 1.5);
grid on;
xlabel('Jarak (cm)');
ylabel('Frekuensi');
title('Distribusi Jarak Dasar');

% Plot 2: Distribusi Jarak Penuh
subplot(2,3,2);
histogram(jarak_penuh, 5, 'FaceAlpha', 0.7);
hold on;
xline(mean_penuh, 'r--', 'Mean', 'LineWidth', 2);
xline(mean_penuh + std_penuh, 'g--', '+1σ', 'LineWidth', 1.5);
xline(mean_penuh - std_penuh, 'g--', '-1σ', 'LineWidth', 1.5);
grid on;
xlabel('Jarak (cm)');
ylabel('Frekuensi');
title('Distribusi Jarak Penuh');

% Plot 3: Validasi Linearity
subplot(2,3,3);
scatter(validasi_target, validasi_bacaan, 100, 'filled', 'MarkerFaceAlpha', 0.6);
hold on;
plot([0 20], [0 20], 'r--', 'LineWidth', 2);  % Perfect line
grid on;
xlabel('Target (cm)');
ylabel('Bacaan (cm)');
title('Validasi Linearity');
legend('Data', 'Ideal', 'Location', 'best');
xlim([4 16]);
ylim([4 16]);

% Plot 4: Error vs Target
subplot(2,3,4);
scatter(validasi_target, error_validasi, 100, 'filled', 'MarkerFaceAlpha', 0.6);
hold on;
xline(0, 'r--', 'LineWidth', 2);
grid on;
xlabel('Target (cm)');
ylabel('Error (cm)');
title('Error vs Target');

% Plot 5: Hubungan Jarak-Volume
subplot(2,3,5);
jarak_plot = 0:0.1:mean_dasar;
volume_plot = luas_penampang * (mean_dasar - jarak_plot);
plot(jarak_plot, volume_plot, 'b-', 'LineWidth', 2);
hold on;
% Plot data test
if exist('jarak_sensor_test', 'var')
    for i = 1:length(jarak_sensor_test)
        vol_test = luas_penampang * (mean_dasar - jarak_sensor_test(i));
        scatter(jarak_sensor_test(i), vol_test, 100, 'r', 'filled', 'MarkerFaceAlpha', 0.6);
    end
end
grid on;
xlabel('Jarak Sensor (cm)');
ylabel('Volume (mL)');
title('Konversi Jarak → Volume');

% Plot 6: Tinggi vs Volume
subplot(2,3,6);
tinggi_plot = 0:0.1:tinggi_maksimal;
volume_tinggi_plot = luas_penampang * tinggi_plot;
plot(tinggi_plot, volume_tinggi_plot, 'b-', 'LineWidth', 2);
hold on;
scatter([tinggi_maksimal], [volume_maksimal], 100, 'r', 'filled');
grid on;
xlabel('Tinggi Texapon (cm)');
ylabel('Volume (mL)');
title('Kurva Volume TK-201');

sgtitle('HC-SR04 - Laporan Kalibrasi', 'FontSize', 14, 'FontWeight', 'bold');

%% 7. Export Results
kalibrasi.jarak_dasar = mean_dasar;
kalibrasi.jarak_penuh = mean_penuh;
kalibrasi.luas_penampang = luas_penampang;
kalibrasi.tinggi_maksimal = tinggi_maksimal;
kalibrasi.volume_maksimal = volume_maksimal;

% Simpan ke workspace
% save('kalibrasi_hcsr04_results.mat', 'kalibrasi');

fprintf('\n=== KONSTANTA KALIBRASI UNTUK ARDUINO ===\n');
fprintf('const float JARAK_DASAR = %.2f;  // cm\n', mean_dasar);
fprintf('const float JARAK_PENUH = %.2f;  // cm\n', mean_penuh);
fprintf('const float LUAS_PENAMPANG = %.2f;  // cm²\n', luas_penampang);
fprintf('const float VOLUME_MAX = %.2f;  // mL\n', volume_maksimal);
```

---

## 5. Kode MATLAB - Simulasi Sistem Kendali Level

### 5.1 Plant Model (HC-SR04 + Motor Lead Screw)

```matlab
%% HC-SR04 Level Control - Plant Model
% Dokumen: B400-SENS-003-KAL
% Aplikasi: TK-201 Level Control (Slave 2)
% Sistem: Motor DC + Lead Screw untuk menggerakkan plunger

clear all; close all; clc;

%% 1. Parameter Wadah TK-201
Luas = 254.1;           % cm²
Tinggi_max = 22.6;      % cm
Volume_max = Luas * Tinggi_max;  % mL

%% 2. Parameter Motor M-201 + Lead Screw
% Motor DC 12V dengan driver TB6612FNG
V_motor = 12;           % Volt
RPM_no_load = 120;      % RPM @ 12V (geared)
Gear_ratio = 50;        % Redaksi gear
Lead_screw_pitch = 2.0; % mm/rev (pitch lead screw)

% Kecepatan linear motor (cm/s)
RPM_actual = RPM_no_load * 0.7;  % Dengan load
speed_cm_per_sec = (RPM_actual / 60) * (Lead_screw_pitch / 10) / Gear_ratio;

fprintf('=== PARAMETER MOTOR M-201 ===\n');
fprintf('Speed: %.2f cm/s\n', speed_cm_per_sec);

%% 3. Parameter Sensor HC-SR04
K_sensor = 1.0;         % unity gain (cm)
sampling_time = 0.5;    % detik
sensor_delay = 0.05;    % detik (propagation delay)
sensor_noise = 0.2;     % cm (std dev)

%% 4. Discrete-time Plant Model
% Model motor sebagai integrator dengan delay
% Position(s) = Speed * u(s)

Ts = 0.5;              % sampling time (detik)
tau_motor = 0.5;       % time constant motor
a = exp(-Ts/tau_motor);
K_motor = speed_cm_per_sec * (1 - a);

% Motor model (first order + delay)
num_motor = [0, K_motor];
den_motor = [1, -a];
motor = tf(num_motor, den_motor, Ts);

% Integrator (jarak → volume)
% Volume = Luas * (Tinggi_max - Jarak)
% Volume_rate = Luas * d(Jarak)/dt

%% 5. Simulasi Respons Step
t = 0:Ts:60;           % 60 detik simulasi

% Input: perintah turunkan plunger (posisi)
% Target: volume tertentu
target_volume = 3000;  % mL
target_tinggi = target_volume / Luas;  % cm
target_jarak = Tinggi_max - target_tinggi;  % cm

% Step input (motor ON)
u = zeros(size(t));
u(t >= 5 & t <= 25) = 100;  % ON dari t=5 sampai t=25

% Simulasi respon posisi
[y_pos, t_pos] = lsim(motor, u, t);

% Konversi posisi ke jarak sensor
jarak_sensor = Tinggi_max - y_pos;

% Simulasi sensor dengan noise
jarak_sensor_noisy = jarak_sensor + sensor_noise * randn(size(jarak_sensor));

% Konversi ke volume
volume_actual = Luas * (Tinggi_max - jarak_sensor);
volume_sensor = Luas * (Tinggi_max - jarak_sensor_noisy);

%% 6. Plot Hasil Simulasi
figure(2);

% Plot 1: Motor Input
subplot(4,1,1);
stairs(t, u, 'r-', 'LineWidth', 2);
grid on;
xlabel('Waktu (detik)');
ylabel('Motor (%)');
title('Input Motor - Turunkan Plunger');
ylim([-10 110]);

% Plot 2: Posisi Plunger
subplot(4,1,2);
plot(t, y_pos, 'b-', 'LineWidth', 2);
hold on;
yline(target_tinggi, 'g--', 'Target', 'LineWidth', 2);
grid on;
xlabel('Waktu (detik)');
ylabel('Posisi (cm)');
title('Posisi Plunger (dari atas)');

% Plot 3: Jarak Sensor
subplot(4,1,3);
plot(t, jarak_sensor, 'b-', 'LineWidth', 2);
hold on;
plot(t, jarak_sensor_noisy, 'g--', 'LineWidth', 1);
yline(target_jarak, 'r--', 'Target', 'LineWidth', 2);
grid on;
xlabel('Waktu (detik)');
ylabel('Jarak (cm)');
title('Bacaan Sensor HC-SR04');
legend('Actual', 'With Noise', 'Target', 'Location', 'best');

% Plot 4: Volume
subplot(4,1,4);
plot(t, volume_actual, 'b-', 'LineWidth', 2);
hold on;
plot(t, volume_sensor, 'g--', 'LineWidth', 1);
yline(target_volume, 'r--', 'Target', 'LineWidth', 2);
grid on;
xlabel('Waktu (detik)');
ylabel('Volume (mL)');
title('Volume Texapon');
legend('Actual', 'Sensor', 'Target', 'Location', 'best');
ylim([0 Volume_max + 500]);

sgtitle('Simulasi Plant Level Control (TK-201)', 'FontSize', 14, 'FontWeight', 'bold');

%% 7. Transfer Function untuk Control System Design
fprintf('\n=== TRANSFER FUNCTION PLANT ===\n');
fprintf('Motor Model:\n');
fprintf('  Position(z) = %.4f / (1 - %.4f*z^(-1)) * Motor(z)\n', K_motor, a);
fprintf('  Speed: %.2f cm/s\n', speed_cm_per_sec);

fprintf('\nVolume Model:\n');
fprintf('  Volume = Luas × (Tinggi_max - Position)\n');
fprintf('  Volume = %.2f × (%.2f - Position)\n', Luas, Tinggi_max);

fprintf('\nSensor Model:\n');
fprintf('  Jarak_sensor = Jarak_actual + noise(σ=%.2f)\n', sensor_noise);
```

### 5.2 PID Controller Simulation

```matlab
%% HC-SR04 Level Control - PID Controller Simulation
% Closed-loop simulation untuk volume control TK-201

clear all; close all; clc;

%% 1. Parameter Sistem
Luas = 254.1;           % cm²
Tinggi_max = 22.6;      % cm
Volume_max = Luas * Tinggi_max;  % mL

% Motor
speed_cm_per_sec = 0.5; % cm/s (slow untuk akurasi)

% Sensor
sensor_noise = 0.2;     % cm

%% 2. Plant Model
Ts = 0.5;              % sampling time
tau_motor = 1.0;       % time constant (lebih lambat untuk akurasi)
K_motor = speed_cm_per_sec * (1 - exp(-Ts/tau_motor));

% Motor plant
num_motor = [0, K_motor];
den_motor = [1, -exp(-Ts/tau_motor)];
motor = tf(num_motor, den_motor, Ts);

%% 3. PID Controller Design
% Deadband: ±0.3 cm (sekitar ±76 mL)
Kp = 8.0;              % Proportional gain
Ki = 0.3;              % Integral gain
Kd = 0.0;              % Derivative (biasanya 0 untuk level)

% PID discrete
num_pid = [Kp + Ki*Ts + Kd/Ts, -(Kp + 2*Kd/Ts), Kd/Ts];
den_pid = [1, -1];
pid = tf(num_pid, den_pid, Ts);

%% 4. Closed-loop System
% Feedback: jarak sensor → motor → posisi → volume → jarak sensor

% Forward path: PID → Motor → Position → Volume → Jarak Sensor
K_conv = Luas;  % Konversi posisi ke volume (dan vice versa)

% Closed loop dengan satuan "jarak"
% Setpoint dalam jarak, PV dalam jarak
open_loop = pid * motor;
closed_loop = feedback(open_loop, 1);

%% 5. Simulation
T_final = 100;  % detik
t = 0:Ts:T_final;

% Reference: volume setpoint 3000 mL
ref_volume = 3000;  % mL
ref_jarak = Tinggi_max - ref_volume / Luas;  % cm

% Step input
ref = ref_jarak * ones(size(t));

% Step response
[y, t_sim, x] = lsim(closed_loop, ref, t);

% Tambahkan noise sensor
y_noisy = y + sensor_noise * randn(size(y));

%% 6. Calculate Control Signals
error = ref - y_noisy;
u = lsim(pid, error, t);

% Deadband implementation
deadband = 0.3;  % cm
u(abs(error) < deadband) = 0;  % Matikan motor jika dalam deadband

% Saturation
u_sat = max(-100, min(100, u));

%% 7. Konversi ke Volume
volume_pv = Luas * (Tinggi_max - y);
volume_pv_noisy = Luas * (Tinggi_max - y_noisy);
volume_sp = ref_volume * ones(size(t));

%% 8. Plot Results
figure(3);

% Plot 1: Volume Response
subplot(3,1,1);
plot(t, volume_sp, 'r--', 'LineWidth', 2);
hold on;
plot(t, volume_pv_noisy, 'b-', 'LineWidth', 1.5);
yline(ref_volume + 76, 'g--', 'Deadband +', 'LineWidth', 1);
yline(ref_volume - 76, 'g--', 'Deadband -', 'LineWidth', 1);
grid on;
xlabel('Waktu (detik)');
ylabel('Volume (mL)');
title('Closed-Loop Response - Level Control (TK-201)');
legend('Setpoint', 'PV (Sensor)', 'Location', 'best');
ylim([ref_volume - 500, ref_volume + 500]);

% Plot 2: Control Output (Motor)
subplot(3,1,2);
stairs(t, u_sat, 'g-', 'LineWidth', 2);
hold on;
yline(100, 'r--', 'Max CW');
yline(-100, 'r--', 'Max CCW');
yline(0, 'k--');
grid on;
xlabel('Waktu (detik)');
ylabel('Motor (%)');
title('Control Output - Motor M-201');
ylim([-110, 110]);

% Plot 3: Error
subplot(3,1,3);
plot(t, error, 'm-', 'LineWidth', 1.5);
hold on;
yline(deadband, 'g--', 'Deadband +', 'LineWidth', 1);
yline(-deadband, 'g--', 'Deadband -', 'LineWidth', 1);
yline(0, 'k--', 'LineWidth', 1);
grid on;
xlabel('Waktu (detik)');
ylabel('Error Jarak (cm)');
title('Error - Setpoint vs PV');

sgtitle('Level Control - Volume Dosing Texapon', 'FontSize', 14, 'FontWeight', 'bold');

%% 9. Performance Metrics
settling_time_idx = find(abs(volume_pv_noisy - ref_volume) < 76, 1);
if ~isempty(settling_time_idx)
    settling_time = t(settling_time_idx);
else
    settling_time = NaN;
end

overshoot = (max(volume_pv_noisy) - ref_volume) / ref_volume * 100;
steady_state_error = volume_pv_noisy(end) - ref_volume;

fprintf('=== PERFORMANCE METRICS ===\n');
fprintf('Setpoint Volume: %.2f mL\n', ref_volume);
fprintf('Settling Time: %.2f detik\n', settling_time);
fprintf('Overshoot: %.2f%%\n', overshoot);
fprintf('Steady State Error: %.2f mL\n', steady_state_error);
fprintf('Final Volume: %.2f mL\n', volume_pv_noisy(end));
fprintf('Deadband: ±%.2f cm (±%.2f mL)\n', deadband, deadband * Luas);
```

---

## 6. Placeholder Screenshot MATLAB

### 6.1 Screenshot Plot Kalibrasi

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                     [PASTE SCREENSHOT DISINI]                      │
│                  (Plot Kalibrasi HC-SR04)                           │
│                                                                     │
│  Tips:                                                               │
│  1. Run kode MATLAB di Section 4.1                                  │
│  2. Capture figure(1)                                               │
│  3. Paste screenshot di sini                                        │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 6.2 Screenshot Simulasi Plant

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                     [PASTE SCREENSHOT DISINI]                      │
│                 (Simulasi Plant Level Control)                     │
│                                                                     │
│  Tips:                                                               │
│  1. Run kode MATLAB di Section 5.1                                  │
│  2. Capture figure(2)                                               │
│  3. Paste screenshot di sini                                        │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 6.3 Screenshot Closed-Loop Response

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                     [PASTE SCREENSHOT DISINI]                      │
│              (PID Controller - Level Control Response)              │
│                                                                     │
│  Tips:                                                               │
│  1. Run kode MATLAB di Section 5.2                                  │
│  2. Capture figure(3)                                               │
│  3. Paste screenshot di sini                                        │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 7. Catatan Penting

1. **Isi data kalibrasi** di Section 1 sebelum menjalankan kode MATLAB
2. **Update parameter wadah** jika ada perubahan dimensi TK-201
3. **Deadband sistem: ±0.3 cm** (sekitar ±76 mL)
4. **Kalibrasi ulang diperlukan** setiap kali posisi sensor/hander berubah

---

**Dibuat oleh:** Hifzhudin
**Tanggal:** 2026-05-06
**Status:** COMPLETE
