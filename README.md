# ESP32-Openweather-Display 🌦️

A lightweight, high-efficiency current weather display client for the **ESP32-C3** and **SSD1306 128x64 I2C OLED**, utilizing the **U8g2** graphics framework and the **OpenWeather Current Weather 2.5 API**.

## 🛠️ Features
- **Dynamic Font Formatting**: Large temperature numbers paired with an elegantly scaled small `°C` unit display.
- **Custom 16x16 Weather Icons**: Uses tailored, space-saving pixel arrays mapped dynamically to OpenWeather condition codes.
- **Live Localized Metrics**: Real-time extraction of wind speed (converted from m/s to `km/h`) and humidity data.
- **Optimized Memory Footprint**: Bypasses heavy font libraries by using custom XBMP bitmap buffers.

## 📐 Hardware Connection Setup

| ESP32-C3 Pin | SSD1306 OLED Pin | Description |
| :--- | :--- | :--- |
| **GPIO 6** | SDA | I2C Data Line |
| **GPIO 7** | SCL | I2C Clock Line |
| **3V3** | VCC | 3.3V Logic Power |
| **GND** | GND | Common Ground |

## 📦 Core Library Dependencies
Ensure you have the following libraries installed via the Arduino IDE Library Manager:
- `U8g2` by oliver
- `ArduinoJson` (v6.x or newer)
- Standard ESP32 `WiFi` and `HTTPClient` modules
