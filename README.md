# 💳 Sistem Absensi RFID Online: ESP32/ESP8266 ke Google Sheets

Proyek ini menyajikan solusi absensi *real-time* menggunakan mikrokontroler **ESP32** atau **ESP8266**, modul **RFID RC522**, dan **Google Apps Script** untuk pencatatan data ke Google Sheets dengan mekanisme anti-duplikasi yang cerdas.

## ✨ Fitur Utama Program

1.  **Anti-Duplikasi Data**: Google Apps Script mencari UID dan Tanggal yang sama. Jika ditemukan, waktu akan diperbarui (mengisi Waktu Pulang), bukan menambah baris baru.
2.  **Pemisahan Waktu Absensi**: Data Waktu Datang dan Waktu Pulang dicatat di kolom yang berbeda di Google Sheets.
3.  **Alur Tombol-ke-Kartu**: Pengguna harus menekan tombol (**Masuk** atau **Pulang**) terlebih dahulu sebelum menempelkan kartu RFID, memastikan niat absensi yang jelas.
4.  **Umpan Balik Audio (Buzzer)**:
    * Bunyi **sekali** untuk absensi berhasil atau koneksi WiFi sukses.
    * Bunyi **cepat berulang** (tiga kali) untuk kegagalan (gagal kirim data, *timeout*, atau WiFi terputus).
5.  **Umpan Balik Visual (LCD)**: Menampilkan instruksi, status koneksi, dan hasil pengiriman data secara *real-time*.

## 🛠️ Hardware yang Digunakan

| Komponen | Deskripsi |
| :--- | :--- |
| **Mikrokontroler** | ESP32 DEVKIT V1 atau ESP8266 NodeMCU |
| **Pembaca Kartu** | Modul RFID RC522 |
| **Layar** | LCD 16x2 I2C |
| **Umpan Balik Audio** | Buzzer Aktif |
| **Input** | 2x Tombol *Push Button* |

## 📌 Skema Wiring (Pinout untuk ESP32)

Berikut adalah ringkasan koneksi pin yang digunakan dalam file `esp32.cpp` dan `esp8266.cpp`:

| Komponen | Pin Modul | Pin ESP32 | Pin ESP8266 | Keterangan |
| :--- | :--- | :--- | :--- | :--- |
| **RFID RC522** | RST | **D4** | **D0** | Reset Pin (`RST_PIN`) |
| | SDA (SS) | **D5** | **D8** | SPI Slave Select (`SS_PIN`) |
| | MOSI | **D23** | **D7** | SPI Data Out (`MOSI_PIN`) |
| | MISO | **D19** | **D6** | SPI Data In (`MISO_PIN`) |
| | SCK | **D18** | **D5** | SPI Clock (`SCK_PIN`) |
| **LCD I2C** | SDA | **D21** | **D2** | I2C Data (Umumnya) |
| | SCL | **D22** | **D1** | I2C Clock (Umumnya) |
| **Tombol Masuk** | Kaki Sinyal | **D13** | **D3** | Input `INPUT_PULLUP` (`BUTTON_MASUK_PIN`) |
| **Tombol Pulang**| Kaki Sinyal | **D15** | **D4** | Input `INPUT_PULLUP` (`BUTTON_PULANG_PIN`) |
| **Buzzer** | Kaki Positif (+) | **D2** | **D9 / RX** |  Umpan Balik Audio (`BUZZER_PIN`) |

## ☁️ Logika Program (Konsep Kerja)

### 1. Program Mikrokontroler (ESP32 / ESP8266)

* **Tugas**: Bertindak sebagai klien jaringan yang membaca kartu dan mengirim data.
* **Alur Data**: Membaca UID kartu setelah tombol ditekan.
* **Pengiriman**: Mengirimkan UID dan Status (`Masuk` atau `Pulang`) sebagai *query* URL (`?id=UID&status=Status`) ke URL Web App.
* **Waktu Tunggu**: Memberi batas waktu 10 detik untuk menempelkan kartu sebelum *timeout*.

### 2. Google Apps Script (Sisi Server)

Kode JavaScript ini adalah inti dari mekanisme anti-duplikasi.

1.  **Penerimaan Data**: Fungsi `doGet(e)` menerima UID (`e.parameter.id`) dan Status (`e.parameter.status`).
2.  **Format Waktu**: Menggunakan `Utilities.formatDate()` dengan zona waktu (`Asia/Jakarta`) untuk mendapatkan Tanggal dan Waktu yang konsisten.
3.  **Pencarian Baris**: Skrip mencari baris di `Sheet4` yang memiliki **UID yang sama** (Kolom A) **DAN Tanggal Hari Ini** (Kolom B).
4.  **Logika Update/Append**:
    * **Jika baris ditemukan**: Skrip memperbarui waktu yang sesuai (Kolom C untuk **Masuk** atau Kolom D untuk **Pulang**) pada baris tersebut. **Ini mencegah duplikasi baris**.
    * **Jika baris tidak ditemukan**: Skrip menambahkan baris baru dengan data lengkap.

## 📊 Struktur Data Google Sheets (`Sheet4`)

Pastikan *header* di **Sheet4** diatur dengan tepat agar Apps Script dapat memproses data.

| Kolom A | Kolom B | Kolom C | Kolom D |
| :---: | :---: | :---: | :---: |
| **UID** | **Tanggal Absen** | **Waktu Datang** | **Waktu Pulang** |

## 🚀 Panduan Penggunaan (Deployment)

### 1. Siapkan Spreadsheet & Apps Script

1.  Buat Google Spreadsheet baru.
2.  Pastikan ada *sheet* bernama **`Sheet4`** dengan *header* kolom di atas.
3.  Buka **Ekstensi** > **Apps Script**, salin kode dari `apps sript.js` ke editor.
4.  **Deploy sebagai Web App**: Pilih akses **"Anyone"** dan salin **URL Web App** yang dihasilkan.

### 2. Konfigurasi dan Unggah Kode Mikrokontroler

1.  Buka kode Arduino (`esp32.cpp` atau `esp8266.cpp`).
2.  Masukkan `ssid`, `password` WiFi, dan **URL Web App** yang sudah Anda salin ke dalam variabel yang sesuai.
3.  Unggah kode ke mikrokontroler.

### 3. Uji Coba

1.  Setelah terhubung, tekan tombol **Masuk** dan tempelkan kartu. Data harus tercatat di Kolom C.
2.  Tekan tombol **Pulang** dan tempelkan kartu yang sama. Data harus **diperbarui** di Kolom D, tanpa menambah baris baru.
