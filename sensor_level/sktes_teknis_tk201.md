# Sketsa Teknis Tangki TK-201

## Material

- **Jumlah:** 3 lembar akrilik bening/clear
- **Ukuran per lembar:** 24.9 × 24.9 cm
- **Tebal:** 5 mm (0.5 cm)

---

## Rencana Potongan

### Lembar 1 & 2: Dinding Tangki

Setiap lembar dipotong menjadi 2 panel:

```
┌─────────────────────────────────────┐
│  LEMBAR 1 / LEMBAR 2 (24.9 × 24.9)  │
├─────────────────────────────────────┤
│                                     │
│    ┌───────────────┐ ┌─────────────┐
│    │   Panel A     │ │  Panel B    │
│    │   (24.9 ×     │ │  (24.9 ×    │
│    │    ~12.45)    │ │   ~12.45)   │
│    │               │ │             │
│    └───────────────┘ └─────────────┘
│                                     │
└─────────────────────────────────────┘
```

**Spesifikasi Panel:**

| Panel | Dimensi (cm) | Jumlah | Fungsi |
|-------|--------------|--------|--------|
| A | 24.9 × 12.45 | 2 | Dinding depan & belakang |
| B | 24.9 × 12.45 | 2 | Dinding kiri & kanan |

**Total dari Lembar 1 & 2:** 4 panel dinding

### Lembar 3: Alas + Atap

```
┌─────────────────────────────────────┐
│      LEMBAR 3 (24.9 × 24.9)         │
├─────────────────────────────────────┤
│                                     │
│    ┌─────────────────────────────┐  │
│    │                             │  │
│    │      PANEL ATAP             │  │
│    │    (24.9 × 24.9)            │  │
│    │                             │  │
│    │         ┌───┐               │  │
│    │         │ 8 │ ← Lubang      │  │
│    │         │mm │   leadscrew   │  │
│    │         └───┘               │  │
│    │                             │  │
│    └─────────────────────────────┘  │
│                                     │
│    Bagian bawah memanjang → Alas    │
│    (gunakan seluruh lembar)         │
└─────────────────────────────────────┘
```

**Catatan:** Lembar 3 bisa dibagi menjadi:
- **Atap:** 24.9 × 24.9 cm dengan lubang 8mm di tengah
- **Alas:** 24.9 × 24.9 cm (utuh, tanpa lubang)

Atau jika material cukup, gunakan lembar terpisah untuk alas.

---

## Detail Lubang Leadscrew (Atap)

```
        ATAP TANGKI (24.9 × 24.9 cm)

    ╔═══════════════════════════╗
    ║                           ║
    ║                           ║
    ║           ○ 8mm           ║  ← Lubang leadscrew
    ║          (tengah)         ║     di tengah panel
    ║                           ║
    ║                           ║
    ╚═══════════════════════════╝
```

**Dimensi lubang:**
- Diameter: 8 mm (sesuai ulir luar leadscrew)
- Posisi: Center of panel (12.45 × 12.45 cm dari sudut)

**Toleransi:** +0.5 mm untuk clearance (rekomendasi: 8.5 mm)

---

## Assembly Tangki

```
    ╔═══════════════════════════════╗  ← Atap (dgn lubang 8mm)
    ║           HC-SR04             ║
    ║            ↓↓                 ║
    ║       ┌─────────┐             ║
    ║       │ Plunger │             ║
    ║       └─────────┘             ║
    ╠═══════════════════════════════╣
    ║                               ║
    ║                               ║  ← Dinding (4 panel: A×2, B×2)
    ║         TEXAPON               ║     Tinggi dalam: ~23.9 cm
    ║                               ║
    ║                               ║
    ╠═══════════════════════════════╣
    ║                               ║  ← Alas (24.9 × 24.9 cm)
    ╚═══════════════════════════════╝
```

---

## Dimensi Final Tangki

| Dimensi | Nilai | Keterangan |
|---------|-------|------------|
| Tinggi luar | 24.9 cm | Atap + tinggi dinding |
| Tinggi dalam | ~23.4 cm | Kurang tebal atap & alas |
| Lebar sisi dalam | 23.9 cm | 24.9 - 2×0.5 |
| Volume total | ~13,355 mL | 23.9 × 23.9 × 23.4 |

---

## Metode Sambung

Rekomendasi:

1. **Akrilik Cement** (chloroform atau ACRILIC GLUE)
   - Lem siku akrilik untuk joint dinding-alas
   - Apply di luar, biarkan mengalir ke dalam joint

2. **Sealing**
   - Silicone sealant di dalam joint untuk waterproof
   - Khususnya di sudut-sudut

3. **Reinforcement** (opsional)
   - Bracket sudut akrilik untuk kekuatan tambahan
   - Terutama jika tangki akan menahan beban cairan banyak

---

## Checklist Pemotongan

### Lembar 1 & 2
- [ ] Potong menjadi 2 panel (24.9 × ~12.45 cm)
- [ ] Amplas sisi potong (haluskan tepi)
- [ ] Bersihkan debu

### Lembar 3
- [ ] Tandai titik tengah (12.45, 12.45)
- [ ] Bor lubang 8.5 mm (atau 8 mm untuk tight fit)
- [ ] Jika perlu split alas/atap: potong horizontal

### Assembly
- [ ] Dry fit dulu (tanpa lem) untuk cek kesesuaian
- [ ] Apply akrilik cement pada joint luar
- [ ] Tahan dengan selotip/penjepit selama curing
- [ ] Apply silicone sealant di dalam setelah cement kering
- [ ] Test dengan air untuk cek kebocoran

---

## Catatan untuk Leadscrew

- Pastikan lubang cukup lurus (vertical) agar leadscrew tidak binding
- Jika perlu, tambahan bushing/bearing plastik di lubang untuk mengurangi friksi
- Plunger harus bisa bergerak bebas tanpa menabrak dinding
