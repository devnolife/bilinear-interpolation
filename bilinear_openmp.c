/**
 * ============================================================================
 *              BILINEAR INTERPOLATION - Pure C with OpenMP
 * ============================================================================
 * Studi Kasus: Perbandingan Performa Serial vs OpenMP dalam bahasa C
 * Author: Andi Agung Dwi Arya
 * ============================================================================
 *
 * Compile:
 *   Serial:  gcc -o bilinear_serial bilinear_openmp.c -std=c99 -O3
 *   OpenMP:  gcc -o bilinear_omp bilinear_openmp.c -std=c99 -O3 -fopenmp -DUSE_OPENMP
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifdef USE_OPENMP
#include <omp.h>
#endif

/* ============================================================================
 * STRUKTUR DATA
 * ============================================================================ */

typedef struct {
    float r, g, b;
} Pixel;

typedef struct {
    Pixel *data;
    int width;
    int height;
} Image;

/* ============================================================================
 * FUNGSI UTILITAS IMAGE
 * ============================================================================ */

Image* createImage(int width, int height) {
    Image *img = (Image*)malloc(sizeof(Image));
    if (!img) return NULL;

    img->width = width;
    img->height = height;
    img->data = (Pixel*)calloc(width * height, sizeof(Pixel));

    if (!img->data) {
        free(img);
        return NULL;
    }

    return img;
}

void freeImage(Image *img) {
    if (img) {
        if (img->data) free(img->data);
        free(img);
    }
}

Pixel getPixel(const Image *img, int x, int y) {
    return img->data[y * img->width + x];
}

void setPixel(Image *img, int x, int y, Pixel p) {
    img->data[y * img->width + x] = p;
}

/* ============================================================================
 * FUNGSI CLAMP & MIN
 * ============================================================================ */

float clampf(float value, float minVal, float maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

int mini(int a, int b) {
    return (a < b) ? a : b;
}

/* ============================================================================
 * BILINEAR INTERPOLATION (Core Algorithm)
 * ============================================================================ */

Pixel bilinearInterpolate(const Image *img, float x, float y) {
    float maxX, maxY, fx, fy;
    int x0, y0, x1, y1;
    Pixel f00, f10, f01, f11, result;
    float w00, w10, w01, w11;

    /* Clamp koordinat */
    maxX = (float)img->width - 1.001f;
    maxY = (float)img->height - 1.001f;
    x = clampf(x, 0.0f, maxX);
    y = clampf(y, 0.0f, maxY);

    /* Tentukan 4 pixel tetangga */
    x0 = (int)floor(x);
    y0 = (int)floor(y);
    x1 = mini(x0 + 1, img->width - 1);
    y1 = mini(y0 + 1, img->height - 1);

    /* Hitung fraksi */
    fx = x - (float)x0;
    fy = y - (float)y0;

    /* Ambil nilai 4 tetangga */
    f00 = getPixel(img, x0, y0);
    f10 = getPixel(img, x1, y0);
    f01 = getPixel(img, x0, y1);
    f11 = getPixel(img, x1, y1);

    /* Hitung bobot */
    w00 = (1.0f - fx) * (1.0f - fy);
    w10 = fx * (1.0f - fy);
    w01 = (1.0f - fx) * fy;
    w11 = fx * fy;

    /* Hitung hasil (weighted sum) */
    result.r = f00.r * w00 + f10.r * w10 + f01.r * w01 + f11.r * w11;
    result.g = f00.g * w00 + f10.g * w10 + f01.g * w01 + f11.g * w11;
    result.b = f00.b * w00 + f10.b * w10 + f01.b * w01 + f11.b * w11;

    return result;
}

/* ============================================================================
 * RESIZE IMAGE - SERIAL
 * ============================================================================ */

Image* resizeSerial(const Image *source, int newWidth, int newHeight) {
    Image *dest;
    float scaleX, scaleY;
    int x, y;

    dest = createImage(newWidth, newHeight);
    if (!dest) return NULL;

    scaleX = (float)source->width / newWidth;
    scaleY = (float)source->height / newHeight;

    /* Loop serial - sequential */
    for (y = 0; y < newHeight; y++) {
        for (x = 0; x < newWidth; x++) {
            float srcX = x * scaleX;
            float srcY = y * scaleY;
            Pixel p = bilinearInterpolate(source, srcX, srcY);
            setPixel(dest, x, y, p);
        }
    }

    return dest;
}

/* ============================================================================
 * RESIZE IMAGE - OPENMP PARALLEL
 * ============================================================================ */

#ifdef USE_OPENMP
Image* resizeOpenMP(const Image *source, int newWidth, int newHeight, int numThreads) {
    Image *dest;
    float scaleX, scaleY;
    int x, y;

    dest = createImage(newWidth, newHeight);
    if (!dest) return NULL;

    scaleX = (float)source->width / newWidth;
    scaleY = (float)source->height / newHeight;

    omp_set_num_threads(numThreads);

    /* Loop parallel dengan OpenMP */
    #pragma omp parallel for collapse(2) private(x, y)
    for (y = 0; y < newHeight; y++) {
        for (x = 0; x < newWidth; x++) {
            float srcX = x * scaleX;
            float srcY = y * scaleY;
            Pixel p = bilinearInterpolate(source, srcX, srcY);
            setPixel(dest, x, y, p);
        }
    }

    return dest;
}
#endif

/* ============================================================================
 * CREATE TEST IMAGE
 * ============================================================================ */

Image* createTestImage(int size) {
    Image *img;
    int x, y;

    img = createImage(size, size);
    if (!img) return NULL;

    /* Isi dengan gradient pattern */
    for (y = 0; y < size; y++) {
        for (x = 0; x < size; x++) {
            float val = (float)((x + y) % 256);
            Pixel p;
            p.r = val;
            p.g = val;
            p.b = val;
            setPixel(img, x, y, p);
        }
    }

    return img;
}

/* ============================================================================
 * BENCHMARK FUNCTION
 * ============================================================================ */

void runBenchmark() {
    int testSizes[] = {512, 1024, 2048};
    int numTests = 3;
    int targetSize = 2048;
    int t, i;

    printf("\n");
    printf("========================================================================\n");
    printf("      BENCHMARK: BILINEAR INTERPOLATION (Pure C with OpenMP)\n");
    printf("========================================================================\n\n");

    for (t = 0; t < numTests; t++) {
        int size = testSizes[t];
        Image *testImg;
        Image *resultSerial;
        clock_t startTime, endTime;
        double timeSerial;

        printf("Test: Resize %dx%d -> %dx%d\n", size, size, targetSize, targetSize);
        printf("------------------------------------------------------------------------\n");

        /* Create test image */
        testImg = createTestImage(size);
        if (!testImg) {
            printf("Error: Failed to create test image\n");
            continue;
        }

        /* BENCHMARK SERIAL */
        startTime = clock();
        resultSerial = resizeSerial(testImg, targetSize, targetSize);
        endTime = clock();
        timeSerial = ((double)(endTime - startTime)) / CLOCKS_PER_SEC * 1000.0;

        printf("  [SERIAL]       Time: %7.0f ms\n", timeSerial);

        if (resultSerial) freeImage(resultSerial);

        /* BENCHMARK OPENMP */
#ifdef USE_OPENMP
        {
            int threadCounts[] = {2, 4, 8};
            int numThreadTests = 3;

            for (i = 0; i < numThreadTests; i++) {
                int threads = threadCounts[i];
                Image *resultOmp;
                double timeOmp, speedup;

                startTime = clock();
                resultOmp = resizeOpenMP(testImg, targetSize, targetSize, threads);
                endTime = clock();
                timeOmp = ((double)(endTime - startTime)) / CLOCKS_PER_SEC * 1000.0;

                speedup = timeSerial / timeOmp;
                printf("  [OpenMP-%d]     Time: %7.0f ms  |  Speedup: %.2fx\n",
                       threads, timeOmp, speedup);

                if (resultOmp) freeImage(resultOmp);
            }
        }
#else
        printf("  [OpenMP]       Not compiled (compile with -fopenmp -DUSE_OPENMP)\n");
#endif

        printf("\n");
        freeImage(testImg);
    }

    printf("========================================================================\n");
}

/* ============================================================================
 * PRINT CONCEPT
 * ============================================================================ */

void printConcept() {
    printf("\n");
    printf("========================================================================\n");
    printf("                      ANALISIS KONSEP\n");
    printf("========================================================================\n\n");

    printf("1. SERIAL (Sequential)\n");
    printf("   - Implementasi langsung rumus: f(x,y) = sum(f(xi,yi) * w(xi,yi))\n");
    printf("   - Eksekusi berurutan pixel per pixel\n");
    printf("   - Kompleksitas: O(n*m) untuk resize n×m\n");
    printf("   - Keuntungan: Simple, predictable, no overhead\n");
    printf("   - Kekurangan: Tidak memanfaatkan multi-core CPU\n\n");

#ifdef USE_OPENMP
    printf("2. PARALLEL OpenMP\n");
    printf("   - Paralelisasi dengan #pragma omp parallel for\n");
    printf("   - Compiler otomatis membagi loop ke threads\n");
    printf("   - collapse(2): Paralelisasi 2 nested loops sekaligus\n");
    printf("   - Keuntungan: Mudah implement, kontrol threads eksplisit\n");
    printf("   - Overhead: Thread creation & synchronization\n\n");
#else
    printf("2. PARALLEL OpenMP\n");
    printf("   - [NOT COMPILED] Compile dengan -fopenmp -DUSE_OPENMP\n\n");
#endif

    printf("Expected Speedup:\n");
    printf("   - Ideal: S = P (P = jumlah cores)\n");
    printf("   - Real: S < P (karena overhead & Amdahl's law)\n");
    printf("   - OpenMP: ~60-80%% efficiency pada 4-8 threads\n");
    printf("========================================================================\n\n");
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main() {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║     BILINEAR INTERPOLATION: SERIAL vs PARALLEL (C + OpenMP)  ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");

    printConcept();

    printf("Compilation Mode:\n");
    printf("  Language: Pure C (C99)\n");
    printf("  Serial:   ENABLED\n");

#ifdef USE_OPENMP
    printf("  OpenMP:   ENABLED\n");
    printf("  Max Threads: %d\n", omp_get_max_threads());
#else
    printf("  OpenMP:   DISABLED (compile with -fopenmp -DUSE_OPENMP)\n");
#endif

    runBenchmark();

    return 0;
}
