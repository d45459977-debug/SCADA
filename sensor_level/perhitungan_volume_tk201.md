# Perhitungan Volume Tangki TK-201

## Dimensi Fisik

| Parameter | Nilai | Satuan |
|-----------|-------|--------|
| Ukuran luar akrilik | 24.9 × 24.9 | cm |
| Tebal akrilik | 0.5 | cm |
| Tinggi tangki | ~25 | cm (sama dengan sisi) |

### Dimensi Dalam (Internal)

```
Ukuran dalam = Ukuran luar - 2 × tebal akrilik
             = 24.9 - 2 × 0.5
             = 23.9 cm
```

**Luas Penampang Dalam:**
```
A = sisi × sisi
  = 23.9 × 23.9
  = 571.21 cm²
```

---

## Hubungan Jarak-Volume untuk Kalibrasi HC-SR04

### Konfigurasi Sensor

```
┌─────────────────────────────────────┐
│           HC-SR04                    │  ← Sensor di atas
│            ↓↓                        │
│         Plunger                      │  ← Target solid (dicat hitam)
│    ────────────────                 │
│    │               │                 │
│    │   Texapon     │  ← Cairan      │
│    │               │                 │
│    └───────────────┘                 │
└─────────────────────────────────────┘
```

### Parameter Kalibrasi

| Parameter | Simbol | Nilai | Satuan |
|-----------|--------|-------|--------|
| Luas penampang dalam | A | 571.21 | cm² |
| Jarak min sensor | d_min | 2 | cm (spesifikasi HC-SR04) |
| Jarak max sensor | d_max | 23.9 | cm (dasar tangki) |
| Range pengukuran | Δd | 21.9 | cm |

### Konversi Jarak → Volume

**Volume pada posisi plunger:**

```
V = A × h

Dimana:
h = (d_max - d_sensor)  → tinggi cairan dari dasar
d_sensor = jarak HC-SR04 ke plunger
```

**Atau langsung dari jarak sensor:**

```
V(d) = A × (d_max - d)

Keterangan:
d  = jarak pembacaan HC-SR04 (cm)
V  = volume texapon (mL)
```

**Faktor Konversi:**
```
1 cm jarak = 571.21 mL volume
```

### Tabel Referensi Kalibrasi

| Jarak Sensor (cm) | Tinggi Cairan (cm) | Volume (mL) | Catatan |
|-------------------|-------------------|-------------|---------|
| 2.0 | 21.9 | 12,510 | Min (limit sensor) |
| 5.0 | 18.9 | 10,796 |  |
| 10.0 | 13.9 | 7,940 |  |
| 15.0 | 8.9 | 5,084 |  |
| 20.0 | 3.9 | 2,228 |  |
| 23.9 | 0.0 | 0 | Max (plunger di dasar) |

---

## Range Operasional TK-201

Berdasarkan B300, Slave 2 adalah *Level Control* untuk dosing texapon.

**Pertimbangan operasi:**
- **Dead zone HC-SR04:** 0-2 cm (tidak terbaca)
- **Buffer atas:** minimal 2 cm dari sensor
- **Volume efektif:** 0 - ~12.5 liter

**Rekomendasi:** Atur jarak mounting HC-SR04 sehingga:
- Posisi plunger TOP = jarak ~5 cm dari sensor
- Posisi plunger BOTTOM = jarak ~20 cm dari sensor
- Range kerja sensor = 15 cm → ~8.5 liter

---

## Perhitungan Leadscrew

### Spesifikasi
- Diameter ulir luar: 8 mm
- Pitch (standar leadscrew 8mm): 1.25 mm (atau 2 mm tergantung jenis)

### Konversi Rotasi → Linear

```
Pergerakan plunger per rotasi = pitch

Jika pitch = 1.25 mm:
  1 rotasi = 0.125 cm
  1 cm plunger = 8 rotasi

Volume per rotasi:
  V_rotasi = A × pitch
           = 571.21 cm² × 0.125 cm
           = 71.4 mL/rotasi
```

### Resolusi Dosing

| Step | Pergerakan | Volume |
|------|------------|--------|
| 1 rotasi | 0.125 cm | 71.4 mL |
| 1/2 rotasi | 0.063 cm | 35.7 mL |
| 1/4 rotasi | 0.031 cm | 17.9 mL |

---

## Checklist Kalibrasi

### Hubungan HC-SR04 dengan Volume

1. **Kalibrasi jarak** (dengan ruler/solid target)
   - Record: d_referensi vs d_sensor
   - Hitung error, linearity, precision

2. **Konversi ke volume**
   - Gunakan rumus: V = 571.21 × (23.9 - d_sensor)
   - d_sensor dalam cm, V dalam mL

3. **Validasi dengan leadscrew**
   - Putar motor N rotasi
   - Cek perubahan jarak HC-SR04
   - Bandingkan Δd teori vs aktual

### Catatan Penting

- [ ] Pastikan permukaan plunger PARALEL dengan sensor HC-SR04
- [ ] Plunger dicat warna gelap (matte) untuk mengurangi noise
- [ ] Jarak mounting HC-SR04 dari atas tangki diukur akurat

---

## Referensi B300

**Slave 2 (TK-201) - Level Control:**
- LUAS_PENAMPANG = 571.21 cm² (revisi dari 314.16 → tangki persegi, bukan bulat)
- FAKTOR_KONVERSI = 5712.1 (cm → mL, per 10 cm)
- Toleransi = 0.3 cm
- Loop delay: 500 ms
