# Bilinear Interpolation: Serial vs Parallel Comparison

## 📋 Ringkasan Studi Kasus

File **bilinear_comparison.cpp** membandingkan performa 3 implementasi:
1. **Serial** - Implementasi langsung rumus (sequential)
2. **OpenMP** - Paralelisasi dengan OpenMP
3. **Cilk** - Paralelisasi dengan Cilk Plus

## 🔧 Cara Compile

### 1. Serial Only (Baseline)
```bash
g++ -o bilinear_serial bilinear_comparison.cpp -std=c++17
```

### 2. Dengan OpenMP
```bash
g++ -o bilinear_omp bilinear_comparison.cpp -std=c++17 -fopenmp -DUSE_OPENMP
```

### 3. Dengan Cilk
```bash
g++ -o bilinear_cilk bilinear_comparison.cpp -std=c++17 -fcilkplus -DUSE_CILK
```

### 4. Semua Mode (OpenMP + Cilk)
```bash
g++ -o bilinear_full bilinear_comparison.cpp -std=c++17 -fopenmp -fcilkplus -DUSE_OPENMP -DUSE_CILK
```

## ▶️ Cara Menjalankan

```bash
./bilinear_serial    # Hanya test serial
./bilinear_omp       # Test serial + OpenMP
./bilinear_cilk      # Test serial + Cilk
./bilinear_full      # Test semua (serial + OpenMP + Cilk)
```

## 📊 Output yang Diharapkan

Program akan menampilkan:

1. **Analisis Konsep** - Penjelasan perbedaan serial vs parallel
2. **Benchmark Results** - Perbandingan waktu eksekusi:
   - Test size: 512×512 → 2048×2048
   - Test size: 1024×1024 → 2048×2048
   - Test size: 2048×2048 → 2048×2048

3. **Speedup Metrics**:
   - Serial: Baseline time
   - OpenMP (2/4/8 threads): Time + Speedup
   - Cilk: Time + Speedup

### Contoh Output:
```
========================================================================
      BENCHMARK: BILINEAR INTERPOLATION SERIAL vs PARALLEL
========================================================================

Test: Resize 1024x1024 -> 2048x2048
------------------------------------------------------------------------
  [SERIAL]       Time:   2340 ms
  [OpenMP-2]     Time:   1250 ms  |  Speedup: 1.87x
  [OpenMP-4]     Time:    680 ms  |  Speedup: 3.44x
  [OpenMP-8]     Time:    420 ms  |  Speedup: 5.57x
  [Cilk]         Time:    390 ms  |  Speedup: 6.00x
```

## 🎯 Kesimpulan yang Diharapkan

### Serial Implementation
- **Kelebihan**: Sederhana, tidak ada overhead
- **Kekurangan**: Lambat untuk image besar
- **Use Case**: Image kecil atau single-core processor

### OpenMP Implementation
- **Kelebihan**: Mudah implementasi, kontrol eksplisit threads
- **Speedup**: ~60-80% efficiency (3-6x pada 8 cores)
- **Use Case**: Shared-memory parallelism, CPU-bound tasks

### Cilk Implementation
- **Kelebihan**: Work-stealing scheduler, better load balancing
- **Speedup**: ~70-90% efficiency (5-7x pada 8 cores)
- **Use Case**: Complex parallel patterns, dynamic workloads

## 📝 Catatan Penting

1. **Kompiler**: Pastikan GCC mendukung OpenMP dan Cilk
   - OpenMP: Biasanya sudah tersedia di GCC modern
   - Cilk: Mungkin perlu install `gcc-cilk` atau gunakan Intel ICC

2. **Hardware**: Hasil tergantung jumlah cores CPU
   - Lebih banyak cores = speedup lebih tinggi
   - Hyperthreading dapat meningkatkan performa

3. **Test Size**: Image lebih besar = perbedaan lebih jelas
   - Small: < 512×512 (overhead paralelisasi mungkin tidak worth it)
   - Medium: 1024×1024 (sweet spot)
   - Large: > 2048×2048 (parallel sangat menguntungkan)

## 🔍 Analisis Formula

**Rumus Bilinear Interpolation:**
```
f(x,y) = f(x₀,y₀)·(1-fx)·(1-fy) + f(x₁,y₀)·fx·(1-fy)
       + f(x₀,y₁)·(1-fx)·fy     + f(x₁,y₁)·fx·fy
```

- **Serial**: Hitung 1 pixel per iterasi, sequential
- **Parallel**: Hitung N pixels concurrent, independent operations
- **Speedup**: Ideal = P (cores), Real < P (overhead)

## 📚 Referensi

- OpenMP: https://www.openmp.org/
- Cilk: https://cilk.mit.edu/
- Amdahl's Law: S(P) = 1 / (1-p + p/P)
