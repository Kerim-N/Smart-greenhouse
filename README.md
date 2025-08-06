# 🌿 Smart Greenhouse on Arduino

## 📋 Description

This project is a **smart greenhouse system** based on the Arduino Mega platform.  
It integrates multiple features: RFID and keypad access control, automatic lighting, climate control, irrigation, and IR remote management.

It’s designed for learning and experimenting with **RFID, EEPROM, servo motors, DHT sensors, photoresistors, and more**.

---

## ⚙️ Features

- 🔐 **Access Control:**
  - Up to 10 RFID cards stored in EEPROM
  - 4-digit PIN code input via 4x4 keypad
  - Internal unlock button

- 🌞 **Smart Light and Sun Tracking:**
  - 3 photoresistors detect light intensity
  - Servo motor rotates the panel toward the light
  - LED lighting turns on automatically in low light

- 🌡️ **Climate Monitoring:**
  - Temperature and humidity detection using DHT11
  - Fan automatically turns on when temperature exceeds threshold

- 💧 **Auto Watering:**
  - Analog water level sensor
  - Pump activates if water level is below threshold

- 🎮 **IR Remote Control:**
  - Adjust temperature and water thresholds
  - Display infrared signal response

- 📟 **LCD Interface:**
  - 16x2 LCD displays temperature, humidity, water level, and door state

---

## 🔌 Hardware Components

| Component           | Purpose                                 |
|---------------------|------------------------------------------|
| Arduino Mega        | Main microcontroller                     |
| MFRC522             | RFID card reader                         |
| DHT11               | Temperature and humidity sensor          |
| LCD 16x2            | Displays system info                     |
| IR Receiver         | Accepts signals from remote              |
| 4x4 Keypad          | Enters PIN code                          |
| SG90 Servo (x2)     | For door lock and solar panel tracking   |
| Photoresistors (x3) | Detect ambient light                     |
| Relay Module        | Controls door lock                       |
| Buzzer              | Sound indication                         |
| EEPROM              | Stores RFID cards                        |

---

## 📟 LCD Display Output


- **H:** Humidity (%)
- **T:** Temperature (°C)
- **D:** Door status (Open or Lock)
- **WL:** Water level (0–100 range)

---

## 🧠 Code Structure

- `lock()` / `unlock()`  
  Lock and unlock the door via servo motor

- `indicate(type)`  
  LED + buzzer indication for SUCCESS, DECLINE, SAVED, or DELETED

- `saveOrDeleteTag()`  
  Adds or deletes an RFID tag to/from EEPROM

- `compareUIDs()`  
  Compares two RFID UIDs byte-by-byte

- `foundTag()`  
  Checks if the scanned RFID exists in EEPROM

- `loop()`  
  Main logic for automatic control of light, water, ventilation, and access

---

## 📦 Required Libraries

- `Servo.h`
- `SPI.h`
- `MFRC522.h`
- `EEPROM.h`
- `LiquidCrystal.h`
- `Wire.h`
- `dht11.h`
- `GyverHacks.h`
- `IRremote.h`
- `Keypad.h`

---

## 🛠️ Possible Improvements

- 📱 Add ESP8266 or ESP32 for Wi-Fi-based monitoring and control
- 📊 Send real-time data to a server or mobile app (e.g., Telegram bot)
- 📈 Log historical temperature/humidity graphs
- 🔔 Add notifications for abnormal conditions (e.g., high temp or low water)
