# Smart Package Box IoT

Dashboard monitoring kotak penerima paket otomatis berbasis ESP32 dengan Firebase dan Telegram Bot.

## 🚀 Quick Setup

### 1. Hardware
- ESP32 + komponen sesuai wiring diagram
- Upload code Arduino ke ESP32
- Update WiFi credentials di code

### 2. Firebase
- Buat project di Firebase Console
- Enable Realtime Database
- Copy config ke HTML file

### 3. Telegram Bot
- Create bot via @BotFather
- Get bot token dan chat ID
- Update di Arduino code

### 4. Dashboard
- Buka `public/index.html` di browser
- Update ESP32 IP address di HTML

## 📁 Struktur Folder

```
smart-package-box/
├── docs/
│   └── README.md           # Dokumentasi ini
├── public/
│   └── index.html          # Dashboard web
├── src/
│   ├── index.js            # Logic tambahan (opsional)
│   ├── main.js             # JavaScript utilities
│   └── style.css           # CSS tambahan
├── arduino/
│   └── smart_package_box.ino # Code ESP32
├── .gitignore
└── package.json
```

## 🔧 Pin Configuration

| Component | ESP32 Pin |
|-----------|-----------|
| Solenoid Lock | GPIO 27 |
| Buzzer | GPIO 33 |
| PIR Sensor | GPIO 32 |
| SW-420 Vibration | GPIO 35 |
| Servo Lock | GPIO 25 |
| Servo Open | GPIO 26 |
| LCD SDA | GPIO 21 |
| LCD SCL | GPIO 22 |

## 📱 Fitur

- ✅ Real-time monitoring status kotak
- ✅ Kontrol servo dan solenoid via web/Telegram
- ✅ Sensor PIR dan getaran dengan notifikasi
- ✅ Firebase database untuk logging
- ✅ Dashboard responsif dengan chart 24 jam
- ✅ Auto-close timer dan alert system

## 🔗 API Endpoints

- `GET /cek_status` - Status semua komponen
- `GET /buka_paket` - Buka kotak paket
- `GET /tutup_paket` - Tutup kotak paket
- `GET /buka_cod` - Buka COD compartment
- `GET /tutup_cod` - Tutup COD compartment

## 📞 Support

Created by **M.Nawval Alfazri** - Politeknik Negeri Sriwijaya

---
© 2025 Smart Package Box IoT Project