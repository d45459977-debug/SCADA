# DATA_KALIBRASI_YFS401.md

**Nomor Dokumen:** B400-SENS-002-KAL
**Sensor:** YFS401 Hall-Effect Flow Sensor
**Aplikasi:** TK-101 Flow Control (Pompa Air)
**Tanggal Kalibrasi:** [BELUM ADA - PENDING PENGUJIAN]

---

## 1. Data Kalibrasi Mentah

### 1.1 Pengujian Flow Rate vs Pulse Count

| No | Target Volume (mL) | Actual Volume (mL) | Time (s) | Pulse Count | Flow (mL/min) | Pulse/L | Keterangan |
|----|-------------------|--------------------|----------|-------------|---------------|---------|------------|
| 1 | 100 | | | | | | |
| 2 | 200 | | | | | | |
| 3 | 300 | | | | | | |
| 4 | 400 | | | | | | |
| 5 | 500 | | | | | | |
| 6 | 600 | | | | | | |
| 7 | 700 | | | | | | |
| 8 | 800 | | | | | | |
| 9 | 900 | | | | | | |
| 10 | 1000 | | | | | | |
| 11 | 1100 | | | | | | |
| 12 | 1200 | | | | | | |
| 13 | 1300 | | | | | | |
| 14 | 1400 | | | | | | |
| 15 | 1500 | | | | | | |

### 1.2 Pengujian Repeatability (10x pengukuran @ 1000 mL)

| No | Volume (mL) | Pulse Count | Flow (mL/min) | Pulse/L |
|----|-------------|-------------|---------------|---------|
| 1 | | | | |
| 2 | | | | |
| 3 | | | | |
| 4 | | | | |
| 5 | | | | |
| 6 | | | | |
| 7 | | | | |
| 8 | | | | |
| 9 | | | | |
| 10 | | | | |
| **Mean** | | | | |
| **Std Dev** | | | | |
| **CV (%)** | | | | |

### 1.3 Pengujian Signal Quality

| Parameter | Nilai | Keterangan |
|-----------|-------|------------|
| Total Pulse (10s) | | |
| Avg Interval (ms) | | |
| Min Interval (ms) | | |
| Max Interval (ms) | | |
| Bounce Detected | [YA/TIDAK] | |

---

## 2. Hasil Kalibrasi

### 2.1 Konstanta K_pulse

| Metode | K_pulse (pulse/L) | Error vs Datasheet |
|--------|-------------------|-------------------|
| Datasheet | 98.0 | - |
| Rata-rata Pengukuran | | % |
| Linear Regression | | % |

### 2.2 Persamaan Kalibrasi

**Volume (mL) = Pulse_Count × K**

Dimana:
- K = [isi nilai kalibrasi] mL/pulse

### 2.3 Akurasi vs Datasheet

| Parameter | Nilai | Status |
|-----------|-------|--------|
| Error rata-rata | | |
| Error maksimum | | |
| Dalam toleransi ±2%? | [YA/TIDAK] | |

---

## 3. Kode MATLAB - Analisis Kalibrasi

### 3.1 Plot Data Kalibrasi

```matlab
%% YFS401 Flow Sensor - Kalibrasi Data Analysis
% Dokumen: B400-SENS-002-KAL
% Sensor: YFS401 Hall-Effect Flow Sensor

clear all; close all; clc;

%% 1. Input Data Kalibrasi
% [ISI DATA DARI PENGUKURAN]

% Volume target (mL)
volume_target = [100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500];

% Volume actual (mL) - ISI HASIL PENGUKURAN
volume_actual = [...

% Pulse count - ISI HASIL PENGUKURAN
pulse_count = [...

% Flow rate (mL/min) - ISI HASIL PENGUKURAN
flow_rate = [...

%% 2. Hitung Pulse per Liter
pulse_per_liter = (pulse_count ./ volume_actual) * 1000;
K_pulse_mean = mean(pulse_per_liter);
K_pulse_std = std(pulse_per_liter);

fprintf('=== HASIL KALIBRASI YFS401 ===\n');
fprintf('K_pulse (rata-rata): %.2f pulse/L\n', K_pulse_mean);
fprintf('K_pulse (std dev): %.2f pulse/L\n', K_pulse_std);
fprintf('K_pulse (datasheet): 98.00 pulse/L\n');
fprintf('Error vs datasheet: %.2f%%\n', abs(K_pulse_mean - 98)/98 * 100);

%% 3. Linear Regression
% Volume = K * Pulse
p = polyfit(pulse_count, volume_actual, 1);
K_regression = p(1);
offset = p(2);

fprintf('\n=== LINEAR REGRESSION ===\n');
fprintf('Volume = %.4f * Pulse + %.2f\n', K_regression, offset);
fprintf('R-squared: %.4f\n', ...

% Hitung R-squared
y_pred = K_regression * pulse_count + offset;
SS_res = sum((volume_actual - y_pred).^2);
SS_tot = sum((volume_actual - mean(volume_actual)).^2);
R_squared = 1 - (SS_res / SS_tot);

%% 4. Plot 1: Volume vs Pulse Count
figure(1);
subplot(2,2,1);
scatter(pulse_count, volume_actual, 100, 'filled', 'MarkerFaceAlpha', 0.6);
hold on;

% Plot linear regression
x_plot = linspace(min(pulse_count), max(pulse_count), 100);
y_plot = K_regression * x_plot + offset;
plot(x_plot, y_plot, 'r-', 'LineWidth', 2);

% Plot datasheet reference
y_datasheet = x_plot / 98;
plot(x_plot, y_datasheet, 'g--', 'LineWidth', 1.5);

grid on;
xlabel('Pulse Count');
ylabel('Volume (mL)');
title('Volume vs Pulse Count');
legend('Data Pengukuran', 'Linear Regression', 'Datasheet (K=98)', 'Location', 'best');

%% 5. Plot 2: Error vs Volume
subplot(2,2,2);
volume_pred = K_regression * pulse_count;
error_vol = volume_actual - volume_pred;

scatter(volume_actual, error_vol, 100, 'filled', 'MarkerFaceAlpha', 0.6);
hold on;
yline(0, 'r--', 'LineWidth', 2);
grid on;
xlabel('Volume Actual (mL)');
ylabel('Error (mL)');
title('Error Kalibrasi vs Volume');

%% 6. Plot 3: Pulse/Liter vs Volume
subplot(2,2,3);
scatter(volume_actual, pulse_per_liter, 100, 'filled', 'MarkerFaceAlpha', 0.6);
hold on;
yline(98, 'g--', 'Datasheet', 'LineWidth', 2);
yline(K_pulse_mean, 'r--', 'Measured', 'LineWidth', 2);
grid on;
xlabel('Volume (mL)');
ylabel('Pulse per Liter');
title('K_p vs Volume');

%% 7. Plot 4: Flow Rate Stability
subplot(2,2,4);
if length(flow_rate) > 1
    plot(1:length(flow_rate), flow_rate, 'b-o', 'LineWidth', 1.5, 'MarkerSize', 8);
    hold on;
    yline(mean(flow_rate), 'r--', 'Mean', 'LineWidth', 2);
    grid on;
    xlabel('Sample Number');
    ylabel('Flow Rate (mL/min)');
    title('Flow Rate Stability');
else
    text(0.5, 0.5, 'Insufficient Data', 'HorizontalAlignment', 'center');
end

%% 8. Save Figure
sgtitle('YFS401 Flow Sensor - Laporan Kalibrasi', 'FontSize', 14, 'FontWeight', 'bold');

% Simpan figure
% saveas(gcf, 'kalibrasi_yfs401.png');
% saveas(gcf, 'kalibrasi_yfs401.fig');

%% 9. Export Results
results.K_pulse_mean = K_pulse_mean;
results.K_pulse_std = K_pulse_std;
results.K_regression = K_regression;
results.offset = offset;
results.R_squared = R_squared;
results.error_vs_datasheet = abs(K_pulse_mean - 98)/98 * 100;

% Simpan ke workspace
% save('kalibrasi_yfs401_results.mat', 'results');

fprintf('\n=== KONSTANTA KALIBRASI UNTUK ARDUINO ===\n');
fprintf('float K_PULSE = %.4f;  // mL per pulse\n', 1000/K_pulse_mean);
fprintf('// Atau:\n');
fprintf('// volume_mL = pulse_count * %.4f;\n', 1000/K_pulse_mean);
```

---

## 4. Kode MATLAB - Simulasi Sistem Kendali Flow

### 4.1 Plant Model (YFS401 + Pompa)

```matlab
%% YFS401 Flow Control - Plant Model
% Dokumen: B400-SENS-002-KAL
% Aplikasi: TK-101 Flow Control (Slave 1)

clear all; close all; clc;

%% 1. Parameter Sensor YFS401
K_pulse = 98;           % pulse/L (datasheet) atau [isi nilai kalibrasi]
sampling_time = 0.5;    % detik
window_size = 1000;     % ms (1 detik)

%% 2. Parameter Pompa
max_flow = 120;         % mL/min (maksimal flow rate pompa)
pump_delay = 0.5;       % detik (waktu respon pompa)

%% 3. Discrete-time Plant Model
% Y(z) = K * z^(-d) / (1 - a*z^(-1)) * U(z)
% dimana:
%   Y = volume (mL)
%   U = pump command (0-100%)
%   d = delay (sampel)
%   a = pole (konstanta waktu)

Ts = 0.5;              % sampling time (detik)
tau = 2.0;             % time constant (detik)
a = exp(-Ts/tau);      % pole
Kp = max_flow / 100;   % gain: mL/min per % duty cycle
d = round(pump_delay / Ts);  % delay in samples

% State-space representation
A = a;
B = Kp * (1 - a);
C = 1;
D = 0;

sys = ss(A, B, C, D, Ts);
sys.OutputDelay = d;

%% 4. Simulasi Respons Step
t = 0:0.5:60;          % 60 detik simulasi
u = zeros(size(t));
u(t >= 5) = 50;        % Step 50% pada t = 5 detik

[y, t, x] = lsim(sys, u, t);

% Konversi flow rate ke volume (integral)
volume = cumsum(y * Ts);

%% 5. Simulasi Sensor YFS401 (dengan noise)
pulse_count = round(volume * K_pulse / 1000);  % Convert mL to pulse
volume_sensor = pulse_count * 1000 / K_pulse;  % Convert pulse back to mL

% Tambahkan noise pengukuran
noise_std = 5;  % mL
volume_sensor_noisy = volume_sensor + noise_std * randn(size(volume_sensor));

%% 6. Plot Hasil Simulasi
figure(2);

% Plot 1: Flow Rate
subplot(3,1,1);
plot(t, u, 'r-', 'LineWidth', 2);
hold on;
plot(t, y, 'b-', 'LineWidth', 2);
grid on;
xlabel('Waktu (detik)');
ylabel('Flow Rate (mL/min)');
title('Respons Plant - Pompa Flow Control');
legend('Input (Duty Cycle %)', 'Output (Flow mL/min)', 'Location', 'best');

% Plot 2: Volume
subplot(3,1,2);
plot(t, volume, 'b-', 'LineWidth', 2);
hold on;
plot(t, volume_sensor_noisy, 'g--', 'LineWidth', 1.5);
grid on;
xlabel('Waktu (detik)');
ylabel('Volume (mL)');
title('Akumulasi Volume');
legend('Volume Actual', 'Volume Sensor (dengan noise)', 'Location', 'best');

% Plot 3: Pulse Count
subplot(3,1,3);
stairs(t, pulse_count, 'b-', 'LineWidth', 1.5);
grid on;
xlabel('Waktu (detik)');
ylabel('Pulse Count');
title('Output Sensor YFS401 - Raw Pulse');

sgtitle('Simulasi Plant Flow Control (TK-101)', 'FontSize', 14, 'FontWeight', 'bold');

%% 7. Transfer Function untuk Control System Design
fprintf('=== TRANSFER FUNCTION PLANT ===\n');
fprintf('Flow Rate Model:\n');
fprintf('  G(z) = %.2f * z^(-%d) / (1 - %.4f*z^(-1))\n', B, d, a);
fprintf('  dengan Ts = %.1f detik\n', Ts);

fprintf('\nVolume Model (Integrator):\n');
fprintf('  Volume(z) = (Ts / (1 - z^(-1))) * Flow(z)\n');

fprintf('\nSensor Model:\n');
fprintf('  Volume_sensor = Pulse_Count * %.4f\n', 1000/K_pulse);
fprintf('  Pulse_Count = Volume_actual * %.4f / 1000\n', K_pulse);
```

### 4.2 PID Controller Simulation

```matlab
%% YFS401 Flow Control - PID Controller Simulation
% Closed-loop simulation dengan feedback dari sensor YFS401

clear all; close all; clc;

%% 1. Plant Model (dari kode sebelumnya)
Ts = 0.5;              % sampling time (detik)
tau = 2.0;             % time constant (detik)
Kp_plant = 1.2;        % gain: mL/min per % duty cycle
a = exp(-Ts/tau);
B = Kp_plant * (1 - a);
A = [1, -a];
d = 1;                 % delay

% Plant discrete transfer function
num_plant = [zeros(1,d), B];
den_plant = A;
plant = tf(num_plant, den_plant, Ts);

%% 2. Integrator (Volume akumulasi)
num_int = [Ts, 0];  % Ts * z / (z - 1) = Ts / (1 - z^(-1))
den_int = [1, -1];
integrator = tf(num_int, den_int, Ts);

%% 3. Sensor YFS401 Model
K_pulse = 98;  % pulse/L
K_sensor = 1;  % unity gain (sebagai volume)

%% 4. PID Controller Design
% C(z) = Kp + Ki*Ts/(1-z^(-1)) + Kd*(1-z^(-1))/Ts
Kp = 2.0;
Ki = 0.5;
Kd = 0.1;

% PID discrete
num_pid = [Kp + Ki*Ts + Kd/Ts, -(Kp + 2*Kd/Ts), Kd/Ts];
den_pid = [1, -1];
controller = tf(num_pid, den_pid, Ts);

%% 5. Closed-loop System
% Total plant flow control
open_loop = controller * plant;
closed_loop_flow = feedback(open_loop, 1);

% Total volume control (dengan integrator)
total_plant = series(plant, integrator);
closed_loop_vol = feedback(controller * total_plant, K_sensor);

%% 6. Simulation Parameters
T_final = 120;  % detik
t = 0:Ts:T_final;

% Reference: volume setpoint = 1000 mL
ref_vol = 1000 * ones(size(t));

% Step response
[y_vol, t_vol] = lsim(closed_loop_vol, ref_vol, t);

%% 7. Calculate Control Signals
% Error
error = ref_vol - y_vol;

% Control output (pump duty cycle)
u = lsim(controller, error, t);

% Saturation
u_sat = max(0, min(100, u));

%% 8. Plot Results
figure(3);

% Plot 1: Volume Response
subplot(3,1,1);
plot(t, ref_vol, 'r--', 'LineWidth', 2);
hold on;
plot(t, y_vol, 'b-', 'LineWidth', 2);
grid on;
xlabel('Waktu (detik)');
ylabel('Volume (mL)');
title('Closed-Loop Response - Volume Control');
legend('Setpoint (1000 mL)', 'PV (Volume)', 'Location', 'best');
ylim([0, 1200]);

% Plot 2: Control Output
subplot(3,1,2);
stairs(t, u_sat, 'g-', 'LineWidth', 2);
hold on;
yline(100, 'r--', 'Max');
yline(0, 'r--', 'Min');
grid on;
xlabel('Waktu (detik)');
ylabel('Duty Cycle (%)');
title('Control Output - Pompa');
ylim([-10, 110]);

% Plot 3: Error
subplot(3,1,3);
plot(t, error, 'm-', 'LineWidth', 1.5);
hold on;
yline(0, 'k--');
grid on;
xlabel('Waktu (detik)');
ylabel('Error (mL)');
title('Error - Setpoint vs PV');

sgtitle('PID Controller - Flow Control (TK-101)', 'FontSize', 14, 'FontWeight', 'bold');

%% 9. Performance Metrics
settling_time_idx = find(abs(y_vol - 1000) < 50, 1);
if ~isempty(settling_time_idx)
    settling_time = t(settling_time_idx);
else
    settling_time = NaN;
end

overshoot = (max(y_vol) - 1000) / 1000 * 100;
steady_state_error = y_vol(end) - 1000;

fprintf('=== PERFORMANCE METRICS ===\n');
fprintf('Rise Time (10-90%%): %.2f detik\n', ...);
fprintf('Settling Time (±5%%): %.2f detik\n', settling_time);
fprintf('Overshoot: %.2f%%\n', overshoot);
fprintf('Steady State Error: %.2f mL\n', steady_state_error);
fprintf('Final Value: %.2f mL\n', y_vol(end));
```

---

## 5. Placeholder Screenshot MATLAB

### 5.1 Screenshot Plot Kalibrasi

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                     [PASTE SCREENSHOT DISINI]                      │
│                    (Plot Volume vs Pulse Count)                     │
│                                                                     │
│  Tips:                                                               │
│  1. Run kode MATLAB di Section 3.1                                  │
│  2. Capture figure(1)                                               │
│  3. Paste screenshot di sini                                        │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 5.2 Screenshot Simulasi Plant

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                     [PASTE SCREENSHOT DISINI]                      │
│                  (Simulasi Plant Flow Control)                     │
│                                                                     │
│  Tips:                                                               │
│  1. Run kode MATLAB di Section 4.1                                  │
│  2. Capture figure(2)                                               │
│  3. Paste screenshot di sini                                        │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 5.3 Screenshot Closed-Loop Response

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│                     [PASTE SCREENSHOT DISINI]                      │
│                (PID Controller Closed-Loop Response)                │
│                                                                     │
│  Tips:                                                               │
│  1. Run kode MATLAB di Section 4.2                                  │
│  2. Capture figure(3)                                               │
│  3. Paste screenshot di sini                                        │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 6. Catatan Penting

1. **Isi data kalibrasi** di Section 1 sebelum menjalankan kode MATLAB
2. **Update nilai K_pulse** di kode MATLAB jika berbeda dari datasheet (98)
3. **Screenshot hasil** MATLAB dan paste di placeholder Section 5
4. **Simpan .m file** terpisah jika perlu modifikasi parameter

---

**Dibuat oleh:** Hifzhudin
**Tanggal:** 2026-05-11
**Status:** PENDING (Belum ada data kalibrasi - perlu pengujian)
