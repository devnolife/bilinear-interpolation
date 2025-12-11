/**
 * ============================================================================
 *                    BILINEAR INTERPOLATION
 *              Implementasi dalam Bahasa C dengan OpenMP
 * ============================================================================
 *
 * Author : Andi Agung Dwi Arya
 * Course : Algoritma Komputasi S2 - 2025
 *
 * DESKRIPSI:
 * Program ini mengimplementasikan algoritma Bilinear Interpolation untuk
 * image resizing dengan 2 pendekatan:
 * 1. SERIAL   - Eksekusi sequential (1 core)
 * 2. OPENMP   - Eksekusi parallel (multi-core)
 *
 * COMPILE:
 *   Serial  : gcc -o bilinear bilinear.c -O3
 *   OpenMP  : gcc -o bilinear bilinear.c -O3 -fopenmp -DUSE_OPENMP
 *
 * RUN:
 *   ./bilinear
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifdef USE_OPENMP
#include <omp.h>
#endif

/* ============================================================================
 * DEFINISI STRUKTUR DATA
 * ============================================================================ */

/* Struktur untuk menyimpan 1 pixel dengan 3 channel warna (RGB) */
typedef struct {
    float r;    /* Red   (0-255) */
    float g;    /* Green (0-255) */
    float b;    /* Blue  (0-255) */
} Pixel;

/* Struktur untuk menyimpan image 2D */
typedef struct {
    Pixel *data;    /* Array 1D untuk menyimpan semua pixel */
    int width;      /* Lebar image (jumlah kolom) */
    int height;     /* Tinggi image (jumlah baris) */
} Image;

/* ============================================================================
 * FUNGSI-FUNGSI MANAJEMEN IMAGE
 * ============================================================================ */

/**
 * Membuat image baru dengan ukuran tertentu
 * @param width  - Lebar image
 * @param height - Tinggi image
 * @return Pointer ke Image yang baru dibuat, atau NULL jika gagal
 */
Image* buatImage(int width, int height) {
    /* Alokasi memory untuk struct Image */
    Image *img = (Image*)malloc(sizeof(Image));
    if (!img) return NULL;

    /* Set ukuran */
    img->width = width;
    img->height = height;

    /* Alokasi memory untuk data pixel (width × height pixel) */
    img->data = (Pixel*)calloc(width * height, sizeof(Pixel));
    if (!img->data) {
        free(img);
        return NULL;
    }

    return img;
}

/**
 * Menghapus image dan free memory
 */
void hapusImage(Image *img) {
    if (img) {
        if (img->data) free(img->data);
        free(img);
    }
}

/**
 * Mengambil pixel pada koordinat (x, y)
 * Akses array 1D dengan rumus: index = y × width + x
 */
Pixel ambilPixel(const Image *img, int x, int y) {
    return img->data[y * img->width + x];
}

/**
 * Set nilai pixel pada koordinat (x, y)
 */
void setPixel(Image *img, int x, int y, Pixel p) {
    img->data[y * img->width + x] = p;
}

/* ============================================================================
 * FUNGSI UTILITAS
 * ============================================================================ */

/**
 * Membatasi nilai dalam range tertentu
 * Contoh: clamp(150, 0, 100) = 100
 */
float clamp(float nilai, float min, float max) {
    if (nilai < min) return min;
    if (nilai > max) return max;
    return nilai;
}

/**
 * Mengembalikan nilai minimum dari 2 integer
 */
int minimum(int a, int b) {
    return (a < b) ? a : b;
}

/* ============================================================================
 * ALGORITMA INTI: BILINEAR INTERPOLATION
 * ============================================================================
 *
 * CARA KERJA:
 * 1. Cari 4 pixel tetangga terdekat dari koordinat (x, y)
 * 2. Hitung jarak relatif (fraksi) dari pojok kiri-atas
 * 3. Hitung bobot untuk setiap tetangga
 * 4. Hasil = weighted average dari 4 tetangga
 *
 * ILUSTRASI:
 *      (x0,y0)-----------(x1,y0)
 *         |                 |
 *         |       P(x,y)    |
 *         |                 |
 *      (x0,y1)-----------(x1,y1)
 *
 * ============================================================================ */

Pixel bilinearInterpolasi(const Image *img, float x, float y) {
    float maxX, maxY, fx, fy;
    int x0, y0, x1, y1;
    Pixel f00, f10, f01, f11, hasil;
    float w00, w10, w01, w11;

    /* STEP 1: Batasi koordinat agar tidak keluar dari image */
    maxX = (float)img->width - 1.001f;
    maxY = (float)img->height - 1.001f;
    x = clamp(x, 0.0f, maxX);
    y = clamp(y, 0.0f, maxY);

    /* STEP 2: Tentukan 4 pixel tetangga */
    x0 = (int)floor(x);  /* Koordinat kiri */
    y0 = (int)floor(y);  /* Koordinat atas */
    x1 = minimum(x0 + 1, img->width - 1);   /* Koordinat kanan */
    y1 = minimum(y0 + 1, img->height - 1);  /* Koordinat bawah */

    /* STEP 3: Hitung fraksi (jarak relatif dari x0 dan y0) */
    fx = x - (float)x0;  /* Jarak horizontal: 0.0 - 1.0 */
    fy = y - (float)y0;  /* Jarak vertikal: 0.0 - 1.0 */

    /* STEP 4: Ambil nilai 4 pixel tetangga */
    f00 = ambilPixel(img, x0, y0);  /* Kiri-Atas */
    f10 = ambilPixel(img, x1, y0);  /* Kanan-Atas */
    f01 = ambilPixel(img, x0, y1);  /* Kiri-Bawah */
    f11 = ambilPixel(img, x1, y1);  /* Kanan-Bawah */

    /* STEP 5: Hitung bobot untuk setiap tetangga */
    /* Bobot berbanding terbalik dengan jarak */
    w00 = (1.0f - fx) * (1.0f - fy);  /* Bobot kiri-atas */
    w10 = fx * (1.0f - fy);            /* Bobot kanan-atas */
    w01 = (1.0f - fx) * fy;            /* Bobot kiri-bawah */
    w11 = fx * fy;                     /* Bobot kanan-bawah */

    /* STEP 6: Hitung hasil (weighted sum) */
    hasil.r = f00.r * w00 + f10.r * w10 + f01.r * w01 + f11.r * w11;
    hasil.g = f00.g * w00 + f10.g * w10 + f01.g * w01 + f11.g * w11;
    hasil.b = f00.b * w00 + f10.b * w10 + f01.b * w01 + f11.b * w11;

    return hasil;
}

/* ============================================================================
 * IMPLEMENTASI SERIAL (Sequential)
 * ============================================================================ */

Image* resizeSerial(const Image *source, int lebarBaru, int tinggiBaru) {
    Image *dest;
    float scaleX, scaleY;
    int x, y;

    /* Buat image tujuan */
    dest = buatImage(lebarBaru, tinggiBaru);
    if (!dest) return NULL;

    /* Hitung faktor skala */
    scaleX = (float)source->width / lebarBaru;
    scaleY = (float)source->height / tinggiBaru;

    /* LOOP SERIAL - Proses pixel satu per satu */
    for (y = 0; y < tinggiBaru; y++) {
        for (x = 0; x < lebarBaru; x++) {
            /* Mapping: koordinat tujuan → koordinat sumber */
            float srcX = x * scaleX;
            float srcY = y * scaleY;

            /* Interpolasi */
            Pixel p = bilinearInterpolasi(source, srcX, srcY);

            /* Simpan hasil */
            setPixel(dest, x, y, p);
        }
    }

    return dest;
}

/* ============================================================================
 * IMPLEMENTASI OPENMP (Parallel)
 * ============================================================================ */

#ifdef USE_OPENMP
Image* resizeOpenMP(const Image *source, int lebarBaru, int tinggiBaru, int numThreads) {
    Image *dest;
    float scaleX, scaleY;
    int x, y;

    /* Buat image tujuan */
    dest = buatImage(lebarBaru, tinggiBaru);
    if (!dest) return NULL;

    /* Hitung faktor skala */
    scaleX = (float)source->width / lebarBaru;
    scaleY = (float)source->height / tinggiBaru;

    /* Set jumlah threads */
    omp_set_num_threads(numThreads);

    /* LOOP PARALLEL - OpenMP membagi loop ke multiple threads */
    #pragma omp parallel for collapse(2) private(x, y)
    for (y = 0; y < tinggiBaru; y++) {
        for (x = 0; x < lebarBaru; x++) {
            /* Mapping: koordinat tujuan → koordinat sumber */
            float srcX = x * scaleX;
            float srcY = y * scaleY;

            /* Interpolasi */
            Pixel p = bilinearInterpolasi(source, srcX, srcY);

            /* Simpan hasil */
            setPixel(dest, x, y, p);
        }
    }

    return dest;
}
#endif

/* ============================================================================
 * FUNGSI UNTUK MEMBUAT TEST IMAGE
 * ============================================================================ */

Image* buatTestImage(int ukuran) {
    Image *img;
    int x, y;

    img = buatImage(ukuran, ukuran);
    if (!img) return NULL;

    /* Isi dengan gradient pattern */
    for (y = 0; y < ukuran; y++) {
        for (x = 0; x < ukuran; x++) {
            float nilai = (float)((x + y) % 256);
            Pixel p;
            p.r = nilai;
            p.g = nilai;
            p.b = nilai;
            setPixel(img, x, y, p);
        }
    }

    return img;
}

/* ============================================================================
 * FUNGSI BENCHMARK
 * ============================================================================ */

void jalankanBenchmark() {
    /* Ukuran test cases */
    int ukuranTest[] = {512, 1024, 2048};
    int jumlahTest = 3;
    int ukuranTarget = 2048;
    int t, i;

    printf("\n");
    printf("========================================================================\n");
    printf("              BENCHMARK: SERIAL vs OPENMP\n");
    printf("========================================================================\n\n");

    /* Loop untuk setiap test case */
    for (t = 0; t < jumlahTest; t++) {
        int ukuran = ukuranTest[t];
        Image *testImg;
        Image *hasilSerial;
        clock_t mulai, selesai;
        double waktuSerial;

        printf("Test %d: Resize %dx%d → %dx%d\n", t+1, ukuran, ukuran, ukuranTarget, ukuranTarget);
        printf("------------------------------------------------------------------------\n");

        /* Buat test image */
        testImg = buatTestImage(ukuran);
        if (!testImg) {
            printf("Error: Gagal membuat test image\n");
            continue;
        }

        /* ====== BENCHMARK SERIAL ====== */
        mulai = clock();
        hasilSerial = resizeSerial(testImg, ukuranTarget, ukuranTarget);
        selesai = clock();
        waktuSerial = ((double)(selesai - mulai)) / CLOCKS_PER_SEC * 1000.0;

        printf("  [SERIAL]       Waktu: %7.0f ms\n", waktuSerial);

        if (hasilSerial) hapusImage(hasilSerial);

        /* ====== BENCHMARK OPENMP ====== */
#ifdef USE_OPENMP
        {
            int jumlahThreads[] = {2, 4, 8};
            int jumlahTestThread = 3;

            for (i = 0; i < jumlahTestThread; i++) {
                int threads = jumlahThreads[i];
                Image *hasilOmp;
                double waktuOmp, speedup;

                mulai = clock();
                hasilOmp = resizeOpenMP(testImg, ukuranTarget, ukuranTarget, threads);
                selesai = clock();
                waktuOmp = ((double)(selesai - mulai)) / CLOCKS_PER_SEC * 1000.0;

                speedup = waktuSerial / waktuOmp;
                printf("  [OpenMP-%d]     Waktu: %7.0f ms  |  Speedup: %.2fx\n",
                       threads, waktuOmp, speedup);

                if (hasilOmp) hapusImage(hasilOmp);
            }
        }
#else
        printf("  [OpenMP]       TIDAK AKTIF (compile dengan -fopenmp -DUSE_OPENMP)\n");
#endif

        printf("\n");
        hapusImage(testImg);
    }

    printf("========================================================================\n");
}

/* ============================================================================
 * FUNGSI UNTUK PRINT PENJELASAN KONSEP
 * ============================================================================ */

void tampilkanKonsep() {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════════╗\n");
    printf("║                  BILINEAR INTERPOLATION                           ║\n");
    printf("║              Perbandingan Serial vs OpenMP (C)                    ║\n");
    printf("╚═══════════════════════════════════════════════════════════════════╝\n\n");

    printf("KONSEP DASAR:\n");
    printf("-------------\n");
    printf("Bilinear Interpolation menghitung nilai pixel pada koordinat desimal\n");
    printf("dengan menggunakan weighted average dari 4 pixel tetangga terdekat.\n\n");

    printf("FORMULA:\n");
    printf("--------\n");
    printf("  f(x,y) = f00×w00 + f10×w10 + f01×w01 + f11×w11\n\n");
    printf("  Dimana:\n");
    printf("    w00 = (1-fx) × (1-fy)    w10 = fx × (1-fy)\n");
    printf("    w01 = (1-fx) × fy        w11 = fx × fy\n\n");

    printf("IMPLEMENTASI:\n");
    printf("-------------\n");
    printf("1. SERIAL\n");
    printf("   - Loop sequential (for biasa)\n");
    printf("   - Proses 1 pixel per iterasi\n");
    printf("   - Gunakan: Image kecil, debugging\n\n");

#ifdef USE_OPENMP
    printf("2. OPENMP (AKTIF)\n");
    printf("   - #pragma omp parallel for\n");
    printf("   - Multiple threads bekerja bersamaan\n");
    printf("   - Speedup: ~3-6x pada 8 cores\n");
    printf("   - Max threads: %d\n\n", omp_get_max_threads());
#else
    printf("2. OPENMP (TIDAK AKTIF)\n");
    printf("   - Compile dengan: gcc -fopenmp -DUSE_OPENMP\n\n");
#endif

    printf("========================================================================\n");
}

/* ============================================================================
 * MAIN PROGRAM
 * ============================================================================ */

int main() {
    /* Tampilkan konsep */
    tampilkanKonsep();

    /* Jalankan benchmark */
    jalankanBenchmark();

    printf("\nProgram selesai.\n\n");
    return 0;
}
