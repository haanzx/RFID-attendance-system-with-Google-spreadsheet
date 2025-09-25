Tentu\! Berikut adalah penjelasan lengkap mengenai program dan kode yang telah dibuat, disusun dalam format **README** yang siap Anda gunakan di GitHub.

Dokumen ini mencakup penjelasan konsep, daftar *hardware*, skema pin, dan kode lengkap untuk kedua sisi (**ESP32** dan **Google Apps Script**).

# 💻 Sistem Absensi RFID Online (ESP32 & Google Sheets)

Proyek ini adalah sistem absensi modern yang menggunakan modul **RFID RC522** dan mikrokontroler **ESP32** untuk mencatat waktu **Datang** dan **Pulang** ke dalam Google Sheets secara *real-time*. Data absensi dijamin akurat dan tidak terjadi penumpukan data (duplikasi baris) berkat logika pencarian canggih di Google Apps Script.

-----

## 🛠️ Hardware yang Digunakan

| Komponen | Deskripsi |
| :--- | :--- |
| **Mikrokontroler** | ESP32 DEVKIT V1 atau sejenisnya |
| **Pembaca Kartu** | Modul RFID RC522 |
| **Layar** | LCD 16x2 I2C (untuk *feedback* visual) |
| **Umpan Balik Audio** | Buzzer Aktif (untuk *feedback* suara) |
| **Input** | 2x Tombol *Push Button* (untuk Absen Masuk & Pulang) |
| **Kabel** | Kabel Jumper Male-to-Female & Male-to-Male |

-----

## 📌 Skema Wiring (Pinout)

Berikut adalah ringkasan koneksi pin yang digunakan dalam program ini (didasarkan pada penyesuaian pin terakhir Anda):

| Komponen | Pin Modul | Pin ESP32 | Keterangan |
| :--- | :--- | :--- | :--- |
| **RFID RC522** | RST | **GPIO 4** | Reset Pin |
| | SDA (SS) | **GPIO 5** | SPI Slave Select |
| | MOSI | **GPIO 23** | SPI Data Out |
| | MISO | **GPIO 19** | SPI Data In |
| | SCK | **GPIO 18** | SPI Clock |
| **LCD I2C** | SDA | **GPIO 21** | I2C Data |
| | SCL | **GPIO 22** | I2C Clock |
| **Tombol Masuk** | Kaki Sinyal | **GPIO 13** | Menggunakan *Input Pull-up* Internal |
| **Tombol Pulang** | Kaki Sinyal | **GPIO 15** | Menggunakan *Input Pull-up* Internal |
| **Buzzer** | Kaki Positif (+) | **GPIO 2** | Umpan Balik Audio |

*Catatan: Tombol (GPIO 13 & 15) dihubungkan ke **GND** pada kaki satunya untuk memanfaatkan mode `INPUT_PULLUP`.*

-----

## 💻 Kode Program (Arduino IDE - ESP32)

Kode ini mengatur koneksi WiFi, menginisialisasi modul LCD dan RFID, membaca tombol, dan mengirimkan data ke Google Sheets.

### Fitur Utama Kode ESP32:

1.  **Alur Tombol-ke-Kartu**: Program menunggu penekanan tombol (`Masuk` atau `Pulang`) sebelum mengaktifkan pembacaan kartu RFID.
2.  **Umpan Balik LCD**: Menampilkan status koneksi, instruksi, dan hasil pengiriman data.
3.  **Umpan Balik Buzzer**: Bunyi **sekali** untuk absensi berhasil, dan bunyi **cepat berulang** untuk gagal (misalnya, *timeout* atau gagal koneksi).

<!-- end list -->

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// --- Definisi Pin Hardware ---
// Pin RFID (Sesuai penyesuaian terakhir)
#define SS_PIN 5
#define RST_PIN 4  
#define MOSI_PIN 23
#define MISO_PIN 19
#define SCK_PIN 18

MFRC522 mfrc522(SS_PIN, RST_PIN);

// Pin Tombol (Sesuai penyesuaian terakhir)
#define BUTTON_MASUK_PIN 13
#define BUTTON_PULANG_PIN 15

// Pin Buzzer
#define BUZZER_PIN 2 

// Alamat I2C LCD (0x27 adalah yang paling umum)
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// --- Konfigurasi Jaringan & Google Apps Script ---
const char* ssid = "NAMA_WIFI_ANDA"; 
const char* password = "PASSWORD_WIFI_ANDA"; 
const String GOOGLE_SCRIPT_URL = "URL_WEB_APP_ANDA"; // Ganti URL setelah Deploy Apps Script

// Deklarasi Fungsi
void sendDataToGoogleSheets(String uid, String status);
void waitForCardAndSend(String status);
void beepSuccess();
void beepFailure();

// --- Fungsi Buzzer ---
void beepSuccess() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
}

void beepFailure() {
  for (int i = 0; i < 3; i++) { 
    digitalWrite(BUZZER_PIN, HIGH);
    delay(50);
    digitalWrite(BUZZER_PIN, LOW);
    delay(50);
  }
}

// --- Fungsi Setup ---
void setup() {
  Serial.begin(9600);
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();
  
  // Konfigurasi pin
  pinMode(BUTTON_MASUK_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PULANG_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT); 

  // Inisialisasi LCD
  lcd.init(); 
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connect WiFi");
  
  Serial.println("Menghubungkan ke WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.print(".");
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Terhubung!");
  beepSuccess(); 
  delay(2000);
  lcd.clear();
  lcd.print("Siap");
  lcd.setCursor(2,1);
  lcd.print("Tekan Tombol");
}

// --- Fungsi Loop Utama ---
void loop() {
  // Tombol MASUK
  if (digitalRead(BUTTON_MASUK_PIN) == LOW) {
    delay(50); // Debounce
    if (digitalRead(BUTTON_MASUK_PIN) == LOW) {
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("Absen  Masuk");
      lcd.setCursor(2,1);
      lcd.print("Tempel Kartu");
      
      waitForCardAndSend("Masuk");
    }
  }

  // Tombol PULANG
  if (digitalRead(BUTTON_PULANG_PIN) == LOW) {
    delay(50); // Debounce
    if (digitalRead(BUTTON_PULANG_PIN) == LOW) {
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("Absen Pulang");
      lcd.setCursor(2,1);
      lcd.print("Tempel Kartu");
      
      waitForCardAndSend("Pulang");
    }
  }
}

// --- Fungsi Penanganan RFID ---
void waitForCardAndSend(String status) {
  long startTime = millis();
  while (millis() - startTime < 10000) { // Timeout 10 detik
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      String uidString = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        uidString += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
        uidString += String(mfrc522.uid.uidByte[i], HEX);
      }
      
      lcd.setCursor(0, 1);
      lcd.print("Mengirim........");
      
      sendDataToGoogleSheets(uidString, status);
      
      mfrc522.PICC_HaltA();
      delay(2000); 
      
      lcd.clear();
      lcd.print("Siap");
      lcd.setCursor(2,1);
      lcd.print("Tekan tombol");
      return; 
    }
    delay(50);
  }

  // Jika timeout
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Waktu Habis!");
  beepFailure(); 
  delay(2000);
  lcd.clear();
  lcd.print("Siap");
  lcd.setCursor(2,1);
  lcd.print("Tekan tombol");
}

// --- Fungsi Kirim Data ke Google Sheets ---
void sendDataToGoogleSheets(String uid, String status) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    // Mengirim UID dan Status (Masuk/Pulang) ke Apps Script
    String url = GOOGLE_SCRIPT_URL + "?id=" + uid + "&status=" + status;
    
    http.begin(url);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
      
      lcd.clear();
      lcd.print("Data Terkirim!");
      beepSuccess(); 
    } else {
      Serial.println("Error saat mengirim data. HTTP code: " + String(httpCode));
      lcd.clear();
      lcd.print("Gagal Kirim Data!");
      beepFailure(); 
    }
    
    http.end();
  } else {
    Serial.println("WiFi tidak terhubung.");
    lcd.clear();
    lcd.print("WiFi Terputus!");
    beepFailure(); 
  }
}
```

-----

## ☁️ Kode Google Apps Script (Sisi Server)

Kode ini adalah *backend* yang berjalan di server Google. Fungsinya adalah menerima data dari ESP32, melakukan pencarian, dan memperbarui *spreadsheet*.

### Logika Utama Apps Script:

1.  **Cek Duplikasi**: Mencari kombinasi **UID** dan **Tanggal** yang sama pada hari ini.
2.  **Pembaruan Baris**: Jika UID ditemukan, ia tidak membuat baris baru, melainkan memperbarui kolom **Waktu Datang** atau **Waktu Pulang** pada baris yang sudah ada.
3.  **Format Waktu Kuat**: Menggunakan `Utilities.formatDate` untuk memastikan format waktu konsisten (`YYYY-MM-DD` dan `HH:MM:SS`) terlepas dari pengaturan regional pengguna.

<!-- end list -->

```javascript
var SHEET_NAME = "Sheet4"; 
var TIME_ZONE = "Asia/Jakarta"; // Pastikan zona waktu sudah benar

function doGet(e) {
  var data = e.parameter;
  var uid = data.id;
  var status = data.status; 
  var now = new Date(); 
  
  // Format Tanggal dan Waktu
  var dateString = Utilities.formatDate(now, TIME_ZONE, "yyyy-MM-dd");
  var timeString = Utilities.formatDate(now, TIME_ZONE, "HH:mm:ss");

  if (!uid || !status) {
    return ContentService.createTextOutput("Error: UID atau Status tidak terkirim.");
  }
  
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName(SHEET_NAME);
  
  var dataRange = sheet.getDataRange();
  var values = dataRange.getValues();
  
  // Indeks Kolom (di Apps Script, indeks dimulai dari 0)
  var UID_COLUMN_INDEX = 0;    // Kolom A
  var DATE_COLUMN_INDEX = 1;   // Kolom B
  var DATANG_COLUMN_INDEX = 2; // Kolom C
  var PULANG_COLUMN_INDEX = 3; // Kolom D

  // Mulai dari baris kedua (indeks 1) untuk melewati header
  for (var i = 1; i < values.length; i++) { 
    var rowUID = values[i][UID_COLUMN_INDEX]; 
    var rowDate = values[i][DATE_COLUMN_INDEX]; 

    // Mencari baris yang cocok (UID DAN Tanggal hari ini)
    if (rowUID === uid && rowDate === dateString) {
      
      // Jika status 'Masuk', perbarui Kolom C (Waktu Datang)
      if (status === 'Masuk') {
        sheet.getRange(i + 1, DATANG_COLUMN_INDEX + 1).setValue(timeString);
      } 
      // Jika status 'Pulang', perbarui Kolom D (Waktu Pulang)
      else if (status === 'Pulang') {
        sheet.getRange(i + 1, PULANG_COLUMN_INDEX + 1).setValue(timeString);
      }
      
      return ContentService.createTextOutput("Data Berhasil Diperbarui: " + status);
    }
  }

  // Jika UID tidak ditemukan pada tanggal hari ini, tambahkan baris baru
  var newRowData;
    
  if (status === 'Masuk') {
    // [UID, Tanggal, Waktu Datang, Waktu Pulang]
    newRowData = [uid, dateString, timeString, '']; 
  } else {
    // Jika Pulang duluan, Masuk dikosongkan
    newRowData = [uid, dateString, '', timeString]; 
  }

  sheet.appendRow(newRowData);
  return ContentService.createTextOutput("Data Baru Berhasil Ditambahkan: " + status);
}
```

-----

## 🚀 Cara Penggunaan Sistem (Langkah-langkah)

### A. Persiapan Google Sheets

1.  Buat *Spreadsheet* baru di Google Drive.
2.  Pastikan nama salah satu *sheet* adalah **`Sheet4`**.
3.  Di **`Sheet4`**, atur *header* kolom pada baris pertama persis seperti ini:

| Kolom A | Kolom B | Kolom C | Kolom D |
| :---: | :---: | :---: | :---: |
| **UID** | **Tanggal Absen** | **Waktu Datang** | **Waktu Pulang** |

4.  Di menu, klik **Ekstensi** \> **Apps Script**.
5.  Salin dan tempel Kode **Google Apps Script** di atas ke editor.
6.  Klik **Simpan**.
7.  Klik **Deploy** \> **New Deployment**.
8.  Pilih jenis **Web App**. Atur "Execute as" ke **"Me"** dan "Who has access" ke **"Anyone"**.
9.  Klik **Deploy** dan salin **URL Web App** yang muncul.

### B. Persiapan Arduino IDE & ESP32

1.  Buka Arduino IDE. Pastikan *board* ESP32 telah terinstal.
2.  Pastikan Anda telah menginstal pustaka: `MFRC522`, `LiquidCrystal_I2C`, `SPI`, dan `Wire`.
3.  Salin dan tempel Kode **Arduino (ESP32)** di atas.
4.  Ganti nilai `ssid`, `password`, dan `GOOGLE_SCRIPT_URL` dengan data yang valid.
5.  Pilih *port* dan *board* yang benar, lalu **Upload** kode ke ESP32.

### C. Pengujian Absensi

1.  Setelah *upload* berhasil, ESP32 akan terhubung ke WiFi.
2.  **Untuk Absen Masuk**: Tekan tombol yang terhubung ke **GPIO 13**. LCD akan menampilkan "Tempel Kartu". Tempelkan kartu RFID.
3.  **Untuk Absen Pulang**: Tekan tombol yang terhubung ke **GPIO 15**. LCD akan menampilkan "Tempel Kartu". Tempelkan kartu RFID yang sama.
4.  **Verifikasi**: Cek **`Sheet4`** di Google Sheets. Absensi pertama akan mengisi Kolom A, B, dan C. Absensi Pulang akan **memperbarui** Kolom D pada baris yang sama. Jika absensi gagal, *buzzer* akan berbunyi cepat.
