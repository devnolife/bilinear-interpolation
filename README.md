# 🖼️ Bilinear Interpolation

**Image Processing Algorithm - Visual Demo**

[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![GitHub](https://img.shields.io/badge/GitHub-devnolife-black.svg)](https://github.com/devnolife)

---

## 👤 Author

**Andi Agung Dwi Arya**

- 🎓 **Program**: Algoritma Komputasi S2 (Master's Degree)
- 💻 **GitHub**: [@devnolife](https://github.com/devnolife)
- 📅 **Year**: 2025

---

## 📁 Struktur Files

```
interpolasi/
├── bilinear_interpolation.cpp   # 💻 Program utama (C++)
├── index.html                   # 🌐 Demo interaktif (Web)
└── README.md                    # 📖 Dokumentasi (File ini)
```

**Hanya 3 file!** Sederhana dan mudah dipahami.

---

## 🔍 Apa itu Bilinear Interpolation?

**Bilinear Interpolation** adalah metode untuk menentukan nilai pada koordinat non-integer (desimal) dengan menggunakan **4 pixel tetangga terdekat**.

### Ilustrasi

```
    x0              x1
    │               │
y0 ─┼───────────────┼─
    │ f00       f10 │
    │       ●       │  ← Target point P(x,y)
    │ f01       f11 │
y1 ─┼───────────────┼─
```

### Formula

```
f(x,y) = f00·(1-fx)·(1-fy) + f10·fx·(1-fy) + f01·(1-fx)·fy + f11·fx·fy
```

Dimana:

- `fx = x - floor(x)` → jarak horizontal
- `fy = y - floor(y)` → jarak vertical
- `f00, f10, f01, f11` → nilai 4 pixel tetangga

---

## 🚀 Cara Kompilasi

### Prerequisites

- **C++ Compiler** (MinGW-w64 / GCC / Clang)
- C++17 support

### Compile

```bash
g++ -o bilinear.exe bilinear_interpolation.cpp -std=c++17
```

### Jalankan

```bash
./bilinear.exe
```

---

## 💻 Program C++ (Console)

Program `bilinear_interpolation.cpp` menyediakan:

1. **Demo Single Pixel** - Interpolasi pada satu koordinat
2. **Demo Resize** - Resize gambar 4×4 ke 6×6
3. **Custom Koordinat** - Input koordinat sendiri
4. **Visualisasi Grid** - Tampilan ASCII dalam terminal

### Contoh Output

```
┌─────────────────────────────────────┐
│        BILINEAR INTERPOLATION       │
│         Image Processing Demo       │
├─────────────────────────────────────┤
│   Author: Andi Agung Dwi Arya       │
│   GitHub: devnolife                 │
│   Course: Algoritma Komputasi S2    │
└─────────────────────────────────────┘

[Tekan Enter untuk melanjutkan...]
```

---

## 🌐 Demo HTML Interaktif

Buka file `index.html` di browser untuk melihat:

1. **📚 Penjelasan Teori** - Konsep dan formula lengkap
2. **🎮 Demo Interaktif** - Geser slider untuk eksplorasi real-time
3. **💻 Source Code** - Lihat implementasi C++ dengan syntax highlighting
4. **📝 Step-by-Step** - Perhitungan detail otomatis

### Fitur Interaktif

- Canvas visualization dengan 4×4 grid
- X/Y slider untuk koordinat
- Animasi step-by-step
- Real-time calculation display

---

## 📐 Contoh Perhitungan

```
Koordinat: P(1.5, 1.5)

4 Tetangga:
f00 = 60  (kiri-atas)
f10 = 80  (kanan-atas)
f01 = 90  (kiri-bawah)
f11 = 110 (kanan-bawah)

Fraksi:
fx = 0.5, fy = 0.5

Bobot:
w00 = 0.25, w10 = 0.25
w01 = 0.25, w11 = 0.25

Hasil:
f(1.5, 1.5) = 60×0.25 + 80×0.25 + 90×0.25 + 110×0.25
            = 15 + 20 + 22.5 + 27.5
            = 85
```

---

## 📚 Aplikasi

Bilinear Interpolation digunakan dalam:

- **Image Resizing** - Zoom in/out gambar
- **Image Rotation** - Rotasi dengan hasil smooth
- **Texture Mapping** - Grafik 3D
- **Computer Vision** - Preprocessing gambar

---

## 📝 License

MIT License - Free to use for educational purposes.

---

<p align="center">
  Made with ❤️ by <a href="https://github.com/devnolife">Andi Agung Dwi Arya</a>
  <br>
  Algoritma Komputasi S2 - 2025
</p>
