#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- Pin Definition ---
// RFID Pin
#define SS_PIN D8   
#define RST_PIN D0  
// Switch Pin 
#define BUTTON_MASUK_PIN D3  
#define BUTTON_PULANG_PIN D4  
// Buzzer Pin
#define BUZZER_PIN D10

// --- Object Initialization ---
MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Network Configuration & Google Apps Script ---
const char* ssid = "your-ssid";
const char* password = "your-password-wifi";
const String GOOGLE_SCRIPT_URL = "your-google-script-url";

// --- Function Declaration ---
void sendDataToGoogleSheets(String uid, String status);
void waitForCardAndSend(String status);
void beepSuccess();
void beepFailure(); 

void setup() {
    Serial.begin(9600);
    Wire.begin(); 
    SPI.begin();
    mfrc522.PCD_Init();
    
    // --- Pin Konfiguration ---
    pinMode(BUTTON_MASUK_PIN, INPUT_PULLUP);
    pinMode(BUTTON_PULANG_PIN, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);

  // --- Initialize LCD ---
  lcd.init(); 
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connect WiFi");
  lcd.setCursor(0, 1);

    // --- WiFi Settings ---
  Serial.println("Menghubungkan ke WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.print(".");
    Serial.print(".");
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Terhubung!");
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  beepSuccess(); // Bunyi sekali saat koneksi berhasil
  delay(2000);
  lcd.clear();
  lcd.print("Siap");
  lcd.setCursor(2,1);
  lcd.print("Tekan Tombol");
}

void loop() {
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
      return; // Keluar dari fungsi
    }
    delay(50);
  }

  // Jika timeout
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Waktu Habis!");
  beepFailure(); // Bunyi gagal saat timeout
  delay(2000);
  lcd.clear();
  lcd.print("Siap");
  lcd.setCursor(2,1);
  lcd.print("Tekan tombol");
}

// --- Fungsi Kirim Data ke Google Sheets ---
void sendDataToGoogleSheets(String uid, String status) {
  if (WiFi.status() == WL_CONNECTED) {
    // Gunakan WiFiClientSecure untuk koneksi HTTPS ke Google Sheets
    WiFiClientSecure client;
    client.setInsecure(); // Opsional: mengabaikan verifikasi sertifikat SSL
    // Inisialisasi HTTP Client
    HTTPClient http;
    String url = GOOGLE_SCRIPT_URL + "?id=" + uid + "&status=" + status;
    // Perubahan: Gunakan klien yang aman (secure client)
    if (http.begin(client, url)) { // Coba koneksi dengan klien aman 
        int httpCode = http.GET();
        
        if (httpCode > 0) {
          String payload = http.getString();
          Serial.println(payload);
          
          lcd.clear();
          lcd.print("Data Terkirim!");
          beepSuccess(); // Bunyi sekali saat data berhasil dikirim
        } else {
          Serial.println("Error saat mengirim data. HTTP code: " + String(httpCode));
          lcd.clear();
          lcd.print("Gagal Kirim Data!");
          beepFailure(); // Bunyi berkali-kali saat gagal kirim
        }
        
        http.end();
    } else {
        Serial.println("Gagal memulai koneksi HTTP.");
        lcd.clear();
        lcd.print("HTTP Error!");
        beepFailure();
    }
  } else {
    Serial.println("WiFi tidak terhubung.");
    lcd.clear();
    lcd.print("WiFi Terputus!");
    beepFailure(); // Bunyi berkali-kali jika WiFi terputus
  }
}

// --- Buzzer Function ---
// Buzzer bunyi sekali (Success)
void beepSuccess() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
}
// Buzzer bunyi cepat berkali-kali (Failure)
void beepFailure() {
  for (int i = 0; i < 3; i++) { // Bunyi 3 kali cepat
    digitalWrite(BUZZER_PIN, HIGH);
    delay(50);
    digitalWrite(BUZZER_PIN, LOW);
    delay(50);
  }
}
