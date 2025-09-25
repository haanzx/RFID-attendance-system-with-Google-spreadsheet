# 💻 Sistem Absensi RFID Online (ESP32 & Google Sheets)

Proyek ini adalah sistem absensi modern yang memanfaatkan modul **RFID** dan **ESP32** untuk mencatat waktu **Datang** dan **Pulang** secara *real-time* ke dalam Google Sheets. Sistem ini dirancang untuk mengatasi masalah duplikasi data dengan memperbarui baris absensi yang sudah ada, bukan membuat baris baru.

---

## ✨ Fitur Utama Program

1.  **Anti-Duplikasi Data**: Google Apps Script secara cerdas mencari UID dan Tanggal yang sama. Jika ditemukan, waktu akan diperbarui (misalnya, mengisi Waktu Pulang), bukan menambah baris baru.
2.  **Pemisahan Waktu Absensi**: Data Waktu Datang dan Waktu Pulang dicatat di kolom yang berbeda di Google Sheets.
3.  **Alur Tombol-ke-Kartu**: Pengguna harus menekan tombol (**Masuk** atau **Pulang**) terlebih dahulu sebelum menempelkan kartu RFID, memastikan niat absensi yang jelas.
4.  **Umpan Balik Audio (Buzzer)**:
    * Bunyi **sekali** untuk absensi berhasil atau koneksi WiFi sukses.
    * Bunyi **cepat berulang** (tiga kali) untuk kegagalan (gagal kirim data, *timeout*, atau WiFi terputus).
5.  **Umpan Balik Visual (LCD)**: Menampilkan instruksi, status koneksi, dan hasil pengiriman data secara *real-time*.

---

## 🛠️ Hardware yang Digunakan

| Komponen | Deskripsi |
| :--- | :--- |
| **Mikrokontroler** | ESP32 DEVKIT V1 |
| **Pembaca Kartu** | Modul RFID RC522 |
| **Layar** | LCD 16x2 I2C |
| **Umpan Balik Audio** | Buzzer Aktif |
| **Input** | 2x Tombol *Push Button* |

---

## 📌 Skema Wiring (Pinout)

Berikut adalah ringkasan koneksi pin yang digunakan dalam program ini, berdasarkan penyesuaian GPIO Anda:

| Komponen | Pin Modul | Pin ESP32 | Keterangan |
| :--- | :--- | :--- | :--- |
| **RFID RC522** | RST | **GPIO 4** | Reset Pin |
| | SDA (SS) | **GPIO 5** | SPI Slave Select |
| | MOSI | **GPIO 23** | SPI Data Out |
| | MISO | **GPIO 19** | SPI Data In |
| | SCK | **GPIO 18** | SPI Clock |
| **LCD I2C** | SDA | **GPIO 21** | I2C Data (Umumnya) |
| | SCL | **GPIO 22** | I2C Clock (Umumnya) |
| **Tombol Masuk** | Kaki Sinyal | **GPIO 13** | Input `INPUT_PULLUP` |
| **Tombol Pulang** | Kaki Sinyal | **GPIO 15** | Input `INPUT_PULLUP` |
| **Buzzer** | Kaki Positif (+) | **GPIO 2** | Umpan Balik Audio |

*Catatan: Pin ESP32 13, 15, dan 2 menggunakan resistor pull-up internal. Kaki sinyal pada tombol dan buzzer cukup dihubungkan ke **GND**.*

---

## ☁️ Logika Program (Konsep)

### 1. Program ESP32 (Arduino IDE)

Tugas utama ESP32 adalah sebagai klien jaringan:
* **Deteksi Tombol**: Di dalam fungsi `loop()`, ESP32 secara konstan memantau penekanan tombol **Masuk (GPIO 13)** atau **Pulang (GPIO 15)**.
* **Pembacaan Kartu**: Setelah tombol ditekan, fungsi `waitForCardAndSend()` diaktifkan, memberikan batas waktu 10 detik untuk menempelkan kartu.
* **Pengiriman Data**: Fungsi `sendDataToGoogleSheets()` mem-format UID dan Status (`Masuk` atau `Pulang`) menjadi *query* URL (`?id=UID&status=Status`) dan mengirimkannya ke URL Web App.
* **Feedback**: Hasil HTTP Code (Absensi Berhasil/Gagal) menentukan apakah `beepSuccess()` atau `beepFailure()` dipanggil.

### 2. Google Apps Script (Sisi Server)

Kode JavaScript ini adalah inti dari logika absensi yang anti-duplikasi:
1.  **Penerimaan Data**: Fungsi `doGet(e)` menerima UID (`e.parameter.id`) dan Status (`e.parameter.status`).
2.  **Format Tanggal Kuat**: Menggunakan `Utilities.formatDate()` untuk mendapatkan format tanggal (`yyyy-MM-dd`) dan waktu (`HH:mm:ss`) yang **konsisten** dan tidak terpengaruh pengaturan regional *spreadsheet*.
3.  **Pencarian Baris**: Skrip melakukan iterasi melalui `Sheet4`, mencari baris yang memiliki **UID yang sama** (Kolom A) **DAN Tanggal Hari Ini** (Kolom B).
4.  **Logika Update/Append**:
    * Jika baris ditemukan: Skrip memperbarui waktu yang sesuai (Kolom C untuk **Masuk** atau Kolom D untuk **Pulang**) pada baris tersebut. **Ini mencegah duplikasi baris.**
    * Jika baris tidak ditemukan: Skrip menambahkan baris baru dengan UID, Tanggal, dan Waktu Datang/Pulang yang sesuai.

---

## 📊 Struktur Data Google Sheets (`Sheet4`)

Pastikan *header* di **Sheet4** diatur dengan tepat agar Apps Script dapat membaca dan menulis data ke kolom yang benar.

| Kolom A | Kolom B | Kolom C | Kolom D |
| :---: | :---: | :---: | :---: |
| **UID** | **Tanggal Absen** | **Waktu Datang** | **Waktu Pulang** |

---

## 🚀 Panduan Penggunaan (Deployment)

1.  **Siapkan Spreadsheet**: Buat *spreadsheet* baru dan pastikan ada *sheet* bernama **`Sheet4`** dengan *header* kolom di atas.
2.  **Deploy Apps Script**:
    * Salin kode Apps Script ke editor di Google Sheets.
    * Deploy sebagai **Web App** dengan akses **"Anyone"**.
    * Salin **URL Web App** yang dihasilkan.
3.  **Konfigurasi ESP32**:
    * Masukkan `ssid`, `password` WiFi, dan **URL Web App** yang sudah Anda salin ke dalam kode Arduino.
4.  **Uji Coba**:
    * Unggah kode ke ESP32.
    * Setelah terhubung, tekan tombol **Masuk** dan tempelkan kartu. Cek data di Kolom C.
    * Tekan tombol **Pulang** dan tempelkan kartu yang sama. Cek data di Kolom D (Kolom C tidak boleh bertambah).
