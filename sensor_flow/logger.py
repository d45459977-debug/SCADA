#!/usr/bin/env python3
"""
Serial Data Logger untuk YFS401 Karakteristik
Jalankan script ini, lalu buka Arduino IDE dan testing manual
Data otomatis tersimpan ke file CSV
"""

import serial
import time
from datetime import datetime

# Konfigurasi
SERIAL_PORT = '/dev/ttyUSB0'
BAUDRATE = 9600
OUTPUT_DIR = '/home/hifzhudin/heating-mixing-miniplant/b400/sensor_flow'

def main():
    # Buat file dengan timestamp
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    output_file = f"{OUTPUT_DIR}/data_{timestamp}.csv"

    # Buka serial port
    print(f"Membuka {SERIAL_PORT}...")
    s = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)
    time.sleep(1)

    print(f"Logging data ke: {output_file}")
    print("=" * 60)
    print("Tekan Ctrl+C untuk stop logging")
    print("=" * 60)

    # Buka file untuk write
    with open(output_file, 'w') as f:
        # Write header
        f.write("timestamp,volume_ml,pulse_count,flow_ml_min,pulse_per_l,time_ms,raw_line\n")

        line_count = 0
        start_time = time.time()

        try:
            while True:
                if s.in_waiting:
                    data = s.read(s.in_waiting).decode('utf-8', errors='ignore')
                    lines = data.split('\n')

                    for line in lines:
                        if line.strip():
                            # Parse CSV
                            parts = line.split(',')
                            if len(parts) >= 5:
                                try:
                                    ts = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")
                                    vol = parts[0] if parts[0] != 'VOLUME_ML' else ''
                                    pul = parts[1] if parts[1] != 'PULSE_COUNT' else ''
                                    flow = parts[2] if parts[2] != 'FLOW_ML_MIN' else ''
                                    ppl = parts[3] if parts[3] != 'PULSE_PER_L' else ''
                                    ms = parts[4] if len(parts) > 4 else ''

                                    # Skip header lines
                                    if vol and vol != 'VOLUME_ML':
                                        f.write(f"{ts},{vol},{pul},{flow},{ppl},{ms},{line}\n")
                                        line_count += 1

                                        # Print ke terminal (tiap 10 baris)
                                        if line_count % 10 == 0:
                                            elapsed = time.time() - start_time
                                            print(f"[{elapsed:.1f}s] Line {line_count}: Vol={vol} mL, Pulse={pul}")
                                except:
                                    pass

                time.sleep(0.05)

        except KeyboardInterrupt:
            elapsed = time.time() - start_time
            print(f"\n" + "=" * 60)
            print(f"Logging STOPPED")
            print(f"Data disimpan: {output_file}")
            print(f"Total baris: {line_count}")
            print(f"Durasi: {elapsed:.1f} detik")
            print("=" * 60)

    s.close()

if __name__ == "__main__":
    main()
