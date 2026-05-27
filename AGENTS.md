# Repository Guidelines

## Project Structure & Module Organization

This repository contains B400 heating and mixing miniplant implementation notes, Arduino sketches, calibration data, and TCP/IP communication examples.

- `sensor_flow/`: YFS401 flow sensor sketches, calibration logs, reports, and Python logger.
- `sensor_level/`: HC-SR04 level/volume calibration sketches, data templates, and reports.
- `sensor_temp/`: DS18B20 temperature sensor tests and documentation.
- `wiring/`: actuator wiring notes and motor lead screw test sketches.
- `KOMDAT_TCPIP/`: ESP32 Modbus TCP gateway examples, PlatformIO config, and Kepware tag/setup files.
- `*.md` files at the root document control concepts, implementation notes, and design decisions.

Keep Arduino sketches in their own folder named the same as the `.ino` file, for example `hcsr04_karakteristik/hcsr04_karakteristik.ino`.

## Build, Test, and Development Commands

Use Arduino CLI for sketches:

```bash
arduino-cli compile --fqbn arduino:avr:uno sensor_flow/yfs401_simple
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:uno sensor_flow/yfs401_simple
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=9600
```

For ESP32 sketches:

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 KOMDAT_TCPIP/esp32_gateway/esp32_simulasi_dummy
```

For PlatformIO-based TCP/IP gateway work:

```bash
cd KOMDAT_TCPIP/esp32_gateway
pio run
```

## Coding Style & Naming Conventions

Use C/C++ Arduino style with clear pin constants near the top of each sketch. Prefer uppercase names for hardware constants, for example `TRIG_PIN`, `ECHO_PIN`, and `DE_RE`. Use descriptive sketch and folder names based on device/function, such as `yfs401_terkalibrasi` or `hcsr04_kalibrasi_volume`.

Keep serial output concise and practical for field debugging. Document register maps and pinouts in comments when a sketch communicates over Modbus or controls actuators.

## Testing Guidelines

There is no automated test suite. Validate changes by compiling the target sketch and testing on hardware through Serial Monitor. Record calibration results in CSV files or Markdown reports next to the relevant sensor folder. For communication work, verify Kepware tags against `KOMDAT_TCPIP/kepserver/*.csv`.

## Commit & Pull Request Guidelines

This folder does not include Git history. Use concise, imperative commit messages when contributing, for example `Add HC-SR04 calibration sketch` or `Update Kepware tag map`. Pull requests should include changed hardware targets, tested board/FQBN, serial baudrate, wiring changes, and screenshots or logs when relevant.

## Security & Configuration Tips

Avoid committing real WiFi passwords, plant-specific credentials, or machine-local IP addresses unless they are intentional lab defaults. Keep Kepware device settings and tag CSV files synchronized with the ESP32 register map.
