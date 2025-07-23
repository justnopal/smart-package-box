#include <WiFi.h> //Library yang di pakai
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <UniversalTelegramBot.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <time.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

// WiFi & Credentials
const char* ssid = "Naktaubae";  // GANTI dengan WiFi Anda
const char* password = "Gilang.alf123"; // GANTI Password WiFi anda

// Telegram
#define BOTtoken "8039590983:AAGQU8YEdKN6I8T_IlzbGuUJYVI150E6-dY" //Masukan Token dari BotFather
#define CHAT_ID "7984670497"              // Chat_ID_token

// Firebase Configuration
const char* FIREBASE_HOST = "smart-package-box-bfdbc-default-rtdb.asia-southeast1.firebasedatabase.app";
const char* FIREBASE_AUTH = "AIzaSyCGqkuKVjFNf7-zyQhoCHUgZtWJmMh0ces"; // Use your API key

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
WebServer server(80);

// Pin setup
#define solenoidPin 27
#define buzzerPin 33
#define pirPin 32
#define sw420Pin 35
#define servoLockPin 25
#define servoOpenPin 26

Servo servoLock;
Servo servoOpen;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Status & waktu
bool paketTerbuka = false;
bool codTerbuka = false;
bool instruksi30DetikShown = false;
unsigned long paketBukaTime = 0;
unsigned long codBukaTime = 0;
int jumlahPaketMasuk = 0;

// Sensor PIR delay
unsigned long lastPirCheck = 0;
const unsigned long pirInterval = 120000;
unsigned long pirDetectedAt = 0;
bool pirActive = false;

// Firebase timing
unsigned long lastFirebaseUpdate = 0;
const unsigned long firebaseUpdateInterval = 3000; // Update setiap 3 detik

String getDateTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "Time Error";
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%H:%M:%S | %d-%m-%Y", &timeinfo);
  return String(buffer);
}

String getTimeOnly() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "Time Error";
  char buffer[10];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
  return String(buffer);
}

void buzzerBeep(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(200);
    digitalWrite(buzzerPin, LOW);
    delay(200);
  }
}

void showDefaultLCD() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(" Kotak Penerima ");
  lcd.setCursor(0, 1); lcd.print("     Paket      ");
}

// ===== FIREBASE FUNCTIONS =====
void sendToFirebase(String path, String jsonData) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://" + String(FIREBASE_HOST) + path + ".json?auth=" + String(FIREBASE_AUTH));
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.PUT(jsonData);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("✅ Firebase Update: " + response);
    } else {
      Serial.println("❌ Firebase Error: " + String(httpResponseCode));
    }
    http.end();
  }
}

void pushToFirebaseLogs(String jsonData) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://" + String(FIREBASE_HOST) + "/logs.json?auth=" + String(FIREBASE_AUTH));
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.POST(jsonData);
    
    if (httpResponseCode > 0) {
      Serial.println("✅ Firebase Log Added");
    } else {
      Serial.println("❌ Firebase Log Error: " + String(httpResponseCode));
    }
    http.end();
  }
}

void updateFirebaseStatus() {
  StaticJsonDocument<300> doc;
  doc["paket"] = paketTerbuka;
  doc["cod"] = codTerbuka;
  doc["pir"] = digitalRead(pirPin);
  doc["sw"] = digitalRead(sw420Pin);
  doc["jumlah_paket"] = jumlahPaketMasuk;
  doc["timestamp"] = getDateTime();
  doc["last_update"] = millis();
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  sendToFirebase("/status", jsonString);
  
  Serial.println("📡 Status sent to Firebase: " + jsonString);
}

void addFirebaseLog(String sensor, int value, String action = "") {
  StaticJsonDocument<200> doc;
  doc["time"] = getTimeOnly();
  doc["pir"] = (sensor == "pir") ? value : digitalRead(pirPin);
  doc["sw"] = (sensor == "sw") ? value : digitalRead(sw420Pin);
  doc["timestamp"] = getDateTime();
  if (action != "") {
    doc["action"] = action;
  }
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  pushToFirebaseLogs(jsonString);
  
  Serial.println("📝 Log added to Firebase: " + jsonString);
}

// ----- Handler Web -----
void handleStatus() {
  StaticJsonDocument<300> doc;
  doc["paket"] = paketTerbuka;
  doc["cod"] = codTerbuka;
  doc["pir"] = digitalRead(pirPin);
  doc["sw"] = digitalRead(sw420Pin);
  doc["jumlah_paket"] = jumlahPaketMasuk;
  doc["timestamp"] = getDateTime();
  doc["uptime"] = millis();

  String json;
  serializeJson(doc, json);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(200, "application/json", json);
}

void handleKontrol(String aksi) {
  Serial.println("🎛️ Kontrol: " + aksi);
  
  if (aksi == "buka_paket") {
    digitalWrite(solenoidPin, HIGH);
    buzzerBeep(1);
    paketTerbuka = true;
    paketBukaTime = millis();
    instruksi30DetikShown = false;
    lcd.clear(); lcd.setCursor(0, 0); lcd.print("Masukkan Paket");
    lcd.setCursor(0, 1); lcd.print("dengan Hati-hati");
    bot.sendMessage(CHAT_ID, "📦 Paket Dibuka\n🕒 " + getDateTime(), "");
    
    // Update Firebase
    updateFirebaseStatus();
    addFirebaseLog("action", 1, "buka_paket");
  }
  
  if (aksi == "tutup_paket") {
    digitalWrite(solenoidPin, LOW);
    buzzerBeep(1);
    if (paketTerbuka) jumlahPaketMasuk++;
    paketTerbuka = false;
    lcd.clear(); lcd.setCursor(0, 0); lcd.print("Paket Tertutup");
    lcd.setCursor(0, 1); lcd.print("Dengan Aman");
    bot.sendMessage(CHAT_ID, "🔐 Paket Terkunci\n🕒 " + getDateTime(), "");
    delay(2000);
    showDefaultLCD();
    
    // Update Firebase
    updateFirebaseStatus();
    addFirebaseLog("action", 0, "tutup_paket");
  }
  
  if (aksi == "buka_cod") {
    servoLock.write(30); delay(1200);
    servoOpen.write(90);
    buzzerBeep(1);
    codTerbuka = true;
    codBukaTime = millis();
    lcd.clear(); lcd.setCursor(0, 0); lcd.print("COD Dibuka");
    lcd.setCursor(0, 1); lcd.print("Ambil & Bayar");
    bot.sendMessage(CHAT_ID, "💵 COD Dibuka\n🕒 " + getDateTime(), "");
    
    // Update Firebase
    updateFirebaseStatus();
    addFirebaseLog("action", 1, "buka_cod");
  }
  
  if (aksi == "tutup_cod") {
    servoOpen.write(0); delay(1200);
    servoLock.write(0);
    buzzerBeep(1);
    codTerbuka = false;
    lcd.clear(); lcd.setCursor(0, 0); lcd.print("COD Tertutup");
    lcd.setCursor(0, 1); lcd.print("Dengan Aman");
    bot.sendMessage(CHAT_ID, "📥 COD Terkunci\n🕒 " + getDateTime(), "");
    delay(2000);
    showDefaultLCD();
    
    // Update Firebase
    updateFirebaseStatus();
    addFirebaseLog("action", 0, "tutup_cod");
  }
  
  handleStatus(); // return status langsung
}

// CORS Handler
void handleCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(200, "text/plain", "OK");
}

void setupWebServer() {
  // CORS preflight
  server.on("/cek_status", HTTP_OPTIONS, handleCORS);
  server.on("/buka_paket", HTTP_OPTIONS, handleCORS);
  server.on("/tutup_paket", HTTP_OPTIONS, handleCORS);
  server.on("/buka_cod", HTTP_OPTIONS, handleCORS);
  server.on("/tutup_cod", HTTP_OPTIONS, handleCORS);
  
  // Main endpoints
  server.on("/cek_status", HTTP_GET, handleStatus);
  server.on("/buka_paket", HTTP_GET, []() { handleKontrol("buka_paket"); });
  server.on("/tutup_paket", HTTP_GET, []() { handleKontrol("tutup_paket"); });
  server.on("/buka_cod", HTTP_GET, []() { handleKontrol("buka_cod"); });
  server.on("/tutup_cod", HTTP_GET, []() { handleKontrol("tutup_cod"); });
  
  // Root endpoint untuk testing
  server.on("/", []() {
    String html = "<h1>ESP32 Smart Package Box</h1>";
    html += "<p>Status: Connected</p>";
    html += "<p>IP: " + WiFi.localIP().toString() + "</p>";
    html += "<p>Uptime: " + String(millis()/1000) + " seconds</p>";
    server.send(200, "text/html", html);
  });
  
  server.begin();
  Serial.println("🌐 Web Server Started on: http://" + WiFi.localIP().toString());
}

// ----- Telegram -----
void handleTextCommand(int num) {
  for (int i = 0; i < num; i++) {
    String text = bot.messages[i].text;
    String chat_id = bot.messages[i].chat_id;
    
    if (text == "/menu") {
      String keyboardJson = F("{\"inline_keyboard\":["
        "[{\"text\":\"📤 Buka Paket\",\"callback_data\":\"BUKA_PINTU\"},"
        "{\"text\":\"🔐 Tutup Paket\",\"callback_data\":\"TUTUP_PINTU\"}],"
        "[{\"text\":\"💵 Buka COD\",\"callback_data\":\"BUKA_COD\"},"
        "{\"text\":\"📥 Tutup COD\",\"callback_data\":\"TUTUP_COD\"}],"
        "[{\"text\":\"📊 Cek Status\",\"callback_data\":\"CEK_STATUS\"}]]}");
      bot.sendMessageWithInlineKeyboard(chat_id, "*📦 Kotak Aktif, pilih menu:*", "Markdown", keyboardJson);
    }

    if (text == "/buka_paket") handleKontrol("buka_paket");
    if (text == "/tutup_paket") handleKontrol("tutup_paket");
    if (text == "/buka_cod") handleKontrol("buka_cod");
    if (text == "/tutup_cod") handleKontrol("tutup_cod");
    if (text == "/cek_status") {
      String status = paketTerbuka ? "Terbuka" : "Tertutup";
      String codStatus = codTerbuka ? "Terbuka" : "Tertutup";
      String message = "📊 *Status Kotak:*\n";
      message += "📦 Paket: " + status + "\n";
      message += "💵 COD: " + codStatus + "\n";
      message += "📈 Total Paket: " + String(jumlahPaketMasuk) + "\n";
      message += "🕒 " + getDateTime();
      bot.sendMessage(chat_id, message, "Markdown");
    }
    if (text == "/ip") {
      bot.sendMessage(chat_id, "🌐 IP Address: " + WiFi.localIP().toString(), "");
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("🚀 Starting ESP32 Smart Package Box...");
  
  // WiFi Connection
  WiFi.begin(ssid, password);
  client.setInsecure();

  Serial.print("📡 Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); 
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected!");
  Serial.println("🌐 IP Address: " + WiFi.localIP().toString());

  // Time configuration
  configTime(25200, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("⏰ Time configured (WIB UTC+7)");

  // LCD & Hardware
  Wire.begin(21, 22); //Pin Lcd di ESP 32
  lcd.init(); 
  lcd.backlight(); 
  showDefaultLCD();

  pinMode(solenoidPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(pirPin, INPUT);
  pinMode(sw420Pin, INPUT);
  
  servoLock.attach(servoLockPin);
  servoOpen.attach(servoOpenPin);
  servoLock.write(0); 
  servoOpen.write(0);

  // Initial status
  digitalWrite(solenoidPin, LOW);
  digitalWrite(buzzerPin, LOW);

  Serial.println("🔧 Hardware initialized");

  // Start services
  setupWebServer();
  
  // Send startup notification
  bot.sendMessage(CHAT_ID, "🚀 Smart Package Box Online!\n🌐 IP: " + WiFi.localIP().toString() + "\n🕒 " + getDateTime(), "");
  
  // Initial Firebase update
  updateFirebaseStatus();
  addFirebaseLog("system", 1, "startup");
  
  Serial.println("✅ System Ready!");
  buzzerBeep(2); // Startup sound
}

void loop() {
  // Telegram polling
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  if (numNewMessages) {
    handleTextCommand(numNewMessages);
  }

  // Handle web requests
  server.handleClient();

  // Periodic Firebase update
  if (millis() - lastFirebaseUpdate >= firebaseUpdateInterval) {
    updateFirebaseStatus();
    lastFirebaseUpdate = millis();
  }

  // Auto tutup paket setelah 1 menit
  if (paketTerbuka && millis() - paketBukaTime >= 60000) {
    Serial.println("⏰ Auto close paket (timeout)");
    handleKontrol("tutup_paket");
  }

  // Auto tutup COD setelah 40 detik
  if (codTerbuka && millis() - codBukaTime >= 40000) {
    Serial.println("⏰ Auto close COD (timeout)");
    handleKontrol("tutup_cod");
  }

  // PIR deteksi (minimal 2 detik aktivasi)
  if (millis() - lastPirCheck >= pirInterval) {
    if (digitalRead(pirPin) == HIGH) {
      if (!pirActive) {
        pirDetectedAt = millis();
        pirActive = true;
        Serial.println("👁️ PIR activated");
      } else if (millis() - pirDetectedAt >= 2000) {
        Serial.println("👁️ PIR confirmed (2s+)");
        bot.sendMessage(CHAT_ID, "👁️ Kurir Terdeteksi\n🕒 " + getDateTime(), "");
        buzzerBeep(1);
        lcd.clear(); 
        lcd.setCursor(0, 0); lcd.print("Kurir Terdeteksi");
        lcd.setCursor(0, 1); lcd.print("Tunggu Konfirmasi");
        
        // Add to Firebase log
        addFirebaseLog("pir", 1, "kurir_detected");
        
        delay(3000); 
        showDefaultLCD();
        lastPirCheck = millis();
        pirActive = false;
      }
    } else {
      if (pirActive) {
        Serial.println("👁️ PIR deactivated");
        pirActive = false;
      }
    }
  }

  // Sensor getaran
  if (digitalRead(sw420Pin) == HIGH) {
    Serial.println("📳 Vibration detected!");
    bot.sendMessage(CHAT_ID, "🚨 Getaran Terdeteksi!\n🕒 " + getDateTime(), "");
    lcd.clear(); 
    lcd.setCursor(0, 0); lcd.print("Getaran Terdeteksi");
    lcd.setCursor(0, 1); lcd.print("Harap Periksa!");
    
    // Add to Firebase log
    addFirebaseLog("sw", 1, "vibration_detected");
    
    // Alert buzzer
    for (int i = 0; i < 6; i++) {
      digitalWrite(buzzerPin, HIGH); delay(200);
      digitalWrite(buzzerPin, LOW); delay(200);
    }
    delay(5000); 
    showDefaultLCD();
  }

  // Small delay untuk stabilitas
  delay(100);
}
