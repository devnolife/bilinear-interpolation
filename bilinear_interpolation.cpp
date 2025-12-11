/**
 * ╔═══════════════════════════════════════════════════════════════════════════╗
 * ║                    BILINEAR INTERPOLATION                                 ║
 * ║                   Image Processing Algorithm                              ║
 * ╠═══════════════════════════════════════════════════════════════════════════╣
 * ║  Author  : Andi Agung Dwi Arya                                            ║
 * ║  GitHub  : devnolife                                                      ║
 * ║  Course  : Algoritma Komputasi S2 - 2025                                  ║
 * ╚═══════════════════════════════════════════════════════════════════════════╝
 * 
 * File ini berisi implementasi LENGKAP Bilinear Interpolation dalam satu file
 * C++ tanpa memerlukan file header (.h) terpisah.
 * 
 * Compile: g++ -o bilinear bilinear_interpolation.cpp -std=c++17
 * Run    : ./bilinear
 */

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  BAGIAN 1: STRUKTUR DATA PIXEL                                            ║
// ╚═══════════════════════════════════════════════════════════════════════════╝
/**
 * Struct Pixel menyimpan nilai warna untuk satu titik dalam gambar.
 * 
 * Dalam gambar digital:
 * - Setiap pixel memiliki 3 komponen warna: Red, Green, Blue (RGB)
 * - Nilai berkisar 0-255 (8-bit per channel)
 * - Kita gunakan float untuk presisi saat interpolasi
 */
struct Pixel {
    float r;  // Red   (0-255)
    float g;  // Green (0-255)
    float b;  // Blue  (0-255)
    
    // Constructor default: pixel hitam
    Pixel() : r(0), g(0), b(0) {}
    
    // Constructor dengan nilai RGB
    Pixel(float red, float green, float blue) : r(red), g(green), b(blue) {}
    
    // Constructor untuk grayscale (satu nilai untuk semua channel)
    Pixel(float gray) : r(gray), g(gray), b(gray) {}
};


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  BAGIAN 2: KELAS IMAGE                                                    ║
// ╚═══════════════════════════════════════════════════════════════════════════╝
/**
 * Class Image merepresentasikan gambar 2D sebagai grid pixel.
 * 
 * Struktur penyimpanan:
 * - Gambar disimpan dalam vector 1D untuk efisiensi memory
 * - Akses pixel (x,y) menggunakan rumus: data[y * width + x]
 * 
 * Contoh gambar 4x3:
 *     x=0   x=1   x=2   x=3
 *   +-----+-----+-----+-----+
 * y=0|  0  |  1  |  2  |  3  |
 *   +-----+-----+-----+-----+
 * y=1|  4  |  5  |  6  |  7  |
 *   +-----+-----+-----+-----+
 * y=2|  8  |  9  | 10  | 11  |
 *   +-----+-----+-----+-----+
 */
class Image {
private:
    vector<Pixel> data;   // Array 1D untuk menyimpan semua pixel
    int width;            // Lebar gambar (jumlah kolom)
    int height;           // Tinggi gambar (jumlah baris)

public:
    // Constructor: buat gambar kosong dengan ukuran tertentu
    Image(int w, int h) : width(w), height(h) {
        data.resize(w * h);  // Alokasi memory untuk semua pixel
    }
    
    // Getter untuk dimensi
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
    /**
     * Mengambil pixel pada koordinat (x, y)
     * 
     * @param x - Koordinat horizontal (kolom), 0 <= x < width
     * @param y - Koordinat vertical (baris), 0 <= y < height
     * @return Reference ke Pixel pada posisi tersebut
     * 
     * Rumus konversi 2D ke 1D: index = y * width + x
     */
    Pixel& getPixel(int x, int y) {
        return data[y * width + x];
    }
    
    // Versi const untuk read-only access
    const Pixel& getPixel(int x, int y) const {
        return data[y * width + x];
    }
    
    // Set nilai pixel pada koordinat (x, y)
    void setPixel(int x, int y, const Pixel& p) {
        data[y * width + x] = p;
    }
};


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  BAGIAN 3: FUNGSI UTILITAS                                                ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

/**
 * Fungsi clamp: membatasi nilai dalam range tertentu
 * 
 * @param value  - Nilai yang akan dibatasi
 * @param minVal - Batas minimum
 * @param maxVal - Batas maximum
 * @return Nilai yang sudah dibatasi
 * 
 * Contoh:
 *   clamp(150, 0, 100) = 100  (melebihi max, jadi diset ke max)
 *   clamp(-10, 0, 100) = 0    (kurang dari min, jadi diset ke min)
 *   clamp(50, 0, 100)  = 50   (dalam range, tidak berubah)
 */
float clamp(float value, float minVal, float maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  BAGIAN 4: INTI ALGORITMA - BILINEAR INTERPOLATION                        ║
// ╚═══════════════════════════════════════════════════════════════════════════╝
/**
 * ============================================================================
 *                         BILINEAR INTERPOLATION
 * ============================================================================
 * 
 * Bilinear interpolation menghitung nilai pixel pada koordinat NON-INTEGER
 * dengan menggunakan 4 pixel tetangga terdekat.
 * 
 * Ilustrasi:
 * 
 *      x0              x1
 *      |               |
 * y0 --+---------------+--
 *      | f(x0,y0)      | f(x1,y0)
 *      |       *-------|--> fx
 *      |       | P     |
 *      |       v fy    |
 * y1 --+---------------+--
 *      | f(x0,y1)      | f(x1,y1)
 * 
 * P adalah titik yang ingin kita interpolasi
 * fx = jarak horizontal dari x0
 * fy = jarak vertical dari y0
 * 
 * Formula:
 * f(P) = f(x0,y0) * (1-fx) * (1-fy) + f(x1,y0) * fx * (1-fy)
 *      + f(x0,y1) * (1-fx) * fy     + f(x1,y1) * fx * fy
 * 
 * ============================================================================
 */

/**
 * Fungsi utama bilinear interpolation
 * 
 * @param image - Gambar sumber
 * @param x     - Koordinat x (float, bisa desimal)
 * @param y     - Koordinat y (float, bisa desimal)
 * @return      - Pixel hasil interpolasi
 */
Pixel bilinearInterpolate(const Image& image, float x, float y) {
    
    // =========================================================================
    // LANGKAH 1: Batasi koordinat agar tidak keluar dari gambar
    // =========================================================================
    // Koordinat harus dalam range [0, dimension-1]
    // Kita kurangi sedikit (0.001) untuk menghindari akses di luar batas
    
    float maxX = (float)image.getWidth() - 1.001f;
    float maxY = (float)image.getHeight() - 1.001f;
    
    x = clamp(x, 0.0f, maxX);
    y = clamp(y, 0.0f, maxY);
    
    
    // =========================================================================
    // LANGKAH 2: Tentukan 4 pixel tetangga
    // =========================================================================
    // floor() membulatkan ke bawah untuk mendapatkan koordinat integer
    //
    //  (x0,y0) ---- (x1,y0)
    //     |           |
    //     |     P     |
    //     |           |
    //  (x0,y1) ---- (x1,y1)
    
    int x0 = (int)floor(x);  // Koordinat kiri
    int y0 = (int)floor(y);  // Koordinat atas
    int x1 = x0 + 1;         // Koordinat kanan
    int y1 = y0 + 1;         // Koordinat bawah
    
    // Pastikan tidak melebihi batas gambar
    x1 = min(x1, image.getWidth() - 1);
    y1 = min(y1, image.getHeight() - 1);
    
    
    // =========================================================================
    // LANGKAH 3: Hitung fraksi (jarak relatif dari pojok kiri atas)
    // =========================================================================
    // fx = seberapa jauh ke KANAN dari x0 (range 0-1)
    // fy = seberapa jauh ke BAWAH dari y0 (range 0-1)
    //
    // Contoh: jika x = 2.7 dan x0 = 2, maka fx = 2.7 - 2 = 0.7
    //         artinya titik P 70% ke kanan dari x0
    
    float fx = x - floor(x);  // Fraksi horizontal
    float fy = y - floor(y);  // Fraksi vertical
    
    
    // =========================================================================
    // LANGKAH 4: Ambil nilai 4 pixel tetangga
    // =========================================================================
    
    Pixel f00 = image.getPixel(x0, y0);  // Kiri-Atas     (Top-Left)
    Pixel f10 = image.getPixel(x1, y0);  // Kanan-Atas    (Top-Right)
    Pixel f01 = image.getPixel(x0, y1);  // Kiri-Bawah    (Bottom-Left)
    Pixel f11 = image.getPixel(x1, y1);  // Kanan-Bawah   (Bottom-Right)
    
    
    // =========================================================================
    // LANGKAH 5: Hitung bobot (weights) untuk setiap tetangga
    // =========================================================================
    /**
     * Bobot dihitung berdasarkan LUAS persegi panjang diagonal.
     * 
     *  (x0,y0)-----------------(x1,y0)
     *     |                      |
     *     |    w11  |   w01      |    w00 = (1-fx)(1-fy) = area diagonal dari P ke (x1,y1)
     *     |   ------P------      |    w10 = fx(1-fy)     = area diagonal dari P ke (x0,y1)
     *     |    w10  |   w00      |    w01 = (1-fx)fy     = area diagonal dari P ke (x1,y0)
     *     |                      |    w11 = fx * fy      = area diagonal dari P ke (x0,y0)
     *  (x0,y1)-----------------(x1,y1)
     * 
     * Properti penting: w00 + w10 + w01 + w11 = 1.0 (total bobot = 100%)
     */
    
    float w00 = (1.0f - fx) * (1.0f - fy);  // Bobot untuk kiri-atas
    float w10 = fx * (1.0f - fy);            // Bobot untuk kanan-atas
    float w01 = (1.0f - fx) * fy;            // Bobot untuk kiri-bawah
    float w11 = fx * fy;                     // Bobot untuk kanan-bawah
    
    
    // =========================================================================
    // LANGKAH 6: Hitung hasil interpolasi (weighted sum)
    // =========================================================================
    // Hasil = jumlah (nilai pixel * bobot) untuk setiap channel warna
    
    Pixel result;
    
    result.r = f00.r * w00 + f10.r * w10 + f01.r * w01 + f11.r * w11;
    result.g = f00.g * w00 + f10.g * w10 + f01.g * w01 + f11.g * w11;
    result.b = f00.b * w00 + f10.b * w10 + f01.b * w01 + f11.b * w11;
    
    return result;
}


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  BAGIAN 5: FUNGSI RESIZE GAMBAR                                           ║
// ╚═══════════════════════════════════════════════════════════════════════════╝
/**
 * ============================================================================
 *                           IMAGE RESIZE
 * ============================================================================
 * 
 * Resize gambar menggunakan bilinear interpolation untuk setiap pixel baru.
 * 
 * Proses:
 * 1. Hitung scale factor (rasio ukuran lama : baru)
 * 2. Untuk setiap pixel di gambar baru:
 *    - Mapping koordinat baru -> koordinat di gambar asli (float)
 *    - Interpolasi untuk mendapatkan nilai pixel
 * 
 * Contoh resize 4x4 -> 6x6:
 * 
 *   Source (4x4)           Destination (6x6)
 *   +--+--+--+--+          +--+--+--+--+--+--+
 *   |  |  |  |  |          |  |  |  |  |  |  |
 *   +--+--+--+--+    ->    +--+--+--+--+--+--+
 *   |  |  |  |  |          |  |  |  |  |  |  |
 *   +--+--+--+--+          +--+--+--+--+--+--+
 *   |  |  |  |  |          |  |  |  |  |  |  |
 *   +--+--+--+--+          +--+--+--+--+--+--+
 *   |  |  |  |  |          |  |  |  |  |  |  |
 *                          +--+--+--+--+--+--+
 *   scale_x = 4/6 = 0.67   |  |  |  |  |  |  |
 *   scale_y = 4/6 = 0.67   +--+--+--+--+--+--+
 * 
 * ============================================================================
 */

/**
 * Resize gambar menggunakan Bilinear Interpolation
 * 
 * @param source    - Gambar asli
 * @param newWidth  - Lebar gambar baru
 * @param newHeight - Tinggi gambar baru
 * @return          - Gambar hasil resize
 */
Image resizeImage(const Image& source, int newWidth, int newHeight) {
    
    // Buat gambar baru dengan ukuran target
    Image destination(newWidth, newHeight);
    
    // Hitung scale factor
    // scale_x = berapa pixel sumber per 1 pixel tujuan
    float scaleX = (float)source.getWidth() / newWidth;
    float scaleY = (float)source.getHeight() / newHeight;
    
    // Loop setiap pixel di gambar tujuan
    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {
            
            // Mapping: koordinat tujuan -> koordinat sumber
            // Contoh: jika newWidth=6, sourceWidth=4, x=3
            //         sourceX = 3 * (4/6) = 3 * 0.67 = 2.0
            float sourceX = x * scaleX;
            float sourceY = y * scaleY;
            
            // Interpolasi pada koordinat float
            Pixel interpolatedPixel = bilinearInterpolate(source, sourceX, sourceY);
            
            // Set pixel di gambar tujuan
            destination.setPixel(x, y, interpolatedPixel);
        }
    }
    
    return destination;
}


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  BAGIAN 6: FUNGSI DISPLAY DAN DEMO                                        ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

/**
 * Menampilkan gambar dalam format grid ASCII
 */
void displayImage(const Image& img, const string& title) {
    cout << "\n" << title << " (" << img.getWidth() << "x" << img.getHeight() << "):\n";
    cout << "+";
    for (int x = 0; x < img.getWidth(); x++) cout << "------";
    cout << "-+\n";
    
    for (int y = 0; y < img.getHeight(); y++) {
        cout << "|";
        for (int x = 0; x < img.getWidth(); x++) {
            cout << " " << setw(4) << fixed << setprecision(0) << img.getPixel(x, y).r << " ";
        }
        cout << " |\n";
    }
    
    cout << "+";
    for (int x = 0; x < img.getWidth(); x++) cout << "------";
    cout << "-+\n";
}


/**
 * Demo step-by-step interpolasi satu pixel
 */
void demoSingleInterpolation(const Image& img, float x, float y) {
    cout << "\n";
    cout << "+=====================================================================+\n";
    cout << "|       DEMO: Interpolasi pada koordinat (" << fixed << setprecision(2) << x << ", " << y << ")                |\n";
    cout << "+=====================================================================+\n\n";
    
    // Langkah 1: Cari tetangga
    int x0 = (int)floor(x);
    int y0 = (int)floor(y);
    int x1 = min(x0 + 1, img.getWidth() - 1);
    int y1 = min(y0 + 1, img.getHeight() - 1);
    
    cout << "LANGKAH 1: Tentukan 4 pixel tetangga\n";
    cout << "-------------------------------------\n";
    cout << "  x0 = floor(" << x << ") = " << x0 << "\n";
    cout << "  y0 = floor(" << y << ") = " << y0 << "\n";
    cout << "  x1 = x0 + 1 = " << x1 << "\n";
    cout << "  y1 = y0 + 1 = " << y1 << "\n\n";
    
    // Langkah 2: Ambil nilai
    float f00 = img.getPixel(x0, y0).r;
    float f10 = img.getPixel(x1, y0).r;
    float f01 = img.getPixel(x0, y1).r;
    float f11 = img.getPixel(x1, y1).r;
    
    cout << "LANGKAH 2: Ambil nilai 4 tetangga\n";
    cout << "-------------------------------------\n";
    cout << "  f00 = image[" << x0 << "][" << y0 << "] = " << f00 << " (Kiri-Atas)\n";
    cout << "  f10 = image[" << x1 << "][" << y0 << "] = " << f10 << " (Kanan-Atas)\n";
    cout << "  f01 = image[" << x0 << "][" << y1 << "] = " << f01 << " (Kiri-Bawah)\n";
    cout << "  f11 = image[" << x1 << "][" << y1 << "] = " << f11 << " (Kanan-Bawah)\n\n";
    
    // Langkah 3: Hitung fraksi
    float fx = x - floor(x);
    float fy = y - floor(y);
    
    cout << "LANGKAH 3: Hitung fraksi (fx, fy)\n";
    cout << "-------------------------------------\n";
    cout << "  fx = " << x << " - " << x0 << " = " << fx << "\n";
    cout << "  fy = " << y << " - " << y0 << " = " << fy << "\n\n";
    
    // Langkah 4: Hitung bobot
    float w00 = (1 - fx) * (1 - fy);
    float w10 = fx * (1 - fy);
    float w01 = (1 - fx) * fy;
    float w11 = fx * fy;
    
    cout << "LANGKAH 4: Hitung bobot (weights)\n";
    cout << "-------------------------------------\n";
    cout << "  w00 = (1-" << fx << ") x (1-" << fy << ") = " << setprecision(4) << w00 << "\n";
    cout << "  w10 = " << fx << " x (1-" << fy << ")     = " << w10 << "\n";
    cout << "  w01 = (1-" << fx << ") x " << fy << "     = " << w01 << "\n";
    cout << "  w11 = " << fx << " x " << fy << "         = " << w11 << "\n";
    cout << "  Total = " << (w00 + w10 + w01 + w11) << " (harus = 1.0)\n\n";
    
    // Langkah 5: Hasil
    float result = f00 * w00 + f10 * w10 + f01 * w01 + f11 * w11;
    
    cout << "LANGKAH 5: Hitung hasil interpolasi\n";
    cout << "-------------------------------------\n";
    cout << "  f(x,y) = f00 x w00 + f10 x w10 + f01 x w01 + f11 x w11\n";
    cout << "         = " << f00 << " x " << w00 << " + " << f10 << " x " << w10 << " + " << f01 << " x " << w01 << " + " << f11 << " x " << w11 << "\n";
    cout << "         = " << (f00*w00) << " + " << (f10*w10) << " + " << (f01*w01) << " + " << (f11*w11) << "\n";
    cout << "         = " << setprecision(4) << result << "\n\n";
    
    cout << "+=====================================================================+\n";
    cout << "|  HASIL: f(" << setprecision(2) << x << ", " << y << ") = " << setprecision(4) << result << "                                       |\n";
    cout << "+=====================================================================+\n";
}


// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║  BAGIAN 7: MAIN PROGRAM                                                   ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

int main() {
    cout << "\n";
    cout << "+=====================================================================+\n";
    cout << "|                                                                     |\n";
    cout << "|               BILINEAR INTERPOLATION DEMO                           |\n";
    cout << "|                 Image Processing Algorithm                          |\n";
    cout << "|                                                                     |\n";
    cout << "+=====================================================================+\n";
    cout << "|  Author  : Andi Agung Dwi Arya                                      |\n";
    cout << "|  GitHub  : devnolife                                                |\n";
    cout << "|  Course  : Algoritma Komputasi S2 - 2025                            |\n";
    cout << "+=====================================================================+\n";
    
    // Buat gambar sample 4x4 dengan nilai grayscale
    Image sourceImage(4, 4);
    
    // Isi dengan nilai gradient
    int values[4][4] = {
        { 10,  30,  50,  70},
        { 40,  60,  80, 100},
        { 70,  90, 110, 130},
        {100, 120, 140, 160}
    };
    
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            sourceImage.setPixel(x, y, Pixel(values[y][x]));
        }
    }
    
    // Tampilkan gambar asli
    displayImage(sourceImage, "GAMBAR ASLI");
    
    // Menu interaktif
    int choice;
    do {
        cout << "\n";
        cout << "+-------------------------------------+\n";
        cout << "|            MENU UTAMA               |\n";
        cout << "+-------------------------------------+\n";
        cout << "|  [1] Demo Interpolasi Single Pixel  |\n";
        cout << "|  [2] Demo Resize Gambar             |\n";
        cout << "|  [3] Input Koordinat Custom         |\n";
        cout << "|  [0] Keluar                         |\n";
        cout << "+-------------------------------------+\n";
        cout << "Pilihan: ";
        cin >> choice;
        
        switch (choice) {
            case 1: {
                // Demo dengan koordinat preset
                demoSingleInterpolation(sourceImage, 1.5f, 1.5f);
                break;
            }
            
            case 2: {
                // Demo resize
                cout << "\nResize gambar dari 4x4 ke 6x6...\n";
                Image resized = resizeImage(sourceImage, 6, 6);
                
                displayImage(sourceImage, "SEBELUM (4x4)");
                displayImage(resized, "SESUDAH (6x6)");
                
                cout << "\nSetiap pixel baru dihitung dengan bilinear interpolation!\n";
                break;
            }
            
            case 3: {
                // Input custom
                float cx, cy;
                cout << "Masukkan koordinat X (0.0 - 2.99): ";
                cin >> cx;
                cout << "Masukkan koordinat Y (0.0 - 2.99): ";
                cin >> cy;
                
                demoSingleInterpolation(sourceImage, cx, cy);
                break;
            }
            
            case 0:
                cout << "\nTerima kasih! Sampai jumpa.\n\n";
                break;
                
            default:
                cout << "\nPilihan tidak valid!\n";
        }
        
    } while (choice != 0);
    
    return 0;
}
