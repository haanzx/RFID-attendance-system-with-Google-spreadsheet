#include <WiFi.h>
#include <HTTPClient.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Definisi pin RFID
#define SS_PIN 5
#define RST_PIN 4
#define MOSI_PIN 23
#define MISO_PIN 19
#define SCK_PIN 18

MFRC522 mfrc522(SS_PIN, RST_PIN);

// Definisi pin untuk tombol
#define BUTTON_MASUK_PIN 13
#define BUTTON_PULANG_PIN 15

// Alamat I2C LCD (0x27 atau 0x3F) dan ukuran LCD (16 kolom, 2 baris)
LiquidCrystal_I2C lcd(0x27, 16, 2); 

const char* ssid = "MG-Service"; // Ganti
const char* password = "onomugi1945"; // Ganti

const String GOOGLE_SCRIPT_URL = "https://script.google.com/macros/s/AKfycbwKOw03y-_ER1Hh3iMJjxnsUdfh227k_rQh7zWS64DBKa2B0Ec1CtIhvUSPnLXPPPit/exec"; // Ganti

void sendDataToGoogleSheets(String uid, String status);
void waitForCardAndSend(String status);

void setup() {
  Serial.begin(9600);
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();
  
  // Konfigurasi pin tombol sebagai input pull-up
  pinMode(BUTTON_MASUK_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PULANG_PIN, INPUT_PULLUP);

  // Inisialisasi LCD
  lcd.init(); 
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connect WiFi");
  lcd.setCursor(0, 1);
  
  Serial.println("Menghubungkan ke WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.print(".");
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Terhubung!");
  delay(2000);
  lcd.clear();
  lcd.print("Siap");
  lcd.setCursor(2,1);
  lcd.print("Tekan Tombol");
}

void loop() {
  // Cek apakah tombol "Masuk" ditekan (LOW karena pull-up)
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

  // Cek apakah tombol "Pulang" ditekan (LOW karena pull-up)
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
      return; // Keluar dari fungsi
    }
    delay(50);
  }

  // Jika timeout
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Waktu Habis!");
  delay(2000);
  lcd.clear();
  lcd.print("Siap");
  lcd.setCursor(2,1);
  lcd.print("Tekan tombol");
}

void sendDataToGoogleSheets(String uid, String status) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = GOOGLE_SCRIPT_URL + "?id=" + uid + "&status=" + status;
    
    http.begin(url);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
      
      lcd.clear();
      lcd.print("Data Terkirim!");
    } else {
      Serial.println("Error saat mengirim data.");
      lcd.clear();
      lcd.print("Gagal Kirim Data!");
    }
    
    http.end();
  } else {
    Serial.println("WiFi tidak terhubung.");
    lcd.clear();
    lcd.print("WiFi Terputus!");
  }
}